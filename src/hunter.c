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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void do_plantbug( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   BUG_DATA *pbug;
   BUG_DATA *cbug;
   OBJ_DATA *obj;
   bool checkbug = FALSE;
   int schance;

   if( IS_NPC( ch ) )
      return;

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't bug NPC's!\n\r", ch );
      return;
   }

   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "Don't try to plant bugs on immortals.\n\r", ch );
      return;
   }

   if( in_arena( ch ) )
   {
      send_to_char( "You're here to FIGHT, not spy.\n\r", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "You can't bug yourself!\n\r", ch );
      return;
   }

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->item_type == ITEM_BUG )
         checkbug = TRUE;

   if( checkbug == FALSE )
   {
      send_to_char( "You don't have any bugs to plant.\n\r", ch );
      return;
   }

   for( cbug = victim->first_bug; cbug; cbug = cbug->next_in_bug )
      if( !str_cmp( ch->name, cbug->name ) )
      {
         send_to_char( "You have already planted a bug on this person.\n\r", ch );
         return;
      }

   schance = number_percent(  ) - UMIN( 0, ( get_curr_lck( ch ) - 14 ) ) + UMIN( 0, ( get_curr_lck( victim ) - 13 ) );

   if( schance < ch->pcdata->learned[gsn_plantbug] )
   {
      act( AT_WHITE, "You carefully reach into $N's pocket and place a bug.", ch, NULL, victim, TO_CHAR );
      CREATE( pbug, BUG_DATA, 1 );
      pbug->name = ch->name;
      LINK( pbug, victim->first_bug, victim->last_bug, next_in_bug, prev_in_bug );
      learn_from_success( ch, gsn_plantbug );

      for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      {
         if( obj->item_type == ITEM_BUG )
         {
            separate_obj( obj );
            obj_from_char( obj );
            extract_obj( obj );
            break;
         }
      }
      return;
   }
   else
   {
      send_to_char( "&RYou try to find a pocket to plant the bug in but fail!\n\r", ch );
      learn_from_failure( ch, gsn_plantbug );
      if( number_bits( 0 ) == 0 )
         ch_printf( victim, "You feel a slight brush against your pocket to find %s's hand there.\n\r", PERS( ch, victim ) );
      return;
   }
}

void do_showbugs( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   int schance;
   char buf2[MAX_STRING_LENGTH];
   SHIP_DATA *ship;
   BUG_DATA *pbug;
   char buf[MAX_STRING_LENGTH];
   schance = number_percent(  ) - 20;
   if( schance > ch->pcdata->learned[gsn_showbugs] )
   {
      send_to_char( "You can't figure out what to do.\n\r", ch );
      learn_from_failure( ch, gsn_showbugs );
      return;
   }
   send_to_char( "Player                Planet/Ship        Room Name\n\r", ch );
   send_to_char( "------                -----------        ---------\n\r", ch );

   for( d = first_descriptor; d; d = d->next )
      if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) && ( victim = d->character ) != NULL )
      {
         for( pbug = victim->first_bug; pbug; pbug = pbug->next_in_bug )
            if( !str_cmp( pbug->name, ch->name ) )
            {
               if( victim->in_room->area && victim->in_room->area->planet )
                  sprintf( buf2, "%s", victim->in_room->area->planet->name );
               else if( ( ship = ship_from_room( victim->in_room->vnum ) ) != NULL )
                  sprintf( buf2, "%s", ship->name );
               else
                  sprintf( buf2, "Unknown" );
               sprintf( buf, "%-21.21s %-18.18s %s\n\r", PERS( victim, ch ), buf2, victim->in_room->name );
               send_to_char( buf, ch );
               break;
            }
      }
   learn_from_success( ch, gsn_showbugs );
}

void do_bind( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   OBJ_DATA *tobj;
   int schance;
   CHAR_DATA *victim;
   bool checkbinders = FALSE;

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: Bind <victim>\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They are not here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You can not bind yourself!\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only bind players.\n\r", ch );
      return;
   }

   if( IS_SET( victim->pcdata->act2, ACT_BOUND ) )
   {
      send_to_char( "They've already been bound!\n\r", ch );
      return;
   }

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->item_type == ITEM_BINDERS )
      {
         checkbinders = TRUE;
         break;
      }

   if( checkbinders == FALSE )
   {
      send_to_char( "You don't have any binders to bind them with.\n\r", ch );
      return;
   }

   if( victim->position != POS_STUNNED && victim->position != POS_SLEEPING )
   {
      send_to_char( "They need to be stunned or asleep.\n\r", ch );
      return;
   }

   schance = ( int )( ch->pcdata->learned[gsn_bind] );

   if( number_percent(  ) < schance )
   {
      separate_obj( obj );
      obj_from_char( obj );
      obj_to_char( obj, victim );
      act( AT_WHITE, "You quickly bind $N's wrists.", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "$n quickly binds your wrists.", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "$n quickly binds $N's wrists.", ch, NULL, victim, TO_NOTVICT );
      tobj = get_eq_char( ch, WEAR_BOTH_WRISTS );
      if( tobj )
         unequip_char( ch, tobj );

      equip_char( victim, obj, WEAR_BOTH_WRISTS );
      SET_BIT( victim->pcdata->act2, ACT_BOUND );
      learn_from_success( ch, gsn_bind );
   }
   else
   {
      send_to_char( "You peer at the binders, curious upon how to use them.\n\r", ch );
      learn_from_failure( ch, gsn_bind );
   }

   return;
}

