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

/*
 *  Locals
 */
const char *tiny_affect_loc_name( int location );

void do_gold( CHAR_DATA * ch, const char *argument )
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch, "You have %d credits.\r\n", ch->gold );
   return;
}


char *drawlife( int min, int max )
{
   static char buf[MAX_STRING_LENGTH];
   int per;
   per = ( ( min * 100 ) / max );
   if( per < 0 )
      per = 0;
   if( per > 100 )
      per = 100;
   if( per == 100 )
      sprintf( buf, "&G&W[&R|||||&Y||||&G||||&W]" );
   else if( per >= 90 && per < 100 )
      sprintf( buf, "&G&W[&R|||||&Y||||&G|||&G-&W]" );
   else if( per >= 80 && per < 90 )
      sprintf( buf, "&G&W[&R|||||&Y||||&G||&G--&W]" );
   else if( per >= 70 && per < 80 )
      sprintf( buf, "&G&W[&R|||||&Y||||&G|&G---&W]" );
   else if( per >= 60 && per < 70 )
      sprintf( buf, "&G&W[&R|||||&Y|||&G&G-----&W]" );
   else if( per >= 50 && per < 60 )
      sprintf( buf, "&G&W[&R|||||&Y||&G&G------&W]" );
   else if( per >= 40 && per < 50 )
      sprintf( buf, "&G&W[&R|||||&Y|&G&G-------&W]" );
   else if( per >= 30 && per < 40 )
      sprintf( buf, "&G&W[&R|||||&Y&G&G--------&W]" );
   else if( per >= 30 && per < 40 )
      sprintf( buf, "&G&W[&R||||&Y&G&G---------&W]" );
   else if( per >= 20 && per < 40 )
      sprintf( buf, "&G&W[&R|||&Y&G&G----------&W]" );
   else if( per >= 10 && per < 40 )
      sprintf( buf, "&G&W[&R||&Y&G&G-----------&W]" );
   else if( per >= 0 && per < 10 )
      sprintf( buf, "&G&W[&R&Y&G&G-------------&W]" );
   //else sprintf(buf, "&G&W[&R&W]");
   return buf;
}

char *drawmove( int min, int max )
{
   static char buf[MAX_STRING_LENGTH];
   int per;
   per = ( ( min * 100 ) / max );
   if( per == 100 )
      sprintf( buf, "&G&W[&R|||&Y||&G||&W]" );
   else if( per >= 90 && per < 100 )
      sprintf( buf, "&G&W[&R|||&Y||&G|&G-&W]" );
   else if( per >= 80 && per < 90 )
      sprintf( buf, "&G&W[&R|||&Y||&G&G--&W]" );
   else if( per >= 70 && per < 80 )
      sprintf( buf, "&G&W[&R|||&Y|&G&G---&W]" );
   else if( per >= 60 && per < 70 )
      sprintf( buf, "&G&W[&R|||&Y&G&G----&W]" );
   else if( per >= 50 && per < 60 )
      sprintf( buf, "&G&W[&R||&Y&G&G-----&W]" );
   else if( per >= 40 && per < 50 )
      sprintf( buf, "&G&W[&R|&Y&G&G------&W]" );
   else if( per >= 30 && per < 40 )
      sprintf( buf, "&G&W[&R|&G&Y&G------&W]" );
   else if( per >= 20 && per < 40 )
      sprintf( buf, "&G&W[&R|&G&Y&G------&W]" );
   else if( per >= 10 && per < 40 )
      sprintf( buf, "&G&W[&R|&G&Y&G------&W]" );
   else if( per >= 0 && per < 10 )
      sprintf( buf, "&G&W[&R&G&Y&G-------&W]" );
   else
      sprintf( buf, "&G&W[&R&W]" );
   return buf;
}

