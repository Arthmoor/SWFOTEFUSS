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
#include "sha256.h"

#define RESTORE_INTERVAL 21600

const char *const save_flag[] =
   { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap", "auction", "get", "receive", "idle", "backup", "r13",
   "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30",
   "r31"
};

const char *const cmd_flags[] = {
    "possessed", "action", "mudprog",
    "noforce", "ooc", "build", "player_only", "fullname", "r08", "r09", "r10", "r11", "r12", "r13", "r14", "r15", "r16",
    "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
    "r28", "r29", "r30", "r31"
};

/*
 * For use with cedit --Shaddai
 */
int get_cmdflag( const char *flag )
{
    unsigned int x;

    for( x = 0; x < ( sizeof( cmd_flags ) / sizeof( cmd_flags[0] ) ); x++ )
        if( !str_cmp( flag, cmd_flags[x] ) )
            return x;
    return -1;
}

/* from space.c */
void remship( SHIP_DATA * ship );

/*
 * Local functions.
 */
void save_banlist( void );
void ostat_plus( CHAR_DATA * ch, OBJ_DATA * obj );
int get_color( const char *argument ); /* function proto */

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

int get_saveflag( const char *name )
{
   unsigned int x;

   for( x = 0; x < sizeof( save_flag ) / sizeof( save_flag[0] ); x++ )
      if( !str_cmp( name, save_flag[x] ) )
         return x;
   return -1;
}

void do_wizhelp( CHAR_DATA * ch, const char *argument )
{
   CMDTYPE *cmd;
   int col, hash;
   int curr_lvl;
   col = 0;
   set_pager_color( AT_WHITE, ch );

   for( curr_lvl = LEVEL_AVATAR; curr_lvl <= get_trust( ch ); curr_lvl++ )
   {
      send_to_pager( "\r\n\r\n", ch );
      col = 1;
      pager_printf( ch, "[LEVEL %-2d]  ", curr_lvl );
      for( hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
            if( ( cmd->level == curr_lvl ) && cmd->level <= get_trust( ch ) )
            {
               pager_printf( ch, "%-12s", cmd->name );
               if( ++col % 6 == 0 )
                  send_to_pager( "\r\n", ch );
            }
   }
   if( col % 6 != 0 )
      send_to_pager( "\r\n", ch );
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
		    send_to_pager( "\r\n", ch );
	    }

    if ( col % 6 != 0 )
	send_to_pager( "\r\n", ch );
    return;
}*/

void do_restrict( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Restrict which command?\r\n", ch );
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
         snprintf( buf, MAX_STRING_LENGTH, "%s show", cmd->name );
         do_cedit( ch, buf );
/*    		ch_printf( ch, "%s is at level %d.\r\n", cmd->name, cmd->level );*/
         return;
      }
      cmd->level = level;
      ch_printf( ch, "You restrict %s to level %d\r\n", cmd->name, level );
      log_printf( "%s restricting %s to level %d", ch->name, cmd->name, level );
   }
   else
      send_to_char( "You may not restrict that command.\r\n", ch );

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
            ch_printf( ch, "%s does not uniquely identify a char.\r\n", name );
            return NULL;
         }
         ret_char = d->character;   /* return current char on exit */
      }
   }
   if( number_of_hits == 1 )
      return ret_char;
   else
   {
      send_to_char( "No one like that waiting for authorization.\r\n", ch );
      return NULL;
   }
}

void do_authorize( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Usage:  authorize <player> <yes|name|no/deny>\r\n", ch );
      send_to_char( "Pending authorizations:\r\n", ch );
      send_to_char( " Chosen Character Name\r\n", ch );
      send_to_char( "---------------------------------------------\r\n", ch );
      for( d = first_descriptor; d; d = d->next )
         if( ( victim = d->character ) != NULL && IS_WAITING_FOR_AUTH( victim ) )
            ch_printf( ch, " %s@%s new %s...\r\n", victim->name, victim->desc->host, race_table[victim->race].race_name );
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
      snprintf( buf, MAX_STRING_LENGTH, "%s authorized %s", ch->name, victim->name );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
      ch_printf( ch, "You have authorized %s.\r\n", victim->name );

      /*
       * Below sends a message to player when name is accepted - Brittany   
       */

      ch_printf( victim,   /* B */
                 "The MUD Administrators have accepted the name %s.\r\n"   /* B */
                 "You are now fully authorized to play SWFotE.\r\n", victim->name );   /* B */
      return;
   }
   else if( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
   {
      send_to_char( "You have been denied access.\r\n", victim );
      snprintf( buf, MAX_STRING_LENGTH, "%s denied authorization to %s", ch->name, victim->name );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
      ch_printf( ch, "You have denied %s.\r\n", victim->name );
      do_quit( victim, "" );
   }

   else if( !str_cmp( arg2, "name" ) || !str_cmp( arg2, "n" ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s has denied %s's name", ch->name, victim->name );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
      ch_printf( victim,
                 "The MUD Administrators have found the name %s "
                 "to be unacceptable.\r\n" "Use 'name' to change it to something more apropriate.\r\n", victim->name );
      ch_printf( ch, "You requested %s change names.\r\n", victim->name );
      victim->pcdata->auth_state = 2;
      return;
   }

   else
   {
      send_to_char( "Invalid argument.\r\n", ch );
      return;
   }
}

void do_bamfin( CHAR_DATA * ch, const char *argument )
{
   if( !IS_NPC( ch ) )
   {
      smash_tilde( argument );
      DISPOSE( ch->pcdata->bamfin );
      ch->pcdata->bamfin = strdup( argument );
      send_to_char( "Ok.\r\n", ch );
   }
   return;
}



void do_bamfout( CHAR_DATA * ch, const char *argument )
{
   if( !IS_NPC( ch ) )
   {
      smash_tilde( argument );
      DISPOSE( ch->pcdata->bamfout );
      ch->pcdata->bamfout = strdup( argument );
      send_to_char( "Ok.\r\n", ch );
   }
   return;
}

// Still used for setrank self. If you're feeling ambitious, port
// the crud over there to get rid of one command.
void do_rank( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: setrank  self <string/none>\r\n", ch );
      return;
   }

   if( strlen( argument ) > 40 || strlen( remand( argument ) ) > 19 )
   {
      send_to_char( "&RThat rank is too long. Choose one under 40 characters with color codes and under 20 without.\r\n",
                    ch );
      return;
   }

   smash_tilde( argument );
   argument = rembg( argument );
   if( !str_cmp( argument, "none" ) )
      ch->rank = strdup( "" );
   else
      ch->rank = strdup( centertext( argument, 19 ) );


   ch_printf( ch, "Your rank is now: %s", argument );
   return;
}


void do_retire( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Retire whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( victim->top_level < LEVEL_SAVIOR )
   {
      send_to_char( "The minimum level for retirement is savior.\r\n", ch );
      return;
   }

   if( IS_RETIRED( victim ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s returns from retirement.\r\n", victim->name );
      ch_printf( victim, "%s brings you back from retirement.\r\n", ch->name );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s is now a retired immortal.\r\n", victim->name );
      ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\r\n", ch->name );
   }
   return;
}

void do_deny( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Deny whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   SET_BIT( victim->act, PLR_DENY );
   send_to_char( "You are denied access!\r\n", victim );
   send_to_char( "OK.\r\n", ch );
   do_quit( victim, "" );

   return;
}



void do_disconnect( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Disconnect whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim->desc == NULL )
   {
      act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( get_trust( ch ) <= get_trust( victim ) )
   {
      send_to_char( "They might not like that...\r\n", ch );
      return;
   }

   for( d = first_descriptor; d; d = d->next )
   {
      if( d == victim->desc )
      {
         close_socket( d, FALSE );
         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }

   bug( "%s: *** desc not found ***.", __func__ );
   send_to_char( "Descriptor not found!\r\n", ch );
   return;
}

/*
 * Force a level one player to quit.             Gorog
 */
void do_fquit( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Force whom to quit?\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim->top_level != 1 )
   {
      send_to_char( "They are not level one!\r\n", ch );
      return;
   }

   send_to_char( "The MUD administrators force you to quit\r\n", victim );
   do_quit( victim, "" );
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_forceclose( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int desc;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: forceclose <descriptor#>\r\n", ch );
      return;
   }
   desc = atoi( arg );

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->descriptor == desc )
      {
         if( d->character && get_trust( d->character ) >= get_trust( ch ) )
         {
            send_to_char( "They might not like that...\r\n", ch );
            return;
         }
         close_socket( d, FALSE );
         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }

   send_to_char( "Not found!\r\n", ch );
   return;
}



void do_pardon( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: pardon <character> <planet>.\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   send_to_char
      ( "Syntax: pardon <character> <planet>.... But it doesn't work .... Tell Durga to hurry up and finish it :p\r\n", ch );
   return;
}


void echo_to_all( short AT_COLOR, const char *argument, short tar )
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
         send_to_char( "\r\n", d->character );
      }
   }
   return;
}

void do_echo( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short color;
   int target;
   const char *parg;

   if( IS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You are noemoted and can not echo.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Echo what?\r\n", ch );
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
      ch_printf( ch, "I don't think %s would like that!\r\n", arg );
      return;
   }
   echo_to_all( color, argument, target );
}

void echo_to_room( short AT_COLOR, ROOM_INDEX_DATA * room, const char *argument )
{
   CHAR_DATA *vic;

   if( room == NULL )
      return;


   for( vic = room->first_person; vic; vic = vic->next_in_room )
   {
      set_char_color( AT_COLOR, vic );
      send_to_char( argument, vic );
      send_to_char( "\r\n", vic );
   }
}