void do_unbind( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   bool checkbinders = FALSE;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   if( IS_NPC( ch ) )
   {
      send_to_char( "You're a mob.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: Unbind <victim>\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You can not unbind yourself!\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only unbind players.\n\r", ch );
      return;
   }

   if( IS_SET( ch->pcdata->act2, ACT_BOUND ) )
   {
      send_to_char( "Nice try. You're bound yourself!\n\r", ch );
      return;
   }

   if( !IS_SET( victim->pcdata->act2, ACT_BOUND ) )
   {
      send_to_char( "But they're not bound.\n\r", ch );
      return;
   }


   obj = get_eq_char( victim, WEAR_BOTH_WRISTS );
   if( obj )
      unequip_char( victim, obj );
   else
   {
      send_to_char( "Something went wrong. get an imm.\n\r", ch );
      sprintf( buf, "%s unbinding %s: has no bothwrists object!", ch->name, victim->name );
      bug( buf );
      return;
   }

   for( obj = victim->last_carrying; obj; obj = obj->prev_content )
      if( obj->item_type == ITEM_BINDERS )
      {
         checkbinders = TRUE;
         break;
      }

   if( checkbinders == FALSE )
   {
      bug( "Unbind: no binders in victims inventory." );
      send_to_char( "Something went wrong. get an imm.\n\r", ch );
      return;
   }

   separate_obj( obj );
   obj_from_char( obj );
   obj_to_char( obj, ch );
   act( AT_WHITE, "You quickly unbind $N's wrists.", ch, NULL, victim, TO_CHAR );
   act( AT_WHITE, "$n quickly unbinds your wrists.", ch, NULL, victim, TO_VICT );
   act( AT_WHITE, "$n quickly unbinds $N's wrists.", ch, NULL, victim, TO_NOTVICT );
   REMOVE_BIT( victim->pcdata->act2, ACT_BOUND );
}

void do_gag( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   int schance;

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: Gag <victim>\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They are not here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You can not gag yourself!\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only gag players.\n\r", ch );
      return;
   }

   if( ( victim->position != POS_STUNNED ) && ( victim->position != POS_SLEEPING )
       && !IS_SET( victim->pcdata->act2, ACT_BOUND ) )
   {
      send_to_char( "They need to be stunned, asleep, or bound.\n\r", ch );
      return;
   }

   schance = ( int )( ch->pcdata->learned[gsn_gag] );

   if( number_percent(  ) < schance )
   {
      act( AT_WHITE, "You quickly place a gag over $N's mouth.", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "$n roughly puts a gag over your mouth.", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "$n roughly places a gag on $N's mouth.", ch, NULL, victim, TO_NOTVICT );
      SET_BIT( victim->pcdata->act2, ACT_GAGGED );
      learn_from_success( ch, gsn_gag );
   }
   else
   {
      send_to_char( "You look puzzled as you wonder how to put on such a contraption.\n\r", ch );
      learn_from_failure( ch, gsn_gag );
   }

   return;
}

void do_ungag( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: Ungag <victim>\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch && IS_SET( victim->pcdata->act2, ACT_BOUND ) )
   {
      send_to_char( "You can not ungag yourself when you're bound!\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only ungag players.\n\r", ch );
      return;
   }

   if( !IS_SET( victim->pcdata->act2, ACT_GAGGED ) )
   {
      send_to_char( "But they're not gagged.\n\r", ch );
      return;
   }

   if( victim != ch )
   {
      act( AT_WHITE, "You quickly rip off $N's gag.", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "$n quickly rips off your gag.", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "$n quickly rips off $N's gag.", ch, NULL, victim, TO_NOTVICT );
   }
   else
   {
      act( AT_WHITE, "You quickly rip off your gag.", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "$n quickly rips off his gag.", ch, NULL, victim, TO_NOTVICT );
   }

   REMOVE_BIT( victim->pcdata->act2, ACT_GAGGED );
}

