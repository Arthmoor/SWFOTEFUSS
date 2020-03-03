/* 

SWFotE copyright (c) 2002 was created by
Chris 'Tawnos' Dary (cadary@uwm.edu),
Korey 'Eleven' King (no email),
Matt 'Trillen' White (mwhite17@ureach.com),
Daniel 'Danimal' Berrill (danimal924@yahoo.com),
Richard 'Bambua' Berrill (email unknown),
Stuart 'Ackbar' Unknown (email unknown)

SWR 1.0 copyright (c) 1997, 1998 was created by Sean Cooper
based on a concept and ideas from the original SWR immortals: 
Himself (Durga), Mark Matt (Merth), Jp Coldarone (Exar), Greg Baily (Thrawn), 
Ackbar, Satin, Streen and Bib as well as much input from our other builders 
and players.

Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,
Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,
Grishnakh, Fireblade, and Nivek.

Original MERC 2.1 code by Hatchet, Furey, and Kahn.

Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,
Michael Seifert, and Sebastian Hammer.

*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "mud.h"

#define RESTORE_INTERVAL 21600

char *const save_flag[] =
   { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap", "auction", "get", "receive", "idle", "backup", "r13",
   "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30",
   "r31"
};


/* from comm.c */
bool write_to_descriptor args( ( int desc, char *txt, int length ) );

/* from space.c */
void remship args( ( SHIP_DATA * ship ) );

/*
 * Local functions.
 */
void save_banlist args( ( void ) );
void ostat_plus args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
int get_color( char *argument ); /* function proto */

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

int get_saveflag( char *name )
{
   int x;

   for( x = 0; x < sizeof( save_flag ) / sizeof( save_flag[0] ); x++ )
      if( !str_cmp( name, save_flag[x] ) )
         return x;
   return -1;
}

void do_wizhelp( CHAR_DATA * ch, char *argument )
{
   CMDTYPE *cmd;
   int col, hash;
   int curr_lvl;
   col = 0;
   set_pager_color( AT_WHITE, ch );

   for( curr_lvl = LEVEL_AVATAR; curr_lvl <= get_trust( ch ); curr_lvl++ )
   {
      send_to_pager( "\n\r\n\r", ch );
      col = 1;
      pager_printf( ch, "[LEVEL %-2d]  ", curr_lvl );
      for( hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
            if( ( cmd->level == curr_lvl ) && cmd->level <= get_trust( ch ) )
            {
               pager_printf( ch, "%-12s", cmd->name );
               if( ++col % 6 == 0 )
                  send_to_pager( "\n\r", ch );
            }
   }
   if( col % 6 != 0 )
      send_to_pager( "\n\r", ch );
   return;
}

/*void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    CMDTYPE * cmd;
    int col, hash;

    col = 0;
    set_pager_color( AT_PLAIN, ch );
    for ( hash = 0; hash < 126; hash++ )
	for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	    if ( cmd->level >= LEVEL_HERO
	    &&   cmd->level <= get_trust( ch ) )
	    {
		pager_printf( ch, "&G(&W%-2d&G)&W %-15s", cmd->level, cmd->name );
		if ( ++col % 4 == 0 )
		    send_to_pager( "\n\r", ch );
	    }

    if ( col % 6 != 0 )
	send_to_pager( "\n\r", ch );
    return;
}*/

void do_restrict( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   short level, hash;
   CMDTYPE *cmd;
   bool found;

   found = FALSE;

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Restrict which command?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg2 );
   if( arg2[0] == '\0' )
      level = get_trust( ch );
   else
      level = atoi( arg2 );

   level = UMAX( UMIN( get_trust( ch ), level ), 0 );

   hash = arg[0] % 126;
   for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
   {
      if( !str_prefix( arg, cmd->name ) && cmd->level <= get_trust( ch ) )
      {
         found = TRUE;
         break;
      }
   }

   if( found )
   {
      if( !str_prefix( arg2, "show" ) )
      {
         sprintf( buf, "%s show", cmd->name );
         do_cedit( ch, buf );
/*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
         return;
      }
      cmd->level = level;
      ch_printf( ch, "You restrict %s to level %d\n\r", cmd->name, level );
      sprintf( buf, "%s restricting %s to level %d", ch->name, cmd->name, level );
      log_string( buf );
   }
   else
      send_to_char( "You may not restrict that command.\n\r", ch );

   return;
}

/* 
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc( CHAR_DATA * ch, char *name )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ret_char;
   static unsigned int number_of_hits;

   number_of_hits = 0;
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->character && ( !str_prefix( name, d->character->name ) ) && IS_WAITING_FOR_AUTH( d->character ) )
      {
         if( ++number_of_hits > 1 )
         {
            ch_printf( ch, "%s does not uniquely identify a char.\n\r", name );
            return NULL;
         }
         ret_char = d->character;   /* return current char on exit */
      }
   }
   if( number_of_hits == 1 )
      return ret_char;
   else
   {
      send_to_char( "No one like that waiting for authorization.\n\r", ch );
      return NULL;
   }
}

void do_authorize( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage:  authorize <player> <yes|name|no/deny>\n\r", ch );
      send_to_char( "Pending authorizations:\n\r", ch );
      send_to_char( " Chosen Character Name\n\r", ch );
      send_to_char( "---------------------------------------------\n\r", ch );
      for( d = first_descriptor; d; d = d->next )
         if( ( victim = d->character ) != NULL && IS_WAITING_FOR_AUTH( victim ) )
            ch_printf( ch, " %s@%s new %s...\n\r", victim->name, victim->desc->host, race_table[victim->race].race_name );
      return;
   }

   victim = get_waiting_desc( ch, arg1 );
   if( victim == NULL )
      return;

   if( arg2[0] == '\0' || !str_cmp( arg2, "accept" ) || !str_cmp( arg2, "yes" ) )
   {
      victim->pcdata->auth_state = 3;
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_UNAUTHED );
      if( victim->pcdata->authed_by )
         STRFREE( victim->pcdata->authed_by );
      victim->pcdata->authed_by = QUICKLINK( ch->name );
      sprintf( buf, "%s authorized %s", ch->name, victim->name );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
      ch_printf( ch, "You have authorized %s.\n\r", victim->name );

      /*
       * Below sends a message to player when name is accepted - Brittany   
       */

      ch_printf( victim,   /* B */
                 "The MUD Administrators have accepted the name %s.\n\r"   /* B */
                 "You are now fully authorized to play SWFotE.\n\r", victim->name );   /* B */
      return;
   }
   else if( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
   {
      send_to_char( "You have been denied access.\n\r", victim );
      sprintf( buf, "%s denied authorization to %s", ch->name, victim->name );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
      ch_printf( ch, "You have denied %s.\n\r", victim->name );
      do_quit( victim, "" );
   }

   else if( !str_cmp( arg2, "name" ) || !str_cmp( arg2, "n" ) )
   {
      sprintf( buf, "%s has denied %s's name", ch->name, victim->name );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
      ch_printf( victim,
                 "The MUD Administrators have found the name %s "
                 "to be unacceptable.\n\r" "Use 'name' to change it to something more apropriate.\n\r", victim->name );
      ch_printf( ch, "You requested %s change names.\n\r", victim->name );
      victim->pcdata->auth_state = 2;
      return;
   }

   else
   {
      send_to_char( "Invalid argument.\n\r", ch );
      return;
   }
}

void do_bamfin( CHAR_DATA * ch, char *argument )
{
   if( !IS_NPC( ch ) )
   {
      smash_tilde( argument );
      DISPOSE( ch->pcdata->bamfin );
      ch->pcdata->bamfin = str_dup( argument );
      send_to_char( "Ok.\n\r", ch );
   }
   return;
}



void do_bamfout( CHAR_DATA * ch, char *argument )
{
   if( !IS_NPC( ch ) )
   {
      smash_tilde( argument );
      DISPOSE( ch->pcdata->bamfout );
      ch->pcdata->bamfout = str_dup( argument );
      send_to_char( "Ok.\n\r", ch );
   }
   return;
}

// Still used for setrank self. If you're feeling ambitious, port
// the crud over there to get rid of one command.
void do_rank( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: setrank  self <string/none>\n\r", ch );
      return;
   }

   if( strlen( argument ) > 40 || strlen( remand( argument ) ) > 19 )
   {
      send_to_char( "&RThat rank is too long. Choose one under 40 characters with color codes and under 20 without.\n\r",
                    ch );
      return;
   }

   smash_tilde( argument );
   argument = rembg( argument );
   if( !str_cmp( argument, "none" ) )
      ch->rank = str_dup( "" );
   else
      ch->rank = str_dup( centertext( argument, 19 ) );


   ch_printf( ch, "Your rank is now: %s", argument );
   return;
}


void do_retire( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Retire whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( victim->top_level < LEVEL_SAVIOR )
   {
      send_to_char( "The minimum level for retirement is savior.\n\r", ch );
      return;
   }

   if( IS_RETIRED( victim ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s returns from retirement.\n\r", victim->name );
      ch_printf( victim, "%s brings you back from retirement.\n\r", ch->name );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s is now a retired immortal.\n\r", victim->name );
      ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\n\r", ch->name );
   }
   return;
}

void do_deny( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Deny whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   SET_BIT( victim->act, PLR_DENY );
   send_to_char( "You are denied access!\n\r", victim );
   send_to_char( "OK.\n\r", ch );
   do_quit( victim, "" );

   return;
}



void do_disconnect( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Disconnect whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim->desc == NULL )
   {
      act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( get_trust( ch ) <= get_trust( victim ) )
   {
      send_to_char( "They might not like that...\n\r", ch );
      return;
   }

   for( d = first_descriptor; d; d = d->next )
   {
      if( d == victim->desc )
      {
         close_socket( d, FALSE );
         send_to_char( "Ok.\n\r", ch );
         return;
      }
   }

   bug( "Do_disconnect: *** desc not found ***.", 0 );
   send_to_char( "Descriptor not found!\n\r", ch );
   return;
}

/*
 * Force a level one player to quit.             Gorog
 */
void do_fquit( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Force whom to quit?\n\r", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim->top_level != 1 )
   {
      send_to_char( "They are not level one!\n\r", ch );
      return;
   }

   send_to_char( "The MUD administrators force you to quit\n\r", victim );
   do_quit( victim, "" );
   send_to_char( "Ok.\n\r", ch );
   return;
}


void do_forceclose( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int desc;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: forceclose <descriptor#>\n\r", ch );
      return;
   }
   desc = atoi( arg );

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->descriptor == desc )
      {
         if( d->character && get_trust( d->character ) >= get_trust( ch ) )
         {
            send_to_char( "They might not like that...\n\r", ch );
            return;
         }
         close_socket( d, FALSE );
         send_to_char( "Ok.\n\r", ch );
         return;
      }
   }

   send_to_char( "Not found!\n\r", ch );
   return;
}



void do_pardon( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: pardon <character> <planet>.\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   send_to_char
      ( "Syntax: pardon <character> <planet>.... But it doesn't work .... Tell Durga to hurry up and finish it :p\n\r", ch );
   return;
}


void echo_to_all( short AT_COLOR, char *argument, short tar )
{
   DESCRIPTOR_DATA *d;

   if( !argument || argument[0] == '\0' )
      return;

   for( d = first_descriptor; d; d = d->next )
   {
      /*
       * Added showing echoes to players who are editing, so they won't
       * miss out on important info like upcoming reboots. --Narn 
       */
      if( d->connected == CON_PLAYING || d->connected == CON_EDITING )
      {
         /*
          * This one is kinda useless except for switched.. 
          */
         if( tar == ECHOTAR_PC && IS_NPC( d->character ) )
            continue;
         else if( tar == ECHOTAR_IMM && !IS_IMMORTAL( d->character ) )
            continue;
         set_char_color( AT_COLOR, d->character );
         send_to_char( argument, d->character );
         send_to_char( "\n\r", d->character );
      }
   }
   return;
}

void do_echo( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short color;
   int target;
   char *parg;

   if( IS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You are noemoted and can not echo.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Echo what?\n\r", ch );
      return;
   }

   if( ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );
   parg = argument;
   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "PC" ) || !str_cmp( arg, "player" ) )
      target = ECHOTAR_PC;
   else if( !str_cmp( arg, "imm" ) )
      target = ECHOTAR_IMM;
   else
   {
      target = ECHOTAR_ALL;
      argument = parg;
   }
   if( !color && ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );
   if( !color )
      color = AT_IMMORT;
   one_argument( argument, arg );
   if( !str_cmp( arg, "Merth" ) || !str_cmp( arg, "Durga" ) )
   {
      ch_printf( ch, "I don't think %s would like that!\n\r", arg );
      return;
   }
   echo_to_all( color, argument, target );
}

void echo_to_room( short AT_COLOR, ROOM_INDEX_DATA * room, char *argument )
{
   CHAR_DATA *vic;

   if( room == NULL )
      return;


   for( vic = room->first_person; vic; vic = vic->next_in_room )
   {
      set_char_color( AT_COLOR, vic );
      send_to_char( argument, vic );
      send_to_char( "\n\r", vic );
   }
}

void do_recho( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short color;

   if( IS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You are noemoted and can not recho.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Recho what?\n\r", ch );
      return;
   }

   one_argument( argument, arg );
   if( !str_cmp( arg, "Thoric" )
       || !str_cmp( arg, "Dominus" )
       || !str_cmp( arg, "Circe" )
       || !str_cmp( arg, "Haus" )
       || !str_cmp( arg, "Narn" ) || !str_cmp( arg, "Scryn" ) || !str_cmp( arg, "Blodkai" ) || !str_cmp( arg, "Damian" ) )
   {
      ch_printf( ch, "I don't think %s would like that!\n\r", arg );
      return;
   }
   if( ( color = get_color( argument ) ) )
   {
      argument = one_argument( argument, arg );
      echo_to_room( color, ch->in_room, argument );
   }
   else
      echo_to_room( AT_IMMORT, ch->in_room, argument );
}


ROOM_INDEX_DATA *find_location( CHAR_DATA * ch, char *arg )
{
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   if( is_number( arg ) )
      return get_room_index( atoi( arg ) );

   if( ( victim = get_char_world( ch, arg ) ) != NULL )
      return victim->in_room;

   if( ( obj = get_obj_world( ch, arg ) ) != NULL )
      return obj->in_room;

   return NULL;
}

/* This function shared by do_transfer and do_mptransfer
 *
 * Immortals bypass most restrictions on where to transfer victims.
 * NPCs cannot transfer victims who are:
 * 1. Not authorized yet.
 * 2. Outside of the level range for the target room's area.
 * 3. Being sent to private rooms.
 */
void transfer_char( CHAR_DATA *ch, CHAR_DATA *victim, ROOM_INDEX_DATA *location )
{
   if( !victim->in_room )
   {
      bug( "%s: victim in NULL room: %s", __FUNCTION__, victim->name );
      return;
   }

   if( IS_NPC(ch) && room_is_private( victim, location ) )
   {
      progbug( "Mptransfer - Private room", ch );
      return;
   }

   if( !can_see( ch, victim ) )
      return;

   if( IS_NPC(ch) && NOT_AUTHED( victim ) && location->area != victim->in_room->area )
   {
      char buf[MAX_STRING_LENGTH];

      snprintf( buf, MAX_STRING_LENGTH, "Mptransfer - unauthed char (%s)", victim->name );
      progbug( buf, ch );
      return;
   }

   /* If victim not in area's level range, do not transfer */
   if( IS_NPC(ch) && !in_hard_range( victim, location->area ) && !IS_SET( location->room_flags, ROOM_PROTOTYPE ) )
      return;

   if( victim->fighting )
      stop_fighting( victim, TRUE );

   if( !IS_NPC(ch) )
   {
      act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
      victim->retran = victim->in_room->vnum;
   }
   char_from_room( victim );
   char_to_room( victim, location );
   if( !IS_NPC(ch) )
   {
      act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
      if( ch != victim )
         act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
      do_look( victim, "auto" );
      if( !IS_IMMORTAL( victim ) && !IS_NPC( victim ) && !in_hard_range( victim, location->area ) )
         act( AT_DANGER, "Warning:  this player's level is not within the area's level range.", ch, NULL, NULL, TO_CHAR );
   }
}

void do_transfer( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Transfer whom (and where)?\n\r", ch );
      return;
   }

   if( arg2[0] != '\0' )
   {
      if( !( location = find_location( ch, arg2 ) ) )
      {
         send_to_char( "That location does not exist.\n\r", ch );
         return;
      }
   }
   else
      location = ch->in_room;

   if( !str_cmp( arg1, "all" ) && get_trust( ch ) >= LEVEL_GREATER )
   {
      for( d = first_descriptor; d; d = d->next )
      {
         if( d->connected == CON_PLAYING && d->character && d->character != ch && d->character->in_room )
            transfer_char( ch, d->character, location );
      }
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   transfer_char( ch, victim, location );
}

void do_retran( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Retransfer whom?\n\r", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   sprintf( buf, "'%s' %d", victim->name, victim->retran );
   do_transfer( ch, buf );
   return;
}

void do_regoto( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   sprintf( buf, "%d", ch->regoto );
   do_goto( ch, buf );
   return;
}

void do_at( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *wch;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "At where what?\n\r", ch );
      return;
   }

   if( ( location = find_location( ch, arg ) ) == NULL )
   {
      send_to_char( "No such location.\n\r", ch );
      return;
   }

   if( room_is_private( ch, location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\n\r", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\n\r", ch );
      }

   }

   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument );

   /*
    * See if 'ch' still exists before continuing!
    * Handles 'at XXXX quit' case.
    */
   for( wch = first_char; wch; wch = wch->next )
   {
      if( wch == ch )
      {
         char_from_room( ch );
         char_to_room( ch, original );
         break;
      }
   }

   return;
}

