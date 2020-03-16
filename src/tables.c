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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "mud.h"

/* global variables */
int top_sn;
int top_herb;

SKILLTYPE *skill_table[MAX_SKILL];
SKILLTYPE *herb_table[MAX_HERB];

const char *const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb" };

SPELL_FUN *spell_function( const char *name )
{
   void *funHandle;
   const char *error;

   funHandle = dlsym( sysdata.dlHandle, name );
   if( ( error = dlerror(  ) ) != NULL )
   {
      bug( "%s: Error locating %s in symbol table. %s", __func__, name, error );
      return spell_notfound;
   }
   return ( SPELL_FUN * ) funHandle;
}

DO_FUN *skill_function( const char *name )
{
   void *funHandle;
   const char *error;

   funHandle = dlsym( sysdata.dlHandle, name );
   if( ( error = dlerror(  ) ) != NULL )
   {
      bug( "%s: Error locating %s in symbol table. %s", __func__, name, error );
      return skill_notfound;
   }
   return ( DO_FUN * ) funHandle;
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE ** sk1, SKILLTYPE ** sk2 )
{
   SKILLTYPE *skill1 = ( *sk1 );
   SKILLTYPE *skill2 = ( *sk2 );

   if( !skill1 && skill2 )
      return 1;
   if( skill1 && !skill2 )
      return -1;
   if( !skill1 && !skill2 )
      return 0;
   if( skill1->type < skill2->type )
      return -1;
   if( skill1->type > skill2->type )
      return 1;
   return strcasecmp( skill1->name, skill2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table(  )
{
   log_string( "Sorting skill table..." );
   qsort( &skill_table[1], top_sn - 1, sizeof( SKILLTYPE * ), ( int ( * )( const void *, const void * ) )skill_comp );
}


/*
 * Write skill data to a file
 */
void fwrite_skill( FILE * fpout, SKILLTYPE * skill )
{
   SMAUG_AFF *aff;

   fprintf( fpout, "Name         %s~\n", skill->name );
   fprintf( fpout, "Type         %s\n", skill_tname[skill->type] );
   fprintf( fpout, "Flags        %d\n", skill->flags );
   if( skill->target )
      fprintf( fpout, "Target       %d\n", skill->target );
   if( skill->minimum_position )
      fprintf( fpout, "Minpos       %d\n", skill->minimum_position );
   if( skill->saves )
      fprintf( fpout, "Saves        %d\n", skill->saves );
   if( skill->slot )
      fprintf( fpout, "Slot         %d\n", skill->slot );
   if( skill->min_mana )
      fprintf( fpout, "Mana         %d\n", skill->min_mana );
   if( skill->beats )
      fprintf( fpout, "Rounds       %d\n", skill->beats );
   if( skill->guild != -1 )
      fprintf( fpout, "Guild        %d\n", skill->guild );
   if( skill->skill_fun )
      fprintf( fpout, "Code         %s\n", skill->skill_fun_name );
   else if( skill->spell_fun )
      fprintf( fpout, "Code         %s\n", skill->spell_fun_name );
   fprintf( fpout, "Dammsg       %s~\n", skill->noun_damage );
   if( skill->msg_off && skill->msg_off[0] != '\0' )
      fprintf( fpout, "Wearoff      %s~\n", skill->msg_off );

   if( skill->hit_char && skill->hit_char[0] != '\0' )
      fprintf( fpout, "Hitchar      %s~\n", skill->hit_char );
   if( skill->hit_vict && skill->hit_vict[0] != '\0' )
      fprintf( fpout, "Hitvict      %s~\n", skill->hit_vict );
   if( skill->hit_room && skill->hit_room[0] != '\0' )
      fprintf( fpout, "Hitroom      %s~\n", skill->hit_room );

   if( skill->miss_char && skill->miss_char[0] != '\0' )
      fprintf( fpout, "Misschar     %s~\n", skill->miss_char );
   if( skill->miss_vict && skill->miss_vict[0] != '\0' )
      fprintf( fpout, "Missvict     %s~\n", skill->miss_vict );
   if( skill->miss_room && skill->miss_room[0] != '\0' )
      fprintf( fpout, "Missroom     %s~\n", skill->miss_room );

   if( skill->die_char && skill->die_char[0] != '\0' )
      fprintf( fpout, "Diechar      %s~\n", skill->die_char );
   if( skill->die_vict && skill->die_vict[0] != '\0' )
      fprintf( fpout, "Dievict      %s~\n", skill->die_vict );
   if( skill->die_room && skill->die_room[0] != '\0' )
      fprintf( fpout, "Dieroom      %s~\n", skill->die_room );

   if( skill->imm_char && skill->imm_char[0] != '\0' )
      fprintf( fpout, "Immchar      %s~\n", skill->imm_char );
   if( skill->imm_vict && skill->imm_vict[0] != '\0' )
      fprintf( fpout, "Immvict      %s~\n", skill->imm_vict );
   if( skill->imm_room && skill->imm_room[0] != '\0' )
      fprintf( fpout, "Immroom      %s~\n", skill->imm_room );

   if( skill->dice && skill->dice[0] != '\0' )
      fprintf( fpout, "Dice         %s~\n", skill->dice );
   if( skill->value )
      fprintf( fpout, "Value        %d\n", skill->value );
   if( skill->difficulty )
      fprintf( fpout, "Difficulty   %d\n", skill->difficulty );
   if( skill->participants )
      fprintf( fpout, "Participants %d\n", skill->participants );
   if( skill->components && skill->components[0] != '\0' )
      fprintf( fpout, "Components   %s~\n", skill->components );
   if( skill->teachers && skill->teachers[0] != '\0' )
      fprintf( fpout, "Teachers     %s~\n", skill->teachers );
   for( aff = skill->first_affect; aff; aff = aff->next )
      fprintf( fpout, "Affect       '%s' %d '%s' %d\n", aff->duration, aff->location, aff->modifier, aff->bitvector );
   if( skill->alignment )
      fprintf( fpout, "Alignment   %d\n", skill->alignment );

   if( skill->type != SKILL_HERB )
   {
      fprintf( fpout, "Minlevel     %d\n", skill->min_level );
   }
   fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table( int delnum )
{
   int x;
   FILE *fpout;

   if( ( fpout = fopen( SKILL_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open skills.dat for writting", __func__ );
      perror( SKILL_FILE );
      return;
   }

   for( x = 0; x < top_sn; x++ )
   {
      if( x == delnum )
         continue;
      if( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
         break;
      fprintf( fpout, "#SKILL\n" );
      fwrite_skill( fpout, skill_table[x] );
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table(  )
{
   int x;
   FILE *fpout;

   if( ( fpout = fopen( HERB_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open herbs.dat for writting", __func__ );
      perror( HERB_FILE );
      return;
   }

   for( x = 0; x < top_herb; x++ )
   {
      if( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
         break;
      fprintf( fpout, "#HERB\n" );
      fwrite_skill( fpout, herb_table[x] );
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials(  )
{
   FILE *fpout;
   SOCIALTYPE *social;
   int x;

   if( ( fpout = fopen( SOCIAL_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open socials.dat for writting", __func__ );
      perror( SOCIAL_FILE );
      return;
   }

   for( x = 0; x < 27; x++ )
   {
      for( social = social_index[x]; social; social = social->next )
      {
         if( !social->name || social->name[0] == '\0' )
         {
            bug( "%s: blank social in hash bucket %d", __func__, x );
            continue;
         }
         fprintf( fpout, "#SOCIAL\n" );
         fprintf( fpout, "Name        %s~\n", social->name );
         if( social->char_no_arg )
            fprintf( fpout, "CharNoArg   %s~\n", social->char_no_arg );
         else
            bug( "%s: NULL char_no_arg in hash bucket %d", __func__, x );
         if( social->others_no_arg )
            fprintf( fpout, "OthersNoArg %s~\n", social->others_no_arg );
         if( social->char_found )
            fprintf( fpout, "CharFound   %s~\n", social->char_found );
         if( social->others_found )
            fprintf( fpout, "OthersFound %s~\n", social->others_found );
         if( social->vict_found )
            fprintf( fpout, "VictFound   %s~\n", social->vict_found );
         if( social->char_auto )
            fprintf( fpout, "CharAuto    %s~\n", social->char_auto );
         if( social->others_auto )
            fprintf( fpout, "OthersAuto  %s~\n", social->others_auto );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

int get_skill( const char *skilltype )
{
   if( !str_cmp( skilltype, "Spell" ) )
      return SKILL_SPELL;
   if( !str_cmp( skilltype, "Skill" ) )
      return SKILL_SKILL;
   if( !str_cmp( skilltype, "Weapon" ) )
      return SKILL_WEAPON;
   if( !str_cmp( skilltype, "Tongue" ) )
      return SKILL_TONGUE;
   if( !str_cmp( skilltype, "Herb" ) )
      return SKILL_HERB;
   return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 */
void save_commands(  )
{
   FILE *fpout;
   CMDTYPE *command;
   int x;

   if( ( fpout = fopen( COMMAND_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open commands.dat for writing", __func__ );
      perror( COMMAND_FILE );
      return;
   }

   for( x = 0; x < 126; x++ )
   {
      for( command = command_hash[x]; command; command = command->next )
      {
         if( !command->name || command->name[0] == '\0' )
         {
            bug( "%s: blank command in hash bucket %d", __func__, x );
            continue;
         }
         fprintf( fpout, "#COMMAND\n" );
         fprintf( fpout, "Name        %s~\n", command->name );
         fprintf( fpout, "Code        %s\n", command->fun_name ? command->fun_name : "" );   // Modded to use new field - Trax
         fprintf( fpout, "Position    %d\n", command->position );
         fprintf( fpout, "Level       %d\n", command->level );
         fprintf( fpout, "Log         %d\n", command->log );
         if( command->flags )
             fprintf( fpout, "Flags       %s~\n", flag_string( command->flags, cmd_flags ) );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

SKILLTYPE *fread_skill( FILE * fp )
{
   const char *word;
   bool fMatch;
   SKILLTYPE *skill;

   CREATE( skill, SKILLTYPE, 1 );

   skill->guild = -1;

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
            KEY( "Alignment", skill->alignment, fread_number( fp ) );
            if( !str_cmp( word, "Affect" ) )
            {
               SMAUG_AFF *aff;

               CREATE( aff, SMAUG_AFF, 1 );
               aff->duration = str_dup( fread_word( fp ) );
               aff->location = fread_number( fp );
               aff->modifier = str_dup( fread_word( fp ) );
               aff->bitvector = fread_number( fp );
               LINK( aff, skill->first_affect, skill->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            break;

         case 'C':
            if( !str_cmp( word, "Code" ) )
            {
               SPELL_FUN *spellfun;
               DO_FUN *dofun;
               char *w = fread_word( fp );

               fMatch = TRUE;
               if( !str_prefix( "do_", w ) && ( dofun = skill_function( w ) ) != skill_notfound )
               {
                  skill->skill_fun = dofun;
                  skill->spell_fun = NULL;
                  skill->skill_fun_name = str_dup( w );
               }
               else if( str_prefix( "do_", w ) && ( spellfun = spell_function( w ) ) != spell_notfound )
               {
                  skill->spell_fun = spellfun;
                  skill->skill_fun = NULL;
                  skill->spell_fun_name = str_dup( w );
               }
               else
               {
                  bug( "%s: unknown skill/spell %s", __func__, w );
                  skill->spell_fun = spell_null;
               }
               break;
            }
            KEY( "Components", skill->components, fread_string_nohash( fp ) );
            break;

         case 'D':
            KEY( "Dammsg", skill->noun_damage, fread_string_nohash( fp ) );
            KEY( "Dice", skill->dice, fread_string_nohash( fp ) );
            KEY( "Diechar", skill->die_char, fread_string_nohash( fp ) );
            KEY( "Dieroom", skill->die_room, fread_string_nohash( fp ) );
            KEY( "Dievict", skill->die_vict, fread_string_nohash( fp ) );
            KEY( "Difficulty", skill->difficulty, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
               return skill;
            break;

         case 'F':
            KEY( "Flags", skill->flags, fread_number( fp ) );
            break;

         case 'G':
            KEY( "Guild", skill->guild, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Hitchar", skill->hit_char, fread_string_nohash( fp ) );
            KEY( "Hitroom", skill->hit_room, fread_string_nohash( fp ) );
            KEY( "Hitvict", skill->hit_vict, fread_string_nohash( fp ) );
            break;

         case 'I':
            KEY( "Immchar", skill->imm_char, fread_string_nohash( fp ) );
            KEY( "Immroom", skill->imm_room, fread_string_nohash( fp ) );
            KEY( "Immvict", skill->imm_vict, fread_string_nohash( fp ) );
            break;

         case 'M':
            KEY( "Mana", skill->min_mana, fread_number( fp ) );
            KEY( "Minlevel", skill->min_level, fread_number( fp ) );
            KEY( "Minpos", skill->minimum_position, fread_number( fp ) );
            KEY( "Misschar", skill->miss_char, fread_string_nohash( fp ) );
            KEY( "Missroom", skill->miss_room, fread_string_nohash( fp ) );
            KEY( "Missvict", skill->miss_vict, fread_string_nohash( fp ) );
            break;

         case 'N':
            KEY( "Name", skill->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            KEY( "Participants", skill->participants, fread_number( fp ) );
            break;

         case 'R':
            KEY( "Rounds", skill->beats, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Slot", skill->slot, fread_number( fp ) );
            KEY( "Saves", skill->saves, fread_number( fp ) );
            break;

         case 'T':
            KEY( "Target", skill->target, fread_number( fp ) );
            KEY( "Teachers", skill->teachers, fread_string_nohash( fp ) );
            KEY( "Type", skill->type, get_skill( fread_word( fp ) ) );
            break;

         case 'V':
            KEY( "Value", skill->value, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Wearoff", skill->msg_off, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }
   }
}

void load_skill_table(  )
{
   FILE *fp;

   if( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
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
         if( !str_cmp( word, "SKILL" ) )
         {
            if( top_sn >= MAX_SKILL )
            {
               bug( "%s: more skills than MAX_SKILL %d", __func__, MAX_SKILL );
               FCLOSE( fp );
               return;
            }
            skill_table[top_sn++] = fread_skill( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      bug( "%s: Cannot open skills.dat", __func__ );
      exit( 0 );
   }
}

void load_herb_table(  )
{
   FILE *fp;

   if( ( fp = fopen( HERB_FILE, "r" ) ) != NULL )
   {
      top_herb = 0;
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
         if( !str_cmp( word, "HERB" ) )
         {
            if( top_herb >= MAX_HERB )
            {
               bug( "%s: more herbs than MAX_HERB %d", __func__, MAX_HERB );
               FCLOSE( fp );
               return;
            }
            herb_table[top_herb++] = fread_skill( fp );
            if( herb_table[top_herb - 1]->slot == 0 )
               herb_table[top_herb - 1]->slot = top_herb - 1;
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      bug( "%s: Cannot open herbs.dat", __func__ );
      exit( 0 );
   }
}

void fread_social( FILE * fp )
{
   const char *word;
   bool fMatch;
   SOCIALTYPE *social;

   CREATE( social, SOCIALTYPE, 1 );

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

         case 'C':
            KEY( "CharNoArg", social->char_no_arg, fread_string_nohash( fp ) );
            KEY( "CharFound", social->char_found, fread_string_nohash( fp ) );
            KEY( "CharAuto", social->char_auto, fread_string_nohash( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !social->name )
               {
                  bug( "%s: Name not found", __func__ );
                  free_social( social );
                  return;
               }
               if( !social->char_no_arg )
               {
                  bug( "%s: CharNoArg not found", __func__ );
                  free_social( social );
                  return;
               }
               add_social( social );
               return;
            }
            break;

         case 'N':
            KEY( "Name", social->name, fread_string_nohash( fp ) );
            break;

         case 'O':
            KEY( "OthersNoArg", social->others_no_arg, fread_string_nohash( fp ) );
            KEY( "OthersFound", social->others_found, fread_string_nohash( fp ) );
            KEY( "OthersAuto", social->others_auto, fread_string_nohash( fp ) );
            break;

         case 'V':
            KEY( "VictFound", social->vict_found, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }
   }
}

void load_socials(  )
{
   FILE *fp;

   if( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
   {
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
         if( !str_cmp( word, "SOCIAL" ) )
         {
            fread_social( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      bug( "%s: Cannot open socials.dat", __func__ );
      exit( 0 );
   }
}

void fread_command( FILE * fp )
{
   const char *word;
   bool fMatch;
   CMDTYPE *command;

   CREATE( command, CMDTYPE, 1 );

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

         case 'C':
            KEY( "Code", command->fun_name, str_dup( fread_word( fp ) ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !command->name )
               {
                  bug( "%s: Name not found", __func__ );
                  free_command( command );
                  return;
               }
               if( !command->fun_name )
               {
                  bug( "%s: No function name supplied for %s", __func__, command->name );
                  free_command( command );
                  return;
               }
               /*
                * Mods by Trax
                * Fread in code into char* and try linkage here then
                * deal in the "usual" way I suppose..
                */
               command->do_fun = skill_function( command->fun_name );
               if( command->do_fun == skill_notfound )
               {
                  bug( "%s: Function %s not found for %s", __func__, command->fun_name, command->name );
                  free_command( command );
                  return;
               }
               add_command( command );
               return;
            }
            break;
           case 'F':
               if( !str_cmp( word, "flags" ) )
               {
                   char *cmdflags = NULL;
                   char flag[MIL];
                   int value;

                   cmdflags = fread_flagstring( fp );

                   while( cmdflags[0] != '\0' )
                   {
                       cmdflags = one_argument( cmdflags, flag );
                       value = get_cmdflag( flag );
                       if( value < 0 || value > 31 )
                           bug( "%s: Unknown command flag: %s\r\n", __func__, flag );
                       else
                           TOGGLE_BIT( command->flags, 1 << value );
                   }
                   fMatch = TRUE;
                   break;
               }
               break;

         case 'L':
            KEY( "Level", command->level, fread_number( fp ) );
            KEY( "Log", command->log, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", command->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            KEY( "Position", command->position, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }
   }
}

void load_commands(  )
{
   FILE *fp;

   if( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
   {
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
         if( !str_cmp( word, "COMMAND" ) )
         {
            fread_command( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      bug( "%s: Cannot open commands.dat", 1 );
      exit( 1 );
   }
}