char *drawalign( int align )
{
   static char buf[MAX_STRING_LENGTH];
   if( align >= 100 )
      sprintf( buf, "&W[&C============&W|&W]" );
   else if( align >= 90 && align < 100 )
      sprintf( buf, "&W[&C===========&W|&C=&W]" );
   else if( align >= 60 && align < 90 )
      sprintf( buf, "&W[&C==========&W|&C==&W]" );
   else if( align >= 40 && align < 60 )
      sprintf( buf, "&W[&C=========&W|&C===&W]" );
   else if( align >= 20 && align < 40 )
      sprintf( buf, "&W[&C========&W|&C====&W]" );
   else if( align >= 10 && align < 20 )
      sprintf( buf, "&W[&C=======&W|&C=====&W]" );
   else if( align >= 0 && align < 10 )
      sprintf( buf, "&W[&C======&W|&C======&W]" );
   else if( align <= -1 && align > -20 )
      sprintf( buf, "&W[&C=====&W|&C=======&W]" );
   else if( align <= -20 && align > -40 )
      sprintf( buf, "&W[&C====&W|&C========&W]" );
   else if( align <= -60 && align > -80 )
      sprintf( buf, "&W[&C===&W|&C=========&W]" );
   else if( align <= -80 && align > -100 )
      sprintf( buf, "&W[&C==&W|&C==========&W]" );
   else if( align <= -100 )
      sprintf( buf, "&W[&W|&C============&W]" );

   return buf;
}

/* 
  New Score by Goku
*/
void do_score( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   int ability;
   if( !argument || argument[0] == '\0' )
      victim = ch;
   else if( IS_IMMORTAL( ch ) )
   {
      if( ( victim = get_char_world( ch, argument ) ) == NULL )
      {
         send_to_char( "Victim not found.\r\n", ch );
         return;
      }
      if( IS_SET( victim->act, PLR_WIZINVIS ) && victim->pcdata->wizinvis > ch->top_level )
      {
         send_to_char( "Victim not found.\r\n", ch );
         return;
      }
   }
   else
      victim = ch;
   if( IS_NPC( victim ) )
   {
      do_oldscore( ch, argument );
      return;
   }

   send_to_char( "&z.----------------------------------------------------------.&W\r\n", ch );
   send_to_char( "&z|  &cImperial DataSheet                                      &z|&W\r\n", ch );
   send_to_char( "&z|                                                          |&W\r\n", ch );
   send_to_char( "&z|  &cData File: 19049-SWFotE-3940305                         &z|&W\r\n", ch );
   ch_printf( ch, "&z|  &cName:  &g%-25.25s          &cStrength: &g%-2d  &z|\r\n", victim->name, get_curr_str( victim ) );
   if( victim->pcdata->clan )
      ch_printf( ch, "&z|  &cClan:  &g%-25.25s         &cDexterity: &g%-2d  &z|\r\n", victim->pcdata->clan->name,
                 get_curr_dex( victim ) );
   else
      ch_printf( ch, "&z|  &cClan:  &g%-25.25s         &cDexterity: &g%-2d  &z|\r\n", "None", get_curr_dex( victim ) );
   ch_printf( ch, "&z|  &cRace:  &g%-25.25s      &cConstitution: &g%-2d  &z|\r\n", capitalize( get_race( victim ) ),
              get_curr_con( victim ) );
   ch_printf( ch, "&z|  &cArmor: &g%-25d      &cIntelligence: &g%-2d  &z|\r\n", GET_AC( victim ), get_curr_int( victim ) );
   ch_printf( ch, "&z|  &cHP:    %s  &cMove: %s     &cWisdom: &g%-2d  &z|\r\n", drawlife( victim->hit, victim->max_hit ),
              drawmove( victim->move, victim->max_move ), get_curr_wis( victim ) );
   ch_printf( ch, "&z|  &cAlign: %s                    &cCharisma: &g%-2d  &z|\r\n", drawalign( victim->alignment ),
              get_curr_cha( victim ) );
   send_to_char( "&z|----------------------------------------------------------|&W\r\n", ch );
   ch_printf( ch, "&z|  &cCredits: &g%-10d             &cSavings: &g%-10d     &z|&W\r\n", victim->gold,
              victim->pcdata->bank );
   ch_printf( ch, "&z|  &cWeight:  %s        &cItems: %s  &z|&W\r\n",
              drawlife( victim->carry_weight, can_carry_w( victim ) ), drawlife( victim->carry_number,
                                                                                 can_carry_n( victim ) ) );
   send_to_char( "&z|----------------------------------------------------------|&W\r\n", ch );
   for( ability = 0; ability < MAX_ABILITY; ability++ )
      if( ability != FORCE_ABILITY )
         ch_printf( ch, "&z| &W%s&c%-15s   Level: &g%-3d   &cof  &g%-3d   &cExp: &g%-10ld&z|\r\n",
                    victim->main_ability == ability ? "+" : victim->secondary_ability == ability ? "-" : " ",
                    ability_name[ability], victim->skill_level[ability], max_level( victim, ability ),
                    victim->experience[ability] );
   send_to_char( "&z|----------------------------------------------------------|&W\r\n", ch );
   send_to_char( "&z|  &W+&c = Primary Ability, &R&W-&c = Secondary Ability            &z  |\r\n", ch );
   send_to_char( "&z|----------------------------------------------------------|&W\r\n", ch );
   ch_printf( ch, "&R&z| &cCurrent Comm Frequency: &g%-9s     &cGood RP Points: &g%d  &z|&W\r\n", victim->comfreq,
              victim->rppoints ? victim->rppoints : 0 );
   send_to_char( "&z|----------------------------------------------------------|&W\r\n", ch );
   send_to_char( "&z|  &cFor more Information see lang, aff, group               &z|\r\n", ch );
   send_to_char( "&z+----------------------------------------------------------+&W\r\n", ch );

}

