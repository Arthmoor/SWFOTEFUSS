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
#include <time.h>
#include "mud.h"

extern int top_area;
extern int top_r_vnum;
void write_area_list( void );
void write_starsystem_list( void );

PLANET_DATA *first_planet;
PLANET_DATA *last_planet;

extern GUARD_DATA *first_guard;
extern GUARD_DATA *last_guard;

/* local routines */
void fread_planet( PLANET_DATA * planet, FILE * fp );
bool load_planet_file( const char *planetfile );
void write_planet_list( void );

PLANET_DATA *get_planet( const char *name )
{
   PLANET_DATA *planet;

   if( name[0] == '\0' )
      return NULL;

   for( planet = first_planet; planet; planet = planet->next )
      if( !str_cmp( name, planet->name ) )
         return planet;

   for( planet = first_planet; planet; planet = planet->next )
      if( nifty_is_name( name, planet->name ) )
         return planet;

   for( planet = first_planet; planet; planet = planet->next )
      if( !str_prefix( name, planet->name ) )
         return planet;

   for( planet = first_planet; planet; planet = planet->next )
      if( nifty_is_name_prefix( name, planet->name ) )
         return planet;

   return NULL;
}

void write_planet_list(  )
{
   PLANET_DATA *tplanet;
   FILE *fpout;
   char filename[256];

   snprintf( filename, 256, "%s%s", PLANET_DIR, PLANET_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: %s: cannot open planet.lst for writing!\r\n", __func__ );
      return;
   }
   for( tplanet = first_planet; tplanet; tplanet = tplanet->next )
      fprintf( fpout, "%s\n", tplanet->filename );
   fprintf( fpout, "$\n" );
   FCLOSE( fpout );
}

void save_planet( PLANET_DATA * planet )
{
   FILE *fp;
   char filename[256];

   if( !planet )
   {
      bug( "%s: null planet pointer!", __func__ );
      return;
   }

   if( !planet->filename || planet->filename[0] == '\0' )
   {
      bug( "%s: %s has no filename", __func__, planet->name );
      return;
   }

   snprintf( filename, 256, "%s%s", PLANET_DIR, planet->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: fopen", __func__ );
      perror( filename );
   }
   else
   {
      AREA_DATA *pArea;

      fprintf( fp, "#PLANET\n" );
      fprintf( fp, "Name         %s~\n", planet->name );
      fprintf( fp, "Filename     %s~\n", planet->filename );
      fprintf( fp, "X            %d\n", planet->x );
      fprintf( fp, "Y            %d\n", planet->y );
      fprintf( fp, "Z            %d\n", planet->z );
      fprintf( fp, "Sector       %d\n", planet->sector );
      fprintf( fp, "Type    	   %d\n", planet->controls );
      fprintf( fp, "BaseValue    %d\n",       planet->base_value );
      fprintf( fp, "PopSupport   %d\n", ( int )( planet->pop_support ) );
      if( planet->starsystem && planet->starsystem->name )
         fprintf( fp, "Starsystem   %s~\n", planet->starsystem->name );
      if( planet->governed_by && planet->governed_by->name )
         fprintf( fp, "GovernedBy   %s~\n", planet->governed_by->name );
      for( pArea = planet->first_area; pArea; pArea = pArea->next_on_planet )
         fprintf( fp, "Area         %s~\n", pArea->filename );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      FCLOSE( fp );
   }
   return;
}

void fread_planet( PLANET_DATA * planet, FILE * fp )
{
   const char *word;
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
            if( !str_cmp( word, "area" ) )
            {
               planet->area = get_area( fread_string( fp ) );
               if( planet->area )
               {
                  AREA_DATA *area = planet->area;
                  area->planet = planet;
                  LINK( area, planet->first_area, planet->last_area, next_on_planet, prev_on_planet );
               }
               fMatch = TRUE;
            }
            break;

           case 'B':
               KEY( "BaseValue", planet->base_value, fread_number( fp ) );
               break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !planet->name )
                  planet->name = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Filename", planet->filename, fread_string_nohash( fp ) );
            break;

         case 'G':
            if( !str_cmp( word, "GovernedBy" ) )
            {
               const char *clan_name = fread_string( fp );
               planet->governed_by = get_clan( clan_name );
               fMatch = TRUE;
               STRFREE( clan_name );
            }
            break;

         case 'N':
            KEY( "Name", planet->name, fread_string( fp ) );
            break;

         case 'P':
            KEY( "PopSupport", planet->pop_support, fread_float( fp ) );
            break;

         case 'S':
            KEY( "Sector", planet->sector, fread_number( fp ) );
            if( !str_cmp( word, "Starsystem" ) )
            {
               const char *starsystem_name = fread_string( fp );
               planet->starsystem = starsystem_from_name( starsystem_name );
               if( planet->starsystem )
               {
                  SPACE_DATA *starsystem = planet->starsystem;

                  LINK( planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
               }
               fMatch = TRUE;
               STRFREE( starsystem_name );
            }
            break;
         case 'T':
            KEY( "Type", planet->controls, fread_number( fp ) );
            break;
         case 'X':
            KEY( "X", planet->x, fread_number( fp ) );
            break;

         case 'Y':
            KEY( "Y", planet->y, fread_number( fp ) );
            break;

         case 'Z':
            KEY( "Z", planet->z, fread_number( fp ) );
            break;

      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }

   }
}

