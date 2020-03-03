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

/*
 * This is a new automated arena for Smaug 1.4.
 * You can do anything you want with this code, I hope it will be
 * compatible with the DOS version.
 *
 * INSTALLATION:
 * Add to mud.h
 * in pc_data ...
 * char *     betted_on;
 * int 	      bet_amt;
 * down at the bottom of mud.h with all the rest of this stuff ...
 * #define GET_BETTED_ON(ch)    ((ch)->betted_on)
 * #define GET_BET_AMT(ch) ((ch)->bet_amt)
 *
 * change around the Makefile to include this file,
 * You also have to set the room flags in the limbo.are for the arena.
 * The arena bit is 67108864 (It's not included in the help roomflags)
 * This snippet is based on the ROM arena snippet by Kevin Hoogheem
 * It was ported to SMAUG1.4 by LrdElder
 * If you have any cool additions or questions just e-mail me at
 * tdison@swetland.net - LrdElder 10/24/98
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#define PREP_START  42  /* vnum of first prep room */
#define PREP_END    43  /* vnum of last prep room */
#define ARENA_START number_range( 29, 41) /* vnum of first real arena room */
#define ARENA_END   41  /* vnum of last real arena room */
#define HALL_FAME_FILE  SYSTEM_DIR "halloffame.lst"


struct hall_of_fame_element
{
   char name[MAX_INPUT_LENGTH + 1];
   time_t date;
   int award;
   struct hall_of_fame_element *next;
};

void sportschan( char * );
void start_arena( void );
void write_one_fame_node( FILE * fp, struct hall_of_fame_element *node );
void find_bet_winners( CHAR_DATA * winner );
void lost_arena( CHAR_DATA * ch );

struct hall_of_fame_element *fame_list = NULL;

int ppl_challenged = 0;
int ppl_in_arena = 0;
int in_start_arena = 0;
int start_time;
int game_length;
int lo_lim;
int hi_lim;
int time_to_start;
int time_left_in_game;
int arena_pot;
int bet_pot;
int barena = 0;

extern int parsebet( const int currentbet, char *s );
extern int advatoi( char *s );

void do_bet( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   char buf1[MAX_INPUT_LENGTH];
   int newbet;

   argument = one_argument( argument, arg );
   one_argument( argument, buf1 );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs cant bet on the arena.\r\n", ch );
      return;
   }

   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: bet <player> <amt>\r\n", ch );
      return;
   }
   else if( !in_start_arena && !ppl_challenged )
   {
      send_to_char( "Sorry the arena is closed, wait until it opens up to bet.\r\n", ch );
      return;
   }
   else if( ppl_in_arena )
   {
      send_to_char( "Sorry Arena has already started, no more bets.\r\n", ch );
      return;
   }
   else if( !( ch->betted_on = get_char_world_ooc( ch, arg ) ) )
      send_to_char( "No such person exists in the Necropolis.", ch );
   else if( ch->betted_on == ch )
      send_to_char( "That doesn't make much sense, does it?\r\n", ch );
   else if( !( IS_SET( ch->betted_on->in_room->room_flags2, ROOM_ARENA ) ) )
      send_to_char( "Sorry that person is not in the arena.\r\n", ch );
   else
   {
      if( GET_BET_AMT( ch ) > 0 )
      {
         send_to_char( "Sorry you have already bet.\r\n", ch );
         return;
      }
      GET_BETTED_ON( ch ) = ch->betted_on;
      newbet = parsebet( bet_pot, buf1 );
      if( newbet == 0 )
      {
         send_to_char( "Bet some credits why dont you!\r\n", ch );
         return;
      }
      if( newbet > ch->gold )
      {
         send_to_char( "You don't have that much money!\n\r", ch );
         return;
      }
      if( newbet > 500 )
      {
         send_to_char( "Sorry the house will not accept that much.\r\n", ch );
         return;
      }

      ch->gold -= newbet;
      arena_pot += ( newbet / 2 );
      bet_pot += ( newbet / 2 );
      GET_BET_AMT( ch ) = newbet;
      sprintf( buf, "You place %d credits on %s.\r\n", newbet, ch->betted_on->name );
      send_to_char( buf, ch );
      sprintf( buf, "%s has placed %d credits on %s.", ch->name, newbet, ch->betted_on->name );
      sportschan( buf );
   }
}

