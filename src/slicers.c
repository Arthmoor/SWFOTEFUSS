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

/*Global Variables*/
char title[MAX_INPUT_LENGTH];
char disable[MAX_INPUT_LENGTH];  /*stores what portion of the ship will be disabled. */
                                /*
                                 * Used in void do_disable
                                 */
bool autofly( SHIP_DATA * ship );

/*
 * Slicers.c Containing skills created by Ackbar, Eleven, and Tawnos. *plug*
 *
*/

/* 
 * Tell snoop - modified to set tell_snoop to commfreq, then show all 
 *  incoming/outgoing whatnot on tell
*/
void do_tellsnoop( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   int schance;
   int i = 0;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "&RSyntax: spy <commfreq/clear>\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "clear" ) || !str_cmp( arg, "self" ) )
   {
      send_to_char( "You turn your radio off.\n\r", ch );
      ch->pcdata->tell_snoop = NULL;
      return;
   }

   for( i = 0; i < strlen( arg ); i++ )
   {
      if( isalpha( arg[i] ) )
      {
         send_to_char( "&RSyntax: spy <commfreg/clear>\n\r", ch );
         return;
      }
   }

   if( strlen( arg ) > 3 && arg[3] != '.' )
   {
      send_to_char( "&RSyntax: spy <commfreg/clear>\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_spy] );

   if( number_percent(  ) < schance )
   {
      learn_from_success( ch, gsn_spy );
      ch->pcdata->tell_snoop = STRALLOC( arg );
      sprintf( buf, "You are now listening to all communications with %s.\n\r", ch->pcdata->tell_snoop );
      send_to_char( buf, ch );
   }
   else
   {
      send_to_char( "You fail to find the correct frequency.\n\r", ch );
      learn_from_failure( ch, gsn_spy );
   }
}

char blaha[MAX_STRING_LENGTH];

char *acctname( CHAR_DATA * ch )
{
   static char buf[MAX_STRING_LENGTH];
   const char *name;
   char *s;
   int len;

   *buf = '\0';
   s = buf;
   len = 0;
   name = ch->name;
#define add_to_s(chr) (*s++ = chr, ++len)
   for( ; *name && len < 15; ++name )
   {
      if( isalpha( *name ) )
      {
         switch ( tolower( *name ) )
         {
            case 'a':
               add_to_s( '9' );
               add_to_s( '1' );
               break;
            case 'b':
               add_to_s( '0' );
               add_to_s( '2' );
               break;
            case 'c':
               add_to_s( '5' );
               add_to_s( '9' );
               break;
            case 'd':
               add_to_s( '1' );
               add_to_s( '4' );
               break;
            case 'e':
               add_to_s( '5' );
               break;
            case 'f':
               add_to_s( '6' );
               break;
            case 'g':
               add_to_s( '7' );
               break;
            case 'h':
               add_to_s( '8' );
               break;
            case 'i':
               add_to_s( '9' );
               break;
            case 'j':
               add_to_s( '1' );
               add_to_s( '0' );
               break;
            case 'k':
               add_to_s( '1' );
               add_to_s( '1' );
               break;
            case 'l':
               add_to_s( '2' );
               add_to_s( '0' );
               break;
            case 'm':
               add_to_s( '0' );
               add_to_s( '1' );
               break;
            case 'n':
               add_to_s( '1' );
               add_to_s( '4' );
               break;
            case 'o':
               add_to_s( '0' );
               add_to_s( '9' );
               break;
            case 'p':
               add_to_s( '1' );
               add_to_s( '6' );
               break;
            case 'q':
               add_to_s( '1' );
               add_to_s( '7' );
               break;
            case 'r':
               add_to_s( '5' );
               add_to_s( '4' );
               break;
            case 's':
               add_to_s( '1' );
               add_to_s( '9' );
               break;
            case 't':
               add_to_s( '2' );
               add_to_s( '0' );
               break;
            case 'u':
               add_to_s( '2' );
               add_to_s( '1' );
               break;
            case 'v':
               add_to_s( '2' );
               add_to_s( '2' );
               break;
            case 'w':
               add_to_s( '2' );
               add_to_s( '4' );
               break;
            case 'x':
               add_to_s( '2' );
               add_to_s( '3' );
               break;
            case 'y':
               add_to_s( '5' );
               add_to_s( '2' );
               break;
            case 'z':
               add_to_s( '2' );
               add_to_s( '6' );
               break;
         }
      }
   }
   if( len < 15 )
   {
      size_t namelen;
      char *filler;
      char fillerbuf[MAX_STRING_LENGTH];
      const char *const fillers[] = { "gewhinnqnppali", "hmmithinkishou",
         "ldinsertsomehi", "ddenmessagesin",
         "thisforfuturec", "coderstolaughat",
         "ireallyshouldb", "esleepingnowbu",
         "timaddictedtot", "hisshit"
      };

      *fillerbuf = '\0';
      name = ch->name;
      namelen = strlen( name );
      strcpy( fillerbuf, name );
      if( namelen == 3 )
         strcpy( fillerbuf + namelen, fillers[0] );
      else if( namelen > 11 || namelen < 3 )
         strcpy( fillerbuf + namelen, fillers[9] );
      else
         strcpy( fillerbuf + namelen, fillers[namelen - 3] );

      *s = '\0';
      filler = fillerbuf + strlen( buf );

      for( ; *filler && len < 15; ++filler )
      {
         if( isalpha( *filler ) )
         {
            switch ( tolower( *filler ) )
            {
               case 'a':
                  add_to_s( '6' );
                  add_to_s( '6' );
                  break;
               case 'b':
                  add_to_s( '9' );
                  add_to_s( '0' );
                  break;
               case 'c':
                  add_to_s( '2' );
                  add_to_s( '7' );
                  break;
               case 'd':
                  add_to_s( '2' );
                  add_to_s( '1' );
                  break;
               case 'e':
                  add_to_s( '2' );
                  add_to_s( '2' );
                  break;
               case 'f':
                  add_to_s( '6' );
                  break;
               case 'g':
                  add_to_s( '7' );
                  break;
               case 'h':
                  add_to_s( '5' );
                  add_to_s( '0' );
                  break;
               case 'i':
                  add_to_s( '9' );
                  break;
               case 'j':
                  add_to_s( '1' );
                  add_to_s( '0' );
                  break;
               case 'k':
                  add_to_s( '1' );
                  add_to_s( '1' );
                  break;
               case 'l':
                  add_to_s( '1' );
                  add_to_s( '2' );
                  break;
               case 'm':
                  add_to_s( '1' );
                  add_to_s( '3' );
                  break;
               case 'n':
                  add_to_s( '0' );
                  add_to_s( '1' );
                  break;
               case 'o':
                  add_to_s( '1' );
                  add_to_s( '5' );
                  break;
               case 'p':
                  add_to_s( '1' );
                  add_to_s( '6' );
                  break;
               case 'q':
                  add_to_s( '2' );
                  break;
               case 'r':
                  add_to_s( '5' );
                  add_to_s( '1' );
                  break;
               case 's':
                  add_to_s( '1' );
                  add_to_s( '8' );
                  break;
               case 't':
                  add_to_s( '7' );
                  add_to_s( '2' );
                  break;
               case 'u':
                  add_to_s( '4' );
                  add_to_s( '4' );
                  break;
               case 'v':
                  add_to_s( '9' );
                  break;
               case 'w':
                  add_to_s( '8' );
                  add_to_s( '2' );
                  break;
               case 'x':
                  add_to_s( '1' );
                  add_to_s( '1' );
                  break;
               case 'y':
                  add_to_s( '1' );
                  add_to_s( '4' );
                  break;
               case 'z':
                  add_to_s( '5' );
                  break;
            }

            if( len >= 15 )
               break;

            switch ( tolower( *filler ) )
            {
               case 'a':
                  add_to_s( '2' );
                  break;
               case 'b':
                  add_to_s( '1' );
                  add_to_s( '7' );
                  add_to_s( '3' );
                  break;
               case 'c':
                  add_to_s( '5' );
                  add_to_s( '5' );
                  add_to_s( '8' );
                  break;
               case 'd':
                  add_to_s( '8' );
                  add_to_s( '1' );
                  break;
               case 'e':
                  add_to_s( '3' );
                  add_to_s( '0' );
                  add_to_s( '9' );
                  break;
               case 'f':
                  add_to_s( '6' );
                  add_to_s( '4' );
                  add_to_s( '1' );
                  break;
               case 'g':
                  add_to_s( '6' );
                  add_to_s( '7' );
                  add_to_s( '8' );
                  break;
               case 'h':
                  add_to_s( '5' );
                  break;
               case 'i':
                  add_to_s( '2' );
                  add_to_s( '0' );
                  add_to_s( '2' );
                  break;
               case 'j':
                  add_to_s( '1' );
                  add_to_s( '7' );
                  add_to_s( '0' );
                  break;
               case 'k':
                  add_to_s( '1' );
                  add_to_s( '1' );
                  add_to_s( '1' );
                  break;
               case 'l':
                  add_to_s( '1' );
                  add_to_s( '5' );
                  add_to_s( '2' );
                  break;
               case 'm':
                  add_to_s( '1' );
                  add_to_s( '3' );
                  break;
               case 'n':
                  add_to_s( '0' );
                  add_to_s( '1' );
                  break;
               case 'o':
                  add_to_s( '6' );
                  add_to_s( '1' );
                  add_to_s( '5' );
                  break;
               case 'p':
                  add_to_s( '1' );
                  add_to_s( '6' );
                  break;
               case 'q':
                  add_to_s( '2' );
                  break;
               case 'r':
                  add_to_s( '3' );
                  add_to_s( '3' );
                  break;
               case 's':
                  add_to_s( '0' );
                  add_to_s( '6' );
                  break;
               case 't':
                  add_to_s( '7' );
                  add_to_s( '2' );
                  break;
               case 'u':
                  add_to_s( '9' );
                  add_to_s( '4' );
                  add_to_s( '7' );
                  break;
               case 'v':
                  add_to_s( '5' );
                  add_to_s( '9' );
                  break;
               case 'w':
                  add_to_s( '7' );
                  add_to_s( '1' );
                  break;
               case 'x':
                  add_to_s( '6' );
                  add_to_s( '1' );
                  add_to_s( '1' );
                  break;
               case 'y':
                  add_to_s( '2' );
                  add_to_s( '4' );
                  break;
               case 'z':
                  add_to_s( '6' );
                  add_to_s( '1' );
                  add_to_s( '7' );
                  break;
            }

            if( len >= 15 )
               break;

            switch ( tolower( *filler ) )
            {
               case 'a':
                  add_to_s( '1' );
                  break;
               case 'b':
                  add_to_s( '2' );
                  break;
               case 'c':
                  add_to_s( '3' );
                  break;
               case 'd':
                  add_to_s( '4' );
                  break;
               case 'e':
                  add_to_s( '5' );
                  break;
               case 'f':
                  add_to_s( '6' );
                  break;
               case 'g':
                  add_to_s( '7' );
                  break;
               case 'h':
                  add_to_s( '8' );
                  break;
               case 'i':
                  add_to_s( '9' );
                  break;
               case 'j':
                  add_to_s( '1' );
                  add_to_s( '0' );
                  break;
               case 'k':
                  add_to_s( '1' );
                  add_to_s( '1' );
                  break;
               case 'l':
                  add_to_s( '1' );
                  add_to_s( '2' );
                  break;
               case 'm':
                  add_to_s( '1' );
                  add_to_s( '3' );
                  break;
               case 'n':
                  add_to_s( '1' );
                  add_to_s( '4' );
                  break;
               case 'o':
                  add_to_s( '1' );
                  add_to_s( '5' );
                  break;
               case 'p':
                  add_to_s( '1' );
                  add_to_s( '6' );
                  break;
               case 'q':
                  add_to_s( '1' );
                  add_to_s( '7' );
                  break;
               case 'r':
                  add_to_s( '1' );
                  add_to_s( '8' );
                  break;
               case 's':
                  add_to_s( '1' );
                  add_to_s( '9' );
                  break;
               case 't':
                  add_to_s( '2' );
                  add_to_s( '0' );
                  break;
               case 'u':
                  add_to_s( '2' );
                  add_to_s( '1' );
                  break;
               case 'v':
                  add_to_s( '2' );
                  add_to_s( '2' );
                  break;
               case 'w':
                  add_to_s( '2' );
                  add_to_s( '3' );
                  break;
               case 'x':
                  add_to_s( '2' );
                  add_to_s( '4' );
                  break;
               case 'y':
                  add_to_s( '2' );
                  add_to_s( '5' );
                  break;
               case 'z':
                  add_to_s( '2' );
                  add_to_s( '6' );
                  break;
            }
         }
      }
#undef add_to_s
   }
   buf[15] = '\0';
   return buf;
}

void do_inquire( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   bool checkdata;
   OBJ_DATA *obj;
   int x;
   long xpgain;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   int schance;

   strcpy( arg, argument );
   checkdata = FALSE;
   switch ( ch->substate )
   {
      default:

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_DATAPAD )
               checkdata = TRUE;
         }

         if( ( checkdata == FALSE ) )
         {
            send_to_char( "You need a datapad to slice into the banking computer system.\n\r", ch );
            return;
         }
         if( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
         {
            send_to_char( "You must be in a bank.\n\r", ch );
            return;
         }
         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_inquire] );
         if( number_percent(  ) < schance )
         {

            send_to_char( "&GYou begin the long process of trying to slice into the banking computer system.\n\r", ch );
            sprintf( buf, "$n takes $s datapad and hooks into a data port." );
            act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 10, do_inquire, 1 );
            return;

         }
         send_to_char( "&RYou are unable to find the banking computer system.\n\r", ch );
         learn_from_failure( ch, gsn_inquire );
         return;

      case 1:
         break;
      case SUB_TIMER_DO_ABORT:
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interrupted and fail to finish slicing into the banking computer system.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_inquire] );

   x = number_percent(  );

   if( number_percent(  ) > schance * 2 )
   {
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Galactic Bank Database. Unauthorized entry prohibited. ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                                                          ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                                                   ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Invalid passcode.                                                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      learn_from_failure( ch, gsn_inquire );
      return;
   }

   ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Welcome to the Galactic Bank Database. Unauthorized entry prohibited. ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login: %d                                                          ^x&z|\n\r",
              number_range( 11111, 99999 ) );
   ch_printf( ch, "&z|^g&x Passcode: *********                                                   ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login accepted...retrieving account data, stand by.                   ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x _______  Account  _____________________________________ Savings _____ &z^x|\n\r" );
   for( d = first_descriptor; d; d = d->next )
   {
      if( !d->character )
         continue;
      if( d->connected != CON_PLAYING )
         continue;
      if( IS_IMMORTAL( d->character ) )
         continue;
      ch_printf( ch, "&z|^g&x     # %s                                  %-9.9d      ^x&z|\n\r", acctname( d->character ),
                 d->character->pcdata->bank );
   }
   ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
   ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );

   xpgain = 3000;
   gain_exp( ch, xpgain, SLICER_ABILITY );
   ch_printf( ch, " You gain %d experience points for being a Slicer.\n\r", xpgain );
   learn_from_success( ch, gsn_inquire );
   return;

}