void do_rat( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   int Start, End, vnum;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
      return;
   }

   Start = atoi( arg1 );
   End = atoi( arg2 );

   if( Start < 1 || End < Start || Start > End || Start == End || End > 32767 )
   {
      send_to_char( "Invalid range.\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "quit" ) )
   {
      send_to_char( "I don't think so!\n\r", ch );
      return;
   }

   original = ch->in_room;
   for( vnum = Start; vnum <= End; vnum++ )
   {
      if( ( location = get_room_index( vnum ) ) == NULL )
         continue;
      char_from_room( ch );
      char_to_room( ch, location );
      interpret( ch, argument );
   }

   char_from_room( ch );
   char_to_room( ch, original );
   send_to_char( "Done.\n\r", ch );
   return;
}


void do_rstat( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   OBJ_DATA *obj;
   CHAR_DATA *rch;
   EXIT_DATA *pexit;
   int cnt;
   static char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

   one_argument( argument, arg );

   if( get_trust( ch ) < LEVEL_IMMORTAL )
   {
      AREA_DATA *pArea;

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to goto.\n\r", ch );
         return;
      }

      if( ch->in_room->vnum < pArea->low_r_vnum || ch->in_room->vnum > pArea->hi_r_vnum )
      {
         send_to_char( "You can only rstat within your assigned range.\n\r", ch );
         return;
      }

   }


   if( !str_cmp( arg, "exits" ) )
   {
      location = ch->in_room;

      ch_printf( ch, "&GExits for room '&W%s.' &Gvnum &W%d\n\r", location->name, location->vnum );

      for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
         ch_printf( ch,
                    "%2d) %2s to %-5d.  Key: %d  Flags: %d  Keywords: '%s'.\n\rDescription: %sExit links back to vnum: %d  Exit's RoomVnum: %d  Distance: %d\n\r",
                    ++cnt,
                    dir_text[pexit->vdir],
                    pexit->to_room ? pexit->to_room->vnum : 0,
                    pexit->key,
                    pexit->exit_info,
                    pexit->keyword,
                    pexit->description[0] != '\0'
                    ? pexit->description : "(none).\n\r",
                    pexit->rexit ? pexit->rexit->vnum : 0, pexit->rvnum, pexit->distance );
      return;
   }
   location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
   if( !location )
   {
      send_to_char( "No such location.\n\r", ch );
      return;
   }

   if( ch->in_room != location && room_is_private( ch, location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\n\r", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\n\r", ch );
      }

   }

   ch_printf( ch, "&GName: &W%s.\n\r&GArea: &W%s  &GFilename: &W%s.\n\r",
              location->name,
              location->area ? location->area->name : "None????", location->area ? location->area->filename : "None????" );

   ch_printf( ch,
              "&GVnum: &W%d.  &GSector: &W%d.  &GLight: &W%d.  &GTeleDelay: &W%d.  &GTeleVnum: &W%d  &GTunnel: &W%d.\n\r",
              location->vnum,
              location->sector_type, location->light, location->tele_delay, location->tele_vnum, location->tunnel );

   ch_printf( ch, "&GRoom flags: &W%s\n\r", flag_string( location->room_flags, r_flags ) );
   ch_printf( ch, "&GRoom flags2: &W%s\n\r", flag_string( location->room_flags2, r_flags2 ) );
   ch_printf( ch, "&GDescription:\n\r&W%s", location->description );

   if( location->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char( "&GExtra description keywords: &W'", ch );
      for( ed = location->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'.\n\r", ch );
   }

   send_to_char( "&GCharacters:&W", ch );
   for( rch = location->first_person; rch; rch = rch->next_in_room )
   {
      if( can_see( ch, rch ) )
      {
         send_to_char( " ", ch );
         one_argument( rch->name, buf );
         send_to_char( buf, ch );
      }
   }

   send_to_char( ".\n\r&GObjects:   &W", ch );
   for( obj = location->first_content; obj; obj = obj->next_content )
   {
      send_to_char( " ", ch );
      one_argument( obj->name, buf );
      send_to_char( buf, ch );
   }
   send_to_char( ".\n\r", ch );

   if( location->first_exit )
      send_to_char( "&G------------------- EXITS -------------------&W\n\r", ch );
   for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
      ch_printf( ch,
                 "&G%2d) &W%-2s &Gto &W%-5d.  &GKey: &W%d  &GFlags: &W%d  &GKeywords: &W%s.\n\r",
                 ++cnt,
                 dir_text[pexit->vdir],
                 pexit->to_room ? pexit->to_room->vnum : 0,
                 pexit->key, pexit->exit_info, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
   return;
}



void do_ostat( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   OBJ_DATA *obj;
   char *pdesc;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Ostat what?\n\r", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strcpy( arg, argument );

   if( ( obj = get_obj_world( ch, arg ) ) == NULL )
   {
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
      return;
   }

   ch_printf( ch, "&w&GName: &W%s\n\r", obj->name );

   pdesc = get_extra_descr( arg, obj->first_extradesc );
   if( !pdesc )
      pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc );
   if( !pdesc )
      pdesc = get_extra_descr( obj->name, obj->first_extradesc );
   if( !pdesc )
      pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
   if( pdesc )
      send_to_char( pdesc, ch );


   ch_printf( ch, "&GVnum: &W%d  &GType: &W%s  &GCount: &W%d  &GGcount: &W%d\n\r",
              obj->pIndexData->vnum, item_type_name( obj ), obj->pIndexData->count, obj->count );

   ch_printf( ch, "&GSerial#: &W%d  &GTopIdxSerial#: &W%d  &GTopSerial#: &W%d\n\r",
              obj->serial, obj->pIndexData->serial, cur_obj_serial );

   ch_printf( ch, "&GShort description: &W%s\n\r&GLong description: &W%s\n\r", obj->short_descr, obj->description );

   if( obj->action_desc[0] != '\0' )
      ch_printf( ch, "&GAction description: &W%s\n\r", obj->action_desc );

   ch_printf( ch, "&GWear flags : &W%s\n\r", flag_string( obj->wear_flags, w_flags ) );
   ch_printf( ch, "&GExtra flags: &W%s\n\r", flag_string( obj->extra_flags, o_flags ) );

   ch_printf( ch, "&GNumber: &W%d&G/&W%d  &GWeight: &W%d&G/&W%d  &GLayers: &W%d\n\r",
              1, get_obj_number( obj ), obj->weight, get_obj_weight( obj ), obj->pIndexData->layers );

   ch_printf( ch, "&GCost: &W%d  &GRent: &W%d  &GTimer: &W%d  &GLevel: &W%d\n\r",
              obj->cost, obj->pIndexData->rent, obj->timer, obj->level );

   ch_printf( ch,
              "&GIn room: &W%d  &GIn object: &W%s  &GCarried by: &W%s  &GWear_loc: &W%d\n\r",
              obj->in_room == NULL ? 0 : obj->in_room->vnum,
              obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
              obj->carried_by == NULL ? "(none)" : obj->carried_by->name, obj->wear_loc );

   ch_printf( ch, "&GIndex Values : &W%d %d %d %d %d %d\n\r",
              obj->pIndexData->value[0], obj->pIndexData->value[1],
              obj->pIndexData->value[2], obj->pIndexData->value[3], obj->pIndexData->value[4], obj->pIndexData->value[5] );
   ch_printf( ch, "&GObject Values: &W%d %d %d %d %d %d\n\r",
              obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

   ostat_plus( ch, obj );
                        /*-Druid*/
   set_char_color( AT_CYAN, ch ); /*-Druid*/

   if( obj->pIndexData->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char( "&GPrimary description keywords:&W   '", ch );
      for( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'.\n\r", ch );
   }
   if( obj->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char( "&GSecondary description keywords:&W '", ch );
      for( ed = obj->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'.\n\r", ch );
   }

   for( paf = obj->first_affect; paf; paf = paf->next )
      ch_printf( ch, "&w&GAffects &W%s&G by &W%d&G. (extra)\n\r", affect_loc_name( paf->location ), paf->modifier );

   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      ch_printf( ch, "&w&GAffects &W%s&G by &W%d&G.\n\r", affect_loc_name( paf->location ), paf->modifier );

   if( ( obj->item_type == ITEM_CONTAINER ) && ( obj->first_content ) )
   {
      send_to_char( "&w&GContents:&W\n\r", ch );
      show_list_to_char( obj->first_content, ch, TRUE, FALSE );
   }

   return;
}

/* New mstat by tawnos. Holy shit this took a while :P*/
void do_mstat( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char langbuf[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   CHAR_DATA *victim;
   SKILLTYPE *skill;
   int x, ability;

   set_char_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mstat whom?\n\r", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strcpy( arg, argument );

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( ch ) )
   {
      send_to_char( "Why would a mob need to mstat something?\n\r", ch );
      return;
   }

   if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "Their godly glow prevents you from getting a good look.\n\r", ch );
      return;
   }

   ch_printf( ch, "&W&GCharacter Data for %s\n\r", IS_NPC( victim ) ? victim->short_descr : victim->name );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   ch_printf( ch, "&W&z| &GName&W: %-12.12s     &GLastname&W: %-12s      &GClan&W: %-19s &z|\n\r", victim->name,
              ( IS_NPC( victim ) || !victim->pcdata->last_name ) ? "(none)" : victim->pcdata->last_name, ( IS_NPC( victim )
                                                                                                           || !victim->
                                                                                                           pcdata->
                                                                                                           clan ) ? "(none)"
              : victim->pcdata->clan->name );
   ch_printf( ch,
              "&W&z|  &GStr&W: %-2d  &GInt&W: %-2d  &GWis&W: %-2d  &GDex&W: %-2d  &GCon&W: %-2d  &GCha&W: %-2d  &GLck&W: %-2d  &GFrc&W: %-2d      &z|\n\r",
              get_curr_str( victim ), get_curr_int( victim ), get_curr_wis( victim ), get_curr_dex( victim ),
              get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ), get_curr_frc( victim ) );
   ch_printf( ch, "&W&z|  &GSex&W: %-6s               &GVnum&W: %-6d          &GInRoom&W: %-6d              &z|\n\r",
              victim->sex == SEX_MALE ? "Male" : victim->sex == SEX_FEMALE ? "Female" : "Neuter",
              IS_NPC( victim ) ? victim->pIndexData->vnum : 0, victim->in_room == NULL ? 0 : victim->in_room->vnum );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   if( !IS_NPC( victim ) && victim->desc )
   {
      ch_printf( ch, "&W&z|       &GUser&W: %-44s &GTrust&W: %-2d           &z|\n\r", victim->desc->host, victim->trust );
      ch_printf( ch, "&W&z| &GDescriptor&W: %-3d                                       &GAuthedBy&W: %-12s &z|\n\r",
                 victim->desc->descriptor, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)" );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   ch_printf( ch, "&W&z|       &GGold&W: %-10d          &GRace&W: %-17s  &GHit&W: %5d/%-5d    &z|\n\r", victim->gold,
              npc_race[victim->race], victim->hit, victim->max_hit );
   ch_printf( ch, "&W&z|       &GBank&W: %-10d            &GAC&W: %-5d             &GMana&W: %5d/%-5d    &z|\n\r",
              ( IS_NPC( victim )
                || !victim->pcdata->bank ) ? 0 : victim->pcdata->bank, GET_AC( victim ), victim->mana, victim->max_mana );
   ch_printf( ch, "&W&z|   &GTopLevel&W: %-2d                 &GAlign&W: %-5d             &GMove&W: %5d/%-5d    &z|\n\r",
              victim->top_level, victim->alignment, victim->move, victim->max_move );
   if( !IS_NPC( victim ) )
      ch_printf( ch, "&W&z|       &GSalary&W: %-8d        &GThirst&W: %-4d             &GFull&W: %-4d            &z|\n\r",
                 victim->pcdata->salary, victim->pcdata->condition[COND_THIRST], victim->pcdata->condition[COND_FULL] );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   if( !IS_NPC( victim ) )
   {
      for( ability = 0; ability < MAX_ABILITY; ability++ )
         ch_printf( ch, "&W&z|   &G%-15s Level&W: %-3d    &GMax&W: %-3d    &GExp&W: %-8d     &GNext&W: %-8d &z|\n\r",
                    ability_name[ability], victim->skill_level[ability], max_level( victim, ability ),
                    victim->experience[ability], exp_level( victim->skill_level[ability] + 1 ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   ch_printf( ch, "&W&z|    &GHitroll&W: %-3d           &GMentalstate&W: %-4d         &GFighting&W: %-12s   &z|\n\r",
              GET_HITROLL( victim ), victim->mental_state, victim->fighting ? victim->fighting->who->name : "(none)" );
   ch_printf( ch, "&W&z|    &GDamroll&W: %-3d        &GEmotionalstate&W: %-4d           &GMaster&W: %-12s   &z|\n\r",
              GET_DAMROLL( victim ), victim->emotional_state, victim->master ? victim->master->name : "(none)" );
   ch_printf( ch, "&W&z|   &GPosition&W: %-1d                   &GWimpy&W: %-4d           &GLeader&W: %-12s   &z|\n\r",
              victim->position, victim->wimpy, victim->leader ? victim->leader->name : "(none)" );
   ch_printf( ch, "&W&z| &GAffectedBy&W: %-64s &z|\n\r", affect_bit_name( victim->affected_by ) );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   if( !IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z|      &GYears&W: %-3d                     &GSeconds Played&W: %-10d               &z|\n\r",
                 get_age( victim ), ( int )victim->played );
      ch_printf( ch, "&W&z|        &GAct&W: %-14d                   &GTimer&W: %-6d                   &z|\n\r", victim->act,
                 victim->timer );
      ch_printf( ch, "&W&z| &GCarry Info: Items&W: %4d/%-4d                &GWeight&W: %7.7d/%-7.7d          &z|\n\r",
                 victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   if( !IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z| &GPlrFlags&W: %-66.66s &z|\n\r", flag_string( victim->act, plr_flags ) );
      ch_printf( ch, "&W&z| &GPcflags&W: %-67s &z|\n\r", flag_string( victim->pcdata->flags, pc_flags ) );
      ch_printf( ch, "&W&z| &GWanted Flags&W: %-62s &z|\n\r", flag_string( victim->pcdata->wanted_flags, planet_flags ) );
      ch_printf( ch, "&W&z| &GBestowments&W: %-63s &z|\n\r",
                 victim->pcdata->bestowments != NULL ? victim->pcdata->bestowments : "None" );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   if( IS_NPC( victim ) )
   {
      if( victim->pIndexData->count != 1 || victim->pIndexData->killed != 0 )
         ch_printf( ch,
                    "&W&z|      &GCount&W: %-3d                             &GKilled&W: %-3d                      &z|\n\r",
                    victim->pIndexData->count, victim->pIndexData->killed );
      ch_printf( ch,
                 "&W&z|   &GHit Dice&W: %-2dd%-2d+%-4d                    &GDam Dice&W: %-2dd%-2d+%-4d               &z|\n\r",
                 victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice, victim->pIndexData->hitplus,
                 victim->pIndexData->damnodice, victim->pIndexData->damsizedice, victim->pIndexData->damplus );
      ch_printf( ch,
                 "&W&z| &GSaving throws&W: %-1d %-1d %-1d %-1d %-1d                                                     &z|\n\r",
                 victim->saving_poison_death, victim->saving_wand, victim->saving_para_petri, victim->saving_breath,
                 victim->saving_spell_staff );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z|     &GSpeaks&W: %-10d  &GSpeaking&W: %-10d                                 &z|\n\r",
                 victim->speaks, victim->speaking );
      sprintf( langbuf, " " );
      for( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
      {
         if( knows_language( victim, lang_array[x], victim ) || ( IS_NPC( victim ) && victim->speaks == 0 ) )
         {
            strcat( langbuf, lang_names[x] );
            strcat( langbuf, " " );
         }
      }
      ch_printf( ch, "&W&z|  &GLanguages&W: %-64s &z|\n\r", langbuf );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   if( victim->resistant || victim->immune || victim->susceptible )
   {
      ch_printf( ch, "&W&z|   &GRes&W: %-69s &z|\n\r", flag_string( victim->resistant, ris_flags ) );
      ch_printf( ch, "&W&z|   &GImm&W: %-69s &z|\n\r", flag_string( victim->immune, ris_flags ) );
      ch_printf( ch, "&W&z|   &GSus&W: %-69s &z|\n\r", flag_string( victim->susceptible, ris_flags ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z|  &GNumAttacks&W: %-2d                                                              &z|\n\r",
                 victim->numattacks );
      ch_printf( ch, "&W&z|     &GAttacks&W: %-63s &z|\n\r", flag_string( victim->attacks, attack_flags ) );
      ch_printf( ch, "&W&z|     &GDefense&W: %-63s &z|\n\r", flag_string( victim->defenses, defense_flags ) );
      ch_printf( ch, "&W&z|       &GParts&W: %-63s &z|\n\r", flag_string( victim->xflags, part_flags ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\n\r" );
   }
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "&GAct flags&W: %s\n\r", flag_string( victim->act, act_flags ) );
      ch_printf( ch, "&GVIP flags&W: %s\n\r", flag_string( victim->vip_flags, planet_flags ) );
      ch_printf( ch, "    &GNames&W: %s\n\r", victim->name );
      ch_printf( ch, "&GShortDesc&W: %s\n\r", victim->short_descr );
      ch_printf( ch, " &GLongDesc&W: %s\n\r", victim->long_descr );
   }
   for( paf = victim->first_affect; paf; paf = paf->next )
      if( ( skill = get_skilltype( paf->type ) ) != NULL )
         ch_printf( ch, "&W%s: &G'%s'&W modifies &G%s&W by &G%d&W for %d rounds with bits %s.\n\r", skill_tname[skill->type],
                    skill->name, affect_loc_name( paf->location ), paf->modifier, paf->duration,
                    affect_bit_name( paf->bitvector ) );

   if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
      ch_printf( ch, "&RHelled until %24.24s by %s.&W\n\r",
                 ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );

   if( IS_NPC( victim ) && ( victim->spec_fun || victim->spec_2 ) )
      ch_printf( ch, "\n\r&GMobile has spec fun: %s %s&W\n\r",
                 lookup_spec( victim->spec_fun ), victim->spec_2 ? lookup_spec( victim->spec_2 ) : "" );

/*
	if (!IS_NPC(victim))
    {
      ch_printf( ch, "Stage: %d\n\r     : %d\n\r     : %d\n\r", victim->pcdata->stage[0], victim->pcdata->stage[1], victim->pcdata->stage[2]);
    }
    
    if (ch->pcdata->comm_channel)
    {
    	if ( !IS_NPC( victim ) )
    	ch_printf(ch, "Comm_channel: %d", victim->pcdata->comm_channel);
    }
*/

   return;
}

void do_oldmstat( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   CHAR_DATA *victim;
   SKILLTYPE *skill;
   int x;

   set_char_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mstat whom?\n\r", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strcpy( arg, argument );
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "Their godly glow prevents you from getting a good look.\n\r", ch );
      return;
   }
   ch_printf( ch, "Name: %s    Lastname: %s    Organization: %s\n\r",
              victim->name, ( IS_NPC( victim ) || !victim->pcdata->last_name ) ? "(none)" : victim->pcdata->last_name,
              ( IS_NPC( victim ) || !victim->pcdata->clan ) ? "(none)" : victim->pcdata->clan->name );
   if( get_trust( ch ) >= LEVEL_GOD && !IS_NPC( victim ) && victim->desc )
      ch_printf( ch, "Host: %s   Descriptor: %d   Trust: %d   AuthedBy: %s\n\r",
                 victim->desc->host, victim->desc->descriptor,
                 victim->trust, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)" );
   if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
      ch_printf( ch, "Helled until %24.24s by %s.\n\r", ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );

   ch_printf( ch, "Vnum: %d   Sex: %s   Room: %d   Count: %d  Killed: %d\n\r",
              IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
              victim->sex == SEX_MALE ? "male" :
              victim->sex == SEX_FEMALE ? "female" : "neutral",
              !victim->in_room ? 0 : victim->in_room->vnum,
              IS_NPC( victim ) ? victim->pIndexData->count : 1,
              IS_NPC( victim ) ? victim->pIndexData->killed : victim->pcdata->mdeaths + victim->pcdata->pdeaths );
   ch_printf( ch, "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d  Frc: %d\n\r",
              get_curr_str( victim ),
              get_curr_int( victim ),
              get_curr_wis( victim ),
              get_curr_dex( victim ),
              get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ), get_curr_frc( victim ) );
   ch_printf( ch, "Hps: %d/%d  Force: %d/%d   Move: %d/%d\n\r",
              victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move );
   if( !IS_NPC( victim ) )
   {
      int ability;

      for( ability = 0; ability < MAX_ABILITY; ability++ )
         ch_printf( ch, "%-15s   Level: %-3d   Max: %-3d   Exp: %-10ld   Next: %-10ld\n\r",
                    ability_name[ability], victim->skill_level[ability], max_level( victim, ability ),
                    victim->experience[ability], exp_level( victim->skill_level[ability] + 1 ) );
   }
   ch_printf( ch,
              "Top Level: %d     Race: %d  Align: %d  AC: %d  Gold: %d\n\r",
              victim->top_level, victim->race, victim->alignment, GET_AC( victim ), victim->gold );
   if( victim->race < MAX_NPC_RACE && victim->race >= 0 )
      ch_printf( ch, "Race: %s\n\r", npc_race[victim->race] );
   ch_printf( ch, "Hitroll: %d   Damroll: %d   Position: %d   Wimpy: %d \n\r",
              GET_HITROLL( victim ), GET_DAMROLL( victim ), victim->position, victim->wimpy );
   ch_printf( ch, "Fighting: %s    Master: %s    Leader: %s\n\r",
              victim->fighting ? victim->fighting->who->name : "(none)",
              victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)" );
   if( !IS_NPC( victim ) )
      ch_printf( ch,
                 "Thirst: %d   Full: %d   Drunk: %d     Glory: %d/%d\n\r",
                 victim->pcdata->condition[COND_THIRST],
                 victim->pcdata->condition[COND_FULL],
                 victim->pcdata->condition[COND_DRUNK], victim->pcdata->quest_curr, victim->pcdata->quest_accum );
   else
      ch_printf( ch, "Hit dice: %dd%d+%d.  Damage dice: %dd%d+%d.\n\r",
                 victim->pIndexData->hitnodice,
                 victim->pIndexData->hitsizedice,
                 victim->pIndexData->hitplus,
                 victim->pIndexData->damnodice, victim->pIndexData->damsizedice, victim->pIndexData->damplus );
   ch_printf( ch, "MentalState: %d   EmotionalState: %d\n\r", victim->mental_state, victim->emotional_state );
   ch_printf( ch, "Saving throws: %d %d %d %d %d.\n\r",
              victim->saving_poison_death,
              victim->saving_wand, victim->saving_para_petri, victim->saving_breath, victim->saving_spell_staff );
   ch_printf( ch, "Carry figures: items (%d/%d)  weight (%d/%d)   Numattacks: %d\n\r",
              victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ), victim->numattacks );
   ch_printf( ch, "Years: %d   Seconds Played: %d   Timer: %d   Act: %d\n\r",
              get_age( victim ), ( int )victim->played, victim->timer, victim->act );
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "Act flags: %s\n\r", flag_string( victim->act, act_flags ) );
      ch_printf( ch, "VIP flags: %s\n\r", flag_string( victim->vip_flags, planet_flags ) );
   }
   else
   {
      ch_printf( ch, "Player flags: %s\n\r", flag_string( victim->act, plr_flags ) );
      ch_printf( ch, "Pcflags: %s\n\r", flag_string( victim->pcdata->flags, pc_flags ) );
      ch_printf( ch, "Wanted flags: %s\n\r", flag_string( victim->pcdata->wanted_flags, planet_flags ) );
   }
   ch_printf( ch, "Affected by: %s\n\r", affect_bit_name( victim->affected_by ) );
   ch_printf( ch, "Speaks: %d   Speaking: %d\n\r", victim->speaks, victim->speaking );
   send_to_char( "Languages: ", ch );
   for( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
      if( knows_language( victim, lang_array[x], victim ) || ( IS_NPC( victim ) && victim->speaks == 0 ) )
      {
         if( IS_SET( lang_array[x], victim->speaking ) || ( IS_NPC( victim ) && !victim->speaking ) )
            set_char_color( AT_RED, ch );
         send_to_char( lang_names[x], ch );
         send_to_char( " ", ch );
         set_char_color( AT_PLAIN, ch );
      }
      else if( IS_SET( lang_array[x], victim->speaking ) || ( IS_NPC( victim ) && !victim->speaking ) )
      {
         set_char_color( AT_PINK, ch );
         send_to_char( lang_names[x], ch );
         send_to_char( " ", ch );
         set_char_color( AT_PLAIN, ch );
      }
   send_to_char( "\n\r", ch );
   if( victim->pcdata && victim->pcdata->bestowments && victim->pcdata->bestowments[0] != '\0' )
      ch_printf( ch, "Bestowments: %s\n\r", victim->pcdata->bestowments );
   ch_printf( ch, "Short description: %s\n\rLong  description: %s",
              victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
   if( IS_NPC( victim ) && ( victim->spec_fun || victim->spec_2 ) )
      ch_printf( ch, "Mobile has spec fun: %s %s\n\r",
                 lookup_spec( victim->spec_fun ), victim->spec_2 ? lookup_spec( victim->spec_2 ) : "" );
   ch_printf( ch, "Body Parts : %s\n\r", flag_string( victim->xflags, part_flags ) );
   ch_printf( ch, "Resistant  : %s\n\r", flag_string( victim->resistant, ris_flags ) );
   ch_printf( ch, "Immune     : %s\n\r", flag_string( victim->immune, ris_flags ) );
   ch_printf( ch, "Susceptible: %s\n\r", flag_string( victim->susceptible, ris_flags ) );
   ch_printf( ch, "Attacks    : %s\n\r", flag_string( victim->attacks, attack_flags ) );
   ch_printf( ch, "Defenses   : %s\n\r", flag_string( victim->defenses, defense_flags ) );
   for( paf = victim->first_affect; paf; paf = paf->next )
      if( ( skill = get_skilltype( paf->type ) ) != NULL )
         ch_printf( ch,
                    "%s: '%s' modifies %s by %d for %d rounds with bits %s.\n\r",
                    skill_tname[skill->type],
                    skill->name,
                    affect_loc_name( paf->location ), paf->modifier, paf->duration, affect_bit_name( paf->bitvector ) );
   return;
}



void do_mfind( CHAR_DATA * ch, char *argument )
{
/*  extern int top_mob_index; */
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
/*  int vnum; */
   int hash;
   int nMatch;
   bool fAll;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mfind whom?\n\r", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );
   nMatch = 0;
   set_pager_color( AT_PLAIN, ch );

   /*
    * Yeah, so iterating over all vnum's takes 10,000 loops.
    * Get_mob_index is fast, and I don't feel like threading another link.
    * Do you?
    * -- Furey
    */
/*  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		nMatch++;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_mob_index()... which loops itself, an average of 1-2 times...
    * So theoretically, the above routine may loop well over 40,000 times,
    * and my routine bellow will loop for as many index_mobiles are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
         if( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\n\r", pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

   return;
}



void do_ofind( CHAR_DATA * ch, char *argument )
{
/*  extern int top_obj_index; */
   char arg[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
/*  int vnum; */
   int hash;
   int nMatch;
   bool fAll;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Ofind what?\n\r", ch );
      return;
   }

   set_pager_color( AT_PLAIN, ch );
   fAll = !str_cmp( arg, "all" );
   nMatch = 0;
/*  nLoop	= 0; */

   /*
    * Yeah, so iterating over all vnum's takes 10,000 loops.
    * Get_obj_index is fast, and I don't feel like threading another link.
    * Do you?
    * -- Furey
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
    nLoop++;
    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
    {
    if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
    {
    nMatch++;
    sprintf( buf, "[%5d] %s\n\r",
    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
    send_to_char( buf, ch );
    }
    }
    }
    */

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_obj_index()... which loops itself, an average of 2-3 times...
    * So theoretically, the above routine may loop well over 50,000 times,
    * and my routine bellow will loop for as many index_objects are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
         if( fAll || nifty_is_name( arg, pObjIndex->name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\n\r", pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

   return;
}



void do_mwhere( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   bool found;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mwhere whom?\n\r", ch );
      return;
   }

   set_pager_color( AT_PLAIN, ch );
   found = FALSE;
   for( victim = first_char; victim; victim = victim->next )
   {
      if( IS_NPC( victim ) && victim->in_room && nifty_is_name( arg, victim->name ) )
      {
         found = TRUE;
         pager_printf( ch, "[%5d] %-28s [%5d] %s\n\r",
                       victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name );
      }
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

   return;
}


void do_bodybag( CHAR_DATA * ch, char *argument )
{
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   bool found;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Bodybag whom?\n\r", ch );
      return;
   }

   /*
    * make sure the buf3 is clear? 
    */
   sprintf( buf3, " " );
   /*
    * check to see if vict is playing? 
    */
   sprintf( buf2, "the corpse of %s", arg );
   found = FALSE;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( obj->in_room && !str_cmp( buf2, obj->short_descr ) && ( obj->pIndexData->vnum == 11 ) )
      {
         found = TRUE;
         ch_printf( ch, "Bagging body: [%5d] %-28s [%5d] %s\n\r",
                    obj->pIndexData->vnum, obj->short_descr, obj->in_room->vnum, obj->in_room->name );
         obj_from_room( obj );
         obj = obj_to_char( obj, ch );
         obj->timer = -1;
         save_char_obj( ch );
      }
   }

   if( !found )
      ch_printf( ch, " You couldn't find any %s\n\r", buf2 );
   return;
}


/* New owhere by Altrag, 03/14/96 */
void do_owhere( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   bool found;
   int icnt = 0;

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Owhere what?\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg1 );

   set_pager_color( AT_PLAIN, ch );
   if( arg1[0] != '\0' && !str_prefix( arg1, "nesthunt" ) )
   {
      if( !( obj = get_obj_world( ch, arg ) ) )
      {
         send_to_char( "Nesthunt for what object?\n\r", ch );
         return;
      }
      for( ; obj->in_obj; obj = obj->in_obj )
      {
         pager_printf( ch, "[%5d] %-28s in object [%5d] %s\n\r",
                       obj->pIndexData->vnum, obj_short( obj ), obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr );
         ++icnt;
      }
      sprintf( buf, "[%5d] %-28s in ", obj->pIndexData->vnum, obj_short( obj ) );
      if( obj->carried_by )
         sprintf( buf + strlen( buf ), "invent [%5d] %s\n\r",
                  ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
      else if( obj->in_room )
         sprintf( buf + strlen( buf ), "room   [%5d] %s\n\r", obj->in_room->vnum, obj->in_room->name );
      else if( obj->in_obj )
      {
         bug( "do_owhere: obj->in_obj after NULL!", 0 );
         strcat( buf, "object??\n\r" );
      }
      else
      {
         bug( "do_owhere: object doesnt have location!", 0 );
         strcat( buf, "nowhere??\n\r" );
      }
      send_to_pager( buf, ch );
      ++icnt;
      pager_printf( ch, "Nested %d levels deep.\n\r", icnt );
      return;
   }

   found = FALSE;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( !nifty_is_name( arg, obj->name ) )
         continue;
      found = TRUE;

      sprintf( buf, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum, obj_short( obj ) );
      if( obj->carried_by )
         sprintf( buf + strlen( buf ), "invent [%5d] %s\n\r",
                  ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
      else if( obj->in_room )
         sprintf( buf + strlen( buf ), "room   [%5d] %s\n\r", obj->in_room->vnum, obj->in_room->name );
      else if( obj->in_obj )
         sprintf( buf + strlen( buf ), "object [%5d] %s\n\r", obj->in_obj->pIndexData->vnum, obj_short( obj->in_obj ) );
      else
      {
         bug( "do_owhere: object doesnt have location!", 0 );
         strcat( buf, "nowhere??\n\r" );
      }
      send_to_pager( buf, ch );
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   else
      pager_printf( ch, "%d matches.\n\r", icnt );

   return;
}


void do_reboo( CHAR_DATA * ch, char *argument )
{
   send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
   return;
}



void do_reboot( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) && str_cmp( argument, "and sort skill table" ) )
   {
      send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\n\r", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );

   sprintf( buf, "Reboot by %s.", ch->name );
   do_echo( ch, buf );

   if( !str_cmp( argument, "and sort skill table" ) )
   {
      sort_skill_table(  );
      save_skill_table( -1 );
   }

   /*
    * Save all characters before booting. 
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );

   mud_down = TRUE;
   return;
}



void do_shutdow( CHAR_DATA * ch, char *argument )
{
   send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
   return;
}



void do_shutdown( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) )
   {
      send_to_char( "Syntax: 'shutdown mud now' or 'shutdown nosave'\n\r", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );

   sprintf( buf, "Shutdown by %s.", ch->name );
   append_file( ch, SHUTDOWN_FILE, buf );
   strcat( buf, "\n\r" );
   do_echo( ch, buf );

   /*
    * Save all characters before booting. 
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );
   mud_down = TRUE;
   return;
}


void do_snoop( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Snoop whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( !victim->desc )
   {
      send_to_char( "No descriptor to snoop.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Cancelling all snoops.\n\r", ch );
      for( d = first_descriptor; d; d = d->next )
         if( d->snoop_by == ch->desc )
            d->snoop_by = NULL;
      return;
   }

   if( victim->desc->snoop_by )
   {
      send_to_char( "Busy already.\n\r", ch );
      return;
   }

   /*
    * Minimum snoop level... a secret mset value
    * makes the snooper think that the victim is already being snooped
    */
   if( get_trust( victim ) >= get_trust( ch ) || ( victim->pcdata && victim->pcdata->min_snoop > get_trust( ch ) ) )
   {
      send_to_char( "Busy already.\n\r", ch );
      return;
   }

   if( ch->desc )
   {
      for( d = ch->desc->snoop_by; d; d = d->snoop_by )
         if( d->character == victim || d->original == victim )
         {
            send_to_char( "No snoop loops.\n\r", ch );
            return;
         }
   }

/*  Snoop notification for higher imms, if desired, uncomment this
    if ( get_trust(victim) > LEVEL_GOD && get_trust(ch) < LEVEL_SUPREME )
      write_to_descriptor( victim->desc->descriptor, "\n\rYou feel like someone is watching your every move...\n\r", 0 );
*/
   victim->desc->snoop_by = ch->desc;
   send_to_char( "Ok.\n\r", ch );
   return;
}



void do_switch( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Switch into whom?\n\r", ch );
      return;
   }

   if( !ch->desc )
      return;

   if( ch->desc->original )
   {
      send_to_char( "You are already switched.\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Ok.\n\r", ch );
      return;
   }

   if( victim->desc )
   {
      send_to_char( "Character in use.\n\r", ch );
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "You cannot switch into a player!\n\r", ch );
      return;
   }

   ch->desc->character = victim;
   ch->desc->original = ch;
   victim->desc = ch->desc;
   ch->desc = NULL;
   ch->switched = victim;
   send_to_char( "Ok.\n\r", victim );
   return;
}



void do_return( CHAR_DATA * ch, char *argument )
{
   if( !ch->desc )
      return;

   if( !ch->desc->original )
   {
      send_to_char( "You aren't switched.\n\r", ch );
      return;
   }

   if( IS_SET( ch->act, ACT_POLYMORPHED ) )
   {
      send_to_char( "Use revert to return from a polymorphed mob.\n\r", ch );
      return;
   }

   send_to_char( "You return to your original body.\n\r", ch );
   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_POSSESS ) )
   {
      affect_strip( ch, gsn_possess );
      REMOVE_BIT( ch->affected_by, AFF_POSSESS );
   }
