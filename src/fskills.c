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

extern int top_affect;

void fskill_refresh( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   fskill = force_test_skill_use( "meditate", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   switch ( ch->substate )
   {
      default:
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[0] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[0] ) );
         add_timer( ch, TIMER_DO_FUN, 5, fskill_refresh, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_REFRESH] )
         {
            send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
            force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
            force_learn_from_failure( ch, fskill );
            ch->substate = SUB_NONE;
            return;
         }
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[1] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[1] ) );
         ch->mana += number_range( 10, ch->force_control * 40 / 100 ) * ch->force_level_status;
         if( ch->mana > ch->max_mana )
            ch->mana = ch->max_mana;
         force_learn_from_success( ch, fskill );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void fskill_awareness( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   AFFECT_DATA af;

   fskill = force_test_skill_use( "awareness", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
   {
      send_to_char( "You are already fully aware.\n\r", ch );
      return;
   }
   switch ( ch->substate )
   {
      default:

         send_to_char( "You close your eyes, and focus on nothing.\n\r", ch );
         act( AT_PLAIN, "$n closes $s eyes.", ch, NULL, NULL, TO_ROOM );
         add_timer( ch, TIMER_DO_FUN, 3, fskill_awareness, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_AWARENESS] )
         {
            send_to_char( "You open your eyes, but everything looks the same.\n\r", ch );
            act( AT_PLAIN, "$n opens $s eyes.", ch, NULL, NULL, TO_ROOM );
            force_learn_from_failure( ch, fskill );
            ch->substate = SUB_NONE;
            return;
         }

         send_to_char( "You open your eyes with greater awareness of your surroundings.\n\r", ch );
         act( AT_PLAIN, "$n opens $s eyes.", ch, NULL, NULL, TO_ROOM );
         af.type = gsn_truesight;
         af.duration = 112 * ch->force_level_status;
         af.location = APPLY_NONE;
         af.modifier = 0;
         af.bitvector = AFF_TRUESIGHT;
         affect_to_char( ch, &af );
         ch->mana -= fskill->cost;

         force_learn_from_success( ch, fskill );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( "You open your eyes, but everything looks the same.\n\r", ch );
         act( AT_PLAIN, "$n opens $s eyes.", ch, NULL, NULL, TO_ROOM );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void fskill_finfo( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill, *skill;
   int count;
   fskill = force_test_skill_use( "finfo", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_FINFO] )
   {
      send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
      force_learn_from_failure( ch, fskill );
      return;
   }

   force_learn_from_success( ch, fskill );
   draw_force_line( ch, 69 );
   if( ch->force_type == FORCE_JEDI )
      send_to_char( "\n\r         &bYou are a &BJedi&b. ", ch );
   if( ch->force_type == FORCE_SITH )
      send_to_char( "\n\r         &rYou are a &RSith&r. ", ch );
   ch_printf( ch, "   Mana: %s%d%s/%s%d    ",
              ch->force_type == FORCE_JEDI ? "&B" : "&R", ch->mana,
              ch->force_type == FORCE_JEDI ? "&b" : "&r", ch->force_type == FORCE_JEDI ? "&B" : "&R", ch->max_mana );

   ch_printf( ch, "%sLevel: %s\r\n",
              ch->force_type == FORCE_JEDI ? "&b" : "&r",
              ch->force_type == FORCE_JEDI ? force_parse_string( ch, NULL, "&B$nfl" ) : force_parse_string( ch, NULL,
                                                                                                            "&R$nfl" ) );
   draw_force_line_rev( ch, 69 );
   ch_printf( ch, "\r\n\r\n" );
   // Level 1 skills
   if( ch->force_type == FORCE_JEDI )
      send_to_char( "                             &z(&BApprentice &bskills&z)", ch );
   else
      send_to_char( "                             &z(&RApprentice &rskills&z)", ch );
   send_to_char( "\n\r", ch );
   for( count = 1, skill = first_force_skill; skill; skill = skill->next, count++ )
   {
      if( skill->status != 1 )
      {
         count--;
         continue;
      }
      if( ch->force_skill[skill->index] == 0 || skill->notskill == 1 )
      {
         count--;
         continue;
      }
/*
    	if(ch->force_converted)
    	{
    	    if(fskill->status > ch->force_level_status && fskill->type == ch->force_type)
    	    {
    	        count--;
    	        continue;
    	    }
    	}
    	else if(fskill->status > ch->force_level_status)
        {
    	    count--;
    	  continue;
    	}*/
      ch_printf( ch, " %s %s%-15s %s(%s%3d%%%s)&w  ",
                 skill->type == FORCE_JEDI ? "&b" :
                 skill->type == FORCE_SITH ? "&r" : "&W",
                 skill->type == FORCE_JEDI ? "&B" :
                 skill->type == FORCE_SITH ? "&R" : "&w", skill->name,
                 skill->type == FORCE_JEDI ? "&b" :
                 skill->type == FORCE_SITH ? "&r" : "&W",
                 skill->type == FORCE_JEDI ? "&B" :
                 skill->type == FORCE_SITH ? "&R" : "&w", ch->force_skill[skill->index],
                 skill->type == FORCE_JEDI ? "&b" : skill->type == FORCE_SITH ? "&r" : "&W" );

      if( count == 3 )
      {
         send_to_char( "\n\r", ch );
         count = 0;
      }
   }
   send_to_char( "\n\r", ch );
   // Level 2 skills
   if( ch->force_level_status >= 2 )
   {
      if( ch->force_type == FORCE_JEDI )
         send_to_char( "                             &z(&BJedi Knight &bskills&z)", ch );
      else
         send_to_char( "                              &z(&RSith Lord &rskills&z)", ch );
      send_to_char( "\n\r", ch );
      for( count = 1, skill = first_force_skill; skill; skill = skill->next, count++ )
      {
         if( skill->status != 2 )
         {
            count--;
            continue;
         }
         if( ch->force_skill[skill->index] == 0 || skill->notskill == 1 )
         {
            count--;
            continue;
         }
/*
    	if(ch->force_converted)
    	{
    	    if(fskill->status > ch->force_level_status && fskill->type == ch->force_type)
    	    {
    	        count--;
    	        continue;
    	    }
    	}
    	else if(fskill->status > ch->force_level_status)
        {
    	    count--;
    	  continue;
    	}*/
         ch_printf( ch, " %s %s%-15s %s(%s%3d%%%s)&w  ",
                    skill->type == FORCE_JEDI ? "&b" :
                    skill->type == FORCE_SITH ? "&r" : "&W",
                    skill->type == FORCE_JEDI ? "&B" :
                    skill->type == FORCE_SITH ? "&R" : "&w", skill->name,
                    skill->type == FORCE_JEDI ? "&b" :
                    skill->type == FORCE_SITH ? "&r" : "&W",
                    skill->type == FORCE_JEDI ? "&B" :
                    skill->type == FORCE_SITH ? "&R" : "&w", ch->force_skill[skill->index],
                    skill->type == FORCE_JEDI ? "&b" : skill->type == FORCE_SITH ? "&r" : "&W" );

         if( count == 3 )
         {
            send_to_char( "\n\r", ch );
            count = 0;
         }
      }
      send_to_char( "\n\r\n\r", ch );
   }
   if( ch->force_level_status >= 3 )
   {
      // Level 3 skills
      if( ch->force_type == FORCE_JEDI )
         send_to_char( "                             &z(&BJedi Master &bskills&z)", ch );
      else
         send_to_char( "                             &z(&RSith Master &rskills&z)", ch );
      send_to_char( "\n\r", ch );
      for( count = 1, skill = first_force_skill; skill; skill = skill->next, count++ )
      {
         if( skill->status != 3 )
         {
            count--;
            continue;
         }
         if( ch->force_skill[skill->index] == 0 || skill->notskill == 1 )
         {
            count--;
            continue;
         }
/*
    	if(ch->force_converted)
    	{
    	    if(fskill->status > ch->force_level_status && fskill->type == ch->force_type)
    	    {
    	        count--;
    	        continue;
    	    }
    	}
    	else if(fskill->status > ch->force_level_status)
        {
    	    count--;
    	  continue;
    	}*/
         ch_printf( ch, " %s %s%-15s %s(%s%3d%%%s)&w  ",
                    skill->type == FORCE_JEDI ? "&b" :
                    skill->type == FORCE_SITH ? "&r" : "&W",
                    skill->type == FORCE_JEDI ? "&B" :
                    skill->type == FORCE_SITH ? "&R" : "&w", skill->name,
                    skill->type == FORCE_JEDI ? "&b" :
                    skill->type == FORCE_SITH ? "&r" : "&W",
                    skill->type == FORCE_JEDI ? "&B" :
                    skill->type == FORCE_SITH ? "&R" : "&w", ch->force_skill[skill->index],
                    skill->type == FORCE_JEDI ? "&b" : skill->type == FORCE_SITH ? "&r" : "&W" );

         if( count == 3 )
         {
            send_to_char( "\n\r", ch );
            count = 0;
         }
      }
   }
   if( count != 0 )
      send_to_char( "\n\r\n\r", ch );


   return;
}

void fskill_student( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   fskill = force_test_skill_use( "apprentice", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
      return;
   if( IS_IMMORTAL( ch ) )
   {
      send_to_char( "You cannot have an apprentice.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot find that player to make them an apprentice.\r\n", ch );
      return;
   }
   if( victim->force_identified != 1 )
   {
      send_to_char( "You must sense if the force lies in that one first.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "You cannot be your own master.\r\n", ch );
      return;
   }
   if( victim->force_type != FORCE_GENERAL && victim->force_type != ch->force_type )
   {
      send_to_char( "You cannot make that person your apprentice.\r\n", ch );
      return;
   }
   if( victim->force_level_status > FORCE_KNIGHT )
   {
      send_to_char( "You cannot take a master as your apprentice.\r\n", ch );
      return;
   }
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
   STRFREE( victim->force_temp_master );
   victim->force_temp_master = STRALLOC( ch->name );
   return;
}

void fskill_master( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   fskill = force_test_skill_use( "master", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot find that player to take them as your master.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "You cannot be your own apprentice.\r\n", ch );
      return;
   }
   if( !ch->force_temp_master || strcmp( ch->force_temp_master, victim->name ) )
   {
      send_to_char( "They havn't asked you to be their apprentice.\r\n", ch );
      return;
   }
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
   STRFREE( ch->force_master );
   ch->force_master = STRALLOC( ch->force_temp_master );
   if( ch->force_level_status < FORCE_APPRENTICE )
      ch->force_level_status = FORCE_APPRENTICE;
   if( victim->force_type != ch->force_type && ch->force_type != FORCE_GENERAL )
   {
      ch->force_converted = 1;
      ch->force_level_status = FORCE_APPRENTICE;
   }
   ch->force_type = victim->force_type;
   ch->max_mana = ( ch->force_control + ch->force_sense + ch->force_alter ) * 3 * ch->force_level_status;
   ch->mana = ch->max_mana;
   return;
}

void fskill_identify( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   int force_calc;
   if( argument[0] == '\0' )
      argument = str_dup( ch->dest_buf );
   fskill = force_test_skill_use( "sense", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot sense a NPC.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "You sense much force in yourself...\r\n", ch );
      return;
   }
   switch ( ch->substate )
   {
      default:
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
         ch->dest_buf = str_dup( argument );
         add_timer( ch, TIMER_DO_FUN, 5, fskill_identify, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_IDENTIFY] )
         {
            send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
            force_learn_from_failure( ch, fskill );
            ch->substate = SUB_NONE;
            return;
         }
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[1] ), ch );
         if( victim->force_identified == 0 )
         {
            bug( "(%s)Force_chance: %d", victim->name, victim->force_chance );
            if( victim->force_chance != 0 )
               force_calc = number_range( victim->force_chance, 20 );
            else
               force_calc = number_range( -300, 10 );
            bug( "(%s)Force_calc: %d", victim->name, force_calc );
         }
         if( force_calc > 0 && victim->force_identified == 0 )
         {
            int base, x;
            base = force_calc * 100 / 20;
            victim->perm_frc = force_calc;
            victim->force_control = number_range( base, 100 );
            victim->force_alter = number_range( base, 100 );
            victim->force_sense = number_range( base, 100 );
            victim->force_identified = 1;
            x = victim->force_control + victim->force_alter + victim->force_sense;
            if( x < 150 )
               send_to_char( force_parse_string( ch, victim, "The force is with this one.\r\n" ), ch );
            else if( x >= 150 && x < 200 )
               send_to_char( force_parse_string( ch, victim, "The force is strong with this one.\n\r" ), ch );
            else if( x >= 200 && x < 225 )
               send_to_char( force_parse_string( ch, victim, "The force is very strong with this one.\n\r" ), ch );
            else if( x >= 225 && x < 250 )
               send_to_char( force_parse_string( ch, victim, "The force is exceedingly strong with this one...\n\r" ), ch );
            else
               send_to_char( force_parse_string
                             ( ch, victim,
                               "You feel briefly lightheaded...the force possesses this one like few others..." ), ch );

         }
         else if( victim->force_identified == 1 || victim->force_identified == 2 )
         {
            int x;
            victim->force_identified = 1;
            x = victim->force_control + victim->force_alter + victim->force_sense;
            if( x < 150 )
               send_to_char( force_parse_string( ch, victim, "The force is with this one.\r\n" ), ch );
            else if( x >= 150 && x < 200 )
               send_to_char( force_parse_string( ch, victim, "The force is strong with this one.\n\r" ), ch );
            else if( x >= 200 && x < 225 )
               send_to_char( force_parse_string( ch, victim, "The force is very strong with this one.\n\r" ), ch );
            else if( x >= 225 && x < 250 )
               send_to_char( force_parse_string( ch, victim, "The force is exceedingly strong with this one...\n\r" ), ch );
            else
               send_to_char( force_parse_string
                             ( ch, victim,
                               "You feel briefly lightheaded...the force possesses this one like few others..." ), ch );

         }
         else
         {
            send_to_char( force_parse_string( ch, victim, "The force is not with that one.\r\n" ), ch );
            victim->force_identified = -1;
         }
         force_learn_from_success( ch, fskill );
         if( !ch->dest_buf )
            break;
         DISPOSE( ch->dest_buf );
         break;

   }
   ch->substate = SUB_NONE;
   return;
}

