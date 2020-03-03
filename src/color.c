/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine, and Adjani.    *
 * All Rights Reserved.                                                     *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *               Color Module -- Allow user customizable Colors.            *
 *                                   --Matthew                              *
 *                      Enhanced ANSI parser by Samson                      *
 ****************************************************************************/

/*
* The following instructions assume you know at least a little bit about
* coding.  I firmly believe that if you can't code (at least a little bit),
* you don't belong running a mud.  So, with that in mind, I don't hold your
* hand through these instructions.
*
* You may use this code provided that:
*
*     1)  You understand that the authors _DO NOT_ support this code
*         Any help you need must be obtained from other sources.  The
*         authors will ignore any and all requests for help.
*     2)  You will mention the authors if someone asks about the code.
*         You will not take credit for the code, but you can take credit
*         for any enhancements you make.
*     3)  This message remains intact.
*
* If you would like to find out how to send the authors large sums of money,
* you may e-mail the following address:
*
* Matthew Bafford & Christopher Wigginton
* wiggy@mudservices.com
*/

/*
 * To add new color types:
 *
 * 1.  Edit color.h, and:
 *     1.  Add a new AT_ define.
 *     2.  Increment MAX_COLORS by however many AT_'s you added.
 * 2.  Edit color.c and:
 *     1.  Add the name(s) for your new color(s) to the end of the pc_displays array.
 *     2.  Add the default color(s) to the end of the default_set array.
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include "mud.h"

char *const pc_displays[MAX_COLORS] = {
   "black", "dred", "dgreen", "orange",
   "dblue", "purple", "cyan", "grey",
   "dgrey", "red", "green", "yellow",
   "blue", "pink", "lblue", "white",
   "blink", "plain", "action", "say",
   "chat", "yells", "tell", "hit",
   "hitme", "immortal", "hurting", "falling",
   "danger", "magic", "consider", "report",
   "poison", "social", "dying", "dead",
   "skills", "carnage", "damage", "fleeing",
   "rmname", "rmdesc", "objects", "people",
   "list", "bye", "gold", "gtells",
   "note", "hungry", "thirsty", "fire",
   "sober", "wearoff", "exits", "score",
   "reset", "log", "die_msg", "wartalk",
   "arena", "muse", "think", "aflags",
   "who", "racetalk", "ignore", "whisper",
   "divider", "morph", "shout", "rflags",
   "stype", "aname", "auction", "score2",
   "score3", "score4", "who2", "who3",
   "who4", "intermud", "helpfiles", "who5",
   "score5", "who6", "who7", "prac",
   "prac2", "prac3", "prac4", "mxpprompt",
   "guildtalk", "ooc", "avatar", "ship",
   "clan"
};

/* All defaults are set to Alsherok default scheme, if you don't 
like it, change it around to suite your own needs - Samson */
const short default_set[MAX_COLORS] = {
   AT_BLACK, AT_BLOOD, AT_DGREEN, AT_ORANGE, /*  3 */
   AT_DBLUE, AT_PURPLE, AT_CYAN, AT_GREY, /*  7 */
   AT_DGREY, AT_RED, AT_GREEN, AT_YELLOW, /* 11 */
   AT_BLUE, AT_PINK, AT_LBLUE, AT_WHITE,  /* 15 */

   AT_RED + AT_BLINK, AT_GREY, AT_GREY, AT_BLUE,   /* 19 */
   AT_GREEN, AT_LBLUE, AT_WHITE, AT_GREY, /* 23 */
   AT_GREY, AT_YELLOW, AT_GREY, AT_GREY,  /* 27 */
   AT_GREY, AT_BLUE, AT_GREY, AT_GREY, /* 31 */
   AT_DGREEN, AT_CYAN, AT_GREY, AT_GREY,  /* 35 */
   AT_BLUE, AT_GREY, AT_GREY, AT_GREY, /* 39 */
   AT_RED, AT_GREY, AT_BLUE, AT_PINK,  /* 43 */
   AT_GREY, AT_GREY, AT_YELLOW, AT_GREY,  /* 47 */
   AT_GREY, AT_ORANGE, AT_BLUE, AT_RED,   /* 51 */
   AT_GREY, AT_GREY, AT_GREEN, AT_DGREEN, /* 55 */
   AT_DGREEN, AT_ORANGE, AT_GREY, AT_RED, /* 59 */
   AT_GREY, AT_DGREEN, AT_RED, AT_BLUE,   /* 63 */
   AT_RED, AT_CYAN, AT_YELLOW, AT_PINK,   /* 67 */
   AT_DGREEN, AT_PINK, AT_WHITE, AT_BLUE, /* 71 */
   AT_BLUE, AT_BLUE, AT_GREEN, AT_GREY,   /* 75 */
   AT_GREEN, AT_GREEN, AT_YELLOW, AT_DGREY,  /* 79 */
   AT_GREEN, AT_PINK, AT_DGREEN, AT_CYAN, /* 83 */
   AT_RED, AT_WHITE, AT_BLUE, AT_DGREEN,  /* 87 */
   AT_CYAN, AT_BLOOD, AT_RED, AT_DGREEN,  /* 91 */
   AT_GREEN, AT_GREEN, AT_GREEN, AT_GREEN,   /* 95 */
   AT_GREEN
};