/*    if ( IS_NPC( ch->desc->character ) )
      REMOVE_BIT( ch->desc->character->affected_by, AFF_POSSESS );*/
   ch->desc->character = ch->desc->original;
   ch->desc->original = NULL;
   ch->desc->character->desc = ch->desc;
   ch->desc->character->switched = NULL;
   ch->desc = NULL;
   return;
}



void do_minvoke( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *victim;
   int vnum;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: minvoke <vnum>.\n\r", ch );
      return;
   }

   if( !is_number( arg ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg, arg2 );

      vnum = -1;
      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
            if( nifty_is_name( arg2, pMobIndex->player_name ) && ++cnt == count )
            {
               vnum = pMobIndex->vnum;
               break;
            }
      if( vnum == -1 )
      {
         send_to_char( "No such mobile exists.\n\r", ch );
         return;
      }
   }
   else
      vnum = atoi( arg );

   if( get_trust( ch ) < LEVEL_DEMI )
   {
      AREA_DATA *pArea;

      if( IS_NPC( ch ) )
      {
         send_to_char( "Huh?\n\r", ch );
         return;
      }

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this mobile.\n\r", ch );
         return;
      }
      if( vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum )
      {
         send_to_char( "That number is not in your allocated range.\n\r", ch );
         return;
      }
   }

   if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "No mobile has that vnum.\n\r", ch );
      return;
   }

   victim = create_mobile( pMobIndex );
   char_to_room( victim, ch->in_room );
   act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
   send_to_char( "Ok.\n\r", ch );
   return;
}

void do_oinvoke( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int vnum, level;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: oinvoke <vnum> <level>.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      level = get_trust( ch );
   }
   else
   {
      if( !is_number( arg2 ) )
      {
         send_to_char( "Syntax: oinvoke <vnum> <level>.\n\r", ch );
         return;
      }
      level = atoi( arg2 );
      if( level < 0 || level > get_trust( ch ) )
      {
         send_to_char( "Limited to your trust level.\n\r", ch );
         return;
      }
   }

   if( !is_number( arg1 ) )
   {
      char arg[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg1, arg );

      vnum = -1;
      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
            if( nifty_is_name( arg, pObjIndex->name ) && ++cnt == count )
            {
               vnum = pObjIndex->vnum;
               break;
            }
      if( vnum == -1 )
      {
         send_to_char( "No such object exists.\n\r", ch );
         return;
      }
   }
   else
      vnum = atoi( arg1 );

   if( get_trust( ch ) < LEVEL_DEMI )
   {
      AREA_DATA *pArea;

      if( IS_NPC( ch ) )
      {
         send_to_char( "Huh?\n\r", ch );
         return;
      }

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this object.\n\r", ch );
         return;
      }
      if( vnum < pArea->low_o_vnum || vnum > pArea->hi_o_vnum )
      {
         send_to_char( "That number is not in your allocated range.\n\r", ch );
         return;
      }
   }

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char( "No object has that vnum.\n\r", ch );
      return;
   }

   obj = create_object( pObjIndex, level );
   if( CAN_WEAR( obj, ITEM_TAKE ) )
   {
      obj = obj_to_char( obj, ch );
   }
   else
   {
      obj = obj_to_room( obj, ch->in_room );
      act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
   }
   send_to_char( "Ok.\n\r", ch );
   return;
}

void do_purge( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      /*
       * 'purge' 
       */
      CHAR_DATA *vnext;
      OBJ_DATA *obj_next;

      for( victim = ch->in_room->first_person; victim; victim = vnext )
      {
         vnext = victim->next_in_room;
         if( IS_NPC( victim ) && victim != ch && !IS_SET( victim->act, ACT_POLYMORPHED ) )
            extract_char( victim, TRUE );
      }

      for( obj = ch->in_room->first_content; obj; obj = obj_next )
      {
         obj_next = obj->next_content;
         if( obj->item_type == ITEM_SPACECRAFT )
            continue;
         extract_obj( obj );
      }

      act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM );
      send_to_char( "Ok.\n\r", ch );
      return;
   }
   victim = NULL;
   obj = NULL;

   /*
    * fixed to get things in room first -- i.e., purge portal (obj),
    * * no more purging mobs with that keyword in another room first
    * * -- Tri 
    */
   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      if( ( victim = get_char_world( ch, arg ) ) == NULL && ( obj = get_obj_world( ch, arg ) ) == NULL ) /* no get_obj_room */
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
   }

/* Single object purge in room for high level purge - Scryn 8/12*/
   if( obj )
   {
      separate_obj( obj );
      act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM );
      act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
   }


   if( !IS_NPC( victim ) )
   {
      send_to_char( "Not on PC's.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\n\r", ch );
      return;
   }

   if( IS_SET( victim->act, ACT_POLYMORPHED ) )
   {
      send_to_char( "You cannot purge a polymorphed player.\n\r", ch );
      return;
   }
   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
   extract_char( victim, TRUE );
   return;
}


void do_low_purge( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Purge what?\n\r", ch );
      return;
   }

   victim = NULL;
   obj = NULL;
   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      send_to_char( "You can't find that here.\n\r", ch );
      return;
   }

   if( obj )
   {
      separate_obj( obj );
      act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
      act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "Not on PC's.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\n\r", ch );
      return;
   }

   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
   act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
   extract_char( victim, TRUE );
   return;
}


void do_balzhur( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Who is deserving of such a fate?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't playing.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "I wouldn't even think of that if I were you...\n\r", ch );
      return;
   }

   set_char_color( AT_WHITE, ch );
   send_to_char( "You summon the demon Balzhur to wreak your wrath!\n\r", ch );
   send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r", ch );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\n\r", victim );
   sprintf( buf, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
   echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
   victim->top_level = 1;
   victim->trust = 0;
   {
      int ability;

      for( ability = 0; ability < MAX_ABILITY; ability++ )
      {
         victim->experience[ability] = 1;
         victim->skill_level[ability] = 1;
      }
   }
   victim->max_hit = 500;
   victim->max_mana = 0;
   victim->max_move = 1000;
   for( sn = 0; sn < top_sn; sn++ )
      victim->pcdata->learned[sn] = 0;
   victim->hit = victim->max_hit;
   victim->mana = victim->max_mana;
   victim->move = victim->max_move;


   sprintf( buf, "%s%s", GOD_DIR, capitalize( victim->name ) );

   if( !remove( buf ) )
      send_to_char( "Player's immortal data destroyed.\n\r", ch );
   else if( errno != ENOENT )
   {
      ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r", errno, strerror( errno ) );
      sprintf( buf2, "%s balzhuring %s", ch->name, buf );
      perror( buf2 );
   }
   sprintf( buf2, "%s.are", capitalize( arg ) );
   for( pArea = first_build; pArea; pArea = pArea->next )
      if( !strcmp( pArea->filename, buf2 ) )
      {
         sprintf( buf, "%s%s", BUILD_DIR, buf2 );
         if( IS_SET( pArea->status, AREA_LOADED ) )
            fold_area( pArea, buf, FALSE );
         close_area( pArea );
         sprintf( buf2, "%s.bak", buf );
         set_char_color( AT_RED, ch ); /* Log message changes colors */
         if( !rename( buf, buf2 ) )
            send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch );
         else if( errno != ENOENT )
         {
            ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r", errno, strerror( errno ) );
            sprintf( buf2, "%s destroying %s", ch->name, buf );
            perror( buf2 );
         }
      }


   make_wizlist(  );
   do_help( victim, "M_BALZHUR_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake after a long period of time...\n\r", victim );
   while( victim->first_carrying )
      extract_obj( victim->first_carrying );
   return;
}