void do_makecommsystem( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checktool, checkdura, checkbattery, checkcrystal, checkcircuit;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;
   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makecommsystem <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbattery = FALSE;
         checkcrystal = FALSE;
         checkcircuit = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "You need to be in a factory to build a commsystem", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DURAPLAST )
               checkdura = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbattery = TRUE;
            if( obj->item_type == ITEM_CRYSTAL )
               checkcrystal = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcircuit = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "You need a toolkit to build a commsystem!\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "You need some duraplast to build a commsystem!\n\r", ch );
            return;
         }

         if( !checkbattery )
         {
            send_to_char( "You need a battery to power your commsystem!\n\r", ch );
            return;
         }

         if( !checkcrystal )
         {
            send_to_char( "You need a small crystal to focus the signal!\n\r", ch );
            return;
         }

         if( !checkcircuit )
         {
            send_to_char( "You need a small circuit to control the commsystem!\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecommsystem] );

         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a commsystem.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 5, do_makecommsystem, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makecommsystem );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecommsystem] );
   vnum = COMMSYS_VNUM;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkdura = FALSE;
   checkbattery = FALSE;
   checkcrystal = FALSE;
   checkcircuit = FALSE;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_DURAPLAST && checkdura == FALSE )

      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }

      if( obj->item_type == ITEM_BATTERY && checkbattery == FALSE )
      {
         checkbattery = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_CRYSTAL && checkcrystal == FALSE )
      {
         checkcrystal = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_CIRCUIT && checkcircuit == FALSE )
      {
         checkcircuit = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecommsystem] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkcrystal ) || ( !checkbattery )
       || ( !checkcircuit ) )
   {
      send_to_char( "&RYou hold up your new commsystem and press a couple of buttons\n\r", ch );
      send_to_char( "&RYour new commsystem begins to shake violently.\n\r", ch );
      send_to_char( "&RYou new commsystem suddnely explodes in your hand.\n\r", ch );
      ch->hit -= 15;
      learn_from_failure( ch, gsn_makecommsystem );
      return;
   }

   obj = create_object( pObjIndex, level );
   obj->item_type = ITEM_COMMSYSTEM;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 2 + level / 10;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " CommSystem" );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was dropped on the floor." );
   obj->description = STRALLOC( buf );;
   obj->cost = 45000;
   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your new commsystem.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new commsystem.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 10,
               ( exp_level( ch->skill_level[SLICER_ABILITY] + 1 ) - exp_level( ch->skill_level[SLICER_ABILITY] ) ) );
      gain_exp( ch, xpgain, SLICER_ABILITY );
      ch_printf( ch, "You gain %d experience as for being a Slicer.", xpgain );
   }
   learn_from_success( ch, gsn_makecommsystem );
}