void fskill_promote( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   fskill = force_test_skill_use( "promote", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot find that player to promote them.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "You cannot promote yourself.\r\n", ch );
      return;
   }
   if( victim->force_identified != 1 )
   {
      send_to_char( force_parse_string( ch, victim, "$n has not been identified as having ability in the force.\r\n" ), ch );
      return;
   }
   if( victim->force_level_status == FORCE_MASTER )
   {
      send_to_char( force_parse_string( ch, victim, "$n has no need to be promoted!\r\n" ), ch );
      return;
   }
/*
    if(victim->force_level_status == FORCE_KNIGHT && victim->force_type == FORCE_JEDI)
    {
        send_to_char(force_parse_string(ch,victim,"$n cannot be promoted until the Jedi council is finished!\r\n"),ch);
      return;
    }
*/
   if( strcmp( victim->force_master, ch->name ) )
   {
      send_to_char( "You can only promote your own student.\r\n", ch );
      return;
   }
   if( !force_promote_ready( victim ) )
   {
      send_to_char( "They are not ready to be promoted yet.\r\n", ch );
      return;
   }
   victim->force_level_status++;
   if( victim->force_level_status == FORCE_MASTER )
      victim->force_converted = 0;
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
   victim->max_mana = ( victim->force_control + victim->force_sense + victim->force_alter ) * 3 * victim->force_level_status;
   victim->mana = ch->max_mana;
   return;
}