char *const valid_color[] = {
   "black",
   "dred",
   "dgreen",
   "orange",
   "dblue",
   "purple",
   "cyan",
   "grey",
   "dgrey",
   "red",
   "green",
   "yellow",
   "blue",
   "pink",
   "lblue",
   "white",
   "\0"
};

/* Color align functions by Justice@Aaern */
int const_color_str_len( const char *argument )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
#ifdef OVERLANDCODE
      else if( argument[str] == '{' )
#else
      else if( argument[str] == '^' )
#endif
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '}' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }
   }

   return count;
}

int const_color_strnlen( const char *argument, int maxlength )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
#ifdef OVERLANDCODE
      else if( argument[str] == '{' )
#else
      else if( argument[str] == '^' )
#endif
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '}' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }

      if( count >= maxlength )
         break;
   }
   if( count < maxlength )
      return ( ( str - count ) + maxlength );

   str++;
   return str;
}

int color_str_len( char *argument )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
#ifdef OVERLANDCODE
      else if( argument[str] == '{' )
#else
      else if( argument[str] == '^' )
#endif
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '}' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }
   }

   return count;
}

int color_strnlen( char *argument, int maxlength )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
#ifdef OVERLANDCODE
      else if( argument[str] == '{' )
#else
      else if( argument[str] == '^' )
#endif
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '}' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }

      if( count >= maxlength )
         break;
   }
   if( count < maxlength )
      return ( ( str - count ) + maxlength );

   str++;
   return str;
}

const char *const_color_align( const char *argument, int size, int align )
{
   int space = ( size - const_color_str_len( argument ) );
   static char buf[MAX_STRING_LENGTH];

   if( align == ALIGN_RIGHT || const_color_str_len( argument ) >= size )
      snprintf( buf, MAX_STRING_LENGTH, "%*.*s", const_color_strnlen( argument, size ),
                const_color_strnlen( argument, size ), argument );
   else if( align == ALIGN_CENTER )
      snprintf( buf, MAX_STRING_LENGTH, "%*s%s%*s", ( space / 2 ), "", argument,
                ( ( space / 2 ) * 2 ) == space ? ( space / 2 ) : ( ( space / 2 ) + 1 ), "" );
   else
      snprintf( buf, MAX_STRING_LENGTH, "%s%*s", argument, space, "" );

   return buf;
}

char *color_align( char *argument, int size, int align )
{
   int space = ( size - color_str_len( argument ) );
   static char buf[MAX_STRING_LENGTH];

   if( align == ALIGN_RIGHT || color_str_len( argument ) >= size )
      snprintf( buf, MAX_STRING_LENGTH, "%*.*s", color_strnlen( argument, size ), color_strnlen( argument, size ),
                argument );
   else if( align == ALIGN_CENTER )
      snprintf( buf, MAX_STRING_LENGTH, "%*s%s%*s", ( space / 2 ), "", argument,
                ( ( space / 2 ) * 2 ) == space ? ( space / 2 ) : ( ( space / 2 ) + 1 ), "" );
   else if( align == ALIGN_LEFT )
      snprintf( buf, MAX_STRING_LENGTH, "%s%*s", argument, space, "" );

   return buf;
}

void show_colorthemes( CHAR_DATA * ch )
{
   DIR *dp;
   struct dirent *dentry;
   int count = 0, col = 0;

   send_to_pager( "&YThe following themes are available:\n\r", ch );

   dp = opendir( COLOR_DIR );
   dentry = readdir( dp );
   while( dentry )
   {
      /*
       * Added by Tarl 3 Dec 02 because we are now using CVS 
       */
      if( !str_cmp( dentry->d_name, "CVS" ) )
      {
         dentry = readdir( dp );
         continue;
      }
      if( dentry->d_name[0] != '.' )
      {
         count++;
         pager_printf( ch, "%s%-15.15s", color_str( AT_PLAIN, ch ), dentry->d_name );
         if( ++col % 6 == 0 )
            send_to_pager( "\n\r", ch );
      }
      dentry = readdir( dp );
   }
   closedir( dp );

   if( count == 0 )
      send_to_pager( "No themes defined yet.\n\r", ch );

   if( col % 6 != 0 )
      send_to_pager( "\n\r", ch );
   return;
}