bool load_planet_file( const char *planetfile )
{
   char filename[256];
   PLANET_DATA *planet;
   FILE *fp;
   bool found;

   CREATE( planet, PLANET_DATA, 1 );

   planet->governed_by = NULL;
   planet->next_in_system = NULL;
   planet->prev_in_system = NULL;
   planet->starsystem = NULL;
   planet->area = NULL;
   planet->first_guard = NULL;
   planet->last_guard = NULL;

   found = FALSE;
   snprintf( filename, 256, "%s%s", PLANET_DIR, planetfile );

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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "PLANET" ) )
         {
            fread_planet( planet, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section: %s.", __func__, word );
            break;
         }
      }
      FCLOSE( fp );
   }

   if( !found )
      DISPOSE( planet );
   else
      LINK( planet, first_planet, last_planet, next, prev );

   return found;
}

void load_planets(  )
{
   FILE *fpList;
   const char *filename;
   char planetlist[256];

   first_planet = NULL;
   last_planet = NULL;

   log_string( "Loading planets..." );

   snprintf( planetlist, 256, "%s%s", PLANET_DIR, PLANET_LIST );
   if( ( fpList = fopen( planetlist, "r" ) ) == NULL )
   {
      perror( planetlist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( filename );
      if( filename[0] == '$' )
         break;

      if( !load_planet_file( filename ) )
      {
         bug( "%s: Cannot load planet file: %s", __func__, filename );
      }
   }
   FCLOSE( fpList );
   log_string( " Done planets " );
   return;
}

AREA_DATA *get_area( const char *argument )
{
   AREA_DATA *pArea;
   for( pArea = first_area; pArea; pArea = pArea->next )
      if( pArea->filename && !str_cmp( pArea->filename, argument ) )
         return pArea;

   return NULL;
}

void do_setplanet( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   PLANET_DATA *planet;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: setplanet <planet> <field> [value]\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( " name filename area base_value starsystem governed_by x y z\r\n", ch );
      return;
   }

   if( !( planet = get_planet( arg1 ) ) )
   {
      send_to_char( "No such planet.\r\n", ch );
      return;
   }

   if( !strcmp( arg2, "name" ) )
   {
      PLANET_DATA *tplanet;
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "You must choose a name.\r\n", ch );
         return;
      }
      if( ( tplanet = get_planet( argument ) ) != NULL )
      {
         send_to_char( "A planet with that name already Exists!\r\n", ch );
         return;
      }

      STRFREE( planet->name );
      planet->name = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      return;
   }

    if( !strcmp( arg2, "base_value" ) )
    {
        int bval;

        bval = atoi( argument );

        if( (bval < 1) || (bval > MAX_PLANET_BASE_VALUE) )
        {
           ch_printf( ch, "&YPlanet base values range from &W%d &Yto &W%d&Y!\n\r", 1, MAX_PLANET_BASE_VALUE );
           return;
        }
        planet->base_value = bval;
        send_to_char( "Done.\n\r", ch );
        save_planet( planet );
        write_planet_list(  );
        return;
    }

   if( !strcmp( arg2, "type" ) )
   {
      if( !argument )
         planet->controls = 0;
      else
         planet->controls = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      return;
   }

   if( !strcmp( arg2, "sector" ) )
   {
      planet->sector = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      return;
   }

   if( !strcmp( arg2, "area" ) )
   {
      planet->area = get_area( argument );
      if( planet->area )
      {
         AREA_DATA *area = planet->area;
         LINK( area, planet->first_area, planet->last_area, next_on_planet, prev_on_planet );
      }
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      return;
   }

   if( !strcmp( arg2, "governed_by" ) )
   {
      CLAN_DATA *clan;
      clan = get_clan( argument );
      if( clan )
      {
         planet->governed_by = clan;
         send_to_char( "Done.\r\n", ch );
         save_planet( planet );
      }
      else
         send_to_char( "No such clan.\r\n", ch );
      return;
   }

   if( !strcmp( arg2, "starsystem" ) )
   {
      SPACE_DATA *starsystem;

      if( ( starsystem = planet->starsystem ) != NULL )
         UNLINK( planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
      if( ( planet->starsystem = starsystem_from_name( argument ) ) )
      {
         starsystem = planet->starsystem;
         LINK( planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
         send_to_char( "Done.\r\n", ch );
      }
      else
         send_to_char( "No such starsystem.\r\n", ch );
      save_planet( planet );
      return;
   }

   if( !strcmp( arg2, "x" ) )
   {
      planet->x = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      write_planet_list(  );
      return;
   }

   if( !strcmp( arg2, "y" ) )
   {
      planet->y = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      write_planet_list(  );
      return;
   }

   if( !strcmp( arg2, "z" ) )
   {
      planet->z = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      write_planet_list(  );
      return;
   }

   if( !strcmp( arg2, "filename" ) )
   {
      PLANET_DATA *tplanet;

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "You must choose a file name.\r\n", ch );
         return;
      }
      for( tplanet = first_planet; tplanet; tplanet = tplanet->next )
      {
          if( !str_cmp( tplanet->filename, argument ) )
          {
              send_to_char( "A planet with that filename already exists!\r\n", ch );
              return;
          }
      }

      DISPOSE( planet->filename );
      planet->filename = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      save_planet( planet );
      write_planet_list(  );
      return;
   }

   do_setplanet( ch, "" );
   return;
}

