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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"

#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

CLAN_DATA *first_clan;
CLAN_DATA *last_clan;

SENATE_DATA *first_senator;
SENATE_DATA *last_senator;

PLANET_DATA *first_planet;
PLANET_DATA *last_planet;

GUARD_DATA *first_guard;
GUARD_DATA *last_guard;

/* local routines */
void fread_clan args( ( CLAN_DATA * clan, FILE * fp ) );
bool load_clan_file args( ( char *clanfile ) );
void write_clan_list args( ( void ) );
bool load_planet_file args( ( char *planetfile ) );


/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
   CLAN_DATA *clan;

   for( clan = first_clan; clan; clan = clan->next )
      if( !str_cmp( name, clan->name ) )
         return clan;
   return NULL;
}


void do_remclan( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;

   if( ( clan = get_clan( argument ) ) == NULL )
   {
      send_to_char( "No such clan.\n\r", ch );
      return;
   }

   UNLINK( clan, first_clan, last_clan, next, prev );
   STRFREE( clan->name );
   DISPOSE( clan );
   write_clan_list(  );
}

void write_clan_list(  )
{
   CLAN_DATA *tclan;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open clan.lst for writing!\n\r", 0 );
      return;
   }
   for( tclan = first_clan; tclan; tclan = tclan->next )
      fprintf( fpout, "%s\n", tclan->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA * clan )
{
   FILE *fp;
   char filename[256];
   char buf[MAX_STRING_LENGTH];

   if( !clan )
   {
      bug( "save_clan: null clan pointer!", 0 );
      return;
   }

   if( !clan->filename || clan->filename[0] == '\0' )
   {
      sprintf( buf, "save_clan: %s has no filename", clan->name );
      bug( buf, 0 );
      return;
   }

   sprintf( filename, "%s%s", CLAN_DIR, clan->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_clan: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#CLAN\n" );
      fprintf( fp, "Name         %s~\n", clan->name );
      fprintf( fp, "Shortname    %s~\n", clan->shortname );
      fprintf( fp, "Filename     %s~\n", clan->filename );
      fprintf( fp, "Description  %s~\n", clan->description );
      fprintf( fp, "Leader       %s~\n", clan->leader );
      fprintf( fp, "NumberOne    %s~\n", clan->number1 );
      fprintf( fp, "NumberTwo    %s~\n", clan->number2 );
      fprintf( fp, "PKills       %d\n", clan->pkills );
      fprintf( fp, "PDeaths      %d\n", clan->pdeaths );
      fprintf( fp, "MKills       %d\n", clan->mkills );
      fprintf( fp, "MDeaths      %d\n", clan->mdeaths );
      fprintf( fp, "Type         %d\n", clan->clan_type );
      fprintf( fp, "Members      %d\n", clan->members );
      fprintf( fp, "Board        %d\n", clan->board );
      fprintf( fp, "Storeroom    %d\n", clan->storeroom );
      fprintf( fp, "GuardOne     %d\n", clan->guard1 );
      fprintf( fp, "atwar	   %s~\n", clan->atwar );
      fprintf( fp, "GuardTwo     %d\n", clan->guard2 );
      fprintf( fp, "PatrolOne    %d\n", clan->patrol1 );
      fprintf( fp, "PatrolTwo    %d\n", clan->patrol2 );
      fprintf( fp, "Acro         %s~\n", clan->acro );
      fprintf( fp, "TrooperOne   %d\n", clan->trooper1 );
      fprintf( fp, "TrooperTwo   %d\n", clan->trooper2 );
      fprintf( fp, "Troops       %d\n", clan->troops );
      fprintf( fp, "Funds        %ld\n", clan->funds );
      fprintf( fp, "Jail         %d\n", clan->jail );
      if( clan->mainclan )
         fprintf( fp, "MainClan     %s~\n", clan->mainclan->name );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}

/*
 * Read in actual clan data.
 */
void fread_clan( CLAN_DATA * clan, FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "atwar", clan->atwar, fread_string( fp ) );
            KEY( "Acro", clan->acro, fread_string( fp ) );
            break;

         case 'B':
            KEY( "Board", clan->board, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Description", clan->description, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !clan->name )
                  clan->name = STRALLOC( "" );
               if( !clan->leader )
                  clan->leader = STRALLOC( "" );
               if( !clan->description )
                  clan->description = STRALLOC( "" );
               if( !clan->number1 )
                  clan->number1 = STRALLOC( "" );
               if( !clan->number2 )
                  clan->number2 = STRALLOC( "" );
               if( !clan->tmpstr )
                  clan->tmpstr = STRALLOC( "" );
               if( !clan->atwar )
                  clan->atwar = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Funds", clan->funds, fread_number( fp ) );
            KEY( "Filename", clan->filename, fread_string_nohash( fp ) );
            break;

         case 'G':
            KEY( "GuardOne", clan->guard1, fread_number( fp ) );
            KEY( "GuardTwo", clan->guard2, fread_number( fp ) );
            break;

         case 'J':
            KEY( "Jail", clan->jail, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Leader", clan->leader, fread_string( fp ) );
            break;

         case 'M':
            KEY( "MDeaths", clan->mdeaths, fread_number( fp ) );
            KEY( "Members", clan->members, fread_number( fp ) );
            KEY( "MKills", clan->mkills, fread_number( fp ) );
            KEY( "MainClan", clan->tmpstr, fread_string( fp ) );
            break;

         case 'N':
            KEY( "Name", clan->name, fread_string( fp ) );
            KEY( "NumberOne", clan->number1, fread_string( fp ) );
            KEY( "NumberTwo", clan->number2, fread_string( fp ) );
            break;

         case 'P':
            KEY( "PDeaths", clan->pdeaths, fread_number( fp ) );
            KEY( "PKills", clan->pkills, fread_number( fp ) );
            KEY( "PatrolOne", clan->patrol1, fread_number( fp ) );
            KEY( "PatrolTwo", clan->patrol2, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Storeroom", clan->storeroom, fread_number( fp ) );
            KEY( "Shortname", clan->shortname, fread_string( fp ) );
            break;

         case 'T':
            KEY( "Type", clan->clan_type, fread_number( fp ) );
            KEY( "TrooperOne", clan->trooper1, fread_number( fp ) );
            KEY( "TrooperTwo", clan->trooper2, fread_number( fp ) );
            KEY( "Troops", clan->troops, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_clan: no match: %s", word );
         bug( buf, 0 );
      }

   }
}

/*
 * Load a clan file
 */

bool load_clan_file( char *clanfile )
{
   char filename[256];
   CLAN_DATA *clan;
   FILE *fp;
   bool found;

   CREATE( clan, CLAN_DATA, 1 );
   clan->next_subclan = NULL;
   clan->prev_subclan = NULL;
   clan->last_subclan = NULL;
   clan->first_subclan = NULL;
   clan->mainclan = NULL;

   found = FALSE;
   sprintf( filename, "%s%s", CLAN_DIR, clanfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      found = TRUE;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_clan_file: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "CLAN" ) )
         {
            fread_clan( clan, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "Load_clan_file: bad section: %s.", word );
            bug( buf, 0 );
            break;
         }
      }
      fclose( fp );
   }

   if( found )
   {
      ROOM_INDEX_DATA *storeroom;

      LINK( clan, first_clan, last_clan, next, prev );

      if( clan->storeroom == 0 || ( storeroom = get_room_index( clan->storeroom ) ) == NULL )
      {
         log_string( "Storeroom not found" );
         return found;
      }

      sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename );
      if( ( fp = fopen( filename, "r" ) ) != NULL )
      {
         int iNest;
         OBJ_DATA *tobj, *tobj_next;

         log_string( "Loading clan storage room" );
         rset_supermob( storeroom );
         for( iNest = 0; iNest < MAX_NEST; iNest++ )
            rgObjNest[iNest] = NULL;

         for( ;; )
         {
            char letter;
            char *word;

            letter = fread_letter( fp );
            if( letter == '*' )
            {
               fread_to_eol( fp );
               continue;
            }

            if( letter != '#' )
            {
               bug( "Load_clan_vault: # not found.", 0 );
               bug( clan->name, 0 );
               break;
            }

            word = fread_word( fp );
            if( !str_cmp( word, "OBJECT" ) ) /* Objects  */
               fread_obj( supermob, fp, OS_CARRY );
            else if( !str_cmp( word, "END" ) )  /* Done     */
               break;
            else
            {
               bug( "Load_clan_vault: bad section.", 0 );
               bug( clan->name, 0 );
               break;
            }
         }
         fclose( fp );
         for( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
         {
            tobj_next = tobj->next_content;
            obj_from_char( tobj );
            obj_to_room( tobj, storeroom );
         }
         release_supermob(  );
      }
      else
         log_string( "Cannot open clan vault" );
   }
   else
      DISPOSE( clan );

   return found;
}

