/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2019 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,                *
 * Xorith, and Adjani.                                                      *
 * All Rights Reserved.                                                     *
 *                                                                          *
 *                                                                          *
 * External contributions from Remcon, Quixadhal, Zarius, and many others.  *
 *                                                                          *
 * Original SMAUG 1.8b written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, Edmond, Conran, and Nivek.                         *
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include "mud.h"

const char *const pc_displays[MAX_COLORS] = {
   "black", "dred", "dgreen", "orange",        // 3
   "dblue", "purple", "cyan", "grey",          // 7
   "dgrey", "red", "green", "yellow",          // 11
   "blue", "pink", "lblue", "white", "blink",  // 16
   "bblack", "bdred", "bdgreen", "bdorange",   // 20
   "bdblue", "bpurple", "bcyan", "bgrey",      // 24
   "bdgrey", "bred", "bgreen", "byellow",      // 28
   "bblue", "bpink", "blblue", "bwhite",       // 32
   "plain", "action", "say", "chat",           // 36
   "yells", "tell", "hit", "hitme",            // 40
   "immortal", "hurt", "falling", "danger",    // 44
   "magic", "consider", "report", "poison",    // 48
   "social", "dying", "dead", "skill",         // 52
   "carnage", "damage", "fleeing", "rmname",   // 56
   "rmdesc", "objects", "people", "list",      // 60
   "bye", "gold", "gtells", "note",            // 64
   "hungry", "thirsty", "fire", "sober",       // 68
   "wearoff", "exits", "score", "reset",       // 72
   "log", "die_msg", "wartalk", "arena",       // 76
   "muse", "think", "aflags", "who",           // 80
   "racetalk", "ignore", "whisper", "divider", // 84
   "morph", "shout", "rflags", "stype",        // 88
   "aname", "auction", "score2", "score3",     // 92
   "score4", "who2", "who3", "who4",           // 96
   "intermud", "helpfiles", "who5", "score5",  // 100
   "who6", "who7", "prac", "prac2",            // 104
   "prac3", "prac4", "mxpprompt", "guildtalk", // 108
   "board", "board2", "board3", "ooc",         // 112
   "avatar", "ship", "clan"                    // 115
};

/* All defaults are set to Alsherok default scheme, if you don't 
like it, change it around to suite your own needs - Samson */
const short default_set[MAX_COLORS] = {
   AT_BLACK, AT_BLOOD, AT_DGREEN, AT_ORANGE, /*  3 */
   AT_DBLUE, AT_PURPLE, AT_CYAN, AT_GREY,   /*  7 */
   AT_DGREY, AT_RED, AT_GREEN, AT_YELLOW,   /* 11 */
   AT_BLUE, AT_PINK, AT_LBLUE, AT_WHITE, AT_BLINK, /* 16 */
   AT_BLACK_BLINK, AT_BLOOD_BLINK, AT_DGREEN_BLINK, AT_ORANGE_BLINK, /* 20 */
   AT_DBLUE_BLINK, AT_PURPLE_BLINK, AT_CYAN_BLINK, AT_GREY_BLINK, /* 24 */
   AT_DGREY_BLINK, AT_RED_BLINK, AT_GREEN_BLINK, AT_YELLOW_BLINK, /* 28 */
   AT_BLUE_BLINK, AT_PINK_BLINK, AT_LBLUE_BLINK, AT_WHITE_BLINK,  /* 32 */
   AT_GREY, AT_GREY, AT_BLUE,             /* 35 */
   AT_GREEN, AT_LBLUE, AT_WHITE, AT_GREY, /* 39 */
   AT_GREY, AT_YELLOW, AT_GREY, AT_GREY,  /* 43 */
   AT_GREY, AT_BLUE, AT_GREY, AT_GREY,    /* 47 */
   AT_DGREEN, AT_CYAN, AT_GREY, AT_GREY,  /* 51 */
   AT_BLUE, AT_GREY, AT_GREY, AT_GREY,    /* 55 */
   AT_RED, AT_GREY, AT_BLUE, AT_PINK,     /* 59 */
   AT_GREY, AT_GREY, AT_YELLOW, AT_GREY,  /* 63 */
   AT_GREY, AT_ORANGE, AT_BLUE, AT_RED,   /* 67 */
   AT_GREY, AT_GREY, AT_GREEN, AT_DGREEN, /* 71 */
   AT_DGREEN, AT_ORANGE, AT_GREY, AT_RED, /* 75 */
   AT_GREY, AT_DGREEN, AT_RED, AT_BLUE,   /* 79 */
   AT_RED, AT_CYAN, AT_YELLOW, AT_PINK,   /* 83 */
   AT_DGREEN, AT_PINK, AT_WHITE, AT_BLUE, /* 87 */
   AT_BLUE, AT_BLUE, AT_GREEN, AT_GREY,   /* 91 */
   AT_GREEN, AT_GREEN, AT_YELLOW, AT_DGREY,  /* 95 */
   AT_GREEN, AT_PINK, AT_DGREEN, AT_CYAN, /* 99 */
   AT_RED, AT_WHITE, AT_BLUE, AT_DGREEN,  /* 103 */
   AT_CYAN, AT_BLOOD, AT_RED, AT_DGREEN,  /* 107 */
   AT_GREEN, AT_GREY, AT_GREEN, AT_WHITE, /* 111 */
   AT_WHITE, AT_YELLOW, AT_LBLUE, AT_DGREY /* 115 */
};