void do_recho( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short color;

   if( IS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You are noemoted and can not recho.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Recho what?\r\n", ch );
      return;
   }

   one_argument( argument, arg );
   if( !str_cmp( arg, "Thoric" )
       || !str_cmp( arg, "Dominus" )
       || !str_cmp( arg, "Circe" )
       || !str_cmp( arg, "Haus" )
       || !str_cmp( arg, "Narn" ) || !str_cmp( arg, "Scryn" ) || !str_cmp( arg, "Blodkai" ) || !str_cmp( arg, "Damian" ) )
   {
      ch_printf( ch, "I don't think %s would like that!\r\n", arg );
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
void transfer_char( CHAR_DATA * ch, CHAR_DATA * victim, ROOM_INDEX_DATA * location )
{
   if( !victim->in_room )
   {
      bug( "%s: victim in NULL room: %s", __func__, victim->name );
      return;
   }

   if( IS_NPC( ch ) && room_is_private( victim, location ) )
   {
      progbug( "Mptransfer - Private room", ch );
      return;
   }

   if( !can_see( ch, victim ) )
      return;

   if( IS_NPC( ch ) && NOT_AUTHED( victim ) && location->area != victim->in_room->area )
   {
      char buf[MAX_STRING_LENGTH];

      snprintf( buf, MAX_STRING_LENGTH, "Mptransfer - unauthed char (%s)", victim->name );
      progbug( buf, ch );
      return;
   }

   /*
    * If victim not in area's level range, do not transfer 
    */
   if( IS_NPC( ch ) && !in_hard_range( victim, location->area ) && !IS_SET( location->room_flags, ROOM_PROTOTYPE ) )
      return;

   if( victim->fighting )
      stop_fighting( victim, TRUE );

   if( !IS_NPC( ch ) )
   {
      act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
      victim->retran = victim->in_room->vnum;
   }
   char_from_room( victim );
   char_to_room( victim, location );
   if( !IS_NPC( ch ) )
   {
      act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
      if( ch != victim )
         act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
      do_look( victim, "auto" );
      if( !IS_IMMORTAL( victim ) && !IS_NPC( victim ) && !in_hard_range( victim, location->area ) )
         act( AT_DANGER, "Warning:  this player's level is not within the area's level range.", ch, NULL, NULL, TO_CHAR );
   }
}

void do_transfer( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Transfer whom (and where)?\r\n", ch );
      return;
   }

   if( arg2[0] != '\0' )
   {
      if( !( location = find_location( ch, arg2 ) ) )
      {
         send_to_char( "That location does not exist.\r\n", ch );
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
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   transfer_char( ch, victim, location );
}

void do_retran( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Retransfer whom?\r\n", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   snprintf( buf, MAX_STRING_LENGTH, "'%s' %d", victim->name, victim->retran );
   do_transfer( ch, buf );
   return;
}

void do_regoto( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];

   snprintf( buf, MAX_STRING_LENGTH, "%d", ch->regoto );
   do_goto( ch, buf );
   return;
}

void do_at( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *wch;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "At where what?\r\n", ch );
      return;
   }

   if( ( location = find_location( ch, arg ) ) == NULL )
   {
      send_to_char( "No such location.\r\n", ch );
      return;
   }

   if( room_is_private( ch, location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\r\n", ch );
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

void do_rat( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Syntax: rat <start> <end> <command>\r\n", ch );
      return;
   }

   Start = atoi( arg1 );
   End = atoi( arg2 );

   if( Start < 1 || End < Start || Start > End || Start == End || End > 32767 )
   {
      send_to_char( "Invalid range.\r\n", ch );
      return;
   }

   if( !str_cmp( argument, "quit" ) )
   {
      send_to_char( "I don't think so!\r\n", ch );
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
   send_to_char( "Done.\r\n", ch );
   return;
}


void do_rstat( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   OBJ_DATA *obj;
   CHAR_DATA *rch;
   EXIT_DATA *pexit;
   int cnt;
   const static char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

   one_argument( argument, arg );

   if( get_trust( ch ) < LEVEL_IMMORTAL )
   {
      AREA_DATA *pArea;

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to goto.\r\n", ch );
         return;
      }

      if( ch->in_room->vnum < pArea->low_r_vnum || ch->in_room->vnum > pArea->hi_r_vnum )
      {
         send_to_char( "You can only rstat within your assigned range.\r\n", ch );
         return;
      }

   }


   if( !str_cmp( arg, "exits" ) )
   {
      location = ch->in_room;

      ch_printf( ch, "&GExits for room '&W%s.' &Gvnum &W%d\r\n", location->name, location->vnum );

      for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
         ch_printf( ch,
                    "%2d) %2s to %-5d.  Key: %d  Flags: %d  Keywords: '%s'.\r\nDescription: %sExit links back to vnum: %d  Exit's RoomVnum: %d  Distance: %d\r\n",
                    ++cnt,
                    dir_text[pexit->vdir],
                    pexit->to_room ? pexit->to_room->vnum : 0,
                    pexit->key,
                    pexit->exit_info,
                    pexit->keyword,
                    pexit->description[0] != '\0'
                    ? pexit->description : "(none).\r\n",
                    pexit->rexit ? pexit->rexit->vnum : 0, pexit->rvnum, pexit->distance );
      return;
   }
   location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
   if( !location )
   {
      send_to_char( "No such location.\r\n", ch );
      return;
   }

   if( ch->in_room != location && room_is_private( ch, location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\r\n", ch );
      }

   }

   ch_printf( ch, "&GName: &W%s.\r\n&GArea: &W%s  &GFilename: &W%s.\r\n",
              location->name,
              location->area ? location->area->name : "None????", location->area ? location->area->filename : "None????" );

   ch_printf( ch,
              "&GVnum: &W%d.  &GSector: &W%d.  &GLight: &W%d.  &GTeleDelay: &W%d.  &GTeleVnum: &W%d  &GTunnel: &W%d.\r\n",
              location->vnum,
              location->sector_type, location->light, location->tele_delay, location->tele_vnum, location->tunnel );

   ch_printf( ch, "&GRoom flags: &W%s\r\n", flag_string( location->room_flags, r_flags ) );
   ch_printf( ch, "&GRoom flags2: &W%s\r\n", flag_string( location->room_flags2, r_flags2 ) );
   ch_printf( ch, "&GDescription:\r\n&W%s", location->description );

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
      send_to_char( "'.\r\n", ch );
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

   send_to_char( ".\r\n&GObjects:   &W", ch );
   for( obj = location->first_content; obj; obj = obj->next_content )
   {
      send_to_char( " ", ch );
      one_argument( obj->name, buf );
      send_to_char( buf, ch );
   }
   send_to_char( ".\r\n", ch );

   if( location->first_exit )
      send_to_char( "&G------------------- EXITS -------------------&W\r\n", ch );
   for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
      ch_printf( ch,
                 "&G%2d) &W%-2s &Gto &W%-5d.  &GKey: &W%d  &GFlags: &W%d  &GKeywords: &W%s.\r\n",
                 ++cnt,
                 dir_text[pexit->vdir],
                 pexit->to_room ? pexit->to_room->vnum : 0,
                 pexit->key, pexit->exit_info, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
   return;
}



void do_ostat( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   OBJ_DATA *obj;
   char *pdesc;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Ostat what?\r\n", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strlcpy( arg, argument, MAX_INPUT_LENGTH );

   if( ( obj = get_obj_world( ch, arg ) ) == NULL )
   {
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
      return;
   }

   ch_printf( ch, "&w&GName: &W%s\r\n", obj->name );

   pdesc = get_extra_descr( arg, obj->first_extradesc );
   if( !pdesc )
      pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc );
   if( !pdesc )
      pdesc = get_extra_descr( obj->name, obj->first_extradesc );
   if( !pdesc )
      pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
   if( pdesc )
      send_to_char( pdesc, ch );


   ch_printf( ch, "&GVnum: &W%d  &GType: &W%s  &GCount: &W%d  &GGcount: &W%d\r\n",
              obj->pIndexData->vnum, item_type_name( obj ), obj->pIndexData->count, obj->count );

   ch_printf( ch, "&GSerial#: &W%d  &GTopIdxSerial#: &W%d  &GTopSerial#: &W%d\r\n",
              obj->serial, obj->pIndexData->serial, cur_obj_serial );

   ch_printf( ch, "&GShort description: &W%s\r\n&GLong description: &W%s\r\n", obj->short_descr, obj->description );

   if( obj->action_desc[0] != '\0' )
      ch_printf( ch, "&GAction description: &W%s\r\n", obj->action_desc );

   ch_printf( ch, "&GWear flags : &W%s\r\n", flag_string( obj->wear_flags, w_flags ) );
   ch_printf( ch, "&GExtra flags: &W%s\r\n", flag_string( obj->extra_flags, o_flags ) );

   ch_printf( ch, "&GNumber: &W%d&G/&W%d  &GWeight: &W%d&G/&W%d  &GLayers: &W%d\r\n",
              1, get_obj_number( obj ), obj->weight, get_obj_weight( obj ), obj->pIndexData->layers );

   ch_printf( ch, "&GCost: &W%d  &GRent: &W%d  &GTimer: &W%d  &GLevel: &W%d\r\n",
              obj->cost, obj->pIndexData->rent, obj->timer, obj->level );

   ch_printf( ch,
              "&GIn room: &W%d  &GIn object: &W%s  &GCarried by: &W%s  &GWear_loc: &W%d\r\n",
              obj->in_room == NULL ? 0 : obj->in_room->vnum,
              obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
              obj->carried_by == NULL ? "(none)" : obj->carried_by->name, obj->wear_loc );

   ch_printf( ch, "&GIndex Values : &W%d %d %d %d %d %d\r\n",
              obj->pIndexData->value[0], obj->pIndexData->value[1],
              obj->pIndexData->value[2], obj->pIndexData->value[3], obj->pIndexData->value[4], obj->pIndexData->value[5] );
   ch_printf( ch, "&GObject Values: &W%d %d %d %d %d %d\r\n",
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
      send_to_char( "'.\r\n", ch );
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
      send_to_char( "'.\r\n", ch );
   }

   for( paf = obj->first_affect; paf; paf = paf->next )
      ch_printf( ch, "&w&GAffects &W%s&G by &W%d&G. (extra)\r\n", affect_loc_name( paf->location ), paf->modifier );

   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      ch_printf( ch, "&w&GAffects &W%s&G by &W%d&G.\r\n", affect_loc_name( paf->location ), paf->modifier );

   if( ( obj->item_type == ITEM_CONTAINER ) && ( obj->first_content ) )
   {
      send_to_char( "&w&GContents:&W\r\n", ch );
      show_list_to_char( obj->first_content, ch, TRUE, FALSE );
   }

   return;
}

/* New mstat by tawnos. Holy shit this took a while :P*/
void do_mstat( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Mstat whom?\r\n", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strlcpy( arg, argument, MAX_INPUT_LENGTH );

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( ch ) )
   {
      send_to_char( "Why would a mob need to mstat something?\r\n", ch );
      return;
   }

   if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }

   ch_printf( ch, "&W&GCharacter Data for %s\r\n", IS_NPC( victim ) ? victim->short_descr : victim->name );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   ch_printf( ch, "&W&z| &GName&W: %-12.12s     &GLastname&W: %-12s      &GClan&W: %-19s &z|\r\n", victim->name,
              ( IS_NPC( victim ) || !victim->pcdata->last_name ) ? "(none)" : victim->pcdata->last_name, ( IS_NPC( victim )
                                                                                                           ||
                                                                                                           !victim->pcdata->
                                                                                                           clan ) ? "(none)"
              : victim->pcdata->clan->name );
   ch_printf( ch,
              "&W&z|  &GStr&W: %-2d  &GInt&W: %-2d  &GWis&W: %-2d  &GDex&W: %-2d  &GCon&W: %-2d  &GCha&W: %-2d  &GLck&W: %-2d  &GFrc&W: %-2d      &z|\r\n",
              get_curr_str( victim ), get_curr_int( victim ), get_curr_wis( victim ), get_curr_dex( victim ),
              get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ), get_curr_frc( victim ) );
   ch_printf( ch, "&W&z|  &GSex&W: %-6s               &GVnum&W: %-6d          &GInRoom&W: %-6d              &z|\r\n",
              victim->sex == SEX_MALE ? "Male" : victim->sex == SEX_FEMALE ? "Female" : "Neuter",
              IS_NPC( victim ) ? victim->pIndexData->vnum : 0, victim->in_room == NULL ? 0 : victim->in_room->vnum );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   if( !IS_NPC( victim ) && victim->desc )
   {
      ch_printf( ch, "&W&z|       &GUser&W: %-44s &GTrust&W: %-2d           &z|\r\n", victim->desc->host, victim->trust );
      ch_printf( ch, "&W&z| &GDescriptor&W: %-3d                                       &GAuthedBy&W: %-12s &z|\r\n",
                 victim->desc->descriptor, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)" );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   ch_printf( ch, "&W&z|       &GGold&W: %-10d          &GRace&W: %-17s  &GHit&W: %5d/%-5d    &z|\r\n", victim->gold,
              npc_race[victim->race], victim->hit, victim->max_hit );
   ch_printf( ch, "&W&z|       &GBank&W: %-10ld            &GAC&W: %-5d             &GMana&W: %5d/%-5d    &z|\r\n",
              ( IS_NPC( victim )
                || !victim->pcdata->bank ) ? 0 : victim->pcdata->bank, GET_AC( victim ), victim->mana, victim->max_mana );
   ch_printf( ch, "&W&z|   &GTopLevel&W: %-2d                 &GAlign&W: %-5d             &GMove&W: %5d/%-5d    &z|\r\n",
              victim->top_level, victim->alignment, victim->move, victim->max_move );
   if( !IS_NPC( victim ) )
      ch_printf( ch, "&W&z|       &GSalary&W: %-8d        &GThirst&W: %-4d             &GFull&W: %-4d            &z|\r\n",
                 victim->pcdata->salary, victim->pcdata->condition[COND_THIRST], victim->pcdata->condition[COND_FULL] );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   if( !IS_NPC( victim ) )
   {
      for( ability = 0; ability < MAX_ABILITY; ability++ )
         ch_printf( ch, "&W&z|   &G%-15s Level&W: %-3d    &GMax&W: %-3d    &GExp&W: %-8ld     &GNext&W: %-8d &z|\r\n",
                    ability_name[ability], victim->skill_level[ability], max_level( victim, ability ),
                    victim->experience[ability], exp_level( victim->skill_level[ability] + 1 ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   ch_printf( ch, "&W&z|    &GHitroll&W: %-3d           &GMentalstate&W: %-4d         &GFighting&W: %-12s   &z|\r\n",
              GET_HITROLL( victim ), victim->mental_state, victim->fighting ? victim->fighting->who->name : "(none)" );
   ch_printf( ch, "&W&z|    &GDamroll&W: %-3d        &GEmotionalstate&W: %-4d           &GMaster&W: %-12s   &z|\r\n",
              GET_DAMROLL( victim ), victim->emotional_state, victim->master ? victim->master->name : "(none)" );
   ch_printf( ch, "&W&z|   &GPosition&W: %-1d                   &GWimpy&W: %-4d           &GLeader&W: %-12s   &z|\r\n",
              victim->position, victim->wimpy, victim->leader ? victim->leader->name : "(none)" );
   ch_printf( ch, "&W&z| &GAffectedBy&W: %-64s &z|\r\n", affect_bit_name( victim->affected_by ) );
   ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   if( !IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z|      &GYears&W: %-3d                     &GSeconds Played&W: %-10d               &z|\r\n",
                 get_age( victim ), ( int )victim->played );
      ch_printf( ch, "&W&z|        &GAct&W: %-14d                   &GTimer&W: %-6d                   &z|\r\n", victim->act,
                 victim->timer );
      ch_printf( ch, "&W&z| &GCarry Info: Items&W: %4d/%-4d                &GWeight&W: %7.7d/%-7.7d          &z|\r\n",
                 victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   if( !IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z| &GPlrFlags&W: %-66.66s &z|\r\n", flag_string( victim->act, plr_flags ) );
      ch_printf( ch, "&W&z| &GPcflags&W: %-67s &z|\r\n", flag_string( victim->pcdata->flags, pc_flags ) );
      ch_printf( ch, "&W&z| &GWanted Flags&W: %-62s &z|\r\n", flag_string( victim->pcdata->wanted_flags, planet_flags ) );
      ch_printf( ch, "&W&z| &GBestowments&W: %-63s &z|\r\n",
                 victim->pcdata->bestowments != NULL ? victim->pcdata->bestowments : "None" );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   if( IS_NPC( victim ) )
   {
      if( victim->pIndexData->count != 1 || victim->pIndexData->killed != 0 )
         ch_printf( ch,
                    "&W&z|      &GCount&W: %-3d                             &GKilled&W: %-3d                      &z|\r\n",
                    victim->pIndexData->count, victim->pIndexData->killed );
      ch_printf( ch,
                 "&W&z|   &GHit Dice&W: %-2dd%-2d+%-4d                    &GDam Dice&W: %-2dd%-2d+%-4d               &z|\r\n",
                 victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice, victim->pIndexData->hitplus,
                 victim->pIndexData->damnodice, victim->pIndexData->damsizedice, victim->pIndexData->damplus );
      ch_printf( ch,
                 "&W&z| &GSaving throws&W: %-1d %-1d %-1d %-1d %-1d                                                     &z|\r\n",
                 victim->saving_poison_death, victim->saving_wand, victim->saving_para_petri, victim->saving_breath,
                 victim->saving_spell_staff );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z|     &GSpeaks&W: %-10d  &GSpeaking&W: %-10d                                 &z|\r\n",
                 victim->speaks, victim->speaking );
      strlcpy( langbuf, " ", MAX_STRING_LENGTH );
      for( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
      {
         if( knows_language( victim, lang_array[x], victim ) || ( IS_NPC( victim ) && victim->speaks == 0 ) )
         {
            strlcat( langbuf, lang_names[x], MAX_STRING_LENGTH );
            strlcat( langbuf, " ", MAX_STRING_LENGTH );
         }
      }
      ch_printf( ch, "&W&z|  &GLanguages&W: %-64s &z|\r\n", langbuf );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   if( victim->resistant || victim->immune || victim->susceptible )
   {
      ch_printf( ch, "&W&z|   &GRes&W: %-69s &z|\r\n", flag_string( victim->resistant, ris_flags ) );
      ch_printf( ch, "&W&z|   &GImm&W: %-69s &z|\r\n", flag_string( victim->immune, ris_flags ) );
      ch_printf( ch, "&W&z|   &GSus&W: %-69s &z|\r\n", flag_string( victim->susceptible, ris_flags ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "&W&z|  &GNumAttacks&W: %-2d                                                              &z|\r\n",
                 victim->numattacks );
      ch_printf( ch, "&W&z|     &GAttacks&W: %-63s &z|\r\n", flag_string( victim->attacks, attack_flags ) );
      ch_printf( ch, "&W&z|     &GDefense&W: %-63s &z|\r\n", flag_string( victim->defenses, defense_flags ) );
      ch_printf( ch, "&W&z|       &GParts&W: %-63s &z|\r\n", flag_string( victim->xflags, part_flags ) );
      ch_printf( ch, "&W&z+------------------------------------------------------------------------------+\r\n" );
   }
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "&GAct flags&W: %s\r\n", flag_string( victim->act, act_flags ) );
      ch_printf( ch, "&GVIP flags&W: %s\r\n", flag_string( victim->vip_flags, planet_flags ) );
      ch_printf( ch, "    &GNames&W: %s\r\n", victim->name );
      ch_printf( ch, "&GShortDesc&W: %s\r\n", victim->short_descr );
      ch_printf( ch, " &GLongDesc&W: %s\r\n", victim->long_descr );
   }
   for( paf = victim->first_affect; paf; paf = paf->next )
      if( ( skill = get_skilltype( paf->type ) ) != NULL )
         ch_printf( ch, "&W%s: &G'%s'&W modifies &G%s&W by &G%d&W for %d rounds with bits %s.\r\n", skill_tname[skill->type],
                    skill->name, affect_loc_name( paf->location ), paf->modifier, paf->duration,
                    affect_bit_name( paf->bitvector ) );

   if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
      ch_printf( ch, "&RHelled until %24.24s by %s.&W\r\n",
                 ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );

   if( IS_NPC( victim ) && ( victim->spec_fun || victim->spec_2 ) )
      ch_printf( ch, "Mobile has spec fun: %s %s\r\n", victim->spec_funname, victim->spec_2 ? victim->spec_funname2 : "" );
   return;
}

void do_oldmstat( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Mstat whom?\r\n", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strlcpy( arg, argument, MAX_INPUT_LENGTH );
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }
   ch_printf( ch, "Name: %s    Lastname: %s    Organization: %s\r\n",
              victim->name, ( IS_NPC( victim ) || !victim->pcdata->last_name ) ? "(none)" : victim->pcdata->last_name,
              ( IS_NPC( victim ) || !victim->pcdata->clan ) ? "(none)" : victim->pcdata->clan->name );
   if( get_trust( ch ) >= LEVEL_GOD && !IS_NPC( victim ) && victim->desc )
      ch_printf( ch, "Host: %s   Descriptor: %d   Trust: %d   AuthedBy: %s\r\n",
                 victim->desc->host, victim->desc->descriptor,
                 victim->trust, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)" );
   if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
      ch_printf( ch, "Helled until %24.24s by %s.\r\n", ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );

   ch_printf( ch, "Vnum: %d   Sex: %s   Room: %d   Count: %d  Killed: %d\r\n",
              IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
              victim->sex == SEX_MALE ? "male" :
              victim->sex == SEX_FEMALE ? "female" : "neutral",
              !victim->in_room ? 0 : victim->in_room->vnum,
              IS_NPC( victim ) ? victim->pIndexData->count : 1,
              IS_NPC( victim ) ? victim->pIndexData->killed : victim->pcdata->mdeaths + victim->pcdata->pdeaths );
   ch_printf( ch, "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d  Frc: %d\r\n",
              get_curr_str( victim ),
              get_curr_int( victim ),
              get_curr_wis( victim ),
              get_curr_dex( victim ),
              get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ), get_curr_frc( victim ) );
   ch_printf( ch, "Hps: %d/%d  Force: %d/%d   Move: %d/%d\r\n",
              victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move );
   if( !IS_NPC( victim ) )
   {
      int ability;

      for( ability = 0; ability < MAX_ABILITY; ability++ )
         ch_printf( ch, "%-15s   Level: %-3d   Max: %-3d   Exp: %-10ld   Next: %-10d\r\n",
                    ability_name[ability], victim->skill_level[ability], max_level( victim, ability ),
                    victim->experience[ability], exp_level( victim->skill_level[ability] + 1 ) );
   }
   ch_printf( ch,
              "Top Level: %d     Race: %d  Align: %d  AC: %d  Gold: %d\r\n",
              victim->top_level, victim->race, victim->alignment, GET_AC( victim ), victim->gold );
   if( victim->race < MAX_NPC_RACE && victim->race >= 0 )
      ch_printf( ch, "Race: %s\r\n", npc_race[victim->race] );
   ch_printf( ch, "Hitroll: %d   Damroll: %d   Position: %d   Wimpy: %d \r\n",
              GET_HITROLL( victim ), GET_DAMROLL( victim ), victim->position, victim->wimpy );
   ch_printf( ch, "Fighting: %s    Master: %s    Leader: %s\r\n",
              victim->fighting ? victim->fighting->who->name : "(none)",
              victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)" );
   if( !IS_NPC( victim ) )
      ch_printf( ch,
                 "Thirst: %d   Full: %d   Drunk: %d     Glory: %d/%d\r\n",
                 victim->pcdata->condition[COND_THIRST],
                 victim->pcdata->condition[COND_FULL],
                 victim->pcdata->condition[COND_DRUNK], victim->pcdata->quest_curr, victim->pcdata->quest_accum );
   else
      ch_printf( ch, "Hit dice: %dd%d+%d.  Damage dice: %dd%d+%d.\r\n",
                 victim->pIndexData->hitnodice,
                 victim->pIndexData->hitsizedice,
                 victim->pIndexData->hitplus,
                 victim->pIndexData->damnodice, victim->pIndexData->damsizedice, victim->pIndexData->damplus );
   ch_printf( ch, "MentalState: %d   EmotionalState: %d\r\n", victim->mental_state, victim->emotional_state );
   ch_printf( ch, "Saving throws: %d %d %d %d %d.\r\n",
              victim->saving_poison_death,
              victim->saving_wand, victim->saving_para_petri, victim->saving_breath, victim->saving_spell_staff );
   ch_printf( ch, "Carry figures: items (%d/%d)  weight (%d/%d)   Numattacks: %d\r\n",
              victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ), victim->numattacks );
   ch_printf( ch, "Years: %d   Seconds Played: %d   Timer: %d   Act: %d\r\n",
              get_age( victim ), ( int )victim->played, victim->timer, victim->act );
   if( IS_NPC( victim ) )
   {
      ch_printf( ch, "Act flags: %s\r\n", flag_string( victim->act, act_flags ) );
      ch_printf( ch, "VIP flags: %s\r\n", flag_string( victim->vip_flags, planet_flags ) );
   }
   else
   {
      ch_printf( ch, "Player flags: %s\r\n", flag_string( victim->act, plr_flags ) );
      ch_printf( ch, "Pcflags: %s\r\n", flag_string( victim->pcdata->flags, pc_flags ) );
      ch_printf( ch, "Wanted flags: %s\r\n", flag_string( victim->pcdata->wanted_flags, planet_flags ) );
   }
   ch_printf( ch, "Affected by: %s\r\n", affect_bit_name( victim->affected_by ) );
   ch_printf( ch, "Speaks: %d   Speaking: %d\r\n", victim->speaks, victim->speaking );
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
   send_to_char( "\r\n", ch );
   if( victim->pcdata && victim->pcdata->bestowments && victim->pcdata->bestowments[0] != '\0' )
      ch_printf( ch, "Bestowments: %s\r\n", victim->pcdata->bestowments );
   ch_printf( ch, "Short description: %s\r\nLong  description: %s",
              victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\r\n" );
   if( IS_NPC( victim ) && ( victim->spec_fun || victim->spec_2 ) )
      ch_printf( ch, "Mobile has spec fun: %s %s\r\n", victim->spec_funname, victim->spec_2 ? victim->spec_funname2 : "" );
   ch_printf( ch, "Body Parts : %s\r\n", flag_string( victim->xflags, part_flags ) );
   ch_printf( ch, "Resistant  : %s\r\n", flag_string( victim->resistant, ris_flags ) );
   ch_printf( ch, "Immune     : %s\r\n", flag_string( victim->immune, ris_flags ) );
   ch_printf( ch, "Susceptible: %s\r\n", flag_string( victim->susceptible, ris_flags ) );
   ch_printf( ch, "Attacks    : %s\r\n", flag_string( victim->attacks, attack_flags ) );
   ch_printf( ch, "Defenses   : %s\r\n", flag_string( victim->defenses, defense_flags ) );
   for( paf = victim->first_affect; paf; paf = paf->next )
      if( ( skill = get_skilltype( paf->type ) ) != NULL )
         ch_printf( ch,
                    "%s: '%s' modifies %s by %d for %d rounds with bits %s.\r\n",
                    skill_tname[skill->type],
                    skill->name,
                    affect_loc_name( paf->location ), paf->modifier, paf->duration, affect_bit_name( paf->bitvector ) );
   return;
}

void do_mfind( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   int hash;
   int nMatch;
   bool fAll;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mfind whom?\r\n", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );
   nMatch = 0;
   set_pager_color( AT_PLAIN, ch );

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
            pager_printf( ch, "[%5d] %s\r\n", pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

   return;
}

void do_ofind( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Ofind what?\r\n", ch );
      return;
   }

   set_pager_color( AT_PLAIN, ch );
   fAll = !str_cmp( arg, "all" );
   nMatch = 0;
/*  nLoop	= 0; */

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
            pager_printf( ch, "[%5d] %s\r\n", pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

   return;
}

void do_mwhere( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   bool found;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mwhere whom?\r\n", ch );
      return;
   }

   set_pager_color( AT_PLAIN, ch );
   found = FALSE;
   for( victim = first_char; victim; victim = victim->next )
   {
      if( IS_NPC( victim ) && victim->in_room && nifty_is_name( arg, victim->name ) )
      {
         found = TRUE;
         pager_printf( ch, "[%5d] %-28s [%5d] %s\r\n",
                       victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name );
      }
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

   return;
}

void do_bodybag( CHAR_DATA * ch, const char *argument )
{
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   bool found;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Bodybag whom?\r\n", ch );
      return;
   }

   /*
    * check to see if vict is playing? 
    */
   snprintf( buf2, MAX_STRING_LENGTH, "the corpse of %s", arg );
   found = FALSE;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( obj->in_room && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && !str_cmp( buf2, obj->short_descr ) )
      {
         found = TRUE;
         ch_printf( ch, "Bagging body: [%5d] %-28s [%5d] %s\r\n",
                    obj->pIndexData->vnum, obj->short_descr, obj->in_room->vnum, obj->in_room->name );
         obj_from_room( obj );
         obj = obj_to_char( obj, ch );
         obj->timer = -1;
         save_char_obj( ch );
      }
   }

   if( !found )
      ch_printf( ch, " You couldn't find any %s\r\n", buf2 );
   return;
}

/* New owhere by Altrag, 03/14/96 */
void do_owhere( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Owhere what?\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg1 );

   set_pager_color( AT_PLAIN, ch );
   if( arg1[0] != '\0' && !str_prefix( arg1, "nesthunt" ) )
   {
      if( !( obj = get_obj_world( ch, arg ) ) )
      {
         send_to_char( "Nesthunt for what object?\r\n", ch );
         return;
      }
      for( ; obj->in_obj; obj = obj->in_obj )
      {
         pager_printf( ch, "[%5d] %-28s in object [%5d] %s\r\n",
                       obj->pIndexData->vnum, obj_short( obj ), obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr );
         ++icnt;
      }
      snprintf( buf, MAX_STRING_LENGTH, "[%5d] %-28s in ", obj->pIndexData->vnum, obj_short( obj ) );
      if( obj->carried_by )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "invent [%5d] %s\r\n",
                  ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
      else if( obj->in_room )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "room   [%5d] %s\r\n", obj->in_room->vnum, obj->in_room->name );
      else if( obj->in_obj )
      {
         bug( "%s: obj->in_obj after NULL!", __func__ );
         strlcat( buf, "object??\r\n", MAX_STRING_LENGTH );
      }
      else
      {
         bug( "%s: object doesnt have location!", __func__, 0 );
         strlcat( buf, "nowhere??\r\n", MAX_STRING_LENGTH );
      }
      send_to_pager( buf, ch );
      ++icnt;
      pager_printf( ch, "Nested %d levels deep.\r\n", icnt );
      return;
   }

   found = FALSE;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( !nifty_is_name( arg, obj->name ) )
         continue;
      found = TRUE;

      snprintf( buf, MAX_STRING_LENGTH, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum, obj_short( obj ) );
      if( obj->carried_by )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "invent [%5d] %s\r\n",
                  ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
      else if( obj->in_room )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "room   [%5d] %s\r\n", obj->in_room->vnum, obj->in_room->name );
      else if( obj->in_obj )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "object [%5d] %s\r\n", obj->in_obj->pIndexData->vnum, obj_short( obj->in_obj ) );
      else
      {
         bug( "%s: object doesnt have location!", __func__ );
         strlcat( buf, "nowhere??\r\n", MAX_STRING_LENGTH );
      }
      send_to_pager( buf, ch );
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   else
      pager_printf( ch, "%d matches.\r\n", icnt );

   return;
}

