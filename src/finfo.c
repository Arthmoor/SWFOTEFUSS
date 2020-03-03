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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

void fread_forcehelp( FORCE_HELP * fhelp, FILE * fp )
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
         case 'D':
            KEY( "Desc", fhelp->desc, fread_string( fp ) );
         case 'E':
            if( !str_cmp( word, "End" ) )
               return;
         case 'N':
            KEY( "Name", fhelp->name, fread_string( fp ) );
         case 'S':
            KEY( "Status", fhelp->status, fread_number( fp ) );
            KEY( "Skill", fhelp->skill, fread_number( fp ) );
         case 'T':
            KEY( "Type", fhelp->type, fread_number( fp ) );
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_forcehelp: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

bool load_forcehelp( char *forcehelpfile )
{
   char filename[256];
   FORCE_HELP *fhelp;
   FILE *fp;
   bool found;

   CREATE( fhelp, FORCE_HELP, 1 );

   found = FALSE;
   sprintf( filename, "%s%s", FORCE_HELP_DIR, forcehelpfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      LINK( fhelp, first_force_help, last_force_help, next, prev );
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
            bug( "Load_forcehelp: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "FORCE" ) )
         {
            fread_forcehelp( fhelp, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "Load_forcehelp: bad section: %s.", word );
            bug( buf, 0 );
            break;
         }
      }
      fclose( fp );
   }

   if( !( found ) )
      DISPOSE( fhelp );
   return found;
}