const char *const valid_color[] = {
   "black", "dred", "dgreen", "orange", "dblue", "purple", "cyan", "grey",
   "dgrey", "red", "green", "yellow", "blue", "pink", "lblue", "white", "\0"
};

void show_colorthemes( CHAR_DATA * ch )
{
   DIR *dp;
   struct dirent *dentry;
   int count = 0, col = 0;

   send_to_pager( "&YThe following themes are available:\r\n", ch );

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
         ++count;
         pager_printf( ch, "%s%-15.15s", color_str( AT_PLAIN, ch ), dentry->d_name );
         if( ++col % 6 == 0 )
            send_to_pager( "\r\n", ch );
      }
      dentry = readdir( dp );
   }
   closedir( dp );

   if( count == 0 )
      send_to_pager( "No themes defined yet.\r\n", ch );

   if( col % 6 != 0 )
      send_to_pager( "\r\n", ch );
}

void show_colors( CHAR_DATA * ch )
{
   short count;

   send_to_pager( "&BSyntax: color [color type] [color] | default\r\n", ch );
   send_to_pager( "&BSyntax: color _reset_ (Resets all colors to default set)\r\n", ch );
   send_to_pager( "&BSyntax: color _all_ [color] (Sets all color types to [color])\r\n\r\n", ch );
   send_to_pager( "&BSyntax: color theme [name] (Sets all color types to a defined theme)\r\n\r\n", ch );

   send_to_pager( "&W********************************[ COLORS ]*********************************\r\n", ch );

   for( count = 0; count < 16; ++count )
   {
      if( ( count % 8 ) == 0 && count != 0 )
      {
         send_to_pager( "\r\n", ch );
      }
      pager_printf( ch, "%s%-10s", color_str( count, ch ), pc_displays[count] );
   }

   send_to_pager( "\r\n\r\n&W******************************[ COLOR TYPES ]******************************\r\n", ch );

   for( count = 33; count < MAX_COLORS; ++count )
   {
      if( ( count % 8 ) == 0 && count != 33 )
      {
         send_to_pager( "\r\n", ch );
      }
      pager_printf( ch, "%s%-10s%s", color_str( count, ch ), pc_displays[count], ANSI_RESET );
   }
   send_to_pager( "\r\n\r\n", ch );
   send_to_pager( "&YAvailable colors are:\r\n", ch );

   for( count = 0; valid_color[count][0] != '\0'; ++count )
   {
      if( ( count % 8 ) == 0 && count != 0 )
         send_to_pager( "\r\n", ch );

      pager_printf( ch, "%s%-10s", color_str( AT_PLAIN, ch ), valid_color[count] );
   }
   send_to_pager( "\r\n", ch );
   show_colorthemes( ch );
}

void reset_colors( CHAR_DATA * ch )
{
   int x;

   if( !IS_NPC( ch ) )
   {
      char filename[256];
      FILE *fp;
      int max_colors = 0;

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
            int temp = fread_number( fp );
            max_colors = UMIN( temp, MAX_COLORS );
            continue;
         }

         if( !str_cmp( word, "Colors" ) )
         {
            for( x = 0; x < max_colors; ++x )
               ch->colors[x] = fread_number( fp );
            fread_to_eol( fp );
            continue;
         }

         if( !str_cmp( word, "End" ) )
         {
            FCLOSE( fp );
            return;
         }
      }
      FCLOSE( fp );
      return;
   }
   else
      log_printf( "%s: Attempting to reset NPC colors: %s", __func__, ch->short_descr );
}