void do_reboo( CHAR_DATA * ch, const char *argument )
{
   send_to_char( "If you want to REBOOT, spell it out.\r\n", ch );
   return;
}

void do_reboot( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) && str_cmp( argument, "and sort skill table" ) )
   {
      send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\r\n", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );

   snprintf( buf, MAX_STRING_LENGTH, "Reboot by %s.", ch->name );
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

void do_shutdow( CHAR_DATA * ch, const char *argument )
{
   send_to_char( "If you want to SHUTDOWN, spell it out.\r\n", ch );
   return;
}

void do_shutdown( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) )
   {
      send_to_char( "Syntax: 'shutdown mud now' or 'shutdown nosave'\r\n", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );

   snprintf( buf, MAX_STRING_LENGTH, "Shutdown by %s.", ch->name );
   append_file( ch, SHUTDOWN_FILE, buf );
   strlcat( buf, "\r\n", MAX_STRING_LENGTH );
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

void do_snoop( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Snoop whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( !victim->desc )
   {
      send_to_char( "No descriptor to snoop.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Cancelling all snoops.\r\n", ch );
      for( d = first_descriptor; d; d = d->next )
         if( d->snoop_by == ch->desc )
            d->snoop_by = NULL;
      return;
   }

   if( victim->desc->snoop_by )
   {
      send_to_char( "Busy already.\r\n", ch );
      return;
   }

   /*
    * Minimum snoop level... a secret mset value
    * makes the snooper think that the victim is already being snooped
    */
   if( get_trust( victim ) >= get_trust( ch ) || ( victim->pcdata && victim->pcdata->min_snoop > get_trust( ch ) ) )
   {
      send_to_char( "Busy already.\r\n", ch );
      return;
   }

   if( ch->desc )
   {
      for( d = ch->desc->snoop_by; d; d = d->snoop_by )
         if( d->character == victim || d->original == victim )
         {
            send_to_char( "No snoop loops.\r\n", ch );
            return;
         }
   }

/*  Snoop notification for higher imms, if desired, uncomment this
    if ( get_trust(victim) > LEVEL_GOD && get_trust(ch) < LEVEL_SUPREME )
      write_to_descriptor( victim->desc, "\r\nYou feel like someone is watching your every move...\r\n", 0 );
*/
   victim->desc->snoop_by = ch->desc;
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_switch( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Switch into whom?\r\n", ch );
      return;
   }

   if( !ch->desc )
      return;

   if( ch->desc->original )
   {
      send_to_char( "You are already switched.\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( victim->desc )
   {
      send_to_char( "Character in use.\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "You cannot switch into a player!\r\n", ch );
      return;
   }

   ch->desc->character = victim;
   ch->desc->original = ch;
   victim->desc = ch->desc;
   ch->desc = NULL;
   ch->switched = victim;
   send_to_char( "Ok.\r\n", victim );
   return;
}



void do_return( CHAR_DATA * ch, const char *argument )
{
   if( !ch->desc )
      return;

   if( !ch->desc->original )
   {
      send_to_char( "You aren't switched.\r\n", ch );
      return;
   }

   if( IS_SET( ch->act, ACT_POLYMORPHED ) )
   {
      send_to_char( "Use revert to return from a polymorphed mob.\r\n", ch );
      return;
   }

   send_to_char( "You return to your original body.\r\n", ch );
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



void do_minvoke( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *victim;
   int vnum;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: minvoke <vnum>.\r\n", ch );
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
         send_to_char( "No such mobile exists.\r\n", ch );
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
         send_to_char( "Huh?\r\n", ch );
         return;
      }

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this mobile.\r\n", ch );
         return;
      }
      if( vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum )
      {
         send_to_char( "That number is not in your allocated range.\r\n", ch );
         return;
      }
   }

   if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "No mobile has that vnum.\r\n", ch );
      return;
   }

   victim = create_mobile( pMobIndex );
   char_to_room( victim, ch->in_room );
   act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_oinvoke( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Syntax: oinvoke <vnum> <level>.\r\n", ch );
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
         send_to_char( "Syntax: oinvoke <vnum> <level>.\r\n", ch );
         return;
      }
      level = atoi( arg2 );
      if( level < 0 || level > get_trust( ch ) )
      {
         send_to_char( "Limited to your trust level.\r\n", ch );
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
         send_to_char( "No such object exists.\r\n", ch );
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
         send_to_char( "Huh?\r\n", ch );
         return;
      }

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this object.\r\n", ch );
         return;
      }
      if( vnum < pArea->low_o_vnum || vnum > pArea->hi_o_vnum )
      {
         send_to_char( "That number is not in your allocated range.\r\n", ch );
         return;
      }
   }

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char( "No object has that vnum.\r\n", ch );
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
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_purge( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Ok.\r\n", ch );
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
         send_to_char( "They aren't here.\r\n", ch );
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
      send_to_char( "Not on PC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\r\n", ch );
      return;
   }

   if( IS_SET( victim->act, ACT_POLYMORPHED ) )
   {
      send_to_char( "You cannot purge a polymorphed player.\r\n", ch );
      return;
   }
   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
   extract_char( victim, TRUE );
   return;
}


void do_low_purge( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Purge what?\r\n", ch );
      return;
   }

   victim = NULL;
   obj = NULL;
   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      send_to_char( "You can't find that here.\r\n", ch );
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
      send_to_char( "Not on PC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\r\n", ch );
      return;
   }

   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
   act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
   extract_char( victim, TRUE );
   return;
}