/*
 * Load in all the clan files.
 */
void load_clans(  )
{
   FILE *fpList;
   char *filename;
   char clanlist[256];
   char buf[MAX_STRING_LENGTH];
   CLAN_DATA *clan;
   CLAN_DATA *bosclan;

   first_clan = NULL;
   last_clan = NULL;

   log_string( "Loading clans..." );

   sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
   if( ( fpList = fopen( clanlist, "r" ) ) == NULL )
   {
      perror( clanlist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( filename );
      if( filename[0] == '$' )
         break;

      if( !load_clan_file( filename ) )
      {
         sprintf( buf, "Cannot load clan file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   log_string( " Done clans\n\rSorting clans...." );

   for( clan = first_clan; clan; clan = clan->next )
   {
      if( !clan->tmpstr || clan->tmpstr[0] == '\0' )
         continue;

      bosclan = get_clan( clan->tmpstr );
      if( !bosclan )
         continue;

      LINK( clan, bosclan->first_subclan, bosclan->last_subclan, next_subclan, prev_subclan );
      clan->mainclan = bosclan;
   }

   log_string( " Done sorting" );
   return;
}

void do_make( CHAR_DATA * ch, char *argument )
{
   send_to_char( "Huh?\n\r", ch );
   return;
}

void do_induct( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "induct", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) )
      ;
   else
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Induct whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( victim->pcdata->clan )
   {
      if( victim->pcdata->clan->clan_type == CLAN_CRIME )
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your crime family!\n\r", ch );
         else
            send_to_char( "This player already belongs to an organization!\n\r", ch );
         return;
      }
      else if( victim->pcdata->clan->clan_type == CLAN_GUILD )
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your guild!\n\r", ch );
         else
            send_to_char( "This player already belongs to an organization!\n\r", ch );
         return;
      }
      else
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your organization!\n\r", ch );
         else
            send_to_char( "This player already belongs to an organization!\n\r", ch );
         return;
      }

   }

   clan->members++;
   add_member( victim->name, clan->shortname );
   victim->pcdata->clan = clan;
   STRFREE( victim->pcdata->clan_name );
   victim->pcdata->clan_name = QUICKLINK( clan->name );
   act( AT_MAGIC, "You induct $N into $t", ch, clan->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT );
   act( AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim, TO_VICT );
   save_char_obj( victim );
   return;
}

void do_outcast( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;
   DESCRIPTOR_DATA *d;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "outcast", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) )
      ;
   else
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }


   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Outcast whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Kick yourself out of your own clan?\n\r", ch );
      return;
   }

   if( victim->pcdata->clan != ch->pcdata->clan )
   {
      send_to_char( "This player does not belong to your clan!\n\r", ch );
      return;
   }


/*    if ( victim->speaking & LANG_CLAN )
        victim->speaking = LANG_BASIC;
    REMOVE_BIT( victim->speaks, LANG_CLAN );*/
   --clan->members;
    if( clan->members < 0 )
       clan->members = 0;
   if( !str_cmp( victim->name, ch->pcdata->clan->number1 ) )
   {
      STRFREE( ch->pcdata->clan->number1 );
      ch->pcdata->clan->number1 = STRALLOC( "" );
   }
   if( !str_cmp( victim->name, ch->pcdata->clan->number2 ) )
   {
      STRFREE( ch->pcdata->clan->number2 );
      ch->pcdata->clan->number2 = STRALLOC( "" );
   }

   act( AT_MAGIC, "You outcast $N from $t.", ch, clan->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n outcasts $N from $t.", ch, clan->name, victim, TO_ROOM );
   act( AT_MAGIC, "$n outcasts you from $t.", ch, clan->name, victim, TO_VICT );

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
      {
         if( IS_NPC( d->character ) )
            continue;
         if( victim == d->character )
            continue;
         if( !d->character->pcdata->clan )
            continue;

         if( !str_cmp( victim->pcdata->clan->name, d->character->pcdata->clan->name ) )
            ch_printf( d->character, "&B%s has been outcast from %s.\n\r", victim->name, victim->pcdata->clan->name );
      }
   }


   remove_member( victim->name, victim->pcdata->clan->shortname );
   if( victim->pcdata->salary )
      victim->pcdata->salary = 0;
   victim->pcdata->clan = NULL;
   STRFREE( victim->pcdata->clan_name );
   victim->pcdata->clan_name = STRALLOC( "" );

   DISPOSE( victim->pcdata->bestowments );
   victim->pcdata->bestowments = str_dup( "" );

   save_char_obj( victim );   /* clan gets saved when pfile is saved */
   return;
}

void do_setclan( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CLAN_DATA *clan;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: setclan <clan> <field> <leader|number1|number2> <player>\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( " leader number1 number2 subclan\n\r", ch );
      send_to_char( " members board recall storage\n\r", ch );
      send_to_char( " funds trooper1 trooper2 jail", ch );
      send_to_char( " guard1 guard2 patrol1 patrol2\n\r", ch );
      if( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
      {
         send_to_char( " name filename desc shortname\n\r", ch );
      }
      return;
   }

   clan = get_clan( arg1 );
   if( !clan )
   {
      send_to_char( "No such clan.\n\r", ch );
      return;
   }

   if( !strcmp( arg2, "leader" ) )
   {
      STRFREE( clan->leader );
      clan->leader = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "subclan" ) )
   {
      CLAN_DATA *subclan;
      subclan = get_clan( argument );
      if( !subclan )
      {
         send_to_char( "Subclan is not a clan.\n\r", ch );
         return;
      }
      if( subclan->clan_type == CLAN_SUBCLAN || subclan->mainclan )
      {
         send_to_char( "Subclan is already part of another organization.\n\r", ch );
         return;
      }
      if( subclan->first_subclan )
      {
         send_to_char( "Subclan has subclans of its own that need removing first.\n\r", ch );
         return;
      }
      subclan->clan_type = CLAN_SUBCLAN;
      subclan->mainclan = clan;
      LINK( subclan, clan->first_subclan, clan->last_subclan, next_subclan, prev_subclan );
      save_clan( clan );
      save_clan( subclan );
      return;
   }
   if( !strcmp( arg2, "acro" ) )
   {
      if( strlen( argument ) > 3 )
      {
         send_to_char( "The acronym must be no more than 3 characters.\n\r", ch );
         return;
      }
      STRFREE( clan->acro );
      clan->acro = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }
   if( !strcmp( arg2, "number1" ) )
   {
      STRFREE( clan->number1 );
      clan->number1 = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "number2" ) )
   {
      STRFREE( clan->number2 );
      clan->number2 = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "board" ) )
   {
      clan->board = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "members" ) )
   {
      clan->members = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "funds" ) )
   {
      clan->funds = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "troops" ) )
   {
      clan->troops = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "storage" ) )
   {
      clan->storeroom = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "guard1" ) )
   {
      clan->guard1 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "jail" ) )
   {
      clan->jail = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "guard2" ) )
   {
      clan->guard2 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "trooper1" ) )
   {
      clan->trooper1 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "trooper2" ) )
   {
      clan->trooper2 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }
   if( !strcmp( arg2, "patrol1" ) )
   {
      clan->patrol1 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "patrol2" ) )
   {
      clan->patrol2 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
   {
      do_setclan( ch, "" );
      return;
   }

   if( !strcmp( arg2, "type" ) )
   {
      if( clan->mainclan )
      {
         UNLINK( clan, clan->mainclan->first_subclan, clan->mainclan->last_subclan, next_subclan, prev_subclan );
         clan->mainclan = NULL;
      }
      if( !str_cmp( argument, "crime" ) )
         clan->clan_type = CLAN_CRIME;
      else if( !str_cmp( argument, "guild" ) )
         clan->clan_type = CLAN_GUILD;
      else if( !str_cmp( argument, "corp" ) )
         clan->clan_type = CLAN_CORPORATION;
      else if( !str_cmp( argument, "govn" ) )
         clan->clan_type = CLAN_PLAIN;
      else if( argument[0] == '\0' )
         send_to_char( "You must pick from the following: Crime, Guild, Corperation, or Government.\n\r", ch );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "name" ) )
   {
      STRFREE( clan->name );
      clan->name = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   if( !strcmp( arg2, "filename" ) )
   {
      DISPOSE( clan->filename );
      clan->filename = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      write_clan_list(  );
      return;
   }

   if( !strcmp( arg2, "shortname" ) )
   {
      STRFREE( clan->shortname );
      clan->shortname = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      write_clan_list(  );
      return;
   }

   if( !strcmp( arg2, "desc" ) )
   {
      STRFREE( clan->description );
      clan->description = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_clan( clan );
      return;
   }

   do_setclan( ch, "" );
   return;
}