void do_makedatapad( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checktool, checklens, checkdura, checkbattery, checksuper, checkcircuit;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;
   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makedatapad <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbattery = FALSE;
         checksuper = FALSE;
         checkcircuit = FALSE;
         checklens = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {

            send_to_char( "You need to be in a factory to build a datapad", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DURASTEEL )
               checkdura = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbattery = TRUE;
            if( obj->item_type == ITEM_SUPERCONDUCTOR )
               checksuper = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcircuit = TRUE;
            if( obj->item_type == ITEM_LENS )
               checklens = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "You need a toolkit to build a datapad!\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "You need some durasteel to build your datapad!\n\r", ch );
            return;
         }

         if( !checkbattery )
         {
            send_to_char( "You need a battery to power your datapad!\n\r", ch );
            return;
         }

         if( !checksuper )
         {
            send_to_char( "You need a superconductor to focus the energy of the battery.!\n\r", ch );
            return;
         }

         if( !checkcircuit )
         {
            send_to_char( "You need a small circuit to control the datapad!\n\r", ch );
            return;
         }


         if( !checklens )
         {
            send_to_char( "You need a lens for the display.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makedatapad] );

         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a datapad.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 5, do_makedatapad, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makedatapad );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makedatapad] );
   vnum = DATAPAD_VNUM;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkdura = FALSE;
   checkbattery = FALSE;
   checksuper = FALSE;
   checkcircuit = FALSE;
   checklens = FALSE;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_DURASTEEL && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }

      if( obj->item_type == ITEM_BATTERY && checkbattery == FALSE )
      {
         checkbattery = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_SUPERCONDUCTOR && checksuper == FALSE )
      {
         checksuper = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_CIRCUIT && checkcircuit == FALSE )
      {
         checkcircuit = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_LENS && checklens == FALSE )
      {
         checklens = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makedatapad] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checksuper ) || ( !checkbattery )
       || ( !checkcircuit ) || ( !checklens ) )
   {
      send_to_char( "&RYou hold up your new datapad and begin entering data.\n\r", ch );
      send_to_char( "&RYour new datapad begins to shake violently.\n\r", ch );
      send_to_char( "&RYou new datapad suddnely explodes in your hand.\n\r", ch );
      ch->hit -= 15;
      learn_from_failure( ch, gsn_makedatapad );
      return;
   }

   obj = create_object( pObjIndex, level );
   obj->item_type = ITEM_DATAPAD;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 2 + level / 10;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " CommSystem" );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was dropped on the floor." );
   obj->description = STRALLOC( buf );;
   obj->cost = 45000;
   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your new datpad.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new datapad.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 10,
               ( exp_level( ch->skill_level[SLICER_ABILITY] + 1 ) - exp_level( ch->skill_level[SLICER_ABILITY] ) ) );
      gain_exp( ch, xpgain, SLICER_ABILITY );
      ch_printf( ch, "You gain %d experience as for being a Slicer.", xpgain );
   }
   learn_from_success( ch, gsn_makedatapad );
}