void do_advance( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int level, ability;
   int iLevel, iAbility;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg3 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || !is_number( arg2 ) )
   {
      send_to_char( "Syntax: advance <char> <ability> <level>.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   ability = -1;
/*    if (!str_cmp(arg3, "all")){
    	for (iAbility = 0; iAbility < MAX_ABILITY; iAbility++ ){
    	  ability = iAbility;
          for ( iLevel = victim->skill_level[ability] ; iLevel < level; iLevel++ )
           {
	     victim->experience[ability] = exp_level(iLevel+1);
	     gain_exp2( victim, 0 , ability );
	   }
        }
        return;
    }*/

   for( iAbility = 0; iAbility < MAX_ABILITY; iAbility++ )
   {
      if( !str_prefix( arg3, ability_name[iAbility] ) )
      {
         ability = iAbility;
         break;
      }
   }

   if( ability == -1 )
   {
      send_to_char( "No Such Ability.\n\r", ch );
      do_advance( ch, "" );
      return;
   }


   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( ch ) <= get_trust( victim ) || ch == victim )
   {
      send_to_char( "You can't do that.\n\r", ch );
      return;
   }

   if( ( level = atoi( arg2 ) ) < 1 || level > 500 )
   {
      send_to_char( "Level must be 1 to 500.\n\r", ch );
      return;
   }

   /*
    * Lower level:
    *   Reset to level 1.
    *   Then raise again.
    *   Currently, an imp can lower another imp.
    *   -- Swiftest
    */
   if( level <= victim->skill_level[ability] )
   {
      send_to_char( "Lowering a player's level!\n\r", ch );
      set_char_color( AT_IMMORT, victim );
      send_to_char( "Cursed and forsaken! The gods have lowered your level.\n\r", victim );
      victim->experience[ability] = 0;
      victim->skill_level[ability] = 1;
      if( ability == COMBAT_ABILITY )
         victim->max_hit = 500;
      if( ability == FORCE_ABILITY )
         victim->max_mana = 0;
   }
   else
   {
      send_to_char( "Raising a player's level!\n\r", ch );
      send_to_char( "The gods feel fit to raise your level!\n\r", victim );
   }

   for( iLevel = victim->skill_level[ability]; iLevel < level; iLevel++ )
   {
      victim->experience[ability] = exp_level( iLevel + 1 );
      gain_exp( victim, 0, ability );
   }
   return;
}

void do_immortalize( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int level;
   CHAR_DATA *victim;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Syntax: immortalize <char> <level>\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }
   level = atoi( argument );
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( level > MAX_LEVEL )
   {
      send_to_char( "That is beyond the level range.\n\r", ch );
      return;
   }
   if( level < LEVEL_IMMORTAL )
   {
      send_to_char( "That is below the level range.\n\r", ch );
      return;
   }

   do_help( victim, "M_GODLVL1_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake... all your possessions are gone.\n\r", victim );
   while( victim->first_carrying )
      extract_obj( victim->first_carrying );

   //victim->top_level = LEVEL_IMMORTAL;

/*    advance_level( victim );  */
   victim->top_level = level;
   victim->trust = 0;
   return;
}



void do_trust( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int level;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
   {
      send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
      return;
   }

   if( level > get_trust( ch ) )
   {
      send_to_char( "Limited to your own trust.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You can't do that.\n\r", ch );
      return;
   }

   victim->trust = level;
   send_to_char( "Ok.\n\r", ch );
   return;
}



void do_restore( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Restore whom?\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( !ch->pcdata )
         return;

      if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
      {
         if( IS_NPC( ch ) )
         {
            send_to_char( "You can't do that.\n\r", ch );
            return;
         }
         else
         {
            /*
             * Check if the player did a restore all within the last 18 hours. 
             */
            if( current_time - last_restore_all_time < RESTORE_INTERVAL )
            {
               send_to_char( "Sorry, you can't do a restore all yet.\n\r", ch );
               do_restoretime( ch, "" );
               return;
            }
         }
      }
      last_restore_all_time = current_time;
      ch->pcdata->restore_time = current_time;
      save_char_obj( ch );
      send_to_char( "Ok.\n\r", ch );
      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
         {
            vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
            vch->pcdata->condition[COND_BLOODTHIRST] = ( 10 + vch->top_level );
            update_pos( vch );
            act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT );
         }
      }
   }
   else
   {

      CHAR_DATA *victim;

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }

      if( get_trust( ch ) < LEVEL_LESSER && victim != ch && !( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) ) )
      {
         send_to_char( "You can't do that.\n\r", ch );
         return;
      }

      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      if( victim->pcdata )
         victim->pcdata->condition[COND_BLOODTHIRST] = ( 10 + victim->top_level );
      update_pos( victim );
      if( ch != victim )
         act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );
      send_to_char( "Ok.\n\r", ch );
      return;
   }
}

void do_restoretime( CHAR_DATA * ch, char *argument )
{
   long int time_passed;
   int hour, minute;

   if( !last_restore_all_time )
      ch_printf( ch, "There has been no restore all since reboot\n\r" );
   else
   {
      time_passed = current_time - last_restore_all_time;
      hour = ( int )( time_passed / 3600 );
      minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
      ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\n\r", hour, minute );
   }

   if( !ch->pcdata )
      return;

   if( !ch->pcdata->restore_time )
   {
      send_to_char( "You have never done a restore all.\n\r", ch );
      return;
   }

   time_passed = current_time - ch->pcdata->restore_time;
   hour = ( int )( time_passed / 3600 );
   minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
   ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\n\r", hour, minute );
   return;
}

void do_freeze( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Freeze whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_FREEZE ) )
   {
      REMOVE_BIT( victim->act, PLR_FREEZE );
      send_to_char( "You can play again.\n\r", victim );
      send_to_char( "FREEZE removed.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_FREEZE );
      send_to_char( "You can't do ANYthing!\n\r", victim );
      send_to_char( "FREEZE set.\n\r", ch );
   }

   save_char_obj( victim );

   return;
}

void do_slog( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Secret Log whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   /*
    * No level check, gods can log anyone.
    */
   if( IS_SET( victim->act, PLR_SLOG ) )
   {
      REMOVE_BIT( victim->act, PLR_SLOG );
      send_to_char( "Secret Log removed.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_SLOG );
      send_to_char( "Secret Log set.\n\r", ch );
   }

   return;
}

void do_log( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Log whom?\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      if( fLogAll )
      {
         fLogAll = FALSE;
         send_to_char( "Log ALL off.\n\r", ch );
      }
      else
      {
         fLogAll = TRUE;
         send_to_char( "Log ALL on.\n\r", ch );
      }
      return;
   }

   if( !str_cmp( arg, "pc" ) )
   {
      if( fLogPC )
      {
         fLogPC = FALSE;
         send_to_char( "Log all PC's off.\n\r", ch );
      }
      else
      {
         fLogPC = TRUE;
         send_to_char( "Log ALL PC's on.\n\r", ch );
      }
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   /*
    * No level check, gods can log anyone.
    */
   if( IS_SET( victim->act, PLR_LOG ) )
   {
      REMOVE_BIT( victim->act, PLR_LOG );
      send_to_char( "LOG removed.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_LOG );
      send_to_char( "LOG set.\n\r", ch );
   }

   return;
}


void do_litterbug( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Set litterbug flag on whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_LITTERBUG ) )
   {
      REMOVE_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "You can drop items again.\n\r", victim );
      send_to_char( "LITTERBUG removed.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "You a strange force prevents you from dropping any more items!\n\r", victim );
      send_to_char( "LITTERBUG set.\n\r", ch );
   }

   return;
}


void do_noemote( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Noemote whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_NO_EMOTE ) )
   {
      REMOVE_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can emote again.\n\r", victim );
      send_to_char( "NO_EMOTE removed.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can't emote!\n\r", victim );
      send_to_char( "NO_EMOTE set.\n\r", ch );
   }

   return;
}



void do_notell( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Notell whom?", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_NO_TELL ) )
   {
      REMOVE_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can tell again.\n\r", victim );
      send_to_char( "NO_TELL removed.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can't tell!\n\r", victim );
      send_to_char( "NO_TELL set.\n\r", ch );
   }

   return;
}


void do_notitle( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Notitle whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      send_to_char( "You can set your own title again.\n\r", victim );
      send_to_char( "NOTITLE removed.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      sprintf( buf, "%s", victim->name );
      set_title( victim, buf );
      send_to_char( "You can't set your own title!\n\r", victim );
      send_to_char( "NOTITLE set.\n\r", ch );
   }

   return;
}

void do_silence( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Silence whom?", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_SILENCE ) )
   {
      send_to_char( "Player already silenced, use unsilence to remove.\n\r", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can't use channels!\n\r", victim );
      send_to_char( "SILENCE set.\n\r", ch );
   }

   return;
}

void do_unsilence( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Unsilence whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\n\r", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_SILENCE ) )
   {
      REMOVE_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can use channels again.\n\r", victim );
      send_to_char( "SILENCE removed.\n\r", ch );
   }
   else
   {
      send_to_char( "That player is not silenced.\n\r", ch );
   }

   return;
}




void do_peace( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *rch;

   act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( rch->fighting )
         stop_fighting( rch, TRUE );

      /*
       * Added by Narn, Nov 28/95 
       */
      stop_hating( rch );
      stop_hunting( rch );
      stop_fearing( rch );
   }

   send_to_char( "Ok.\n\r", ch );
   return;
}



BAN_DATA *first_ban;
BAN_DATA *last_ban;

void save_banlist( void )
{
   BAN_DATA *pban;
   FILE *fp;

   if( !( fp = fopen( SYSTEM_DIR BAN_LIST, "w" ) ) )
   {
      bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
      perror( BAN_LIST );
      return;
   }
   for( pban = first_ban; pban; pban = pban->next )
      fprintf( fp, "%d %s~~%s~\n", pban->level, pban->name, pban->ban_time );
   fprintf( fp, "-1\n" );
   fclose( fp );
   return;
}

void do_ban( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   BAN_DATA *pban;
   int bnum;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg );

   set_pager_color( AT_PLAIN, ch );
   if( arg[0] == '\0' )
   {
      send_to_pager( "Banned sites:\n\r", ch );
      send_to_pager( "[ #] (Lv) Time                     Site\n\r", ch );
      send_to_pager( "---- ---- ------------------------ ---------------\n\r", ch );
      for( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
         pager_printf( ch, "[%2d] (%2d) %-24s %s\n\r", bnum, pban->level, pban->ban_time, pban->name );
      return;
   }

   /*
    * People are gonna need .# instead of just # to ban by just last
    * number in the site ip.                               -- Altrag 
    */
   if( is_number( arg ) )
   {
      for( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
         if( bnum == atoi( arg ) )
            break;
      if( !pban )
      {
         do_ban( ch, "" );
         return;
      }
      argument = one_argument( argument, arg );
      if( arg[0] == '\0' )
      {
         do_ban( ch, "help" );
         return;
      }
      if( !str_cmp( arg, "level" ) )
      {
         argument = one_argument( argument, arg );
         if( arg[0] == '\0' || !is_number( arg ) )
         {
            do_ban( ch, "help" );
            return;
         }
         if( atoi( arg ) < 1 || atoi( arg ) > LEVEL_SUPREME )
         {
            ch_printf( ch, "Level range: 1 - %d.\n\r", LEVEL_SUPREME );
            return;
         }
         pban->level = atoi( arg );
         send_to_char( "Ban level set.\n\r", ch );
      }
      else if( !str_cmp( arg, "newban" ) )
      {
         pban->level = 1;
         send_to_char( "New characters banned.\n\r", ch );
      }
      else if( !str_cmp( arg, "mortal" ) )
      {
         pban->level = LEVEL_AVATAR;
         send_to_char( "All mortals banned.\n\r", ch );
      }
      else if( !str_cmp( arg, "total" ) )
      {
         pban->level = LEVEL_SUPREME;
         send_to_char( "Everyone banned.\n\r", ch );
      }
      else
      {
         do_ban( ch, "help" );
         return;
      }
      save_banlist(  );
      return;
   }

   if( !str_cmp( arg, "help" ) )
   {
      send_to_char( "Syntax: ban <site address>\n\r", ch );
      send_to_char( "Syntax: ban <ban number> <level <lev>|newban|mortal|" "total>\n\r", ch );
      return;
   }

   for( pban = first_ban; pban; pban = pban->next )
   {
      if( !str_cmp( arg, pban->name ) )
      {
         send_to_char( "That site is already banned!\n\r", ch );
         return;
      }
   }

   CREATE( pban, BAN_DATA, 1 );
   LINK( pban, first_ban, last_ban, next, prev );
   pban->name = str_dup( arg );
   pban->level = LEVEL_AVATAR;
   sprintf( buf, "%24.24s", ctime( &current_time ) );
   pban->ban_time = str_dup( buf );
   save_banlist(  );
   send_to_char( "Ban created.  Mortals banned from site.\n\r", ch );
   return;
}


void do_allow( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   BAN_DATA *pban;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Remove which site from the ban list?\n\r", ch );
      return;
   }

   for( pban = first_ban; pban; pban = pban->next )
   {
      if( !str_cmp( arg, pban->name ) )
      {
         UNLINK( pban, first_ban, last_ban, next, prev );
         if( pban->ban_time )
            DISPOSE( pban->ban_time );
         DISPOSE( pban->name );
         DISPOSE( pban );
         save_banlist(  );
         send_to_char( "Site no longer banned.\n\r", ch );
         return;
      }
   }

   send_to_char( "Site is not banned.\n\r", ch );
   return;
}



void do_wizlock( CHAR_DATA * ch, char *argument )
{
   extern bool wizlock;
   wizlock = !wizlock;

   if( wizlock )
      send_to_char( "Game wizlocked.\n\r", ch );
   else
      send_to_char( "Game un-wizlocked.\n\r", ch );

   return;
}


void do_noresolve( CHAR_DATA * ch, char *argument )
{
   sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

   if( sysdata.NO_NAME_RESOLVING )
      send_to_char( "Name resolving disabled.\n\r", ch );
   else
      send_to_char( "Name resolving enabled.\n\r", ch );

   return;
}

/* Output of command reformmated by Samson 2-8-98, and again on 4-7-98 */
void do_users( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   int count;
   char *st;

   set_pager_color( AT_PLAIN, ch );

   count = 0;
   send_to_pager( "Desc|     Constate      |Idle|    Player    | HostIP                   \n\r", ch );
   send_to_pager( "----+-------------------+----+--------------+--------------------------\n\r", ch );
   for( d = first_descriptor; d; d = d->next )
   {
      switch ( d->connected )
      {
         case CON_PLAYING:
            st = "Playing";
            break;
         case CON_GET_NAME:
            st = "Get name";
            break;
         case CON_GET_OLD_PASSWORD:
            st = "Get password";
            break;
         case CON_CONFIRM_NEW_NAME:
            st = "Confirm name";
            break;
         case CON_GET_NEW_PASSWORD:
            st = "New password";
            break;
         case CON_CONFIRM_NEW_PASSWORD:
            st = "Confirm password";
            break;
         case CON_GET_NEW_SEX:
            st = "Get sex";
            break;
         case CON_READ_MOTD:
            st = "Reading MOTD";
            break;
         case CON_EDITING:
            st = "In line editor";
            break;
         case CON_PRESS_ENTER:
            st = "Press enter";
            break;
         case CON_ROLL_STATS:
            st = "Rolling stats";
            break;
         case CON_COPYOVER_RECOVER:
            st = "Hotboot";
            break;
         default:
            st = "Invalid!!!!";
            break;
      }

      if( !argument || argument[0] == '\0' )
      {
         if( get_trust( ch ) >= LEVEL_ASCENDANT || ( d->character && can_see( ch, d->character ) ) )
         {
            count++;
            pager_printf( ch, " %3d| %-17s |%4d| %-12s | %s \n\r", d->descriptor, st, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
      else
      {
         if( ( get_trust( ch ) >= LEVEL_SUPREME || ( d->character && can_see( ch, d->character ) ) )
             && ( !str_prefix( argument, d->host ) || ( d->character && !str_prefix( argument, d->character->name ) ) ) )
         {
            count++;
            pager_printf( ch, " %3d| %2d|%4d| %-12s | %s \n\r", d->descriptor, d->connected, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
   }
   pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
   return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   bool mobsonly;
   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Force whom to do what?\n\r", ch );
      return;
   }

   mobsonly = get_trust( ch ) < sysdata.level_forcepc;

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( mobsonly )
      {
         send_to_char( "Force whom to do what?\n\r", ch );
         return;
      }

      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch ) )
         {
            act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
            interpret( vch, argument );
         }
      }
   }
   else
   {
      CHAR_DATA *victim;

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }

      if( victim == ch )
      {
         send_to_char( "Aye aye, right away!\n\r", ch );
         return;
      }

      if( ( get_trust( victim ) >= get_trust( ch ) ) || ( mobsonly && !IS_NPC( victim ) ) )
      {
         send_to_char( "Do it yourself!\n\r", ch );
         return;
      }

      act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
      interpret( victim, argument );
   }

   send_to_char( "Ok.\n\r", ch );
   return;
}


void do_invis( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short level;

   /*
    * if ( IS_NPC(ch))
    * return;
    */

   argument = one_argument( argument, arg );
   if( arg && arg[0] != '\0' )
   {
      if( !is_number( arg ) )
      {
         send_to_char( "Usage: invis | invis <level>\n\r", ch );
         return;
      }
      level = atoi( arg );
      if( level < 2 || level > get_trust( ch ) )
      {
         send_to_char( "Invalid level.\n\r", ch );
         return;
      }

      if( !IS_NPC( ch ) )
      {
         ch->pcdata->wizinvis = level;
         ch_printf( ch, "Wizinvis level set to %d.\n\r", level );
      }

      if( IS_NPC( ch ) )
      {
         ch->mobinvis = level;
         ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
      }
      return;
   }

   if( !IS_NPC( ch ) )
   {
      if( ch->pcdata->wizinvis < 2 )
         ch->pcdata->wizinvis = ch->top_level;
   }

   if( IS_NPC( ch ) )
   {
      if( ch->mobinvis < 2 )
         ch->mobinvis = ch->top_level;
   }

   if( IS_SET( ch->act, PLR_WIZINVIS ) )
   {
      REMOVE_BIT( ch->act, PLR_WIZINVIS );
      act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\n\r", ch );
   }
   else
   {
      SET_BIT( ch->act, PLR_WIZINVIS );
      act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\n\r", ch );
   }

   return;
}


void do_holylight( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->act, PLR_HOLYLIGHT ) )
   {
      REMOVE_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode off.\n\r", ch );
   }
   else
   {
      SET_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode on.\n\r", ch );
   }

   return;
}

bool check_area_conflict( AREA_DATA *area, int low_range, int hi_range )
{
   if( low_range < area->low_r_vnum && area->low_r_vnum < hi_range )
      return TRUE;
   if( low_range < area->low_m_vnum && area->low_m_vnum < hi_range )
      return TRUE;
   if( low_range < area->low_o_vnum && area->low_o_vnum < hi_range )
      return TRUE;

   if( low_range < area->hi_r_vnum && area->hi_r_vnum < hi_range )
      return TRUE;
   if( low_range < area->hi_m_vnum && area->hi_m_vnum < hi_range )
      return TRUE;
   if( low_range < area->hi_o_vnum && area->hi_o_vnum < hi_range )
      return TRUE;

   if( ( low_range >= area->low_r_vnum ) && ( low_range <= area->hi_r_vnum ) )
      return TRUE;
   if( ( low_range >= area->low_m_vnum ) && ( low_range <= area->hi_m_vnum ) )
      return TRUE;
   if( ( low_range >= area->low_o_vnum ) && ( low_range <= area->hi_o_vnum ) )
      return TRUE;

   if( ( hi_range <= area->hi_r_vnum ) && ( hi_range >= area->low_r_vnum ) )
      return TRUE;
   if( ( hi_range <= area->hi_m_vnum ) && ( hi_range >= area->low_m_vnum ) )
      return TRUE;
   if( ( hi_range <= area->hi_o_vnum ) && ( hi_range >= area->low_o_vnum ) )
      return TRUE;

   return FALSE;
}