/*
 * Return ascii name of an affect location.
 */
const char *tiny_affect_loc_name( int location )
{
   switch ( location )
   {
      case APPLY_NONE:
         return "NIL";
      case APPLY_STR:
         return " STR  ";
      case APPLY_DEX:
         return " DEX  ";
      case APPLY_INT:
         return " INT  ";
      case APPLY_WIS:
         return " WIS  ";
      case APPLY_CON:
         return " CON  ";
      case APPLY_CHA:
         return " CHA  ";
      case APPLY_LCK:
         return " LCK  ";
      case APPLY_SEX:
         return " SEX  ";
      case APPLY_LEVEL:
         return " LVL  ";
      case APPLY_AGE:
         return " AGE  ";
      case APPLY_MANA:
         return " MANA ";
      case APPLY_HIT:
         return " HV   ";
      case APPLY_MOVE:
         return " MOVE ";
      case APPLY_GOLD:
         return " GOLD ";
      case APPLY_EXP:
         return " EXP  ";
      case APPLY_AC:
         return " AC   ";
      case APPLY_HITROLL:
         return " HITRL";
      case APPLY_DAMROLL:
         return " DAMRL";
      case APPLY_SAVING_POISON:
         return "SV POI";
      case APPLY_SAVING_ROD:
         return "SV ROD";
      case APPLY_SAVING_PARA:
         return "SV PARA";
      case APPLY_SAVING_BREATH:
         return "SV BRTH";
      case APPLY_SAVING_SPELL:
         return "SV SPLL";
      case APPLY_HEIGHT:
         return "HEIGHT";
      case APPLY_WEIGHT:
         return "WEIGHT";
      case APPLY_AFFECT:
         return "AFF BY";
      case APPLY_RESISTANT:
         return "RESIST";
      case APPLY_IMMUNE:
         return "IMMUNE";
      case APPLY_SUSCEPTIBLE:
         return "SUSCEPT";
      case APPLY_WEAPONSPELL:
         return " WEAPON";
      case APPLY_BACKSTAB:
         return "BACKSTB";
      case APPLY_PICK:
         return " PICK  ";
      case APPLY_TRACK:
         return " TRACK ";
      case APPLY_STEAL:
         return " STEAL ";
      case APPLY_SNEAK:
         return " SNEAK ";
      case APPLY_HIDE:
         return " HIDE  ";
      case APPLY_PALM:
         return " PALM  ";
      case APPLY_DETRAP:
         return " DETRAP";
      case APPLY_DODGE:
         return " DODGE ";
      case APPLY_PEEK:
         return " PEEK  ";
      case APPLY_SCAN:
         return " SCAN  ";
      case APPLY_GOUGE:
         return " GOUGE ";
      case APPLY_SEARCH:
         return " SEARCH";
      case APPLY_MOUNT:
         return " MOUNT ";
      case APPLY_DISARM:
         return " DISARM";
      case APPLY_KICK:
         return " KICK  ";
      case APPLY_PARRY:
         return " PARRY ";
      case APPLY_BASH:
         return " BASH  ";
      case APPLY_STUN:
         return " STUN  ";
      case APPLY_PUNCH:
         return " PUNCH ";
      case APPLY_CLIMB:
         return " CLIMB ";
      case APPLY_GRIP:
         return " GRIP  ";
      case APPLY_SCRIBE:
         return " SCRIBE";
      case APPLY_COVER_TRAIL:
         return " COVER ";
      case APPLY_WEARSPELL:
         return " WEAR  ";
      case APPLY_REMOVESPELL:
         return " REMOVE";
      case APPLY_EMOTION:
         return "EMOTION";
      case APPLY_MENTALSTATE:
         return " MENTAL";
      case APPLY_STRIPSN:
         return " DISPEL";
      case APPLY_REMOVE:
         return " REMOVE";
      case APPLY_DIG:
         return " DIG   ";
      case APPLY_FULL:
         return " HUNGER";
      case APPLY_THIRST:
         return " THIRST";
      case APPLY_DRUNK:
         return " DRUNK ";
      case APPLY_BLOOD:
         return " BLOOD ";
   }

   bug( "Affect_location_name: unknown location %d.", location );
   return "(? ? ?)";
}