void do_balzhur( CHAR_DATA* ch, const char* argument )
{
   char ebuf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char godfile[256];
   char areafile[256];
   char *name;
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;

   set_char_color( AT_BLOOD, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Who is deserving of such a fate?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't currently playing.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "I wouldn't even think of that if I were you...\r\n", ch );
      return;
   }

   victim->top_level = 1;
   victim->trust = 0;
   check_switch( victim, TRUE );
   set_char_color( AT_WHITE, ch );
   send_to_char( "You summon the demon Balzhur to wreak your wrath!\r\n", ch );
   send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\r\n", ch );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\r\n", victim );
   snprintf( ebuf, MAX_STRING_LENGTH, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
   echo_to_all( AT_IMMORT, ebuf, ECHOTAR_ALL );

   for( int ability = 0; ability < MAX_ABILITY; ability++ )
   {
      victim->experience[ability] = 1;
      victim->skill_level[ability] = 1;
   }

   victim->max_hit = 500;
   victim->max_mana = 0;
   victim->max_move = 1000;
   for( sn = 0; sn < top_sn; sn++ )
      victim->pcdata->learned[sn] = 0;
   victim->hit = victim->max_hit;
   victim->mana = victim->max_mana;
   victim->move = victim->max_move;

   name = capitalize( victim->name );
   snprintf( godfile, 256, "%s%s", GOD_DIR, name );

   set_char_color( AT_RED, ch );
   if( !remove( godfile ) )
      send_to_char( "Player's immortal data destroyed.\r\n", ch );
   else if( errno != ENOENT )
   {
      ch_printf( ch, "Unknown error #%d - %s (immortal data). Report to the admins.\r\n", errno, strerror( errno ) );
      snprintf( ebuf, MAX_STRING_LENGTH, "%s balzhuring %s", ch->name, name );
      perror( ebuf );
   }

   snprintf( areafile, 256, "%s.are", name );
   for( pArea = first_build; pArea; pArea = pArea->next )
   {
      if( !str_cmp( pArea->filename, areafile ) )
      {
         char buildfile[256];
         char buildbackup[256];

         int bc = snprintf( buildfile, 256, "%s%s", BUILD_DIR, areafile );
         if( bc < 0 )
            bug( "%s: Output buffer error!", __func__ );

         if( IS_SET( pArea->status, AREA_LOADED ) )
            fold_area( pArea, buildfile, FALSE );
         close_area( pArea );

         bc = snprintf( buildbackup, 256, "%s.bak", buildfile );
         if( bc < 0 )
            bug( "%s: Output buffer error!", __func__ );

         set_char_color( AT_RED, ch ); /* Log message changes colors */
         if( !rename( buildfile, buildbackup ) )
            send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
         else if( errno != ENOENT )
         {
            ch_printf( ch, "Unknown error #%d - %s (area data). Report to the admins.\r\n", errno, strerror( errno ) );
            snprintf( ebuf, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buildfile );
            perror( ebuf );
         }
         break;
      }
   }

   make_wizlist(  );
   do_help( victim, "M_BALZHUR_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake after a long period of time...\r\n", victim );
   while( victim->first_carrying )
      extract_obj( victim->first_carrying );
}

void do_advance( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Syntax: advance <char> <ability> <level>.\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   ability = -1;

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
      send_to_char( "No Such Ability.\r\n", ch );
      do_advance( ch, "" );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( ch ) <= get_trust( victim ) && ch != victim )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }

   if( ( level = atoi( arg2 ) ) < 1 || level > 500 )
   {
      send_to_char( "Level must be 1 to 500.\r\n", ch );
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
      send_to_char( "Lowering a player's level!\r\n", ch );
      set_char_color( AT_IMMORT, victim );
      send_to_char( "Cursed and forsaken! The gods have lowered your level.\r\n", victim );
      victim->experience[ability] = 0;
      victim->skill_level[ability] = 1;
      if( ability == COMBAT_ABILITY )
         victim->max_hit = 500;
      if( ability == FORCE_ABILITY )
         victim->max_mana = 0;
   }
   else
   {
      send_to_char( "Raising a player's level!\r\n", ch );
      send_to_char( "The gods feel fit to raise your level!\r\n", victim );
   }

   for( iLevel = victim->skill_level[ability]; iLevel < level; iLevel++ )
   {
      victim->experience[ability] = exp_level( iLevel + 1 );
      gain_exp( victim, 0, ability );
   }
   return;
}

void do_immortalize( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int level;
   CHAR_DATA *victim;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Syntax: immortalize <char> <level>\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }
   level = atoi( argument );
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( level > MAX_LEVEL )
   {
      send_to_char( "That is beyond the level range.\r\n", ch );
      return;
   }
   if( level < LEVEL_IMMORTAL )
   {
      send_to_char( "That is below the level range.\r\n", ch );
      return;
   }

   do_help( victim, "M_GODLVL1_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake... all your possessions are gone.\r\n", victim );
   while( victim->first_carrying )
      extract_obj( victim->first_carrying );

   //victim->top_level = LEVEL_IMMORTAL;

/*    advance_level( victim );  */
   victim->top_level = level;
   victim->trust = 0;
   return;
}

void do_trust( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int level;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
   {
      send_to_char( "Syntax: trust <char> <level>.\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Level must be 0 (reset) or 1 to 60.\r\n", ch );
      return;
   }

   if( level > get_trust( ch ) )
   {
      send_to_char( "Limited to your own trust.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }

   victim->trust = level;
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_restore( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Restore whom?\r\n", ch );
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
            send_to_char( "You can't do that.\r\n", ch );
            return;
         }
         else
         {
            /*
             * Check if the player did a restore all within the last 18 hours. 
             */
            if( current_time - last_restore_all_time < RESTORE_INTERVAL )
            {
               send_to_char( "Sorry, you can't do a restore all yet.\r\n", ch );
               do_restoretime( ch, "" );
               return;
            }
         }
      }
      last_restore_all_time = current_time;
      ch->pcdata->restore_time = current_time;
      save_char_obj( ch );
      send_to_char( "Ok.\r\n", ch );
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
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }

      if( get_trust( ch ) < LEVEL_LESSER && victim != ch && !( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) ) )
      {
         send_to_char( "You can't do that.\r\n", ch );
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
      send_to_char( "Ok.\r\n", ch );
      return;
   }
}

void do_restoretime( CHAR_DATA * ch, const char *argument )
{
   long int time_passed;
   int hour, minute;

   if( !last_restore_all_time )
      ch_printf( ch, "There has been no restore all since reboot\r\n" );
   else
   {
      time_passed = current_time - last_restore_all_time;
      hour = ( int )( time_passed / 3600 );
      minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
      ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\r\n", hour, minute );
   }

   if( !ch->pcdata )
      return;

   if( !ch->pcdata->restore_time )
   {
      send_to_char( "You have never done a restore all.\r\n", ch );
      return;
   }

   time_passed = current_time - ch->pcdata->restore_time;
   hour = ( int )( time_passed / 3600 );
   minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
   ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\r\n", hour, minute );
   return;
}

void do_freeze( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Freeze whom?\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( victim->desc && victim->desc->original && get_trust( victim->desc->original ) >= get_trust( ch ) )
   {
      send_to_char( "For some inexplicable reason, you failed.\r\n", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_FREEZE ) )
   {
      REMOVE_BIT( victim->act, PLR_FREEZE );
      send_to_char( "Your frozen form suddenly thaws.\r\n", victim );
      ch_printf( ch, "%s is now unfrozen.\r\n", victim->name );
   }
   else
   {
      if( victim->switched )
      {
         do_return( victim->switched, "" );
         set_char_color( AT_LBLUE, victim );
      }
      SET_BIT( victim->act, PLR_FREEZE );
      send_to_char( "A godly force turns your body to ice!\r\n", victim );
      ch_printf( ch, "You have frozen %s.\r\n", victim->name );
   }

   save_char_obj( victim );
   return;
}

void do_slog( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Secret Log whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   /*
    * No level check, gods can log anyone.
    */
   if( IS_SET( victim->act, PLR_SLOG ) )
   {
      REMOVE_BIT( victim->act, PLR_SLOG );
      send_to_char( "Secret Log removed.\r\n", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_SLOG );
      send_to_char( "Secret Log set.\r\n", ch );
   }

   return;
}

void do_log( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Log whom?\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      if( fLogAll )
      {
         fLogAll = FALSE;
         send_to_char( "Log ALL off.\r\n", ch );
      }
      else
      {
         fLogAll = TRUE;
         send_to_char( "Log ALL on.\r\n", ch );
      }
      return;
   }

   if( !str_cmp( arg, "pc" ) )
   {
      if( fLogPC )
      {
         fLogPC = FALSE;
         send_to_char( "Log all PC's off.\r\n", ch );
      }
      else
      {
         fLogPC = TRUE;
         send_to_char( "Log ALL PC's on.\r\n", ch );
      }
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   /*
    * No level check, gods can log anyone.
    */
   if( IS_SET( victim->act, PLR_LOG ) )
   {
      REMOVE_BIT( victim->act, PLR_LOG );
      send_to_char( "LOG removed.\r\n", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_LOG );
      send_to_char( "LOG set.\r\n", ch );
   }

   return;
}

void do_litterbug( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Set litterbug flag on whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_LITTERBUG ) )
   {
      REMOVE_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "You can drop items again.\r\n", victim );
      send_to_char( "LITTERBUG removed.\r\n", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "You a strange force prevents you from dropping any more items!\r\n", victim );
      send_to_char( "LITTERBUG set.\r\n", ch );
   }

   return;
}


void do_noemote( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Noemote whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_NO_EMOTE ) )
   {
      REMOVE_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can emote again.\r\n", victim );
      send_to_char( "NO_EMOTE removed.\r\n", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can't emote!\r\n", victim );
      send_to_char( "NO_EMOTE set.\r\n", ch );
   }

   return;
}



void do_notell( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_NO_TELL ) )
   {
      REMOVE_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can tell again.\r\n", victim );
      send_to_char( "NO_TELL removed.\r\n", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can't tell!\r\n", victim );
      send_to_char( "NO_TELL set.\r\n", ch );
   }

   return;
}

void do_notitle( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Notitle whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      send_to_char( "You can set your own title again.\r\n", victim );
      send_to_char( "NOTITLE removed.\r\n", ch );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      snprintf( buf, MAX_STRING_LENGTH, "%s", victim->name );
      set_title( victim, buf );
      send_to_char( "You can't set your own title!\r\n", victim );
      send_to_char( "NOTITLE set.\r\n", ch );
   }

   return;
}

void do_silence( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_SILENCE ) )
   {
      send_to_char( "Player already silenced, use unsilence to remove.\r\n", ch );
   }
   else
   {
      SET_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can't use channels!\r\n", victim );
      send_to_char( "SILENCE set.\r\n", ch );
   }

   return;
}

void do_unsilence( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Unsilence whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( IS_SET( victim->act, PLR_SILENCE ) )
   {
      REMOVE_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can use channels again.\r\n", victim );
      send_to_char( "SILENCE removed.\r\n", ch );
   }
   else
   {
      send_to_char( "That player is not silenced.\r\n", ch );
   }

   return;
}

void do_peace( CHAR_DATA * ch, const char *argument )
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

   send_to_char( "Ok.\r\n", ch );
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
      bug( "%s: Cannot open " BAN_LIST, __func__ );
      perror( BAN_LIST );
      return;
   }
   for( pban = first_ban; pban; pban = pban->next )
      fprintf( fp, "%d %s~~%s~\n", pban->level, pban->name, pban->ban_time );
   fprintf( fp, "-1\n" );
   FCLOSE( fp );
   return;
}

void do_ban( CHAR_DATA * ch, const char *argument )
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
      send_to_pager( "Banned sites:\r\n", ch );
      send_to_pager( "[ #] (Lv) Time                     Site\r\n", ch );
      send_to_pager( "---- ---- ------------------------ ---------------\r\n", ch );
      for( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
         pager_printf( ch, "[%2d] (%2d) %-24s %s\r\n", bnum, pban->level, pban->ban_time, pban->name );
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
            ch_printf( ch, "Level range: 1 - %d.\r\n", LEVEL_SUPREME );
            return;
         }
         pban->level = atoi( arg );
         send_to_char( "Ban level set.\r\n", ch );
      }
      else if( !str_cmp( arg, "newban" ) )
      {
         pban->level = 1;
         send_to_char( "New characters banned.\r\n", ch );
      }
      else if( !str_cmp( arg, "mortal" ) )
      {
         pban->level = LEVEL_AVATAR;
         send_to_char( "All mortals banned.\r\n", ch );
      }
      else if( !str_cmp( arg, "total" ) )
      {
         pban->level = LEVEL_SUPREME;
         send_to_char( "Everyone banned.\r\n", ch );
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
      send_to_char( "Syntax: ban <site address>\r\n", ch );
      send_to_char( "Syntax: ban <ban number> <level <lev>|newban|mortal|" "total>\r\n", ch );
      return;
   }

   for( pban = first_ban; pban; pban = pban->next )
   {
      if( !str_cmp( arg, pban->name ) )
      {
         send_to_char( "That site is already banned!\r\n", ch );
         return;
      }
   }

   CREATE( pban, BAN_DATA, 1 );
   LINK( pban, first_ban, last_ban, next, prev );
   pban->name = strdup( arg );
   pban->level = LEVEL_AVATAR;
   snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
   pban->ban_time = strdup( buf );
   save_banlist(  );
   send_to_char( "Ban created.  Mortals banned from site.\r\n", ch );
   return;
}

void do_allow( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   BAN_DATA *pban;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Remove which site from the ban list?\r\n", ch );
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
         send_to_char( "Site no longer banned.\r\n", ch );
         return;
      }
   }

   send_to_char( "Site is not banned.\r\n", ch );
   return;
}

void do_wizlock( CHAR_DATA * ch, const char *argument )
{
   extern bool wizlock;
   wizlock = !wizlock;

   if( wizlock )
      send_to_char( "Game wizlocked.\r\n", ch );
   else
      send_to_char( "Game un-wizlocked.\r\n", ch );

   return;
}


void do_noresolve( CHAR_DATA * ch, const char *argument )
{
   sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

   if( sysdata.NO_NAME_RESOLVING )
      send_to_char( "Name resolving disabled.\r\n", ch );
   else
      send_to_char( "Name resolving enabled.\r\n", ch );

   return;
}

/* Output of command reformmated by Samson 2-8-98, and again on 4-7-98 */
void do_users( CHAR_DATA * ch, const char *argument )
{
   DESCRIPTOR_DATA *d;
   int count;
   const char *st;

   set_pager_color( AT_PLAIN, ch );

   count = 0;
   send_to_pager( "Desc|     Constate      |Idle|    Player    | HostIP                   \r\n", ch );
   send_to_pager( "----+-------------------+----+--------------+--------------------------\r\n", ch );
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
            pager_printf( ch, " %3d| %-17s |%4d| %-12s | %s \r\n", d->descriptor, st, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
      else
      {
         if( ( get_trust( ch ) >= LEVEL_SUPREME || ( d->character && can_see( ch, d->character ) ) )
             && ( !str_prefix( argument, d->host ) || ( d->character && !str_prefix( argument, d->character->name ) ) ) )
         {
            count++;
            pager_printf( ch, " %3d| %2d|%4d| %-12s | %s \r\n", d->descriptor, d->connected, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
   }
   pager_printf( ch, "%d user%s.\r\n", count, count == 1 ? "" : "s" );
   return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   bool mobsonly;
   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Force whom to do what?\r\n", ch );
      return;
   }

   mobsonly = get_trust( ch ) < sysdata.level_forcepc;

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( mobsonly )
      {
         send_to_char( "Force whom to do what?\r\n", ch );
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
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }

      if( victim == ch )
      {
         send_to_char( "Aye aye, right away!\r\n", ch );
         return;
      }

      if( ( get_trust( victim ) >= get_trust( ch ) ) || ( mobsonly && !IS_NPC( victim ) ) )
      {
         send_to_char( "Do it yourself!\r\n", ch );
         return;
      }

      act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
      interpret( victim, argument );
   }

   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_invis( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short level;

   /*
    * if ( IS_NPC(ch))
    * return;
    */

   argument = one_argument( argument, arg );
   if( arg[0] != '\0' )
   {
      if( !is_number( arg ) )
      {
         send_to_char( "Usage: invis | invis <level>\r\n", ch );
         return;
      }
      level = atoi( arg );
      if( level < 2 || level > get_trust( ch ) )
      {
         send_to_char( "Invalid level.\r\n", ch );
         return;
      }

      if( !IS_NPC( ch ) )
      {
         ch->pcdata->wizinvis = level;
         ch_printf( ch, "Wizinvis level set to %d.\r\n", level );
      }

      if( IS_NPC( ch ) )
      {
         ch->mobinvis = level;
         ch_printf( ch, "Mobinvis level set to %d.\r\n", level );
      }
      return;
   }

   if( IS_NPC( ch ) )
   {
      if( ch->mobinvis < 2 )
         ch->mobinvis = ch->top_level;
      return;
   }

   if( ch->pcdata->wizinvis < 2 )
      ch->pcdata->wizinvis = ch->top_level;

   if( IS_SET( ch->act, PLR_WIZINVIS ) )
   {
      REMOVE_BIT( ch->act, PLR_WIZINVIS );
      act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\r\n", ch );
   }
   else
   {
      SET_BIT( ch->act, PLR_WIZINVIS );
      act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\r\n", ch );
   }

   return;
}


void do_holylight( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->act, PLR_HOLYLIGHT ) )
   {
      REMOVE_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode off.\r\n", ch );
   }
   else
   {
      SET_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode on.\r\n", ch );
   }

   return;
}