/* Runs the entire list, easier to call in places that have to check them all */
bool check_area_conflicts( int lo, int hi )
{
   AREA_DATA *area;

   for( area = first_area; area; area = area->next )
      if( check_area_conflict( area, lo, hi ) )
         return TRUE;

   for( area = first_build; area; area = area->next )
      if( check_area_conflict( area, lo, hi ) )
         return TRUE;

   return FALSE;
}

/* Consolidated *assign function. 
 * Assigns room/obj/mob ranges and initializes new zone - Samson 2-12-99 
 */
/* Bugfix: Vnum range would not be saved properly without placeholders at both ends - Samson 1-6-00 */
void do_vassign( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   int lo = -1, hi = -1;
   CHAR_DATA *victim, *mob;
   ROOM_INDEX_DATA *room;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   AREA_DATA *tarea;
   char filename[256];

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   lo = atoi( arg2 );
   hi = atoi( arg3 );

   if( arg1[0] == '\0' || lo < 0 || hi < 0 )
   {
      send_to_char( "Syntax: vassign <who> <low> <high>\n\r", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They don't seem to be around.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) || get_trust( victim ) < LEVEL_CREATOR )
   {
      send_to_char( "They wouldn't know what to do with a vnum range.\n\r", ch );
      return;
   }

   if( lo == 0 && hi == 0 )
   {
      victim->pcdata->area = NULL;
      victim->pcdata->r_range_lo = 0;
      victim->pcdata->r_range_hi = 0;
      victim->pcdata->o_range_lo = 0;
      victim->pcdata->o_range_hi = 0;
      victim->pcdata->m_range_lo = 0;
      victim->pcdata->m_range_hi = 0;
      ch_printf( victim, "%s has removed your vnum range.\n\r", ch->name );
      save_char_obj( victim );
      return;
   }

   if( victim->pcdata->area && lo != 0 )
   {
      send_to_char( "You cannot assign them a range, they already have one!\n\r", ch );
      return;
   }

   if( lo == 0 && hi != 0 )
   {
      send_to_char( "Unacceptable vnum range, low vnum cannot be 0 when hi vnum is not.\n\r", ch );
      return;
   }

   if( lo > hi )
   {
      send_to_char( "Unacceptable vnum range, low vnum must be smaller than high vnum.\n\r", ch );
      return;
   }

   if( check_area_conflicts( lo, hi ) )
   {
      send_to_char( "That vnum range conflicts with another area. Check the zones or vnums command.\n\r", ch );
      return;
   }

   victim->pcdata->r_range_lo = lo;
   victim->pcdata->r_range_hi = hi;
   victim->pcdata->o_range_lo = lo;
   victim->pcdata->o_range_hi = hi;
   victim->pcdata->m_range_lo = lo;
   victim->pcdata->m_range_hi = hi;
   assign_area( victim );
   send_to_char( "Done.\n\r", ch );
   ch_printf( victim, "%s has assigned you the vnum range %d - %d.\n\r", ch->name, lo, hi );
   assign_area( victim );  /* Put back by Thoric on 02/07/96 */

   if( !victim->pcdata->area )
   {
      bug( "%s: assign_area failed", __FUNCTION__ );
      return;
   }

   tarea = victim->pcdata->area;

   /*
    * Initialize first and last rooms in range 
    */
   if( !( room = make_room( lo, tarea ) ) )
   {
      bug( "%s: make_room failed to initialize first room.", __FUNCTION__ );
      return;
   }

   if( !( room = make_room( hi, tarea ) ) )
   {
      bug( "%s: make_room failed to initialize last room.", __FUNCTION__ );
      return;
   }

   /*
    * Initialize first mob in range 
    */
   if( !( pMobIndex = make_mobile( lo, 0, "first mob" ) ) )
   {
      bug( "%s: make_mobile failed to initialize first mob.", __FUNCTION__ );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, room );

   /*
    * Initialize last mob in range 
    */
   if( !( pMobIndex = make_mobile( hi, 0, "last mob" ) ) )
   {
      bug( "%s: make_mobile failed to initialize last mob.", __FUNCTION__ );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, room );

   /*
    * Initialize first obj in range 
    */
   if( !( pObjIndex = make_object( lo, 0, "first obj" ) ) )
   {
      bug( "%s: make_object failed to initialize first obj.", __FUNCTION__ );
      return;
   }
   obj = create_object( pObjIndex, 0 );
   obj_to_room( obj, room );

   /*
    * Initialize last obj in range 
    */
   if( !( pObjIndex = make_object( hi, 0, "last obj" ) ) )
   {
      bug( "%s: make_object failed to initialize last obj.", __FUNCTION__ );
      return;
   }
   obj = create_object( pObjIndex, 0 );
   obj_to_room( obj, room );

   /*
    * Save character and newly created zone 
    */
   save_char_obj( victim );

   if( !IS_SET( tarea->status, AREA_DELETED ) )
   {
      snprintf( filename, 256, "%s%s", BUILD_DIR, tarea->filename );
      fold_area( tarea, filename, FALSE );
   }

   set_char_color( AT_IMMORT, ch );
   ch_printf( ch, "Vnum range set for %s and initialized.\n\r", victim->name );

   return;
}

void do_cmdtable( CHAR_DATA * ch, char *argument )
{
   int hash, cnt;
   CMDTYPE *cmd;

   set_pager_color( AT_PLAIN, ch );
   send_to_pager( "Commands and Number of Uses This Run\n\r", ch );

   for( cnt = hash = 0; hash < 126; hash++ )
      for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
      {
         if( ( ++cnt ) % 4 )
            pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses );
         else
            pager_printf( ch, "%-6.6s %4d\n\r", cmd->name, cmd->userec.num_uses );
      }
   return;
}

/* 
 * Load up a player file 
 */
void do_loadup( CHAR_DATA * ch, char *argument )
{
   char fname[1024];
   char name[256];
   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *temp;
   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: loadup <playername>\n\r", ch );
      return;
   }
   for( temp = first_char; temp; temp = temp->next )
   {
      if( IS_NPC( temp ) )
         continue;
      if( can_see( ch, temp ) && !str_cmp( argument, temp->name ) )
         break;
   }
   if( temp != NULL )
   {
      send_to_char( "They are already playing.\n\r", ch );
      return;
   }

   name[0] = UPPER( name[0] );

   sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
   if( stat( fname, &fst ) != -1 )
   {
      CREATE( d, DESCRIPTOR_DATA, 1 );
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE( d->outbuf, char, d->outsize );

      loaded = load_char_obj( d, name, FALSE );
      add_char( d->character );
      old_room_vnum = d->character->in_room->vnum;
      char_to_room( d->character, ch->in_room );
      if( get_trust( d->character ) >= get_trust( ch ) )
      {
         do_say( d->character, "Do *NOT* disturb me again!" );
         send_to_char( "I think you'd better leave that player alone!\n\r", ch );
         d->character->desc = NULL;
         do_quit( d->character, "" );
         return;
      }
      d->character->desc = NULL;
      d->character->retran = old_room_vnum;
      d->character = NULL;
      DISPOSE( d->outbuf );
      DISPOSE( d );
      ch_printf( ch, "Player %s loaded from room %d.\n\r", capitalize( name ), old_room_vnum );
      sprintf( buf, "%s appears from nowhere, eyes glazed over.\n\r", capitalize( name ) );
      act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   /*
    * else no player file 
    */
   send_to_char( "No such player.\n\r", ch );
   return;
}

void do_fixchar( CHAR_DATA * ch, char *argument )
{
   char name[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: fixchar <playername>\n\r", ch );
      return;
   }
   victim = get_char_room( ch, name );
   if( !victim )
   {
      send_to_char( "They're not here.\n\r", ch );
      return;
   }
   fix_char( victim );
/*  victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0; */
   send_to_char( "Done.\n\r", ch );
}

void do_newbieset( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   CHAR_DATA *victim;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: newbieset <char>.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( ( victim->top_level < 1 ) || ( victim->top_level > 5 ) )
   {
      send_to_char( "Level of victim must be 1 to 5.\n\r", ch );
      return;
   }
   obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_SHIELD ), 1 );
   obj_to_char( obj, victim );

   obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_DAGGER ), 1 );
   obj_to_char( obj, victim );

   /*
    * Added by Brittany, on Nov. 24, 1996. The object is the adventurer's 
    * guide to the realms of despair, part of academy.are. 
    */
   {
      OBJ_INDEX_DATA *obj_ind = get_obj_index( 10333 );
      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, 1 );
         obj_to_char( obj, victim );
      }
   }

/* Added the burlap sack to the newbieset.  The sack is part of sgate.are
   called Spectral Gate.  Brittany */

   {

      OBJ_INDEX_DATA *obj_ind = get_obj_index( 123 );
      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, 1 );
         obj_to_char( obj, victim );
      }
   }

   act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT );
   ch_printf( ch, "You have re-equipped %s.\n\r", victim->name );
   return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names( char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;

   *out = '\0';
   while( inp && *inp )
   {
      inp = one_argument( inp, buf );
      if( ( len = strlen( buf ) ) >= 5 && !strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            strcat( out, " " );
         strcat( out, buf );
      }
   }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names( char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;

   *out = '\0';
   while( inp && *inp )
   {
      inp = one_argument( inp, buf );
      if( ( len = strlen( buf ) ) < 5 || strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            strcat( out, " " );
         strcat( out, buf );
      }
   }
}

void do_bestowarea( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   int arg_len;

   argument = one_argument( argument, arg );

   if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
   {
      send_to_char( "Sorry...\n\r", ch );
      return;
   }

   if( !*arg )
   {
      send_to_char( "Syntax:\n\r"
                    "bestowarea <victim> <filename>.are\n\r"
                    "bestowarea <victim> none             removes bestowed areas\n\r"
                    "bestowarea <victim> list             lists bestowed areas\n\r"
                    "bestowarea <victim>                  lists bestowed areas\n\r", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\n\r", ch );
      return;
   }

   if( get_trust( victim ) < LEVEL_IMMORTAL )
   {
      send_to_char( "They aren't an immortal.\n\r", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = str_dup( "" );

   if( !*argument || !str_cmp( argument, "list" ) )
   {
      extract_area_names( victim->pcdata->bestowments, buf );
      ch_printf( ch, "Bestowed areas: %s\n\r", buf );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      remove_area_names( victim->pcdata->bestowments, buf );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   arg_len = strlen( argument );
   if( arg_len < 5
       || argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a'
       || argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e' )
   {
      send_to_char( "You can only bestow an area name\n\r", ch );
      send_to_char( "E.G. bestow joe sam.are\n\r", ch );
      return;
   }

   sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );
   DISPOSE( victim->pcdata->bestowments );
   victim->pcdata->bestowments = str_dup( buf );
   ch_printf( victim, "%s has bestowed on you the area: %s\n\r", ch->name, argument );
   send_to_char( "Done.\n\r", ch );
}

void do_bestow( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], arg_buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   CMDTYPE *cmd;
   bool fComm = FALSE;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Bestow whom with what?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\n\r", ch );
      return;
   }
   if( victim == ch || get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You aren't powerful enough...\n\r", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = str_dup( "" );

   if( argument[0] == '\0' || !str_cmp( argument, "show list" ) )
   {
      ch_printf( ch, "Current bestowed commands on %s: %s.\n\r", victim->name, victim->pcdata->bestowments );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( "" );
      ch_printf( ch, "Bestowments removed from %s.\n\r", victim->name );
      ch_printf( victim, "%s has removed your bestowed commands.\n\r", ch->name );
      return;
   }

   arg_buf[0] = '\0';

   argument = one_argument( argument, arg );

   while( arg && arg[0] != '\0' )
   {
      char *cmd_buf, cmd_tmp[MAX_INPUT_LENGTH];
      bool cFound = FALSE;

      if( !( cmd = find_command( arg ) ) )
      {
         ch_printf( ch, "No such command as %s!\n\r", arg );
         argument = one_argument( argument, arg );
         continue;
      }
      else if( cmd->level > get_trust( ch ) )
      {
         ch_printf( ch, "You can't bestow the %s command!\n\r", arg );
         argument = one_argument( argument, arg );
         continue;
      }

      cmd_buf = victim->pcdata->bestowments;
      cmd_buf = one_argument( cmd_buf, cmd_tmp );
      while( cmd_tmp && cmd_tmp[0] != '\0' )
      {
         if( !str_cmp( cmd_tmp, arg ) )
         {
            cFound = TRUE;
            break;
         }

         cmd_buf = one_argument( cmd_buf, cmd_tmp );
      }

      if( cFound == TRUE )
      {
         argument = one_argument( argument, arg );
         continue;
      }

      sprintf( arg, "%s ", arg );
      strcat( arg_buf, arg );
      argument = one_argument( argument, arg );
      fComm = TRUE;
   }
   if( !fComm )
   {
      send_to_char( "Good job, knucklehead... you just bestowed them with that master command called 'NOTHING!'\n\r", ch );
      return;
   }

   if( arg_buf[strlen( arg_buf ) - 1] == ' ' )
      arg_buf[strlen( arg_buf ) - 1] = '\0';

   sprintf( buf, "%s %s", victim->pcdata->bestowments, arg_buf );
   DISPOSE( victim->pcdata->bestowments );
   smash_tilde( buf );
   victim->pcdata->bestowments = str_dup( buf );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has bestowed on you the command(s): %s\n\r", ch->name, arg_buf );
   send_to_char( "Done.\n\r", ch );
}

struct tm *update_time( struct tm *old_time )
{
   time_t ntime;

   ntime = mktime( old_time );
   return localtime( &ntime );
}

void do_set_boot_time( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   bool check;

   check = FALSE;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch );
      send_to_char( "        setboot manual {0/1}\n\r", ch );
      send_to_char( "        setboot default\n\r", ch );
      ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\n\r", reboot_time, set_boot_time->manual );
      return;
   }

   if( !str_cmp( arg, "time" ) )
   {
      struct tm *now_time;

      argument = one_argument( argument, arg );
      argument = one_argument( argument, arg1 );
      if( !*arg || !*arg1 || !is_number( arg ) || !is_number( arg1 ) )
      {
         send_to_char( "You must input a value for hour and minute.\n\r", ch );
         return;
      }
      now_time = localtime( &current_time );

      if( ( now_time->tm_hour = atoi( arg ) ) < 0 || now_time->tm_hour > 23 )
      {
         send_to_char( "Valid range for hour is 0 to 23.\n\r", ch );
         return;
      }

      if( ( now_time->tm_min = atoi( arg1 ) ) < 0 || now_time->tm_min > 59 )
      {
         send_to_char( "Valid range for minute is 0 to 59.\n\r", ch );
         return;
      }

      argument = one_argument( argument, arg );
      if( *arg != '\0' && is_number( arg ) )
      {
         if( ( now_time->tm_mday = atoi( arg ) ) < 1 || now_time->tm_mday > 31 )
         {
            send_to_char( "Valid range for day is 1 to 31.\n\r", ch );
            return;
         }
         argument = one_argument( argument, arg );
         if( *arg != '\0' && is_number( arg ) )
         {
            if( ( now_time->tm_mon = atoi( arg ) ) < 1 || now_time->tm_mon > 12 )
            {
               send_to_char( "Valid range for month is 1 to 12.\n\r", ch );
               return;
            }
            now_time->tm_mon--;
            argument = one_argument( argument, arg );
            if( ( now_time->tm_year = atoi( arg ) - 1900 ) < 0 || now_time->tm_year > 199 )
            {
               send_to_char( "Valid range for year is 1900 to 2099.\n\r", ch );
               return;
            }
         }
      }
      now_time->tm_sec = 0;
      if( mktime( now_time ) < current_time )
      {
         send_to_char( "You can't set a time previous to today!\n\r", ch );
         return;
      }
      if( set_boot_time->manual == 0 )
         set_boot_time->manual = 1;
      new_boot_time = update_time( now_time );
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check( mktime( new_boot_time ) );
      get_reboot_string(  );

      ch_printf( ch, "Boot time set to %s\n\r", reboot_time );
      check = TRUE;
   }
   else if( !str_cmp( arg, "manual" ) )
   {
      argument = one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Please enter a value for manual boot on/off\n\r", ch );
         return;
      }

      if( !is_number( arg1 ) )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\n\r", ch );
         return;
      }

      if( atoi( arg1 ) < 0 || atoi( arg1 ) > 1 )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\n\r", ch );
         return;
      }

      set_boot_time->manual = atoi( arg1 );
      ch_printf( ch, "Manual bit set to %s\n\r", arg1 );
      check = TRUE;
      get_reboot_string(  );
      return;
   }

   else if( !str_cmp( arg, "default" ) )
   {
      set_boot_time->manual = 0;
      /*
       * Reinitialize new_boot_time 
       */
      new_boot_time = localtime( &current_time );
      new_boot_time->tm_mday += 1;
      if( new_boot_time->tm_hour > 12 )
         new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time( new_boot_time );

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char( "Reboot time set back to normal.\n\r", ch );
      check = TRUE;
   }

   if( !check )
   {
      send_to_char( "Invalid argument for setboot.\n\r", ch );
      return;
   }

   else
   {
      get_reboot_string(  );
      new_boot_time_t = mktime( new_boot_time );
   }
}

/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to 
 * pfiles and the correct password
 *
 * Updated to MD5 - only needs one argument now - Samson 7-25-04
 */