const char *get_race( CHAR_DATA * ch )
{
   if( ch->race < MAX_NPC_RACE && ch->race >= 0 )
      return ( npc_race[ch->race] );
   return ( "Unknown" );
}

void do_oldscore( CHAR_DATA * ch, const char *argument )
{
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   if( IS_AFFECTED( ch, AFF_POSSESS ) )
   {
      send_to_char( "You can't do that in your current state of mind!\r\n", ch );
      return;
   }

   set_char_color( AT_SCORE, ch );
   ch_printf( ch,
              "You are %s%s, level %d, %d years old (%d hours).\r\n",
              ch->name, IS_NPC( ch ) ? "" : ch->pcdata->title, ch->top_level, get_age( ch ), ( get_age( ch ) - 17 ) );

   if( get_trust( ch ) != ch->top_level )
      ch_printf( ch, "You are trusted at level %d.\r\n", get_trust( ch ) );

   if( IS_SET( ch->act, ACT_MOBINVIS ) )
      ch_printf( ch, "You are mobinvis at level %d.\r\n", ch->mobinvis );


   ch_printf( ch, "You have %d/%d hit, %d/%d movement.\r\n", ch->hit, ch->max_hit, ch->move, ch->max_move );

   ch_printf( ch,
              "You are carrying %d/%d items with weight %d/%d kg.\r\n",
              ch->carry_number, can_carry_n( ch ), ch->carry_weight, can_carry_w( ch ) );

   ch_printf( ch,
              "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: ??  Frc: ??\r\n",
              get_curr_str( ch ),
              get_curr_int( ch ), get_curr_wis( ch ), get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ) );

   ch_printf( ch, "You have have %d credits.\r\n", ch->gold );

   if( !IS_NPC( ch ) )
      ch_printf( ch,
                 "You have achieved %d glory during your life, and currently have %d.\r\n",
                 ch->pcdata->quest_accum, ch->pcdata->quest_curr );

   ch_printf( ch,
              "Autoexit: %s   Autoloot: %s   Autosac: %s   Autocred: %s\r\n",
              ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) ) ? "yes" : "no",
              ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOLOOT ) ) ? "yes" : "no",
              ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOSAC ) ) ? "yes" : "no",
              ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOGOLD ) ) ? "yes" : "no" );

   ch_printf( ch, "Wimpy set to %d hit points.\r\n", ch->wimpy );

   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
      send_to_char( "You are drunk.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
      send_to_char( "You are thirsty.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
      send_to_char( "You are hungry.\r\n", ch );

   switch ( ch->mental_state / 10 )
   {
      default:
         send_to_char( "You're completely messed up!\r\n", ch );
         break;
      case -10:
         send_to_char( "You're barely conscious.\r\n", ch );
         break;
      case -9:
         send_to_char( "You can barely keep your eyes open.\r\n", ch );
         break;
      case -8:
         send_to_char( "You're extremely drowsy.\r\n", ch );
         break;
      case -7:
         send_to_char( "You feel very unmotivated.\r\n", ch );
         break;
      case -6:
         send_to_char( "You feel sedated.\r\n", ch );
         break;
      case -5:
         send_to_char( "You feel sleepy.\r\n", ch );
         break;
      case -4:
         send_to_char( "You feel tired.\r\n", ch );
         break;
      case -3:
         send_to_char( "You could use a rest.\r\n", ch );
         break;
      case -2:
         send_to_char( "You feel a little under the weather.\r\n", ch );
         break;
      case -1:
         send_to_char( "You feel fine.\r\n", ch );
         break;
      case 0:
         send_to_char( "You feel great.\r\n", ch );
         break;
      case 1:
         send_to_char( "You feel energetic.\r\n", ch );
         break;
      case 2:
         send_to_char( "Your mind is racing.\r\n", ch );
         break;
      case 3:
         send_to_char( "You can't think straight.\r\n", ch );
         break;
      case 4:
         send_to_char( "Your mind is going 100 miles an hour.\r\n", ch );
         break;
      case 5:
         send_to_char( "You're high as a kite.\r\n", ch );
         break;
      case 6:
         send_to_char( "Your mind and body are slipping appart.\r\n", ch );
         break;
      case 7:
         send_to_char( "Reality is slipping away.\r\n", ch );
         break;
      case 8:
         send_to_char( "You have no idea what is real, and what is not.\r\n", ch );
         break;
      case 9:
         send_to_char( "You feel immortal.\r\n", ch );
         break;
      case 10:
         send_to_char( "You are a Supreme Entity.\r\n", ch );
         break;
   }

   switch ( ch->position )
   {
      case POS_DEAD:
         send_to_char( "You are DEAD!!\r\n", ch );
         break;
      case POS_MORTAL:
         send_to_char( "You are mortally wounded.\r\n", ch );
         break;
      case POS_INCAP:
         send_to_char( "You are incapacitated.\r\n", ch );
         break;
      case POS_STUNNED:
         send_to_char( "You are stunned.\r\n", ch );
         break;
      case POS_SLEEPING:
         send_to_char( "You are sleeping.\r\n", ch );
         break;
      case POS_RESTING:
         send_to_char( "You are resting.\r\n", ch );
         break;
      case POS_STANDING:
         send_to_char( "You are standing.\r\n", ch );
         break;
      case POS_FIGHTING:
         send_to_char( "You are fighting.\r\n", ch );
         break;
      case POS_MOUNTED:
         send_to_char( "Mounted.\r\n", ch );
         break;
      case POS_SHOVE:
         send_to_char( "Being shoved.\r\n", ch );
         break;
      case POS_DRAG:
         send_to_char( "Being dragged.\r\n", ch );
         break;
   }

   if( ch->top_level >= 25 )
      ch_printf( ch, "AC: %d.  ", GET_AC( ch ) );

   send_to_char( "You are ", ch );
   if( GET_AC( ch ) >= 101 )
      send_to_char( "WORSE than naked!\r\n", ch );
   else if( GET_AC( ch ) >= 80 )
      send_to_char( "naked.\r\n", ch );
   else if( GET_AC( ch ) >= 60 )
      send_to_char( "wearing clothes.\r\n", ch );
   else if( GET_AC( ch ) >= 40 )
      send_to_char( "slightly armored.\r\n", ch );
   else if( GET_AC( ch ) >= 20 )
      send_to_char( "somewhat armored.\r\n", ch );
   else if( GET_AC( ch ) >= 0 )
      send_to_char( "armored.\r\n", ch );
   else if( GET_AC( ch ) >= -20 )
      send_to_char( "well armored.\r\n", ch );
   else if( GET_AC( ch ) >= -40 )
      send_to_char( "strongly armored.\r\n", ch );
   else if( GET_AC( ch ) >= -60 )
      send_to_char( "heavily armored.\r\n", ch );
   else if( GET_AC( ch ) >= -80 )
      send_to_char( "superbly armored.\r\n", ch );
   else if( GET_AC( ch ) >= -100 )
      send_to_char( "divinely armored.\r\n", ch );
   else
      send_to_char( "invincible!\r\n", ch );

   if( ch->top_level >= 15 )
      ch_printf( ch, "Hitroll: %d  Damroll: %d.\r\n", GET_HITROLL( ch ), GET_DAMROLL( ch ) );

   if( ch->top_level >= 10 )
      ch_printf( ch, "Alignment: %d.  ", ch->alignment );

   send_to_char( "You are ", ch );
   if( ch->alignment > 900 )
      send_to_char( "angelic.\r\n", ch );
   else if( ch->alignment > 700 )
      send_to_char( "saintly.\r\n", ch );
   else if( ch->alignment > 350 )
      send_to_char( "good.\r\n", ch );
   else if( ch->alignment > 100 )
      send_to_char( "kind.\r\n", ch );
   else if( ch->alignment > -100 )
      send_to_char( "neutral.\r\n", ch );
   else if( ch->alignment > -350 )
      send_to_char( "mean.\r\n", ch );
   else if( ch->alignment > -700 )
      send_to_char( "evil.\r\n", ch );
   else if( ch->alignment > -900 )
      send_to_char( "demonic.\r\n", ch );
   else
      send_to_char( "satanic.\r\n", ch );

   if( ch->first_affect )
   {
      send_to_char( "You are affected by:\r\n", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
         if( ( skill = get_skilltype( paf->type ) ) != NULL )
         {
            ch_printf( ch, "Spell: '%s'", skill->name );

            if( ch->top_level >= 20 )
               ch_printf( ch,
                          " modifies %s by %d for %d rounds",
                          affect_loc_name( paf->location ), paf->modifier, paf->duration );

            send_to_char( ".\r\n", ch );
         }
   }

   if( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
   {
      ch_printf( ch, "WizInvis level: %d   WizInvis is %s\r\n",
                 ch->pcdata->wizinvis, IS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
      if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
         ch_printf( ch, "Room Range: %d - %d\r\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi );
      if( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
         ch_printf( ch, "Obj Range : %d - %d\r\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi );
      if( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
         ch_printf( ch, "Mob Range : %d - %d\r\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi );
   }

   return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA * ch, const char *argument )
{
   int ability;

   for( ability = 0; ability < MAX_ABILITY; ability++ )
      if( ability != FORCE_ABILITY )
         ch_printf( ch, "&G%-15s Level&W: %-3d    &GMax&W: %-3d    &GExp&W: %-10ld     &GNext&W: %-10ld&W\r\n",
                    ability_name[ability], ch->skill_level[ability], max_level( ch, ability ), ch->experience[ability],
                    exp_level( ch->skill_level[ability] + 1 ) );
}


void do_affected( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "by" ) )
   {
      set_char_color( AT_BLUE, ch );
      send_to_char( "\r\nImbued with:\r\n", ch );
      set_char_color( AT_SCORE, ch );
      ch_printf( ch, "%s\r\n", affect_bit_name( ch->affected_by ) );
      if( ch->top_level >= 20 )
      {
         send_to_char( "\r\n", ch );
         if( ch->resistant > 0 )
         {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Resistances:  ", ch );
            set_char_color( AT_SCORE, ch );
            ch_printf( ch, "%s\r\n", flag_string( ch->resistant, ris_flags ) );
         }
         if( ch->immune > 0 )
         {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Immunities:   ", ch );
            set_char_color( AT_SCORE, ch );
            ch_printf( ch, "%s\r\n", flag_string( ch->immune, ris_flags ) );
         }
         if( ch->susceptible > 0 )
         {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Suscepts:     ", ch );
            set_char_color( AT_SCORE, ch );
            ch_printf( ch, "%s\r\n", flag_string( ch->susceptible, ris_flags ) );
         }
      }
      return;
   }

   if( !ch->first_affect )
   {
      set_char_color( AT_SCORE, ch );
      send_to_char( "\r\nNo cantrip or skill affects you.\r\n", ch );
   }
   else
   {
      send_to_char( "\r\n", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
         if( ( skill = get_skilltype( paf->type ) ) != NULL )
         {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Affected:  ", ch );
            set_char_color( AT_SCORE, ch );
            if( ch->top_level >= 20 )
            {
               if( paf->duration < 25 )
                  set_char_color( AT_WHITE, ch );
               if( paf->duration < 6 )
                  set_char_color( AT_WHITE + AT_BLINK, ch );
               ch_printf( ch, "(%5d)   ", paf->duration );
            }
            ch_printf( ch, "%-18s\r\n", skill->name );
         }
   }

   if( IS_IMMORTAL( ch ) && ch->wait_state )
      ch_printf( ch, "Your wait_state: %d", ch->wait_state );
   ch_printf( ch, "See also: aff by\r\n" );

   return;
}

void do_inventory( CHAR_DATA * ch, const char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "You are carrying:\r\n", ch );
   show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
   return;
}


void do_equipment( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;
   int iWear, dam;
   bool found;
   char buf[MAX_STRING_LENGTH];

   set_char_color( AT_RED, ch );
   send_to_char( "You are using:\r\n", ch );
   found = FALSE;
   set_char_color( AT_OBJECT, ch );
   for( iWear = 0; iWear < MAX_WEAR; iWear++ )
   {
      for( obj = ch->first_carrying; obj; obj = obj->next_content )
         if( obj->wear_loc == iWear )
         {
            send_to_char( where_name[iWear], ch );
            if( can_see_obj( ch, obj ) )
            {
               send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
               strcpy( buf, "" );
               switch ( obj->item_type )
               {
                  default:
                     break;

                  case ITEM_ARMOR:
                     if( obj->value[1] == 0 )
                        obj->value[1] = obj->value[0];
                     if( obj->value[1] == 0 )
                        obj->value[1] = 1;
                     dam = ( short )( ( obj->value[0] * 10 ) / obj->value[1] );
                     if( dam >= 10 )
                        strcat( buf, " (superb) " );
                     else if( dam >= 7 )
                        strcat( buf, " (good) " );
                     else if( dam >= 5 )
                        strcat( buf, " (worn) " );
                     else if( dam >= 3 )
                        strcat( buf, " (poor) " );
                     else if( dam >= 1 )
                        strcat( buf, " (awful) " );
                     else if( dam == 0 )
                        strcat( buf, " (broken) " );
                     send_to_char( buf, ch );
                     break;

                  case ITEM_WEAPON:
                     dam = INIT_WEAPON_CONDITION - obj->value[0];
                     if( dam < 2 )
                        strcat( buf, " (superb) " );
                     else if( dam < 4 )
                        strcat( buf, " (good) " );
                     else if( dam < 7 )
                        strcat( buf, " (worn) " );
                     else if( dam < 10 )
                        strcat( buf, " (poor) " );
                     else if( dam < 12 )
                        strcat( buf, " (awful) " );
                     else if( dam == 12 )
                        strcat( buf, " (broken) " );
                     send_to_char( buf, ch );
                     
                     break;
               }
               send_to_char( "\r\n", ch );
            }
            else
               send_to_char( "something.\r\n", ch );
            found = TRUE;
         }
   }

   if( !found )
      send_to_char( "Nothing.\r\n", ch );

   return;
}



void set_title( CHAR_DATA * ch, const char *title )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      bug( "Set_title: NPC.", 0 );
      return;
   }

   if( isalpha( title[0] ) || isdigit( title[0] ) )
   {
      buf[0] = ' ';
      strcpy( buf + 1, title );
   }
   else
      strcpy( buf, title );

   STRFREE( ch->pcdata->title );
   ch->pcdata->title = STRALLOC( buf );
   return;
}



void do_title( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      send_to_char( "You try but the Force resists you.\r\n", ch );
      return;
   }


   if( argument[0] == '\0' )
   {
      send_to_char( "Change your title to what?\r\n", ch );
      return;
   }

   if( ( get_trust( ch ) <= LEVEL_IMMORTAL ) && ( !nifty_is_name( ch->name, remand( argument ) ) ) )
   {
      send_to_char( "You must include your name somewhere in your title!", ch );
      return;
   }

   smash_tilde( argument );
   set_title( ch, argument );
   send_to_char( "Ok.\r\n", ch );
}

void do_email( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->email )
         ch->pcdata->email = str_dup( "" );
      ch_printf( ch, "Your email address is: %s\r\n", show_tilde( ch->pcdata->email ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->email )
         DISPOSE( ch->pcdata->email );
      ch->pcdata->email = str_dup( "" );

      send_to_char( "Email address cleared.\r\n", ch );
      return;
   }

   strcpy( buf, argument );

   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->email )
      DISPOSE( ch->pcdata->email );
   ch->pcdata->email = str_dup( buf );
   send_to_char( "Email address set.\r\n", ch );
}

void do_screenname( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->screenname )
         ch->pcdata->screenname = str_dup( "" );
      ch_printf( ch, "Your AIM screenname is: %s\r\n", show_tilde( ch->pcdata->screenname ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->screenname )
         DISPOSE( ch->pcdata->screenname );
      ch->pcdata->screenname = str_dup( "" );

      send_to_char( "AIM Screnname cleared.\r\n", ch );
      return;
   }

   strcpy( buf, argument );

   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->screenname )
      DISPOSE( ch->pcdata->screenname );
   ch->pcdata->screenname = str_dup( buf );
   send_to_char( "AIM Screnname set.\r\n", ch );
}