void do_arena( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;


   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs cant play in the arena.\r\n", ch );
      return;
   }

   if( !in_start_arena )
   {
      send_to_char( "The killing fields are closed right now.\r\n", ch );
      return;
   }

   if( ch->top_level < lo_lim )
   {
      sprintf( buf, "Sorry but you must be at least level %d to enter this arena.\r\n", lo_lim );
      send_to_char( buf, ch );
      return;
   }

   if( ch->top_level > hi_lim )
   {
      send_to_char( "This arena is for lower level characters.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags2, ROOM_ARENA ) )
   {
      send_to_char( "You are in the arena already\r\n", ch );
      return;
   }
   else
   {

      location = ch->in_room;
      ch->pcdata->roomarena = location;
      act( AT_RED, "$n has been whisked away to the killing fields.", ch, NULL, NULL, TO_ROOM );
      char_from_room( ch );
      char_to_room( ch, get_room_index( PREP_START ) );
      act( AT_WHITE, "$n is dropped from the sky.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You have been taken to the killing fields\r\n", ch );
      do_look( ch, "auto" );
      sprintf( buf, "%s has joined the blood bath.", ch->name );
      sportschan( buf );
      send_to_char( buf, ch );
      ch->hit = ch->max_hit;
      ch->mana = ch->max_mana;
      ch->move = ch->max_move;
      return;
   }
}

void do_chaos( CHAR_DATA * ch, char *argument )
{
   char lolimit[MAX_INPUT_LENGTH];
   char hilimit[MAX_INPUT_LENGTH], start_delay[MAX_INPUT_LENGTH];
   char length[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: chaos lo hi start_delay length", ch );
      return;
   }

   /*
    * Usage: chaos lo hi start_delay cost/lev length
    */

   argument = one_argument( argument, lolimit );
   lo_lim = atoi( lolimit );
   argument = one_argument( argument, hilimit );
   hi_lim = atoi( hilimit );
   argument = one_argument( argument, start_delay );
   start_time = atoi( start_delay );
   one_argument( argument, length );
   game_length = atoi( length );

   sprintf( buf, "LowLim %d HiLim %d Delay %d Length %d\n\r", lo_lim, hi_lim, start_time, game_length );
   send_to_char( buf, ch );

   if( hi_lim >= LEVEL_GOD + 1 )
   {
      send_to_char( "Please choose a hi_lim under the Imps top_level\r\n", ch );
      return;
   }

   if( !*lolimit || !*hilimit || !*start_delay || !*length )
   {
      send_to_char( "Usage: chaos lo hi start_delay length", ch );
      return;
   }

   if( lo_lim >= hi_lim )
   {
      send_to_char( "Ya that just might be smart.\r\n", ch );
      return;
   }

   if( ( lo_lim || hi_lim || game_length ) < 0 )
   {
      send_to_char( "I like positive numbers thank you.\r\n", ch );
      return;
   }

   if( start_time <= 0 )
   {
      send_to_char( "Lets at least give them a chance to enter!\r\n", ch );
      return;
   }
   ppl_in_arena = 0;
   in_start_arena = 1;
   time_to_start = start_time;
   time_left_in_game = 0;
   arena_pot = 0;
   bet_pot = 0;
   barena = 1;
   start_arena(  );

}


void start_arena(  )
{
   char buf1[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
//  DESCRIPTOR_DATA *d;


   if( !( ppl_challenged ) )
   {
      if( time_to_start == 0 )
      {
         in_start_arena = 0;
         show_jack_pot(  );
         ppl_in_arena = 1; /* start the blood shed */
         time_left_in_game = game_length;
         start_game(  );
      }
      else
      {
         if( time_to_start > 1 )
         {
            sprintf( buf1, "&WThe Killing Fields are open to top_levels &R%d &Wthru &R%d\r\n", lo_lim, hi_lim );
            sprintf( buf1, "%s%d &Whours to start\r\n", buf1, time_to_start );
            sprintf( buf1, "%s\r\nType &Rarena &Wto enter.\r\n", buf1 );
            sprintf( buf, "With just %d hours to go till the start of the arena %d people have accepted the challenge!\n\r",
                     time_to_start, num_in_arena(  ) );
            sportschan( buf );
            sportschan( buf1 );
         }
         else
         {
            sprintf( buf1, "&WThe Killing Fields are open to top_levels &R%d &Wthru &R%d\r\n", lo_lim, hi_lim );
            sprintf( buf1, "%s1 &Whour to start\r\n", buf1 );
            sprintf( buf1, "%s\r\nType &Rarena &Wto enter.\r\n", buf1 );
            sprintf( buf, "With just 1 hour to go till the start of the arena %d people have accepted the challenge!\n\r",
                     num_in_arena(  ) );
            sportschan( buf );
            sportschan( buf1 );
         }
         sprintf( buf, "&WThe arena has been opened. &R%d &Whour(s) to start.\r\n", time_to_start );
         sportschan( buf );
      }
      /*
       * echo_to_all(AT_WHITE, buf1, ECHOTAR_ALL); 
       */
      time_to_start--;
   }
   else if( !( ppl_in_arena ) )
   {
      if( time_to_start == 0 )
      {
         ppl_challenged = 0;
         game_length = 8;
         show_jack_pot(  );
         ppl_in_arena = 1; /* start the blood shed */
         time_left_in_game = 8;
         start_game(  );
      }
      else
      {
         if( time_to_start > 1 )
         {
            sprintf( buf1, "The duel will start in %d hours.", time_to_start );
         }
         else
         {
            sprintf( buf1, "The duel will start in 1 hour." );
         }
         sportschan( buf1 );
         time_to_start--;
      }
   }
}

void start_game(  )
{
   CHAR_DATA *i;
   DESCRIPTOR_DATA *d;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
      {
         i = d->character;
         if( IS_SET( i->in_room->room_flags2, ROOM_ARENA ) )
         {
            send_to_char( "\r\nThe floor falls out from bellow, droping you in the arena\r\n", i );
            char_from_room( i );
//      i->pcdata->oldac = i->armor;
//       i->armor = -1500;
            char_to_room( i, get_room_index( ARENA_START ) );
            do_look( i, "auto" );
         }
      }
   }
   do_game(  );
}

void do_game(  )
{
   char buf[MAX_INPUT_LENGTH];

   if( num_in_arena(  ) == 1 )
   {
      ppl_in_arena = 0;
      ppl_challenged = 0;
      find_game_winner(  );
   }
   else if( time_left_in_game == 0 )
   {
      do_end_game(  );
   }
   else if( num_in_arena(  ) == 0 )
   {
      ppl_in_arena = 0;
      ppl_challenged = 0;
      silent_end(  );
   }
   else if( time_left_in_game % 5 )
   {
      sprintf( buf, "With %d hours left in the game there are %d players left.", time_left_in_game, num_in_arena(  ) );
      sportschan( buf );
   }
   else if( time_left_in_game == 1 )
   {
      sprintf( buf, "With 1 hour left in the game there are %d players left.", num_in_arena(  ) );
      sportschan( buf );
   }
   else if( time_left_in_game <= 4 )
   {
      sprintf( buf, "With %d hours left in the game there are %d players left.", time_left_in_game, num_in_arena(  ) );
      sportschan( buf );
   }
   time_left_in_game--;
}

void find_game_winner(  )
{
   char buf[MAX_INPUT_LENGTH];
//  char buf2[MAX_INPUT_LENGTH];
   CHAR_DATA *i;
   DESCRIPTOR_DATA *d;
   ROOM_INDEX_DATA *location;

   struct hall_of_fame_element *fame_node;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
      {
         i = d->original ? d->original : d->character;
         if( IS_SET( i->in_room->room_flags2, ROOM_ARENA ) && ( i->top_level < LEVEL_IMMORTAL ) )
         {
            i->hit = i->max_hit;
            i->mana = i->max_mana;
            i->move = i->max_move;
            i->challenged = NULL;
//          i->armor = i->pcdata->oldac;
            char_from_room( i );
            location = i->pcdata->roomarena;
            char_to_room( i, location );
            do_look( i, "auto" );
            act( AT_YELLOW, "$n falls from the sky.", i, NULL, NULL, TO_ROOM );
            if( time_left_in_game == 1 )
            {
               sprintf( buf, "After 1 hour of battle %s is declared the winner", i->name );
               sportschan( buf );
            }
            else
            {
               sprintf( buf, "After %d hours of battle %s is declared the winner",
                        game_length - time_left_in_game, i->name );
               sportschan( buf );
            }
            i->gold += arena_pot / 2;
            sprintf( buf, "You have been awarded %d credits for winning the arena\r\n", ( arena_pot / 2 ) );
            send_to_char( buf, i );
//          sprintf(buf2, "%s awarded %d credits for winning arena", i->name,                 (arena_pot/2));
//          bug(buf2, 0);
            CREATE( fame_node, struct hall_of_fame_element, 1 );
            strncpy( fame_node->name, i->name, MAX_INPUT_LENGTH );
            fame_node->name[MAX_INPUT_LENGTH] = '\0';
            fame_node->date = time( 0 );
            fame_node->award = ( arena_pot / 2 );
            fame_node->next = fame_list;
            fame_list = fame_node;
            write_fame_list(  );
            find_bet_winners( i );
            ppl_in_arena = 0;
            ppl_challenged = 0;
         }
      }
   }
}
void show_jack_pot(  )
{
   char buf1[MAX_INPUT_LENGTH];

   sprintf( buf1, "&G&W\r\n\007\007Lets get ready to RUMBLE!!!!!!!!\r\n" );
   sprintf( buf1, "%sThe jack pot for this arena is %d credits\r\n", buf1, arena_pot );
   sprintf( buf1, "%s%d credits have been bet on this arena.\r\n\r\n", buf1, bet_pot );
   sportschan( buf1 );

}