void do_color( CHAR_DATA* ch, const char* argument )
{
   bool dMatch, cMatch;
   short count = 0, y = 0;
   int x;
   char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

   dMatch = FALSE;
   cMatch = FALSE;

   if( IS_NPC( ch ) )
   {
      send_to_pager( "Only PC's can change colors.\r\n", ch );
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

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "You must specify a name for this theme to save it.\r\n", ch );
         return;
      }

      if( strstr( argument, "." ) || strstr( argument, "/" ) || strstr( argument, "\\" ) )
      {
         send_to_char( "Invalid theme name.\r\n", ch );
         return;
      }

      snprintf( filename, sizeof( filename ), "%s%s", COLOR_DIR, argument );
      if( !( fp = fopen( filename, "w" ) ) )
      {
         ch_printf( ch, "Unable to write to color file %s\r\n", filename );
         return;
      }

      fprintf( fp, "%s", "#COLORTHEME\n" );
      fprintf( fp, "Name         %s~\n", argument );
      fprintf( fp, "MaxColors    %d\n", MAX_COLORS );
      fprintf( fp, "%s", "Colors      " );
      for( x = 0; x < MAX_COLORS; ++x )
         fprintf( fp, " %d", ch->colors[x] );
      fprintf( fp, "%s", "\nEnd\n" );
      FCLOSE( fp );
      ch_printf( ch, "Color theme %s saved.\r\n", argument );
      return;
   }

   if( !str_cmp( arg, "theme" ) )
   {
      FILE *fp;
      char filename[256];
      int max_colors = 0;

      if( !argument || argument[0] == '\0' )
      {
         show_colorthemes( ch );
         return;
      }

      if( strstr( argument, "." ) || strstr( argument, "/" ) || strstr( argument, "\\" ) )
      {
         send_to_char( "Invalid theme.\r\n", ch );
         return;
      }

      snprintf( filename, sizeof( filename ), "%s%s", COLOR_DIR, argument );
      if( !( fp = fopen( filename, "r" ) ) )
      {
         ch_printf( ch, "There is no theme called %s.\r\n", argument );
         return;
      }

      while( !feof( fp ) )
      {
         char *word = fread_word( fp );
         if( !str_cmp( word, "MaxColors" ) )
         {
            int temp = fread_number( fp );
            max_colors = UMIN( temp, MAX_COLORS );
            continue;
         }

         if( !str_cmp( word, "Colors" ) )
         {
            for( x = 0; x < max_colors; ++x )
               ch->colors[x] = fread_number( fp );
            fread_to_eol( fp );
            continue;
         }

         if( !str_cmp( word, "End" ) )
         {
            FCLOSE( fp );
            ch_printf( ch, "Color theme has been changed to %s.\r\n", argument );
            save_char_obj( ch );
            return;
         }
      }
      FCLOSE( fp );
      ch_printf( ch, "An error occured while trying to set color theme %s.\r\n", argument );
      return;
   }

   if( !str_cmp( arg, "ansitest" ) )
   {
      buffer_printf( ch->desc, "%sBlack\r\n", ANSI_BLACK );
      buffer_printf( ch->desc, "%sDark Red\r\n", ANSI_DRED );
      buffer_printf( ch->desc, "%sDark Green\r\n", ANSI_DGREEN );
      buffer_printf( ch->desc, "%sOrange/Brown\r\n", ANSI_ORANGE );
      buffer_printf( ch->desc, "%sDark Blue\r\n", ANSI_DBLUE );
      buffer_printf( ch->desc, "%sPurple\r\n", ANSI_PURPLE );
      buffer_printf( ch->desc, "%sCyan\r\n", ANSI_CYAN );
      buffer_printf( ch->desc, "%sGrey\r\n", ANSI_GREY );
      buffer_printf( ch->desc, "%sDark Grey\r\n", ANSI_DGREY );
      buffer_printf( ch->desc, "%sRed\r\n", ANSI_RED );
      buffer_printf( ch->desc, "%sGreen\r\n", ANSI_GREEN );
      buffer_printf( ch->desc, "%sYellow\r\n", ANSI_YELLOW );
      buffer_printf( ch->desc, "%sBlue\r\n", ANSI_BLUE );
      buffer_printf( ch->desc, "%sPink\r\n", ANSI_PINK );
      buffer_printf( ch->desc, "%sLight Blue\r\n", ANSI_LBLUE );
      buffer_printf( ch->desc, "%sWhite\r\n", ANSI_WHITE );
      buffer_printf( ch->desc, "%sBlack\r\n", BLINK_BLACK );
      buffer_printf( ch->desc, "%sDark Red\r\n", BLINK_DRED );
      buffer_printf( ch->desc, "%sDark Green\r\n", BLINK_DGREEN );
      buffer_printf( ch->desc, "%sOrange/Brown\r\n", BLINK_ORANGE );
      buffer_printf( ch->desc, "%sDark Blue\r\n", BLINK_DBLUE );
      buffer_printf( ch->desc, "%sPurple\r\n", BLINK_PURPLE );
      buffer_printf( ch->desc, "%sCyan\r\n", BLINK_CYAN );
      buffer_printf( ch->desc, "%sGrey\r\n", BLINK_GREY );
      buffer_printf( ch->desc, "%sDark Grey\r\n", BLINK_DGREY );
      buffer_printf( ch->desc, "%sRed\r\n", BLINK_RED );
      buffer_printf( ch->desc, "%sGreen\r\n", BLINK_GREEN );
      buffer_printf( ch->desc, "%sYellow\r\n", BLINK_YELLOW );
      buffer_printf( ch->desc, "%sBlue\r\n", BLINK_BLUE );
      buffer_printf( ch->desc, "%sPink\r\n", BLINK_PINK );
      buffer_printf( ch->desc, "%sLight Blue\r\n", BLINK_LBLUE );
      buffer_printf( ch->desc, "%sWhite\r\n", BLINK_WHITE );
      write_to_buffer( ch->desc, ANSI_RESET, 0 );

      buffer_printf( ch->desc, "%s%sBlack\r\n", ANSI_WHITE, BACK_BLACK );
      buffer_printf( ch->desc, "%s%sDark Red\r\n", ANSI_BLACK, BACK_DRED );
      buffer_printf( ch->desc, "%s%sDark Green\r\n", ANSI_BLACK, BACK_DGREEN );
      buffer_printf( ch->desc, "%s%sOrange/Brown\r\n", ANSI_BLACK, BACK_ORANGE );
      buffer_printf( ch->desc, "%s%sDark Blue\r\n", ANSI_BLACK, BACK_DBLUE );
      buffer_printf( ch->desc, "%s%sPurple\r\n", ANSI_BLACK, BACK_PURPLE );
      buffer_printf( ch->desc, "%s%sCyan\r\n", ANSI_BLACK, BACK_CYAN );
      buffer_printf( ch->desc, "%s%sGrey\r\n", ANSI_BLACK, BACK_GREY );
      buffer_printf( ch->desc, "%s%sDark Grey Background\r\n", ANSI_BLACK, BACK_DGREY );
      buffer_printf( ch->desc, "%s%sRed Background\r\n", ANSI_BLACK, BACK_RED );
      buffer_printf( ch->desc, "%s%sGreen Background\r\n", ANSI_BLACK, BACK_GREEN );
      buffer_printf( ch->desc, "%s%sYellow Background\r\n", ANSI_BLACK, BACK_YELLOW );
      buffer_printf( ch->desc, "%s%sBlue Background\r\n", ANSI_BLACK, BACK_BLUE );
      buffer_printf( ch->desc, "%s%sPink Background\r\n", ANSI_BLACK, BACK_PINK );
      buffer_printf( ch->desc, "%s%sLight Blue Background\r\n", ANSI_BLACK, BACK_LBLUE );
      buffer_printf( ch->desc, "%s%sWhite Background\r\n", ANSI_BLACK, BACK_WHITE );
      buffer_printf( ch->desc, "%s%sItalics%s\r\n", ANSI_GREY, ANSI_ITALIC, ANSI_RESET );
      buffer_printf( ch->desc, "%sStrikeout%s\r\n", ANSI_STRIKEOUT, ANSI_RESET );
      buffer_printf( ch->desc, "%sUnderline\r\n", ANSI_UNDERLINE );
      write_to_buffer( ch->desc, ANSI_RESET, 0 );
      return;
   }

   if( !str_prefix( arg, "_reset_" ) )
   {
      reset_colors( ch );
      send_to_pager( "All color types reset to default colors.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg2 );

   if( arg[0] == '\0' )
   {
      send_to_char( "Change which color type?\r\n", ch );
      return;
   }

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
      cMatch = FALSE;
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
         ch_printf( ch, "%s is an invalid color type.\r\n", arg );
         send_to_char( "Type color with no arguments to see available options.\r\n", ch );
         return;
      }

      if( !str_cmp( arg2, "default" ) )
      {
         ch->colors[count] = default_set[count];
         ch_printf( ch, "Display %s set back to default.\r\n", pc_displays[count] );
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
         ch_printf( ch, "Invalid color for type %s.\n", arg );
      else
         send_to_pager( "Invalid color.\r\n", ch );

      send_to_pager( "Choices are:\r\n", ch );

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
      pager_printf( ch, "Color type %s set to color %s.\r\n", count == -1 ? "_all_" : pc_displays[count], valid_color[y] );

   if( !str_cmp( argument, "blink" ) )
      y += AT_BLINK;

   if( count == -1 )
   {
      int ccount;

      for( ccount = 0; ccount < MAX_COLORS; ++ccount )
         ch->colors[ccount] = y;

      set_pager_color( y, ch );

      pager_printf( ch, "All color types set to color %s%s.%s\r\n",
                    valid_color[y > AT_BLINK ? y - AT_BLINK : y], y > AT_BLINK ? " [BLINKING]" : "", ANSI_RESET );
   }
   else
   {
      ch->colors[count] = y;

      set_pager_color( count, ch );

      if( !str_cmp( argument, "blink" ) )
         ch_printf( ch, "Display %s set to color %s [BLINKING]%s\r\n",
                    pc_displays[count], valid_color[y - AT_BLINK], ANSI_RESET );
      else
         ch_printf( ch, "Display %s set to color %s.\r\n", pc_displays[count], valid_color[y] );
   }
}

const char *color_str( short AType, CHAR_DATA * ch )
{
   if( !ch )
   {
      bug( "%s: NULL ch!", __func__ );
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
          * 17 thru 32 are for blinking colors 
          */
      case 17:
         return ( BLINK_BLACK );
      case 18:
         return ( BLINK_DRED );
      case 19:
         return ( BLINK_DGREEN );
      case 20:
         return ( BLINK_ORANGE );
      case 21:
         return ( BLINK_DBLUE );
      case 22:
         return ( BLINK_PURPLE );
      case 23:
         return ( BLINK_CYAN );
      case 24:
         return ( BLINK_GREY );
      case 25:
         return ( BLINK_DGREY );
      case 26:
         return ( BLINK_RED );
      case 27:
         return ( BLINK_GREEN );
      case 28:
         return ( BLINK_YELLOW );
      case 29:
         return ( BLINK_BLUE );
      case 30:
         return ( BLINK_PINK );
      case 31:
         return ( BLINK_LBLUE );
      case 32:
         return ( BLINK_WHITE );

      default:
         return ( ANSI_RESET );
   }
}

/* Random Ansi Color Code -- Xorith */
const char *random_ansi( short type )
{
   switch ( type )
   {
      default:
      case 1: /* Default ANSI Fore-ground */
         switch ( number_range( 1, 15 ) )
         {
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
            default:
               return ( ANSI_RESET );
         }

      case 2: /* ANSI Blinking */
         switch ( number_range( 1, 14 ) )
         {
            case 1:
               return ( BLINK_DGREEN );
            case 2:
               return ( BLINK_ORANGE );
            case 3:
               return ( BLINK_DBLUE );
            case 4:
               return ( BLINK_PURPLE );
            case 5:
               return ( BLINK_CYAN );
            case 6:
               return ( BLINK_GREY );
            case 7:
               return ( BLINK_DGREY );
            case 8:
               return ( BLINK_RED );
            case 9:
               return ( BLINK_GREEN );
            case 10:
               return ( BLINK_YELLOW );
            case 11:
               return ( BLINK_BLUE );
            case 12:
               return ( BLINK_PINK );
            case 13:
               return ( BLINK_LBLUE );
            default:
            case 14:
               return ( BLINK_WHITE );
         }

      case 3: /* ANSI Background */
         switch ( number_range( 1, 15 ) )
         {
            case 1:
               return ( BACK_DRED );
            case 2:
               return ( BACK_DGREEN );
            case 3:
               return ( BACK_ORANGE );
            case 4:
               return ( BACK_DBLUE );
            case 5:
               return ( BACK_PURPLE );
            case 6:
               return ( BACK_CYAN );
            case 7:
               return ( BACK_GREY );
            case 8:
               return ( BACK_DGREY );
            case 9:
               return ( BACK_RED );
            case 10:
               return ( BACK_GREEN );
            case 11:
               return ( BACK_YELLOW );
            case 12:
               return ( BACK_BLUE );
            case 13:
               return ( BACK_PINK );
            case 14:
               return ( BACK_LBLUE );
            default:
            case 15:
               return ( BACK_WHITE );
         }
   }
}

/*
 * Quixadhal - I rewrote this from scratch.  It now returns the number of
 * characters in the SOURCE string that should be skipped, it always fills
 * the DESTINATION string with a valid translation (even if that is itself,
 * or an empty string), and the default for ANSI is FALSE, since mobs and
 * logfiles shouldn't need colour.
 *
 * NOTE:  dstlen is the length of your pre-allocated buffer that you passed
 * in.  It must be at least 3 bytes, but should be long enough to hold the
 * longest translation sequence (probably around 16-32).
 *
 * NOTE:  vislen is the "visible" length of the translation token.  This is
 * used by color_strlen to properly figure the visual length of a string.
 * If you need the number of bytes (such as for output buffering), use the
 * normal strlen function.
 */
int colorcode( const char *src, char *dst, DESCRIPTOR_DATA * d, int dstlen, int *vislen )
{
   CHAR_DATA *ch = NULL;
   bool ansi = FALSE;
   const char *sympos = NULL;

   /*
    * No descriptor, assume ANSI conversion can't be done. 
    */
   if( !d )
      ansi = FALSE;
   /*
    * But, if we have one, check for a PC and set accordingly. If no PC, assume ANSI can be done. For color logins. 
    */
   else
   {
      ch = d->original ? d->original : d->character;

      if( ch )
         ansi = ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_ANSI ) );
      else
         ansi = TRUE;
   }

   if( !dst )
      return 0;   /* HEY, I said at least 3 BYTES! */

   dst[0] = '\0'; /* Initialize the the default NOTHING */

   /*
    * Move along, nothing to see here 
    */
   if( !src || !*src )
      return 0;

   switch ( *src )
   {
      case '&':  /* NORMAL, Foreground colour */
         switch ( src[1] )
         {
            case '&':  /* Escaped self, return one of us */
               dst[0] = src[0];
               dst[1] = '\0';
               if( vislen )
                  *vislen = 1;
               return 2;

            case 'Z':  /* Random Ansi Foreground */
               if( ansi )
                  strlcpy( dst, random_ansi( 1 ), dstlen );
               break;

            case '[':  /* Symbolic color name */
               if( ( sympos = strchr( src + 2, ']' ) ) )
               {
                  int subcnt = 0;
                  unsigned int sublen = 0;

                  sublen = sympos - src - 2;
                  for( subcnt = 0; subcnt < MAX_COLORS; subcnt++ )
                  {
                     if( !strncmp( src + 2, pc_displays[subcnt], sublen ) )
                     {
                        if( strlen( pc_displays[subcnt] ) == sublen )
                        {
                           /*
                            * These can only be used with a logged in char 
                            */
                           if( ansi && ch )
                              strlcpy( dst, color_str( subcnt, ch ), dstlen );
                           if( vislen )
                              *vislen = 0;
                           return sublen + 3;
                        }
                     }
                  }
               }  /* found matching ] */

               /*
                * Unknown symbolic name, return just the sequence  
                */
               dst[0] = src[0];
               dst[1] = src[1];
               dst[2] = '\0';
               if( vislen )
                  *vislen = 2;
               return 2;

            case 'i':  /* Italic text */
            case 'I':
               if( ansi )
                  strlcpy( dst, ANSI_ITALIC, dstlen );
               break;

            case 'v':  /* Reverse colors */
            case 'V':
               if( ansi )
                  strlcpy( dst, ANSI_REVERSE, dstlen );
               break;

            case 'u':  /* Underline */
            case 'U':
               if( ansi )
                  strlcpy( dst, ANSI_UNDERLINE, dstlen );
               break;

            case 's':  /* Strikeover */
            case 'S':
               if( ansi )
                  strlcpy( dst, ANSI_STRIKEOUT, dstlen );
               break;

            case 'd':  /* Player's client default color */
               if( ansi )
                  strlcpy( dst, ANSI_RESET, dstlen );
               break;

            case 'D':  /* Reset to custom color for whatever is being displayed */
               if( ansi )
               {
                  /*
                   * Yes, this reset here is quite necessary to cancel out other things 
                   */
                  strlcpy( dst, ANSI_RESET, dstlen );
                  if( ch && ch->desc )
                     strlcat( dst, color_str( ch->desc->pagecolor, ch ), dstlen );
               }
               break;

            case 'x':  /* Black */
               if( ansi )
                  strlcpy( dst, ANSI_BLACK, dstlen );
               break;

            case 'O':  /* Orange/Brown */
               if( ansi )
                  strlcpy( dst, ANSI_ORANGE, dstlen );
               break;

            case 'c':  /* Cyan */
               if( ansi )
                  strlcpy( dst, ANSI_CYAN, dstlen );
               break;

            case 'z':  /* Dark Grey */
               if( ansi )
                  strlcpy( dst, ANSI_DGREY, dstlen );
               break;

            case 'g':  /* Dark Green */
               if( ansi )
                  strlcpy( dst, ANSI_DGREEN, dstlen );
               break;

            case 'G':  /* Light Green */
               if( ansi )
                  strlcpy( dst, ANSI_GREEN, dstlen );
               break;

            case 'P':  /* Pink/Light Purple */
               if( ansi )
                  strlcpy( dst, ANSI_PINK, dstlen );
               break;

            case 'r':  /* Dark Red */
               if( ansi )
                  strlcpy( dst, ANSI_DRED, dstlen );
               break;

            case 'b':  /* Dark Blue */
               if( ansi )
                  strlcpy( dst, ANSI_DBLUE, dstlen );
               break;

            case 'w':  /* Grey */
               if( ansi )
                  strlcpy( dst, ANSI_GREY, dstlen );
               break;

            case 'Y':  /* Yellow */
               if( ansi )
                  strlcpy( dst, ANSI_YELLOW, dstlen );
               break;

            case 'C':  /* Light Blue */
               if( ansi )
                  strlcpy( dst, ANSI_LBLUE, dstlen );
               break;

            case 'p':  /* Purple */
               if( ansi )
                  strlcpy( dst, ANSI_PURPLE, dstlen );
               break;

            case 'R':  /* Red */
               if( ansi )
                  strlcpy( dst, ANSI_RED, dstlen );
               break;

            case 'B':  /* Blue */
               if( ansi )
                  strlcpy( dst, ANSI_BLUE, dstlen );
               break;

            case 'W':  /* White */
               if( ansi )
                  strlcpy( dst, ANSI_WHITE, dstlen );
               break;

            default:   /* Unknown sequence, return all the chars */
               dst[0] = src[0];
               dst[1] = src[1];
               dst[2] = '\0';
               if( vislen )
                  *vislen = 2;
               return 2;
         }
         break;

      case '^':  /* BACKGROUND colour */
         switch ( src[1] )
         {
            case '^':  /* Escaped self, return one of us */
               dst[0] = src[0];
               dst[1] = '\0';
               if( vislen )
                  *vislen = 1;
               return 2;

            case 'Z':  /* Random Ansi Background */
               if( ansi )
                  strlcpy( dst, random_ansi( 3 ), dstlen );
               break;

            case 'x':  /* Black */
               if( ansi )
                  strlcpy( dst, BACK_BLACK, dstlen );
               break;

            case 'r':  /* Dark Red */
               if( ansi )
                  strlcpy( dst, BACK_DRED, dstlen );
               break;

            case 'g':  /* Dark Green */
               if( ansi )
                  strlcpy( dst, BACK_DGREEN, dstlen );
               break;

            case 'O':  /* Orange/Brown */
               if( ansi )
                  strlcpy( dst, BACK_ORANGE, dstlen );
               break;

            case 'b':  /* Dark Blue */
               if( ansi )
                  strlcpy( dst, BACK_DBLUE, dstlen );
               break;

            case 'p':  /* Purple */
               if( ansi )
                  strlcpy( dst, BACK_PURPLE, dstlen );
               break;

            case 'c':  /* Cyan */
               if( ansi )
                  strlcpy( dst, BACK_CYAN, dstlen );
               break;

            case 'w':  /* Grey */
               if( ansi )
                  strlcpy( dst, BACK_GREY, dstlen );
               break;

            case 'z':  /* Dark Grey */
               if( ansi )
                  strlcpy( dst, BACK_DGREY, dstlen );
               break;

            case 'R':  /* Red */
               if( ansi )
                  strlcpy( dst, BACK_RED, dstlen );
               break;

            case 'G':  /* Green */
               if( ansi )
                  strlcpy( dst, BACK_GREEN, dstlen );
               break;

            case 'Y':  /* Yellow */
               if( ansi )
                  strlcpy( dst, BACK_YELLOW, dstlen );
               break;

            case 'B':  /* Blue */
               if( ansi )
                  strlcpy( dst, BACK_BLUE, dstlen );
               break;

            case 'P':  /* Pink */
               if( ansi )
                  strlcpy( dst, BACK_PINK, dstlen );
               break;

            case 'C':  /* Light Blue */
               if( ansi )
                  strlcpy( dst, BACK_LBLUE, dstlen );
               break;

            case 'W':  /* White */
               if( ansi )
                  strlcpy( dst, BACK_WHITE, dstlen );
               break;

            default:   /* Unknown sequence, return all the chars */
               dst[0] = src[0];
               dst[1] = src[1];
               dst[2] = '\0';
               if( vislen )
                  *vislen = 2;
               return 2;
         }
         break;

      case '}':  /* BLINK Foreground colour */
         switch ( src[1] )
         {
            case '}':  /* Escaped self, return one of us */
               dst[0] = src[0];
               dst[1] = '\0';
               if( vislen )
                  *vislen = 1;
               return 2;

            case 'Z':  /* Random Ansi Blink */
               if( ansi )
                  strlcpy( dst, random_ansi( 2 ), dstlen );
               break;

            case 'x':  /* Black */
               if( ansi )
                  strlcpy( dst, BLINK_BLACK, dstlen );
               break;

            case 'O':  /* Orange/Brown */
               if( ansi )
                  strlcpy( dst, BLINK_ORANGE, dstlen );
               break;

            case 'c':  /* Cyan */
               if( ansi )
                  strlcpy( dst, BLINK_CYAN, dstlen );
               break;

            case 'z':  /* Dark Grey */
               if( ansi )
                  strlcpy( dst, BLINK_DGREY, dstlen );
               break;

            case 'g':  /* Dark Green */
               if( ansi )
                  strlcpy( dst, BLINK_DGREEN, dstlen );
               break;

            case 'G':  /* Light Green */
               if( ansi )
                  strlcpy( dst, BLINK_GREEN, dstlen );
               break;

            case 'P':  /* Pink/Light Purple */
               if( ansi )
                  strlcpy( dst, BLINK_PINK, dstlen );
               break;

            case 'r':  /* Dark Red */
               if( ansi )
                  strlcpy( dst, BLINK_DRED, dstlen );
               break;

            case 'b':  /* Dark Blue */
               if( ansi )
                  strlcpy( dst, BLINK_DBLUE, dstlen );
               break;

            case 'w':  /* Grey */
               if( ansi )
                  strlcpy( dst, BLINK_GREY, dstlen );
               break;

            case 'Y':  /* Yellow */
               if( ansi )
                  strlcpy( dst, BLINK_YELLOW, dstlen );
               break;

            case 'C':  /* Light Blue */
               if( ansi )
                  strlcpy( dst, BLINK_LBLUE, dstlen );
               break;

            case 'p':  /* Purple */
               if( ansi )
                  strlcpy( dst, BLINK_PURPLE, dstlen );
               break;

            case 'R':  /* Red */
               if( ansi )
                  strlcpy( dst, BLINK_RED, dstlen );
               break;

            case 'B':  /* Blue */
               if( ansi )
                  strlcpy( dst, BLINK_BLUE, dstlen );
               break;

            case 'W':  /* White */
               if( ansi )
                  strlcpy( dst, BLINK_WHITE, dstlen );
               break;

            default:   /* Unknown sequence, return all the chars */
               dst[0] = src[0];
               dst[1] = src[1];
               dst[2] = '\0';
               if( vislen )
                  *vislen = 2;
               return 2;
         }
         break;

      default:   /* Just a normal character */
         dst[0] = *src;
         dst[1] = '\0';
         if( vislen )
            *vislen = 1;
         return 1;
   }
   if( vislen )
      *vislen = 0;
   return 2;
}