bool check_area_conflict( AREA_DATA * area, int low_range, int hi_range )
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
void do_vassign( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Syntax: vassign <who> <low> <high>\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They don't seem to be around.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || get_trust( victim ) < LEVEL_CREATOR )
   {
      send_to_char( "They wouldn't know what to do with a vnum range.\r\n", ch );
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
      ch_printf( victim, "%s has removed your vnum range.\r\n", ch->name );
      save_char_obj( victim );
      return;
   }

   if( victim->pcdata->area && lo != 0 )
   {
      send_to_char( "You cannot assign them a range, they already have one!\r\n", ch );
      return;
   }

   if( lo == 0 && hi != 0 )
   {
      send_to_char( "Unacceptable vnum range, low vnum cannot be 0 when hi vnum is not.\r\n", ch );
      return;
   }

   if( lo > hi )
   {
      send_to_char( "Unacceptable vnum range, low vnum must be smaller than high vnum.\r\n", ch );
      return;
   }

   if( check_area_conflicts( lo, hi ) )
   {
      send_to_char( "That vnum range conflicts with another area. Check the zones or vnums command.\r\n", ch );
      return;
   }

   victim->pcdata->r_range_lo = lo;
   victim->pcdata->r_range_hi = hi;
   victim->pcdata->o_range_lo = lo;
   victim->pcdata->o_range_hi = hi;
   victim->pcdata->m_range_lo = lo;
   victim->pcdata->m_range_hi = hi;
   assign_area( victim );
   send_to_char( "Done.\r\n", ch );
   ch_printf( victim, "%s has assigned you the vnum range %d - %d.\r\n", ch->name, lo, hi );
   assign_area( victim );  /* Put back by Thoric on 02/07/96 */

   if( !victim->pcdata->area )
   {
      bug( "%s: assign_area failed", __func__ );
      return;
   }

   tarea = victim->pcdata->area;

   /*
    * Initialize first and last rooms in range 
    */
   if( !( room = make_room( lo, tarea ) ) )
   {
      bug( "%s: make_room failed to initialize first room.", __func__ );
      return;
   }

   if( !( room = make_room( hi, tarea ) ) )
   {
      bug( "%s: make_room failed to initialize last room.", __func__ );
      return;
   }

   /*
    * Initialize first mob in range 
    */
   if( !( pMobIndex = make_mobile( lo, 0, "first mob" ) ) )
   {
      bug( "%s: make_mobile failed to initialize first mob.", __func__ );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, room );

   /*
    * Initialize last mob in range 
    */
   if( !( pMobIndex = make_mobile( hi, 0, "last mob" ) ) )
   {
      bug( "%s: make_mobile failed to initialize last mob.", __func__ );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, room );

   /*
    * Initialize first obj in range 
    */
   if( !( pObjIndex = make_object( lo, 0, "first obj" ) ) )
   {
      bug( "%s: make_object failed to initialize first obj.", __func__ );
      return;
   }
   obj = create_object( pObjIndex, 0 );
   obj_to_room( obj, room );

   /*
    * Initialize last obj in range 
    */
   if( !( pObjIndex = make_object( hi, 0, "last obj" ) ) )
   {
      bug( "%s: make_object failed to initialize last obj.", __func__ );
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
   ch_printf( ch, "Vnum range set for %s and initialized.\r\n", victim->name );

   return;
}

void do_cmdtable( CHAR_DATA * ch, const char *argument )
{
   int hash, cnt;
   CMDTYPE *cmd;

   set_pager_color( AT_PLAIN, ch );
   send_to_pager( "Commands and Number of Uses This Run\r\n", ch );

   for( cnt = hash = 0; hash < 126; hash++ )
      for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
      {
         if( ( ++cnt ) % 4 )
            pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses );
         else
            pager_printf( ch, "%-6.6s %4d\r\n", cmd->name, cmd->userec.num_uses );
      }
   return;
}

/* 
 * Load up a player file 
 */
void do_loadup( CHAR_DATA * ch, const char *argument )
{
   char fname[256];
   char name[256];
   struct stat fst;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *temp;

   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: loadup <playername>\r\n", ch );
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
      send_to_char( "They are already playing.\r\n", ch );
      return;
   }

   name[0] = UPPER( name[0] );

   snprintf( fname, 256, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
   if( stat( fname, &fst ) != -1 )
   {
      CREATE( d, DESCRIPTOR_DATA, 1 );
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE( d->outbuf, char, d->outsize );

      load_char_obj( d, name, FALSE, FALSE );
      add_char( d->character );
      old_room_vnum = d->character->in_room->vnum;
      char_to_room( d->character, ch->in_room );
      if( get_trust( d->character ) >= get_trust( ch ) )
      {
         do_say( d->character, "Do *NOT* disturb me again!" );
         send_to_char( "I think you'd better leave that player alone!\r\n", ch );
         d->character->desc = NULL;
         do_quit( d->character, "" );
         return;
      }
      d->character->desc = NULL;
      d->character->retran = old_room_vnum;
      d->character = NULL;
      DISPOSE( d->outbuf );
      DISPOSE( d );
      ch_printf( ch, "Player %s loaded from room %d.\r\n", capitalize( name ), old_room_vnum );
      snprintf( buf, MAX_STRING_LENGTH, "%s appears from nowhere, eyes glazed over.\r\n", capitalize( name ) );
      act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   /*
    * else no player file 
    */
   send_to_char( "No such player.\r\n", ch );
   return;
}

void do_fixchar( CHAR_DATA * ch, const char *argument )
{
   char name[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: fixchar <playername>\r\n", ch );
      return;
   }
   victim = get_char_room( ch, name );
   if( !victim )
   {
      send_to_char( "They're not here.\r\n", ch );
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
   send_to_char( "Done.\r\n", ch );
}

void do_newbieset( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   CHAR_DATA *victim;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: newbieset <char>.\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( ( victim->top_level < 1 ) || ( victim->top_level > 5 ) )
   {
      send_to_char( "Level of victim must be 1 to 5.\r\n", ch );
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
   ch_printf( ch, "You have re-equipped %s.\r\n", victim->name );
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
            strlcat( out, " ", MAX_STRING_LENGTH );
         strlcat( out, buf, MAX_STRING_LENGTH );
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
            strlcat( out, " ", MAX_STRING_LENGTH );
         strlcat( out, buf, MAX_STRING_LENGTH );
      }
   }
}

void do_bestowarea( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   int arg_len;

   argument = one_argument( argument, arg );

   if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
   {
      send_to_char( "Sorry...\r\n", ch );
      return;
   }

   if( !*arg )
   {
      send_to_char( "Syntax:\r\n"
                    "bestowarea <victim> <filename>.are\r\n"
                    "bestowarea <victim> none             removes bestowed areas\r\n"
                    "bestowarea <victim> list             lists bestowed areas\r\n"
                    "bestowarea <victim>                  lists bestowed areas\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\r\n", ch );
      return;
   }

   if( get_trust( victim ) < LEVEL_IMMORTAL )
   {
      send_to_char( "They aren't an immortal.\r\n", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = strdup( "" );

   if( !*argument || !str_cmp( argument, "list" ) )
   {
      extract_area_names( victim->pcdata->bestowments, buf );
      ch_printf( ch, "Bestowed areas: %s\r\n", buf );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      remove_area_names( victim->pcdata->bestowments, buf );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = strdup( buf );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   arg_len = strlen( argument );
   if( arg_len < 5
       || argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a'
       || argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e' )
   {
      send_to_char( "You can only bestow an area name\r\n", ch );
      send_to_char( "E.G. bestow joe sam.are\r\n", ch );
      return;
   }

   snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, argument );
   DISPOSE( victim->pcdata->bestowments );
   victim->pcdata->bestowments = strdup( buf );
   ch_printf( victim, "%s has bestowed on you the area: %s\r\n", ch->name, argument );
   send_to_char( "Done.\r\n", ch );
}

void do_bestow( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], arg_buf[MAX_STRING_LENGTH-30];
   char tmparg[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   CMDTYPE *cmd;
   bool fComm = FALSE;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Bestow whom with what?\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\r\n", ch );
      return;
   }

   if( victim == ch || get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You aren't powerful enough...\r\n", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = strdup( "" );

   if( argument[0] == '\0' || !str_cmp( argument, "show list" ) )
   {
      ch_printf( ch, "Current bestowed commands on %s: %s.\r\n", victim->name, victim->pcdata->bestowments );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = strdup( "" );
      ch_printf( ch, "Bestowments removed from %s.\r\n", victim->name );
      ch_printf( victim, "%s has removed your bestowed commands.\r\n", ch->name );
      check_switch( victim, FALSE );
      return;
   }

   arg_buf[0] = '\0';

   argument = one_argument( argument, arg );

   while( arg[0] != '\0' )
   {
      char *cmd_buf, cmd_tmp[MAX_INPUT_LENGTH];
      bool cFound = FALSE;

      if( !( cmd = find_command( arg, TRUE ) ) )
      {
         ch_printf( ch, "No such command as %s!\r\n", arg );
         argument = one_argument( argument, arg );
         continue;
      }
      else if( cmd->level > get_trust( ch ) )
      {
         ch_printf( ch, "You can't bestow the %s command!\r\n", arg );
         argument = one_argument( argument, arg );
         continue;
      }

      cmd_buf = victim->pcdata->bestowments;
      cmd_buf = one_argument( cmd_buf, cmd_tmp );
      while( cmd_tmp[0] != '\0' )
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

      snprintf( tmparg, MAX_STRING_LENGTH, "%s ", arg );
      strlcat( arg_buf, tmparg, MAX_STRING_LENGTH-30 );
      argument = one_argument( argument, arg );
      fComm = TRUE;
   }
   if( !fComm )
   {
      send_to_char( "Good job, knucklehead... you just bestowed them with that master command called 'NOTHING!'\r\n", ch );
      return;
   }

   if( arg_buf[strlen( arg_buf ) - 1] == ' ' )
      arg_buf[strlen( arg_buf ) - 1] = '\0';

   snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg_buf );
   DISPOSE( victim->pcdata->bestowments );
   smash_tilde( buf );
   victim->pcdata->bestowments = strdup( buf );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has bestowed on you the command(s): %s\r\n", ch->name, arg_buf );
   send_to_char( "Done.\r\n", ch );
}

struct tm *update_time( struct tm *old_time )
{
   time_t ntime;

   ntime = mktime( old_time );
   return localtime( &ntime );
}

void do_set_boot_time( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   bool check;

   check = FALSE;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\r\n", ch );
      send_to_char( "        setboot manual {0/1}\r\n", ch );
      send_to_char( "        setboot default\r\n", ch );
      ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\r\n", reboot_time, set_boot_time->manual );
      return;
   }

   if( !str_cmp( arg, "time" ) )
   {
      struct tm *now_time;

      argument = one_argument( argument, arg );
      argument = one_argument( argument, arg1 );
      if( !*arg || !*arg1 || !is_number( arg ) || !is_number( arg1 ) )
      {
         send_to_char( "You must input a value for hour and minute.\r\n", ch );
         return;
      }
      now_time = localtime( &current_time );

      if( ( now_time->tm_hour = atoi( arg ) ) < 0 || now_time->tm_hour > 23 )
      {
         send_to_char( "Valid range for hour is 0 to 23.\r\n", ch );
         return;
      }

      if( ( now_time->tm_min = atoi( arg1 ) ) < 0 || now_time->tm_min > 59 )
      {
         send_to_char( "Valid range for minute is 0 to 59.\r\n", ch );
         return;
      }

      argument = one_argument( argument, arg );
      if( *arg != '\0' && is_number( arg ) )
      {
         if( ( now_time->tm_mday = atoi( arg ) ) < 1 || now_time->tm_mday > 31 )
         {
            send_to_char( "Valid range for day is 1 to 31.\r\n", ch );
            return;
         }
         argument = one_argument( argument, arg );
         if( *arg != '\0' && is_number( arg ) )
         {
            if( ( now_time->tm_mon = atoi( arg ) ) < 1 || now_time->tm_mon > 12 )
            {
               send_to_char( "Valid range for month is 1 to 12.\r\n", ch );
               return;
            }
            now_time->tm_mon--;
            argument = one_argument( argument, arg );
            if( ( now_time->tm_year = atoi( arg ) - 1900 ) < 0 || now_time->tm_year > 199 )
            {
               send_to_char( "Valid range for year is 1900 to 2099.\r\n", ch );
               return;
            }
         }
      }
      now_time->tm_sec = 0;
      if( mktime( now_time ) < current_time )
      {
         send_to_char( "You can't set a time previous to today!\r\n", ch );
         return;
      }
      if( set_boot_time->manual == 0 )
         set_boot_time->manual = 1;
      new_boot_time = update_time( now_time );
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check( mktime( new_boot_time ) );
      get_reboot_string(  );

      ch_printf( ch, "Boot time set to %s\r\n", reboot_time );
      check = TRUE;
   }
   else if( !str_cmp( arg, "manual" ) )
   {
      argument = one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Please enter a value for manual boot on/off\r\n", ch );
         return;
      }

      if( !is_number( arg1 ) )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\r\n", ch );
         return;
      }

      if( atoi( arg1 ) < 0 || atoi( arg1 ) > 1 )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\r\n", ch );
         return;
      }

      set_boot_time->manual = atoi( arg1 );
      ch_printf( ch, "Manual bit set to %s\r\n", arg1 );
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

      send_to_char( "Reboot time set back to normal.\r\n", ch );
      check = TRUE;
   }

   if( !check )
   {
      send_to_char( "Invalid argument for setboot.\r\n", ch );
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
void do_form_password( CHAR_DATA * ch, const char *argument )
{
   char *pwcheck;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      return;
   }

   /*
    * This is arbitrary to discourage weak passwords 
    */
   if( strlen( argument ) < 5 )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      send_to_char( "New password must be at least 5 characters in length.\r\n", ch );
      return;
   }

   if( argument[0] == '!' )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      send_to_char( "New password cannot begin with the '!' character.\r\n", ch );
      return;
   }

   pwcheck = sha256_crypt( argument );

   ch_printf( ch, "%s results in the encrypted string: %s\r\n", argument, pwcheck );
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA * ch, const char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "If you want to destroy a character, spell it out!\r\n", ch );
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
            bug( "%s: room with people #%d", __func__, rid->vnum );
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
            bug( "%s: room with contents #%d", __func__, rid->vnum );
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
               bug( "%s: rid not in hash list %d", __func__, rid->vnum );
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
               bug( "%s: mid not in hash list %s", __func__, mid->vnum );
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
               bug( "%s: oid not in hash list %s", __func__, oid->vnum );
            else
               toid->next = oid->next;
         }
         DISPOSE( oid );
      }
   }
   DISPOSE( pArea->name );
   DISPOSE( pArea->filename );
   STRFREE( pArea->author );
   if( IS_SET( pArea->flags, AFLAG_PROTOTYPE ) )
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