void silent_end(  )
{
   char buf[MAX_INPUT_LENGTH];
   ppl_in_arena = 0;
   ppl_challenged = 0;
   in_start_arena = 0;
   start_time = 0;
   game_length = 0;
   time_to_start = 0;
   time_left_in_game = 0;
   arena_pot = 0;
   bet_pot = 0;
   sprintf( buf, "It looks like no one was brave enough to enter the Arena." );
   sportschan( buf );
}

void do_end_game(  )
{
   char buf[MAX_INPUT_LENGTH];
   CHAR_DATA *i;
   DESCRIPTOR_DATA *d;
   ROOM_INDEX_DATA *location;
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )

      {
         i = d->character;
         if( IS_SET( i->in_room->room_flags2, ROOM_ARENA ) )
         {
            i->hit = i->max_hit;
            i->mana = i->max_mana;
            i->move = i->max_move;
            i->challenged = NULL;
//          i->armor = i->pcdata->oldac;
            stop_fighting( i, TRUE );
            location = i->pcdata->roomarena;
            char_from_room( i );
            char_to_room( i, location );
            do_look( i, "auto" );
            act( AT_TELL, "$n falls from the sky.", i, NULL, NULL, TO_ROOM );
         }
      }
   }
   sprintf( buf, "After %d hours of battle the Match is a draw", game_length );
   sportschan( buf );
   time_left_in_game = 0;
   ppl_in_arena = 0;
   ppl_challenged = 0;
}