void do_showclan( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showclan <clan>\n\r", ch );
      return;
   }

   clan = get_clan( argument );
   if( !clan )
   {
      send_to_char( "No such clan.\n\r", ch );
      return;
   }

   ch_printf( ch, "%s      : %s\n\rFilename: %s\n\r",
              clan->clan_type == CLAN_CRIME ? "Crime Family " :
              clan->clan_type == CLAN_GUILD ? "Guild " : "Organization ", clan->name, clan->filename );
   ch_printf( ch, "Sname: %s\n\rDescription: %s\n\rLeader: %s\n\r", clan->shortname, clan->description, clan->leader );
   ch_printf( ch, "Number1: %s\n\rNumber2: %s\n\rPKills: %6d    PDeaths: %6d\n\r",
              clan->number1, clan->number2, clan->pkills, clan->pdeaths );
   ch_printf( ch, "MKills: %6d    MDeaths: %6d\n\r", clan->mkills, clan->mdeaths );
   ch_printf( ch, "Type: %d\n\r", clan->clan_type );
   ch_printf( ch, "Members: %3d\n\r", clan->members );
   ch_printf( ch, "Board: %5d   Jail: %5d\n\r", clan->board, clan->jail );
   ch_printf( ch, "Guard1: %5d  Guard2: %5d\n\r", clan->guard1, clan->guard2 );
   ch_printf( ch, "Patrol1: %5d  Patrol2: %5d\n\r", clan->patrol1, clan->patrol2 );
   ch_printf( ch, "Trooper1: %5d  Trooper2: %5d\n\r", clan->trooper1, clan->trooper2 );
   ch_printf( ch, "Funds: %ld\n\r", clan->funds );
   return;
}

void do_buytroops( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   int num, cost;
   char buf[MAX_STRING_LENGTH];
   if( argument[0] == '\0' )
      num = 1;
   else
      num = atoi( argument );

   cost = ( num * 5000 );




   clan = ch->pcdata->clan;
   if( clan == NULL )
   {
      send_to_char( "You are not a member of a clan.\n\r", ch );
      return;
   }

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "induct", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) )
      ;
   else
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   if( clan->funds < cost )
   {
      send_to_char( "You don't have enough money to purchase that many ground troops!\n\r", ch );
      return;
   }

   clan->troops += num;
   clan->funds -= cost;
   sprintf( buf, "&RYou have purchased %d ground troops for %s.\n\r", num, clan->name );
   send_to_char( buf, ch );
   save_clan( clan );
}

void do_makeclan( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makeclan <clan name>\n\r", ch );
      return;
   }

/*  Otherwise it would be possible to create multiple clans
    with the same name. Bad thing... */

   set_char_color( AT_PLAIN, ch );
   clan = get_clan( argument );
   if( clan )
   {
      send_to_char( "There is already a clan with that name.\n\r", ch );
      return;
   }

   CREATE( clan, CLAN_DATA, 1 );
   LINK( clan, first_clan, last_clan, next, prev );
   clan->next_subclan = NULL;
   clan->prev_subclan = NULL;
   clan->last_subclan = NULL;
   clan->first_subclan = NULL;
   clan->mainclan = NULL;
   clan->name = STRALLOC( argument );
   clan->description = STRALLOC( "" );
   clan->leader = STRALLOC( "" );
   clan->number1 = STRALLOC( "" );
   clan->number2 = STRALLOC( "" );
   clan->atwar = STRALLOC( "" );
   clan->acro = STRALLOC( "NNE" );
   clan->tmpstr = STRALLOC( "" );
   clan->shortname = STRALLOC( "temp" );
   send_to_char( "Done. Shortname set to temp. Be sure to reset this.", ch );
}

void do_gatherclans( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   PLANET_DATA *planet;
   int count = 0;
   int pCount = 0;
   int support;
   long revenue;

   for( clan = first_clan; clan; clan = clan->next )
   {
      if( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN
          || clan->clan_type == CLAN_CORPORATION || clan->clan_type == CLAN_PLAIN )
         continue;
      if( !str_cmp( clan->name, "Neutral" ) )
         continue;
      pCount = 0;
      support = 0;
      revenue = 0;

      for( planet = first_planet; planet; planet = planet->next )
         if( clan == planet->governed_by )
         {
            support += planet->pop_support;
            pCount++;
            revenue += get_taxes( planet );
         }

      if( pCount > 1 )
         support /= pCount;

      ch_printf( ch, "&WOrganization: &Y%s\n\r", clan->name );
      ch_printf( ch, "  &WPlanets: &O%-2d       &WAvg Pop Support: &O%-3d&W    &WRevenue: &O%-10ld\n\r",
                 pCount, support, revenue );
      ch_printf( ch, "  &WDivision             Leaders             Spacecraft/Vehicles/Members/Troops/Funds\n\r" );
      ch_printf( ch, "  &O%-20s %-10s %-10s %-10s %-2d %-2d %-3d %-3d %ld\n\r",
                 "Main", clan->leader, clan->number1, clan->number2, clan->spacecraft, clan->vehicles, clan->members,
                 clan->troops, clan->funds );
      if( clan->first_subclan )
      {
         CLAN_DATA *subclan;

         for( subclan = clan->first_subclan; subclan; subclan = subclan->next_subclan )
            ch_printf( ch, "  &O%-20s %-10s %-10s %-10s %-2d %-2d %-3d %-3d %ld\n\r",
                       subclan->name, subclan->leader, subclan->number1, subclan->number2, subclan->spacecraft,
                       subclan->vehicles, subclan->members, subclan->troops, subclan->funds );
      }
      ch_printf( ch, "\n\r" );
      count++;
   }

   ch_printf( ch, "&WAutonomous Groups      Leaders             Spacecraft/Vehicles/Members/Funds\n\r" );
   for( clan = first_clan; clan; clan = clan->next )
   {
      if( clan->clan_type != CLAN_CRIME && clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_CORPORATION
          && clan->clan_type != CLAN_PLAIN )
         continue;
      if( !str_cmp( clan->name, "Neutral" ) )
         continue;

      ch_printf( ch, "&Y%-22s &O%-10s %-10s %-10s %-2d %-2d %-3d %ld\n\r",
                 clan->name, clan->leader, clan->number1, clan->number2, clan->spacecraft, clan->vehicles, clan->members,
                 clan->funds );
      count++;
   }

   if( !count )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char( "There are no organizations currently formed.\n\r", ch );
   }

   set_char_color( AT_WHITE, ch );
   send_to_char( "\n\rSee also: Planets\n\r", ch );

}


void do_orders( CHAR_DATA * ch, char *argument )
{
}