/*
 * Quixadhal - I rewrote this too, so that it uses colorcode.  It may not
 * be as efficient as just walking over the string and counting, but it
 * keeps us from duplicating the code several times.
 *
 * This function returns the intended screen length of a string which has
 * color codes embedded in it.  It does this by stripping the codes out
 * entirely (A NULL descriptor means ANSI will be FALSE).
 */
int color_strlen( const char *src )
{
   unsigned int i = 0;
   int len = 0;

   if( !src || !*src )  /* Move along, nothing to see here */
      return 0;

   for( i = 0; i < strlen( src ); )
   {
      char dst[20];
      int vislen;

      switch ( src[i] )
      {
         case '&':  /* NORMAL, Foreground colour */
         case '^':  /* BACKGROUND colour */
         case '}':  /* BLINK Foreground colour */
            *dst = '\0';
            vislen = 0;
            i += colorcode( &src[i], dst, NULL, 20, &vislen ); /* Skip input token */
            len += vislen; /* Count output token length */
            break;   /* this was missing - if you have issues, remove it */

         default:   /* No conversion, just count */
            ++len;
            ++i;
            break;
      }
   }
   return len;
}

/*
 * Quixadhal - And this one needs to use the new version too.
 */
char *color_align( const char *argument, int size, int align )
{
   int space = 0;
   int len = 0;
   static char buf[MAX_STRING_LENGTH];

   len = color_strlen( argument );
   space = ( size - len );
   if( align == ALIGN_RIGHT || len >= size )
      snprintf( buf, MAX_STRING_LENGTH, "%*.*s", len, len, argument );
   else if( align == ALIGN_CENTER )
      snprintf( buf, MAX_STRING_LENGTH, "%*s%s%*s", ( space / 2 ), "", argument,
                ( ( space / 2 ) * 2 ) == space ? ( space / 2 ) : ( ( space / 2 ) + 1 ), "" );
   else if( align == ALIGN_LEFT )
      snprintf( buf, MAX_STRING_LENGTH, "%s%*s", argument, space, "" );

   return buf;
}