void show_colors( CHAR_DATA * ch )
{
   short count;

   send_to_pager( "&BSyntax: color [color type] [color] | default\n\r", ch );
   send_to_pager( "&BSyntax: color _reset_ (Resets all colors to default set)\n\r", ch );
   send_to_pager( "&BSyntax: color _all_ [color] (Sets all color types to [color])\n\r", ch );
   send_to_pager( "&BSyntax: color theme [name] (Sets all color types to a defined theme)\n\r\n\r", ch );

   send_to_pager( "&W********************************[ COLORS ]*********************************\n\r", ch );

   for( count = 0; count < 16; ++count )
   {
      if( ( count % 8 ) == 0 && count != 0 )
         send_to_pager( "\n\r", ch );
      pager_printf( ch, "%s%-10s", color_str( count, ch ), pc_displays[count] );
   }

   send_to_pager( "\n\r\n\r&W******************************[ COLOR TYPES ]******************************\n\r", ch );

   for( count = 16; count < MAX_COLORS; ++count )
   {
      if( ( count % 8 ) == 0 && count != 16 )
         send_to_pager( "\n\r", ch );
      pager_printf( ch, "%s%-10s%s", color_str( count, ch ), pc_displays[count], ANSI_RESET );
   }
   send_to_pager( "\n\r\n\r", ch );
   send_to_pager( "&YAvailable colors are:\n\r", ch );

   for( count = 0; valid_color[count][0] != '\0'; ++count )
   {
      if( ( count % 8 ) == 0 && count != 0 )
         send_to_pager( "\n\r", ch );

      pager_printf( ch, "%s%-10s", color_str( AT_PLAIN, ch ), valid_color[count] );
   }
   send_to_pager( "\n\r\n\r", ch );
   show_colorthemes( ch );
   return;
}