void do_form_password( CHAR_DATA * ch, char *argument )
{
   char *pwcheck, *p;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: formpass <password>\n\r", ch );
      return;
   }

   /*
    * This is arbitrary to discourage weak passwords 
    */
   if( strlen( argument ) < 5 )
   {
      send_to_char( "Usage: formpass <password>\n\r", ch );
      send_to_char( "New password must be at least 5 characters in length.\n\r", ch );
      return;
   }

   if( argument[0] == '!' )
   {
      send_to_char( "Usage: formpass <password>\n\r", ch );
      send_to_char( "New password cannot begin with the '!' character.\n\r", ch );
      return;
   }

   pwcheck = smaug_crypt( argument );
   for( p = pwcheck; *p != '\0'; p++ )
   {
      if( *p == '~' )
      {
         send_to_char( "New password not acceptable, cannot use the ~ character.\n\r", ch );
         return;
      }
   }
   ch_printf( ch, "%s results in the encrypted string: %s\n\r", argument, pwcheck );
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "If you want to destroy a character, spell it out!\n\r", ch );
   return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA * pArea )
{
   CHAR_DATA *ech;
   CHAR_DATA *ech_next;
   OBJ_DATA *eobj;
   OBJ_DATA *eobj_next;
   int icnt;
   ROOM_INDEX_DATA *rid;
   ROOM_INDEX_DATA *rid_next;
   OBJ_INDEX_DATA *oid;
   OBJ_INDEX_DATA *oid_next;
   MOB_INDEX_DATA *mid;
   MOB_INDEX_DATA *mid_next;
   EXTRA_DESCR_DATA *eed;
   EXTRA_DESCR_DATA *eed_next;
   EXIT_DATA *pexit;
   EXIT_DATA *exit_next;
   MPROG_ACT_LIST *mpact;
   MPROG_ACT_LIST *mpact_next;
   MPROG_DATA *mprog;
   MPROG_DATA *mprog_next;
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;

   for( ech = first_char; ech; ech = ech_next )
   {
      ech_next = ech->next;

      if( ech->fighting )
         stop_fighting( ech, TRUE );
      if( IS_NPC( ech ) )
      {
         /*
          * if mob is in area, or part of area. 
          */
         if( URANGE( pArea->low_m_vnum, ech->pIndexData->vnum,
                     pArea->hi_m_vnum ) == ech->pIndexData->vnum || ( ech->in_room && ech->in_room->area == pArea ) )
            extract_char( ech, TRUE );
         continue;
      }
      if( ech->in_room && ech->in_room->area == pArea )
         do_recall( ech, "" );
   }
   for( eobj = first_object; eobj; eobj = eobj_next )
   {
      eobj_next = eobj->next;
      /*
       * if obj is in area, or part of area. 
       */
      if( URANGE( pArea->low_o_vnum, eobj->pIndexData->vnum,
                  pArea->hi_o_vnum ) == eobj->pIndexData->vnum || ( eobj->in_room && eobj->in_room->area == pArea ) )
         extract_obj( eobj );
   }
   for( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
   {
      for( rid = room_index_hash[icnt]; rid; rid = rid_next )
      {
         rid_next = rid->next;

         for( pexit = rid->first_exit; pexit; pexit = exit_next )
         {
            exit_next = pexit->next;
            if( rid->area == pArea || pexit->to_room->area == pArea )
            {
               STRFREE( pexit->keyword );
               STRFREE( pexit->description );
               UNLINK( pexit, rid->first_exit, rid->last_exit, next, prev );
               DISPOSE( pexit );
            }
         }
         if( rid->area != pArea )
            continue;
         STRFREE( rid->name );
         STRFREE( rid->description );
         if( rid->first_person )
         {
            bug( "close_area: room with people #%d", rid->vnum );
            for( ech = rid->first_person; ech; ech = ech_next )
            {
               ech_next = ech->next_in_room;
               if( ech->fighting )
                  stop_fighting( ech, TRUE );
               if( IS_NPC( ech ) )
                  extract_char( ech, TRUE );
               else
                  do_recall( ech, "" );
            }
         }
         if( rid->first_content )
         {
            bug( "close_area: room with contents #%d", rid->vnum );
            for( eobj = rid->first_content; eobj; eobj = eobj_next )
            {
               eobj_next = eobj->next_content;
               extract_obj( eobj );
            }
         }
         for( eed = rid->first_extradesc; eed; eed = eed_next )
         {
            eed_next = eed->next;
            STRFREE( eed->keyword );
            STRFREE( eed->description );
            DISPOSE( eed );
         }
         for( mpact = rid->mpact; mpact; mpact = mpact_next )
         {
            mpact_next = mpact->next;
            STRFREE( mpact->buf );
            DISPOSE( mpact );
         }
         for( mprog = rid->mudprogs; mprog; mprog = mprog_next )
         {
            mprog_next = mprog->next;
            STRFREE( mprog->arglist );
            STRFREE( mprog->comlist );
            DISPOSE( mprog );
         }
         if( rid == room_index_hash[icnt] )
            room_index_hash[icnt] = rid->next;
         else
         {
            ROOM_INDEX_DATA *trid;

            for( trid = room_index_hash[icnt]; trid; trid = trid->next )
               if( trid->next == rid )
                  break;
            if( !trid )
               bug( "Close_area: rid not in hash list %d", rid->vnum );
            else
               trid->next = rid->next;
         }
         DISPOSE( rid );
      }

      for( mid = mob_index_hash[icnt]; mid; mid = mid_next )
      {
         mid_next = mid->next;

         if( mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum )
            continue;

         STRFREE( mid->player_name );
         STRFREE( mid->short_descr );
         STRFREE( mid->long_descr );
         STRFREE( mid->description );
         if( mid->pShop )
         {
            UNLINK( mid->pShop, first_shop, last_shop, next, prev );
            DISPOSE( mid->pShop );
         }
         if( mid->rShop )
         {
            UNLINK( mid->rShop, first_repair, last_repair, next, prev );
            DISPOSE( mid->rShop );
         }
         for( mprog = mid->mudprogs; mprog; mprog = mprog_next )
         {
            mprog_next = mprog->next;
            STRFREE( mprog->arglist );
            STRFREE( mprog->comlist );
            DISPOSE( mprog );
         }
         if( mid == mob_index_hash[icnt] )
            mob_index_hash[icnt] = mid->next;
         else
         {
            MOB_INDEX_DATA *tmid;

            for( tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next )
               if( tmid->next == mid )
                  break;
            if( !tmid )
               bug( "Close_area: mid not in hash list %s", mid->vnum );
            else
               tmid->next = mid->next;
         }
         DISPOSE( mid );
      }

      for( oid = obj_index_hash[icnt]; oid; oid = oid_next )
      {
         oid_next = oid->next;

         if( oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum )
            continue;

         STRFREE( oid->name );
         STRFREE( oid->short_descr );
         STRFREE( oid->description );
         STRFREE( oid->action_desc );

         for( eed = oid->first_extradesc; eed; eed = eed_next )
         {
            eed_next = eed->next;
            STRFREE( eed->keyword );
            STRFREE( eed->description );
            DISPOSE( eed );
         }
         for( paf = oid->first_affect; paf; paf = paf_next )
         {
            paf_next = paf->next;
            DISPOSE( paf );
         }
         for( mprog = oid->mudprogs; mprog; mprog = mprog_next )
         {
            mprog_next = mprog->next;
            STRFREE( mprog->arglist );
            STRFREE( mprog->comlist );
            DISPOSE( mprog );
         }
         if( oid == obj_index_hash[icnt] )
            obj_index_hash[icnt] = oid->next;
         else
         {
            OBJ_INDEX_DATA *toid;

            for( toid = obj_index_hash[icnt]; toid; toid = toid->next )
               if( toid->next == oid )
                  break;
            if( !toid )
               bug( "Close_area: oid not in hash list %s", oid->vnum );
            else
               toid->next = oid->next;
         }
         DISPOSE( oid );
      }
   }
   DISPOSE( pArea->name );
   DISPOSE( pArea->filename );
   STRFREE( pArea->author );
   if( IS_SET( pArea->flags, AFLAG_PROTOTYPE ))
   {
      UNLINK( pArea, first_build, last_build, next, prev );
      UNLINK( pArea, first_bsort, last_bsort, next_sort, prev_sort );
   }
   else
   {
      UNLINK( pArea, first_area, last_area, next, prev );
      UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
   }
   DISPOSE( pArea );
}

void close_all_areas( void )
{
   AREA_DATA *area, *area_next;

   for( area = first_area; area; area = area_next )
   {
      area_next = area->next;
      close_area( area );
   }
   for( area = first_build; area; area = area_next )
   {
      area_next = area->next;
      close_area( area );
   }
   return;
}

void do_destroy( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char *name;
   struct stat fst;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Destroy what player file?\n\r", ch );
      return;
   }

   /*
    * Set the file points.
    */
   name = capitalize( arg );
   sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
   sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), name );

   /*
    * This check makes sure the name is valid and that the file is there, else there
    * is no need to go on. -Orion
    */
   if( !check_parse_name( name ) || lstat( buf, &fst ) == -1 )
   {
      ch_printf( ch, "No player exists by the name %s.\n\r", name );
      return;
   }

   if( strstr( argument, "." ) )
   {
      send_to_char( "Nope.\n\r", ch );
      return;
   }

   for( victim = first_char; victim; victim = victim->next )
      if( !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
         break;
   if( !victim )
   {
      DESCRIPTOR_DATA *d;

      /*
       * Make sure they aren't halfway logged in. 
       */
      for( d = first_descriptor; d; d = d->next )
         if( ( victim = d->character ) && !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
            break;
      if( d )
         close_socket( d, TRUE );
   }
   else
   {
      int x, y;

      quitting_char = victim;
      save_char_obj( victim );
      saving_char = NULL;
      extract_char( victim, TRUE );
      for( x = 0; x < MAX_WEAR; x++ )
         for( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;
   }

   if( !rename( buf, buf2 ) )
   {
      AREA_DATA *pArea;

      set_char_color( AT_RED, ch );
      send_to_char( "Player destroyed.  Pfile saved in backup directory.\n\r", ch );
      sprintf( buf, "%s%s", GOD_DIR, capitalize( arg ) );
      if( !remove( buf ) )
         send_to_char( "Player's immortal data destroyed.\n\r", ch );
      else if( errno != ENOENT )
      {
         ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric.\n\r", errno, strerror( errno ) );
         sprintf( buf2, "%s destroying %s", ch->name, buf );
         perror( buf2 );
      }

      sprintf( buf2, "%s.are", capitalize( arg ) );
      for( pArea = first_build; pArea; pArea = pArea->next )
         if( !strcmp( pArea->filename, buf2 ) )
         {
            sprintf( buf, "%s%s", BUILD_DIR, buf2 );
            if( IS_SET( pArea->status, AREA_LOADED ) )
            {
               fold_area( pArea, buf, FALSE );
               close_area( pArea );
            }
            sprintf( buf2, "%s.bak", buf );
            set_char_color( AT_RED, ch ); /* Log message changes colors */
            if( !rename( buf, buf2 ) )
               send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch );
            else if( errno != ENOENT )
            {
               ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r", errno, strerror( errno ) );
               sprintf( buf2, "%s destroying %s", ch->name, buf );
               perror( buf2 );
            }
         }
   }
   else if( errno == ENOENT )
   {
      set_char_color( AT_PLAIN, ch );
      send_to_char( "Player does not exist.\n\r", ch );
   }
   else
   {
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "Unknown error #%d - %s.  Report to Thoric.\n\r", errno, strerror( errno ) );
      sprintf( buf, "%s destroying %s", ch->name, arg );
      perror( buf );
   }
   return;
}

/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *name_expand( CHAR_DATA * ch )
{
   int count = 1;
   CHAR_DATA *rch;
   char name[MAX_INPUT_LENGTH];  /*  HOPEFULLY no mob has a name longer than THAT */

   static char outbuf[MAX_INPUT_LENGTH];

   if( !IS_NPC( ch ) )
      return ch->name;

   one_argument( ch->name, name );  /* copy the first word into name */

   if( !name[0] ) /* weird mob .. no keywords */
   {
      strcpy( outbuf, "" );   /* Do not return NULL, just an empty buffer */
      return outbuf;
   }

   /*
    * ->people changed to ->first_person -- TRI 
    */
   for( rch = ch->in_room->first_person; rch && ( rch != ch ); rch = rch->next_in_room )
      if( is_name( name, rch->name ) )
         count++;


   sprintf( outbuf, "%d.%s", count, name );
   return outbuf;
}


void do_for( CHAR_DATA * ch, char *argument )
{
   char range[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
   ROOM_INDEX_DATA *room, *old_room;
   CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
   int i;

   argument = one_argument( argument, range );

   if( !range[0] || !argument[0] )  /* invalid usage? */
   {
      do_help( ch, "for" );
      return;
   }

   if( !str_prefix( "quit", argument ) )
   {
      send_to_char( "Are you trying to crash the MUD or something?\n\r", ch );
      return;
   }

   if( !str_prefix( "for", argument ) )
   {
      send_to_char( "Are you trying to crash the MUD or something?\n\r", ch );
      return;
   }


   if( !str_cmp( range, "all" ) )
   {
      fMortals = TRUE;
      fGods = TRUE;
   }
   else if( !str_cmp( range, "gods" ) )
      fGods = TRUE;
   else if( !str_cmp( range, "mortals" ) )
      fMortals = TRUE;
   else if( !str_cmp( range, "mobs" ) )
      fMobs = TRUE;
   else if( !str_cmp( range, "everywhere" ) )
      fEverywhere = TRUE;
   else
      do_help( ch, "for" );   /* show syntax */

   /*
    * do not allow # to make it easier 
    */
   if( fEverywhere && strchr( argument, '#' ) )
   {
      send_to_char( "Cannot use FOR EVERYWHERE with the # thingie.\n\r", ch );
      return;
   }

   if( strchr( argument, '#' ) ) /* replace # ? */
   {
      /*
       * char_list - last_char, p_next - gch_prev -- TRI 
       */
      for( p = last_char; p; p = p_prev )
      {
         p_prev = p->prev; /* TRI */
         /*
          * p_next = p->next; 
          *//*
          * In case someone DOES try to AT MOBS SLAY # 
          */
         found = FALSE;

         if( !( p->in_room ) || room_is_private( p, p->in_room ) || ( p == ch ) )
            continue;

         if( IS_NPC( p ) && fMobs )
            found = TRUE;
         else if( !IS_NPC( p ) && get_trust( p ) >= LEVEL_IMMORTAL && fGods )
            found = TRUE;
         else if( !IS_NPC( p ) && get_trust( p ) < LEVEL_IMMORTAL && fMortals )
            found = TRUE;

         /*
          * It looks ugly to me.. but it works :) 
          */
         if( found ) /* p is 'appropriate' */
         {
            char *pSource = argument;  /* head of buffer to be parsed */
            char *pDest = buf;   /* parse into this */

            while( *pSource )
            {
               if( *pSource == '#' )   /* Replace # with name of target */
               {
                  const char *namebuf = name_expand( p );

                  if( namebuf )  /* in case there is no mob name ?? */
                     while( *namebuf ) /* copy name over */
                        *( pDest++ ) = *( namebuf++ );

                  pSource++;
               }
               else
                  *( pDest++ ) = *( pSource++ );
            }  /* while */
            *pDest = '\0'; /* Terminate */

            /*
             * Execute 
             */
            old_room = ch->in_room;
            char_from_room( ch );
            char_to_room( ch, p->in_room );
            interpret( ch, buf );
            char_from_room( ch );
            char_to_room( ch, old_room );

         }  /* if found */
      }  /* for every char */
   }
   else  /* just for every room with the appropriate people in it */
   {
      for( i = 0; i < MAX_KEY_HASH; i++ ) /* run through all the buckets */
         for( room = room_index_hash[i]; room; room = room->next )
         {
            found = FALSE;

            /*
             * Anyone in here at all? 
             */
            if( fEverywhere ) /* Everywhere executes always */
               found = TRUE;
            else if( !room->first_person )   /* Skip it if room is empty */
               continue;
            /*
             * ->people changed to first_person -- TRI 
             */

            /*
             * Check if there is anyone here of the requried type 
             */
            /*
             * Stop as soon as a match is found or there are no more ppl in room 
             */
            /*
             * ->people to ->first_person -- TRI 
             */
            for( p = room->first_person; p && !found; p = p->next_in_room )
            {

               if( p == ch )  /* do not execute on oneself */
                  continue;

               if( IS_NPC( p ) && fMobs )
                  found = TRUE;
               else if( !IS_NPC( p ) && ( get_trust( p ) >= LEVEL_IMMORTAL ) && fGods )
                  found = TRUE;
               else if( !IS_NPC( p ) && ( get_trust( p ) <= LEVEL_IMMORTAL ) && fMortals )
                  found = TRUE;
            }  /* for everyone inside the room */

            if( found && !room_is_private( p, room ) )   /* Any of the required type here AND room not private? */
            {
               /*
                * This may be ineffective. Consider moving character out of old_room
                * once at beginning of command then moving back at the end.
                * This however, is more safe?
                */

               old_room = ch->in_room;
               char_from_room( ch );
               char_to_room( ch, room );
               interpret( ch, argument );
               char_from_room( ch );
               char_to_room( ch, old_room );
            }  /* if found */
         }  /* for every room in a bucket */
   }  /* if strchr */
}  /* do_for */

void save_sysdata args( ( SYSTEM_DATA sys ) );

void do_cset( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];
   short level;

   set_char_color( AT_IMMORT, ch );

   if( argument[0] == '\0' )
   {
      ch_printf( ch, "Mail:\n\r  Read all mail: %d. Read mail for free: %d. Write mail for free: %d.\n\r",
                 sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
      ch_printf( ch, "  Take all mail: %d.\n\r", sysdata.take_others_mail );
      ch_printf( ch, "Channels:\n\r  Muse: %d. Think: %d. Log: %d. Build: %d.\n\r",
                 sysdata.muse_level, sysdata.think_level, sysdata.log_level, sysdata.build_level );
      ch_printf( ch, "Building:\n\r  Prototype modification: %d.  Player msetting: %d.\n\r",
                 sysdata.level_modify_proto, sysdata.level_mset_player );
      ch_printf( ch, "Guilds:\n\r  Overseer: %s.  Advisor: %s.\n\r", sysdata.guild_overseer, sysdata.guild_advisor );
      ch_printf( ch, "Other:\n\r  Force on players: %d.  ", sysdata.level_forcepc );
      ch_printf( ch, "Private room override: %d.\n\r", sysdata.level_override_private );
      ch_printf( ch, "  Penalty to regular stun chance: %d.  ", sysdata.stun_regular );
      ch_printf( ch, "Penalty to stun plr vs. plr: %d.\n\r", sysdata.stun_plr_vs_plr );
      ch_printf( ch, "  Percent damage plr vs. plr: %3d.  ", sysdata.dam_plr_vs_plr );
      ch_printf( ch, "Percent damage plr vs. mob: %d.\n\r", sysdata.dam_plr_vs_mob );
      ch_printf( ch, "  Percent damage mob vs. plr: %3d.  ", sysdata.dam_mob_vs_plr );
      ch_printf( ch, "Percent damage mob vs. mob: %d.\n\r", sysdata.dam_mob_vs_mob );
      ch_printf( ch, "  Get object without take flag: %d.  ", sysdata.level_getobjnotake );
      ch_printf( ch, "Autosave frequency (minutes): %d.\n\r", sysdata.save_frequency );
      ch_printf( ch, "\n\r&wPfile autocleanup status: &W%s  &wDays before purging newbies: &W%d\n\r",
                 sysdata.CLEANPFILES ? "On" : "Off", sysdata.newbie_purge );
      ch_printf( ch, "&wDays before purging regular players: &W%d\n\r", sysdata.regular_purge );
      ch_printf( ch, "  Save flags: %s\n\r", flag_string( sysdata.save_flags, save_flag ) );
      return;
   }

   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "help" ) )
   {
      do_help( ch, "controls" );
      return;
   }

   if( !str_cmp( arg, "pfiles" ) )
   {

      sysdata.CLEANPFILES = !sysdata.CLEANPFILES;

      if( sysdata.CLEANPFILES )
         send_to_char( "Pfile autocleanup enabled.\n\r", ch );
      else
         send_to_char( "Pfile autocleanup disabled.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "save" ) )
   {
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "saveflag" ) )
   {
      int x = get_saveflag( argument );

      if( x == -1 )
         send_to_char( "Not a save flag.\n\r", ch );
      else
      {
         TOGGLE_BIT( sysdata.save_flags, 1 << x );
         send_to_char( "Ok.\n\r", ch );
         save_sysdata( sysdata );
      }
      return;
   }

   if( !str_prefix( arg, "guild_overseer" ) )
   {
      DISPOSE( sysdata.guild_overseer );
      sysdata.guild_overseer = str_dup( argument );
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_prefix( arg, "guild_advisor" ) )
   {
      DISPOSE( sysdata.guild_advisor );
      sysdata.guild_advisor = str_dup( argument );
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   level = ( short )atoi( argument );

   if( !str_prefix( arg, "savefrequency" ) )
   {
      sysdata.save_frequency = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "newbie_purge" ) )
   {
      if( level < 1 )
      {
         send_to_char( "You must specify a period of at least 1 day.\n\r", ch );
         return;
      }

      sysdata.newbie_purge = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "regular_purge" ) )
   {
      if( level < 1 )
      {
         send_to_char( "You must specify a period of at least 1 day.\n\r", ch );
         return;
      }

      sysdata.regular_purge = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "stun" ) )
   {
      sysdata.stun_regular = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "stun_pvp" ) )
   {
      sysdata.stun_plr_vs_plr = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_pvp" ) )
   {
      sysdata.dam_plr_vs_plr = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "get_notake" ) )
   {
      sysdata.level_getobjnotake = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_pvm" ) )
   {
      sysdata.dam_plr_vs_mob = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_mvp" ) )
   {
      sysdata.dam_mob_vs_plr = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_mvm" ) )
   {
      sysdata.dam_mob_vs_mob = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( level < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Invalid value for new control.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "read_all" ) )
   {
      sysdata.read_all_mail = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "read_free" ) )
   {
      sysdata.read_mail_free = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "write_free" ) )
   {
      sysdata.write_mail_free = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "take_all" ) )
   {
      sysdata.take_others_mail = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "muse" ) )
   {
      sysdata.muse_level = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "think" ) )
   {
      sysdata.think_level = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "log" ) )
   {
      sysdata.log_level = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "build" ) )
   {
      sysdata.build_level = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "proto_modify" ) )
   {
      sysdata.level_modify_proto = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "override_private" ) )
   {
      sysdata.level_override_private = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "forcepc" ) )
   {
      sysdata.level_forcepc = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "mset_player" ) )
   {
      sysdata.level_mset_player = level;
      send_to_char( "Ok.\n\r", ch );
      save_sysdata( sysdata );
      return;
   }
   else
   {
      send_to_char( "Invalid argument.\n\r", ch );
      return;
   }
}