void do_destroy( CHAR_DATA* ch, const char* argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   char pfile[256];
   char backup[256];
   char ebuf[MAX_STRING_LENGTH];
   char *name;
   struct stat fst;

   set_char_color( AT_RED, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Destroy what player file?\r\n", ch );
      return;
   }

   /*
    * Set the file points.
    */
   name = capitalize( arg );
   snprintf( pfile, 256, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
   snprintf( backup, 256, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), name );

   /*
    * This check makes sure the name is valid and that the file is there, else there
    * is no need to go on. -Orion
    */
   if( !check_parse_name( name ) || lstat( pfile, &fst ) == -1 )
   {
      ch_printf( ch, "No player exists by the name %s.\r\n", name );
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

   if( !rename( pfile, backup ) )
   {
      AREA_DATA *pArea;
      char godfile[256];
      char areafile[256];
      char buildfile[256];
      char buildbackup[256];

      set_char_color( AT_RED, ch );
      ch_printf( ch, "Player %s destroyed.  Pfile saved in backup directory.\r\n", name );

      snprintf( godfile, 256, "%s%s", GOD_DIR, name );
      if( !remove( godfile ) )
         send_to_char( "Player's immortal data destroyed.\r\n", ch );
      else if( errno != ENOENT )
      {
         ch_printf( ch, "Unknown error #%d - %s (immortal data). Report to smaugmuds.afkmods.com\r\n", errno, strerror( errno ) );
         snprintf( ebuf, MAX_STRING_LENGTH, "%s destroying %s", ch->name, godfile );
         perror( ebuf );
      }

      snprintf( areafile, 256, "%s.are", name );
      for( pArea = first_build; pArea; pArea = pArea->next )
      {
         if( !str_cmp( pArea->filename, areafile ) )
         {
            int bc = snprintf( buildfile, 256, "%s%s", BUILD_DIR, areafile );
            if( bc < 0 )
               bug( "%s: Output buffer error!", __func__ );

            if( IS_SET( pArea->status, AREA_LOADED ) )
               fold_area( pArea, buildfile, FALSE );
            close_area( pArea );

            snprintf( buildbackup, 256, "%s.bak", buildfile );
            set_char_color( AT_RED, ch ); /* Log message changes colors */
            if( !rename( buildfile, buildfile ) )
               send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
            else if( errno != ENOENT )
            {
               ch_printf( ch, "Unknown error #%d - %s (area data). Report to smaugmuds.afkmods.com\r\n", errno, strerror( errno ) );
               snprintf( ebuf, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buildfile );
               perror( ebuf );
            }
            break;
         }
      }
   }
   else if( errno == ENOENT )
   {
      set_char_color( AT_PLAIN, ch );
      send_to_char( "Player does not exist.\r\n", ch );
   }
   else
   {
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "Unknown error #%d - %s. Report to smaugmuds.afkmods.com\r\n", errno, strerror( errno ) );
      snprintf( ebuf, MAX_STRING_LENGTH, "%s destroying %s", ch->name, arg );
      perror( ebuf );
   }
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

   static char outbuf[MAX_STRING_LENGTH];

   if( !IS_NPC( ch ) )
      return ch->name;

   one_argument( ch->name, name );  /* copy the first word into name */

   if( !name[0] ) /* weird mob .. no keywords */
   {
      strlcpy( outbuf, "", MAX_STRING_LENGTH );   /* Do not return NULL, just an empty buffer */
      return outbuf;
   }

   /*
    * ->people changed to ->first_person -- TRI 
    */
   for( rch = ch->in_room->first_person; rch && ( rch != ch ); rch = rch->next_in_room )
      if( is_name( name, rch->name ) )
         count++;

   snprintf( outbuf, MAX_STRING_LENGTH, "%d.%s", count, name );
   return outbuf;
}

void do_for( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Are you trying to crash the MUD or something?\r\n", ch );
      return;
   }

   if( !str_prefix( "for", argument ) )
   {
      send_to_char( "Are you trying to crash the MUD or something?\r\n", ch );
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
      send_to_char( "Cannot use FOR EVERYWHERE with the # thingie.\r\n", ch );
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
            const char *pSource = argument;  /* head of buffer to be parsed */
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

void save_sysdata( SYSTEM_DATA sys );

void do_cset( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_STRING_LENGTH];
   short level;

   set_char_color( AT_IMMORT, ch );

   if( argument[0] == '\0' )
   {
      ch_printf( ch, "Mail:\r\n  Read all mail: %d. Read mail for free: %d. Write mail for free: %d.\r\n",
                 sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
      ch_printf( ch, "  Take all mail: %d.\r\n", sysdata.take_others_mail );
      ch_printf( ch, "Channels:\r\n  Muse: %d. Think: %d. Log: %d. Build: %d.\r\n",
                 sysdata.muse_level, sysdata.think_level, sysdata.log_level, sysdata.build_level );
      ch_printf( ch, "Building:\r\n  Prototype modification: %d.  Player msetting: %d.\r\n",
                 sysdata.level_modify_proto, sysdata.level_mset_player );
      ch_printf( ch, "Guilds:\r\n  Overseer: %s.  Advisor: %s.\r\n", sysdata.guild_overseer, sysdata.guild_advisor );
      ch_printf( ch, "Other:\r\n  Force on players: %d.  ", sysdata.level_forcepc );
      ch_printf( ch, "Private room override: %d.\r\n", sysdata.level_override_private );
      ch_printf( ch, "  Penalty to regular stun chance: %d.  ", sysdata.stun_regular );
      ch_printf( ch, "Penalty to stun plr vs. plr: %d.\r\n", sysdata.stun_plr_vs_plr );
      ch_printf( ch, "  Percent damage plr vs. plr: %3d.  ", sysdata.dam_plr_vs_plr );
      ch_printf( ch, "Percent damage plr vs. mob: %d.\r\n", sysdata.dam_plr_vs_mob );
      ch_printf( ch, "  Percent damage mob vs. plr: %3d.  ", sysdata.dam_mob_vs_plr );
      ch_printf( ch, "Percent damage mob vs. mob: %d.\r\n", sysdata.dam_mob_vs_mob );
      ch_printf( ch, "  Get object without take flag: %d.  ", sysdata.level_getobjnotake );
      ch_printf( ch, "Autosave frequency (minutes): %d.\r\n", sysdata.save_frequency );
      ch_printf( ch, "\r\n&wPfile autocleanup status: &W%s  &wDays before purging newbies: &W%d\r\n",
                 sysdata.CLEANPFILES ? "On" : "Off", sysdata.newbie_purge );
      ch_printf( ch, "&wDays before purging regular players: &W%d\r\n", sysdata.regular_purge );
      ch_printf( ch, "  Save flags: %s\r\n", flag_string( sysdata.save_flags, save_flag ) );
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
         send_to_char( "Pfile autocleanup enabled.\r\n", ch );
      else
         send_to_char( "Pfile autocleanup disabled.\r\n", ch );
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
         send_to_char( "Not a save flag.\r\n", ch );
      else
      {
         TOGGLE_BIT( sysdata.save_flags, 1 << x );
         send_to_char( "Ok.\r\n", ch );
         save_sysdata( sysdata );
      }
      return;
   }

   if( !str_prefix( arg, "guild_overseer" ) )
   {
      DISPOSE( sysdata.guild_overseer );
      sysdata.guild_overseer = strdup( argument );
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_prefix( arg, "guild_advisor" ) )
   {
      DISPOSE( sysdata.guild_advisor );
      sysdata.guild_advisor = strdup( argument );
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   level = ( short )atoi( argument );

   if( !str_prefix( arg, "savefrequency" ) )
   {
      sysdata.save_frequency = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "newbie_purge" ) )
   {
      if( level < 1 )
      {
         send_to_char( "You must specify a period of at least 1 day.\r\n", ch );
         return;
      }

      sysdata.newbie_purge = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "regular_purge" ) )
   {
      if( level < 1 )
      {
         send_to_char( "You must specify a period of at least 1 day.\r\n", ch );
         return;
      }

      sysdata.regular_purge = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "stun" ) )
   {
      sysdata.stun_regular = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "stun_pvp" ) )
   {
      sysdata.stun_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_pvp" ) )
   {
      sysdata.dam_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "get_notake" ) )
   {
      sysdata.level_getobjnotake = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_pvm" ) )
   {
      sysdata.dam_plr_vs_mob = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_mvp" ) )
   {
      sysdata.dam_mob_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "dam_mvm" ) )
   {
      sysdata.dam_mob_vs_mob = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( level < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Invalid value for new control.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "read_all" ) )
   {
      sysdata.read_all_mail = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "read_free" ) )
   {
      sysdata.read_mail_free = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "write_free" ) )
   {
      sysdata.write_mail_free = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "take_all" ) )
   {
      sysdata.take_others_mail = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "muse" ) )
   {
      sysdata.muse_level = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "think" ) )
   {
      sysdata.think_level = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "log" ) )
   {
      sysdata.log_level = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "build" ) )
   {
      sysdata.build_level = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "proto_modify" ) )
   {
      sysdata.level_modify_proto = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "override_private" ) )
   {
      sysdata.level_override_private = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "forcepc" ) )
   {
      sysdata.level_forcepc = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   if( !str_cmp( arg, "mset_player" ) )
   {
      sysdata.level_mset_player = level;
      send_to_char( "Ok.\r\n", ch );
      save_sysdata( sysdata );
      return;
   }
   else
   {
      send_to_char( "Invalid argument.\r\n", ch );
      return;
   }
}

void get_reboot_string( void )
{
   snprintf( reboot_time, 50, "%s", asctime( new_boot_time ) );
}

void do_orange( CHAR_DATA * ch, const char *argument )
{
   send_to_char( "Function under construction.\r\n", ch );
   return;
}

void do_mrange( CHAR_DATA * ch, const char *argument )
{
   send_to_char( "Function under construction.\r\n", ch );
   return;
}