void fskill_instruct( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill, *skill;
   CHAR_DATA *victim;
   bool SKILL_FOUND = FALSE;
   char name[MAX_STRING_LENGTH];
   fskill = force_test_skill_use( "instruct", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot find that player to instruct them.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "You cannot instruct yourself.\r\n", ch );
      return;
   }
   argument = one_argument( argument, name );
   if( argument[0] == '\0' )
   {
      send_to_char( "You must specify what to instruct them in.\r\n", ch );
      return;
   }
   for( skill = first_force_skill; skill; skill = skill->next )
   {
      if( skill->status <= ch->force_level_status && !strcmp( argument, skill->name )
          && ( skill->type == FORCE_GENERAL || skill->type == ch->force_type ) && skill->notskill == 0 )
      {
         SKILL_FOUND = TRUE;
         break;
      }
   }
   if( !SKILL_FOUND )
   {
      send_to_char( "That is not a force skill.\r\n", ch );
      return;
   }
   if( skill->status > victim->force_level_status )
   {
      send_to_char( "You cannot teach that person that force skill.\r\n", ch );
      return;
   }
   if( ch->force_skill[skill->index] < 1 )
   {
      send_to_char( "You can only instruct a skill that you know.\r\n", ch );
      return;
   }
   if( skill->type != ch->force_type && skill->type != FORCE_GENERAL )
   {
      send_to_char( "You cannot teach that skill.\r\n", ch );
      return;
   }
   if( victim->force_skill[skill->index] != 0 )
   {
      send_to_char( "You cannot teach that person that force skill.\r\n", ch );
      return;
   }
   if( victim->force_type != ch->force_type && skill->type != FORCE_GENERAL )
   {
      send_to_char( "You cannot teach that person that force skill.\r\n", ch );
      return;
   }
   if( ch->force_type == FORCE_SITH && victim->force_master != ch->name )
   {
      send_to_char( "You are only allowed to teach your own student.\r\n", ch );
      return;
   }
   victim->force_skill[skill->index] = 1;
   send_to_char( force_parse_string( ch, victim, strrep( fskill->ch_effect[0], "$s", skill->name ) ), ch );
   send_to_char( force_parse_string( ch, victim, strrep( fskill->victim_effect[0], "$s", skill->name ) ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, strrep( fskill->room_effect[0], "$s", skill->name ) ) );
   return;
}

void fskill_heal( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   fskill = force_test_skill_use( "heal", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   switch ( ch->substate )
   {
      default:
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[0] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[0] ) );
         add_timer( ch, TIMER_DO_FUN, 10, fskill_heal, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_HEAL] )
         {
            send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
            force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
            force_learn_from_failure( ch, fskill );
            ch->substate = SUB_NONE;
            return;
         }
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[1] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[1] ) );
         ch->hit += number_range( 1, ch->force_control * 500 / 100 );
         if( ch->hit > ch->max_hit )
            ch->hit = ch->max_hit;
         force_learn_from_success( ch, fskill );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void fskill_protect( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   AFFECT_DATA af;
   if( argument[0] == '\0' )
      argument = str_dup( ch->dest_buf );
   fskill = force_test_skill_use( "protect", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot find that player to protect them.\r\n", ch );
      return;
   }
   if( ch == victim )
   {
      send_to_char( "You cannot protect yourself.\r\n", ch );
      return;
   }
   switch ( ch->substate )
   {
      default:
         if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
         {
            send_to_char( "They already have protection.\r\n", ch );
            return;
         }
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
         ch->dest_buf = str_dup( argument );
         add_timer( ch, TIMER_DO_FUN, 5, fskill_protect, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_PROTECT] )
         {
            send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
            send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
            force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
            force_learn_from_failure( ch, fskill );
            victim->hit -= ch->force_alter * 25 / 100;
            ch->substate = SUB_NONE;
            return;
         }
         af.type = TYPE_HIT;
         af.duration = ch->force_alter * 50 / 100;
         af.location = APPLY_NONE;
         af.modifier = 0;
         af.bitvector = AFF_SANCTUARY;
         affect_to_char( victim, &af );
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[1] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[1] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[1] ) );
         force_learn_from_success( ch, fskill );
         if( !ch->dest_buf )
            break;
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
         victim->hit -= ch->force_alter * 25 / 100;
         DISPOSE( ch->dest_buf );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void fskill_fshield( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   AFFECT_DATA af;
   if( argument[0] == '\0' )
      argument = str_dup( ch->dest_buf );
   fskill = force_test_skill_use( "fshield", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = ch;
   switch ( ch->substate )
   {
      default:
         if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
         {
            send_to_char( "You already have a shield.\r\n", ch );
            return;
         }
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[0] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[0] ) );
         ch->dest_buf = str_dup( argument );
         add_timer( ch, TIMER_DO_FUN, 5, fskill_fshield, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_SHIELD] )
         {
            send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
            force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
            force_learn_from_failure( ch, fskill );
            victim->hit -= ch->force_alter * 25 / 100;
            ch->substate = SUB_NONE;
            return;
         }
         af.type = skill_lookup( "sanctuary" );
         af.duration = ch->force_alter * 50 / 100 * 2;
         af.location = APPLY_NONE;
         af.modifier = 0;
         af.bitvector = AFF_SANCTUARY;
         affect_to_char( victim, &af );
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[1] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[1] ) );
         force_learn_from_success( ch, fskill );
         if( !ch->dest_buf )
            break;
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
         victim->hit -= ch->force_alter * 25 / 100;
         DISPOSE( ch->dest_buf );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void secondslash( CHAR_DATA * ch, CHAR_DATA * victim )
{
   int dam;
   if( !ch || !victim )
      return;

   ch_printf( ch, "You follow through with the other end of your saber.\n\r", PERS( victim, ch ) );
   ch_printf( victim, "%s follows through with the other end of their saber!\n\r", PERS( ch, victim ) );
   act( AT_WHITE, "$n follows through with the other end of their saber!\n\r", ch, NULL, victim, TO_NOTVICT );

   if( number_range( 0, 4 ) != 0 && number_range( 0, 125 ) > URANGE( 1, ch->force_skill[FORCE_SKILL_STRIKE], 75 ) )
   {
      ch_printf( ch, "The slash barely misses them.\n\r" );
      ch_printf( victim, "The other end of the saber cuts the air inches away from you.\n\r" );
      act( AT_WHITE, "$n's slash barely misses $N.", ch, NULL, victim, TO_NOTVICT );
      set_fighting( victim, ch );
      return;
   }
   if( check_parry( ch, victim ) )
   {
      ch_printf( ch, "%s quickly parries your attack.\n\r", PERS( victim, ch ) );
      ch_printf( ch, "You quickly parry the attack.\n\r", PERS( ch, victim ) );
      act( AT_WHITE, "$N quickly parries $n's attack.", ch, NULL, victim, TO_NOTVICT );
      set_fighting( victim, ch );
      return;
   }
   ch_printf( ch, "Your attack cleanly slices into them!\n\r" );
   ch_printf( victim, "The other end of the saber slides along your torso!\n\r" );
   act( AT_WHITE, "$N grimaces as $n's lightsaber slices into them!", ch, NULL, victim, TO_NOTVICT );

   dam =
      URANGE( 40,
              number_range( 0,
                            ch->force_skill[FORCE_SKILL_STRIKE] * ch->force_control / ( 20 *
                                                                                        ( 4 - ch->force_level_status ) ) ),
              100 );
   if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
      dam /= 2;
   victim->hit -= dam / 2;
   return;
}