void do_ambush( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   int percent;

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: Ambush <victim>\n\r", ch );
      return;
   }

   if( IS_NPC( ch ) )
   {
      send_to_char( "Only players may use this ability.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "How can you ambush someone who's not even here.\n\r", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "How can you possibly ambush yourself?\n\r", ch );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "You are already fighting someone!\n\r", ch );
      return;
   }

   if( victim->position == POS_FIGHTING )
   {
      send_to_char( "They are already fighting someone!\n\r", ch );
      return;
   }

   if( victim->position <= POS_STUNNED )
   {
      send_to_char( "Come now, there's no honor in that!\n\r", ch );
      return;
   }

   if( !IS_SET( ch->affected_by, AFF_SNEAK ) )
   {
      send_to_char( "You are moving far too loudly to ambush someone!\n\r", ch );
      return;
   }
   percent = number_percent(  ) - ( get_curr_lck( ch ) - 14 ) + ( get_curr_lck( victim ) - 13 );

   affect_strip( ch, gsn_sneak );
   affect_strip( ch, gsn_silent );
   REMOVE_BIT( ch->affected_by, AFF_HIDE );
   if( ch->race != RACE_DEFEL )  /* Defel has perm invis */
      REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
   if( ch->race != RACE_NOGHRI ) /* Noghri has perm sneak */
      REMOVE_BIT( ch->affected_by, AFF_SNEAK );


   act( AT_RED, "You jump out from behind $N and attack $M!", ch, NULL, victim, TO_CHAR );
   act( AT_RED, "$n jumps out from behind you and attacks you!", ch, NULL, victim, TO_VICT );
   act( AT_RED, "$n jumps out from behind $N and attacks $M!", ch, NULL, victim, TO_NOTVICT );
   if( !IS_AWAKE( victim ) || percent < ch->pcdata->learned[gsn_ambush] )
   {
      multi_hit( ch, victim, gsn_ambush );
      learn_from_success( ch, gsn_ambush );
   }
   else
   {
      learn_from_failure( ch, gsn_ambush );
      global_retcode = damage( ch, victim, 0, gsn_ambush );
   }
}

//Contract System by Tawnos.
void do_contract( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   CHAR_DATA *target;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   long amount = 0;
   CONTRACT_DATA *ccontract;
   CONTRACT_DATA *contract;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( IS_NPC( ch ) )
      return;

   if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
   {
      send_to_char( "&RSyntax: contract <person> <target> <amount>\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( ( target = get_char_world_ooc( ch, arg2 ) ) == NULL )
   {
      send_to_char( "Your target is currently not online.\n\r", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "You can't contract yourself!\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) || IS_NPC( target ) )
   {
      send_to_char( "You can't contract NPC's.\n\r", ch );
      return;
   }

   if( ch == target )
   {
      send_to_char( "You can't contract against yourself!\n\r", ch );
      return;
   }

   if( target == victim )
   {
      send_to_char( "You can't contract them to kill themself!\n\r", ch );
      return;
   }

   amount = atoi( arg3 );

   if( amount < 5000 )
   {
      send_to_char( "&RYour contract must be for at least 5000 credits.\n\r", ch );
      return;
   }

   if( ch->gold < amount )
   {
      send_to_char( "&RYou don't have enough credits!\n\r", ch );
      return;
   }

   for( ccontract = victim->first_contract; ccontract; ccontract = ccontract->next_in_contract )
   {
      if( !str_cmp( ccontract->target, target->name ) )
      {
         ch->gold -= amount;
         ccontract->amount += amount;
         ch_printf( ch, "&GYou have contracted %s to kill %s for an amount of %d credits.\n\r", PERS( victim, ch ),
                    target->name, amount );
         ch_printf( victim, "&G%s has contracted you to kill %s, raising your contract reward by %d credits.\n\r",
                    PERS( ch, victim ), target->name, amount );
         return;
      }
   }

   CREATE( contract, CONTRACT_DATA, 1 );
   contract->target = target->name;
   contract->amount = amount;
   LINK( contract, victim->first_contract, victim->last_contract, next_in_contract, prev_in_contract );

   ch->gold -= amount;
   ch_printf( ch, "&GYou have contracted %s to kill %s for an amount of %d credits.\n\r", PERS( victim, ch ), target->name,
              amount );
   ch_printf( victim, "&G%s has contracted you to kill %s for an amount of %d credits.\n\r", PERS( ch, victim ),
              target->name, amount );

}

void do_showcontracts( CHAR_DATA * ch, char *argument )
{
   CONTRACT_DATA *contract;

   send_to_char( "&R   Target   &W|&R Amount\n\r", ch );
   send_to_char( "&W------------|----------\n\r", ch );

   for( contract = ch->first_contract; contract; contract = contract->next_in_contract )
   {
      ch_printf( ch, "&R%-12s&W|&R %d&W\n\r", contract->target, contract->amount );
   }

}

void do_remcontract( CHAR_DATA * ch, char *argument )
{
   CONTRACT_DATA *contract;
   CONTRACT_DATA *scontract = NULL;

   if( argument[0] == '\0' )
   {
      send_to_char( "&RSyntax: remcontract <target name>\n\r", ch );
      return;
   }

   for( contract = ch->first_contract; contract; contract = contract->next_in_contract )
   {
      if( !str_cmp( contract->target, argument ) )
      {
         scontract = contract;
         break;
      }
   }

   if( !scontract || scontract == NULL )
   {
      send_to_char( "No such target.\n\r", ch );
      return;
   }

   STRFREE( scontract->target );
   UNLINK( scontract, ch->first_contract, ch->last_contract, next_in_contract, prev_in_contract );
   DISPOSE( scontract );

   send_to_char( "Contract removed.\n\r", ch );
   return;

}