void load_force_help(  )
{
   FILE *fpList;
   char *filename;
   char forcehelpslist[256];
   char buf[MAX_STRING_LENGTH];


   first_force_help = NULL;
   last_force_help = NULL;

   log_string( "Loading force helps..." );

   sprintf( forcehelpslist, "%sforcehelps.lst", FORCE_HELP_DIR );
   if( ( fpList = fopen( forcehelpslist, "r" ) ) == NULL )
   {
      perror( forcehelpslist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;


      if( !load_forcehelp( filename ) )
      {
         sprintf( buf, "Cannot load forcehelp file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   log_string( " Done force helps " );
   return;
}

void write_forcehelp_list(  )
{
   FORCE_HELP *fhelp;
   FILE *fpout;
   char buf[MAX_STRING_LENGTH];
   char filename[256];

   sprintf( filename, "%s%s", FORCE_HELP_DIR, "forcehelps.lst" );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open forhelps.lst for writing!\n\r", 0 );
      return;
   }
   for( fhelp = first_force_help; fhelp; fhelp = fhelp->next )
   {
      sprintf( buf, "%s_%d", fhelp->name, fhelp->type );
      fprintf( fpout, "%s\n", buf );
   }
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

void write_all_forcehelps(  )
{
   FORCE_HELP *fhelp;
   for( fhelp = first_force_help; fhelp; fhelp = fhelp->next )
      save_forcehelp( fhelp );
   write_forcehelp_list(  );
}

void save_forcehelp( FORCE_HELP * fhelp )
{
   FILE *fp;
   char filename[256];

   if( !fhelp )
   {
      bug( "save_forcehelp: null forcehelp pointer!", 0 );
      return;
   }

   sprintf( filename, "%s%s_%d", FORCE_HELP_DIR, fhelp->name, fhelp->type );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_forcehelp: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#FORCE\n" );
      if( !fhelp->name || fhelp->name[0] == '\0' )
         fprintf( fp, "Name      ~\n" );
      else
         fprintf( fp, "Name      %s~\n", fhelp->name );
      fprintf( fp, "Status    %d\n", fhelp->status );
      fprintf( fp, "Type      %d\n", fhelp->type );
      if( !fhelp->desc || fhelp->desc[0] == '\0' )
         fprintf( fp, "Desc      ~\n" );
      else
         fprintf( fp, "Desc      %s~\n", fhelp->desc );
      fprintf( fp, "Skill     %d\n", fhelp->skill );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );

   }
   fclose( fp );
   return;
}

void fread_forceskill( FORCE_SKILL * fskill, FILE * fp )
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
            KEY( "Alter", fskill->alter, fread_number( fp ) );
         case 'C':
            KEY( "ChEffect0", fskill->ch_effect[0], fread_string( fp ) );
            KEY( "ChEffect1", fskill->ch_effect[1], fread_string( fp ) );
            KEY( "ChEffect2", fskill->ch_effect[2], fread_string( fp ) );
            KEY( "ChEffect3", fskill->ch_effect[3], fread_string( fp ) );
            KEY( "ChEffect4", fskill->ch_effect[4], fread_string( fp ) );
            KEY( "Code", fskill->code, fread_string( fp ) );
            KEY( "Cost", fskill->cost, fread_number( fp ) );
            KEY( "Control", fskill->control, fread_number( fp ) );
         case 'D':
            KEY( "Disabled", fskill->disabled, fread_number( fp ) );
         case 'E':
            if( !str_cmp( word, "End" ) )
               return;
         case 'I':
            KEY( "Index", fskill->index, fread_number( fp ) );
         case 'M':
            KEY( "Mastertrain", fskill->mastertrain, fread_number( fp ) );
         case 'N':
            KEY( "Name", fskill->name, fread_string( fp ) );
            KEY( "Notskill", fskill->notskill, fread_number( fp ) );
         case 'R':
            KEY( "RoomEffect0", fskill->room_effect[0], fread_string( fp ) );
            KEY( "RoomEffect1", fskill->room_effect[1], fread_string( fp ) );
            KEY( "RoomEffect2", fskill->room_effect[2], fread_string( fp ) );
            KEY( "RoomEffect3", fskill->room_effect[3], fread_string( fp ) );
            KEY( "RoomEffect4", fskill->room_effect[4], fread_string( fp ) );
         case 'S':
            KEY( "Sense", fskill->sense, fread_number( fp ) );
            KEY( "Status", fskill->status, fread_number( fp ) );
         case 'T':
            KEY( "Type", fskill->type, fread_number( fp ) );
         case 'V':
            KEY( "VictimEffect0", fskill->victim_effect[0], fread_string( fp ) );
            KEY( "VictimEffect1", fskill->victim_effect[1], fread_string( fp ) );
            KEY( "VictimEffect2", fskill->victim_effect[2], fread_string( fp ) );
            KEY( "VictimEffect3", fskill->victim_effect[3], fread_string( fp ) );
            KEY( "VictimEffect4", fskill->victim_effect[4], fread_string( fp ) );
         case 'W':
            KEY( "WaitState", fskill->wait_state, fread_number( fp ) );
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_forceskill: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

bool load_forceskill( char *forceskillfile )
{
   char filename[256];
   FORCE_SKILL *fskill;
   FILE *fp;
   DO_FUN *fun;
   bool found;

   CREATE( fskill, FORCE_SKILL, 1 );

   found = FALSE;
   sprintf( filename, "%s%s", FORCE_DIR, forceskillfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      LINK( fskill, first_force_skill, last_force_skill, next, prev );
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
            bug( "Load_forceskill: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "FORCE" ) )
         {
            fread_forceskill( fskill, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "Load_forceskill: bad section: %s.", word );
            bug( buf, 0 );
            break;
         }
      }
      fclose( fp );
   }

   if( !( found ) )
      DISPOSE( fskill );
   fun = get_force_skill_function( fskill->code );
   fskill->do_fun = fun;
   return found;
}

void load_force_skills(  )
{
   FILE *fpList;
   char *filename;
   char forceskillslist[256];
   char buf[MAX_STRING_LENGTH];


   first_force_skill = NULL;
   last_force_skill = NULL;

   log_string( "Loading force skills..." );

   sprintf( forceskillslist, "%sforceskills.lst", FORCE_DIR );
   if( ( fpList = fopen( forceskillslist, "r" ) ) == NULL )
   {
      perror( forceskillslist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;


      if( !load_forceskill( filename ) )
      {
         sprintf( buf, "Cannot load forceskill file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   log_string( " Done force skills " );
   return;
}

DO_FUN *get_force_skill_function( char *name )
{
   if( !name || name[0] == '\0' || strlen( name ) < 8 )
      return skill_notfound;
   switch ( name[7] )
   {
      case 'a':
         if( !str_cmp( name, "fskill_awareness" ) )
            return fskill_awareness;
      case 'c':
         if( !str_cmp( name, "fskill_convert" ) )
            return fskill_convert;
         break;
      case 'f':
         if( !str_cmp( name, "fskill_fdisguise" ) )
            return fskill_fdisguise;
         if( !str_cmp( name, "fskill_fhelp" ) )
            return fskill_fhelp;
         if( !str_cmp( name, "fskill_finfo" ) )
            return fskill_finfo;
         if( !str_cmp( name, "fskill_finish" ) )
            return fskill_finish;
         if( !str_cmp( name, "fskill_force_lightning" ) )
            return fskill_force_lightning;
         if( !str_cmp( name, "fskill_fshield" ) )
            return fskill_fshield;
         break;
      case 'h':
         if( !str_cmp( name, "fskill_heal" ) )
            return fskill_heal;
         break;
      case 'i':
         if( !str_cmp( name, "fskill_identify" ) )
            return fskill_identify;
         if( !str_cmp( name, "fskill_instruct" ) )
            return fskill_instruct;
         break;
      case 'm':
         if( !str_cmp( name, "fskill_master" ) )
            return fskill_master;
         if( !str_cmp( name, "fskill_makelightsaber" ) )
            return fskill_makelightsaber;
         if( !str_cmp( name, "fskill_makedualsaber" ) )
            return fskill_makedualsaber;
         break;
      case 'p':
         if( !str_cmp( name, "fskill_promote" ) )
            return fskill_promote;
         if( !str_cmp( name, "fskill_protect" ) )
            return fskill_protect;
         break;
      case 'r':
         if( !str_cmp( name, "fskill_refresh" ) )
            return fskill_refresh;
         break;
      case 's':
         if( !str_cmp( name, "fskill_squeeze" ) )
            return fskill_squeeze;
         if( !str_cmp( name, "fskill_student" ) )
            return fskill_student;
         if( !str_cmp( name, "fskill_slash" ) )
            return fskill_slash;
         break;
      case 'w':
         if( !str_cmp( name, "fskill_whirlwind" ) )
            return fskill_whirlwind;
         break;
      default:
         break;
   }
   return skill_notfound;
}

void write_forceskill_list(  )
{
   FORCE_SKILL *fskill;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", FORCE_DIR, "forceskills.lst" );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open forskills.lst for writing!\n\r", 0 );
      return;
   }
   for( fskill = first_force_skill; fskill; fskill = fskill->next )
      fprintf( fpout, "%s\n", fskill->name );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

void write_all_forceskills(  )
{
   FORCE_SKILL *fskill;
   for( fskill = first_force_skill; fskill; fskill = fskill->next )
      save_forceskill( fskill );
   write_forceskill_list(  );
}

void save_forceskill( FORCE_SKILL * fskill )
{
   FILE *fp;
   char filename[256];

   if( !fskill )
   {
      bug( "save_forceskill: null forceskill pointer!", 0 );
      return;
   }

   sprintf( filename, "%s%s", FORCE_DIR, fskill->name );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_forceskill: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#FORCE\n" );
      fprintf( fp, "Alter          %d\n", fskill->alter );

      if( fskill->ch_effect[0][0] != '\0' )
         fprintf( fp, "ChEffect0     %s~\n", fskill->ch_effect[0] );
      else
         fprintf( fp, "ChEffect0     ~\n" );
      if( fskill->ch_effect[1][0] != '\0' )
         fprintf( fp, "ChEffect1     %s~\n", fskill->ch_effect[1] );
      else
         fprintf( fp, "ChEffect1     ~\n" );
      if( fskill->ch_effect[2][0] != '\0' )
         fprintf( fp, "ChEffect2     %s~\n", fskill->ch_effect[2] );
      else
         fprintf( fp, "ChEffect2     ~\n" );
      if( fskill->ch_effect[3][0] != '\0' )
         fprintf( fp, "ChEffect3     %s~\n", fskill->ch_effect[3] );
      else
         fprintf( fp, "ChEffect3     ~\n" );
      if( fskill->ch_effect[4][0] != '\0' )
         fprintf( fp, "ChEffect4     %s~\n", fskill->ch_effect[4] );
      else
         fprintf( fp, "ChEffect4     ~\n" );

      fprintf( fp, "Code           %s~\n", fskill->code );
      fprintf( fp, "Cost           %d\n", fskill->cost );
      fprintf( fp, "Control        %d\n", fskill->control );
      fprintf( fp, "Disabled       %d\n", fskill->disabled );
      fprintf( fp, "Mastertrain    %d\n", fskill->mastertrain );
      fprintf( fp, "Index          %d\n", fskill->index );
      fprintf( fp, "Name           %s~\n", fskill->name );
      fprintf( fp, "Notskill       %d\n", fskill->notskill );

      if( fskill->room_effect[0][0] != '\0' )
         fprintf( fp, "RoomEffect0     %s~\n", fskill->room_effect[0] );
      else
         fprintf( fp, "RoomEffect0     ~\n" );
      if( fskill->room_effect[1][0] != '\0' )
         fprintf( fp, "RoomEffect1     %s~\n", fskill->room_effect[1] );
      else
         fprintf( fp, "RoomEffect1     ~\n" );
      if( fskill->room_effect[2][0] != '\0' )
         fprintf( fp, "RoomEffect2     %s~\n", fskill->room_effect[2] );
      else
         fprintf( fp, "RoomEffect2     ~\n" );
      if( fskill->room_effect[3][0] != '\0' )
         fprintf( fp, "RoomEffect3     %s~\n", fskill->room_effect[3] );
      else
         fprintf( fp, "RoomEffect3     ~\n" );
      if( fskill->room_effect[4][0] != '\0' )
         fprintf( fp, "RoomEffect4     %s~\n", fskill->room_effect[4] );
      else
         fprintf( fp, "RoomEffect4     ~\n" );

      fprintf( fp, "Sense          %d\n", fskill->sense );
      fprintf( fp, "Status         %d\n", fskill->status );
      fprintf( fp, "Type           %d\n", fskill->type );

      if( fskill->victim_effect[0][0] != '\0' )
         fprintf( fp, "VictimEffect0     %s~\n", fskill->victim_effect[0] );
      else
         fprintf( fp, "VictimEffect0     ~\n" );
      if( fskill->victim_effect[1][0] != '\0' )
         fprintf( fp, "VictimEffect1     %s~\n", fskill->victim_effect[1] );
      else
         fprintf( fp, "VictimEffect1     ~\n" );
      if( fskill->victim_effect[2][0] != '\0' )
         fprintf( fp, "VictimEffect2     %s~\n", fskill->victim_effect[2] );
      else
         fprintf( fp, "VictimEffect2     ~\n" );
      if( fskill->victim_effect[3][0] != '\0' )
         fprintf( fp, "VictimEffect3     %s~\n", fskill->victim_effect[3] );
      else
         fprintf( fp, "VictimEffect3     ~\n" );
      if( fskill->victim_effect[4][0] != '\0' )
         fprintf( fp, "VictimEffect4     %s~\n", fskill->victim_effect[4] );
      else
         fprintf( fp, "VictimEffect4     ~\n" );

      fprintf( fp, "WaitState      %d\n", fskill->wait_state );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}