void fskill_slash( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   OBJ_DATA *wield;
   CHAR_DATA *victim;
   int dam;
   fskill = force_test_skill_use( "slash", ch, FORCE_COMBAT );
   if( fskill == NULL )
      return;
   if( !ch->fighting )
   {
      if( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL
          || ( ( wield->value[3] != WEAPON_LIGHTSABER ) && ( wield->value[3] != WEAPON_DUAL_LIGHTSABER ) ) )
      {
         if( ( wield = get_eq_char( ch, WEAR_DUAL_WIELD ) ) == NULL
             || ( ( wield->value[3] != WEAPON_LIGHTSABER ) && ( wield->value[3] != WEAPON_DUAL_LIGHTSABER ) ) )
         {
            send_to_char( "You must be wielding a lightsaber to use this skill.\r\n", ch );
            return;
         }
      }
      if( ( victim = get_char_room( ch, argument ) ) == NULL )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
      if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
      {
         send_to_char( "You must go elsewhere to do that.\n\r", ch );
         return;
      }
      ch_printf( ch, "You swing your lightsaber at %s!\n\r", PERS( victim, ch ) );
      ch_printf( victim, "%s swings their lightsaber at you!\n\r", PERS( ch, victim ) );
      act( AT_WHITE, "$n swings their lightsaber at $N!\n\r", ch, NULL, victim, TO_NOTVICT );

      if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > URANGE( 1, ch->force_skill[FORCE_SKILL_STRIKE], 75 ) )
      {
         ch_printf( ch, "Your lightsaber cuts the air inches away from them.\n\r" );
         ch_printf( victim, "Their lightsaber cuts the air inches away from your midsection.\n\r" );
         act( AT_WHITE, "$n's slash barely misses $N.", ch, NULL, victim, TO_NOTVICT );
         if( wield->value[3] == WEAPON_DUAL_LIGHTSABER )
            secondslash( ch, victim );
         force_learn_from_failure( ch, fskill );
         return;
      }
      if( check_parry( ch, victim ) )
      {
         ch_printf( ch, "%s quickly parries your attack.\n\r", PERS( victim, ch ) );
         ch_printf( ch, "You quickly parry the attack.\n\r", PERS( ch, victim ) );
         act( AT_WHITE, "$N quickly parries $n's attack.", ch, NULL, victim, TO_NOTVICT );
         if( wield->value[3] == WEAPON_DUAL_LIGHTSABER )
            secondslash( ch, victim );
         force_learn_from_failure( ch, fskill );
         return;
      }
      ch_printf( ch, "Your attack cleanly slices across their midsection!\n\r" );
      ch_printf( victim, "Their attack cleanly slices across your midsection!\n\r" );
      act( AT_WHITE, "$N grimaces as $n's lightsaber slices into them!", ch, NULL, victim, TO_NOTVICT );

      dam =
         URANGE( 40,
                 number_range( 0,
                               ch->force_skill[FORCE_SKILL_STRIKE] * ch->force_control / ( 20 *
                                                                                           ( 4 -
                                                                                             ch->force_level_status ) ) ),
                 100 );
      if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
         dam /= 2;
      victim->hit -= dam * 1.5;
      if( wield->value[3] == WEAPON_DUAL_LIGHTSABER )
         secondslash( ch, victim );
      set_fighting( victim, ch );
      force_learn_from_success( ch, fskill );
      return;
   }

   victim = ch->fighting->who;
   if( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL
       || ( ( wield->value[3] != WEAPON_LIGHTSABER ) && ( wield->value[3] != WEAPON_DUAL_LIGHTSABER ) ) )
   {
      if( ( wield = get_eq_char( ch, WEAR_DUAL_WIELD ) ) == NULL
          || ( ( wield->value[3] != WEAPON_LIGHTSABER ) && ( wield->value[3] != WEAPON_DUAL_LIGHTSABER ) ) )
      {
         send_to_char( "You must be wielding a lightsaber to use this skill.\r\n", ch );
         return;
      }
   }
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
   if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > URANGE( 1, ch->force_skill[FORCE_SKILL_STRIKE], 75 ) )
   {
      send_to_char( force_parse_string( ch, victim, fskill->ch_effect[4] ), ch );
      send_to_char( force_parse_string( ch, victim, fskill->victim_effect[4] ), victim );
      force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[4] ) );
      if( wield->value[3] == WEAPON_DUAL_LIGHTSABER )
         secondslash( ch, victim );
      force_learn_from_failure( ch, fskill );
      return;
   }
   if( check_parry( ch, victim ) )
   {
      send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
      send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
      force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
      if( wield->value[3] == WEAPON_DUAL_LIGHTSABER )
         secondslash( ch, victim );
      force_learn_from_failure( ch, fskill );
      return;
   }

   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[1] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[1] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[1] ) );
   dam =
      URANGE( 40,
              number_range( 0,
                            ch->force_skill[FORCE_SKILL_STRIKE] * ch->force_control / ( 20 *
                                                                                        ( 4 - ch->force_level_status ) ) ),
              100 );
   if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
      dam /= 2;
   victim->hit -= dam;
   if( wield->value[3] == WEAPON_DUAL_LIGHTSABER )
      secondslash( ch, victim );
   force_learn_from_success( ch, fskill );
   return;
}

void fskill_whirlwind( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   OBJ_DATA *wield;
   CHAR_DATA *victim;
   int dam;
   fskill = force_test_skill_use( "whirlwind", ch, FORCE_COMBAT );
   if( fskill == NULL )
      return;
   victim = ch->fighting->who;
   if( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL || ( wield->value[3] != WEAPON_LIGHTSABER ) )
   {
      if( ( wield = get_eq_char( ch, WEAR_DUAL_WIELD ) ) == NULL
          || ( ( wield->value[3] != WEAPON_LIGHTSABER ) && ( wield->value[3] != WEAPON_DUAL_LIGHTSABER ) ) )
      {
         send_to_char( "You must be wielding a lightsaber to use this skill.\r\n", ch );
         return;
      }
   }
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
   if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > URANGE( 1, ch->force_skill[FORCE_SKILL_WHIRLWIND], 75 ) )
   {
      send_to_char( force_parse_string( ch, victim, fskill->ch_effect[4] ), ch );
      send_to_char( force_parse_string( ch, victim, fskill->victim_effect[4] ), victim );
      force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[4] ) );
      force_learn_from_failure( ch, fskill );
      return;
   }
   if( check_parry( ch, victim ) )
   {
      send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
      send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
      force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
      force_learn_from_failure( ch, fskill );
      return;
   }

   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[1] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[1] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[1] ) );
   dam =
      URANGE( 40,
              number_range( 0,
                            ch->force_skill[FORCE_SKILL_WHIRLWIND] * ch->force_control / ( 20 *
                                                                                           ( 4 -
                                                                                             ch->force_level_status ) ) ),
              100 );
   if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
      dam /= 2;
   victim->hit -= dam;
   force_learn_from_success( ch, fskill );
   return;
}