void do_showplanet( CHAR_DATA * ch, const char *argument )
{
   GUARD_DATA *guard;
   PLANET_DATA *planet;
   AREA_DATA *pArea;
   char area[MAX_STRING_LENGTH];
   int num_guards = 0;
   int pf = 0;
   int pc = 0;
   int pw = 0;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showplanet <planet>\r\n", ch );
      return;
   }

   planet = get_planet( argument );
   if( !planet )
   {
      send_to_char( "No such planet.\r\n", ch );
      return;
   }

   for( guard = planet->first_guard; guard; guard = guard->next_on_planet )
      num_guards++;

   if( planet->size > 0 )
   {
      float tempf;

      tempf = planet->citysize;
      pc = ( int )( tempf / planet->size * 100 );

      tempf = planet->wilderness;
      pw = ( int )( tempf / planet->size * 100 );

      tempf = planet->farmland;
      pf = ( int )( tempf / planet->size * 100 );
   }

   ch_printf( ch, "&W%s\r\n", planet->name );
   if( IS_IMMORTAL( ch ) )
      ch_printf( ch, "&WFilename: &G%s\r\n", planet->filename );

   ch_printf( ch, "&WTerrain: &G%s\r\n", sector_name[planet->sector] );
   ch_printf( ch, "&WGoverned by: &G%s\r\n", planet->governed_by ? planet->governed_by->name : "" );
   ch_printf( ch, "&WPlanet Size: &G%d\r\n", planet->size );
   ch_printf( ch, "&WPercent Civilized: &G%d\r\n", pc );
   ch_printf( ch, "&WPercent Wilderness: &G%d\r\n", pw );
   ch_printf( ch, "&WPercent Farmland: &G%d\r\n", pf );
   ch_printf( ch, "&WBarracks: &G%d\r\n", planet->barracks );
   ch_printf( ch, "&WPatrols: &G%d&W/%d\r\n", num_guards, planet->barracks * 5 );
   ch_printf( ch, "&WPopulation: &G%d&W\r\n", planet->population );
   ch_printf( ch, "&WPopular Support: &G%.2f\r\n", planet->pop_support );
   ch_printf( ch, "&WBase Value: &G%d\n\r", planet->base_value );

   ch_printf( ch, "&WCurrent Monthly Revenue: &G%ld\r\n", get_taxes( planet ) );
   area[0] = '\0';
   for( pArea = planet->first_area; pArea; pArea = pArea->next_on_planet )
   {
      mudstrlcat( area, pArea->filename, MAX_STRING_LENGTH );
      mudstrlcat( area, ", ", MAX_STRING_LENGTH );
   }
   ch_printf( ch, "&WAreas: &G%s\r\n", area );
   if( IS_IMMORTAL( ch ) && !planet->area )
   {
      ch_printf( ch, "&RWarning - this planet is not attached to an area!&G" );
      ch_printf( ch, "\r\n" );
   }

   return;
}

void do_makeplanet( CHAR_DATA * ch, const char *argument )
{
   char filename[256];
   PLANET_DATA *planet;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makeplanet <planet name>\r\n", ch );
      return;
   }

   snprintf( filename, 256, "%s%s", PLANET_DIR, strlower( argument ) );

   CREATE( planet, PLANET_DATA, 1 );
   LINK( planet, first_planet, last_planet, next, prev );
   planet->governed_by = NULL;
   planet->next_in_system = NULL;
   planet->prev_in_system = NULL;
   planet->starsystem = NULL;
   planet->first_area = NULL;
   planet->last_area = NULL;
   planet->first_guard = NULL;
   planet->last_guard = NULL;
   planet->name = STRALLOC( argument );
   planet->flags = 0;
}