/*
 * Quixadhal - This takes a string and converts any and all color tokens
 * in it to the desired output tokens, using the provided character's
 * preferences.
 */
char *colorize( const char *txt, DESCRIPTOR_DATA * d )
{
   static char result[MAX_STRING_LENGTH];

   *result = '\0';

   if( txt && *txt && d )
   {
      const char *colstr;
      const char *prevstr = txt;
      char colbuf[20];
      int ln;

      while( ( colstr = strpbrk( prevstr, "&^}hH" ) ) != NULL )
      {
         int reslen = 0;

         if( colstr > prevstr )
         {
            if( ( MAX_STRING_LENGTH - ( reslen = strlen( result ) ) ) <= ( colstr - prevstr ) )
            {
               bug( "%s: OVERFLOW in internal MAX_STRING_LENGTH buffer!", __func__ );
               break;
            }
            strncat( result, prevstr, ( colstr - prevstr ) );  /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
            result[reslen + ( colstr - prevstr )] = '\0';   /* strncat will NOT NULL terminate this! */
         }

         if( colstr[0] == 'h' || colstr[0] == 'H' )
            if( colstr[1] == 't' || colstr[1] == 'T' )
               if( colstr[2] == 't' || colstr[2] == 'T' )
                  if( colstr[3] == 'p' || colstr[3] == 'P' )
                  {
                     char http[MAX_INPUT_LENGTH];

                     one_argument( colstr, http );
                     strlcat( result, http, sizeof( result ) );
                     ln = strlen( http );
                     prevstr = colstr + ln;
                     continue;
                  }

         ln = colorcode( colstr, colbuf, d, 20, NULL );
         if( ln > 0 )
         {
            strlcat( result, colbuf, MAX_STRING_LENGTH );
            prevstr = colstr + ln;
         }
         else
            prevstr = colstr + 1;
      }
      if( *prevstr )
         strlcat( result, prevstr, MAX_STRING_LENGTH );
   }
   return result;
}