void do_codecrack( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   CHAR_DATA *victim;
   bool checkdata;
   OBJ_DATA *obj;
   int x;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   int schance;

   strcpy( arg, argument );
   checkdata = FALSE;
   switch ( ch->substate )
   {
      default:
         if( argument[0] == '\0' )
         {
            send_to_char( "Syntax: Codecrack <ship>\n\r", ch );
            return;
         }
         else if( ( ship = ship_in_room( ch->in_room, arg ) ) != NULL )
         {
            ship = ship_in_room( ch->in_room, arg );
            strcpy( arg, ship->name );
         }
         else
         {
            send_to_char( "There is no such ship docked here.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_DATAPAD )
               checkdata = TRUE;
         }

         if( checkdata == FALSE )
         {
            send_to_char( "You need a datapad to slice into the ships computer system.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_codecrack] );
         if( number_percent(  ) < schance )
         {

            if( ( ship = ship_in_room( ch->in_room, arg ) ) != NULL )
            {
               ship = get_ship( arg );
               ch->dest_buf = str_dup( arg );
               send_to_char( "&GYou begin the long process of trying to slice into a ships computer.\n\r", ch );
               sprintf( buf, "$n takes $s datapad and hooks into the %s's data port.\n\r", ship->name );
               act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
               add_timer( ch, TIMER_DO_FUN, 25, do_codecrack, 1 );
               return;
            }
            else
            {
               send_to_char( "There is no such ship here.\n\r", ch );
               return;
            }
         }

         send_to_char( "&RYou are unable to find this ship's dataport.\n\r", ch );
         ch->pcdata->is_hacking = FALSE;
         learn_from_failure( ch, gsn_codecrack );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         ch->pcdata->is_hacking = FALSE;
         send_to_char( "&RYou are interupted and fail to finish slicing into the ships computer system.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_DATAPAD )
         checkdata = TRUE;
   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_codecrack] );

   ship = ship_in_room( ch->in_room, arg );
   if( !ship )
   {
      send_to_char( "&RThat ship is no longer here.\n\r", ch );
      return;
   }
   x = number_percent(  );
   if( ship->alarm == 1 )
      x = x * 2;
   if( x > schance * 2 || !checkdata || ( !ship ) )
   {
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
      ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
      ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
      ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rCore Access     &W: Denied                         &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      learn_from_failure( ch, gsn_codecrack );
      if( ship->alarm == 1 )
      {
         if( ( victim = get_char_world_ooc( ch, ship->owner ) ) != NULL )
            ch_printf( victim, "&RYou have received a signal from the alarm module installed in %s.&W", ship->name );
      }
      ch->pcdata->is_hacking = FALSE;
      return;
   }
   ch->pcdata->is_hacking = FALSE;
   if( !ship->password )
   {
      sprintf( buf, "Error..%s does not have a password.\n\r", ship->name );
      send_to_char( buf, ch );
      log_string( buf );
      return;
   }
   {
      long xpgain;
      int pssword;
      if( IS_SET( ship->flags, SHIP_NOSLICER ) )
         pssword = number_range( 1111, 9999 );
      else
         pssword = ship->password;

      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
      ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
      ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
      ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rCore Access     &W: Granted                        &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rShip Password   &W: %-7d                        &B^x]&W^x\r\n", pssword );
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      //xpgain = UMIN( obj->cost*10 ,( exp_level(ch->skill_level[SLICER_ABILITY]+1) - exp_level(ch->skill_level[SLICER_ABILITY]) ) );
      xpgain = 3000;
      ch->pcdata->is_hacking = FALSE;
      gain_exp( ch, xpgain, SLICER_ABILITY );
      ch_printf( ch, " You gain %d experience as for being a Slicer.\n\r", xpgain );
      learn_from_success( ch, gsn_codecrack );
   }
   return;

}

void do_disableship( CHAR_DATA * ch, char *argument )
{

   SHIP_DATA *ship1;
   SHIP_DATA *ship2;
   SHIP_DATA *ship;
   int schance, x;
   bool checkcomm, checkdata;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   long xpgain;
   checkcomm = FALSE;
   checkdata = FALSE;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   switch ( ch->substate )
   {
      default:
         if( arg1[0] == '\0' || arg2[0] == '\0' )
         {
            send_to_char( "Syntax: Disable <ship> <system>\n\rSystem being one of: shields, primary, hyper, launcher.\n\r",
                          ch );
            return;
         }

         if( ( ship1 = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
         {
            send_to_char( "You need to be in a ships cockpit to use this skill.\n\r", ch );
            return;
         }

         if( str_cmp( arg2, "primary" ) && str_cmp( arg2, "shields" ) && str_cmp( arg2, "launcher" )
             && str_cmp( arg2, "hyper" ) )
         {
            send_to_char
               ( "You need to pick a system to disable. Please choose either:\n\rprimary, shields, launcher, hyper.\n\r",
                 ch );
            return;
         }

         if( !ship1->starsystem )
         {
            send_to_char( "Don't you think you should be in a starsystem first.?\n\r", ch );
            return;
         }

         if( IS_SET( ship1->flags, SHIP_SIMULATOR ) )
         {
            send_to_char( "&RYou hook your commsystem into the datapor... wha? Theres no dataport!\n\r", ch );
            return;
         }


         ship2 = get_ship_here( arg1, ship1->starsystem );

         if( !ship2 )
         {
            send_to_char( "There is no ship in this starsystem!\n\r", ch );
            return;
         }

         if( ship2->class == 3 )
         {
            send_to_char( "That ship has too great of security to disable it.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_DATAPAD )
               checkdata = TRUE;
            if( obj->item_type == ITEM_COMMSYSTEM )
               checkcomm = TRUE;
         }

         if( !checkdata )
         {
            send_to_char( "You need a datapad to do this.\n\r", ch );
            return;
         }

         if( !checkcomm )
         {
            send_to_char( "You need a commsystem to do this.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_disable] );
         if( number_percent(  ) < schance )
         {
            if( !str_cmp( arg2, "launcher" ) )
            {
               /*
                * Ship Launcher Disable Code
                */
               strcpy( disable, arg2 );
               send_to_char( "You take out your datapad and commsystem and begin working on disabling the ship.\n\r", ch );
               act( AT_PLAIN, "$n takes out $s datapad and begins working on disabling a ships launcher.\n\r", ch, NULL,
                    NULL, TO_ROOM );
               ch->dest_buf = str_dup( arg1 );
               add_timer( ch, TIMER_DO_FUN, 5, do_disableship, 1 );
               return;
            }

            if( !str_cmp( arg2, "shields" ) )
            {
               strcpy( disable, arg2 );
               send_to_char( "You take out your datapad and commsystem and begin working on disabling the ship.\n\r", ch );
               act( AT_PLAIN, "$n takes out $s datapad and begins working on disabling a ships shields.\n\r", ch, NULL, NULL,
                    TO_ROOM );
               ch->dest_buf = str_dup( arg1 );
               add_timer( ch, TIMER_DO_FUN, 15, do_disableship, 1 );
               return;
            }

            if( !str_cmp( arg2, "hyper" ) )
            {
               /*
                * send_to_char("This is temporarily disabled.\r\n",ch);
                * return;
                */
               strcpy( disable, arg2 );
               send_to_char( "You take out your datapad and commsystem and begin working on disabling the ship.\n\r", ch );
               act( AT_PLAIN, "$n takes out $s datapad and begins working on disabling a ships hyperdrive.\n\r", ch, NULL,
                    NULL, TO_ROOM );
               ch->dest_buf = str_dup( arg1 );
               add_timer( ch, TIMER_DO_FUN, 10, do_disableship, 1 );
               return;
            }

            if( !str_cmp( arg2, "primary" ) )
            {
               strcpy( disable, arg2 );
               send_to_char( "You take out your datapad and commsystem and begin working on disabling the ship.\n\r", ch );
               act( AT_PLAIN, "$n takes out $s datapad and begins working on disabling a ships primary weapons system.\n\r",
                    ch, NULL, NULL, TO_ROOM );
               ch->dest_buf = str_dup( arg1 );
               add_timer( ch, TIMER_DO_FUN, 30, do_disableship, 1 );
               return;
            }
         }
         send_to_char( "&RYou are unable to gain access to this ships computer system.\n\r", ch );
         learn_from_failure( ch, gsn_disable );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg1, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish disabling the ships computer system.\n\r", ch );
         return;
   }

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_DATAPAD )
         checkdata = TRUE;
      if( obj->item_type == ITEM_COMMSYSTEM )
         checkcomm = TRUE;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_disable] );

   ship = get_ship( arg1 );
   x = number_percent(  );

   if( number_percent(  ) > schance * 2 || ( !checkdata ) || ( !checkcomm ) || ( !ship ) )
   {
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
      ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
      ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
      ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rCore Access     &W: Denied                         &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      learn_from_failure( ch, gsn_disable );
      return;
   }
/*for ( roomnum = ship->firstroom ; roomnum <= ship->lastroom ; roomnum++ )
{
  room = get_room_index(roomnum);*/
   if( IS_SET( ship->flags, SHIP_NOSLICER ) )   // &&  ch->pcdata->in_room != room)
   {
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
      ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
      ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
      ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rCore Access     &W: Denied - Login Disabled        &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      learn_from_failure( ch, gsn_disable );
      return;
   }
// }

   if( !str_cmp( disable, "shields" ) )
   {
      ship->autorecharge = FALSE;
      ship->shield = 0;
   }

   if( !str_cmp( disable, "launcher" ) )
   {
      ship->missilestate = MISSILE_DAMAGED;
   }

   if( !str_cmp( disable, "primary" ) )
   {
      ship->primaryState = LASER_DAMAGED;
   }

   if( !str_cmp( disable, "hyper" ) )
   {
      ship->hyperstate = LASER_DAMAGED;
   }

   ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
   ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
   ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
   ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
   ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O &rCore Access     &W: Granted                        &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O &rShip %-8.8s   &W: Disabled                       &B^x]&W^x\r\n", disable );
   ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
   //xpgain = UMIN( obj->cost*10 ,( exp_level(ch->skill_level[SLICER_ABILITY]+1) - exp_level(ch->skill_level[SLICER_ABILITY]) ) );
   xpgain = 3000;
   learn_from_success( ch, gsn_disable );
   gain_exp( ch, xpgain, SLICER_ABILITY );
   ch_printf( ch, " You gain %d experience as for being a Slicer.\n\r", xpgain );
   return;


}

void do_assignpilot( CHAR_DATA * ch, char *argument )
{

   SHIP_DATA *ship1;
   SHIP_DATA *ship;
   int schance;
   long xpgain;
   bool checkdata;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   checkdata = FALSE;
   argument = one_argument( argument, arg1 );
   switch ( ch->substate )
   {
      default:
         if( arg1[0] == '\0' )
         {
            send_to_char( "Syntax: Assignpilot <name>\n\r", ch );
            return;
         }

         if( ( ship1 = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
         {
            send_to_char( "You need to be in a ships cockpit to use this skill.\n\r", ch );
            return;
         }


         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_DATAPAD )
               checkdata = TRUE;
         }

         if( !checkdata )
         {
            send_to_char( "You need a datapad to do this.\n\r", ch );
            return;
         }

         if( autofly( ship1 ) )
         {
            send_to_char( "&W&RYou need to have autopilot turned off in order to access the ships databanks.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_assignpilot] );
         if( number_percent(  ) < schance )
         {
            strcpy( disable, arg2 );
            send_to_char( "You take out your datapad working on changing this ships pilot.\n\r", ch );
            act( AT_PLAIN, "$n takes out $s datapad and begins working on something.\n\r", ch, NULL, argument, TO_ROOM );
            ch->dest_buf = str_dup( arg1 );
            add_timer( ch, TIMER_DO_FUN, 5, do_assignpilot, 1 );
            return;
         }
         send_to_char( "&RYou are unable to gain access to this ships computer system.\n\r", ch );
         learn_from_failure( ch, gsn_assignpilot );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg1, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish assigning a ships pilot.\n\r", ch );
         return;
   }

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_DATAPAD )
         checkdata = TRUE;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_assignpilot] );

   ship = ship_from_cockpit( ch->in_room->vnum );
   if( ship == NULL )
      return;
   if( number_percent(  ) > schance * 2 || ( !checkdata ) || ( !ship ) )
   {
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
      ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
      ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
      ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rCore Access     &W: Denied                         &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      learn_from_failure( ch, gsn_assignpilot );
      return;
   }
   if( IS_SET( ship->flags, SHIP_NOSLICER ) )
   {
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
      ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
      ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
      ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rCore Access     &W: Denied - Login Disabled        &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      learn_from_failure( ch, gsn_assignpilot );
      return;
   }

   STRFREE( ship->pilot );
   ship->pilot = STRALLOC( arg1 );

   ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
   ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", ship->name );
   ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );

   ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
   ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O &rCore Access     &W: Granted                        &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O &rShip Pilot Added&W: %-10.10s                     &B^x]&W^x\r\n", arg1 );
   ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
   //xpgain = UMIN( obj->cost*10 ,( exp_level(ch->skill_level[SLICER_ABILITY]+1) - exp_level(ch->skill_level[SLICER_ABILITY]) ) );
   xpgain = 3000;
   learn_from_success( ch, gsn_assignpilot );
   gain_exp( ch, xpgain, SLICER_ABILITY );
   ch_printf( ch, " You gain %d experience as for being a Slicer.\n\r", xpgain );
   return;


}