int num_in_arena(  )
{
   DESCRIPTOR_DATA *d;
   int num = 0;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
      {
         if( IS_SET( d->character->in_room->room_flags2, ROOM_ARENA ) )
         {
            if( d->character->top_level < LEVEL_IMMORTAL )
               num++;
         }
      }
   }
   return num;
}

void sportschan( char *argument )
{
   char buf1[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *i;

   sprintf( buf1, "&RInfo: &W%s\r\n", argument );

   for( i = first_descriptor; i; i = i->next )
   {
      if( !i->connected && i->character )
      {
         if( IS_SET( i->character->deaf, CHANNEL_SPORTS ) )
            continue;

         send_to_char( buf1, i->character );
      }
   }
}

void do_awho( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *tch;
   char buf[MAX_INPUT_LENGTH];
   char buf2[MAX_INPUT_LENGTH];
   int phealth;
   int num = num_in_arena(  );

   if( num == 0 )
   {
      send_to_char( "There is noone in the arena right now.\r\n", ch );
      return;
   }

   sprintf( buf, "&W  Players in the &BChaos&W Arena\r\n" );
   sprintf( buf, "%s-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-", buf );
   sprintf( buf, "%s&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-\r\n", buf );
   sprintf( buf, "%sGame Length = &R%-3d   &WTime To Start &R%-3d\r\n", buf, game_length, time_to_start );
   sprintf( buf, "%s&Wtop_level Limits &R%d &Wto &R%d\r\n", buf, lo_lim, hi_lim );
   sprintf( buf, "%s         &WJackpot = &R%d\r\n", buf, arena_pot );
   sprintf( buf, "%s&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B", buf );
   sprintf( buf, "%s-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B\r\n", buf );
   send_to_char( buf, ch );
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
      {
         tch = d->character;
         phealth = ( 100 * tch->hit ) / tch->max_hit;
         if( IS_SET( tch->in_room->room_flags2, ROOM_ARENA ) && ( tch->top_level < LEVEL_IMMORTAL ) )
         {
            sprintf( buf2, "&W%12.12s         %d%%\n\r", tch->name, phealth );
            send_to_char( buf2, ch );
         }
      }
   }
   return;
}