void do_guilds( CHAR_DATA * ch, char *argument )
{
}

void do_shove( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   short dir;
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   ROOM_INDEX_DATA *to_room;
   int schance;
   SHIP_DATA *ship;

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );


   if( arg[0] == '\0' )
   {
      send_to_char( "Shove whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You shove yourself around, to no avail.\n\r", ch );
      return;
   }

   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "You shove at them, only to fall through. What the...\n\r", ch );

      if( number_bits( 1 ) == 1 )
         send_to_char( "&w&RYou skin your knee. Owwie.\n\r", ch );
      else if( number_bits( 1 ) == 0 )
         send_to_char( "&w&ROwwie! You hurt your elbow.\n\r", ch );
      else
         send_to_char( "&w&RYou stubbed your toe. Owwie. :(\n\r", ch );

      ch->hit -= 1;
      return;
   }

   if( ( victim->position ) != POS_STANDING )
   {
      act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only shove player characters.\n\r", ch );
      return;
   }

   if( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
   {
      send_to_char( "That character cannot be shoved right now.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Shove them in which direction?\n\r", ch );
      return;
   }
   else if( ( dir = get_door( arg2 ) ) != -1 )
   {
      if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
      {
         send_to_char( "Shove them into a wall? Bully!\n\r", ch );
         return;
      }

      if( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         send_to_char( "Perhaps you should open the door first.\n\r", ch );
         return;
      }

      to_room = pexit->to_room;

      if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
      {
         send_to_char( "That character cannot enter that area.\n\r", ch );
         victim->position = POS_STANDING;
         return;
      }

      schance = 50;
      schance += ( ( get_curr_str( ch ) - 15 ) * 3 );
      schance += ( ch->top_level - victim->top_level );

      if( schance < number_percent(  ) )
      {
         send_to_char( "You failed.\n\r", ch );
         victim->position = POS_STANDING;
         return;
      }
      act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
      act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
      move_char( victim, get_exit( ch->in_room, dir ), 0 );
      if( !char_died( victim ) )
         victim->position = POS_STANDING;
      WAIT_STATE( ch, 12 );
      /*
       * Remove protection from shove/drag if char shoves -- Blodkai 
       */
      if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && get_timer( ch, TIMER_SHOVEDRAG ) <= 0 )
         add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
   }
   else
   {
      if( !IS_SET( victim->pcdata->act2, ACT_BOUND ) )
      {
         send_to_char( "They must be restrained in order to properly shove them into a ship.\n\r", ch );
         return;
      }

      if( ( ship = ship_in_room( ch->in_room, arg2 ) ) == NULL )
      {
         act( AT_PLAIN, "Thats not a direction or a ship.", ch, NULL, argument, TO_CHAR );
         return;
      }
      else
      {
         if( !ship->hatchopen )
         {
            send_to_char( "The hatch is closed!\n\r", ch );
            return;
         }

         if( ( to_room = get_room_index( ship->entrance ) ) == NULL )
         {
            send_to_char( "That ship has no entrance!\n\r", ch );
            return;
         }

         act( AT_PLAIN, "You are shoved into $T.", victim, NULL, ship->name, TO_CHAR );
         char_from_room( victim );
         char_to_room( victim, to_room );
         sprintf( buf, "%s is shoved into %s.", victim->name, ship->name );
         echo_to_room( AT_PLAIN, ch->in_room, buf );
         act( AT_PLAIN, "$n is shoved into the ship.", victim, NULL, argument, TO_ROOM );
         do_look( victim, "auto" );
      }
   }

   return;

}

void do_drag( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int exit_dir;
   CHAR_DATA *victim;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *to_room;
   bool nogo;
   int schance;
   short temp;

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( arg[0] == '\0' )
   {
      send_to_char( "Drag whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You take yourself by the scruff of your neck, but go nowhere.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only drag player characters.\n\r", ch );
      return;
   }

   if( victim->fighting )
   {
      send_to_char( "You try, but can't get close enough.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Drag them in which direction?\n\r", ch );
      return;
   }

   exit_dir = get_dir( arg2 );

   if( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
   {
      send_to_char( "That character cannot be dragged right now.\n\r", ch );
      return;
   }

   temp = victim->position;

   nogo = FALSE;
   if( ( pexit = get_exit( ch->in_room, exit_dir ) ) == NULL )
      nogo = TRUE;
   else
      if( IS_SET( pexit->exit_info, EX_CLOSED )
          && ( !IS_AFFECTED( victim, AFF_PASS_DOOR ) || IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
      nogo = TRUE;
   if( nogo )
   {
      send_to_char( "There's no exit in that direction.\n\r", ch );
      return;
   }

   to_room = pexit->to_room;

   if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
   {
      send_to_char( "That character cannot enter that area.\n\r", ch );
      victim->position = temp;
      return;
   }

   schance = 50;

   if( schance < number_percent(  ) )
   {
      send_to_char( "You failed.\n\r", ch );
      victim->position = temp;
      return;
   }

   if( victim->position < POS_STANDING )
   {
      victim->position = POS_DRAG;
      act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR );
      act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT );
      move_char( victim, get_exit( ch->in_room, exit_dir ), 0 );
      victim->position = temp;
      move_char( ch, get_exit( ch->in_room, exit_dir ), 0 );
      WAIT_STATE( ch, 12 );
      return;
   }
   send_to_char( "You cannot do that to someone who is standing.\n\r", ch );
   return;
}

void do_enlist( CHAR_DATA * ch, char *argument )
{

   CLAN_DATA *clan;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "You can't do that.\n\r", ch );
      return;
   }

   if( ch->pcdata->clan )
   {
      ch_printf( ch, "You will have to resign from %s before you can join a new organization.\n\r", ch->pcdata->clan->name );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_R_RECRUIT ) )
      clan = get_clan( "The New Republic" );
   else if( IS_SET( ch->in_room->room_flags, ROOM_E_RECRUIT ) )
      clan = get_clan( "The Empire" );
   else
   {
      send_to_char( "You don't seem to be in a recruitment office.\n\r", ch );
      return;
   }

   if( !clan )
   {
      send_to_char( "They don't seem to be recruiting right now.\n\r", ch );
      return;
   }

/*        SET_BIT( ch->speaks, LANG_CLAN );*/
   ++clan->members;

   add_member( ch->name, clan->shortname );
   STRFREE( ch->pcdata->clan_name );
   ch->pcdata->clan_name = QUICKLINK( clan->name );
   ch->pcdata->clan = clan;
   ch_printf( ch, "Welcome to %s.\n\r", clan->name );
   send_to_char( "To have your salary set, speak with the clan leader.\n\r", ch );

   save_clan( clan );
   return;

}

void do_resign( CHAR_DATA * ch, char *argument )
{

   CLAN_DATA *clan;
   DESCRIPTOR_DATA *d;
   long lose_exp;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "You can't do that.\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( clan == NULL )
   {
      send_to_char( "You have to join an organization before you can quit it.\n\r", ch );
      return;
   }

   if( !str_cmp( ch->name, ch->pcdata->clan->leader ) )
   {
      ch_printf( ch, "You can't resign from %s ... you are the leader!\n\r", clan->name );
      return;
   }

/*    if ( ch->speaking & LANG_CLAN )
      ch->speaking = LANG_BASIC;
    REMOVE_BIT( ch->speaks, LANG_CLAN );*/
   --clan->members;
   if( !str_cmp( ch->name, ch->pcdata->clan->number1 ) )
   {
      STRFREE( ch->pcdata->clan->number1 );
      ch->pcdata->clan->number1 = STRALLOC( "" );
   }
   if( !str_cmp( ch->name, ch->pcdata->clan->number2 ) )
   {
      STRFREE( ch->pcdata->clan->number2 );
      ch->pcdata->clan->number2 = STRALLOC( "" );
   }

   act( AT_MAGIC, "You resign your position in $t.", ch, clan->name, NULL, TO_CHAR );

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
      {
         if( IS_NPC( d->character ) )
            continue;
         if( ch == d->character )
            continue;
         if( !d->character->pcdata->clan )
            continue;

         if( !str_cmp( ch->pcdata->clan->name, d->character->pcdata->clan->name ) )
            ch_printf( d->character, "&B%s has resigned from %s.\n\r", ch->name, ch->pcdata->clan->name );
      }
   }

   remove_member( ch->name, ch->pcdata->clan->shortname );
   ch->pcdata->clan = NULL;
   STRFREE( ch->pcdata->clan_name );
   ch->pcdata->clan_name = STRALLOC( "" );

   lose_exp = UMAX( ch->experience[POLITICIAN_ABILITY] - exp_level( ch->skill_level[POLITICIAN_ABILITY] ), 0 );
   ch_printf( ch, "You lose %ld diplomacy experience.\n\r", lose_exp );
   ch->experience[POLITICIAN_ABILITY] -= lose_exp;

   DISPOSE( ch->pcdata->bestowments );
   ch->pcdata->bestowments = str_dup( "" );

   if( ch->pcdata->salary )
      ch->pcdata->salary = 0;

   save_char_obj( ch ); /* clan gets saved when pfile is saved */

   return;

}