void get_reboot_string( void )
{
   sprintf( reboot_time, "%s", asctime( new_boot_time ) );
}


void do_orange( CHAR_DATA * ch, char *argument )
{
   send_to_char( "Function under construction.\n\r", ch );
   return;
}

void do_mrange( CHAR_DATA * ch, char *argument )
{
   send_to_char( "Function under construction.\n\r", ch );
   return;
}

void do_hell( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   short htime;
   bool h_d = FALSE;
   struct tm *tms;

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Hell who, and for how long?\n\r", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "There is no point in helling an immortal.\n\r", ch );
      return;
   }
   if( victim->pcdata->release_date != 0 )
   {
      ch_printf( ch, "They are already in hell until %24.24s, by %s.\n\r",
                 ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
      return;
   }
   argument = one_argument( argument, arg );
   if( !*arg || !is_number( arg ) )
   {
      send_to_char( "Hell them for how long?\n\r", ch );
      return;
   }
   htime = atoi( arg );
   if( htime <= 0 )
   {
      send_to_char( "You cannot hell for zero or negative time.\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg );
   if( !*arg || !str_prefix( arg, "hours" ) )
      h_d = TRUE;
   else if( str_prefix( arg, "days" ) )
   {
      send_to_char( "Is that value in hours or days?\n\r", ch );
      return;
   }
   else if( htime > 30 )
   {
      send_to_char( "You may not hell a person for more than 30 days at a time.\n\r", ch );
      return;
   }
   tms = localtime( &current_time );
   if( h_d )
      tms->tm_hour += htime;
   else
      tms->tm_mday += htime;
   victim->pcdata->release_date = mktime( tms );
   victim->pcdata->helled_by = STRALLOC( ch->name );
   ch_printf( ch, "%s will be released from hell at %24.24s.\n\r", victim->name, ctime( &victim->pcdata->release_date ) );
   act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   char_to_room( victim, get_room_index( 6 ) );
   act( AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT );
   do_look( victim, "auto" );
   ch_printf( victim, "The immortals are not pleased with your actions.\n\r"
              "You shall remain in hell for %d %s%s.\n\r", htime, ( h_d ? "hour" : "day" ), ( htime == 1 ? "" : "s" ) );
   save_char_obj( victim );   /* used to save ch, fixed by Thoric 09/17/96 */
   return;
}

void do_unhell( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Unhell whom..?\n\r", ch );
      return;
   }
   location = ch->in_room;
   ch->in_room = get_room_index( 6 );
   victim = get_char_room( ch, arg );
   ch->in_room = location; /* The case of unhell self, etc. */
   if( !victim || IS_NPC( victim ) || victim->in_room->vnum != 6 )
   {
      send_to_char( "No one like that is in hell.\n\r", ch );
      return;
   }
   location = get_room_index( wherehome( victim ) );
   if( !location )
      location = ch->in_room;
   MOBtrigger = FALSE;
   act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   char_to_room( victim, location );
   send_to_char( "The gods have smiled on you and released you from hell early!\n\r", victim );
   do_look( victim, "auto" );
   send_to_char( "They have been released.\n\r", ch );

   if( victim->pcdata->helled_by )
   {
      if( str_cmp( ch->name, victim->pcdata->helled_by ) )
         ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\n\r",
                    victim->pcdata->helled_by );
      STRFREE( victim->pcdata->helled_by );
      victim->pcdata->helled_by = NULL;
   }

   MOBtrigger = FALSE;
   act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   victim->pcdata->release_date = 0;
   save_char_obj( victim );
   return;
}

/* Vnum search command by Swordbearer */
void do_vsearch( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   bool found = FALSE;
   OBJ_DATA *obj;
   OBJ_DATA *in_obj;
   int obj_counter = 1;
   int argi;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  vsearch <vnum>.\n\r", ch );
      return;
   }

   set_pager_color( AT_PLAIN, ch );
   argi = atoi( arg );
   argi = atoi( arg );
   if( argi < 1 || argi > 32767 )
   {
      send_to_char( "Vnum out of range.\n\r", ch );
      return;
   }
   for( obj = first_object; obj != NULL; obj = obj->next )
   {
      if( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ) )
         continue;

      found = TRUE;
      for( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

      if( in_obj->carried_by != NULL )
         pager_printf( ch, "[%2d] Level %d %s carried by %s.\n\r",
                       obj_counter, obj->level, obj_short( obj ), PERS( in_obj->carried_by, ch ) );
      else
         pager_printf( ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
                       ( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
                       obj_short( obj ), ( in_obj->in_room == NULL ) ? "somewhere" : in_obj->in_room->name );

      obj_counter++;
   }

   if( !found )
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

   return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
void do_sober( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on mobs.\n\r", ch );
      return;
   }

   if( victim->pcdata )
      victim->pcdata->condition[COND_DRUNK] = 0;
   send_to_char( "Ok.\n\r", ch );
   send_to_char( "You feel sober again.\n\r", victim );
   return;
}


/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE * social )
{
   if( social->name )
      DISPOSE( social->name );
   if( social->char_no_arg )
      DISPOSE( social->char_no_arg );
   if( social->others_no_arg )
      DISPOSE( social->others_no_arg );
   if( social->char_found )
      DISPOSE( social->char_found );
   if( social->others_found )
      DISPOSE( social->others_found );
   if( social->vict_found )
      DISPOSE( social->vict_found );
   if( social->char_auto )
      DISPOSE( social->char_auto );
   if( social->others_auto )
      DISPOSE( social->others_auto );
   DISPOSE( social );
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social( SOCIALTYPE * social )
{
   SOCIALTYPE *tmp, *tmp_next;
   int hash;

   if( !social )
   {
      bug( "Unlink_social: NULL social", 0 );
      return;
   }

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( social == ( tmp = social_index[hash] ) )
   {
      social_index[hash] = tmp->next;
      return;
   }
   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;
      if( social == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE * social )
{
   int hash, x;
   SOCIALTYPE *tmp, *prev;

   if( !social )
   {
      bug( "Add_social: NULL social", 0 );
      return;
   }

   if( !social->name )
   {
      bug( "Add_social: NULL social->name", 0 );
      return;
   }

   if( !social->char_no_arg )
   {
      bug( "Add_social: NULL social->char_no_arg", 0 );
      return;
   }

   /*
    * make sure the name is all lowercase 
    */
   for( x = 0; social->name[x] != '\0'; x++ )
      social->name[x] = LOWER( social->name[x] );

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( ( prev = tmp = social_index[hash] ) == NULL )
   {
      social->next = social_index[hash];
      social_index[hash] = social;
      return;
   }

   for( ; tmp; tmp = tmp->next )
   {
      if( ( x = strcmp( social->name, tmp->name ) ) == 0 )
      {
         bug( "Add_social: trying to add duplicate name to bucket %d", hash );
         free_social( social );
         return;
      }
      else if( x < 0 )
      {
         if( tmp == social_index[hash] )
         {
            social->next = social_index[hash];
            social_index[hash] = social;
            return;
         }
         prev->next = social;
         social->next = tmp;
         return;
      }
      prev = tmp;
   }

   /*
    * add to end 
    */
   prev->next = social;
   social->next = NULL;
   return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit( CHAR_DATA * ch, char *argument )
{
   SOCIALTYPE *social;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_char_color( AT_SOCIAL, ch );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: sedit <social> [field]\n\r", ch );
      send_to_char( "Syntax: sedit <social> create\n\r", ch );
      if( get_trust( ch ) > LEVEL_GOD )
         send_to_char( "Syntax: sedit <social> delete\n\r", ch );
      if( get_trust( ch ) > LEVEL_LESSER )
         send_to_char( "Syntax: sedit <save>\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\n\r", ch );
      return;
   }

   if( get_trust( ch ) > LEVEL_LESSER && !str_cmp( arg1, "save" ) )
   {
      save_socials(  );
      send_to_char( "Saved.\n\r", ch );
      return;
   }

   social = find_social( arg1 );

   if( !str_cmp( arg2, "create" ) )
   {
      if( social )
      {
         send_to_char( "That social already exists!\n\r", ch );
         return;
      }
      CREATE( social, SOCIALTYPE, 1 );
      social->name = str_dup( arg1 );
      sprintf( arg2, "You %s.", arg1 );
      social->char_no_arg = str_dup( arg2 );
      add_social( social );
      send_to_char( "Social added.\n\r", ch );
      return;
   }

   if( !social )
   {
      send_to_char( "Social not found.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Social: %s\n\r\n\rCNoArg: %s\n\r", social->name, social->char_no_arg );
      ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
                 social->others_no_arg ? social->others_no_arg : "(not set)",
                 social->char_found ? social->char_found : "(not set)",
                 social->others_found ? social->others_found : "(not set)" );
      ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
                 social->vict_found ? social->vict_found : "(not set)",
                 social->char_auto ? social->char_auto : "(not set)",
                 social->others_auto ? social->others_auto : "(not set)" );
      return;
   }

   if( get_trust( ch ) > LEVEL_GOD && !str_cmp( arg2, "delete" ) )
   {
      unlink_social( social );
      free_social( social );
      send_to_char( "Deleted.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "cnoarg" ) )
   {
      if( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
      {
         send_to_char( "You cannot clear this field.  It must have a message.\n\r", ch );
         return;
      }
      if( social->char_no_arg )
         DISPOSE( social->char_no_arg );
      social->char_no_arg = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "onoarg" ) )
   {
      if( social->others_no_arg )
         DISPOSE( social->others_no_arg );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_no_arg = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "cfound" ) )
   {
      if( social->char_found )
         DISPOSE( social->char_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_found = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "ofound" ) )
   {
      if( social->others_found )
         DISPOSE( social->others_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_found = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "vfound" ) )
   {
      if( social->vict_found )
         DISPOSE( social->vict_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->vict_found = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "cauto" ) )
   {
      if( social->char_auto )
         DISPOSE( social->char_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_auto = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "oauto" ) )
   {
      if( social->others_auto )
         DISPOSE( social->others_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_auto = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      SOCIALTYPE *checksocial;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\n\r", ch );
         return;
      }
      if( ( checksocial = find_social( arg1 ) ) != NULL )
      {
         ch_printf( ch, "There is already a social named %s.\n\r", arg1 );
         return;
      }
      if( arg1[0] != social->name[0] )
      {
         unlink_social( social );
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if( social->name )
         DISPOSE( social->name );
      social->name = str_dup( arg1 );
      if( relocate )
         add_social( social );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   /*
    * display usage message 
    */
   do_sedit( ch, "" );
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE * command )
{
   if( command->name )
      DISPOSE( command->name );
   DISPOSE( command );
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE * command )
{
   CMDTYPE *tmp, *tmp_next;
   int hash;

   if( !command )
   {
      bug( "Unlink_command NULL command", 0 );
      return;
   }

   hash = command->name[0] % 126;

   if( command == ( tmp = command_hash[hash] ) )
   {
      command_hash[hash] = tmp->next;
      return;
   }
   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;
      if( command == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE * command )
{
   int hash, x;
   CMDTYPE *tmp, *prev;

   if( !command )
   {
      bug( "Add_command: NULL command", 0 );
      return;
   }

   if( !command->name )
   {
      bug( "Add_command: NULL command->name", 0 );
      return;
   }

   if( !command->do_fun )
   {
      bug( "Add_command: NULL command->do_fun", 0 );
      return;
   }

   /*
    * make sure the name is all lowercase 
    */
   for( x = 0; command->name[x] != '\0'; x++ )
      command->name[x] = LOWER( command->name[x] );

   hash = command->name[0] % 126;

   if( ( prev = tmp = command_hash[hash] ) == NULL )
   {
      command->next = command_hash[hash];
      command_hash[hash] = command;
      return;
   }

   /*
    * add to the END of the list 
    */
   for( ; tmp; tmp = tmp->next )
      if( !tmp->next )
      {
         tmp->next = command;
         command->next = NULL;
      }
   return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 */
void do_cedit( CHAR_DATA * ch, char *argument )
{
   CMDTYPE *command;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_char_color( AT_IMMORT, ch );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: cedit save cmdtable\n\r", ch );
      if( get_trust( ch ) > LEVEL_SUB_IMPLEM )
      {
         send_to_char( "Syntax: cedit <command> create [code]\n\r", ch );
         send_to_char( "Syntax: cedit <command> delete\n\r", ch );
         send_to_char( "Syntax: cedit <command> show\n\r", ch );
         send_to_char( "Syntax: cedit <command> [field]\n\r", ch );
         send_to_char( "\n\rField being one of:\n\r", ch );
         send_to_char( "  level position log code\n\r", ch );
      }
      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg1, "save" ) && !str_cmp( arg2, "cmdtable" ) )
   {
      save_commands(  );
      send_to_char( "Saved.\n\r", ch );
      return;
   }

   command = find_command( arg1 );

   if( get_trust( ch ) > LEVEL_SUB_IMPLEM && !str_cmp( arg2, "create" ) )
   {
      if( command )
      {
         send_to_char( "That command already exists!\n\r", ch );
         return;
      }
      CREATE( command, CMDTYPE, 1 );
      command->name = str_dup( arg1 );
      command->level = get_trust( ch );
      if( *argument )
         one_argument( argument, arg2 );
      else
         sprintf( arg2, "do_%s", arg1 );
      command->do_fun = skill_function( arg2 );
      add_command( command );
      send_to_char( "Command added.\n\r", ch );
      if( command->do_fun == skill_notfound )
         ch_printf( ch, "Code %s not found.  Set to no code.\n\r", arg2 );
      return;
   }

   if( !command )
   {
      send_to_char( "Command not found.\n\r", ch );
      return;
   }
   else if( command->level > get_trust( ch ) )
   {
      send_to_char( "You cannot touch this command.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rCode:     %s\n\r",
                 command->name, command->level, command->position, command->log, skill_name( command->do_fun ) );
      if( command->userec.num_uses )
         send_timer( &command->userec, ch );
      return;
   }

   if( get_trust( ch ) <= LEVEL_SUB_IMPLEM )
   {
      do_cedit( ch, "" );
      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      unlink_command( command );
      free_command( command );
      send_to_char( "Deleted.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "code" ) )
   {
      DO_FUN *fun = skill_function( argument );

      if( fun == skill_notfound )
      {
         send_to_char( "Code not found.\n\r", ch );
         return;
      }
      command->do_fun = fun;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "level" ) )
   {
      int level = atoi( argument );

      if( level < 0 || level > get_trust( ch ) )
      {
         send_to_char( "Level out of range.\n\r", ch );
         return;
      }
      command->level = level;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "log" ) )
   {
      int clog = atoi( argument );

      if( clog < 0 || clog > LOG_COMM )
      {
         send_to_char( "Log out of range.\n\r", ch );
         return;
      }
      command->log = clog;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "position" ) )
   {
      int position = atoi( argument );

      if( position < 0 || position > POS_DRAG )
      {
         send_to_char( "Position out of range.\n\r", ch );
         return;
      }
      command->position = position;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      CMDTYPE *checkcmd;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\n\r", ch );
         return;
      }
      if( ( checkcmd = find_command( arg1 ) ) != NULL )
      {
         ch_printf( ch, "There is already a command named %s.\n\r", arg1 );
         return;
      }
      if( arg1[0] != command->name[0] )
      {
         unlink_command( command );
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if( command->name )
         DISPOSE( command->name );
      command->name = str_dup( arg1 );
      if( relocate )
         add_command( command );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   /*
    * display usage message 
    */
   do_cedit( ch, "" );
}

/* Pfile Restore by Tawnos */
void do_restorefile( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char fname[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   struct stat fst;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Restore what player file?\n\r", ch );
      return;
   }

   sprintf( fname, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), capitalize( arg ) );
   if( stat( fname, &fst ) != -1 )
   {
      sprintf( buf2, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), capitalize( arg ) );
      sprintf( buf, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), capitalize( arg ) );
      rename( buf, buf2 );
      send_to_char( "Player data restored.\n\r", ch );
      return;
   }
   send_to_char( "No Such Backup File.\n\r", ch );
}

void do_fslay( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );
   one_argument( argument, arg2 );
   if( arg[0] == '\0' )
   {
      send_to_char( "FSlay whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "You can't fool yourself, you know.\n\r", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char
         ( "If they have any sort of mental capacity, they would realize you would be unable to slay them anyways. So why don't you just stop what you're doing and go play with some Duplo blocks, before we all succumb to complete and utter mindlessness from your total lack of intelligence.\n\r",
           ch );
      return;
   }

   if( !str_cmp( arg2, "immolate" ) )
   {
      act( AT_FIRE, "Your fireball turns $N into a blazing inferno.", ch, NULL, victim, TO_CHAR );
      act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT );
      act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.", ch, NULL, victim, TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "shatter" ) )
   {
      act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.", ch, NULL, victim,
           TO_CHAR );
      act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim,
           TO_VICT );
      act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.", ch, NULL, victim,
           TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "demon" ) )
   {
      act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the", ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,
           TO_CHAR );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on", ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.", ch, NULL, victim,
           TO_VICT );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the", ch, NULL, victim,
           TO_NOTVICT );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,
           TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "pounce" ) )
   {
      act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...", ch,
           NULL, victim, TO_CHAR );
      act( AT_BLOOD,
           "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...",
           ch, NULL, victim, TO_VICT );
      act( AT_BLOOD,
           "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",
           ch, NULL, victim, TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "slit" ) )
   {
      act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
   }

   else
   {
      act( AT_IMMORT, "You slay $N in cold blood!", ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT );
   }

   set_cur_char( victim );
   set_char_color( AT_DIEMSG, victim );
   do_help( victim, "_DIEMSG_" );
   for( d = first_descriptor; d; d = d->next )
   {
      if( d == victim->desc )
      {
         close_socket( d, FALSE );
         send_to_char( "Ok.\n\r", ch );
         return;
      }
   }

   return;
}