void fskill_squeeze( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   AFFECT_DATA af;
   if( argument[0] == '\0' )
      argument = str_dup( ch->dest_buf );
   fskill = force_test_skill_use( "choke", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
   {
      ch->substate = SUB_NONE;
      return;
   }
   if( ch == victim )
   {
      send_to_char( "You cannot squeeze yourself.\r\n", ch );
      return;
   }
   if( IS_AFFECTED( victim, AFF_PARALYSIS ) )
   {
      send_to_char( "They already are paralyzed.\r\n", ch );
      return;
   }
   switch ( ch->substate )
   {
      default:

         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
         ch->dest_buf = str_dup( argument );
         add_timer( ch, TIMER_DO_FUN, 2, fskill_squeeze, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0
             && ( number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_SQUEEZE]
                  || number_range( 0, 100 ) > ch->force_control ) )
         {
            send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
            send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
            force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
            force_learn_from_failure( ch, fskill );
            ch->hit -= ch->force_alter * 25 / 100;
            ch->substate = SUB_NONE;
            return;
         }
         af.type = TYPE_HIT;
         af.duration = number_range( ch->force_alter * 30 / 100, 30 );
         af.location = APPLY_NONE;
         af.modifier = 0;
         af.bitvector = AFF_PARALYSIS;
         affect_to_char( victim, &af );
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[1] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[1] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[1] ) );
         force_learn_from_success( ch, fskill );
         if( !ch->dest_buf )
            break;
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
         victim->hit -= ch->force_alter * 25 / 100;
         DISPOSE( ch->dest_buf );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void fskill_force_lightning( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   OBJ_DATA *wield;
   CHAR_DATA *victim;
   if( argument[0] == '\0' )
      argument = str_dup( ch->dest_buf );
   fskill = force_test_skill_use( "lightning", ch, FORCE_COMBAT );
   if( fskill == NULL )
      return;
   victim = ch->fighting->who;
   if( !victim )
   {
      ch->substate = SUB_NONE;
      return;
   }
   if( ch == victim )
   {
      send_to_char( "You cannot cast force lightning on yourself.\r\n", ch );
      return;
   }
   victim = ch->fighting->who;
   if( ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL || ( wield = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
   {
      send_to_char( "You cannot be wielding anything when you use this skill.\r\n", ch );
      return;
   }
   switch ( ch->substate )
   {
      default:
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
         ch->dest_buf = str_dup( argument );
         add_timer( ch, TIMER_DO_FUN, 2, fskill_force_lightning, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_FORCE_LIGHTNING] )
         {
            send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
            send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
            force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
            force_learn_from_failure( ch, fskill );
            ch->hit -=
               ( ( ch->force_alter * 40 / 100 ) + ( ch->force_sense * 40 / 100 ) + ( ch->force_control * 40 / 100 ) );
            ch->substate = SUB_NONE;
            return;
         }
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[1] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[1] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[1] ) );
         victim->hit -=
            number_range( 50,
                          ( ( ch->force_alter * 200 / 100 ) + ( ch->force_sense * 200 / 100 ) +
                            ( ch->force_control * 200 / 100 ) ) );
         force_learn_from_success( ch, fskill );
         if( !ch->dest_buf )
            break;
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
         send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
         force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
         ch->hit -= ( ( ch->force_alter * 40 / 100 ) + ( ch->force_sense * 40 / 100 ) + ( ch->force_control * 40 / 100 ) );
         DISPOSE( ch->dest_buf );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void fskill_fdisguise( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   fskill = force_test_skill_use( "fdisguise", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   if( argument[0] == '\0' && !ch->dest_buf )
   {
      send_to_char( "You have to specify a disguise.\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
      argument = str_dup( ch->dest_buf );
   if( !strcmp( argument, "clear" ) )
   {
      if( !ch->pcdata->disguise )
      {
         send_to_char( "You are not disguised right now.\r\n", ch );
         return;
      }
      STRFREE( ch->pcdata->disguise );
      ch->pcdata->disguise = STRALLOC( "" );
      send_to_char( "Disguise removed.\r\n", ch );
      ch->mana += fskill->cost;
      ch->substate = SUB_NONE;
      if( !ch->dest_buf )
         return;
      DISPOSE( ch->dest_buf );
      return;
   }
   switch ( ch->substate )
   {
      default:
         ch->dest_buf = strdup( argument );
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[0] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[0] ) );
         add_timer( ch, TIMER_DO_FUN, 5, fskill_fdisguise, 1 );
         return;
      case 1:
         if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > ch->force_skill[FORCE_SKILL_DISGUISE] )
         {
            send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
            force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
            force_learn_from_failure( ch, fskill );
            ch->substate = SUB_NONE;
            DISPOSE( ch->dest_buf );
            return;
         }
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[1] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[1] ) );
         force_learn_from_success( ch, fskill );
         ch->pcdata->disguise = strdup( argument );
         if( !ch->dest_buf )
            break;
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         send_to_char( force_parse_string( ch, NULL, fskill->ch_effect[2] ), ch );
         force_send_to_room( ch, NULL, force_parse_string( ch, NULL, fskill->room_effect[2] ) );
         DISPOSE( ch->dest_buf );
         break;
   }
   ch->substate = SUB_NONE;
   return;
}

void do_make_master( CHAR_DATA * ch, char *argument )
{
   char force_type[MAX_STRING_LENGTH];
   int ft;
   CHAR_DATA *victim;
   FORCE_SKILL *skill;
/*    if(str_cmp(ch->name,"bambua"))
      return;
*/ argument = one_argument( argument, force_type );
   ft = atoi( force_type );
   if( ft < 1 || ft > 2 )
   {
      send_to_char( "Invalid option; use 1 for Jedi, 2 for Sith.\r\n", ch );
      return;
   }
   victim = get_char_world( ch, argument );
   if( !victim )
      return;
   victim->force_identified = 1;
   victim->force_type = ft;
   victim->force_level_status = 3;
   if( ft == 1 )
      victim->force_align = 100;
   else
      victim->force_align = -100;
   for( skill = first_force_skill; skill; skill = skill->next )
      if( ( skill->type == ft || skill->type == FORCE_GENERAL )
          && ( strcmp( skill->name, "master" ) && strcmp( skill->name, "student" ) && strcmp( skill->name, "promote" )
               && strcmp( skill->name, "instruct" ) ) )
      {
         if( victim->force_skill[skill->index] < 50 )
            victim->force_skill[skill->index] = 50;
      }
      else
         victim->force_skill[skill->index] = 0;

   send_to_char( "Set.\r\n", ch );
   return;
}

void fskill_makelightsaber( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int schance;
   bool checktool, checkdura, checkbatt, checkoven, checkcond, checkcirc, checklens, checkgems, checkmirr;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum, level, gems, charge, gemtype;
   AFFECT_DATA *paf;
   AFFECT_DATA *paf2;
   FORCE_SKILL *fskill;
   fskill = force_test_skill_use( "makelightsaber", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makelightsaber <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbatt = FALSE;
         checkoven = FALSE;
         checkcond = FALSE;
         checkcirc = FALSE;
         checklens = FALSE;
         checkgems = FALSE;
         checkmirr = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) || !IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
         {
            send_to_char( "&RYou need to be in a quiet peaceful place to craft a lightsaber.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_LENS )
               checklens = TRUE;
            if( obj->item_type == ITEM_CRYSTAL )
               checkgems = TRUE;
            if( obj->item_type == ITEM_MIRROR )
               checkmirr = TRUE;
            if( obj->item_type == ITEM_DURAPLAST || obj->item_type == ITEM_DURASTEEL )
               checkdura = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_OVEN )
               checkoven = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_SUPERCONDUCTOR )
               checkcond = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a lightsaber.\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "&RYou need something to make it out of.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a power source for your lightsaber.\n\r", ch );
            return;
         }

         if( !checkoven )
         {
            send_to_char( "&RYou need a small furnace to heat and shape the components.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit board.\n\r", ch );
            return;
         }

         if( !checkcond )
         {
            send_to_char( "&RYou still need a small superconductor for your lightsaber.\n\r", ch );
            return;
         }

         if( !checklens )
         {
            send_to_char( "&RYou still need a lens to focus the beam.\n\r", ch );
            return;
         }

         if( !checkgems )
         {
            send_to_char( "&RLightsabers require 1 to 3 gems to work properly.\n\r", ch );
            return;
         }

         if( !checkmirr )
         {
            send_to_char( "&RYou need a high intesity reflective cup to create a lightsaber.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->force_skill[FORCE_SKILL_MAKELIGHTSABER] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of crafting a lightsaber.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, fskill_makelightsaber, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         force_learn_from_failure( ch, fskill );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->force_skill[FORCE_SKILL_MAKELIGHTSABER] );
   vnum = SABER_VNUM;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkdura = FALSE;
   checkbatt = FALSE;
   checkoven = FALSE;
   checkcond = FALSE;
   checkcirc = FALSE;
   checklens = FALSE;
   checkgems = FALSE;
   checkmirr = FALSE;
   gems = 0;
   charge = 0;
   gemtype = 0;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_OVEN )
         checkoven = TRUE;
      if( ( obj->item_type == ITEM_DURAPLAST || obj->item_type == ITEM_DURASTEEL ) && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }

      if( obj->item_type == ITEM_DURASTEEL && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }

      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         charge = UMIN( obj->value[1], 10 );
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }

      if( obj->item_type == ITEM_SUPERCONDUCTOR && checkcond == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcond = TRUE;
      }

      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcirc = TRUE;
      }

      if( obj->item_type == ITEM_LENS && checklens == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checklens = TRUE;
      }

      if( obj->item_type == ITEM_MIRROR && checkmirr == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkmirr = TRUE;
      }

      if( obj->item_type == ITEM_CRYSTAL && gems < 3 )
      {
         gems++;
         if( gemtype < obj->value[0] )
            gemtype = obj->value[0];
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkgems = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->force_skill[FORCE_SKILL_MAKELIGHTSABER] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )
       || ( !checkmirr ) || ( !checklens ) || ( !checkgems ) || ( !checkcond ) || ( !checkcirc ) )
   {
      send_to_char( "&RYou hold up your new lightsaber and press the switch hoping for the best.\n\r", ch );
      send_to_char( "&RInstead of a blade of light, smoke starts pouring from the handle.\n\r", ch );
      send_to_char( "&RYou drop the hot handle and watch as it melts on away on the floor.\n\r", ch );
      force_learn_from_failure( ch, fskill );
      return;
   }

   obj = create_object( pObjIndex, level );
   obj->item_type = ITEM_WEAPON;

   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