void do_hell( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   short htime;
   bool h_d = FALSE;
   struct tm *tms;

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Hell who, and for how long?\r\n", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "There is no point in helling an immortal.\r\n", ch );
      return;
   }
   if( victim->pcdata->release_date != 0 )
   {
      ch_printf( ch, "They are already in hell until %24.24s, by %s.\r\n",
                 ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
      return;
   }
   argument = one_argument( argument, arg );
   if( !*arg || !is_number( arg ) )
   {
      send_to_char( "Hell them for how long?\r\n", ch );
      return;
   }
   htime = atoi( arg );
   if( htime <= 0 )
   {
      send_to_char( "You cannot hell for zero or negative time.\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg );
   if( !*arg || !str_prefix( arg, "hours" ) )
      h_d = TRUE;
   else if( str_prefix( arg, "days" ) )
   {
      send_to_char( "Is that value in hours or days?\r\n", ch );
      return;
   }
   else if( htime > 30 )
   {
      send_to_char( "You may not hell a person for more than 30 days at a time.\r\n", ch );
      return;
   }
   tms = localtime( &current_time );
   if( h_d )
      tms->tm_hour += htime;
   else
      tms->tm_mday += htime;
   victim->pcdata->release_date = mktime( tms );
   victim->pcdata->helled_by = STRALLOC( ch->name );
   ch_printf( ch, "%s will be released from hell at %24.24s.\r\n", victim->name, ctime( &victim->pcdata->release_date ) );
   act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   char_to_room( victim, get_room_index( 6 ) );
   act( AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT );
   do_look( victim, "auto" );
   ch_printf( victim, "The immortals are not pleased with your actions.\r\n"
              "You shall remain in hell for %d %s%s.\r\n", htime, ( h_d ? "hour" : "day" ), ( htime == 1 ? "" : "s" ) );
   save_char_obj( victim );   /* used to save ch, fixed by Thoric 09/17/96 */
   return;
}

void do_unhell( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Unhell whom..?\r\n", ch );
      return;
   }
   location = ch->in_room;
   ch->in_room = get_room_index( 6 );
   victim = get_char_room( ch, arg );
   ch->in_room = location; /* The case of unhell self, etc. */
   if( !victim || IS_NPC( victim ) || victim->in_room->vnum != 6 )
   {
      send_to_char( "No one like that is in hell.\r\n", ch );
      return;
   }
   location = get_room_index( wherehome( victim ) );
   if( !location )
      location = ch->in_room;
   MOBtrigger = FALSE;
   act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   char_to_room( victim, location );
   send_to_char( "The gods have smiled on you and released you from hell early!\r\n", victim );
   do_look( victim, "auto" );
   send_to_char( "They have been released.\r\n", ch );

   if( victim->pcdata->helled_by )
   {
      if( str_cmp( ch->name, victim->pcdata->helled_by ) )
         ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\r\n",
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
void do_vsearch( CHAR_DATA * ch, const char *argument )
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
      send_to_char( "Syntax:  vsearch <vnum>.\r\n", ch );
      return;
   }

   set_pager_color( AT_PLAIN, ch );
   argi = atoi( arg );
   argi = atoi( arg );
   if( argi < 1 || argi > 32767 )
   {
      send_to_char( "Vnum out of range.\r\n", ch );
      return;
   }
   for( obj = first_object; obj != NULL; obj = obj->next )
   {
      if( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ) )
         continue;

      found = TRUE;
      for( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

      if( in_obj->carried_by != NULL )
         pager_printf( ch, "[%2d] Level %d %s carried by %s.\r\n",
                       obj_counter, obj->level, obj_short( obj ), PERS( in_obj->carried_by, ch ) );
      else
         pager_printf( ch, "[%2d] [%-5d] %s in %s.\r\n", obj_counter,
                       ( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
                       obj_short( obj ), ( in_obj->in_room == NULL ) ? "somewhere" : in_obj->in_room->name );

      obj_counter++;
   }

   if( !found )
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

   return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
void do_sober( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];

   smash_tilde( argument );

   argument = one_argument( argument, arg1 );
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on mobs.\r\n", ch );
      return;
   }

   if( victim->pcdata )
      victim->pcdata->condition[COND_DRUNK] = 0;
   send_to_char( "Ok.\r\n", ch );
   send_to_char( "You feel sober again.\r\n", victim );
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
      bug( "%s: NULL social", __func__ );
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
      bug( "%s: NULL social", __func__ );
      return;
   }

   if( !social->name )
   {
      bug( "%s: NULL social->name", __func__ );
      return;
   }

   if( !social->char_no_arg )
   {
      bug( "%s: NULL social->char_no_arg", __func__ );
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
         bug( "%s: trying to add duplicate name to bucket %d", __func__, hash );
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
void do_sedit( CHAR_DATA * ch, const char *argument )
{
   SOCIALTYPE *social;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char snoarg[MAX_INPUT_LENGTH+5];

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_char_color( AT_SOCIAL, ch );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: sedit <social> [field]\r\n", ch );
      send_to_char( "Syntax: sedit <social> create\r\n", ch );
      if( get_trust( ch ) > LEVEL_GOD )
         send_to_char( "Syntax: sedit <social> delete\r\n", ch );
      if( get_trust( ch ) > LEVEL_LESSER )
         send_to_char( "Syntax: sedit <save>\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\r\n", ch );
      return;
   }

   if( get_trust( ch ) > LEVEL_LESSER && !str_cmp( arg1, "save" ) )
   {
      save_socials(  );
      send_to_char( "Saved.\r\n", ch );
      return;
   }

   social = find_social( arg1, FALSE );

   if( !str_cmp( arg2, "create" ) )
   {
      if( social )
      {
         send_to_char( "That social already exists!\r\n", ch );
         return;
      }
      CREATE( social, SOCIALTYPE, 1 );
      social->name = strdup( arg1 );
      snprintf( snoarg, MAX_INPUT_LENGTH+5, "You %s.", arg1 );
      social->char_no_arg = strdup( snoarg );
      add_social( social );
      send_to_char( "Social added.\r\n", ch );
      return;
   }

   if( !social )
   {
      send_to_char( "Social not found.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Social: %s\r\n\r\nCNoArg: %s\r\n", social->name, social->char_no_arg );
      ch_printf( ch, "ONoArg: %s\r\nCFound: %s\r\nOFound: %s\r\n",
                 social->others_no_arg ? social->others_no_arg : "(not set)",
                 social->char_found ? social->char_found : "(not set)",
                 social->others_found ? social->others_found : "(not set)" );
      ch_printf( ch, "VFound: %s\r\nCAuto : %s\r\nOAuto : %s\r\n",
                 social->vict_found ? social->vict_found : "(not set)",
                 social->char_auto ? social->char_auto : "(not set)",
                 social->others_auto ? social->others_auto : "(not set)" );
      return;
   }

   if( get_trust( ch ) > LEVEL_GOD && !str_cmp( arg2, "delete" ) )
   {
      unlink_social( social );
      free_social( social );
      send_to_char( "Deleted.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "cnoarg" ) )
   {
      if( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
      {
         send_to_char( "You cannot clear this field.  It must have a message.\r\n", ch );
         return;
      }
      if( social->char_no_arg )
         DISPOSE( social->char_no_arg );
      social->char_no_arg = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "onoarg" ) )
   {
      if( social->others_no_arg )
         DISPOSE( social->others_no_arg );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_no_arg = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "cfound" ) )
   {
      if( social->char_found )
         DISPOSE( social->char_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_found = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "ofound" ) )
   {
      if( social->others_found )
         DISPOSE( social->others_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_found = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "vfound" ) )
   {
      if( social->vict_found )
         DISPOSE( social->vict_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->vict_found = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "cauto" ) )
   {
      if( social->char_auto )
         DISPOSE( social->char_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_auto = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "oauto" ) )
   {
      if( social->others_auto )
         DISPOSE( social->others_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_auto = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      SOCIALTYPE *checksocial;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\r\n", ch );
         return;
      }
      if( ( checksocial = find_social( arg1, TRUE ) ) != NULL )
      {
         ch_printf( ch, "There is already a social named %s.\r\n", arg1 );
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
      social->name = strdup( arg1 );
      if( relocate )
         add_social( social );
      send_to_char( "Done.\r\n", ch );
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
   if( command->fun_name )
      DISPOSE( command->fun_name );
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
      bug( "%s NULL command", __func__ );
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
      bug( "%s: NULL command", __func__ );
      return;
   }

   if( !command->name )
   {
      bug( "%s: NULL command->name", __func__ );
      return;
   }

   if( !command->do_fun )
   {
      bug( "%s: NULL command->do_fun", __func__ );
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
void do_cedit( CHAR_DATA * ch, const char *argument )
{
   CMDTYPE *command;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH+3];

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_char_color( AT_IMMORT, ch );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: cedit save cmdtable\r\n", ch );
      if( get_trust( ch ) > LEVEL_SUB_IMPLEM )
      {
         send_to_char( "Syntax: cedit <command> create [code]\r\n", ch );
         send_to_char( "Syntax: cedit <command> delete\r\n", ch );
         send_to_char( "Syntax: cedit <command> show\r\n", ch );
         send_to_char( "Syntax: cedit <command> raise\r\n", ch );
         send_to_char( "Syntax: cedit <command> lower\r\n", ch );
         send_to_char( "Syntax: cedit <command> list\r\n", ch );
         send_to_char( "Syntax: cedit <command> [field]\r\n", ch );
         send_to_char( "\r\nField being one of:\r\n", ch );
         send_to_char( "  level position log code flags\r\n", ch );
      }
      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg1, "save" ) && !str_cmp( arg2, "cmdtable" ) )
   {
      save_commands(  );
      send_to_char( "Saved.\r\n", ch );
      return;
   }

   command = find_command( arg1, FALSE );

   if( get_trust( ch ) > LEVEL_SUB_IMPLEM && !str_cmp( arg2, "create" ) )
   {
      if( command )
      {
         send_to_char( "That command already exists!\r\n", ch );
         return;
      }
      CREATE( command, CMDTYPE, 1 );
      command->name = strdup( arg1 );
      command->level = get_trust( ch );
      if( *argument )
         one_argument( argument, arg2 );
      else
         snprintf( arg2, MAX_INPUT_LENGTH+3, "do_%s", arg1 );
      command->do_fun = skill_function( arg2 );
      command->fun_name = strdup( arg2 );
      add_command( command );
      send_to_char( "Command added.\r\n", ch );
      if( command->do_fun == skill_notfound )
         ch_printf( ch, "Code %s not found.  Set to no code.\r\n", arg2 );
      return;
   }

   if( !command )
   {
      send_to_char( "Command not found.\r\n", ch );
      return;
   }
   else if( command->level > get_trust( ch ) )
   {
      send_to_char( "You cannot touch this command.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Command:  %s\r\nLevel:    %d\r\nPosition: %d\r\nLog:      %d\r\nCode:     %s\r\nFlags:     %s\r\n",
                 command->name, command->level, command->position, command->log, command->fun_name, flag_string( command->flags, cmd_flags )  );
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
      send_to_char( "Deleted.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "code" ) )
   {
      DO_FUN *fun = skill_function( argument );

      if( fun == skill_notfound )
      {
         send_to_char( "Code not found.\r\n", ch );
         return;
      }
      command->do_fun = fun;
      DISPOSE( command->fun_name );
      command->fun_name = strdup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "level" ) )
   {
      int level = atoi( argument );

      if( ( level < 0 || level > get_trust( ch ) ) )
      {
         send_to_char( "Level out of range.\r\n", ch );
         return;
      }

      if( level > command->level && command->do_fun == do_switch )
      {
         command->level = level;
         check_switches( FALSE );
      }
      else
         command->level = level;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "log" ) )
   {
      int clog = atoi( argument );

      if( clog < 0 || clog > LOG_COMM )
      {
         send_to_char( "Log out of range.\r\n", ch );
         return;
      }
      command->log = clog;
      send_to_char( "Done.\r\n", ch );
      return;
   }
    if( !str_cmp( arg2, "raise" ) )
    {
        CMDTYPE *tmp, *tmp_next;
        int hash = command->name[0] % 126;

        if( ( tmp = command_hash[hash] ) == command )
        {
            send_to_char( "That command is already at the top.\r\n", ch );
            return;
        }
        if( tmp->next == command )
        {
            command_hash[hash] = command;
            tmp_next = tmp->next;
            tmp->next = command->next;
            command->next = tmp;
            ch_printf( ch, "Moved %s above %s.\r\n", command->name, command->next->name );
            return;
        }
        for( ; tmp; tmp = tmp->next )
        {
            tmp_next = tmp->next;
            if( tmp_next->next == command )
            {
                tmp->next = command;
                tmp_next->next = command->next;
                command->next = tmp_next;
                ch_printf( ch, "Moved %s above %s.\r\n", command->name, command->next->name );
                return;
            }
        }
        send_to_char( "ERROR -- Not Found!\r\n", ch );
        return;
    }
    if( !str_cmp( arg2, "lower" ) )
    {
        CMDTYPE *tmp, *tmp_next;
        int hash = command->name[0] % 126;

        if( command->next == NULL )
        {
            send_to_char( "That command is already at the bottom.\r\n", ch );
            return;
        }
        tmp = command_hash[hash];
        if( tmp == command )
        {
            tmp_next = tmp->next;
            command_hash[hash] = command->next;
            command->next = tmp_next->next;
            tmp_next->next = command;

            ch_printf( ch, "Moved %s below %s.\r\n", command->name, tmp_next->name );
            return;
        }
        for( ; tmp; tmp = tmp->next )
        {
            if( tmp->next == command )
            {
                tmp_next = command->next;
                tmp->next = tmp_next;
                command->next = tmp_next->next;
                tmp_next->next = command;

                ch_printf( ch, "Moved %s below %s.\r\n", command->name, tmp_next->name );
                return;
            }
        }
        send_to_char( "ERROR -- Not Found!\r\n", ch );
        return;
    }
    if( !str_cmp( arg2, "list" ) )
    {
        CMDTYPE *tmp;
        int hash = command->name[0] % 126;

        pager_printf( ch, "Priority placement for [%s]:\r\n", command->name );
        for( tmp = command_hash[hash]; tmp; tmp = tmp->next )
        {
            if( tmp == command )
                set_pager_color( AT_GREEN, ch );
            else
                set_pager_color( AT_PLAIN, ch );
            pager_printf( ch, "  %s\r\n", tmp->name );
        }
        return;
    }
    if( !str_cmp( arg2, "flags" ) )
    {
        int flag;
        if( is_number( argument ) )
            flag = atoi( argument );
        else
            flag = get_cmdflag( argument );
        if( flag < 0 || flag >= 32 )
        {
            if( is_number( argument ) )
                send_to_char( "Invalid flag: range is from 0 to 31.\r\n", ch );
            else
                ch_printf( ch, "Unknown flag %s.\n", argument );
            return;
        }

        TOGGLE_BIT( command->flags, 1 << flag );
        send_to_char( "Command flags updated.\r\n", ch );
        return;
    }

   if( !str_cmp( arg2, "position" ) )
   {
      int position = atoi( argument );

      if( position < 0 || position > POS_DRAG )
      {
         send_to_char( "Position out of range.\r\n", ch );
         return;
      }
      command->position = position;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      CMDTYPE *checkcmd;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\r\n", ch );
         return;
      }
      if( ( checkcmd = find_command( arg1, TRUE ) ) != NULL )
      {
         ch_printf( ch, "There is already a command named %s.\r\n", arg1 );
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
      command->name = strdup( arg1 );
      if( relocate )
         add_command( command );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * display usage message 
    */
   do_cedit( ch, "" );
}

/* Pfile Restore by Tawnos */
void do_restorefile( CHAR_DATA * ch, const char *argument )
{
   char buf[256];
   char buf2[256];
   char fname[256];
   char arg[MAX_INPUT_LENGTH];
   struct stat fst;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Restore what player file?\r\n", ch );
      return;
   }

   snprintf( fname, 256, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), capitalize( arg ) );
   if( stat( fname, &fst ) != -1 )
   {
      snprintf( buf2, 256,"%s%c/%s", PLAYER_DIR, tolower( arg[0] ), capitalize( arg ) );
      snprintf( buf, 256, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), capitalize( arg ) );
      rename( buf, buf2 );
      send_to_char( "Player data restored.\r\n", ch );
      return;
   }
   send_to_char( "No Such Backup File.\r\n", ch );
}