void do_color( CHAR_DATA * ch, char *argument )
{
   bool dMatch, cMatch;
   short count = 0, y = 0;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   dMatch = FALSE;
   cMatch = FALSE;

   if( IS_NPC( ch ) )
   {
      send_to_pager( "Only PC's can change colors.\n\r", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      show_colors( ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "savetheme" ) && IS_IMMORTAL( ch ) )
   {
      FILE *fp;
      char filename[256];
      int x;

      if( !argument || argument[0] == '\0' )
      {
         ch_printf( ch, "You must specify a name for this theme to save it.\n\r" );
         return;
      }

      snprintf( filename, 256, "%s%s", COLOR_DIR, argument );
      if( !( fp = fopen( filename, "w" ) ) )
      {
         ch_printf( ch, "Unable to write to color file %s\n\r", filename );
         return;
      }
      fprintf( fp, "%s", "#COLORTHEME\n" );
      fprintf( fp, "Name         %s~\n", argument );
      fprintf( fp, "MaxColors    %d\n", MAX_COLORS );
      fprintf( fp, "%s", "Colors      " );
      for( x = 0; x < MAX_COLORS; x++ )
         fprintf( fp, " %d", ch->colors[x] );
      fprintf( fp, "%s", "\nEnd\n" );
      fclose( fp );
      fp = NULL;
      ch_printf( ch, "Color theme %s saved.\n\r", argument );
      return;
   }

   if( !str_cmp( arg, "theme" ) )
   {
      FILE *fp;
      char filename[256];
      int max_colors = 0, x;

      if( !argument || argument[0] == '\0' )
      {
         show_colorthemes( ch );
         return;
      }

      snprintf( filename, 256, "%s%s", COLOR_DIR, argument );
      if( !( fp = fopen( filename, "r" ) ) )
      {
         ch_printf( ch, "There is no theme called %s.\n\r", argument );
         return;
      }

      while( !feof( fp ) )
      {
         char *word = fread_word( fp );
         if( !str_cmp( word, "MaxColors" ) )
         {
            max_colors = fread_number( fp );
            continue;
         }
         if( !str_cmp( word, "Colors" ) )
         {
            for( x = 0; x < max_colors; x++ )
               ch->colors[x] = fread_number( fp );
            continue;
         }
         if( !str_cmp( word, "End" ) )
         {
            fclose( fp );
            fp = NULL;
            ch_printf( ch, "Color theme has been changed to %s.\n\r", argument );
            save_char_obj( ch );
            return;
         }
      }
      ch_printf( ch, "An error occured while trying to set color theme %s.\n\r", argument );
      return;
   }

   if( !str_cmp( arg, "ansitest" ) )
   {
      sprintf( log_buf, "%sBlack\n\r", ANSI_BLACK );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Red\n\r", ANSI_DRED );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Green\n\r", ANSI_DGREEN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sOrange/Brown\n\r", ANSI_ORANGE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Blue\n\r", ANSI_DBLUE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sPurple\n\r", ANSI_PURPLE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sCyan\n\r", ANSI_CYAN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sGrey\n\r", ANSI_GREY );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Grey\n\r", ANSI_DGREY );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sRed\n\r", ANSI_RED );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sGreen\n\r", ANSI_GREEN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sYellow\n\r", ANSI_YELLOW );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sBlue\n\r", ANSI_BLUE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sPink\n\r", ANSI_PINK );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sLight Blue\n\r", ANSI_LBLUE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sWhite\n\r", ANSI_WHITE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sBlack\n\r", BLINK_BLACK );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Red\n\r", BLINK_DRED );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Green\n\r", BLINK_DGREEN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sOrange/Brown\n\r", BLINK_ORANGE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Blue\n\r", BLINK_DBLUE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sPurple\n\r", BLINK_PURPLE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sCyan\n\r", BLINK_CYAN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sGrey\n\r", BLINK_GREY );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sDark Grey\n\r", BLINK_DGREY );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sRed\n\r", BLINK_RED );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sGreen\n\r", BLINK_GREEN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sYellow\n\r", BLINK_YELLOW );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sBlue\n\r", BLINK_BLUE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sPink\n\r", BLINK_PINK );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sLight Blue\n\r", BLINK_LBLUE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%sWhite\n\r", BLINK_WHITE );
      write_to_buffer( ch->desc, log_buf, 0 );
      write_to_buffer( ch->desc, ANSI_RESET, 0 );
      sprintf( log_buf, "%s%sBlack\n\r", ANSI_WHITE, BACK_BLACK );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%s%sDark Red\n\r", ANSI_BLACK, BACK_DRED );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%s%sDark Green\n\r", ANSI_BLACK, BACK_DGREEN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%s%sOrange/Brown\n\r", ANSI_BLACK, BACK_ORANGE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%s%sDark Blue\n\r", ANSI_BLACK, BACK_DBLUE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%s%sPurple\n\r", ANSI_BLACK, BACK_PURPLE );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%s%sCyan\n\r", ANSI_BLACK, BACK_CYAN );
      write_to_buffer( ch->desc, log_buf, 0 );
      sprintf( log_buf, "%s%sGrey\n\r", ANSI_BLACK, BACK_GREY );
      write_to_buffer( ch->desc, log_buf, 0 );
      write_to_buffer( ch->desc, ANSI_RESET, 0 );

      return;
   }

   if( !str_prefix( arg, "_reset_" ) )
   {
      reset_colors( ch );
      send_to_pager( "All color types reset to default colors.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg2 );

   if( arg[0] == '\0' )
   {
      send_to_char( "Change which color type?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg3 );

   if( !str_prefix( arg, "_all_" ) )
   {
      dMatch = TRUE;
      count = -1;

      /*
       * search for a valid color setting
       */
      for( y = 0; y < 16; y++ )
      {
         if( !str_cmp( arg2, valid_color[y] ) )
         {
            cMatch = TRUE;
            break;
         }
      }
   }
   else if( arg2[0] == '\0' )
   {
      cMatch = FALSE;
   }
   else
   {
      /*
       * search for the display type and str_cmp
       */
      for( count = 0; count < MAX_COLORS; count++ )
      {
         if( !str_prefix( arg, pc_displays[count] ) )
         {
            dMatch = TRUE;
            break;
         }
      }

      if( !dMatch )
      {
         ch_printf( ch, "%s is an invalid color type.\n\r", arg );
         send_to_char( "Type color with no arguments to see available options.\n\r", ch );
         return;
      }

      if( !str_cmp( arg2, "default" ) )
      {
         ch->colors[count] = default_set[count];
         sprintf( buf, "Display %s set back to default.\n\r", pc_displays[count] );
         send_to_pager( buf, ch );
         return;
      }

      /*
       * search for a valid color setting
       */
      for( y = 0; y < 16; y++ )
      {
         if( !str_cmp( arg2, valid_color[y] ) )
         {
            cMatch = TRUE;
            break;
         }
      }
   }

   if( !cMatch )
   {
      if( arg[0] )
      {
         ch_printf( ch, "Invalid color for type %s.\n", arg );
      }
      else
      {
         send_to_pager( "Invalid color.\n\r", ch );
      }

      send_to_pager( "Choices are:\n\r", ch );

      for( count = 0; count < 16; count++ )
      {
         if( count % 5 == 0 && count != 0 )
            send_to_pager( "\r\n", ch );

         pager_printf( ch, "%-10s", valid_color[count] );
      }

      pager_printf( ch, "%-10s\r\n", "default" );
      return;
   }
   else
   {
      sprintf( buf, "Color type %s set to color %s.\n\r", count == -1 ? "_all_" : pc_displays[count], valid_color[y] );
   }

   if( !str_cmp( arg3, "blink" ) )
   {
      y += AT_BLINK;
   }

   if( count == -1 )
   {
      int ccount;

      for( ccount = 0; ccount < MAX_COLORS; ++ccount )
      {
         ch->colors[ccount] = y;
      }

      set_pager_color( y, ch );

      sprintf( buf, "All color types set to color %s%s.%s\n\r",
               valid_color[y > AT_BLINK ? y - AT_BLINK : y], y > AT_BLINK ? " [BLINKING]" : "", ANSI_RESET );

      send_to_pager( buf, ch );
   }
   else
   {
      ch->colors[count] = y;

      set_pager_color( count, ch );

      if( !str_cmp( arg3, "blink" ) )
         sprintf( buf, "Display %s set to color %s [BLINKING]%s\n\r", pc_displays[count], valid_color[y - AT_BLINK],
                  ANSI_RESET );
      else
         sprintf( buf, "Display %s set to color %s.\n\r", pc_displays[count], valid_color[y] );

      send_to_pager( buf, ch );
   }
   set_pager_color( AT_PLAIN, ch );

   return;
}

/*
 * If the color directory has a file named "default" this will be used if possible.
 * The fallback will go to the hardcoded table in color.h if the "default" file is not available.
 */
void reset_colors( CHAR_DATA * ch )
{
   FILE *fp;
   int max_colors = 0;
   char filename[256];

   snprintf( filename, 256, "%s%s", COLOR_DIR, "default" );

   if( !( fp = fopen( filename, "r" ) ) )
   {
      memcpy( &ch->colors, &default_set, sizeof( default_set ) );
      return;
   }

   while( !feof( fp ) )
   {
      char *word = fread_word( fp );
      if( !str_cmp( word, "MaxColors" ) )
      {
         max_colors = fread_number( fp );
         continue;
      }
      if( !str_cmp( word, "Colors" ) )
      {
         int x;

         for( x = 0; x < max_colors; ++x )
            ch->colors[x] = fread_number( fp );
         continue;
      }
      if( !str_cmp( word, "End" ) )
      {
         fclose( fp );
         fp = NULL;
         return;
      }
   }
   fclose( fp );
   fp = NULL;
   return;
}

char *color_str( short AType, CHAR_DATA * ch )
{
   if( !ch )
   {
      bug( "%s", "color_str: NULL ch!" );
      return ( "" );
   }

   if( IS_NPC( ch ) || !IS_SET( ch->act, PLR_ANSI ) )
      return ( "" );

   switch ( ch->colors[AType] )
   {
      case 0:
         return ( ANSI_BLACK );
      case 1:
         return ( ANSI_DRED );
      case 2:
         return ( ANSI_DGREEN );
      case 3:
         return ( ANSI_ORANGE );
      case 4:
         return ( ANSI_DBLUE );
      case 5:
         return ( ANSI_PURPLE );
      case 6:
         return ( ANSI_CYAN );
      case 7:
         return ( ANSI_GREY );
      case 8:
         return ( ANSI_DGREY );
      case 9:
         return ( ANSI_RED );
      case 10:
         return ( ANSI_GREEN );
      case 11:
         return ( ANSI_YELLOW );
      case 12:
         return ( ANSI_BLUE );
      case 13:
         return ( ANSI_PINK );
      case 14:
         return ( ANSI_LBLUE );
      case 15:
         return ( ANSI_WHITE );

         /*
          * 16 thru 31 are for blinking colors 
          */
      case 16:
         return ( BLINK_BLACK );
      case 17:
         return ( BLINK_DRED );
      case 18:
         return ( BLINK_DGREEN );
      case 19:
         return ( BLINK_ORANGE );
      case 20:
         return ( BLINK_DBLUE );
      case 21:
         return ( BLINK_PURPLE );
      case 22:
         return ( BLINK_CYAN );
      case 23:
         return ( BLINK_GREY );
      case 24:
         return ( BLINK_DGREY );
      case 25:
         return ( BLINK_RED );
      case 26:
         return ( BLINK_GREEN );
      case 27:
         return ( BLINK_YELLOW );
      case 28:
         return ( BLINK_BLUE );
      case 29:
         return ( BLINK_PINK );
      case 30:
         return ( BLINK_LBLUE );
      case 31:
         return ( BLINK_WHITE );

      default:
         return ( ANSI_RESET );
   }
}

int colorcode( const char *col, char *code, CHAR_DATA * ch )
{
   const char *ctype = col;
   int ln;
   bool ansi;

   if( !ch )
      ansi = TRUE;
   else
      ansi = ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_ANSI ) );

   col++;

   if( !*col )
      ln = -1;
#ifdef OVERLANDCODE
   else if( *ctype != '&' && *ctype != '{' && *ctype != '}' )
   {
      bug( "colorcode: command '%c' not '&', '{' or '}'", *ctype );
      ln = -1;
   }
#else
   else if( *ctype != '&' && *ctype != '^' && *ctype != '}' )
   {
      bug( "colorcode: command '%c' not '&', '^' or '}'", *ctype );
      ln = -1;
   }
#endif
   else if( *col == *ctype )
   {
      code[0] = *col;
      code[1] = '\0';
      ln = 1;
   }
   else if( !ansi )
      ln = 0;
   else
   {
      /*
       * Foreground text - non-blinking 
       */
      if( *ctype == '&' )
      {
         switch ( *col )
         {
            default:
               code[0] = *ctype;
               code[1] = *col;
               code[2] = '\0';
               return 2;
            case 'i':  /* Italic text */
            case 'I':
               strncpy( code, ANSI_ITALIC, 20 );
               break;
            case 'v':  /* Reverse colors */
            case 'V':
               strncpy( code, ANSI_REVERSE, 20 );
               break;
            case 'u':  /* Underline */
            case 'U':
               strncpy( code, ANSI_UNDERLINE, 20 );
               break;
            case 's':  /* Strikeover */
            case 'S':
               strncpy( code, ANSI_STRIKEOUT, 20 );
               break;
            case 'd':  /* Player's client default color */
               strncpy( code, ANSI_RESET, 20 );
               break;
            case 'D':  /* Reset to custom color for whatever is being displayed */
               strncpy( code, ANSI_RESET, 20 ); /* Yes, this reset here is quite necessary to cancel out other things */
               if( ch )
                  strncat( code, color_str( ch->desc->pagecolor, ch ), 20 );
               break;
            case 'x':  /* Black */
               strncpy( code, ANSI_BLACK, 20 );
               break;
            case 'O':  /* Orange/Brown */
               strncpy( code, ANSI_ORANGE, 20 );
               break;
            case 'c':  /* Cyan */
               strncpy( code, ANSI_CYAN, 20 );
               break;
            case 'z':  /* Dark Grey */
               strncpy( code, ANSI_DGREY, 20 );
               break;
            case 'g':  /* Dark Green */
               strncpy( code, ANSI_DGREEN, 20 );
               break;
            case 'G':  /* Light Green */
               strncpy( code, ANSI_GREEN, 20 );
               break;
            case 'P':  /* Pink/Light Purple */
               strncpy( code, ANSI_PINK, 20 );
               break;
            case 'r':  /* Dark Red */
               strncpy( code, ANSI_DRED, 20 );
               break;
            case 'b':  /* Dark Blue */
               strncpy( code, ANSI_DBLUE, 20 );
               break;
            case 'w':  /* Grey */
               strncpy( code, ANSI_GREY, 20 );
               break;
            case 'Y':  /* Yellow */
               strncpy( code, ANSI_YELLOW, 20 );
               break;
            case 'C':  /* Light Blue */
               strncpy( code, ANSI_LBLUE, 20 );
               break;
            case 'p':  /* Purple */
               strncpy( code, ANSI_PURPLE, 20 );
               break;
            case 'R':  /* Red */
               strncpy( code, ANSI_RED, 20 );
               break;
            case 'B':  /* Blue */
               strncpy( code, ANSI_BLUE, 20 );
               break;
            case 'W':  /* White */
               strncpy( code, ANSI_WHITE, 20 );
               break;
         }
      }
      /*
       * Foreground text - blinking 
       */
      if( *ctype == '}' )
      {
         switch ( *col )
         {
            default:
               code[0] = *ctype;
               code[1] = *col;
               code[2] = '\0';
               return 2;
            case 'x':  /* Black */
               strncpy( code, BLINK_BLACK, 20 );
               break;
            case 'O':  /* Orange/Brown */
               strncpy( code, BLINK_ORANGE, 20 );
               break;
            case 'c':  /* Cyan */
               strncpy( code, BLINK_CYAN, 20 );
               break;
            case 'z':  /* Dark Grey */
               strncpy( code, BLINK_DGREY, 20 );
               break;
            case 'g':  /* Dark Green */
               strncpy( code, BLINK_DGREEN, 20 );
               break;
            case 'G':  /* Light Green */
               strncpy( code, BLINK_GREEN, 20 );
               break;
            case 'P':  /* Pink/Light Purple */
               strncpy( code, BLINK_PINK, 20 );
               break;
            case 'r':  /* Dark Red */
               strncpy( code, BLINK_DRED, 20 );
               break;
            case 'b':  /* Dark Blue */
               strncpy( code, BLINK_DBLUE, 20 );
               break;
            case 'w':  /* Grey */
               strncpy( code, BLINK_GREY, 20 );
               break;
            case 'Y':  /* Yellow */
               strncpy( code, BLINK_YELLOW, 20 );
               break;
            case 'C':  /* Light Blue */
               strncpy( code, BLINK_LBLUE, 20 );
               break;
            case 'p':  /* Purple */
               strncpy( code, BLINK_PURPLE, 20 );
               break;
            case 'R':  /* Red */
               strncpy( code, BLINK_RED, 20 );
               break;
            case 'B':  /* Blue */
               strncpy( code, BLINK_BLUE, 20 );
               break;
            case 'W':  /* White */
               strncpy( code, BLINK_WHITE, 20 );
               break;
         }
      }
      /*
       * Background color 
       */
#ifdef OVERLANDCODE
      if( *ctype == '{' )
#else
      if( *ctype == '^' )
#endif
      {
         switch ( *col )
         {
            default:
               code[0] = *ctype;
               code[1] = *col;
               code[2] = '\0';
               return 2;
            case 'x':  /* Black */
               strncpy( code, BACK_BLACK, 20 );
               break;
            case 'r':  /* Dark Red */
               strncpy( code, BACK_DRED, 20 );
               break;
            case 'g':  /* Dark Green */
               strncpy( code, BACK_DGREEN, 20 );
               break;
            case 'O':  /* Orange/Brown */
               strncpy( code, BACK_ORANGE, 20 );
               break;
            case 'b':  /* Dark Blue */
               strncpy( code, BACK_DBLUE, 20 );
               break;
            case 'p':  /* Purple */
               strncpy( code, BACK_PURPLE, 20 );
               break;
            case 'c':  /* Cyan */
               strncpy( code, BACK_CYAN, 20 );
               break;
            case 'w':  /* Grey */
               strncpy( code, BACK_GREY, 20 );
               break;
         }
      }
      ln = strlen( code );
   }
   if( ln <= 0 )
      *code = '\0';
   return ln;
}

/* Moved from comm.c */
void set_char_color( short AType, CHAR_DATA * ch )
{
   if( !ch || !ch->desc )
      return;

   write_to_buffer( ch->desc, color_str( AType, ch ), 0 );
   if( !ch->desc )
   {
      bug( "set_char_color: NULL descriptor after WTB! CH: %s", ch->name ? ch->name : "Unknown?!?" );
      return;
   }
   ch->desc->pagecolor = ch->colors[AType];
}

void set_pager_color( short AType, CHAR_DATA * ch )
{
   if( !ch || !ch->desc )
      return;

   write_to_pager( ch->desc, color_str( AType, ch ), 0 );
   if( !ch->desc )
   {
      bug( "set_pager_color: NULL descriptor after WTP! CH: %s", ch->name ? ch->name : "Unknown?!?" );
      return;
   }
   ch->desc->pagecolor = ch->colors[AType];
}

void write_to_pager( DESCRIPTOR_DATA * d, const char *txt, int length )
{
   int pageroffset;  /* Pager fix by thoric */

   if( length <= 0 )
      length = strlen( txt );

   if( length == 0 )
      return;

   if( !d->pagebuf )
   {
      d->pagesize = MAX_STRING_LENGTH;
      CREATE( d->pagebuf, char, d->pagesize );
   }
   if( !d->pagepoint )
   {
      d->pagepoint = d->pagebuf;
      d->pagetop = 0;
      d->pagecmd = '\0';
   }
   if( d->pagetop == 0 && !d->fcommand )
   {
      d->pagebuf[0] = '\n';
      d->pagebuf[1] = '\r';
      d->pagetop = 2;
   }
   pageroffset = d->pagepoint - d->pagebuf;  /* pager fix (goofup fixed 08/21/97) */
   while( d->pagetop + length >= d->pagesize )
   {
      if( d->pagesize > MAX_STRING_LENGTH * 16 )
      {
         bug( "%s", "Pager overflow.  Ignoring.\n\r" );
         d->pagetop = 0;
         d->pagepoint = NULL;
         DISPOSE( d->pagebuf );
         d->pagesize = MAX_STRING_LENGTH;
         return;
      }
      d->pagesize *= 2;
      RECREATE( d->pagebuf, char, d->pagesize );
   }
   d->pagepoint = d->pagebuf + pageroffset;  /* pager fix (goofup fixed 08/21/97) */
   strncpy( d->pagebuf + d->pagetop, txt, length );
   d->pagetop += length;
   d->pagebuf[d->pagetop] = '\0';
   return;
}

/* Writes to a descriptor, usually best used when there's no character to send to ( like logins ) */
void send_to_desc_color( const char *txt, DESCRIPTOR_DATA * d )
{
   char *colstr;
   const char *prevstr = txt;
   char colbuf[20];
   int ln;

   if( !d )
   {
      bug( "%s", "send_to_desc_color: NULL *d" );
      return;
   }

   if( !txt || !d->descriptor )
      return;

#ifdef OVERLANDCODE
   while( ( colstr = strpbrk( prevstr, "&{}" ) ) != NULL )
#else
   while( ( colstr = strpbrk( prevstr, "&^}" ) ) != NULL )
#endif
   {
      if( colstr > prevstr )
         write_to_buffer( d, prevstr, ( colstr - prevstr ) );

      ln = colorcode( colstr, colbuf, d->character );
      if( ln < 0 )
      {
         prevstr = colstr + 1;
         break;
      }

      else if( ln > 0 )
         write_to_buffer( d, colbuf, ln );
      prevstr = colstr + 2;
   }
   if( *prevstr )
      write_to_buffer( d, prevstr, 0 );

   return;
}

/*
 * Write to one char. Convert color into ANSI sequences.
 */
void send_to_char_color( const char *txt, CHAR_DATA * ch )
{
   char *colstr;
   const char *prevstr = txt;
   char colbuf[20];
   int ln;

   if( !ch )
   {
      bug( "%s", "send_to_char_color: NULL ch!" );
      return;
   }

   if( txt && ch->desc )
   {
#ifdef OVERLANDCODE
      while( ( colstr = strpbrk( prevstr, "&{}" ) ) != NULL )
#else
      while( ( colstr = strpbrk( prevstr, "&^}" ) ) != NULL )
#endif
      {
         if( colstr > prevstr )
            write_to_buffer( ch->desc, prevstr, ( colstr - prevstr ) );
         ln = colorcode( colstr, colbuf, ch );
         if( ln < 0 )
         {
            prevstr = colstr + 1;
            break;
         }
         else if( ln > 0 )
            write_to_buffer( ch->desc, colbuf, ln );
         prevstr = colstr + 2;
      }
      if( *prevstr )
         write_to_buffer( ch->desc, prevstr, 0 );
   }
   return;
}

void send_to_pager_color( const char *txt, CHAR_DATA * ch )
{
   char *colstr;
   const char *prevstr = txt;
   char colbuf[20];
   int ln;

   if( !ch )
   {
      bug( "%s", "send_to_pager_color: NULL ch!" );
      return;
   }

   if( txt && ch->desc )
   {
      DESCRIPTOR_DATA *d = ch->desc;

      ch = d->original ? d->original : d->character;
      if( IS_NPC( ch ) || !IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
      {
         send_to_char_color( txt, d->character );
         return;
      }
#ifdef OVERLANDCODE
      while( ( colstr = strpbrk( prevstr, "&{}" ) ) != NULL )
#else
      while( ( colstr = strpbrk( prevstr, "&^}" ) ) != NULL )
#endif
      {
         if( colstr > prevstr )
            write_to_pager( ch->desc, prevstr, ( colstr - prevstr ) );
         ln = colorcode( colstr, colbuf, ch );
         if( ln < 0 )
         {
            prevstr = colstr + 1;
            break;
         }
         else if( ln > 0 )
            write_to_pager( ch->desc, colbuf, ln );
         prevstr = colstr + 2;
      }
      if( *prevstr )
         write_to_pager( ch->desc, prevstr, 0 );
   }
   return;
}

void send_to_char( const char *txt, CHAR_DATA * ch )
{
   send_to_char_color( txt, ch );
   return;
}

void send_to_pager( const char *txt, CHAR_DATA * ch )
{
   send_to_pager_color( txt, ch );
   return;
}

void ch_printf( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_char_color( buf, ch );
}

void pager_printf( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_pager_color( buf, ch );
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
void ch_printf_color( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_char( buf, ch );
}

void pager_printf_color( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_pager( buf, ch );
}
