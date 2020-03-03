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


BOUNTY_DATA *first_bounty;
BOUNTY_DATA *last_bounty;
BOUNTY_DATA *first_disintegration;
BOUNTY_DATA *last_disintegration;


void disintegration args( ( CHAR_DATA * ch, CHAR_DATA * victim, long amount ) );
void nodisintegration args( ( CHAR_DATA * ch, CHAR_DATA * victim, long amount ) );
int xp_compute( CHAR_DATA * ch, CHAR_DATA * victim );

void save_disintegrations(  )
{
   BOUNTY_DATA *tbounty;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", SYSTEM_DIR, disintegration_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open disintegration.lst for writing!\n\r", 0 );
      return;
   }
   for( tbounty = first_disintegration; tbounty; tbounty = tbounty->next )
   {
      fprintf( fpout, "%s\n", tbounty->target );
      fprintf( fpout, "%ld\n", tbounty->amount );
   }
   fprintf( fpout, "$\n" );
   fclose( fpout );

}


bool is_disintegration( CHAR_DATA * victim )
{
   BOUNTY_DATA *bounty;

   for( bounty = first_disintegration; bounty; bounty = bounty->next )
      if( !str_cmp( victim->name, bounty->target ) )
         return TRUE;
   return FALSE;
}

BOUNTY_DATA *get_disintegration( char *target )
{
   BOUNTY_DATA *bounty;

   for( bounty = first_disintegration; bounty; bounty = bounty->next )
      if( !str_cmp( target, bounty->target ) )
         return bounty;
   return NULL;
}

void load_bounties(  )
{
   FILE *fpList;
   char *target;
   char bountylist[256];
   BOUNTY_DATA *bounty;
   long int amount;

   first_disintegration = NULL;
   last_disintegration = NULL;

   log_string( "Loading disintegrations..." );

   sprintf( bountylist, "%s%s", SYSTEM_DIR, disintegration_LIST );
   if( ( fpList = fopen( bountylist, "r" ) ) == NULL )
   {
      perror( bountylist );
      exit( 1 );
   }

   for( ;; )
   {
      target = feof( fpList ) ? "$" : fread_word( fpList );
      if( target[0] == '$' )
         break;
      CREATE( bounty, BOUNTY_DATA, 1 );
      LINK( bounty, first_disintegration, last_disintegration, next, prev );
      bounty->target = STRALLOC( target );
      amount = fread_number( fpList );
      bounty->amount = amount;
   }
   fclose( fpList );
   log_string( " Done bounties " );

   return;
}

void do_bounties( CHAR_DATA * ch, char *argument )
{
   BOUNTY_DATA *bounty;
   int count = 0;

   set_char_color( AT_WHITE, ch );
   send_to_char( "\n\rBounty                      Amount\n\r", ch );
   for( bounty = first_disintegration; bounty; bounty = bounty->next )
   {
      set_char_color( AT_RED, ch );
      ch_printf( ch, "%-27s %-14ld\n\r", bounty->target, bounty->amount );
      count++;
   }

   if( !count )
   {
      set_char_color( AT_GREY, ch );
      send_to_char( "There are no bounties set at this time.\n\r", ch );
      return;
   }
}
void do_rembounty( CHAR_DATA * ch, char *argument )
{
   BOUNTY_DATA *bounty;
   char buf[MAX_INPUT_LENGTH];

   bounty = get_disintegration( argument );

   if( bounty )
      remove_disintegration( bounty );
   else
   {
      send_to_char( "This bounty does not exist!\n\r", ch );
      return;
   }
   sprintf( buf, "The bounty on %s has been removed!", argument );
   echo_to_all( AT_RED, buf, ECHOTAR_ALL );
}

void disintegration( CHAR_DATA * ch, CHAR_DATA * victim, long amount )
{
   BOUNTY_DATA *bounty;
   bool found;
   char buf[MAX_STRING_LENGTH];

   found = FALSE;

   for( bounty = first_disintegration; bounty; bounty = bounty->next )
   {
      if( !str_cmp( bounty->target, victim->name ) )
      {
         found = TRUE;
         break;
      }
   }

   if( !found )
   {
      CREATE( bounty, BOUNTY_DATA, 1 );
      LINK( bounty, first_disintegration, last_disintegration, next, prev );

      bounty->target = STRALLOC( victim->name );
      bounty->amount = 0;
   }

   bounty->amount = bounty->amount + amount;
   save_disintegrations(  );

   sprintf( buf, "Someone has added %ld credits to the bounty on %s.", amount, victim->name );
   echo_to_all( AT_RED, buf, 0 );

}

void do_addbounty( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];
   long int amount;
   CHAR_DATA *victim;

   if( !argument || argument[0] == '\0' )
   {
      do_bounties( ch, argument );
      return;
   }

   argument = one_argument( argument, arg );

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: Addbounty <target> <amount>\n\r", ch );
      return;
   }

   if( ch->pcdata && ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, "the hunters guild" ) )
   {
      send_to_char( "Your job is to collect bounties not post them.", ch );
      return;
   }