void do_homepage( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->homepage )
         ch->pcdata->homepage = str_dup( "" );
      ch_printf( ch, "Your homepage is: %s\r\n", show_tilde( ch->pcdata->homepage ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->homepage )
         DISPOSE( ch->pcdata->homepage );
      ch->pcdata->homepage = str_dup( "" );
      send_to_char( "Homepage cleared.\r\n", ch );
      return;
   }

   if( strstr( argument, "://" ) )
      strcpy( buf, argument );
   else
      sprintf( buf, "http://%s", argument );
   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->homepage )
      DISPOSE( ch->pcdata->homepage );
   ch->pcdata->homepage = str_dup( buf );
   send_to_char( "Homepage set.\r\n", ch );
}

void do_wwwimage( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->image )
         ch->pcdata->image = str_dup( "" );
      ch_printf( ch, "Your www image is: %s\r\n", show_tilde( ch->pcdata->image ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->image )
         DISPOSE( ch->pcdata->image );
      ch->pcdata->image = str_dup( "" );
      send_to_char( "WWW Image cleared.\r\n", ch );
      return;
   }

   if( strstr( argument, "://" ) )
      strcpy( buf, argument );
   else
      sprintf( buf, "http://%s", argument );
   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->image )
      DISPOSE( ch->pcdata->image );
   ch->pcdata->image = str_dup( buf );
   send_to_char( "WWW Image set.\r\n", ch );
}