//    SET_BIT( obj->extra_flags, ITEM_ANTI_SOLDIER );
//    SET_BIT( obj->extra_flags, ITEM_ANTI_THIEF );
   SET_BIT( obj->extra_flags, ITEM_ANTI_HUNTER );
   SET_BIT( obj->extra_flags, ITEM_ANTI_PILOT );
   SET_BIT( obj->extra_flags, ITEM_ANTI_CITIZEN );
   SET_BIT( obj->extra_flags, ITEM_CONTRABAND );

   obj->level = level;
   obj->weight = 5;
   STRFREE( obj->name );
   obj->name = STRALLOC( "lightsaber saber" );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   STRFREE( obj->action_desc );
   strcpy( buf, arg );
   strcat( buf, " ignites with a hum and a soft glow." );
   obj->action_desc = STRALLOC( buf );

   CREATE( paf, AFFECT_DATA, 1 );
   paf->type = -1;
   paf->duration = -1;
   paf->location = get_atype( "hitroll" );
   paf->modifier = URANGE( 0, gems, level / 30 );
   paf->bitvector = 0;
   paf->next = NULL;
   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;

   CREATE( paf2, AFFECT_DATA, 1 );
   paf2->type = -1;
   paf2->duration = -1;
   paf2->location = get_atype( "parry" );
   paf2->modifier = ( level / 3 );
   paf2->bitvector = 0;
   paf2->next = NULL;
   LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;

   obj->value[0] = INIT_WEAPON_CONDITION; /* condition  */
   obj->value[1] = ( int )( level / 10 + gemtype * 2 ) * ch->force_level_status; /* min dmg  */
   obj->value[2] = ( int )( level / 5 + gemtype * 6 ) * ch->force_level_status;  /* max dmg */
   obj->value[3] = WEAPON_LIGHTSABER;
   obj->value[4] = ( 1000 + charge ) * ch->force_level_status;
   obj->value[5] = ( 1000 + charge ) * ch->force_level_status;
   obj->cost = 50000;
   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created lightsaber.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new lightsaber.", ch, NULL, argument, TO_ROOM );

   {
//         xpgain = UMIN( obj->cost/6.25 ,( exp_level(ch->skill_level[FORCE_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );
//         gain_exp(ch, xpgain, FORCE_ABILITY);
//         ch_printf( ch , "You gain %d force experience.", xpgain );

   }

   force_learn_from_success( ch, fskill );

}

void fskill_makedualsaber( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int schance;
   bool checktool, checkdura, checkbatt, checkoven, checkcond, checkcirc, checklens, checkgems, checkmirr;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum, level, gems, charge, gemtype;
   AFFECT_DATA *paf;
   FORCE_SKILL *fskill;
   fskill = force_test_skill_use( "makedualsaber", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makedualsaber <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbatt = FALSE;
         checkoven = FALSE;
         checkcond = FALSE;
         checkcirc = FALSE;
         checklens = FALSE;
         checkgems = FALSE;
         checkmirr = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) || !IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
         {
            send_to_char( "&RYou need to be in a quiet peaceful place to craft a lightsaber.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_LENS )
               checklens = TRUE;
            if( obj->item_type == ITEM_CRYSTAL )
               checkgems = TRUE;
            if( obj->item_type == ITEM_MIRROR )
               checkmirr = TRUE;
            if( obj->item_type == ITEM_DURAPLAST || obj->item_type == ITEM_DURASTEEL )
               checkdura = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_OVEN )
               checkoven = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_SUPERCONDUCTOR )
               checkcond = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a dual-bladed lightsaber.\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "&RYou need something to make it out of.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a power source for your dual-bladed lightsaber.\n\r", ch );
            return;
         }

         if( !checkoven )
         {
            send_to_char( "&RYou need a small furnace to heat and shape the components.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit board.\n\r", ch );
            return;
         }

         if( !checkcond )
         {
            send_to_char( "&RYou still need a small superconductor for your dual-bladed lightsaber.\n\r", ch );
            return;
         }

         if( !checklens )
         {
            send_to_char( "&RYou still need a lens to focus the beams.\n\r", ch );
            return;
         }

         if( !checkgems )
         {
            send_to_char( "&RLightsabers require 1 to 3 gems to work properly.\n\r", ch );
            return;
         }

         if( !checkmirr )
         {
            send_to_char( "&RYou need a high intesity reflective cup to create a lightsaber.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->force_skill[FORCE_SKILL_MAKEDUALSABER] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of crafting a dual-bladed lightsaber.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, fskill_makedualsaber, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         force_learn_from_failure( ch, fskill );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->force_skill[FORCE_SKILL_MAKEDUALSABER] );
   vnum = SABER_VNUM;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkdura = FALSE;
   checkbatt = FALSE;
   checkoven = FALSE;
   checkcond = FALSE;
   checkcirc = FALSE;
   checklens = FALSE;
   checkgems = FALSE;
   checkmirr = FALSE;
   gems = 0;
   charge = 0;
   gemtype = 0;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_OVEN )
         checkoven = TRUE;
      if( ( obj->item_type == ITEM_DURAPLAST || obj->item_type == ITEM_DURASTEEL ) && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }

      if( obj->item_type == ITEM_DURASTEEL && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }

      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         charge = UMIN( obj->value[1], 10 );
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }

      if( obj->item_type == ITEM_SUPERCONDUCTOR && checkcond == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcond = TRUE;
      }

      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcirc = TRUE;
      }

      if( obj->item_type == ITEM_LENS && checklens == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checklens = TRUE;
      }

      if( obj->item_type == ITEM_MIRROR && checkmirr == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkmirr = TRUE;
      }

      if( obj->item_type == ITEM_CRYSTAL && gems < 3 )
      {
         gems++;
         if( gemtype < obj->value[0] )
            gemtype = obj->value[0];
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkgems = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->force_skill[FORCE_SKILL_MAKEDUALSABER] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )
       || ( !checkmirr ) || ( !checklens ) || ( !checkgems ) || ( !checkcond ) || ( !checkcirc ) )
   {
      send_to_char( "&RYou hold up your new dual-bladed lightsaber and press the switch.\n\r", ch );
      send_to_char( "&RInstead of a blade of light, smoke starts pouring from the handle.\n\r", ch );
      send_to_char( "&RYou drop the hot handle and watch as it melts on away on the floor.\n\r", ch );
      force_learn_from_failure( ch, fskill );
      return;
   }

   obj = create_object( pObjIndex, level );
   obj->item_type = ITEM_WEAPON;

   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