void ostat_plus( CHAR_DATA * ch, OBJ_DATA * obj )
{
   SKILLTYPE *sktmp;
   int dam;
   char buf[MAX_STRING_LENGTH];
   int x;

/*****
 * Should Never reach these, but they are here incase...
 *****/

   if( !ch )
   {
      bug( "NULL ch in ostat_plus 	File: %s 	Line: %d", __FILE__, __LINE__ );
      return;
   }
   if( !obj )
   {
      bug( "NULL obj in ostat_plus File: %s	Line: %d", __FILE__, __LINE__ );
      return;
   }

   /******
	* A more informative ostat, so You actually know what those obj->value[x] mean
	* without looking in the code for it. Combines parts of look, examine, the
	* identification spell, and things that were never seen.. Probably overkill
	* on most things, but I'm lazy and hate digging through code to see what
	* value[x] means... -Druid
	******/
   ch_printf( ch, "&cAdditional Object information\n\r" );
   switch ( obj->item_type )
   {
      default:
         ch_printf( ch, "&cSorry, No additional information available.\n\r" );
         break;
      case ITEM_LIGHT:
         ch_printf( ch, "&GValue[&W2&G] Hours left: &W" );
         if( obj->value[2] >= 0 )
            ch_printf( ch, "%d\n\r", obj->value[2] );
         else
            ch_printf( ch, "Infinite\n\r" );
         break;
      case ITEM_POTION:
      case ITEM_PILL:
      case ITEM_SCROLL:
         ch_printf( ch, "&GValue[&W0&G] Spell Level: &W%d\n\r", obj->value[0] );
         for( x = 1; x <= 3; x++ )
         {
            if( obj->value[x] >= 0 && ( sktmp = get_skilltype( obj->value[x] ) ) != NULL )
               ch_printf( ch, "&GValue[&W%d&G] Spell (&W%d&c): &W%s\n\r", x, obj->value[x], sktmp->name );
            else
               ch_printf( ch, "&GValue[&W%d&G] Spell: &WNone\n\r", x );
         }
         if( obj->item_type == ITEM_PILL )
            ch_printf( ch, "&GValue[&W4&G] Food Value: &W%d\n\r", obj->value[4] );
         break;
      case ITEM_SALVE:
      case ITEM_WAND:
      case ITEM_STAFF:
         ch_printf( ch, "&GValue[&W0&G] Spell Level: &W%d\n\r", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Max Charges: &W%d\n\r", obj->value[1] );
         ch_printf( ch, "&GValue[&W2&G] Charges Remaining: &W%d\n\r", obj->value[2] );
         if( obj->item_type != ITEM_SALVE )
         {
            if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
               ch_printf( ch, "&GValue[&W3&G] Spell (&W%d&c): &W%s\n\r", obj->value[3], sktmp->name );
            else
               ch_printf( ch, "&GValue[&W3&G] Spell: &WNone\n\r" );
            break;
         }
         ch_printf( ch, "&GValue[&W3&G] Delay (beats): &W%d\n\r", obj->value[3] );
         for( x = 4; x <= 5; x++ )
         {
            if( obj->value[x] >= 0 && ( sktmp = get_skilltype( obj->value[x] ) ) != NULL )
               ch_printf( ch, "&GValue[&W%d&G] Spell (&W%d&c): &W%s\n\r", x, obj->value[x], sktmp->name );
            else
               ch_printf( ch, "&GValue[&W%d&G] Spell: &WNone\n\r", x );
         }
         break;
      case ITEM_WEAPON:
         ch_printf( ch, "&GValue[&W0&G] Condition:   &W%d\n\r", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Num Dice:    &W%d\n\r", obj->value[1] );
         ch_printf( ch, "&GValue[&W2&G] Size Dice:   &W%d\n\r", obj->value[2] );
         ch_printf( ch, "&GValue[&W3&G] Weapon Type: " );
         if( obj->value[3] == 0 )
            ch_printf( ch, "&WGeneral&W\n\r" );
         else if( obj->value[3] == WEAPON_VIBRO_BLADE )
            ch_printf( ch, "&WVibro-Blade&W\n\r" );
         else if( obj->value[3] == WEAPON_BOWCASTER )
            ch_printf( ch, "&WBowcaster&W\n\r" );
         else if( obj->value[3] == WEAPON_FORCE_PIKE )
            ch_printf( ch, "&WForce-Pike&W\n\r" );
         else if( obj->value[3] == WEAPON_BLASTER )
            ch_printf( ch, "&WBlaster&W\n\r" );
         else if( obj->value[3] == WEAPON_LIGHTSABER || obj->value[3] == WEAPON_DUAL_LIGHTSABER )
            ch_printf( ch, "&WLightsaber&W\n\r" );
         else
            ch_printf( ch, "&WNo Current Weapon Type Set&W\n\r" );

         ch_printf( ch, "&GValue[&W-&G] Low Damage:  &W%d\n\r", obj->value[1] );
         ch_printf( ch, "&GValue[&W-&G] Max Damage:  &W%d\n\r", obj->value[2] );
         ch_printf( ch, "&GValue[&W-&G] Ave Damage:  &W%d\n\r", ( obj->value[1] + obj->value[2] ) / 2 );
         ch_printf( ch, "&GValue[&W4&G] Charges:     &W%d\n\r", obj->value[4] );
         ch_printf( ch, "&GValue[&W5&G] Max Charges: &W%d\n\r", obj->value[5] );
         break;
      case ITEM_ARMOR:
         ch_printf( ch, "&GValue[&W0&G] Current AC: &W%d\n\r", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Original AC: &W%d\n\r", obj->value[1] );
         if( obj->value[1] == 0 )
            dam = 10;
         else
            dam = ( short )( ( obj->value[0] * 10 ) / obj->value[1] );
         ch_printf( ch, "&cCondition (&W%d&c): &W", dam );
         /*****
	   	 * Copied from act_info do_examine
	   	 * Could possibly make a function returning the string....
	   	 *****/
         if( dam >= 10 )
            strcpy( buf, "Superb condition" );
         else if( dam == 9 )
            strcpy( buf, "Very good condition" );
         else if( dam == 8 )
            strcpy( buf, "Good shape" );
         else if( dam == 7 )
            strcpy( buf, "Showing a bit of wear" );
         else if( dam == 6 )
            strcpy( buf, "A little run down" );
         else if( dam == 5 )
            strcpy( buf, "In need of repair" );
         else if( dam == 4 )
            strcpy( buf, "In great need of repair" );
         else if( dam == 3 )
            strcpy( buf, "In dire need of repair" );
         else if( dam == 2 )
            strcpy( buf, "Very badly worn" );
         else if( dam == 1 )
            strcpy( buf, "Practically worthless" );
         else if( dam <= 0 )
            strcpy( buf, "Broken" );
         strcat( buf, "\n\r" );
         send_to_char( buf, ch );
         break;
         /*
          * Bug Fix 7/9/00 -Druid
          */
      case ITEM_FOOD:
         ch_printf( ch, "&GValue[&W0&G] Food Value: &W%d\n\r", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Condition (&W%d&c): &W", obj->value[1] );
         if( obj->timer > 0 && obj->value[1] > 0 )
            dam = ( obj->timer * 10 ) / obj->value[1];
         else
            dam = 10;
         if( dam >= 10 )
            strcpy( buf, "It is fresh." );
         else if( dam == 9 )
            strcpy( buf, "It is nearly fresh." );
         else if( dam == 8 )
            strcpy( buf, "It is perfectly fine." );
         else if( dam == 7 )
            strcpy( buf, "It looks good." );
         else if( dam == 6 )
            strcpy( buf, "It looks ok." );
         else if( dam == 5 )
            strcpy( buf, "It is a little stale." );
         else if( dam == 4 )
            strcpy( buf, "It is a bit stale." );
         else if( dam == 3 )
            strcpy( buf, "It smells slightly off." );
         else if( dam == 2 )
            strcpy( buf, "It smells quite rank." );
         else if( dam == 1 )
            strcpy( buf, "It smells revolting!" );
         else if( dam <= 0 )
            strcpy( buf, "It is crawling with maggots!" );
         strcat( buf, "\n\r" );
         send_to_char( buf, ch );
         if( obj->value[4] )
            ch_printf( ch, "&GValue[&W4&G] Timer: &W%d\n\r", obj->value[4] );
         break;
      case ITEM_DRINK_CON:
         ch_printf( ch, "&GValue[&W0&G] Capacity: &W%d\n\r", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Quantity Left (&W%d&c): &W", obj->value[1] );
         if( obj->value[1] > obj->value[0] )
            ch_printf( ch, "More than Full\n\r" );
         else if( obj->value[1] == obj->value[0] )
            ch_printf( ch, "Full\n\r" );
         else if( obj->value[1] >= ( 3 * obj->value[0] / 4 ) )
            ch_printf( ch, "Almost Full\n\r" );
         else if( obj->value[1] > ( obj->value[0] / 2 ) )
            ch_printf( ch, "More than half full\n\r" );
         else if( obj->value[1] == ( obj->value[0] / 2 ) )
            ch_printf( ch, "Half full\n\r" );
         else if( obj->value[1] >= ( obj->value[0] / 4 ) )
            ch_printf( ch, "Less than half full\n\r" );
         else if( obj->value[1] >= 1 )
            ch_printf( ch, "Almost Empty\n\r" );
         else
            ch_printf( ch, "Empty\n\r" );
         ch_printf( ch, "&GValue[&W2&G] Liquid Type (&W%d&c): &W%s\n\r", obj->value[2], liq_table[obj->value[2]].liq_name );
         ch_printf( ch, "&cLiquid color: &W%s\n\r", liq_table[obj->value[2]].liq_color );
         if( liq_table[obj->value[2]].liq_affect[COND_DRUNK] != 0 )
            ch_printf( ch, "&cAffects Drunkeness by: &W%d\n\r", liq_table[obj->value[2]].liq_affect[COND_DRUNK] );
         if( liq_table[obj->value[2]].liq_affect[COND_FULL] != 0 )
            ch_printf( ch, "&cAffects Fullness by: &W%d\n\r", liq_table[obj->value[2]].liq_affect[COND_FULL] );
         if( liq_table[obj->value[2]].liq_affect[COND_THIRST] != 0 )
            ch_printf( ch, "&cAffects Thirst by: &W%d\n\r", liq_table[obj->value[2]].liq_affect[COND_THIRST] );
         if( liq_table[obj->value[2]].liq_affect[COND_BLOODTHIRST] != 0 )
            ch_printf( ch, "&cAffects BloodThirst by: &W%d\n\r", liq_table[obj->value[2]].liq_affect[COND_BLOODTHIRST] );
         ch_printf( ch, "&GValue[&W3&G] Poisoned (&W%d&c): &W%s\n\r", obj->value[3], obj->value[3] >= 1 ? "Yes" : "No" );
         break;
      case ITEM_HERB:
         ch_printf( ch, "&GValue[&W1&G] Charges: &W%d\n\r", obj->value[1] );
         ch_printf( ch, "&GValue[&W2&G] Herb #: &W%d\n\r", obj->value[2] );
         break;
      case ITEM_CONTAINER:
         ch_printf( ch, "&GValue[&W0&G] Capacity (&W%d&c): &W", obj->value[0] );
         ch_printf( ch, "%s\n\r",
                    obj->value[0] < 76 ? "Small capacity" :
                    obj->value[0] < 150 ? "Small to medium capacity" :
                    obj->value[0] < 300 ? "Medium capacity" :
                    obj->value[0] < 550 ? "Medium to large capacity" :
                    obj->value[0] < 751 ? "Large capacity" : "Giant capacity" );
         ch_printf( ch, "&GValue[&W1&G] Flags (&W%d&c):&W", obj->value[1] );
         if( obj->value[1] <= 0 )
            ch_printf( ch, " None\n\r" );
         else
         {
            if( IS_SET( obj->value[1], CONT_CLOSEABLE ) )
               ch_printf( ch, " Closeable" );
            if( IS_SET( obj->value[1], CONT_PICKPROOF ) )
               ch_printf( ch, " PickProof" );
            if( IS_SET( obj->value[1], CONT_CLOSED ) )
               ch_printf( ch, " Closed" );
            if( IS_SET( obj->value[1], CONT_LOCKED ) )
               ch_printf( ch, " Locked" );
            ch_printf( ch, "\n\r" );
         }
         ch_printf( ch, "&GValue[&W2&G] Key Vnum: &W" );
         if( obj->value[2] <= 0 )
            ch_printf( ch, "None\n\r" );
         else
            ch_printf( ch, "%d\n\r", obj->value[2] );
         ch_printf( ch, "&GValue[&W3&G] Condition: &W%d\n\r", obj->value[3] );
         if( obj->timer )
            ch_printf( ch, "&cObject Timer, Time Left: &W%d\n\r", obj->timer );
         break;
      case ITEM_MONEY:
         ch_printf( ch, "&GValue[&W0&G] # of Coins: &W%d\n\r", obj->value[0] );
         break;
      case ITEM_FURNITURE:
/*
        if(!IS_SET(obj->value[2],SIT_ON) && !IS_SET(obj->value[2],SIT_AT) && !IS_SET(obj->value[2],SIT_IN))
            ch_printf(ch,"You cannot sit on, at, or in this object.\r\n");
        else
        {
            ch_printf(ch,"You can sit:");
            if(IS_SET(obj->value[2],SIT_ON))
                ch_printf(ch," on,");
            if(IS_SET(obj->value[2],SIT_AT))
                ch_printf(ch," at,");
            if(IS_SET(obj->value[2],SIT_IN))
                ch_printf(ch," in,");
            ch_printf(ch," this object.\r\n");
        }
        if(!IS_SET(obj->value[2],STAND_ON) && !IS_SET(obj->value[2],STAND_AT) && !IS_SET(obj->value[2],STAND_IN))
            ch_printf(ch,"You cannot stand on, at, or in this object.\r\n");
        else
        {
            ch_printf(ch,"You can stand:");
            if(IS_SET(obj->value[2],STAND_ON))
                ch_printf(ch," on,");
            if(IS_SET(obj->value[2],STAND_AT))
                ch_printf(ch," at,");
            if(IS_SET(obj->value[2],STAND_IN))
                ch_printf(ch," in,");
            ch_printf(ch," this object.\r\n");
        }
        if(!IS_SET(obj->value[2],REST_ON) && !IS_SET(obj->value[2],REST_AT) && !IS_SET(obj->value[2],REST_IN))
            ch_printf(ch,"You cannot rest on, at, or in this object.\r\n");
        else
        {
            ch_printf(ch,"You can rest:");
            if(IS_SET(obj->value[2],REST_ON))
                ch_printf(ch," on,");
            if(IS_SET(obj->value[2],REST_AT))
                ch_printf(ch," at,");
            if(IS_SET(obj->value[2],REST_IN))
                ch_printf(ch," in,");
            ch_printf(ch," this object.\r\n");
        }
        if(!IS_SET(obj->value[2],SLEEP_ON) && !IS_SET(obj->value[2],SLEEP_AT) && !IS_SET(obj->value[2],SLEEP_IN))
            ch_printf(ch,"You cannot sleep on, at, or in this object.\r\n");
        else
        {
            ch_printf(ch,"You can sleep:");
            if(IS_SET(obj->value[2],SLEEP_ON))
                ch_printf(ch," on,");
            if(IS_SET(obj->value[2],SLEEP_AT))
                ch_printf(ch," at,");
            if(IS_SET(obj->value[2],SLEEP_IN))
                ch_printf(ch," in,");
            ch_printf(ch," this object.\r\n");
        }
*/
         break;

      case ITEM_TRAP:
         ch_printf( ch, "&GValue[&W0&G] Charges Remaining: &W%d\n\r", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Type (&W%d&c): &W", obj->value[1] );
         switch ( obj->value[1] )
         {
            default:
               sprintf( buf, "Hit by a trap" );
               ch_printf( ch, "Default Generic Trap\n\r" );
               ch_printf( ch, "&cDoes Damage from (&W%d&c) to (&W%d&c)\n\r", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_GAS:
               sprintf( buf, "Surrounded by a green cloud of gas" );
               ch_printf( ch, "Poisoned Gas\n\r" );
               ch_printf( ch, "&cCasts spell: &WPoison\n\r" );
               break;
            case TRAP_TYPE_POISON_DART:
               sprintf( buf, "Hit by a dart" );
               ch_printf( ch, "Poisoned Dart\n\r" );
               ch_printf( ch, "&cCasts spell: &WPoison\n\r" );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\n\r", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_NEEDLE:
               sprintf( buf, "Pricked by a needle" );
               ch_printf( ch, "Poisoned Needle\n\r" );
               ch_printf( ch, "&cCasts spell: &WPoison\n\r" );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\n\r", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_DAGGER:
               sprintf( buf, "Stabbed by a dagger" );
               ch_printf( ch, "Poisoned Dagger\n\r" );
               ch_printf( ch, "&cCasts spell: &WPoison\n\r" );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\n\r", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_ARROW:
               sprintf( buf, "Struck with an arrow" );
               ch_printf( ch, "Poisoned Arrow\n\r" );
               ch_printf( ch, "&cCasts spell: &WPoison\n\r" );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\n\r", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_BLINDNESS_GAS:
               sprintf( buf, "Surrounded by a red cloud of gas" );
               ch_printf( ch, "Blinding Gas\n\r" );
               ch_printf( ch, "&cCasts spell: &WBlind\n\r" );
               break;
            case TRAP_TYPE_SLEEPING_GAS:
               sprintf( buf, "Surrounded by a yellow cloud of gas" );
               ch_printf( ch, "Sleeping Gas\n\r" );
               ch_printf( ch, "&cCasts spell: &WSleep\n\r" );
               break;
            case TRAP_TYPE_FLAME:
               sprintf( buf, "Struck by a burst of flame" );
               ch_printf( ch, "Flame\n\r" );
               ch_printf( ch, "&cCasts spell: &WFireball\n\r" );
               break;
            case TRAP_TYPE_EXPLOSION:
               sprintf( buf, "Hit by an explosion" );
               ch_printf( ch, "Explosion\n\r" );
               ch_printf( ch, "&cCasts spell: &WFireball\n\r" );
               break;
            case TRAP_TYPE_ACID_SPRAY:
               sprintf( buf, "Covered by a spray of acid" );
               ch_printf( ch, "Acid Spray\n\r" );
               ch_printf( ch, "&cCasts spell: &WAcid Blast\n\r" );
               break;
            case TRAP_TYPE_ELECTRIC_SHOCK:
               sprintf( buf, "Suddenly shocked" );
               ch_printf( ch, "Electric Shock\n\r" );
               ch_printf( ch, "&cDoes Damage from (&W%d&c) to (&W%d&c)\n\r", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_BLADE:
               sprintf( buf, "Sliced by a razor sharp blade" );
               ch_printf( ch, "Sharp Blade\n\r" );
               ch_printf( ch, "&cDoes Damage from (&W%d&c) to (&W%d&c)\n\r", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case ITEM_KEY:
               ch_printf( ch, "&GValue[&W0&G] Lock #: &W%d\n\r", obj->value[0] );
               break;
            case ITEM_BLOOD:
               ch_printf( ch, "&GValue[&W1&G] Amount Remaining: &W%d\n\r", obj->value[1] );
               if( obj->timer )
                  ch_printf( ch, "&cObject Timer, Time Left: &W%d\n\r", obj->timer );
               break;

         }
   }
}

void do_reward( CHAR_DATA * ch, char *argument )
{
   int amount;
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Reward who how many points?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give a mob RP points!\n\r", ch );
      return;
   }

   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "Immortals have no use for such things!\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "How many RP points do you wish to give?\n\r", ch );
      return;
   }

   amount = atoi( argument );

   if( amount < -1 )
   {
      send_to_char( "You can only give positive points, or -1.\n\r", ch );
      return;
   }

   if( !victim->rppoints )
      victim->rppoints = amount;
   else
      victim->rppoints += amount;

   if( amount >= 1 )
   {
      ch_printf( victim, "&GCheer! The gods have rewarded you with %d RP point%s!\n\r", amount, amount > 1 ? "s" : "" );
      ch_printf( ch, "&GThey have been rewarded with with %d RP point%s.\n\r", amount, amount > 1 ? "s" : "" );
   }
   else if( amount < 0 )
   {
      ch_printf( victim, "&RCurses! The gods have removed an RP point!\n\r" );
      ch_printf( ch, "They have had one RP point removed.\n\r" );
   }

}

void do_changes( CHAR_DATA * ch, char *argument )
{
   FILE *fpList;
   char *buf;
   char display[MAX_STRING_LENGTH];
   int i = 0;

   if( IS_NPC( ch ) )
      return;

   if( ( fpList = fopen( CHANGE_FILE, "r" ) ) == NULL )
   {
      send_to_char( "Something wen't wrong. The imms have been notified.\n\r", ch );
      bug( "Do_changes: Unable to open change list" );
      return;
   }

   send_to_char( "&g::::::::::::::::::::::::::::::::::&w&W[ &GCHANGES &W]&g::::::::::::::::::::::::::::::::::\n\r", ch );
   send_to_char( "&w&W-------------------------------------------------------------------------------\n\r", ch );

   for( ;; )
   {
      if( feof( fpList ) )
         break;
      buf = feof( fpList ) ? "End" : fread_string( fpList );
      if( !str_cmp( buf, "End" ) || buf[0] == '\0' )
         break;
      if( strlen( buf ) < 3 )
         break;

      sprintf( display, "&w&W[&G*&W] &g%s", buf );
      send_to_char( strlinwrp( display, 78 ), ch );
      send_to_char( "&w&W-------------------------------------------------------------------------------\n\r", ch );
      ++i;

   }
   send_to_char( "\n\r", ch );
   fclose( fpList );
}

void do_addchange( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   if( argument[0] == '\0' )
   {
      send_to_char( "&WSyntax: addchange <change>\n\r", ch );
      return;
   }

   smash_tilde( argument );
   sprintf( buf, "%s~", argument );
   append_to_file( CHANGE_FILE, buf );

   sprintf( buf2, "<li> %s<br><br>", argument );
   prepend_to_file( CHANGEHTML_FILE, buf2 );

   send_to_char( "&GChange successfully added.\n\r", ch );
   return;
}

void do_std( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int desc;
   DESCRIPTOR_DATA *d;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Syntax: std <descriptor> <text>\n\r", ch );
      return;
   }

   desc = atoi( arg );

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->descriptor == desc )
      {
         send_to_desc_color( argument, d );
         send_to_char( "&WOk.\n\r", ch );
         return;
      }
   }

   send_to_char( "No such descriptor\n\r", ch );
   return;

}