/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Monsters are too dumb to do that!\r\n", ch );
      return;
   }

   if( !ch->desc )
   {
      bug( "do_description: no descriptor", 0 );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "do_description: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_DESC;
         ch->dest_buf = ch;
         start_editing( ch, ch->description );
         editor_desc_printf( ch, "Your description (%s)", ch->name );
         return;

      case SUB_PERSONAL_DESC:
         STRFREE( ch->description );
         ch->description = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs can't set bio's!\r\n", ch );
      return;
   }

   if( !ch->desc )
   {
      bug( "do_bio: no descriptor", 0 );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "do_bio: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_BIO;
         ch->dest_buf = ch;
         start_editing( ch, ch->pcdata->bio );
         editor_desc_printf( ch, "Your bio (%s).", ch->name );
         return;

      case SUB_PERSONAL_BIO:
         STRFREE( ch->pcdata->bio );
         ch->pcdata->bio = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}



void do_report( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_INPUT_LENGTH];

   if( IS_AFFECTED( ch, AFF_POSSESS ) )
   {
      send_to_char( "You can't do that in your current state of mind!\r\n", ch );
      return;
   }


   ch_printf( ch, "You report: %d/%d hp %d/%d mv.\r\n", ch->hit, ch->max_hit, ch->move, ch->max_move );


   sprintf( buf, "$n reports: %d/%d hp %d/%d.", ch->hit, ch->max_hit, ch->move, ch->max_move );

   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   return;
}

void do_prompt( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char argbuf[MIL];

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\r\n", ch );
      return;
   }
   smash_tilde( argument );
   one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Set prompt to what? (try help prompt)\r\n", ch );
      return;
   }
   if( ch->pcdata->prompt )
      STRFREE( ch->pcdata->prompt );

   snprintf( argbuf, MIL, "%s", argument );
   if( strlen( argbuf ) > 128 )
      argbuf[128] = '\0';

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->prompt = STRALLOC( "" );
   else
      ch->pcdata->prompt = STRALLOC( argbuf );
   send_to_char( "Ok.\r\n", ch );
   return;
}