void do_planets( CHAR_DATA * ch, const char *argument )
{
   PLANET_DATA *planet;
   int count = 0;
   SPACE_DATA *starsystem;

   set_char_color( AT_WHITE, ch );
   send_to_char( "Planet             Starsystem    Governed By                  Popular Support\r\n", ch );

   for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
      for( planet = starsystem->first_planet; planet; planet = planet->next_in_system )
      {
         if( planet->controls != 0 )
            continue;
         ch_printf( ch, "&G%-18s %-12s  %-25s    ",
                    planet->name, starsystem->name, planet->governed_by ? planet->governed_by->name : "" );
         ch_printf( ch, "%.1f\r\n", planet->pop_support );
         if( IS_IMMORTAL( ch ) && !planet->area )
         {
            ch_printf( ch, "&RWarning - this planet is not attached to an area!&G" );
            ch_printf( ch, "\r\n" );
         }

         count++;
      }

   for( planet = first_planet; planet; planet = planet->next )
   {
      if( planet->starsystem )
         continue;

      ch_printf( ch, "&G%-15s %-12s  %-25s    ", planet->name, "", planet->governed_by ? planet->governed_by->name : "" );
      ch_printf( ch, "%.1f\r\n", !str_cmp( planet->governed_by->name, "Neutral" ) ? 100.0 : planet->pop_support );
      if( IS_IMMORTAL( ch ) && !planet->area )
      {
         ch_printf( ch, "&RWarning - this planet is not attached to an area!&G" );
         ch_printf( ch, "\r\n" );
      }

      count++;
   }

   if( !count )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char( "There are no planets currently formed.\r\n", ch );
   }
   send_to_char( "&WUse SHOWPLANET for more information.\r\n", ch );

}

void do_capture( CHAR_DATA * ch, const char *argument )
{
   CLAN_DATA *clan;
   PLANET_DATA *planet;
   PLANET_DATA *cPlanet;
   float support = 0.0;
   int pCount = 0;
   char buf[MAX_STRING_LENGTH];

   if( !ch->in_room || !ch->in_room->area )
      return;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "huh?\r\n", ch );
      return;
   }

   if( !ch->pcdata->clan )
   {
      send_to_char( "You need to be a member of an organization to do that!\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( planet = ch->in_room->area->planet ) == NULL )
   {
      send_to_char( "You must be on a planet to capture it.\r\n", ch );
      return;
   }

   if( clan == planet->governed_by )
   {
      send_to_char( "Your organization already controls this planet.\r\n", ch );
      return;
   }

   if( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD )
   {
      send_to_char( "Your clan can't capture planets!\r\n", ch );
      return;
   }

   if( planet->starsystem )
   {
      SHIP_DATA *ship;
      CLAN_DATA *sClan;

      for( ship = planet->starsystem->first_ship; ship; ship = ship->next_in_starsystem )
      {
         sClan = get_clan( ship->owner );
         if( !sClan )
            continue;
         if( sClan == planet->governed_by )
         {
            send_to_char( "A planet cannot be captured while protected by orbiting spacecraft.\r\n", ch );
            return;
         }
      }
   }

   if( planet->first_guard )
   {
      send_to_char( "This planet is protected by soldiers.\r\n", ch );
      send_to_char( "You will have to eliminate all enemy forces before you can capture it.\r\n", ch );
      return;
   }
   if( planet->governed_by->name && planet->governed_by->name != NULL )
   {
      if( !str_cmp( planet->governed_by->name, "Neutral" ) )
      {
         send_to_char( "This planet cannot be captured.\r\n", ch );
         return;
      }
   }
   if( planet->pop_support > 0 )
   {
      send_to_char( "The population is not in favour of changing leaders right now.\r\n", ch );
      return;
   }

   for( cPlanet = first_planet; cPlanet; cPlanet = cPlanet->next )
      if( clan == cPlanet->governed_by )
      {
         pCount++;
         support += cPlanet->pop_support;
      }

   if( support < 0 )
   {
      send_to_char
         ( "There is not enough popular support for your organization!\r\nTry improving loyalty on the planets that you already control.\r\n",
           ch );
      return;
   }

   planet->governed_by = clan;
   planet->pop_support = 50;

   snprintf( buf, MAX_STRING_LENGTH, "%s has been captured by %s!", planet->name, clan->name );
   echo_to_all( AT_RED, buf, 0 );

   save_planet( planet );

   return;
}

long get_taxes( PLANET_DATA * planet )
{
   long gain;

   gain = planet->base_value;
   gain += ( long )( planet->base_value * ( planet->pop_support / 100 ) );
   gain += ( planet->population * 150 );
   gain += UMAX( 0, ( int )( planet->pop_support / 10 * ( planet->population * 20 ) ) );

   return gain;
}