void do_ahall( CHAR_DATA * ch, char *argument )
{
   char site[MAX_INPUT_LENGTH], format[MAX_INPUT_LENGTH], *timestr;
   char format2[MAX_INPUT_LENGTH];
   struct hall_of_fame_element *fame_node;

   char buf[MAX_INPUT_LENGTH];
   char buf2[MAX_INPUT_LENGTH];

   if( !fame_list )
   {
      send_to_char( "No-one is in the Hall of Fame.\r\n", ch );
      return;
   }

   sprintf( buf2, "&B|---------------------------------------|\r\n" );
   strcat( buf2, "|    &WPast Winners of The FotE Arena&B     |\r\n" );
   strcat( buf2, "|---------------------------------------|\r\n\r\n" );

   send_to_char( buf2, ch );
   strcpy( format, "%-25.25s  %-10.10s  %-16.16s\r\n" );
   sprintf( buf, format, "&RName", "&RDate", "&RAward Amt" );
   send_to_char( buf, ch );
   sprintf( buf, format,
            "&B---------------------------------",
            "&B---------------------------------", "&B---------------------------------" );

   send_to_char( buf, ch );
   strcpy( format2, "&W%-25.25s  &R%-10.10s  &Y%-16d\r\n" );
   for( fame_node = fame_list; fame_node; fame_node = fame_node->next )
   {
      if( fame_node->date )
      {
         timestr = asctime( localtime( &( fame_node->date ) ) );
         *( timestr + 10 ) = 0;
         strcpy( site, timestr );
      }
      else
         strcpy( site, "Unknown" );
      sprintf( buf, format2, fame_node->name, site, fame_node->award );
      send_to_char( buf, ch );
   }
   return;
}

void load_hall_of_fame( void )
{
   FILE *fl;
   int date, award;
   char name[MAX_INPUT_LENGTH + 1];
   struct hall_of_fame_element *next_node;

   fame_list = 0;

   if( !( fl = fopen( HALL_FAME_FILE, "r" ) ) )
   {
      perror( "Unable to open hall of fame file" );
      return;
   }
   while( fscanf( fl, "%s %d %d", name, &date, &award ) == 3 )
   {
      CREATE( next_node, struct hall_of_fame_element, 1 );
      strncpy( next_node->name, name, MAX_INPUT_LENGTH );
      next_node->date = date;
      next_node->award = award;
      next_node->next = fame_list;
      fame_list = next_node;
   }

   fclose( fl );
   return;
}

void write_fame_list( void )
{
   FILE *fl;

   if( !( fl = fopen( HALL_FAME_FILE, "w" ) ) )
   {
      bug( "Error writing _hall_of_fame_list", 0 );
      return;
   }
   write_one_fame_node( fl, fame_list );  /* recursively write from end to start */
   fclose( fl );

   return;
}

void write_one_fame_node( FILE * fp, struct hall_of_fame_element *node )
{
   if( node )
   {
      write_one_fame_node( fp, node->next );
      fprintf( fp, "%s %ld %d\n", node->name, ( long )node->date, node->award );
   }
}