/* Moved from comm.c */
void set_char_color( short AType, CHAR_DATA * ch )
{
   if( !ch || !ch->desc )
      return;

   if( IS_NPC( ch ) )
      return;

   write_to_buffer( ch->desc, color_str( AType, ch ), 0 );
   if( !ch->desc )
   {
      bug( "%s: NULL descriptor after WTB! CH: %s", __func__, ch->name ? ch->name : "Unknown?!?" );
      return;
   }
   ch->desc->pagecolor = ch->colors[AType];
}

void write_to_pager( DESCRIPTOR_DATA * d, const char *txt, size_t length )
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
      d->pagebuf[0] = '\r';
      d->pagebuf[1] = '\n';
      d->pagetop = 2;
   }

   pageroffset = d->pagepoint - d->pagebuf;  /* pager fix (goofup fixed 08/21/97) */
   while( d->pagetop + length >= d->pagesize )
   {
      if( d->pagesize > MAX_STRING_LENGTH * 24 )
      {
         d->pagetop = 0;
         d->pagepoint = NULL;
         DISPOSE( d->pagebuf );
         d->pagesize = MAX_STRING_LENGTH;
         // Move bug call here to avoid infinite loops.  Compliments of Daltorak -- Alty
         bug( "%s: Pager overflow (%s).  Ignoring.", __func__, d->character ? d->character->name : "???" );
         return;
      }

      if( d->pagesize < 8 * MAX_STRING_LENGTH )
         d->pagesize *= 2;
      else
         d->pagesize = 24 * MAX_STRING_LENGTH;
      RECREATE( d->pagebuf, char, d->pagesize );
   }

   d->pagepoint = d->pagebuf + pageroffset;  /* pager fix (goofup fixed 08/21/97) */
   strncpy( d->pagebuf + d->pagetop, txt, length );   /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
   d->pagetop += length;
   d->pagebuf[d->pagetop] = '\0';
}