//    SET_BIT( obj->extra_flags, ITEM_ANTI_SOLDIER );
//    SET_BIT( obj->extra_flags, ITEM_ANTI_THIEF );
   SET_BIT( obj->extra_flags, ITEM_ANTI_HUNTER );
   SET_BIT( obj->extra_flags, ITEM_ANTI_PILOT );
   SET_BIT( obj->extra_flags, ITEM_ANTI_CITIZEN );
   SET_BIT( obj->extra_flags, ITEM_CONTRABAND );

   obj->level = level;
   obj->weight = 5;
   STRFREE( obj->name );
   obj->name = STRALLOC( "lightsaber saber dual" );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   STRFREE( obj->action_desc );
   strcpy( buf, arg );
   strcat( buf, " ignites with a hum and a soft glow." );
   obj->action_desc = STRALLOC( buf );

   CREATE( paf, AFFECT_DATA, 1 );
   paf->type = -1;
   paf->duration = -1;
   paf->location = get_atype( "hitroll" );
   paf->modifier = URANGE( 0, gems + 4, 8 );
   paf->bitvector = 0;
   paf->next = NULL;
   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;

   obj->value[0] = INIT_WEAPON_CONDITION; /* condition  */
   obj->value[1] = ( int )( level / 10 + gemtype * 2 ) * ch->force_level_status; /* min dmg  */
   obj->value[2] = ( int )( level / 5 + gemtype * 6 ) * ch->force_level_status;  /* max dmg */
   obj->value[3] = 12;
   obj->value[4] = ( 1000 + charge ) * ch->force_level_status;
   obj->value[5] = ( 1000 + charge ) * ch->force_level_status;
   obj->cost = 50000;
   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created dual-bladed lightsaber.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new dual-bladed lightsaber.", ch, NULL, argument, TO_ROOM );

   {
//         xpgain = UMIN( obj->cost/6.25 ,( exp_level(ch->skill_level[FORCE_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );
//         gain_exp(ch, xpgain, FORCE_ABILITY);
//         ch_printf( ch , "You gain %d force experience.", xpgain );

   }

   force_learn_from_success( ch, fskill );

}

void fskill_finish( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   OBJ_DATA *wield;
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   fskill = force_test_skill_use( "finish", ch, FORCE_COMBAT );
   if( fskill == NULL )
      return;
   victim = ch->fighting->who;
   if( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL
       || ( ( wield->value[3] != WEAPON_LIGHTSABER ) && wield->value[3] != WEAPON_DUAL_LIGHTSABER ) )
   {
      if( ( wield = get_eq_char( ch, WEAR_DUAL_WIELD ) ) == NULL || ( wield->value[3] != WEAPON_LIGHTSABER ) )
      {
         send_to_char( "You must be wielding a lightsaber to use this skill.\r\n", ch );
         return;
      }
   }
   if( ( IS_DROID( victim ) && victim->hit > 125 ) || victim->hit > 75 )
   {
      ch->mana += fskill->cost;
      send_to_char( "They are not ready to be finished off yet.\r\n", ch );
      return;
   }
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
   if( number_range( 0, 4 ) != 0 && number_range( 0, 100 ) > URANGE( 1, ch->force_skill[FORCE_SKILL_FINISH], 75 ) )
   {
      send_to_char( force_parse_string( ch, victim, fskill->ch_effect[4] ), ch );
      send_to_char( force_parse_string( ch, victim, fskill->victim_effect[4] ), victim );
      force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[4] ) );
      force_learn_from_failure( ch, fskill );
      return;
   }
   if( check_parry( ch, victim ) )
   {
      send_to_char( force_parse_string( ch, victim, fskill->ch_effect[2] ), ch );
      send_to_char( force_parse_string( ch, victim, fskill->victim_effect[2] ), victim );
      force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[2] ) );
      force_learn_from_failure( ch, fskill );
      return;
   }
   if( check_dodge( ch, victim ) && check_dodge( ch, victim ) )
   {
      send_to_char( force_parse_string( ch, victim, fskill->ch_effect[3] ), ch );
      send_to_char( force_parse_string( ch, victim, fskill->victim_effect[3] ), victim );
      force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[3] ) );
      force_learn_from_failure( ch, fskill );
      return;
   }
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[1] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[1] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[1] ) );

/*    if (IS_SET( victim->in_room->room_flags, ROOM_MINIARENA ) && !IS_NPC(victim) )
      {
         char minibuf[MAX_STRING_LENGTH];
                     sprintf(minibuf,"&C[&B-&c=&b| &G&W$n is surrounded by a blue glow before $e is killed &b|&c=&B-&C]&G&w\r\n");
                     act( AT_PLAIN, minibuf, victim,
                        NULL, minibuf , TO_ROOM );
         send_to_char("&C[&B-&c=&b| &G&WYou are surrounded by a blue glow before you are killed &b|&c=&B-&C]&G&w\r\n",victim);
      	stop_fighting( victim, TRUE );
      	stop_fighting( ch, TRUE );
        victim->hit = victim->max_hit;
        victim->mana = victim->max_mana;
        victim->move = victim->max_move; 
        ch->hit = ch->max_hit;
        ch->mana = ch->max_mana;
        ch->move = ch->max_move; 
    }
*/
   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "Normally this is the part where you die, considering you were just cut in half.\r\n", victim );
      send_to_char( "Due to your immortality though, you have surrvived the incredible trauma of being\r\n", victim );
      send_to_char( "severed in two.\r\n", victim );
      stop_fighting( victim, TRUE );
   }
   else
   {
      sprintf( buf, "%s killed by %s at %d, using finish.\n\r", victim->name, ch->name, ch->in_room->vnum );
      log_string( buf );
      raw_kill( ch, victim );
      //add_plr_death(victim->name, "cut into two");
   }

   force_learn_from_success( ch, fskill );
   return;
}