void do_fslay( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );
   one_argument( argument, arg2 );
   if( arg[0] == '\0' )
   {
      send_to_char( "FSlay whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "You can't fool yourself, you know.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char
         ( "If they have any sort of mental capacity, they would realize you would be unable to slay them anyways. So why don't you just stop what you're doing and go play with some Duplo blocks, before we all succumb to complete and utter mindlessness from your total lack of intelligence.\r\n",
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
         send_to_char( "Ok.\r\n", ch );
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
      bug( "%s: NULL ch in ostat_plus 	File: %s 	Line: %d", __func__, __FILE__, __LINE__ );
      return;
   }
   if( !obj )
   {
      bug( "%s: NULL obj in ostat_plus File: %s	Line: %d", __func__, __FILE__, __LINE__ );
      return;
   }

   /******
	* A more informative ostat, so You actually know what those obj->value[x] mean
	* without looking in the code for it. Combines parts of look, examine, the
	* identification spell, and things that were never seen.. Probably overkill
	* on most things, but I'm lazy and hate digging through code to see what
	* value[x] means... -Druid
	******/
   send_to_char( "&cAdditional Object information\r\n", ch );
   switch ( obj->item_type )
   {
      default:
         ch_printf( ch, "&cSorry, No additional information available.\r\n" );
         break;
      case ITEM_LIGHT:
         ch_printf( ch, "&GValue[&W2&G] Hours left: &W" );
         if( obj->value[2] >= 0 )
            ch_printf( ch, "%d\r\n", obj->value[2] );
         else
            ch_printf( ch, "Infinite\r\n" );
         break;
      case ITEM_POTION:
      case ITEM_PILL:
      case ITEM_SCROLL:
         ch_printf( ch, "&GValue[&W0&G] Spell Level: &W%d\r\n", obj->value[0] );
         for( x = 1; x <= 3; x++ )
         {
            if( obj->value[x] >= 0 && ( sktmp = get_skilltype( obj->value[x] ) ) != NULL )
               ch_printf( ch, "&GValue[&W%d&G] Spell (&W%d&c): &W%s\r\n", x, obj->value[x], sktmp->name );
            else
               ch_printf( ch, "&GValue[&W%d&G] Spell: &WNone\r\n", x );
         }
         if( obj->item_type == ITEM_PILL )
            ch_printf( ch, "&GValue[&W4&G] Food Value: &W%d\r\n", obj->value[4] );
         break;
      case ITEM_SALVE:
      case ITEM_WAND:
      case ITEM_STAFF:
         ch_printf( ch, "&GValue[&W0&G] Spell Level: &W%d\r\n", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Max Charges: &W%d\r\n", obj->value[1] );
         ch_printf( ch, "&GValue[&W2&G] Charges Remaining: &W%d\r\n", obj->value[2] );
         if( obj->item_type != ITEM_SALVE )
         {
            if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
               ch_printf( ch, "&GValue[&W3&G] Spell (&W%d&c): &W%s\r\n", obj->value[3], sktmp->name );
            else
               ch_printf( ch, "&GValue[&W3&G] Spell: &WNone\r\n" );
            break;
         }
         ch_printf( ch, "&GValue[&W3&G] Delay (beats): &W%d\r\n", obj->value[3] );
         for( x = 4; x <= 5; x++ )
         {
            if( obj->value[x] >= 0 && ( sktmp = get_skilltype( obj->value[x] ) ) != NULL )
               ch_printf( ch, "&GValue[&W%d&G] Spell (&W%d&c): &W%s\r\n", x, obj->value[x], sktmp->name );
            else
               ch_printf( ch, "&GValue[&W%d&G] Spell: &WNone\r\n", x );
         }
         break;
      case ITEM_WEAPON:
         ch_printf( ch, "&GValue[&W0&G] Condition:   &W%d\r\n", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Num Dice:    &W%d\r\n", obj->value[1] );
         ch_printf( ch, "&GValue[&W2&G] Size Dice:   &W%d\r\n", obj->value[2] );
         ch_printf( ch, "&GValue[&W3&G] Weapon Type: " );
         if( obj->value[3] == 0 )
            ch_printf( ch, "&WGeneral&W\r\n" );
         else if( obj->value[3] == WEAPON_VIBRO_BLADE )
            ch_printf( ch, "&WVibro-Blade&W\r\n" );
         else if( obj->value[3] == WEAPON_BOWCASTER )
            ch_printf( ch, "&WBowcaster&W\r\n" );
         else if( obj->value[3] == WEAPON_FORCE_PIKE )
            ch_printf( ch, "&WForce-Pike&W\r\n" );
         else if( obj->value[3] == WEAPON_BLASTER )
            ch_printf( ch, "&WBlaster&W\r\n" );
         else if( obj->value[3] == WEAPON_LIGHTSABER || obj->value[3] == WEAPON_DUAL_LIGHTSABER )
            ch_printf( ch, "&WLightsaber&W\r\n" );
         else
            ch_printf( ch, "&WNo Current Weapon Type Set&W\r\n" );

         ch_printf( ch, "&GValue[&W-&G] Low Damage:  &W%d\r\n", obj->value[1] );
         ch_printf( ch, "&GValue[&W-&G] Max Damage:  &W%d\r\n", obj->value[2] );
         ch_printf( ch, "&GValue[&W-&G] Ave Damage:  &W%d\r\n", ( obj->value[1] + obj->value[2] ) / 2 );
         ch_printf( ch, "&GValue[&W4&G] Charges:     &W%d\r\n", obj->value[4] );
         ch_printf( ch, "&GValue[&W5&G] Max Charges: &W%d\r\n", obj->value[5] );
         break;
      case ITEM_ARMOR:
         ch_printf( ch, "&GValue[&W0&G] Current AC: &W%d\r\n", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Original AC: &W%d\r\n", obj->value[1] );
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
            strlcpy( buf, "Superb condition", MAX_STRING_LENGTH );
         else if( dam == 9 )
            strlcpy( buf, "Very good condition", MAX_STRING_LENGTH );
         else if( dam == 8 )
            strlcpy( buf, "Good shape", MAX_STRING_LENGTH );
         else if( dam == 7 )
            strlcpy( buf, "Showing a bit of wear", MAX_STRING_LENGTH );
         else if( dam == 6 )
            strlcpy( buf, "A little run down", MAX_STRING_LENGTH );
         else if( dam == 5 )
            strlcpy( buf, "In need of repair", MAX_STRING_LENGTH );
         else if( dam == 4 )
            strlcpy( buf, "In great need of repair", MAX_STRING_LENGTH );
         else if( dam == 3 )
            strlcpy( buf, "In dire need of repair", MAX_STRING_LENGTH );
         else if( dam == 2 )
            strlcpy( buf, "Very badly worn", MAX_STRING_LENGTH );
         else if( dam == 1 )
            strlcpy( buf, "Practically worthless", MAX_STRING_LENGTH );
         else if( dam <= 0 )
            strlcpy( buf, "Broken", MAX_STRING_LENGTH );
         strlcat( buf, "\r\n", MAX_STRING_LENGTH );
         send_to_char( buf, ch );
         break;
         /*
          * Bug Fix 7/9/00 -Druid
          */
      case ITEM_FOOD:
         ch_printf( ch, "&GValue[&W0&G] Food Value: &W%d\r\n", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Condition (&W%d&c): &W", obj->value[1] );
         if( obj->timer > 0 && obj->value[1] > 0 )
            dam = ( obj->timer * 10 ) / obj->value[1];
         else
            dam = 10;
         if( dam >= 10 )
            strlcpy( buf, "It is fresh.", MAX_STRING_LENGTH );
         else if( dam == 9 )
            strlcpy( buf, "It is nearly fresh.", MAX_STRING_LENGTH );
         else if( dam == 8 )
            strlcpy( buf, "It is perfectly fine.", MAX_STRING_LENGTH );
         else if( dam == 7 )
            strlcpy( buf, "It looks good.", MAX_STRING_LENGTH );
         else if( dam == 6 )
            strlcpy( buf, "It looks ok.", MAX_STRING_LENGTH );
         else if( dam == 5 )
            strlcpy( buf, "It is a little stale.", MAX_STRING_LENGTH );
         else if( dam == 4 )
            strlcpy( buf, "It is a bit stale.", MAX_STRING_LENGTH );
         else if( dam == 3 )
            strlcpy( buf, "It smells slightly off.", MAX_STRING_LENGTH );
         else if( dam == 2 )
            strlcpy( buf, "It smells quite rank.", MAX_STRING_LENGTH );
         else if( dam == 1 )
            strlcpy( buf, "It smells revolting!", MAX_STRING_LENGTH );
         else if( dam <= 0 )
            strlcpy( buf, "It is crawling with maggots!", MAX_STRING_LENGTH );
         strlcat( buf, "\r\n", MAX_STRING_LENGTH );
         send_to_char( buf, ch );
         if( obj->value[4] )
            ch_printf( ch, "&GValue[&W4&G] Timer: &W%d\r\n", obj->value[4] );
         break;
      case ITEM_DRINK_CON:
         ch_printf( ch, "&GValue[&W0&G] Capacity: &W%d\r\n", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Quantity Left (&W%d&c): &W", obj->value[1] );
         if( obj->value[1] > obj->value[0] )
            ch_printf( ch, "More than Full\r\n" );
         else if( obj->value[1] == obj->value[0] )
            ch_printf( ch, "Full\r\n" );
         else if( obj->value[1] >= ( 3 * obj->value[0] / 4 ) )
            ch_printf( ch, "Almost Full\r\n" );
         else if( obj->value[1] > ( obj->value[0] / 2 ) )
            ch_printf( ch, "More than half full\r\n" );
         else if( obj->value[1] == ( obj->value[0] / 2 ) )
            ch_printf( ch, "Half full\r\n" );
         else if( obj->value[1] >= ( obj->value[0] / 4 ) )
            ch_printf( ch, "Less than half full\r\n" );
         else if( obj->value[1] >= 1 )
            ch_printf( ch, "Almost Empty\r\n" );
         else
            ch_printf( ch, "Empty\r\n" );
         ch_printf( ch, "&GValue[&W2&G] Liquid Type (&W%d&c): &W%s\r\n", obj->value[2], liq_table[obj->value[2]].liq_name );
         ch_printf( ch, "&cLiquid color: &W%s\r\n", liq_table[obj->value[2]].liq_color );
         if( liq_table[obj->value[2]].liq_affect[COND_DRUNK] != 0 )
            ch_printf( ch, "&cAffects Drunkeness by: &W%d\r\n", liq_table[obj->value[2]].liq_affect[COND_DRUNK] );
         if( liq_table[obj->value[2]].liq_affect[COND_FULL] != 0 )
            ch_printf( ch, "&cAffects Fullness by: &W%d\r\n", liq_table[obj->value[2]].liq_affect[COND_FULL] );
         if( liq_table[obj->value[2]].liq_affect[COND_THIRST] != 0 )
            ch_printf( ch, "&cAffects Thirst by: &W%d\r\n", liq_table[obj->value[2]].liq_affect[COND_THIRST] );
         if( liq_table[obj->value[2]].liq_affect[COND_BLOODTHIRST] != 0 )
            ch_printf( ch, "&cAffects BloodThirst by: &W%d\r\n", liq_table[obj->value[2]].liq_affect[COND_BLOODTHIRST] );
         ch_printf( ch, "&GValue[&W3&G] Poisoned (&W%d&c): &W%s\r\n", obj->value[3], obj->value[3] >= 1 ? "Yes" : "No" );
         break;
      case ITEM_HERB:
         ch_printf( ch, "&GValue[&W1&G] Charges: &W%d\r\n", obj->value[1] );
         ch_printf( ch, "&GValue[&W2&G] Herb #: &W%d\r\n", obj->value[2] );
         break;
      case ITEM_CONTAINER:
         ch_printf( ch, "&GValue[&W0&G] Capacity (&W%d&c): &W", obj->value[0] );
         ch_printf( ch, "%s\r\n",
                    obj->value[0] < 76 ? "Small capacity" :
                    obj->value[0] < 150 ? "Small to medium capacity" :
                    obj->value[0] < 300 ? "Medium capacity" :
                    obj->value[0] < 550 ? "Medium to large capacity" :
                    obj->value[0] < 751 ? "Large capacity" : "Giant capacity" );
         ch_printf( ch, "&GValue[&W1&G] Flags (&W%d&c):&W", obj->value[1] );
         if( obj->value[1] <= 0 )
            ch_printf( ch, " None\r\n" );
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
            ch_printf( ch, "\r\n" );
         }
         ch_printf( ch, "&GValue[&W2&G] Key Vnum: &W" );
         if( obj->value[2] <= 0 )
            ch_printf( ch, "None\r\n" );
         else
            ch_printf( ch, "%d\r\n", obj->value[2] );
         ch_printf( ch, "&GValue[&W3&G] Condition: &W%d\r\n", obj->value[3] );
         if( obj->timer )
            ch_printf( ch, "&cObject Timer, Time Left: &W%d\r\n", obj->timer );
         break;
      case ITEM_MONEY:
         ch_printf( ch, "&GValue[&W0&G] # of Coins: &W%d\r\n", obj->value[0] );
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
         ch_printf( ch, "&GValue[&W0&G] Charges Remaining: &W%d\r\n", obj->value[0] );
         ch_printf( ch, "&GValue[&W1&G] Type (&W%d&c): &W", obj->value[1] );
         switch ( obj->value[1] )
         {
            default:
               strlcpy( buf, "Hit by a trap", MAX_STRING_LENGTH );
               send_to_char( "Default Generic Trap\r\n", ch );
               ch_printf( ch, "&cDoes Damage from (&W%d&c) to (&W%d&c)\r\n", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_GAS:
               strlcpy( buf, "Surrounded by a green cloud of gas", MAX_STRING_LENGTH );
               send_to_char( "Poisoned Gas\r\n", ch );
               ch_printf( ch, "&cCasts spell: &WPoison\r\n" );
               break;
            case TRAP_TYPE_POISON_DART:
               strlcpy( buf, "Hit by a dart", MAX_STRING_LENGTH );
               send_to_char( "Poisoned Dart\r\n", ch );
               send_to_char( "&cCasts spell: &WPoison\r\n", ch );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\r\n", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_NEEDLE:
               strlcpy( buf, "Pricked by a needle", MAX_STRING_LENGTH );
               send_to_char( "Poisoned Needle\r\n", ch );
               send_to_char( "&cCasts spell: &WPoison\r\n", ch );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\r\n", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_DAGGER:
               strlcpy( buf, "Stabbed by a dagger", MAX_STRING_LENGTH );
               send_to_char( "Poisoned Dagger\r\n", ch );
               send_to_char( "&cCasts spell: &WPoison\r\n", ch );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\r\n", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_POISON_ARROW:
               strlcpy( buf, "Struck with an arrow", MAX_STRING_LENGTH );
               send_to_char( "Poisoned Arrow\r\n", ch );
               send_to_char( "&cCasts spell: &WPoison\r\n", ch );
               ch_printf( ch, "&cOR Does Damage from (&W%d&c) to (&W%d&c)\r\n", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_BLINDNESS_GAS:
               strlcpy( buf, "Surrounded by a red cloud of gas", MAX_STRING_LENGTH );
               send_to_char( "Blinding Gas\r\n", ch );
               send_to_char( "&cCasts spell: &WBlind\r\n", ch );
               break;
            case TRAP_TYPE_SLEEPING_GAS:
               strlcpy( buf, "Surrounded by a yellow cloud of gas", MAX_STRING_LENGTH );
               send_to_char( "Sleeping Gas\r\n", ch );
               send_to_char( "&cCasts spell: &WSleep\r\n", ch );
               break;
            case TRAP_TYPE_FLAME:
               strlcpy( buf, "Struck by a burst of flame", MAX_STRING_LENGTH );
               send_to_char( "Flame\r\n", ch );
               send_to_char( "&cCasts spell: &WFireball\r\n", ch );
               break;
            case TRAP_TYPE_EXPLOSION:
               strlcpy( buf, "Hit by an explosion", MAX_STRING_LENGTH );
               send_to_char( "Explosion\r\n", ch );
               send_to_char( "&cCasts spell: &WFireball\r\n", ch );
               break;
            case TRAP_TYPE_ACID_SPRAY:
               strlcpy( buf, "Covered by a spray of acid", MAX_STRING_LENGTH );
               send_to_char( "Acid Spray\r\n", ch );
               send_to_char( "&cCasts spell: &WAcid Blast\r\n", ch );
               break;
            case TRAP_TYPE_ELECTRIC_SHOCK:
               strlcpy( buf, "Suddenly shocked", MAX_STRING_LENGTH );
               send_to_char( "Electric Shock\r\n", ch );
               ch_printf( ch, "&cDoes Damage from (&W%d&c) to (&W%d&c)\r\n", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case TRAP_TYPE_BLADE:
               strlcpy( buf, "Sliced by a razor sharp blade", MAX_STRING_LENGTH );
               send_to_char( "Sharp Blade\r\n", ch );
               ch_printf( ch, "&cDoes Damage from (&W%d&c) to (&W%d&c)\r\n", obj->value[2], ( obj->value[2] * 2 ) );
               break;
            case ITEM_KEY:
               ch_printf( ch, "&GValue[&W0&G] Lock #: &W%d\r\n", obj->value[0] );
               break;
            case ITEM_BLOOD:
               ch_printf( ch, "&GValue[&W1&G] Amount Remaining: &W%d\r\n", obj->value[1] );
               if( obj->timer )
                  ch_printf( ch, "&cObject Timer, Time Left: &W%d\r\n", obj->timer );
               break;
         }
   }
}

void do_reward( CHAR_DATA * ch, const char *argument )
{
   int amount;
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Reward who how many points?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give a mob RP points!\r\n", ch );
      return;
   }

   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "Immortals have no use for such things!\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "How many RP points do you wish to give?\r\n", ch );
      return;
   }

   amount = atoi( argument );

   if( amount < -1 )
   {
      send_to_char( "You can only give positive points, or -1.\r\n", ch );
      return;
   }

   if( !victim->rppoints )
      victim->rppoints = amount;
   else
      victim->rppoints += amount;

   if( amount >= 1 )
   {
      ch_printf( victim, "&GCheer! The gods have rewarded you with %d RP point%s!\r\n", amount, amount > 1 ? "s" : "" );
      ch_printf( ch, "&GThey have been rewarded with with %d RP point%s.\r\n", amount, amount > 1 ? "s" : "" );
   }
   else if( amount < 0 )
   {
      ch_printf( victim, "&RCurses! The gods have removed an RP point!\r\n" );
      ch_printf( ch, "They have had one RP point removed.\r\n" );
   }

}

void do_changes( CHAR_DATA * ch, const char *argument )
{
   FILE *fpList;
   const char *buf;
   char display[MAX_STRING_LENGTH];
   int i = 0;

   if( IS_NPC( ch ) )
      return;

   if( ( fpList = fopen( CHANGE_FILE, "r" ) ) == NULL )
   {
      send_to_char( "Something wen't wrong. The imms have been notified.\r\n", ch );
      bug( "%s: Unable to open change list", __func__ );
      return;
   }

   send_to_char( "&g::::::::::::::::::::::::::::::::::&w&W[ &GCHANGES &W]&g::::::::::::::::::::::::::::::::::\r\n", ch );
   send_to_char( "&w&W-------------------------------------------------------------------------------\r\n", ch );

   for( ;; )
   {
      if( feof( fpList ) )
         break;
      buf = feof( fpList ) ? "End" : fread_string( fpList );
      if( !str_cmp( buf, "End" ) || buf[0] == '\0' )
         break;
      if( strlen( buf ) < 3 )
         break;

      snprintf( display, MAX_STRING_LENGTH, "&w&W[&G*&W] &g%s", buf );
      send_to_char( strlinwrp( display, 78 ), ch );
      send_to_char( "&w&W-------------------------------------------------------------------------------\r\n", ch );
      ++i;

   }
   send_to_char( "\r\n", ch );
   FCLOSE( fpList );
}

void do_addchange( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   if( argument[0] == '\0' )
   {
      send_to_char( "&WSyntax: addchange <change>\r\n", ch );
      return;
   }

   smash_tilde( argument );
   snprintf( buf, MAX_STRING_LENGTH, "%s~", argument );
   append_to_file( CHANGE_FILE, buf );

   snprintf( buf2, MAX_STRING_LENGTH, "<li> %s<br><br>", argument );
   prepend_to_file( CHANGEHTML_FILE, buf2 );

   send_to_char( "&GChange successfully added.\r\n", ch );
   return;
}

void do_std( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int desc;
   DESCRIPTOR_DATA *d;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Syntax: std <descriptor> <text>\r\n", ch );
      return;
   }

   desc = atoi( arg );

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->descriptor == desc )
      {
         send_to_desc_color( argument, d );
         send_to_char( "&WOk.\r\n", ch );
         return;
      }
   }

   send_to_char( "No such descriptor\r\n", ch );
   return;

}