void set_pager_color( short AType, CHAR_DATA * ch )
{
   if( !ch || !ch->desc )
      return;

   if( IS_NPC( ch ) )
      return;

   write_to_pager( ch->desc, color_str( AType, ch ), 0 );
   if( !ch->desc )
   {
      bug( "%s: NULL descriptor after WTP! CH: %s", __func__, ch->name ? ch->name : "Unknown?!?" );
      return;
   }
   ch->desc->pagecolor = ch->colors[AType];
}

/* Writes to a descriptor, usually best used when there's no character to send to ( like logins ) */
void send_to_desc_color( const char *txt, DESCRIPTOR_DATA * d )
{
   if( !d )
   {
      bug( "%s: NULL *d", __func__ );
      return;
   }

   if( !txt || !d->descriptor )
      return;

   write_to_buffer( d, colorize( txt, d ), 0 );
}

/*
 * Write to one char. Convert color into ANSI sequences.
 */
void send_to_char( const char *txt, CHAR_DATA * ch )
{
   if( !ch )
   {
      bug( "%s: NULL ch!", __func__ );
      return;
   }

   if( txt && ch->desc )
      send_to_desc_color( txt, ch->desc );
}

void send_to_pager( const char *txt, CHAR_DATA * ch )
{
   if( !ch )
   {
      bug( "%s: NULL ch!", __func__ );
      return;
   }

   if( txt && ch->desc )
   {
      DESCRIPTOR_DATA *d = ch->desc;

      ch = d->original ? d->original : d->character;
      if( IS_NPC( ch ) || !IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
      {
         if( ch->desc )
            send_to_desc_color( txt, ch->desc );
      }
      else
      {
         if( ch->desc )
            write_to_pager( ch->desc, colorize( txt, ch->desc ), 0 );
      }
   }
}

void ch_printf( CHAR_DATA * ch, const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_char( buf, ch );
}

void pager_printf( CHAR_DATA * ch, const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_pager( buf, ch );
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
void ch_printf_color( CHAR_DATA * ch, const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_char( buf, ch );
}

void pager_printf_color( CHAR_DATA * ch, const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   send_to_pager( buf, ch );
}

void paint( short AType, CHAR_DATA * ch, const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2]; /* better safe than sorry */
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
   va_end( args );

   set_char_color( AType, ch );
   send_to_char( buf, ch );
   set_char_color( AType, ch );
}

/* Wrapper function for any "legacy" code that may be installed later */
void send_to_char_color( const char *txt, CHAR_DATA * ch )
{
   send_to_char( txt, ch );
}

/* Wrapper function for any "legacy" code that may be installed later */
void send_to_pager_color( const char *txt, CHAR_DATA * ch )
{
   send_to_pager( txt, ch );
}