void do_clan_withdraw( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   long amount;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "You don't seem to belong to an organization to withdraw funds from...\n\r", ch );
      return;
   }

   if( !ch->in_room || !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
   {
      send_to_char( "You must be in a bank to do that!\n\r", ch );
      return;
   }

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "withdraw", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, ch->pcdata->clan->leader ) )
      ;
   else
   {
      send_to_char( "&RYour organization hasn't seen fit to bestow you with that ability.", ch );
      return;
   }

   clan = ch->pcdata->clan;

   amount = atoi( argument );

   if( !amount )
   {
      send_to_char( "How much would you like to withdraw?\n\r", ch );
      return;
   }

   if( amount > clan->funds )
   {
      ch_printf( ch, "%s doesn't have that much!\n\r", clan->name );
      return;
   }

   if( amount < 0 )
   {
      ch_printf( ch, "Nice try...\n\r" );
      return;
   }

   ch_printf( ch, "You withdraw %ld credits from %s's funds.\n\r", amount, clan->name );

   clan->funds -= amount;
   ch->gold += amount;
   save_clan( clan );

}

void do_clanbuytroops( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   long amount;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "You don't seem to belong to an organization to withdraw funds from...\n\r", ch );
      return;
   }

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "clanbuyship", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, ch->pcdata->clan->leader ) )
      ;
   else
   {
      send_to_char( "&RYou currently cannot buy things with your clans money. You need clanbuyship.\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   amount = atoi( argument );

   if( !amount )
   {
      send_to_char( "How many troops would you like? 1 troop = 25,000 credits.\n\r", ch );
      return;
   }

   if( amount * 25000 > clan->funds )
   {
      ch_printf( ch, "%s doesn't have enough money!\n\r", clan->name );
      return;
   }

   if( amount < 1 )
   {
      ch_printf( ch, "Nice try...\n\r" );
      return;
   }

   ch_printf( ch, "You buy %d troops for your clan, totalling %d credits.\n\r", amount, amount * 25000 );

   clan->funds -= amount * 25000;
   clan->troops += amount;
   save_clan( clan );

}


void do_clan_donate( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   long amount;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "You don't seem to belong to an organization to donate to...\n\r", ch );
      return;
   }

   if( !ch->in_room || !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
   {
      send_to_char( "You must be in a bank to do that!\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   amount = atoi( argument );

   if( !amount )
   {
      send_to_char( "How much would you like to donate?\n\r", ch );
      return;
   }

   if( amount < 0 )
   {
      ch_printf( ch, "Nice try...\n\r" );
      return;
   }

   if( amount > ch->gold )
   {
      send_to_char( "You don't have that much!\n\r", ch );
      return;
   }

   ch_printf( ch, "You donate %ld credits to %s's funds.\n\r", amount, clan->name );

   clan->funds += amount;
   ch->gold -= amount;
   save_clan( clan );

}

void do_newclan( CHAR_DATA * ch, char *argument )
{
   send_to_char( "This command is being recycled to conserve thought.\n\r", ch );
   return;
}

void do_appoint( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg );

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   if( !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   if( str_cmp( ch->name, ch->pcdata->clan->leader ) )
   {
      send_to_char( "Only your leader can do that!\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Useage: appoint <name> < first | second >\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "first" ) )
   {
      if( ch->pcdata->clan->number1 && str_cmp( ch->pcdata->clan->number1, "" ) )
      {
         send_to_char( "You already have someone in that position ... demote them first.\n\r", ch );
         return;
      }

      STRFREE( ch->pcdata->clan->number1 );
      arg[0] = UPPER( arg[0] );
      ch->pcdata->clan->number1 = STRALLOC( arg );
   }
   else if( !str_cmp( argument, "second" ) )
   {
      if( ch->pcdata->clan->number2 && str_cmp( ch->pcdata->clan->number2, "" ) )
      {
         send_to_char( "You already have someone in that position ... demote them first.\n\r", ch );
         return;
      }

      STRFREE( ch->pcdata->clan->number2 );
      arg[0] = UPPER( arg[0] );
      ch->pcdata->clan->number2 = STRALLOC( arg );
   }
   else
      do_appoint( ch, "" );
   save_clan( ch->pcdata->clan );

}

void do_demote( CHAR_DATA * ch, char *argument )
{

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   if( !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   if( str_cmp( ch->name, ch->pcdata->clan->leader ) )
   {
      send_to_char( "Only your leader can do that!\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Demote who?\n\r", ch );
      return;
   }

   if( !str_cmp( argument, ch->pcdata->clan->number1 ) )
   {
      send_to_char( "Player Demoted!", ch );

      STRFREE( ch->pcdata->clan->number1 );
      ch->pcdata->clan->number1 = STRALLOC( "" );
   }
   else if( !str_cmp( argument, ch->pcdata->clan->number2 ) )
   {
      send_to_char( "Player Demoted!", ch );

      STRFREE( ch->pcdata->clan->number2 );
      ch->pcdata->clan->number2 = STRALLOC( "" );
   }
   else
   {
      send_to_char( "They seem to have been demoted already.\n\r", ch );
      return;
   }
   save_clan( ch->pcdata->clan );

}

void do_war( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *wclan;
   CLAN_DATA *clan;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata->bestowments && is_name( "war", ch->pcdata->bestowments ) ) || nifty_is_name( ch->name, clan->leader ) )
      ;
   else
   {
      send_to_char( "You clan hasn't empowered you to declare war!\n\r", ch );
      return;
   }


   if( argument[0] == '\0' )
   {
      send_to_char( "Declare war on who?\n\r", ch );
      return;
   }

   if( ( wclan = get_clan( argument ) ) == NULL )
   {
      send_to_char( "No such clan.\n\r", ch );
      return;
   }

   if( wclan == clan )
   {
      send_to_char( "Declare war on yourself?!\n\r", ch );
      return;
   }

   if( nifty_is_name( wclan->name, clan->atwar ) )
   {
      CLAN_DATA *tclan;
      strcpy( buf, "" );
      for( tclan = first_clan; tclan; tclan = tclan->next )
         if( nifty_is_name( tclan->name, clan->atwar ) && tclan != wclan )
         {
            strcat( buf, "\n\r " );
            strcat( buf, tclan->name );
            strcat( buf, " " );
         }

      STRFREE( clan->atwar );
      clan->atwar = STRALLOC( buf );

      sprintf( buf, "%s has declared a ceasefire with %s!", clan->name, wclan->name );
      echo_to_all( AT_WHITE, buf, ECHOTAR_ALL );

      save_char_obj( ch ); /* clan gets saved when pfile is saved */

      return;
   }

   if( clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_CRIME )
   {
      send_to_char( "&RGuilds can't declare war!\n\r", ch );
      return;
   }

   if( wclan->clan_type == CLAN_GUILD || wclan->clan_type == CLAN_CRIME )
   {
      send_to_char( "&RYou can't declare war on a guild!\n\r", ch );
      return;
   }

   strcpy( buf, clan->atwar );
   strcat( buf, "\n\r " );
   strcat( buf, wclan->name );
   strcat( buf, " " );

   STRFREE( clan->atwar );
   clan->atwar = STRALLOC( buf );

   sprintf( buf, "%s has declared war on %s!", clan->name, wclan->name );
   echo_to_all( AT_RED, buf, ECHOTAR_ALL );

   save_char_obj( ch ); /* clan gets saved when pfile is saved */

}

void do_checkwar( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   CLAN_DATA *clan2;
   int clancnt, i;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH];
   sprintf( buf, "| %-3.3s |", "" );
   sprintf( buf2, "+-----+" );
   clancnt = 0;
   for( clan = first_clan; clan; clan = clan->next )
   {
      if( !str_cmp( clan->name, "Neutral" ) )
         continue;
      sprintf( buf, "%s %-3.3s |", buf, clan->acro ? clan->acro : "NNE" );
      sprintf( buf2, "%s-----+", buf2 );
   }
   send_to_char( buf2, ch );
   send_to_char( "\n\r", ch );
   send_to_char( buf, ch );
   send_to_char( "\n\r", ch );
   send_to_char( buf2, ch );
   send_to_char( "\n\r", ch );
   for( clan = first_clan; clan; clan = clan->next )
   {
      if( !str_cmp( clan->name, "Neutral" ) )
         continue;
      buf[0] = '\0';
      sprintf( buf, "| %-3.3s |", clan->acro ? clan->acro : "NNE" );
      for( clan2 = first_clan; clan2; clan2 = clan2->next )
      {
         if( !str_cmp( clan2->name, "Neutral" ) || !str_cmp( clan->name, "Neutral" ) )
            continue;
         if( nifty_is_name( clan2->name, clan->atwar ) )
            sprintf( buf, "%s &R%-3.3s &w|", buf, "Yes" );
         else
            sprintf( buf, "%s &G%-3.3s &w|", buf, "No" );

      }
      clancnt++;
      send_to_char( buf, ch );
      send_to_char( "\n\r", ch );
      send_to_char( buf2, ch );
      send_to_char( "\n\r", ch );
   }
   buf3[0] = '\0';
   for( i = 0; i <= ( ( clancnt * 6 ) - 24 ); i++ )
      strcat( buf3, " " );

   for( clan = first_clan; clan; clan = clan->next )
   {
      if( !str_cmp( clan->name, "Neutral" ) )
         continue;
      sprintf( buf, "| &G%-3.3s &w| &R%-20.20s%s &w|\n\r", clan->acro ? clan->acro : "NNE", clan->name, buf3 );
      send_to_char( buf, ch );

   }
   send_to_char( buf2, ch );
   send_to_char( "\n\r", ch );
}
void do_empower( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "withdraw", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
      ;
   else
   {
      send_to_char( "You clan hasn't seen fit to bestow that ability to you!\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( arg[0] == '\0' )
   {
      send_to_char( "Empower whom to do what?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Nice try.\n\r", ch );
      return;
   }

   if( victim->pcdata->clan != ch->pcdata->clan )
   {
      send_to_char( "This player does not belong to your clan!\n\r", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = str_dup( "" );

   if( arg2[0] == '\0' || !str_cmp( arg2, "list" ) )
   {
      ch_printf( ch, "Current bestowed commands on %s: %s.\n\r", victim->name, victim->pcdata->bestowments );
      return;
   }

   if( !str_cmp( arg2, "none" ) )
   {
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( "" );
      ch_printf( ch, "Bestowments removed from %s.\n\r", victim->name );
      ch_printf( victim, "%s has removed your bestowed clan abilities.\n\r", ch->name );
      return;
   }
   else if( !str_cmp( arg2, "pilot" ) )
   {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to fly clan ships.\n\r", ch->name );
      send_to_char( "Ok, they now have the ability to fly clan ships.\n\r", ch );
   }
   else if( !str_cmp( arg2, "withdraw" ) )
   {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to withdraw clan funds.\n\r", ch->name );
      send_to_char( "Ok, they now have the ablitity to withdraw clan funds.\n\r", ch );
   }
   else if( !str_cmp( arg2, "clanbuyship" ) )
   {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to buy clan ships.\n\r", ch->name );
      send_to_char( "Ok, they now have the ablitity to use clanbuyship.\n\r", ch );
   }
   else if( !str_cmp( arg2, "induct" ) )
   {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to induct new members.\n\r", ch->name );
      send_to_char( "Ok, they now have the ablitity to induct new members.\n\r", ch );
   }
   else if( !str_cmp( arg2, "outlaw" ) )
   {
      sprintf( buf, "%s outlaw unoutlaw", victim->pcdata->bestowments );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to outlaw and unoutlaw players.\n\r", ch->name );
      send_to_char( "Ok, they now have the ability to outlaw and unoutlaw players.\n\r", ch );
   }
   else
   {
      send_to_char( "Currently you may empower members with only the following:\n\r", ch );
      send_to_char( "\n\rpilot:       ability to fly clan ships\n\r", ch );
      send_to_char( "withdraw:    ability to withdraw clan funds\n\r", ch );
      send_to_char( "clanbuyship: ability to buy clan ships\n\r", ch );
      send_to_char( "induct:      ability to induct new members\n\r", ch );
      send_to_char( "outlaw:      ability to outlaw and unoutlaw players from your clans planets\n\r", ch );
      send_to_char( "none:        removes bestowed abilities\n\r", ch );
   }

   save_char_obj( victim );   /* clan gets saved when pfile is saved */
   return;


}

void save_senate(  )
{
/*
    BOUNTY_DATA *tbounty;
    FILE *fpout;
    char filename[256];
    
    sprintf( filename, "%s%s", SYSTEM_DIR, BOUNTY_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
         bug( "FATAL: cannot open bounty.lst for writing!\n\r", 0 );
         return;
    }
    for ( tbounty = first_bounty; tbounty; tbounty = tbounty->next )
    {
        fprintf( fpout, "%s\n", tbounty->target );
        fprintf( fpout, "%ld\n", tbounty->amount );
    }
    fprintf( fpout, "$\n" );
    fclose( fpout );
*/
}

void load_senate(  )
{
   first_senator = NULL;
   last_senator = NULL;
/*
    FILE *fpList;
    char *target;
    char bountylist[256];
    BOUNTY_DATA *bounty;
    long int  amount;
     
    first_bounty = NULL;
    last_bounty	= NULL;

    first_disintegration = NULL;
    last_disintegration	= NULL;

    log_string( "Loading disintegrations..." );

    sprintf( bountylist, "%s%s", SYSTEM_DIR, disintegration_LIST );
    if ( ( fpList = fopen( bountylist, "r" ) ) == NULL )
    {
	perror( bountylist );
	exit( 1 );
    }

    for ( ; ; )
    {
        target = feof( fpList ) ? "$" : fread_word( fpList );
        if ( target[0] == '$' )
        break;                                  
	CREATE( bounty, BOUNTY_DATA, 1 );
        LINK( bounty, first_disintegration, last_disintegration, next, prev );
	bounty->target = STRALLOC(target);
	amount = fread_number( fpList );
	bounty->amount = amount;
    }
    fclose( fpList );
    log_string(" Done bounties " );

    return;
*/
}

void do_senate( CHAR_DATA * ch, char *argument )
{
/*
    GOV_DATA *gov;
    int count = 0;
    
    set_char_color( AT_WHITE, ch );
    send_to_char( "\n\rGoverning Area                 Controlled By             Value\n\r", ch );
    for ( gov = first_gov; gov; gov = gov->next )
    {
        set_char_color( AT_YELLOW, ch );
        ch_printf( ch, "%-30s %-25s %-15ld\n\r", gov->name, gov->controlled_by , gov->value );
        count++;
    }

    if ( !count )
    {
        set_char_color( AT_GREY, ch );
        send_to_char( "There are no governments to capture at this time.\n\r", ch );
	return;
    }
*/
}

void do_addsenator( CHAR_DATA * ch, char *argument )
{
/*
    GOVE_DATA *gov;
    
    CREATE( gov, GOV_DATA, 1 );
    LINK( gov, first_gov, last_gov, next, prev );

    gov->name		= STRALLOC( argument );
    gov->value          = atoi( arg2 );
    gov->vnum           = object;
    gov->controlled_by  = STRALLOC( "" );
        
    ch_printf( ch, "OK, making %s.\n\r", argument );
    save_govs();
*/
}

void do_remsenator( CHAR_DATA * ch, char *argument )
{
/*
	UNLINK( bounty, first_bounty, last_bounty, next, prev );
	STRFREE( bounty->target );
	DISPOSE( bounty );
	
	save_bounties();
*/
}


/*
    (link)->prev		= (insert)->prev;		
    if ( !(insert)->prev )					
      (first)			= (link);			
    else							
      (insert)->prev->next	= (link);			
    (insert)->prev		= (link);			
    (link)->next		= (insert);			
*/

/* Outlaw System by Tawnos */
void do_outlaw( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int value;
   PLANET_DATA *planet;
   CHAR_DATA *victim;
   CLAN_DATA *clan;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "outlaw", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) );
   else
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Outlaw whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   for( planet = first_planet; planet; planet = planet->next )
   {
      if( planet->governed_by == clan )
         value = get_wanted_flag( planet->name );
      if( value < 0 || value > 31 )
         ch_printf( ch, "Error. Report to Administration.\n\r" );
      SET_BIT( victim->pcdata->wanted_flags, 1 << value );
   }

   send_to_char( "They have been outlawed from all of your clans planets.\n\r", ch );
   ch_printf( victim,
              "&W&RA slight buzz comes over your comlink and you hear, 'Attention all citizens, %s has been banned from %s's planets until further notice. Guards will attack on sight. Thank you.'\n\r",
              victim->name, clan->name );

   return;
}

void do_unoutlaw( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int value;
   PLANET_DATA *planet;
   CHAR_DATA *victim;
   CLAN_DATA *clan;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "unoutlaw", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) );
   else
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Unoutlaw whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   for( planet = first_planet; planet; planet = planet->next )
   {
      if( planet->governed_by == clan )
      {
         value = get_wanted_flag( planet->name );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Error. Report to Administration.\n\r" );
         if( IS_SET( victim->pcdata->wanted_flags, 1 << value ) )
            REMOVE_BIT( victim->pcdata->wanted_flags, 1 << value );
      }
   }

   send_to_char( "They are now allowed to roam your clans planets freely.\n\r", ch );
   ch_printf( victim,
              "&W&GA slight buzz comes over your comlink and you hear, 'Attention all citizens, %s has been absolved of any crimes created on %s's planets. They are no longer wanted. Thank you.'\n\r",
              victim->name, clan->name );

   return;
}

/* Clanstat by Tawnos. More RP */
void do_clanstat( CHAR_DATA * ch, char *argument )
{
   PLANET_DATA *planet;
   CLAN_DATA *clan;
   long revenue = 0;
   int pCount = 0, support = 0;

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's need no information about clans!\n\r", ch );
      return;
   }

   if( !ch->pcdata->clan )
   {
      send_to_char( "You're not even IN a clan!\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   for( planet = first_planet; planet; planet = planet->next )
      if( clan == planet->governed_by )
      {
         support += planet->pop_support;
         pCount++;
         revenue += get_taxes( planet );
      }

   if( pCount > 1 )
      support /= pCount;

   ch_printf( ch, "\n\r&R&W+---------------------------------------------------------+\n\r" );
   ch_printf( ch, "&O&W|   &BClan Name&W: %-20.20s      &BPlanets&W: %-2.2d      |\n\r", clan->name, pCount );
   if( support >= 0 )
      ch_printf( ch, "&O&W| &BPop Support&W: %-3.3d                       &BRevenue&W: %-6.6d  |\n\r", support, revenue );
   else
      ch_printf( ch, "&O&W| &BPop Support&W: %-3.3d                      &BRevenue&W: %-9.9d |\n\r", support, revenue );
   ch_printf( ch, "&R&W+---------------------------------------------------------+\n\r" );
   ch_printf( ch, "&O&W|         &BMain&W:                         &YFunds&W: %-9.9d  |\n\r", clan->funds );
   if( clan->leader[0] != 0 )
      ch_printf( ch, "&O&W|       &BLeader&W: %-12.12s           &BTroops&W: %-4.4d       |\n\r", clan->leader,
                 clan->troops );
   else
      ch_printf( ch, "&O&W|       &BLeader&W: None.              &BTroops&W: %-4.4d           |\n\r", clan->troops );
   ch_printf( ch, "&O&W|        &BFirst&W: %-12.12s       &BSpacecraft&W: %-2.2d         |\n\r", clan->number1,
              clan->spacecraft );
   ch_printf( ch, "&O&W|       &BSecond&W: %-12.12s          &BMembers&W: %-2.2d         |\n\r", clan->number2,
              clan->members );

   if( clan->first_subclan )
   {
      CLAN_DATA *subclan;

      for( subclan = clan->first_subclan; subclan; subclan = subclan->next_subclan )
      {
         ch_printf( ch, "&R&W+---------------------------------------------------------+\n\r" );
         ch_printf( ch, "&O&W| &B%-12.12s&W:                         &YFunds&W: %-9.9d  |\n\r", subclan->name,
                    subclan->funds );
         if( clan->leader[0] != 0 )
            ch_printf( ch, "&O&W|       &BLeader&W: %-12.12s       &BSpacecraft&W: %-2.2s         |\n\r", subclan->leader,
                       subclan->spacecraft );
         else
            ch_printf( ch, "&O&W|       &BLeader&W: None.              &BTroops&W: %-2.2s             |\n\r",
                       subclan->leader, subclan->troops );
         ch_printf( ch, "&O&W|        &BFirst&W: %-12.12s         &BSpacecraft&W: %-2.2s       |\n\r", subclan->number1,
                    subclan->spacecraft );
         ch_printf( ch, "&O&W|       &BSecond&W: %-12.12s          &BMembers&W: %-2.2s         |\n\r", subclan->number2,
                    subclan->members );
      }
   }

   ch_printf( ch, "&R&W+---------------------------------------------------------+\n\r" );

   return;
}


void do_setwage( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;
   int amount;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "You are not a clan leader!\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   amount = atoi( arg2 );

   clan = ch->pcdata->clan;

   if( str_cmp( ch->name, clan->leader ) && str_cmp( ch->name, clan->number1 ) && str_cmp( ch->name, clan->number2 ) )
   {
      send_to_char( "You are not a prominent member in your clan.\n\r", ch );
      return;
   }

   if( arg[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Set who's clan wages, and how much?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      if( str_cmp( ch->name, clan->leader ) )
      {
         send_to_char( "Just withdraw your wages!\n\r", ch );
         return;
      }
      send_to_char( "You cannot set your own wages.\n\r", ch );
      return;
   }

   if( !str_cmp( victim->name, clan->leader ) )
   {
      send_to_char( "The leader can handle himself.\n\r", ch );
      return;
   }

   if( clan->number2 == ch->name && clan->number1 == victim->name )
   {
      send_to_char( "They are more powerful than you.\n\r", ch );
      return;
   }

   if( victim->pcdata->clan != ch->pcdata->clan )
   {
      send_to_char( "This player does not belong to your clan!\n\r", ch );
      return;
   }

   if( amount < 0 )
   {
      send_to_char( "Maybe try an amount GREATER than 0.\n\r", ch );
      return;
   }

   if( amount > clan->funds )
   {
      send_to_char( "Come now, your clan doesn't even HAVE that much money.\n\r", ch );
      return;
   }

   if( amount == 0 )
   {
      victim->pcdata->salary = 0;
      send_to_char( "&w&GTheir salary has been removed.\n\r", ch );
      send_to_char( "&w&RYour salary has been removed.\n\r", victim );
      return;
   }

   victim->pcdata->salary = amount;
   ch_printf( ch, "&w&G%s's salary is now %d credits per day.\n\r", victim->name, amount );
   ch_printf( victim, "&w&GYour salary has been set to %d credits per day.\n\r", amount );

   save_char_obj( victim );   /* clan gets saved when pfile is saved */
   return;
}

void do_clans( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA *clan;
   PLANET_DATA *planet;
   int count = 0;
   int pCount = 0;
   int support;
   long revenue;
   ch_printf( ch, "\n\r&z+---------------------------------------------------------------+\n\r" );
   for( clan = first_clan; clan; clan = clan->next )
   {
      if( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN
          || clan->clan_type == CLAN_CORPORATION || clan->clan_type == CLAN_PLAIN )
         continue;
      if( !str_cmp( clan->name, "Neutral" ) )
         continue;
      pCount = 0;
      support = 0;
      revenue = 0;

      for( planet = first_planet; planet; planet = planet->next )
         if( clan == planet->governed_by )
         {
            support += planet->pop_support;
            pCount++;
            revenue += get_taxes( planet );
         }

      if( pCount > 1 )
         support /= pCount;

      ch_printf( ch, "&z| &WOrganization: &w%-25.25s &WType: &w%-15.15s &z|\n\r",
                 clan->name, clan->clan_type == 0 ? "Government" :
                 clan->clan_type == 1 ? "Crime Syndicate" :
                 clan->clan_type == 2 ? "Guild" : clan->clan_type == 4 ? "Corporation" : "Clan" );
      ch_printf( ch, "&z| &WControlled planets: &w%-2d   &WSupport: &w%3d%%                        &z|\n\r",
                 pCount, support );
      /*
       * ch_printf( ch, "  &WDivision             Leaders             Spacecraft/Vehicles/Members/Troops/Funds\n\r");
       * ch_printf( ch, "  &O%-20s %-10s %-10s %-10s %-2d %-2d %-3d %-3d %ld\n\r",
       * "Main", clan->leader, clan->number1, clan->number2, clan->spacecraft, clan->vehicles, clan->members, clan->troops, clan->funds );
       * if ( clan->first_subclan )
       * {
       * CLAN_DATA *subclan;
       * 
       * for ( subclan = clan->first_subclan ; subclan ; subclan = subclan->next_subclan ) 
       * ch_printf( ch, "  &O%-20s %-10s %-10s %-10s %-2d %-2d %-3d %-3d %ld\n\r",
       * subclan->name, subclan->leader, subclan->number1, subclan->number2, subclan->spacecraft, subclan->vehicles, subclan->members, subclan->troops, subclan->funds );
       * }
       * ch_printf( ch , "\n\r" ); 
       */
      ch_printf( ch, "&z+---------------------------------------------------------------+\n\r" );
      count++;
   }

   for( clan = first_clan; clan; clan = clan->next )
   {
      if( clan->clan_type != CLAN_CRIME && clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_CORPORATION
          && clan->clan_type != CLAN_PLAIN )
         continue;
      if( !str_cmp( clan->name, "Neutral" ) )
         continue;
      ch_printf( ch, "&z| &WOrganization: &w%-25.25s &WType: &w%-15.15s &z|\n\r",
                 clan->name, clan->clan_type == 0 ? "Government" :
                 clan->clan_type == 1 ? "Crime Syndicate" :
                 clan->clan_type == 2 ? "Guild" : clan->clan_type == 4 ? "Corporation" : "Clan" );
      if( clan->clan_type != 1 )
         ch_printf( ch, "&z|       &W%6.6s: &w%-15.15s                                 &z|\n\r",
                    clan->clan_type == 1 ? "Leader" :
                    clan->clan_type == 2 ? "Leader" : clan->clan_type == 4 ? "CEO" : "Leader", clan->leader );
      ch_printf( ch, "&z+---------------------------------------------------------------+\n\r" );

      count++;
   }

   if( !count )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char( "There are no organizations currently formed.\n\r", ch );
   }

}

void do_members( CHAR_DATA * ch, char *argument )
{
   FILE *fpList;
   char *buf;
   char thebuf[MAX_STRING_LENGTH];
   char list[MAX_STRING_LENGTH];
   char color[MAX_STRING_LENGTH];
   char display[MAX_STRING_LENGTH];
   char prefix[MAX_STRING_LENGTH];
   int i = 0;
   CLAN_DATA *clan;

   if( IS_NPC( ch ) )
      return;

   if( !ch->pcdata->clan )
   {
      send_to_char( "&RYou must first belong to a clan to see its members.\n\r", ch );
      return;
   }

   if( !ch->pcdata->clan->shortname )
   {
      send_to_char( "&RYour clan doesn't have a shortname. tell an imm this.\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   sprintf( list, "%s%s.list", CLAN_DIR, clan->shortname );

   if( ( fpList = fopen( list, "r" ) ) == NULL )
   {
      send_to_char( "Something wen't wrong. The imms have been notified.\n\r", ch );
      bug( "Do_members: Unable to open member list" );
      return;
   }

   sprintf( color, "%s",
            !str_cmp( clan->name, "The Empire" ) ? "&R" : !str_cmp( clan->name, "The New Republic" ) ? "&B" : "&G" );


   sprintf( thebuf, "%s---------------============<&W%s%s>============---------------\n\r", color,
            centertext( clan->name, 24 ), color );
   send_to_char( thebuf, ch );

   for( ;; )
   {
      if( feof( fpList ) )
         break;
      buf = feof( fpList ) ? "End" : fread_string( fpList );
      if( !str_cmp( buf, "End" ) || buf[0] == '\0' )
         break;
      if( strlen( buf ) < 3 )
         break;

      if( i % 3 == 0 )
         send_to_char( "&W          ", ch );

      if( !str_cmp( buf, clan->leader ) )
         sprintf( prefix, "%s[&W1%s]&W", color, color );
      else if( !str_cmp( buf, clan->number1 ) )
         sprintf( prefix, "%s[&W2%s]&W", color, color );
      else if( !str_cmp( buf, clan->number2 ) )
         sprintf( prefix, "%s[&W3%s]&W", color, color );
      else
         sprintf( prefix, "   &W" );

      sprintf( display, "%s%-20s", prefix, buf );
      send_to_char( display, ch );
      ++i;

      if( i % 3 == 0 )
         send_to_char( "\n\r", ch );

   }
   send_to_char( "\n\r", ch );
   fclose( fpList );
}

void add_member( char *name, char *shortname )
{
   char buf[MAX_STRING_LENGTH];
   char fbuf[MAX_STRING_LENGTH];

   if( name[0] == '\0' || !name )
   {
      bug( "add_member: No name!\n\r" );
      return;
   }

   if( shortname[0] == '\0' || !shortname )
   {
      bug( "add_member: No shortname!\n\r" );
      return;
   }

   sprintf( fbuf, "%s%s.list", CLAN_DIR, shortname );
   sprintf( buf, "%s~", name );
   append_to_file( fbuf, buf );

}

void remove_member( char *name, char *shortname )
{
   FILE *fpList;
   FILE *fpNew;
   char *buf;
   char list[MAX_STRING_LENGTH];
   char temp[MAX_STRING_LENGTH];

   if( name[0] == '\0' )
   {
      bug( "remove_member: No name!\n\r" );
      return;
   }

   if( shortname[0] == '\0' || !shortname )
   {
      bug( "remove_member: No shortname!\n\r" );
      return;
   }

   sprintf( list, "%s%s.list", CLAN_DIR, shortname );
   sprintf( temp, "%s.temp", list );

   if( ( fpList = fopen( list, "r" ) ) == NULL )
   {
      bug( "Unable to open member list" );
      return;
   }

   if( ( fpNew = fopen( temp, "w" ) ) == NULL )
   {
      bug( "remove_member: Unable to write temp list" );
      return;
   }

   for( ;; )
   {
      if( feof( fpList ) )
         break;
      buf = feof( fpList ) ? "End" : fread_string( fpList );
      if( !str_cmp( buf, "End" ) || buf[0] == '\0' )
         break;
      if( str_cmp( name, buf ) && strlen( buf ) > 2 )
         fprintf( fpNew, "%s~\n", buf );
   }
   fclose( fpNew );
   fclose( fpList );
   rename( temp, list );
}