/*    if ( !ch->in_room || ch->in_room->vnum != 6604 )
    {
    	send_to_char( "You will have to go to the Hunters Guild on Tatooine to add a new bounty.", ch );
    	return;
    }*/

   if( argument[0] == '\0' )
      amount = 0;
   else
      amount = atoi( argument );

   if( amount < 5000 )
   {
      send_to_char( "A bounty should be at least 5000 credits.\n\r", ch );
      return;
   }

   if( !( victim = get_char_world_ooc( ch, arg ) ) )
   {
      send_to_char( "They don't appear to be here .. wait til they log in.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only set bounties on other players .. not mobs!\n\r", ch );
      return;
   }

   if( amount <= 0 )
   {
      send_to_char( "Nice try! How about 1 or more credits instead...\n\r", ch );
      return;
   }

   if( ch->gold < amount )
   {
      send_to_char( "You don't have that many credits!\n\r", ch );
      return;
   }

   ch->gold = ch->gold - amount;

   disintegration( ch, victim, amount );
}

void remove_disintegration( BOUNTY_DATA * bounty )
{
   UNLINK( bounty, first_disintegration, last_disintegration, next, prev );
   STRFREE( bounty->target );
   DISPOSE( bounty );

   save_disintegrations(  );
}

void claim_disintegration( CHAR_DATA * ch, CHAR_DATA * victim )
{
   BOUNTY_DATA *bounty;
   long int bexp;
   char buf[MAX_STRING_LENGTH];
   CLAN_DATA *clan;

   if( IS_NPC( victim ) )
      return;

   bounty = get_disintegration( victim->name );

   if( ch == victim )
   {
      if( bounty != NULL )
         remove_disintegration( bounty );
      return;
   }

   if( bounty && !ch->pcdata )
   {
      remove_disintegration( bounty );
      bounty = NULL;
   }

/* Assassin Experience for Clan Leaders added by Tawnos */

   if( victim->pcdata && victim->pcdata->clan )
   {
      clan = victim->pcdata->clan;

      if( victim->pcdata->clan
          && ( !str_cmp( victim->name, clan->leader ) || !str_cmp( victim->name, clan->number1 )
               || !str_cmp( victim->name, clan->number2 ) ) )
      {
         if( !str_cmp( victim->name, clan->leader ) )
         {
            bexp = ( exp_level( ch->skill_level[ASSASSIN_ABILITY] + 1 ) - exp_level( ch->skill_level[ASSASSIN_ABILITY] ) );
            gain_exp( ch, bexp, ASSASSIN_ABILITY );
            set_char_color( AT_BLOOD, ch );
            ch_printf( ch, "You receive %ld assassin experience for executing a clan leader.\n\r", bexp );
         }
         else if( !str_cmp( victim->name, clan->number1 ) || !str_cmp( victim->name, clan->number2 ) )
         {
            bexp = ( exp_level( ch->skill_level[ASSASSIN_ABILITY] + 1 ) - exp_level( ch->skill_level[ASSASSIN_ABILITY] ) );
            gain_exp( ch, bexp, ASSASSIN_ABILITY );
            set_char_color( AT_BLOOD, ch );
            ch_printf( ch, "You receive %ld assassin experience for executing a prominent clan member.\n\r", bexp );
         }
      }
   }

   if( bounty == NULL )
   {
      if( IS_SET( victim->act, PLR_KILLER ) && !IS_NPC( ch ) )
      {
         bexp =
            URANGE( 1, xp_compute( ch, victim ),
                    ( exp_level( ch->skill_level[HUNTING_ABILITY] + 1 ) - exp_level( ch->skill_level[HUNTING_ABILITY] ) ) );
         gain_exp( ch, bexp, HUNTING_ABILITY );
         set_char_color( AT_BLOOD, ch );
         ch_printf( ch, "You receive %ld hunting experience for executing a wanted killer.\n\r", bexp );
      }
      else if( !IS_NPC( ch ) )
      {
         SET_BIT( ch->act, PLR_KILLER );
         ch_printf( ch, "You are now wanted for the murder of %s.\n\r", victim->name );
      }
      sprintf( buf, "%s is Dead!", victim->name );
      echo_to_all( AT_RED, buf, 0 );
      return;

   }

   ch->gold += bounty->amount;

   bexp =
      URANGE( 1, bounty->amount + xp_compute( ch, victim ),
              ( exp_level( ch->skill_level[HUNTING_ABILITY] + 1 ) - exp_level( ch->skill_level[HUNTING_ABILITY] ) ) );
   gain_exp( ch, bexp, HUNTING_ABILITY );

   set_char_color( AT_BLOOD, ch );
   ch_printf( ch, "You receive %ld experience and %ld credits,\n\r from the bounty on %s\n\r", exp, bounty->amount,
              bounty->target );

   sprintf( buf, "The disintegration bounty on %s has been claimed!", victim->name );
   echo_to_all( AT_RED, buf, 0 );
   sprintf( buf, "%s is Dead!", victim->name );
   echo_to_all( AT_RED, buf, 0 );

   if( !IS_SET( victim->act, PLR_KILLER ) )
      SET_BIT( ch->act, PLR_KILLER );
   remove_disintegration( bounty );
}