void find_bet_winners( CHAR_DATA * winner )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *wch;

   char buf1[MAX_INPUT_LENGTH];

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
      {
         wch = d->original ? d->original : d->character;
         if( ( !IS_NPC( wch ) ) && ( GET_BET_AMT( wch ) > 0 ) && ( GET_BETTED_ON( wch ) == winner ) )
         {
            sprintf( buf1, "You have won %d coins on your bet.\r\n", ( GET_BET_AMT( wch ) ) * 2 );
            send_to_char( buf1, wch );
            wch->gold += GET_BET_AMT( wch ) * 2;
            GET_BETTED_ON( wch ) = NULL;
            GET_BET_AMT( wch ) = 0;
         }
      }
   }
}
void do_challenge( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;

   if( ( victim = get_char_world_ooc( ch, argument ) ) == NULL )
   {
      send_to_char( "&WThat character is not of these realms!\n\r", ch );
      return;
   }

   if( ( ch->top_level > LEVEL_IMMORTAL ) || ( victim->top_level > LEVEL_IMMORTAL ) )
   {
      send_to_char( "Sorry, Immortal's are not allowed to participate in the arena.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) || IS_NPC( ch ) )
   {
      send_to_char( "&WYou cannot challenge mobiles!\n\r", ch );
      return;
   }

   if( victim->name == ch->name )
   {
      send_to_char( "&WYou cannot challenge yourself!", ch );
      return;
   }

   if( victim->top_level < 5 )
   {
      send_to_char( "&WThat character is too young.\n\r", ch );
      return;
   }

   if( get_timer( victim, TIMER_PKILLED ) > 0 )
   {
      send_to_char( "&WThat player has died within the last 5 minutes and cannot be challenged!\n\r", ch );
      return;
   }

   if( get_timer( ch, TIMER_PKILLED ) > 0 )
   {
      send_to_char( "&WYou have died within the last 5 minutes and cannot challenge anyone.\n\r", ch );
      return;
   }

   if( num_in_arena(  ) > 0 )
   {
      send_to_char( "&WSomeone is already in the arena!\n\r", ch );
      return;
   }
   if( ch->pcdata->release_date != 0 )
   {
      send_to_char( "&RYou're in hell. Dink.\n\r", ch );
      return;
   }
   if( victim->pcdata->release_date != 0 )
   {
      send_to_char( "&RYou cannot challenge someone in hell.\n\r", ch );
      return;
   }

   sprintf( buf, "&R%s &Whas challenged you to a duel!\n\r", ch->name );
   send_to_char( buf, victim );
   send_to_char( "&WPlease either accept or decline the challenge.\n\r\n\r", victim );
   sprintf( buf, "%s has challenged %s to a duel!!\n\r", ch->name, victim->name );
   location = ch->in_room;
   ch->pcdata->roomarena = location;
   sportschan( buf );
   victim->challenged = ch;
}

void do_aaccept( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   if( num_in_arena(  ) > 0 )
   {
      send_to_char( "Please wait until the current arena is closed before you accept.\n\r", ch );
      return;
   }

   if( !( ch->challenged ) )
   {
      send_to_char( "You have not been challenged!\n\r", ch );
      return;
   }
   else
   {
      CHAR_DATA *dch;
      dch = ch->challenged;
      if( !dch || dch == NULL )
      {
         send_to_char( "Your challenger has left!\n\r", ch );
         ch->challenged = NULL;
         return;
      }
      location = ch->in_room;
      ch->pcdata->roomarena = location;
      sprintf( buf, "%s has accepted %s's challenge!", ch->name, dch->name );
      sportschan( buf );
      ch->challenged = NULL;
      char_from_room( ch );
      char_to_room( ch, get_room_index( PREP_END ) );
      do_look( ch, "auto" );
      char_from_room( dch );
      char_to_room( dch, get_room_index( PREP_START ) );
      do_look( dch, "auto" );
      ppl_in_arena = 0;
      ppl_challenged = 1;
      time_to_start = 2;
      time_left_in_game = 0;
      arena_pot = 0;
      bet_pot = 0;

      start_arena(  );
      return;
   }
}

void do_adecline( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];

   if( ch->challenged )
   {
      sprintf( buf, "%s has DECLINED %s's challenge! WHAT A WUSS!!!\n\r", ch->name, ch->challenged->name );
      sportschan( buf );
      ch->challenged = NULL;
      return;
   }
   else
   {
      send_to_char( "You have not been challenged!\n\r", ch );
      return;
   }
}

void lost_arena( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];

   sprintf( buf, "%s has been defeated in the arena!\n\r", ch->name );
   sportschan( buf );

   return;
}