void do_slicebank( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   bool checkdata;
   OBJ_DATA *obj;
   long xpgain;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   long steal;
   int schance;
   bool found;


   argument = one_argument( argument, arg2 );
   strcpy( arg, argument );
   checkdata = FALSE;
   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' || arg2[0] == '\0' )
         {
            send_to_char( "Syntax: Slicebank <account> <amount>\n\r", ch );
            return;
         }

         if( ch->fighting )
         {
            send_to_char( "You're a little preoccupied...\n\r", ch );
            return;
         }


         if( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
         {
            send_to_char( "You must be in a bank to slice someones account.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_DATAPAD )
               checkdata = TRUE;
         }
         if( ( checkdata == FALSE ) )
         {
            send_to_char( "You need a datapad to slice into the banking computer system.\n\r", ch );
            return;
         }
         if( !str_cmp( arg2, acctname( ch ) ) )
         {
            send_to_char( "That's your account. Insurance fraud is not applicable here on FotE.\n\r", ch );
            return;
         }

         if( atoi( arg ) < 0 )
         {
            send_to_char( "Why don't you just GIVE them the money?\n\r", ch );
            return;
         }

         ch->dest_buf = str_dup( arg );
         ch->dest_buf_2 = str_dup( arg2 );
         send_to_char( "&GYou begin the long process of trying to slice into the banking computer system.\n\r", ch );
         sprintf( buf, "$n takes $s datapad and hooks it into a data port." );
         act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
         add_timer( ch, TIMER_DO_FUN, 10, do_slicebank, 1 );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         if( !ch->dest_buf_2 )
            return;

         strcpy( arg, ch->dest_buf );
         strcpy( arg2, ch->dest_buf_2 );
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interrupted and fail to finish slicing into the banking computer system.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_DATAPAD )
         checkdata = TRUE;
   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_slicebank] );
   schance = UMIN( schance, 70 );
   found = FALSE;

   for( d = first_descriptor; d; d = d->next )
   {
      if( !d->character )
         continue;
      if( d->connected != CON_PLAYING )
         continue;
      if( IS_IMMORTAL( d->character ) )
         continue;

      if( !str_cmp( arg2, acctname( d->character ) ) )
      {
         found = TRUE;
         break;
      }
   }
   if( number_percent(  ) > schance )
   {
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Galactic Bank Database. Unauthorized entry prohibited. ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                                                          ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                                                   ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Invalid passcode.                                                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      learn_from_failure( ch, gsn_slicebank );
      return;
   }
   if( number_percent(  ) > schance * 2 && found )
   {
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Galactic Bank Database. Unauthorized entry prohibited. ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                                                          ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                                                   ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account information, stand by.            ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Processing request, stand by.                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Request DENIED. Account owner has been notified.                      ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );

      learn_from_failure( ch, gsn_slicebank );
      send_to_char( "&R[&YBank: &WALERT&R] &WAn attempt was made on your bank account.\n\r", d->character );
      return;
   }
   if( !found )
   {
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Galactic Bank Database. Unauthorized entry prohibited. ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                                                          ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                                                   ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account information, stand by.            ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Account %-15.15s is not active.                                ^x&z|\n\r", arg2 );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      return;
   }

   steal = atoi( arg );
   if( steal > d->character->pcdata->bank / 20 )
   {
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Galactic Bank Database. Unauthorized entry prohibited. ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                                                          ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                                                   ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account information, stand by.            ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Processing request, stand by.                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Request DENIED, transfer too high. Account owner has been notified.   ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
      send_to_char( "&R[&YBank: &WALERT&R] &WAn attempt was made on your bank account.\n\r", d->character );
      return;
   }

   ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Welcome to the Galactic Bank Database. Unauthorized entry prohibited. ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login: %d                                                          ^x&z|\n\r",
              number_range( 11111, 99999 ) );
   ch_printf( ch, "&z|^g&x Passcode: *********                                                   ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login accepted...retrieving account information, stand by.            ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Processing request, stand by.                                         ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Request accepted. Credits transferred.                                ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                                                       ^x&z|\n\r" );
   ch_printf( ch, "&z|+---------------------------------------------------------------------+|&w\n\r" );

   ch->pcdata->bank += steal;
   d->character->pcdata->bank -= steal;
   //xpgain = UMIN( obj->cost*10 ,( exp_level(ch->skill_level[SLICER_ABILITY]+1) - exp_level(ch->skill_level[SLICER_ABILITY]) ) );
   xpgain = 3000;
   gain_exp( ch, xpgain, SLICER_ABILITY );
   ch_printf( ch, " You gain %d experience points for being a Slicer.\n\r", xpgain );
   learn_from_success( ch, gsn_slicebank );
   return;

}