void fskill_fhelp( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   FORCE_HELP *fhelp, *fdefault;
   bool match = FALSE;
   int x, len;
   fskill = force_test_skill_use( "fhelp", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   if( !argument || argument[0] == '\0' )
      argument = STRALLOC( "force" );
   for( fhelp = first_force_help; fhelp; fhelp = fhelp->next )
   {
      if( fhelp->type != ch->force_type && fhelp->type != FORCE_GENERAL )
         continue;
      if( fhelp->status > ch->force_level_status )
         continue;
      if( !strcmp( argument, fhelp->name ) )
      {
         match = TRUE;
         break;
      }
      if( !strcmp( "force", fhelp->name ) )
         fdefault = fhelp;
   }
   if( !match )
   {
      fhelp = fdefault;
      send_to_char( "\r\nNo such Force Help file.\r\n\r\n", ch );
   }
   if( fhelp->skill != -1 && ch->force_skill[fhelp->skill] == 0 )
   {
      send_to_char( "You can only read help files on skills that you know.\r\n", ch );
      return;
   }
   draw_force_line( ch, 55 );
   send_to_char( "\r\n", ch );
   len = strlen( fhelp->name );
   for( x = 0; x < ( 70 - len ) / 2; x++ )
      send_to_char( " ", ch );
   x += len;
   ch_printf( ch, "&G&W%s\r\n", capitalize( strrep( fhelp->name, "_", " " ) ) );
   draw_force_line_rev( ch, 55 );
   send_to_char( "\r\n", ch );
   send_to_char( force_parse_string( ch, NULL, fhelp->desc ), ch );
   return;
}

bool check_reflect( CHAR_DATA * ch, CHAR_DATA * victim, int dam )
{
   OBJ_DATA *wield, *chwield;
   CHAR_DATA *rch;
   int chances;
   if( !IS_AWAKE( victim ) )
      return FALSE;

   if( !IS_NPC( victim ) && victim->force_skill[FORCE_SKILL_REFLECT] < 1 )
      return FALSE;

   if( ( wield = get_eq_char( victim, WEAR_WIELD ) ) == NULL || ( wield->value[3] != WEAPON_LIGHTSABER ) )
   {
      if( ( wield = get_eq_char( victim, WEAR_DUAL_WIELD ) ) == NULL || ( wield->value[3] != WEAPON_LIGHTSABER ) )
         return FALSE;
   }
   if( get_eq_char( ch, WEAR_WIELD ) == NULL && get_eq_char( ch, WEAR_DUAL_WIELD ) == NULL )
      return FALSE;
   if( ( chwield = get_eq_char( ch, WEAR_WIELD ) ) == NULL || ( chwield->value[3] != WEAPON_BLASTER &&
                                                                chwield->value[3] != WEAPON_BOWCASTER ) )
   {
      if( ( chwield = get_eq_char( ch, WEAR_DUAL_WIELD ) ) == NULL || ( chwield->value[3] != WEAPON_BLASTER &&
                                                                        chwield->value[3] != WEAPON_BOWCASTER ) )
         return FALSE;
   }
   chances = ( int )( victim->force_skill[FORCE_SKILL_REFLECT] );
   chances = chances * number_range( 10, ( 25 * victim->force_level_status ) ) / 100;
   if( number_range( 1, 100 ) > chances )
   {
      if( victim->force_level_status > 0 )
         force_learn_from_failure( victim, get_force_skill( "reflect" ) );
      return FALSE;
   }
   victim->pcdata->lost_attacks++;
   {
      char roombuf[MAX_STRING_LENGTH];
      if( IS_NPC( ch ) )
      {
         ch_printf( victim, "&G&YYou swing your lightsaber in an arc, reflecting %s's blast!&w\r\n", ch->short_descr );
         sprintf( roombuf, "&GY%s swings their lightsaber in an arc, reflecting %s's blast!&w\r\n", victim->name,
                  ch->short_descr );
         force_send_to_room( victim, NULL, roombuf );
      }
      else
      {
         ch_printf( victim, "&G&YYou swing your lightsaber in an arc, reflecting %s's blast!&G&w\r\n", ch->name );
         sprintf( roombuf, "&G&Y%s swings their lightsaber in an arc, reflecting %s's blast!&w\r\n", victim->name,
                  ch->name );
         force_send_to_room( victim, NULL, roombuf );
      }
   }
   if( number_range( 1, 100 ) > chances )
   {
      if( number_range( 1, 100 ) < URANGE( 0, chances * victim->force_level_status, 60 ) )
      {
         char roombuf[MAX_STRING_LENGTH];
         rch = ch;
         if( rch )
         {
            if( IS_NPC( rch ) )
            {
               if( !rch->short_descr || rch->short_descr[0] == '\0' )
                  bug( "Fskill_reflect: Rch has no short %d", rch->pIndexData->vnum );
               else if( !victim->name || victim->name[0] == '\0' )
                  bug( "Fskill_reflect: victim has no name" );
               else
               {
                  ch_printf( victim, "You hit %s with a reflected shot.\r\n", rch->short_descr );
                  sprintf( roombuf, "%s hits %s with a reflected shot.\r\n", victim->name, rch->short_descr );
                  force_send_to_room( victim, rch, roombuf );
               }
            }
            else
            {
               if( !rch->name || rch->name[0] == '\0' )
                  bug( "Fskill_reflect: Rch has no name" );
               else if( !victim->name || victim->name[0] == '\0' )
                  bug( "Fskill_reflect: victim has no name" );
               else
               {
                  ch_printf( victim, "You hit %s with a reflected shot.\r\n", rch->name );
                  sprintf( roombuf, "%s hits %s with a reflected shot.\r\n", victim->name, rch->name );
                  force_send_to_room( victim, rch, roombuf );
                  ch_printf( rch, "%s hits you with a reflected shot.\r\n", victim->name );
               }
            }
            rch->hit -= number_range( 50, URANGE( 50, dam / 2, 200 ) );
            if( rch->hit < 0 )
               rch->hit = 0;
         }
      }
      else
      {
         for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
         {
            if( number_range( 1, 100 ) > chances && rch != victim )
            {
               char roombuf[MAX_STRING_LENGTH];
               if( !rch )
                  continue;
               if( IS_NPC( rch ) )
               {
                  if( !rch->short_descr || rch->short_descr[0] == '\0' )
                     bug( "Fskill_reflect: Rch has no short %d", rch->pIndexData->vnum );
                  else if( !victim->name || victim->name[0] == '\0' )
                     bug( "Fskill_reflect: victim has no name" );
                  else
                  {
                     ch_printf( victim, "You hit %s with a reflected shot.\r\n", rch->short_descr );
                     sprintf( roombuf, "%s hits %s with a reflected shot.\r\n", victim->name, rch->short_descr );
                     force_send_to_room( victim, rch, roombuf );
                  }
               }
               else
               {
                  if( !rch->name || rch->name[0] == '\0' )
                     bug( "Fskill_reflect: Rch has no name" );
                  else if( !victim->name || victim->name[0] == '\0' )
                     bug( "Fskill_reflect: victim has no name" );
                  else
                  {
                     ch_printf( victim, "You hit %s with a reflected shot.\r\n", rch->name );
                     sprintf( roombuf, "%s hits %s with a reflected shot.\r\n", victim->name, rch->name );
                     force_send_to_room( victim, rch, roombuf );
                     ch_printf( rch, "%s hits you with a reflected shot.\r\n", victim->name );
                  }
               }
               rch->hit -= number_range( 50, URANGE( 50, dam / 2, 200 ) );
               if( rch->hit < 0 )
                  rch->hit = 0;
               break;
            }
         }
      }
   }
   if( victim->force_level_status > 0 )
      force_learn_from_success( victim, get_force_skill( "reflect" ) );
   return TRUE;
}

void fskill_convert( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   CHAR_DATA *victim;
   fskill = force_test_skill_use( "convert", ch, FORCE_NONCOMBAT );
   if( fskill == NULL )
      return;
   victim = force_get_victim( ch, argument, FORCE_INROOM );
   if( !victim )
      return;
   if( IS_IMMORTAL( ch ) )
   {
      send_to_char( "You cannot have a student.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot find that player to convert them.\r\n", ch );
      return;
   }
   if( victim->force_identified != 1 )
   {
      send_to_char( "That player has not been identified as having the force.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "You cannot convert yourself.\r\n", ch );
      return;
   }
   if( ch->force_type == FORCE_JEDI && victim->force_align < 0 )
   {
      send_to_char( "You cannot convert them until they control their anger.\r\n", ch );
      return;
   }
   if( ch->force_type == FORCE_SITH && victim->force_align > 0 )
   {
      send_to_char( "You cannot convert them, the light side is still too strong in them.\r\n", ch );
      return;
   }
   send_to_char( force_parse_string( ch, victim, fskill->ch_effect[0] ), ch );
   send_to_char( force_parse_string( ch, victim, fskill->victim_effect[0] ), victim );
   force_send_to_room( ch, victim, force_parse_string( ch, victim, fskill->room_effect[0] ) );
   STRFREE( victim->force_temp_master );
   victim->force_temp_master = STRALLOC( ch->name );
   return;
}