void do_checkprints( CHAR_DATA * ch, char *argument )
{
   bool checkdata;
   bool checkcomm;
   OBJ_DATA *obj;
   int x;
   long xpgain;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   int schance;

   strcpy( arg, argument );
   checkdata = FALSE;
   checkcomm = FALSE;
   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "Syntax: checkprints <corpse>\n\r", ch );
            return;
         }


         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_DATAPAD )
               checkdata = TRUE;
            if( obj->item_type == ITEM_COMMSYSTEM )
               checkcomm = TRUE;
         }

         if( ( checkdata == FALSE ) )
         {
            send_to_char( "You need a datapad to gain access to the fingerprint computer system.\n\r", ch );
            return;
         }

         if( ( checkcomm == FALSE ) )
         {
            send_to_char( "You need a commsystem to gain access to the fingerprint computer system.\n\r", ch );
            return;
         }

         if( ch->fighting )
         {
            send_to_char( "While you're fighting?  Nice try.\n\r", ch );
            return;
         }

         obj = get_obj_list( ch, argument, ch->in_room->first_content );
         if( !obj )
         {
            send_to_char( "There's no such corpse here.\n\r", ch );
            return;
         }

         if( obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC )
         {
            send_to_char( "This is not a corpse.\n\r", ch );
            return;
         }

         if( !obj->killer )
         {
            send_to_char( "Error: No Killer. Contact Immortals.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_checkprints] );
         if( number_percent(  ) < schance )
         {
            ch->dest_buf = str_dup( arg );
            send_to_char( "&GYou begin the long process of cross checking fingerprints.\n\r", ch );
            sprintf( buf, "$n takes $s datapad and hooks into a commsystem.\n\r" );
            act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 5, do_checkprints, 1 );
            return;
         }
         send_to_char( "&RYou are unable to find a match for the fingerprints.\n\r", ch );
         learn_from_failure( ch, gsn_checkprints );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish checking the fingerprints.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_DATAPAD )
         checkdata = TRUE;
      if( obj->item_type == ITEM_COMMSYSTEM )
         checkcomm = TRUE;
   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_checkprints] );

   obj = get_obj_list( ch, arg, ch->in_room->first_content );
   if( !obj )
   {
      send_to_char( "There's no such corpse here.\n\r", ch );
      return;
   }

   if( obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC )
   {
      send_to_char( "This is not a corpse.\n\r", ch );
      return;
   }

   if( !obj->killer )
   {
      send_to_char( "Error: No Killer. Contact Immortals.\n\r", ch );
      return;
   }
   x = number_percent(  );

   if( number_percent(  ) > schance * 2 )
   {
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", obj->name );
      ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
      ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
      ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[^O &rCore Access     &W: Denied                         &B^x]&W^x\r\n" );
      ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
      learn_from_failure( ch, gsn_checkprints );
      return;
   }


   ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
   ch_printf( ch, "&B[^O &rTerminal startup&W: %-30.30s &B^x]&W^x\r\n", obj->name );
   ch_printf( ch, "&B[^O &rLogin           &W: %-20.20s           &B^x]&W^x\r\n", ch->name );
   ch_printf( ch, "&B[^O &rAccess Code     &W: %-10d                     &B^x]&W^x\r\n", number_range( 0, 999999 ) );
   ch_printf( ch, "&B[^O &rAccessing Core  &W: Stand by                       &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O                                                  &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O &rCore Access     &W: Granted                        &B^x]&W^x\r\n" );
   ch_printf( ch, "&B[^O &rPrint match     &W: %-15.15s                &B^x]&W^x\r\n",
              obj->killer ? obj->killer : "Error, Show imm" );
   ch_printf( ch, "&B[+-----+-----+-----+-----+-----+-----+-----+-----+-]&W\r\n" );
   //xpgain = UMIN( obj->cost*10 ,( exp_level(ch->skill_level[SLICER_ABILITY]+1) - exp_level(ch->skill_level[SLICER_ABILITY]) ) );
   xpgain = 3000;
   gain_exp( ch, xpgain, SLICER_ABILITY );
   ch_printf( ch, " You gain %d experience points for being a Slicer.\n\r", xpgain );
   learn_from_success( ch, gsn_checkprints );
   return;

}
