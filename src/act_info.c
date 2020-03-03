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
#include <unistd.h>
#include "mud.h"
#include "sha256.h"

/*
 * Keep players from defeating examine progs -Druid
 * False = do not trigger
 * True = Trigger
 */
bool EXA_prog_trigger = TRUE;

ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA * in_room, EXIT_DATA ** pexit );

const char *const where_name[] = {
   "&G&b[&wused as light    &b]&G&w ",
   "&G&b[&wworn on finger   &b]&G&w ",
   "&G&b[&wworn on finger   &b]&G&w ",
   "&G&b[&wworn around neck &b]&G&w ",
   "&G&b[&wworn around neck &b]&G&w ",
   "&G&b[&wworn on body     &b]&G&w ",
   "&G&b[&wworn on head     &b]&G&w ",
   "&G&b[&wworn on legs     &b]&G&w ",
   "&G&b[&wworn on feet     &b]&G&w ",
   "&G&b[&wworn on hands    &b]&G&w ",
   "&G&b[&wworn on arms     &b]&G&w ",
   "&G&b[&wenergy shield    &b]&G&w ",
   "&G&b[&wworn about body  &b]&G&w ",
   "&G&b[&wworn about waist &b]&G&w ",
   "&G&b[&wworn around wrist&b]&G&w ",
   "&G&b[&wworn around wrist&b]&G&w ",
   "&G&b[&wwielded          &b]&G&w ",
   "&G&b[&wheld             &b]&G&w ",
   "&G&b[&wdual wielded     &b]&G&w ",
   "&G&b[&wworn on ears     &b]&G&w ",
   "&G&b[&wworn on eyes     &b]&G&w ",
   "&G&b[&wmissile wielded  &b]&G&w ",
   "&G&b[&wworn on back     &b]&G&w ",
   "&G&b[&wleft holster     &b]&G&w ",
   "&G&b[&wright holster    &b]&G&w ",
   "&G&b[&wworn both wrists &b]&G&w "
};

/*
 * Local functions.
 */
void show_char_to_char_0( CHAR_DATA * victim, CHAR_DATA * ch );
void show_char_to_char_1( CHAR_DATA * victim, CHAR_DATA * ch );
void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch );
void show_ships_to_char( SHIP_DATA * ship, CHAR_DATA * ch );
bool check_blind( CHAR_DATA * ch );
void show_condition( CHAR_DATA * ch, CHAR_DATA * victim );

char *format_obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort )
{
   static char buf[MAX_STRING_LENGTH];

   buf[0] = '\0';
   if( IS_OBJ_STAT( obj, ITEM_INVIS ) )
      strcat( buf, "(Invis) " );
   if( ( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) || IS_IMMORTAL( ch ) ) && IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      strcat( buf, "&B(Blue Aura)&w " );
   if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
      strcat( buf, "&G&W(&OG&Yl&wo&Ww&wi&Yn&Og&G&W) " );
   if( IS_OBJ_STAT( obj, ITEM_HUM ) )
      strcat( buf, "&G&W(&gH&Gu&wm&Wm&wi&Gn&gg&G&W) " );
   if( IS_OBJ_STAT( obj, ITEM_HIDDEN ) )
      strcat( buf, "(Hidden) " );
   if( IS_OBJ_STAT( obj, ITEM_BURRIED ) )
      strcat( buf, "(Burried) " );
   if( IS_IMMORTAL( ch ) && IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      strcat( buf, "(PROTO) " );
   if( ( IS_AFFECTED( ch, AFF_DETECTTRAPS ) || IS_SET( ch->act, PLR_HOLYLIGHT ) ) && is_trapped( obj ) )
      strcat( buf, "(Trap) " );

   if( fShort )
   {
      if( obj->short_descr )
         strcat( buf, obj->short_descr );
   }
   else
   {
      if( obj->description )
         strcat( buf, obj->description );
   }

   return buf;
}


/*
 * Some increasingly freaky halucinated objects		-Thoric
 */
const char *halucinated_object( int ms, bool fShort )
{
   int sms = URANGE( 1, ( ms + 10 ) / 5, 20 );

   if( fShort )
      switch ( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
      {
         case 1:
            return "a sword";
         case 2:
            return "a stick";
         case 3:
            return "something shiny";
         case 4:
            return "something";
         case 5:
            return "something interesting";
         case 6:
            return "something colorful";
         case 7:
            return "something that looks cool";
         case 8:
            return "a nifty thing";
         case 9:
            return "a cloak of flowing colors";
         case 10:
            return "a mystical flaming sword";
         case 11:
            return "a swarm of insects";
         case 12:
            return "a deathbane";
         case 13:
            return "a figment of your imagination";
         case 14:
            return "your gravestone";
         case 15:
            return "the long lost boots of Ranger Thoric";
         case 16:
            return "a glowing tome of arcane knowledge";
         case 17:
            return "a long sought secret";
         case 18:
            return "the meaning of it all";
         case 19:
            return "the answer";
         case 20:
            return "the key to life, the universe and everything";
      }
   switch ( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
   {
      case 1:
         return "A nice looking sword catches your eye.";
      case 2:
         return "The ground is covered in small sticks.";
      case 3:
         return "Something shiny catches your eye.";
      case 4:
         return "Something catches your attention.";
      case 5:
         return "Something interesting catches your eye.";
      case 6:
         return "Something colorful flows by.";
      case 7:
         return "Something that looks cool calls out to you.";
      case 8:
         return "A nifty thing of great importance stands here.";
      case 9:
         return "A cloak of flowing colors asks you to wear it.";
      case 10:
         return "A mystical flaming sword awaits your grasp.";
      case 11:
         return "A swarm of insects buzzes in your face!";
      case 12:
         return "The extremely rare Deathbane lies at your feet.";
      case 13:
         return "A figment of your imagination is at your command.";
      case 14:
         return "You notice a gravestone here... upon closer examination, it reads your name.";
      case 15:
         return "The long lost boots of Ranger Thoric lie off to the side.";
      case 16:
         return "A glowing tome of arcane knowledge hovers in the air before you.";
      case 17:
         return "A long sought secret of all mankind is now clear to you.";
      case 18:
         return "The meaning of it all, so simple, so clear... of course!";
      case 19:
         return "The answer.  One.  It's always been One.";
      case 20:
         return "The key to life, the universe and everything awaits your hand.";
   }
   return "Whoa!!!";
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing )
{
   char **prgpstrShow;
   int *prgnShow;
   int *pitShow;
   char *pstrShow;
   OBJ_DATA *obj;
   int nShow;
   int iShow;
   int count, offcount, tmp, ms, cnt;
   bool fCombine;

   if( !ch->desc )
      return;

   /*
    * if there's no list... then don't do all this crap!  -Thoric
    */
   if( !list )
   {
      if( fShowNothing )
      {
         if( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
            send_to_char( "     ", ch );
         send_to_char( "Nothing.\r\n", ch );
      }
      return;
   }
   /*
    * Alloc space for output lines.
    */
   count = 0;
   for( obj = list; obj; obj = obj->next_content )
      count++;

   ms = ( ch->mental_state ? ch->mental_state : 1 )
      * ( IS_NPC( ch ) ? 1 : ( ch->pcdata->condition[COND_DRUNK] ? ( ch->pcdata->condition[COND_DRUNK] / 12 ) : 1 ) );

   /*
    * If not mentally stable...
    */
   if( abs( ms ) > 40 )
   {
      offcount = URANGE( -( count ), ( count * ms ) / 100, count * 2 );
      if( offcount < 0 )
         offcount += number_range( 0, abs( offcount ) );
      else if( offcount > 0 )
         offcount -= number_range( 0, offcount );
   }
   else
      offcount = 0;

   if( count + offcount <= 0 )
   {
      if( fShowNothing )
      {
         if( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
            send_to_char( "     ", ch );
         send_to_char( "Nothing.\r\n", ch );
      }
      return;
   }

   CREATE( prgpstrShow, char *, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   CREATE( prgnShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   CREATE( pitShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   nShow = 0;
   tmp = ( offcount > 0 ) ? offcount : 0;
   cnt = 0;

   /*
    * Format the list of objects.
    */
   for( obj = list; obj; obj = obj->next_content )
   {
      if( offcount < 0 && ++cnt > ( count + offcount ) )
         break;
      if( tmp > 0 && number_bits( 1 ) == 0 )
      {
         prgpstrShow[nShow] = str_dup( halucinated_object( ms, fShort ) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
         nShow++;
         --tmp;
      }
      if( obj->wear_loc == WEAR_NONE
          && can_see_obj( ch, obj ) && ( obj->item_type != ITEM_TRAP || IS_AFFECTED( ch, AFF_DETECTTRAPS ) ) )
      {
         pstrShow = format_obj_to_char( obj, ch, fShort );
         fCombine = FALSE;

         if( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
         {
            /*
             * Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwords.
             */
            for( iShow = nShow - 1; iShow >= 0; iShow-- )
            {
               if( !strcmp( prgpstrShow[iShow], pstrShow ) )
               {
                  prgnShow[iShow] += obj->count;
                  fCombine = TRUE;
                  break;
               }
            }
         }

         pitShow[nShow] = obj->item_type;
         /*
          * Couldn't combine, or didn't want to.
          */
         if( !fCombine )
         {
            prgpstrShow[nShow] = str_dup( pstrShow );
            prgnShow[nShow] = obj->count;
            nShow++;
         }
      }
   }
   if( tmp > 0 )
   {
      int x;
      for( x = 0; x < tmp; x++ )
      {
         prgpstrShow[nShow] = str_dup( halucinated_object( ms, fShort ) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
         nShow++;
      }
   }

   /*
    * Output the formatted list.      -Color support by Thoric
    */
   for( iShow = 0; iShow < nShow; iShow++ )
   {
      switch ( pitShow[iShow] )
      {
         default:
            set_char_color( AT_OBJECT, ch );
            break;
         case ITEM_BLOOD:
            set_char_color( AT_BLOOD, ch );
            break;
         case ITEM_MONEY:
         case ITEM_TREASURE:
            set_char_color( AT_YELLOW, ch );
            break;
         case ITEM_FOOD:
            set_char_color( AT_HUNGRY, ch );
            break;
         case ITEM_DRINK_CON:
         case ITEM_FOUNTAIN:
            set_char_color( AT_THIRSTY, ch );
            break;
         case ITEM_FIRE:
            set_char_color( AT_FIRE, ch );
            break;
         case ITEM_SCROLL:
         case ITEM_WAND:
         case ITEM_STAFF:
            set_char_color( AT_MAGIC, ch );
            break;
      }
      if( fShowNothing )
         send_to_char( "     ", ch );
      send_to_char( prgpstrShow[iShow], ch );
/*	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) ) */
      {
         if( prgnShow[iShow] != 1 )
            ch_printf( ch, " (%d)", prgnShow[iShow] );
      }

      send_to_char( "\r\n", ch );
      DISPOSE( prgpstrShow[iShow] );
   }

   if( fShowNothing && nShow == 0 )
   {
      if( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
         send_to_char( "     ", ch );
      send_to_char( "Nothing.\r\n", ch );
   }

   /*
    * Clean up.
    */
   DISPOSE( prgpstrShow );
   DISPOSE( prgnShow );
   DISPOSE( pitShow );
   return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char( CHAR_DATA * victim, CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];

/*    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
        if ( IS_GOOD(victim) )
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "%s glows with an aura of divine radiance.\r\n",
		IS_NPC( victim ) ? capitalize(victim->short_descr) : (victim->name) );
        }
        else if ( IS_EVIL(victim) )
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "%s shimmers beneath an aura of dark energy.\r\n",
		IS_NPC( victim ) ? capitalize(victim->short_descr) : (victim->name) );
        }
        else
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "%s is shrouded in flowing shadow and light.\r\n",
		IS_NPC( victim ) ? capitalize(victim->short_descr) : (victim->name) );
        }
    }*/
   if( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
   {
      set_char_color( AT_FIRE, ch );
      ch_printf( ch, "%s is engulfed within a blaze of mystical flame.\r\n",
                 IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
   }
   if( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
   {
      set_char_color( AT_BLUE, ch );
      ch_printf( ch, "%s is surrounded by cascading torrents of energy.\r\n",
                 IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
   }
/*Scryn 8/13*/
   if( IS_AFFECTED( victim, AFF_ICESHIELD ) )
   {
      set_char_color( AT_LBLUE, ch );
      ch_printf( ch, "%s is ensphered by shards of glistening ice.\r\n",
                 IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
   }
   if( IS_AFFECTED( victim, AFF_CHARM ) )
   {
      set_char_color( AT_MAGIC, ch );
      ch_printf( ch, "%s looks ahead free of expression.\r\n",
                 IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
   }
   if( !IS_NPC( victim ) && !victim->desc && victim->switched && IS_AFFECTED( victim->switched, AFF_POSSESS ) )
   {
      set_char_color( AT_MAGIC, ch );
      strcpy( buf, PERS( victim, ch ) );
      strcat( buf, " appears to be in a deep trance...\r\n" );
   }
}

void show_char_to_char_0( CHAR_DATA * victim, CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char buf1[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   buf[0] = '\0';

   if( IS_NPC( victim ) )
      strcat( buf, " " );

   if( !IS_NPC( victim ) && !victim->desc )
   {
      if( !victim->switched )
         strcat( buf, "(Link Dead) " );
      else if( !IS_AFFECTED( victim->switched, AFF_POSSESS ) )
         strcat( buf, "(Switched) " );
   }
   if( !IS_NPC( victim ) && IS_SET( victim->act, PLR_AFK ) )
      strcat( buf, "[AFK] " );
   if( ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_WIZINVIS ) )
       || ( IS_NPC( victim ) && IS_SET( victim->act, ACT_MOBINVIS ) ) )
   {
      if( !IS_NPC( victim ) )
         sprintf( buf1, "(Invis %d) ", victim->pcdata->wizinvis );
      else
         sprintf( buf1, "(Mobinvis %d) ", victim->mobinvis );
      strcat( buf, buf1 );
   }
   if( IS_AFFECTED( victim, AFF_INVISIBLE ) )
      strcat( buf, "(Invis) " );
   if( IS_AFFECTED( victim, AFF_HIDE ) )
      strcat( buf, "(Hide) " );
   if( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
      strcat( buf, "(Translucent) " );
   if( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
      strcat( buf, "&P(Pink Aura)&w " );
   if( !IS_NPC( victim ) && IS_SET( victim->pcdata->act2, ACT_GAGGED ) )
      strcat( buf, "&w&W(&rG&Ra&rg&Rg&re&Rd&w&W)&w " );
   if( !IS_NPC( victim ) && IS_SET( victim->pcdata->act2, ACT_BOUND ) )
      strcat( buf, "&G&W(&gB&Go&wu&Gn&gd&G&W)&w " );
   if( IS_EVIL( victim ) && IS_AFFECTED( ch, AFF_DETECT_EVIL ) )
      strcat( buf, "&R(Red Aura)&w " );
   if( ( victim->mana > 10 ) && ( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) || IS_IMMORTAL( ch ) ) )
      strcat( buf, "&B(Blue Aura)&w " );
   if( !IS_NPC( victim ) && IS_SET( victim->act, PLR_LITTERBUG ) )
      strcat( buf, "(LITTERBUG) " );
   if( IS_NPC( victim ) && IS_IMMORTAL( ch ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
      strcat( buf, "(PROTO) " );
   if( victim->desc && victim->desc->connected == CON_EDITING )
      strcat( buf, "(Writing) " );

   set_char_color( AT_PERSON, ch );
   if( victim && victim->position == victim->defposition && victim->long_descr[0] != '\0' )
   {
      if( IS_AFFECTED( ch, AFF_INFRARED ) )
      {  // \033[1;31m

         sprintf( buf2, "\033[1;31m" );
         strcat( buf, victim->long_descr );
         strcat( buf2, buf );
         send_to_char_noand( buf2, ch );
         show_visible_affects_to_char( victim, ch );
      }
      else
      {
         strcat( buf, victim->long_descr );
         send_to_pager( buf, ch );
         show_visible_affects_to_char( victim, ch );
      }
      return;
   }

   /*
    * strcat( buf, PERS( victim, ch ) );       old system of titles
    * *    removed to prevent prepending of name to title     -Kuran  
    * *
    * *    But added back bellow so that you can see mobs too :P   -Durga 
    */

   strcat( buf, PERS( victim, ch ) );

   switch ( victim->position )
   {
      case POS_DEAD:
         strcat( buf, " is DEAD!!" );
         break;
      case POS_MORTAL:
         strcat( buf, " is mortally wounded." );
         break;
      case POS_INCAP:
         strcat( buf, " is incapacitated." );
         break;
      case POS_STUNNED:
         strcat( buf, " is lying here stunned." );
         break;
      case POS_SLEEPING:
         if( ch->position == POS_SITTING || ch->position == POS_RESTING )
            strcat( buf, " is sleeping nearby." );
         else
            strcat( buf, " is deep in slumber here." );
         break;
      case POS_RESTING:
         if( ch->position == POS_RESTING )
            strcat( buf, " is sprawled out alongside you." );
         else if( ch->position == POS_MOUNTED )
            strcat( buf, " is sprawled out at the foot of your mount." );
         else
            strcat( buf, " is sprawled out here." );
         break;
      case POS_SITTING:
         if( ch->position == POS_SITTING )
            strcat( buf, " sits here with you." );
         else if( ch->position == POS_RESTING )
            strcat( buf, " sits nearby as you lie around." );
         else
            strcat( buf, " sits upright here." );
         break;
      case POS_STANDING:
         if( IS_IMMORTAL( victim ) )
            strcat( buf, " is here before you." );
         else
            if( ( victim->in_room->sector_type == SECT_UNDERWATER )
                && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
            strcat( buf, " is drowning here." );
         else if( victim->in_room->sector_type == SECT_UNDERWATER )
            strcat( buf, " is here in the water." );
         else
            if( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
                && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
            strcat( buf, " is drowning here." );
         else if( victim->in_room->sector_type == SECT_OCEANFLOOR )
            strcat( buf, " is standing here in the water." );
         else if( IS_AFFECTED( victim, AFF_FLOATING ) || IS_AFFECTED( victim, AFF_FLYING ) )
            strcat( buf, " is hovering here." );
         else
            strcat( buf, " is standing here." );
         break;
      case POS_SHOVE:
         strcat( buf, " is being shoved around." );
         break;
      case POS_DRAG:
         strcat( buf, " is being dragged around." );
         break;
      case POS_MOUNTED:
         strcat( buf, " is here, upon " );
         if( !victim->mount )
            strcat( buf, "thin air???" );
         else if( victim->mount == ch )
            strcat( buf, "your back." );
         else if( victim->in_room == victim->mount->in_room )
         {
            strcat( buf, PERS( victim->mount, ch ) );
            strcat( buf, "." );
         }
         else
            strcat( buf, "someone who left??" );
         break;
      case POS_FIGHTING:
         strcat( buf, " is here, fighting " );
         if( !victim->fighting )
            strcat( buf, "thin air???" );
         else if( who_fighting( victim ) == ch )
            strcat( buf, "YOU!" );
         else if( victim->in_room == victim->fighting->who->in_room )
         {
            strcat( buf, PERS( victim->fighting->who, ch ) );
            strcat( buf, "." );
         }
         else
            strcat( buf, "someone who left??" );
         break;
   }

   strcat( buf, "\r\n" );
   buf[0] = UPPER( buf[0] );
   if( IS_AFFECTED( ch, AFF_INFRARED ) )
   {  // \033[1;31m
      sprintf( buf2, "\033[1;31m" );
      strcat( buf2, buf );
      send_to_char_noand( buf2, ch );
      show_visible_affects_to_char( victim, ch );
   }
   else
   {
      send_to_char( buf, ch );   //Nonseq
      show_visible_affects_to_char( victim, ch );
   }
   return;
}



void show_char_to_char_1( CHAR_DATA * victim, CHAR_DATA * ch )
{
   OBJ_DATA *obj;
   int iWear;
   bool found;

   if( can_see( victim, ch ) )
   {
      act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT );
      if( ch != victim )
         act( AT_ACTION, "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
      else
         act( AT_ACTION, "$n looks $Mself over.", ch, NULL, victim, TO_NOTVICT );
   }

   if( victim == ch && !IS_NPC( ch ) )
   {
      if( !IS_DROID( ch ) )
         ch_printf( ch, "You are of %s build and of %s height.\r\n", build_name[ch->build], height_name[ch->pheight] );
      else
         ch_printf( ch, "You are of %s build and of %s height.\r\n", droid_name[ch->build], height_name[ch->pheight] );

      if( ch->pcdata->disguise && ch->pcdata->disguise[0] != '\0' )
         ch_printf( ch, "You are currently disguised as: %s\r\n", ch->pcdata->disguise );
   }

   if( victim->description[0] != '\0' )
   {
      send_to_char( victim->description, ch );
   }
   else
   {
      act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
   }

   show_condition( ch, victim );

   found = FALSE;
   for( iWear = 0; iWear < MAX_WEAR; iWear++ )
   {
      if( ( obj = get_eq_char( victim, iWear ) ) != NULL && can_see_obj( ch, obj ) )
      {
         if( !found )
         {
            send_to_char( "\r\n", ch );
            act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
            found = TRUE;
         }
         send_to_char( where_name[iWear], ch );
         send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
         send_to_char( "\r\n", ch );
      }
   }

   /*
    * Crash fix here by Thoric
    */
   if( IS_NPC( ch ) || victim == ch )
      return;

   if( IS_IMMORTAL( victim ) && ( victim->top_level > ch->top_level ) )
      return;

   if( number_percent(  ) < ch->pcdata->learned[gsn_peek] )
   {
      send_to_char( "\r\nYou peek at the inventory:\r\n", ch );
      show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
      learn_from_success( ch, gsn_peek );
   }
   else if( ch->pcdata->learned[gsn_peek] )
      learn_from_failure( ch, gsn_peek );

   return;
}


void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch )
{
   CHAR_DATA *rch;

   for( rch = list; rch; rch = rch->next_in_room )
   {
      if( rch == ch )
         continue;

      if( can_see( ch, rch ) )
      {
         show_char_to_char_0( rch, ch );
      }
      else if( rch->race == RACE_DEFEL )
      {
         set_char_color( AT_BLOOD, ch );
         send_to_char( "You see a pair of red eyes staring back at you.\r\n", ch );
      }
      else if( room_is_dark( ch->in_room ) && IS_AFFECTED( rch, AFF_INFRARED ) )
      {
         set_char_color( AT_BLOOD, ch );
         send_to_char( "The red form of a living creature is here.\r\n", ch );
      }
   }

   return;
}

void show_ships_to_char( SHIP_DATA * ship, CHAR_DATA * ch )
{
   SHIP_DATA *rship;
   SHIP_DATA *nship = NULL;

   for( rship = ship; rship; rship = nship )
   {
      ch_printf( ch, "&C%-35s     ", rship->name );
      if( ( nship = rship->next_in_room ) != NULL )
      {
         ch_printf( ch, "%-35s", nship->name );
         nship = nship->next_in_room;
      }
      ch_printf( ch, "\r\n&w" );
   }

   return;
}

bool check_blind( CHAR_DATA * ch )
{
   if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;

   if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return TRUE;

   if( IS_AFFECTED( ch, AFF_BLIND ) )
   {
      send_to_char( "You can't see a thing!\r\n", ch );
      return FALSE;
   }
   return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( const char *arg )
{
   int door;

   if( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) )
      door = 0;
   else if( !str_cmp( arg, "e" ) || !str_cmp( arg, "east" ) )
      door = 1;
   else if( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) )
      door = 2;
   else if( !str_cmp( arg, "w" ) || !str_cmp( arg, "west" ) )
      door = 3;
   else if( !str_cmp( arg, "u" ) || !str_cmp( arg, "up" ) )
      door = 4;
   else if( !str_cmp( arg, "d" ) || !str_cmp( arg, "down" ) )
      door = 5;
   else if( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) )
      door = 6;
   else if( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) )
      door = 7;
   else if( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) )
      door = 8;
   else if( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) )
      door = 9;
   else
      door = -1;
   return door;
}

void do_look( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *original;
   char *pdesc;
   short door;
   int number, cnt;

   if( !ch->desc )
      return;

   if( ch->position < POS_SLEEPING )
   {
      send_to_char( "You can't see anything but stars!\r\n", ch );
      return;
   }

   if( ch->position == POS_SLEEPING )
   {
      send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
      return;
   }

   if( !check_blind( ch ) )
      return;

   if( !IS_NPC( ch )
       && !IS_SET( ch->act, PLR_HOLYLIGHT )
       && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && !IS_AFFECTED( ch, AFF_INFRARED ) && room_is_dark( ch->in_room ) )
   {
      set_char_color( AT_DGREY, ch );
      send_to_char( "It is pitch black ... \r\n", ch );
      show_char_to_char( ch->in_room->first_person, ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
   {
      SHIP_DATA *ship;

      /*
       * 'look' or 'look auto' 
       */
      set_char_color( AT_RMNAME, ch );
      ch_printf( ch, "&R-=&r( &G&W%s &G&r)&R=-&C&w ", ch->in_room->name );
      //send_to_char( ch->in_room->name, ch);
      //send_to_char(" ", ch);

      if( !ch->desc->original )
      {
         if( get_trust( ch ) >= LEVEL_IMMORTAL )
         {
            if( IS_SET( ch->act, PLR_ROOMVNUM ) )
            {
               set_char_color( AT_BLUE, ch );   /* Added 10/17 by Kuran of */
               send_to_char( "{", ch );   /* SWReality */
               ch_printf( ch, "&G&W%d", ch->in_room->vnum );
               set_char_color( AT_BLUE, ch );   /* Added 10/17 by Kuran of */
               send_to_char( "}", ch );
            }
            if( IS_SET( ch->pcdata->flags, PCFLAG_ROOM ) )
            {
               set_char_color( AT_CYAN, ch );
               send_to_char( "[", ch );
               send_to_char( " ", ch );
               set_char_color( AT_WHITE, ch );
               send_to_char( flag_string( ch->in_room->room_flags, r_flags ), ch );
               send_to_char( " ", ch );
               if( ch->in_room->room_flags2 != 0 )
               {
                  set_char_color( AT_DGREEN, ch );
                  send_to_char( flag_string( ch->in_room->room_flags2, r_flags2 ), ch );
                  send_to_char( " ", ch );
               }
               set_char_color( AT_CYAN, ch );
               send_to_char( "]", ch );
            }
         }
      }

      send_to_char( "\r\n", ch );
      set_char_color( AT_RMDESC, ch );

      if( arg1[0] == '\0' || ( !IS_NPC( ch ) && !IS_SET( ch->act, PLR_BRIEF ) ) )
         send_to_char( ch->in_room->description, ch );


      if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) )
         do_exits( ch, "" );

      show_ships_to_char( ch->in_room->first_ship, ch );
      show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
      show_char_to_char( ch->in_room->first_person, ch );

      if( str_cmp( arg1, "auto" ) )
         if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) != NULL )
         {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "\r\nThrough the transparisteel windows you see:\r\n" );

            if( ship->starsystem )
            {
               MISSILE_DATA *missile;
               SHIP_DATA *target;
               PLANET_DATA *planet;

               set_char_color( AT_GREEN, ch );
               if( ship->starsystem->star1 && str_cmp( ship->starsystem->star1, "" ) )
                  ch_printf( ch, "&YThe star, %s.\r\n", ship->starsystem->star1 );
               if( ship->starsystem->star2 && str_cmp( ship->starsystem->star2, "" ) )
                  ch_printf( ch, "&YThe star, %s.\r\n", ship->starsystem->star2 );
               for( planet = ship->starsystem->first_planet; planet; planet = planet->next_in_system )
                  if( planet->controls == 0 )
                     ch_printf( ch, "&GThe planet, %s.\r\n", planet->name );
                  else
                     ch_printf( ch, "&OAsteroid designation %s.\r\n", planet->name );
               for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
               {
                  if( target != ship )
                  {
                     if( ship->sclass != 11 )
                        ch_printf( ch, "&C%s\r\n", target->name );
                     else
                        ch_printf( ch, "&O%s\r\n", target->name );
                  }
               }
               for( missile = ship->starsystem->first_missile; missile; missile = missile->next_in_starsystem )
               {
                  ch_printf( ch, "&R%s\r\n",
                             missile->missiletype == CONCUSSION_MISSILE ? "A Concusion Missile" :
                             ( missile->missiletype == PROTON_TORPEDO ? "A Torpedo" :
                               ( missile->missiletype == HEAVY_ROCKET ? "A Heavy Rocket" : "A Heavy Bomb" ) ) );
               }

            }
            else if( ship->location == ship->lastdoc )
            {
               ROOM_INDEX_DATA *to_room;
               to_room = ship->in_room;
               if( to_room )  //get_room_index( ship->location ) ) != NULL )
               {
                  ch_printf( ch, "\r\n" );
                  original = ch->in_room;
                  char_from_room( ch );
                  char_to_room( ch, to_room );
                  do_glance( ch, "" );
                  char_from_room( ch );
                  char_to_room( ch, original );
               }
            }


         }

      return;
   }

   if( !str_cmp( arg1, "under" ) )
   {
      int count;

      /*
       * 'look under' 
       */
      if( arg2[0] == '\0' )
      {
         send_to_char( "Look beneath what?\r\n", ch );
         return;
      }

      if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not see that here.\r\n", ch );
         return;
      }
      if( ch->carry_weight + obj->weight > can_carry_w( ch ) )
      {
         send_to_char( "It's too heavy for you to look under.\r\n", ch );
         return;
      }
      count = obj->count;
      obj->count = 1;
      act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
      act( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
      obj->count = count;
      if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
         show_list_to_char( obj->first_content, ch, TRUE, TRUE );
      else
         send_to_char( "Nothing.\r\n", ch );
      if( EXA_prog_trigger )
         oprog_examine_trigger( ch, obj );
      return;
   }

   if( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
   {
      int count;

      /*
       * 'look in' 
       */
      if( arg2[0] == '\0' )
      {
         send_to_char( "Look in what?\r\n", ch );
         return;
      }

      if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not see that here.\r\n", ch );
         return;
      }

      switch ( obj->item_type )
      {
         default:
            send_to_char( "That is not a container.\r\n", ch );
            break;

         case ITEM_DRINK_CON:
            if( obj->value[1] <= 0 )
            {
               send_to_char( "It is empty.\r\n", ch );
               if( EXA_prog_trigger )
                  oprog_examine_trigger( ch, obj );
               break;
            }

            ch_printf( ch, "It's %s full of a %s liquid.\r\n",
                       obj->value[1] < obj->value[0] / 4
                       ? "less than" :
                       obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq_table[obj->value[2]].liq_color );

            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            break;

         case ITEM_PORTAL:
            for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
            {
               if( pexit->vdir == DIR_PORTAL && IS_SET( pexit->exit_info, EX_PORTAL ) )
               {
                  if( room_is_private( ch, pexit->to_room ) && get_trust( ch ) < sysdata.level_override_private )
                  {
                     set_char_color( AT_WHITE, ch );
                     send_to_char( "That room is private buster!\r\n", ch );
                     return;
                  }
                  original = ch->in_room;
                  char_from_room( ch );
                  char_to_room( ch, pexit->to_room );
                  do_look( ch, "auto" );
                  char_from_room( ch );
                  char_to_room( ch, original );
                  return;
               }
            }
            send_to_char( "You see a swirling chaos...\r\n", ch );
            break;
         case ITEM_CONTAINER:
         case ITEM_CORPSE_NPC:
         case ITEM_CORPSE_PC:
         case ITEM_DROID_CORPSE:
            if( IS_SET( obj->value[1], CONT_CLOSED ) )
            {
               send_to_char( "It is closed.\r\n", ch );
               break;
            }

            count = obj->count;
            obj->count = 1;
            act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
            obj->count = count;
            show_list_to_char( obj->first_content, ch, TRUE, TRUE );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            break;
      }
      return;
   }

   if( ( pdesc = get_extra_descr( arg1, ch->in_room->first_extradesc ) ) != NULL )
   {
      send_to_char( pdesc, ch );
      return;
   }

   door = get_door( arg1 );
   if( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
   {
      if( pexit->keyword )
      {
         if( IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_SET( pexit->exit_info, EX_WINDOW ) )
         {
            if( IS_SET( pexit->exit_info, EX_SECRET ) && door != -1 )
               send_to_char( "Nothing special there.\r\n", ch );
            else
               act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
            return;
         }
         if( IS_SET( pexit->exit_info, EX_BASHED ) )
            act( AT_RED, "The $d has been bashed from its hinges!", ch, NULL, pexit->keyword, TO_CHAR );
      }

      if( pexit->description && pexit->description[0] != '\0' )
         send_to_char( pexit->description, ch );
      else
         send_to_char( "Nothing special there.\r\n", ch );

      /*
       * Ability to look into the next room        -Thoric
       */
      if( pexit->to_room
          && ( IS_AFFECTED( ch, AFF_SCRYING )
               || IS_SET( pexit->exit_info, EX_xLOOK ) || get_trust( ch ) >= LEVEL_IMMORTAL ) )
      {
         if( !IS_SET( pexit->exit_info, EX_xLOOK ) && get_trust( ch ) < LEVEL_IMMORTAL )
         {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "You attempt to scry...\r\n", ch );
            /*
             * Change by Narn, Sept 96 to allow characters who don't have the
             * scry spell to benefit from objects that are affected by scry.
             */
            if( !IS_NPC( ch ) )
            {
               int percent = ch->pcdata->learned[skill_lookup( "scry" )];
               if( !percent )
                  percent = 99;

               if( number_percent(  ) > percent )
               {
                  send_to_char( "You fail.\r\n", ch );
                  return;
               }
            }
         }
         if( room_is_private( ch, pexit->to_room ) && get_trust( ch ) < sysdata.level_override_private )
         {
            set_char_color( AT_WHITE, ch );
            send_to_char( "That room is private buster!\r\n", ch );
            return;
         }
         original = ch->in_room;
         if( pexit->distance > 1 )
         {
            ROOM_INDEX_DATA *to_room;
            if( ( to_room = generate_exit( ch->in_room, &pexit ) ) != NULL )
            {
               char_from_room( ch );
               char_to_room( ch, to_room );
            }
            else
            {
               char_from_room( ch );
               char_to_room( ch, pexit->to_room );
            }
         }
         else
         {
            char_from_room( ch );
            char_to_room( ch, pexit->to_room );
         }
         do_look( ch, "auto" );
         char_from_room( ch );
         char_to_room( ch, original );
      }
      return;
   }
   else if( door != -1 )
   {
      send_to_char( "Nothing special there.\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) != NULL )
   {
      show_char_to_char_1( victim, ch );
      return;
   }


   /*
    * finally fixed the annoying look 2.obj desc bug -Thoric 
    */
   number = number_argument( arg1, arg );
   for( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( can_see_obj( ch, obj ) )
      {
         if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }

         if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }

         if( nifty_is_name_prefix( arg, obj->name ) )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
            if( !pdesc )
               pdesc = get_extra_descr( obj->name, obj->first_extradesc );
            if( !pdesc )
               send_to_char( "You see nothing special.\r\n", ch );
            else
               send_to_char( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }
      }
   }

   for( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
   {
      if( can_see_obj( ch, obj ) )
      {
         if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }

         if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }
         if( nifty_is_name_prefix( arg, obj->name ) )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
            if( !pdesc )
               pdesc = get_extra_descr( obj->name, obj->first_extradesc );
            if( !pdesc )
               send_to_char( "You see nothing special.\r\n", ch );
            else
               send_to_char( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }
      }
   }

   send_to_char( "You do not see that here.\r\n", ch );
   return;
}

void show_condition( CHAR_DATA * ch, CHAR_DATA * victim )
{
   char buf[MAX_STRING_LENGTH];
   int percent;

   if( victim->max_hit > 0 )
      percent = ( int )( ( 100.0 * ( double )( victim->hit ) ) / ( double )( victim->max_hit ) );
   else
      percent = -1;

   strcpy( buf, PERS( victim, ch ) );

   if( ( IS_NPC( victim ) && IS_SET( victim->act, ACT_DROID ) ) || IS_DROID( victim ) )
   {

      if( percent >= 100 )
         strcat( buf, " is in perfect condition.\r\n" );
      else if( percent >= 90 )
         strcat( buf, " is slightly scratched.\r\n" );
      else if( percent >= 80 )
         strcat( buf, " has a few scrapes.\r\n" );
      else if( percent >= 70 )
         strcat( buf, " has some dents.\r\n" );
      else if( percent >= 60 )
         strcat( buf, " has a couple holes in its plating.\r\n" );
      else if( percent >= 50 )
         strcat( buf, " has a many broken pieces.\r\n" );
      else if( percent >= 40 )
         strcat( buf, " has many exposed circuits.\r\n" );
      else if( percent >= 30 )
         strcat( buf, " is leaking oil.\r\n" );
      else if( percent >= 20 )
         strcat( buf, " has smoke coming out of it.\r\n" );
      else if( percent >= 10 )
         strcat( buf, " is almost completely broken.\r\n" );
      else
         strcat( buf, " is about to EXPLODE.\r\n" );

   }
   else
   {

      if( percent >= 100 )
         strcat( buf, " is in perfect health.\r\n" );
      else if( percent >= 90 )
         strcat( buf, " is slightly scratched.\r\n" );
      else if( percent >= 80 )
         strcat( buf, " has a few bruises.\r\n" );
      else if( percent >= 70 )
         strcat( buf, " has some cuts.\r\n" );
      else if( percent >= 60 )
         strcat( buf, " has several wounds.\r\n" );
      else if( percent >= 50 )
         strcat( buf, " has many nasty wounds.\r\n" );
      else if( percent >= 40 )
         strcat( buf, " is bleeding freely.\r\n" );
      else if( percent >= 30 )
         strcat( buf, " is covered in blood.\r\n" );
      else if( percent >= 20 )
         strcat( buf, " is leaking guts.\r\n" );
      else if( percent >= 10 )
         strcat( buf, " is almost dead.\r\n" );
      else
         strcat( buf, " is DYING.\r\n" );

   }
   buf[0] = UPPER( buf[0] );
   send_to_char( buf, ch );
   return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
void do_glance( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int save_act;

   if( !ch->desc )
      return;

   if( ch->position < POS_SLEEPING )
   {
      send_to_char( "You can't see anything but stars!\r\n", ch );
      return;
   }

   if( ch->position == POS_SLEEPING )
   {
      send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
      return;
   }

   if( !check_blind( ch ) )
      return;

   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' )
   {
      save_act = ch->act;
      SET_BIT( ch->act, PLR_BRIEF );
      do_look( ch, "auto" );
      ch->act = save_act;
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They're not here.", ch );
      return;
   }
   else
   {
      if( can_see( victim, ch ) )
      {
         act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT );
         act( AT_ACTION, "$n glances at $N.", ch, NULL, victim, TO_NOTVICT );
      }

      show_condition( ch, victim );
      return;
   }

   return;
}

void do_examine( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   BOARD_DATA *board;
   short dam;

   if( !argument )
   {
      bug( "do_examine: null argument.", 0 );
      return;
   }

   if( !ch )
   {
      bug( "do_examine: null ch.", 0 );
      return;
   }

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Examine what?\r\n", ch );
      return;
   }

   EXA_prog_trigger = FALSE;
   do_look( ch, arg );
   EXA_prog_trigger = TRUE;

   /*
    * Support for looking at boards, checking equipment conditions,
    * and support for trigger positions by Thoric
    */
   if( ( obj = get_obj_here( ch, arg ) ) != NULL )
   {
      if( ( board = get_board( obj ) ) != NULL )
      {
         if( board->num_posts )
            ch_printf( ch, "There are about %d notes posted here.  Type 'note list' to list them.\r\n", board->num_posts );
         else
            send_to_char( "There aren't any notes posted here.\r\n", ch );
      }

      switch ( obj->item_type )
      {
         default:
            break;

         case ITEM_RLAUNCHER:
            if( obj->value[5] == 0 )
               ch_printf( ch, "&wIt isn't loaded with anything.\r\n" );
            else
               ch_printf( ch, "It is loaded with an %s missile.\r\n", obj->value[1] == 1 ? "incendiary" : "explosive" );
            break;

         case ITEM_ARMOR:
            if( obj->value[1] == 0 )
               obj->value[1] = obj->value[0];
            if( obj->value[1] == 0 )
               obj->value[1] = 1;
            dam = ( short )( ( obj->value[0] * 10 ) / obj->value[1] );
            strcpy( buf, "As you look more closely, you notice that it is " );
            if( dam >= 10 )
               strcat( buf, "in superb condition." );
            else if( dam == 9 )
               strcat( buf, "in very good condition." );
            else if( dam == 8 )
               strcat( buf, "in good shape." );
            else if( dam == 7 )
               strcat( buf, "showing a bit of wear." );
            else if( dam == 6 )
               strcat( buf, "a little run down." );
            else if( dam == 5 )
               strcat( buf, "in need of repair." );
            else if( dam == 4 )
               strcat( buf, "in great need of repair." );
            else if( dam == 3 )
               strcat( buf, "in dire need of repair." );
            else if( dam == 2 )
               strcat( buf, "very badly worn." );
            else if( dam == 1 )
               strcat( buf, "practically worthless." );
            else if( dam <= 0 )
               strcat( buf, "broken." );
            strcat( buf, "\r\n" );
            send_to_char( buf, ch );
            break;

         case ITEM_WEAPON:
            dam = INIT_WEAPON_CONDITION - obj->value[0];
            strcpy( buf, "As you look more closely, you notice that it is " );
            if( dam == 0 )
               strcat( buf, "in superb condition." );
            else if( dam == 1 )
               strcat( buf, "in excellent condition." );
            else if( dam == 2 )
               strcat( buf, "in very good condition." );
            else if( dam == 3 )
               strcat( buf, "in good shape." );
            else if( dam == 4 )
               strcat( buf, "showing a bit of wear." );
            else if( dam == 5 )
               strcat( buf, "a little run down." );
            else if( dam == 6 )
               strcat( buf, "in need of repair." );
            else if( dam == 7 )
               strcat( buf, "in great need of repair." );
            else if( dam == 8 )
               strcat( buf, "in dire need of repair." );
            else if( dam == 9 )
               strcat( buf, "very badly worn." );
            else if( dam == 10 )
               strcat( buf, "practically worthless." );
            else if( dam == 11 )
               strcat( buf, "almost broken." );
            else if( dam == 12 )
               strcat( buf, "broken." );
            strcat( buf, "\r\n" );
            send_to_char( buf, ch );
            if( obj->value[3] == WEAPON_BLASTER )
            {
               if( obj->blaster_setting == BLASTER_FULL )
                  ch_printf( ch, "It is set on FULL power.\r\n" );
               else if( obj->blaster_setting == BLASTER_HIGH )
                  ch_printf( ch, "It is set on HIGH power.\r\n" );
               else if( obj->blaster_setting == BLASTER_NORMAL )
                  ch_printf( ch, "It is set on NORMAL power.\r\n" );
               else if( obj->blaster_setting == BLASTER_HALF )
                  ch_printf( ch, "It is set on HALF power.\r\n" );
               else if( obj->blaster_setting == BLASTER_LOW )
                  ch_printf( ch, "It is set on LOW power.\r\n" );
               else if( obj->blaster_setting == BLASTER_STUN )
                  ch_printf( ch, "It is set on STUN.\r\n" );
               ch_printf( ch, "It has from %d to %d shots remaining.\r\n", obj->value[4] / 5, obj->value[4] );
            }
            else if( ( obj->value[3] == WEAPON_LIGHTSABER || obj->value[3] == WEAPON_DUAL_LIGHTSABER ||
                       obj->value[3] == WEAPON_VIBRO_BLADE || obj->value[3] == WEAPON_FORCE_PIKE ) )
            {
               ch_printf( ch, "It has %d/%d units of charge remaining.\r\n", obj->value[4], obj->value[5] );
            }
            break;

         case ITEM_FOOD:
            if( obj->timer > 0 && obj->value[1] > 0 )
               dam = ( obj->timer * 10 ) / obj->value[1];
            else
               dam = 10;
            strcpy( buf, "As you examine it carefully you notice that it " );
            if( dam >= 10 )
               strcat( buf, "is fresh." );
            else if( dam == 9 )
               strcat( buf, "is nearly fresh." );
            else if( dam == 8 )
               strcat( buf, "is perfectly fine." );
            else if( dam == 7 )
               strcat( buf, "looks good." );
            else if( dam == 6 )
               strcat( buf, "looks ok." );
            else if( dam == 5 )
               strcat( buf, "is a little stale." );
            else if( dam == 4 )
               strcat( buf, "is a bit stale." );
            else if( dam == 3 )
               strcat( buf, "smells slightly off." );
            else if( dam == 2 )
               strcat( buf, "smells quite rank." );
            else if( dam == 1 )
               strcat( buf, "smells revolting." );
            else if( dam <= 0 )
               strcat( buf, "is crawling with maggots." );
            strcat( buf, "\r\n" );
            send_to_char( buf, ch );
            break;

         case ITEM_SWITCH:
         case ITEM_LEVER:
         case ITEM_PULLCHAIN:
            if( IS_SET( obj->value[0], TRIG_UP ) )
               send_to_char( "You notice that it is in the up position.\r\n", ch );
            else
               send_to_char( "You notice that it is in the down position.\r\n", ch );
            break;
         case ITEM_BUTTON:
            if( IS_SET( obj->value[0], TRIG_UP ) )
               send_to_char( "You notice that it is depressed.\r\n", ch );
            else
               send_to_char( "You notice that it is not depressed.\r\n", ch );
            break;

         case ITEM_CORPSE_PC:
         case ITEM_CORPSE_NPC:
         {
            short timerfrac = obj->timer;
            if( obj->item_type == ITEM_CORPSE_PC )
               timerfrac = ( int )obj->timer / 8 + 1;

            switch ( timerfrac )
            {
               default:
                  send_to_char( "This corpse has recently been slain.\r\n", ch );
                  break;
               case 4:
                  send_to_char( "This corpse was slain a little while ago.\r\n", ch );
                  break;
               case 3:
                  send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\r\n", ch );
                  break;
               case 2:
                  send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\r\n", ch );
                  break;
               case 1:
               case 0:
                  send_to_char( "Little more than bones, there isn't much left of this corpse.\r\n", ch );
                  break;
            }
         }
            if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
               break;
            send_to_char( "When you look inside, you see:\r\n", ch );
            EXA_prog_trigger = FALSE;
            sprintf( buf, "in %s", arg );
            do_look( ch, buf );
            EXA_prog_trigger = TRUE;
            break;

         case ITEM_DROID_CORPSE:
         {
            short timerfrac = obj->timer;

            switch ( timerfrac )
            {
               default:
                  send_to_char( "These remains are still smoking.\r\n", ch );
                  break;
               case 4:
                  send_to_char( "The parts of this droid have cooled down completely.\r\n", ch );
                  break;
               case 3:
                  send_to_char( "The broken droid components are beginning to rust.\r\n", ch );
                  break;
               case 2:
                  send_to_char( "The pieces are completely covered in rust.\r\n", ch );
                  break;
               case 1:
               case 0:
                  send_to_char( "All that remains of it is a pile of crumbling rust.\r\n", ch );
                  break;
            }
         }

         case ITEM_CONTAINER:
            if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
               break;

         case ITEM_DRINK_CON:
            send_to_char( "When you look inside, you see:\r\n", ch );
            sprintf( buf, "in %s noprog", arg );
            do_look( ch, buf );
      }
      if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
      {
         EXA_prog_trigger = FALSE;
         sprintf( buf, "under %s", arg );
         do_look( ch, buf );
         EXA_prog_trigger = TRUE;
      }
      oprog_examine_trigger( ch, obj );
      if( char_died( ch ) || obj_extracted( obj ) )
         return;

      check_for_trap( ch, obj, TRAP_EXAMINE );
   }
   return;
}

void do_exits( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   EXIT_DATA *pexit;
   bool found;
   bool fAuto;

   set_char_color( AT_EXITS, ch );
   buf[0] = '\0';
   fAuto = !str_cmp( argument, "auto" );

   if( !check_blind( ch ) )
      return;

   if( !IS_NPC( ch )
       && !IS_SET( ch->act, PLR_HOLYLIGHT )
       && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && !IS_AFFECTED( ch, AFF_INFRARED ) && room_is_dark( ch->in_room ) )
   {
      set_char_color( AT_DGREY, ch );
      send_to_char( "It is pitch black ... \r\n", ch );
      return;
   }

   strcpy( buf, fAuto ? "Exits:" : "\r\nObvious exits:\r\n&R--------------&W\r\n" );

   found = FALSE;
   for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( pexit->to_room && !IS_SET( pexit->exit_info, EX_HIDDEN ) )
      {
         found = TRUE;
         if( !fAuto )
         {
            if( IS_SET( pexit->exit_info, EX_CLOSED ) )
            {
               sprintf( buf + strlen( buf ), "%-5s - (closed)\r\n", capitalize( dir_name[pexit->vdir] ) );
            }
            else if( IS_SET( pexit->exit_info, EX_WINDOW ) )
            {
               sprintf( buf + strlen( buf ), "%-5s - (window)\r\n", capitalize( dir_name[pexit->vdir] ) );
            }
            else if( IS_SET( pexit->exit_info, EX_xAUTO ) )
            {
               sprintf( buf + strlen( buf ), "%-5s - %s\r\n",
                        capitalize( pexit->keyword ),
                        room_is_dark( pexit->to_room ) ? "Too dark to tell" : pexit->to_room->name );
            }
            else
               sprintf( buf + strlen( buf ), "%-5s - %s\r\n",
                        capitalize( dir_name[pexit->vdir] ),
                        room_is_dark( pexit->to_room ) ? "Too dark to tell" : pexit->to_room->name );
         }
         else
         {
            sprintf( buf + strlen( buf ), " %s", capitalize( dir_name[pexit->vdir] ) );
         }
      }
   }

   if( !found )
      strcat( buf, fAuto ? " none.\r\n" : "None.\r\n" );
   else if( fAuto )
      strcat( buf, ".\r\n" );
   send_to_char( buf, ch );
   return;
}

const char *const day_name[] = {
   "Redemption", "Stealth", "Deception", "Uprising", "the Fight",
   "Renaissance", "Beauty"
};

const char *const month_name[] = {
   "the Emperor", "the Empire", "the Smuggler", "the Tear",
   "the Twin Suns", "the Jedi", "Honoghr", "D'an Imal", "the Force",
   "the Assassin", "Eleven", "the Clone", "the Dark Shades",
   "the Sith", "the Massassi", "the Ancients", "Kashyyyk"
};

void do_time( CHAR_DATA * ch, const char *argument )
{
   extern char str_boot_time[];
   extern char reboot_time[];
   const char *suf;
   int day;

   day = time_info.day + 1;

   if( day > 4 && day < 20 )
      suf = "th";
   else if( day % 10 == 1 )
      suf = "st";
   else if( day % 10 == 2 )
      suf = "nd";
   else if( day % 10 == 3 )
      suf = "rd";
   else
      suf = "th";

   set_char_color( AT_YELLOW, ch );
   ch_printf( ch,
              "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\r\n"
              "The mud started up at:    %s\r"
              "The system time (E.S.T.): %s\r"
              "Next Reboot is set for:   %s\r",
              ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
              time_info.hour >= 12 ? "pm" : "am",
              day_name[day % 7],
              day, suf, month_name[time_info.month], str_boot_time, ( char * )ctime( &current_time ), reboot_time );
   if( sysdata.CLEANPFILES )
      ch_printf( ch, "Next pfile cleanup is scheduled for: %s\r\n", ( char * )ctime( &new_pfile_time_t ) );


   return;
}



void do_weather( CHAR_DATA * ch, const char *argument )
{
   static const char *const sky_look[4] = {
      "cloudless",
      "cloudy",
      "rainy",
      "lit by flashes of lightning"
   };

   if( !IS_OUTSIDE( ch ) )
   {
      send_to_char( "You can't see the sky from here.\r\n", ch );
      return;
   }

   set_char_color( AT_BLUE, ch );
   ch_printf( ch, "The sky is %s and %s.\r\n",
              sky_look[weather_info.sky],
              weather_info.change >= 0 ? "a warm southerly breeze blows" : "a cold northern gust blows" );
   return;
}


/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help( CHAR_DATA * ch, const char *argument )
{
   char argall[MAX_INPUT_LENGTH];
   char argone[MAX_INPUT_LENGTH];
   char argnew[MAX_INPUT_LENGTH];
   HELP_DATA *pHelp;
   int lev;

   if( argument[0] == '\0' )
      argument = "summary";

   if( isdigit( argument[0] ) && !is_number( argument ) )
   {
      lev = number_argument( argument, argnew );
      argument = argnew;
   }
   else
      lev = -2;
   /*
    * Tricky argument handling so 'help a b' doesn't match a.
    */
   argall[0] = '\0';
   while( argument[0] != '\0' )
   {
      argument = one_argument( argument, argone );
      if( argall[0] != '\0' )
         strcat( argall, " " );
      strcat( argall, argone );
   }

   for( pHelp = first_help; pHelp; pHelp = pHelp->next )
   {
      if( pHelp->level > get_trust( ch ) )
         continue;
      if( lev != -2 && pHelp->level != lev )
         continue;

      if( is_name( argall, pHelp->keyword ) )
         return pHelp;
   }

   return NULL;
}

short str_similarity( const char *astr, const char *bstr )
{
   short matches = 0;

   if( !astr || !bstr )
      return matches;

   for( ; *astr; astr++ )
   {
      if( LOWER( *astr ) == LOWER( *bstr ) )
         matches++;

      if( ++bstr == '\0' )
         return matches;
   }

   return matches;
}

void similar_help_files( CHAR_DATA * ch, const char *argument )
{
   HELP_DATA *pHelp = NULL;
   char buf[MAX_STRING_LENGTH];
   char *extension;
   short lvl = 0;
   bool single = FALSE;
// char *argnew;    

/*
    if ( isdigit(argument[0]) && (index(argument, '.')))
    {
	number_argument( argument, argnew );
	argument = argnew;
    }

*/
   send_to_pager_color( "&C&BSimilar Help Files:\r\n", ch );

   for( pHelp = first_help; pHelp; pHelp = pHelp->next )
   {
      buf[0] = '\0';
      extension = pHelp->keyword;

      if( pHelp->level > get_trust( ch ) )
         continue;

      while( extension[0] != '\0' )
      {
         extension = one_argument( extension, buf );

         if( str_similarity( argument, buf ) > lvl )
         {
            lvl = str_similarity( argument, buf );
            single = TRUE;
         }
         else if( str_similarity( argument, buf ) == lvl && lvl > 0 )
         {
            single = FALSE;
         }
      }
   }

   if( lvl == 0 )
   {
      send_to_pager_color( "&C&GNo similar help files.\r\n", ch );
      return;
   }

   for( pHelp = first_help; pHelp; pHelp = pHelp->next )
   {
      buf[0] = '\0';
      extension = pHelp->keyword;

      while( extension[0] != '\0' )
      {
         extension = one_argument( extension, buf );

         if( str_similarity( argument, buf ) >= lvl && pHelp->level <= get_trust( ch ) )
         {
            if( single )
            {
               send_to_pager_color( "&C&GOpening only similar helpfile.&C\r\n", ch );
               do_help( ch, buf );
               return;
            }

            pager_printf( ch, "&C&G   %s\r\n", pHelp->keyword );
            break;

         }

      }
   }
   return;
}

/*
 * Now this is cleaner
 */
void do_help( CHAR_DATA * ch, const char *argument )
{
   HELP_DATA *pHelp;
   char buf[MAX_STRING_LENGTH];

   if( ( pHelp = get_help( ch, argument ) ) == NULL )
   {
      send_to_char( "No help on that word.\r\n", ch );
      similar_help_files( ch, argument );
      return;
   }

   if( pHelp->level >= 0 && str_cmp( argument, "imotd" ) )
   {
      //send_to_pager( pHelp->keyword, ch );
      //send_to_pager( "\r\n", ch );
      sprintf( buf, "&G=-=-=-=-=-=- &R%s &G-=-=-=-=-=-=&W\r\n", pHelp->keyword );
      send_to_pager( buf, ch );
   }

   if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_SOUND ) )
      send_to_pager( "!!SOUND(help)", ch );

   /*
    * Strip leading '.' to allow initial blanks.
    */
   if( pHelp->text[0] == '.' )
      send_to_pager_color( pHelp->text + 1, ch );
   else
      send_to_pager_color( pHelp->text, ch );
   return;
}

/*
 * Help editor							-Thoric
 */
void do_hedit( CHAR_DATA * ch, const char *argument )
{
   HELP_DATA *pHelp;

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor.\r\n", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_HELP_EDIT:
         if( ( pHelp = ( HELP_DATA * ) ch->dest_buf ) == NULL )
         {
            bug( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
            stop_editing( ch );
            return;
         }
         STRFREE( pHelp->text );
         pHelp->text = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
   if( ( pHelp = get_help( ch, argument ) ) == NULL ) /* new help */
   {
      HELP_DATA *tHelp;
      char argnew[MAX_INPUT_LENGTH];
      int lev;
      bool new_help = TRUE;

      for( tHelp = first_help; tHelp; tHelp = tHelp->next )
         if( !str_cmp( argument, tHelp->keyword ) )
         {
            pHelp = tHelp;
            new_help = FALSE;
            break;
         }
      if( new_help )
      {
         if( isdigit( argument[0] ) )
         {
            lev = number_argument( argument, argnew );
            argument = argnew;
         }
         else
            lev = get_trust( ch );
         CREATE( pHelp, HELP_DATA, 1 );
         pHelp->keyword = STRALLOC( strupper( argument ) );
         pHelp->text = STRALLOC( "" );
         pHelp->level = lev;
         add_help( pHelp );
      }
   }
   ch->substate = SUB_HELP_EDIT;
   ch->dest_buf = pHelp;
   start_editing( ch, pHelp->text );
   editor_desc_printf( ch, "Help topic, keyword '%s', level %d.", pHelp->keyword, pHelp->level );
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
const char *help_fix( const char *text )
{
   char *fixed;

   if( !text )
      return "";
   fixed = strip_cr( text );
   if( fixed[0] == ' ' )
      fixed[0] = '.';
   return fixed;
}

void do_hset( CHAR_DATA * ch, const char *argument )
{
   HELP_DATA *pHelp;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = smash_tilde_copy( argument );
   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: hset <field> [value] [help page]\r\n", ch );
      send_to_char( "\r\n", ch );
      send_to_char( "Field being one of:\r\n", ch );
      send_to_char( "  level keyword remove save\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "save" ) )
   {
      FILE *fpout;

      log_string_plus( "Saving help.are...", LOG_NORMAL, LEVEL_GREATER );

      rename( "help.are", "help.are.bak" );
      if( ( fpout = fopen( "help.are", "w" ) ) == NULL )
      {
         bug( "hset save: fopen", 0 );
         perror( "help.are" );
         return;
      }

      fprintf( fpout, "#HELPS\n\n" );
      for( pHelp = first_help; pHelp; pHelp = pHelp->next )
         fprintf( fpout, "%d %s~\n%s~\n\n", pHelp->level, pHelp->keyword, help_fix( pHelp->text ) );

      fprintf( fpout, "0 $~\n\n\n#$\n" );
      fclose( fpout );
      send_to_char( "Saved.\r\n", ch );
      return;
   }
   if( str_cmp( arg1, "remove" ) )
      argument = one_argument( argument, arg2 );

   if( ( pHelp = get_help( ch, argument ) ) == NULL )
   {
      send_to_char( "Cannot find help on that subject.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "remove" ) )
   {
      UNLINK( pHelp, first_help, last_help, next, prev );
      STRFREE( pHelp->text );
      STRFREE( pHelp->keyword );
      DISPOSE( pHelp );
      send_to_char( "Removed.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "level" ) )
   {
      int lev;

      if( !is_number( arg2 ) )
      {
         send_to_char( "Level field must be numeric.\r\n", ch );
         return;
      }

      lev = atoi( arg2 );
      if( lev < -1 || lev > get_trust( ch ) )
      {
         send_to_char( "You can't set the level to that.\r\n", ch );
         return;
      }
      pHelp->level = lev;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "keyword" ) )
   {
      STRFREE( pHelp->keyword );
      pHelp->keyword = STRALLOC( strupper( arg2 ) );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   do_hset( ch, "" );
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 * prefix keyword indexing added by Fireblade
 */
void do_hlist( CHAR_DATA * ch, const char *argument )
{
   int min, max, minlimit, maxlimit, cnt;
   char arg[MAX_INPUT_LENGTH];
   HELP_DATA *help;
   bool minfound, maxfound;
   char *idx;

   maxlimit = get_trust( ch );
   minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;

   min = minlimit;
   max = maxlimit;

   idx = NULL;
   minfound = FALSE;
   maxfound = FALSE;

   for( argument = one_argument( argument, arg ); arg[0] != '\0'; argument = one_argument( argument, arg ) )
   {
      if( !isdigit( arg[0] ) )
      {
         if( idx )
         {
            set_char_color( AT_GREEN, ch );
            ch_printf( ch, "You may only use a single keyword to index the list.\r\n" );
            return;
         }
         idx = arg;
      }
      else
      {
         if( !minfound )
         {
            min = URANGE( minlimit, atoi( arg ), maxlimit );
            minfound = TRUE;
         }
         else if( !maxfound )
         {
            max = URANGE( minlimit, atoi( arg ), maxlimit );
            maxfound = TRUE;
         }
         else
         {
            set_char_color( AT_GREEN, ch );
            ch_printf( ch, "You may only use two level limits.\r\n" );
            return;
         }
      }
   }

   if( min > max )
   {
      int temp = min;

      min = max;
      max = temp;
   }

   set_pager_color( AT_GREEN, ch );
   pager_printf( ch, "Help Topics in level range %d to %d:\r\n\r\n", min, max );
   for( cnt = 0, help = first_help; help; help = help->next )
      if( help->level >= min && help->level <= max && ( !idx || nifty_is_name_prefix( idx, help->keyword ) ) )
      {
         pager_printf( ch, "  %3d %s\r\n", help->level, help->keyword );
         ++cnt;
      }
   if( cnt )
      pager_printf( ch, "\r\n%d pages found.\r\n", cnt );
   else
      send_to_char( "None found.\r\n", ch );
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 *
void do_hlist( CHAR_DATA *ch, char *argument )
{
    int min, max, minlimit, maxlimit, cnt;
    char arg[MAX_INPUT_LENGTH];
    HELP_DATA *help;

    maxlimit = get_trust(ch);
    minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    {
	min = URANGE( minlimit, atoi(arg), maxlimit );
	if ( argument[0] != '\0' )
	    max = URANGE( min, atoi(argument), maxlimit );
	else
	    max = maxlimit;
    }
    else
    {
	min = minlimit;
	max = maxlimit;
    }
    set_pager_color( AT_GREEN, ch );
    pager_printf( ch, "Help Topics in level range %d to %d:\r\n\r\n", min, max );
    for ( cnt = 0, help = first_help; help; help = help->next )
	if ( help->level >= min && help->level <= max )
	{
	    pager_printf( ch, "  %3d %s\r\n", help->level, help->keyword );
	    ++cnt;
	}
    if ( cnt )
	pager_printf( ch, "\r\n%d pages found.\r\n", cnt );
    else
	send_to_char( "None found.\r\n", ch );
}*/


/* 
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 *
 * Uh, new do_who with no clan or race. Heh. -||
 */
void do_who( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char clan_name[MAX_INPUT_LENGTH];
   char invis_str[MAX_INPUT_LENGTH];
   char char_name[MAX_INPUT_LENGTH];
   char extra_title[MAX_STRING_LENGTH];
   char race_text[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int iRace;
   int iLevelLower;
   int iLevelUpper;
   int nNumber;
   int nMatch;
   int sMatch;
   bool rgfRace[MAX_RACE];
   bool fRaceRestrict;
   bool fImmortalOnly;
   bool fShowHomepage;
   FILE *whoout;

   /*
    * #define WT_IMM    0;
    * #define WT_MORTAL 1;
    */

   WHO_DATA *cur_who = NULL;
   WHO_DATA *next_who = NULL;
   WHO_DATA *first_mortal = NULL;
   WHO_DATA *first_newbie = NULL;
   WHO_DATA *first_imm = NULL;

   /*
    * Set default arguments.
    */
   iLevelLower = 0;
   iLevelUpper = MAX_LEVEL;
   fRaceRestrict = FALSE;
   fImmortalOnly = FALSE;
   fShowHomepage = FALSE;
   for( iRace = 0; iRace < MAX_RACE; iRace++ )
      rgfRace[iRace] = FALSE;

   /*
    * Parse arguments.
    */
   nNumber = 0;
   for( ;; )
   {
      char arg[MAX_STRING_LENGTH];

      argument = one_argument( argument, arg );
      if( arg[0] == '\0' )
         break;

      if( is_number( arg ) )
      {
         switch ( ++nNumber )
         {
            case 1:
               iLevelLower = atoi( arg );
               break;
            case 2:
               iLevelUpper = atoi( arg );
               break;
            default:
               send_to_char( "Only two level numbers allowed.\r\n", ch );
               return;
         }
      }
      else
      {
         if( strlen( arg ) < 3 )
         {
            send_to_char( "Be a little more specific please.\r\n", ch );
            return;
         }

         /*
          * Look for classes to turn on.
          */

         if( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
            fImmortalOnly = TRUE;
         else if( !str_cmp( arg, "www" ) )
            fShowHomepage = TRUE;

      }
   }

   /*
    * Now find matching chars.
    */
   nMatch = 0;
   sMatch = 0;
   buf[0] = '\0';
   /*
    * if ( ch )
    * set_pager_color( AT_GREEN, ch );
    * else 
    */

   if( fShowHomepage )
      whoout = fopen( WEBWHO_FILE, "w" );
   else
      whoout = fopen( WHO_FILE, "w" );

/* start from last to first to get it in the proper order */
   for( d = last_descriptor; d; d = d->prev )
   {
      CHAR_DATA *wch;
      char const *race;

      if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
          || ( !can_see( ch, d->character ) && IS_IMMORTAL( d->character ) ) || d->original )
         continue;
      wch = d->original ? d->original : d->character;
      if( wch->top_level < iLevelLower
          || wch->top_level > iLevelUpper
          || ( fImmortalOnly && wch->top_level < LEVEL_IMMORTAL ) || ( fRaceRestrict && !rgfRace[wch->race] ) )
         continue;

      nMatch++;

      sMatch++;

/*
   No need.

	if ( fShowHomepage
	&&   wch->pcdata->homepage
	&&   wch->pcdata->homepage[0] != '\0' )
	  sprintf( char_name, "<A HREF=\"%s\">%s</A>",
		show_tilde( wch->pcdata->homepage ), wch->name );
	else
*/
      strcpy( char_name, "" );
      if( wch->rank == NULL || !str_cmp( wch->rank, "(null)" ) )
         wch->rank = str_dup( "   " );
      sprintf( race_text, "&G&w%s&G&c &G&w", wch->rank );
      race = race_text;

      switch ( wch->top_level )
      {
         default:
            break;
         case 200:
            race = "The Ghost in the Machine";
            break;
         case MAX_LEVEL - 0:
            race = "&G&z(&G&wAdmin        &G&z)&G&w";
            break;
         case MAX_LEVEL - 1:
            race = "&G&z(&G&wHead Imm     &G&z)&G&w";
            break;
         case MAX_LEVEL - 2:
            race = "&G&z(&G&wHigh Builder &G&z)&G&w";
            break;
         case MAX_LEVEL - 3:
            race = "&G&z(&G&wBuilder      &G&z)&G&w";
            break;
         case MAX_LEVEL - 4:
            race = "&G&z(&G&wApprentice   &G&z)&G&w";
            break;
         case MAX_LEVEL - 5:
            race = "&G&z(&G&wRetired      &G&z)&G&w";
            break;
      }


      if( ch && !nifty_is_name( wch->name, remand( wch->pcdata->title ) ) && ch->top_level > wch->top_level )
         sprintf( extra_title, " [%s]", wch->name );
      else
         strcpy( extra_title, "" );

      /*
       * if ( IS_RETIRED( wch ) )
       * race = "Retired"; 
       * else if ( IS_GUEST( wch ) )
       * race = "Guest"; 
       * else if ( wch->rank && wch->rank[0] != '\0' )
       * race = wch->rank;
       */

      if( ch && ch->pcdata && ch->pcdata->clan && wch->pcdata->clan
          && ( ch->pcdata->clan == wch->pcdata->clan || ch->top_level >= LEVEL_IMMORTAL ) )

      {
         CLAN_DATA *pclan = wch->pcdata->clan;

         strcpy( clan_name, " (" );

         if( !str_cmp( wch->name, pclan->leader ) )
            strcat( clan_name, "Leader, " );
         if( !str_cmp( wch->name, pclan->number1 ) )
            strcat( clan_name, "First, " );
         if( !str_cmp( wch->name, pclan->number2 ) )
            strcat( clan_name, "Second, " );

         strcat( clan_name, pclan->name );
         strcat( clan_name, ")" );
      }
      else
         clan_name[0] = '\0';


      if( IS_SET( wch->act, PLR_WIZINVIS ) )
         sprintf( invis_str, "(%d)", wch->pcdata->wizinvis );
      else
         invis_str[0] = '\0';
      sprintf( buf, "%s   %s%s%s%s %s%s\n%s",
               race,
               invis_str,
               IS_SET( wch->act, PLR_AFK ) ? "[AFK]" : "",
               char_name, wch->pcdata->title, extra_title, clan_name, ch ? "\r" : "" );

      /*
       * This is where the old code would display the found player to the ch.
       * What we do instead is put the found data into a linked list
       */

      /*
       * First make the structure. 
       */
      CREATE( cur_who, WHO_DATA, 1 );
      cur_who->text = str_dup( buf );
      if( IS_IMMORTAL( wch ) )
         cur_who->type = WT_IMM;
      else
         cur_who->type = WT_MORTAL;

      /*
       * Then put it into the appropriate list. 
       */
      switch ( cur_who->type )
      {
         case WT_MORTAL:
            cur_who->next = first_mortal;
            first_mortal = cur_who;
            break;
         case WT_IMM:
            cur_who->next = first_imm;
            first_imm = cur_who;
            break;
         case WT_NEWBIE:
            cur_who->next = first_newbie;
            first_newbie = cur_who;
            break;

      }

   }


   /*
    * Ok, now we have three separate linked lists and what remains is to 
    * * display the information and clean up.
    */

   /*
    * Deadly list removed for swr ... now only 2 lists 
    */

   if( first_newbie )
   {
      if( !ch )
         fprintf( whoout, "%s",
                  htmlcolor
                  ( "&G===&B[ &PFotE &B]&G===&B[ &WMortals &B]&G=====================================================&B[]&W\n\n" ) );
      else
         send_to_pager
            ( "\r\n&G===&B[ &PFotE &B]&G===&B[ &WMortals &B]&G=====================================================&B[]&W\r\n\r\n",
              ch );
   }

   for( cur_who = first_newbie; cur_who; cur_who = next_who )
   {
      if( !ch )
         fprintf( whoout, "%s", htmlcolor( cur_who->text ) );
      else
         send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }


   if( first_mortal )
   {
      if( !ch )
         fprintf( whoout, "%s",
                  htmlcolor
                  ( "&z(  &rPlayers&z  )&R-_-&r^^-_-^^-&R_-&r^^-_-^^-_-^^-&R_-^^-&r_-^^-_-&R^^&z(=====+ &rFall of the Empire &z+====&z=)&w\n\n" ) );
      else
         send_to_pager
            ( "\r\n&z(  &rPlayers&z  )&R-_-&r^^-_-^^-&R_-&r^^-_-^^-_-^^-&R_-^^-&r_-^^-_-&R^^&z(=====+ &rFall of the Empire &z+====&z=)&w\r\n\r\n",
              ch );
   }

   for( cur_who = first_mortal; cur_who; cur_who = next_who )
   {
      if( !ch )
         fprintf( whoout, "%s", htmlcolor( cur_who->text ) );
      else
         send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }

   if( first_imm )
   {
      if( !ch )
         fprintf( whoout, "%s",
                  htmlcolor
                  ( "&z( &rImmortals&z )&R-_&r-^^-_-&R^&r-_-^^-_-^^&R-_-^^&r-_-^^-_-^^-&R_-^^&z(=====+ &rFall of the Empire &z+====&z=)&w\n\n" ) );
      else
         send_to_pager
            ( "\r\n&z( &rImmortals&z )&R-_&r-^^-_-&R^^&r-_-^^-_-^^&R-_-^^&r-_-^^-_-^^-&R_-^^&z(=====+ &rFall of the Empire &z+====&z=)&w\r\n\r\n",
              ch );
   }

   for( cur_who = first_imm; cur_who; cur_who = next_who )
   {
      if( !ch )
         fprintf( whoout, "%s", htmlcolor( cur_who->text ) );
      else
         send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }

   if( !ch )
   {
      fprintf( whoout,
               htmlcolor
               ( "&z( &R%d &rvisible &zplayer%s. &R%d &rtotal&z player%s. )&R-_&r-^^-_-&R^^&r-_-^^-_-^^&R-_-^^&r-_-^^-_-^^-&R^^-\n" ),
               sMatch, sMatch == 1 ? "" : "s", nMatch, nMatch == 1 ? "" : "s" );
      fprintf( whoout, "<div style='position: absolute; color: white; top: 5px; right: 5px;'>%s</div></font>",
               ( char * )ctime( &current_time ) );
      fclose( whoout );
      return;
   }
   fclose( whoout );
   ch_printf( ch,
              "\r\n&z( &R%d &rvisible &zplayer%s. &R%d &rtotal&z player%s. )&R-_&r-^^-_-&R^^&r-_-^^&R-_-^^&r-_-^^-_-^^-&R_-^^&r-_-^^-_-&R^^-\r\n",
              sMatch, sMatch == 1 ? "" : "s", nMatch, nMatch == 1 ? "" : "s" );
   return;
}

void do_setrank( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *vict;
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   bool isleader = FALSE;

   argument = one_argument( argument, arg1 );

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' || arg1[0] == '\0' )
   {
      send_to_char( "&RUsage: setrank <player> <string/none>\r\n", ch );
      return;
   }

   if( ch->pcdata->clan == NULL )
   {
      send_to_char( "&RYou must be in a CLAN to setrank.\r\n", ch );
      return;
   }

   if( !ch->pcdata->clan && str_cmp( arg1, "self" ) )
   {
      send_to_char( "&RYou cannot set others ranks when you are not in a clan.\r\n", ch );
      return;
   }

   if( str_cmp( ch->pcdata->clan->leader, ch->name ) && str_cmp( ch->pcdata->clan->number1, ch->name )
       && !IS_IMMORTAL( ch ) )
   {
      send_to_char( "&RYou are not in a position to set a rank.\r\n", ch );
      return;
   }
   else
      isleader = TRUE;

   if( !( vict = get_char_room( ch, arg1 ) ) )
   {
      ch_printf( ch, "&RThere isn't %s '%s' here.\r\n", aoran( arg1 ), arg1 );
      return;
   }

   if( IS_NPC( vict ) )
   {
      send_to_char( "&RThat lowly peasant is not worthy of rank.\r\n", ch );
      return;
   }

   if( ( vict->pcdata->clan == NULL || ch->pcdata->clan != vict->pcdata->clan ) && !IS_IMMORTAL( ch ) )
   {
      send_to_char( "&RThey are not a PART of your CLAN&d.\r\n", ch );
      return;
   }

   if( !ch->pcdata->clan->leader && !ch->pcdata->clan->number1 && !ch->pcdata->clan->number2 && !IS_IMMORTAL( ch ) )
   {
      send_to_char( "&RYou are not in a position to setranks.\r\n", ch );
      return;
   }

   if( ( ch->pcdata->clan && isleader && !str_cmp( arg1, "self" ) ) || ( !ch->pcdata->clan && !str_cmp( arg1, "self" ) ) )
   {
      do_rank( ch, argument );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      if( vict->rank )
         DISPOSE( vict->rank );
      vict->rank = str_dup( "                  " );
      ch_printf( ch, "You have removed %s's rank.\r\n", PERS( vict, ch ) );
      ch_printf( vict, "%s has removed your rank.\r\n", PERS( ch, vict ) );
      return;
   }

   if( strlen( argument ) > 40 || strlen( remand( argument ) ) > 19 )
   {
      send_to_char( "&RThat rank is too long. Choose one under 40 characters with color codes and under 20 without.\r\n",
                    ch );
      return;
   }

   argument = smash_tilde_copy( argument );
//  argument = rembg(argument);
   sprintf( buf, "%s", argument );

   if( vict->rank )
      DISPOSE( vict->rank );
   vict->rank = str_dup( buf );

   ch_printf( ch, "&wYou have set %s's rank to &w%s&w.\r\n", PERS( vict, ch ), argument );
   ch_printf( vict, "&w%s has assigned the rank of '%s&w' to you.\r\n", PERS( ch, vict ), argument );
}

void do_compare( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj1;
   OBJ_DATA *obj2;
   int value1;
   int value2;
   const char *msg;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Compare what to what?\r\n", ch );
      return;
   }

   if( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that item.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      for( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
      {
         if( obj2->wear_loc != WEAR_NONE
             && can_see_obj( ch, obj2 )
             && obj1->item_type == obj2->item_type && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE ) != 0 )
            break;
      }

      if( !obj2 )
      {
         send_to_char( "You aren't wearing anything comparable.\r\n", ch );
         return;
      }
   }
   else
   {
      if( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not have that item.\r\n", ch );
         return;
      }
   }

   msg = NULL;
   value1 = 0;
   value2 = 0;

   if( obj1 == obj2 )
   {
      msg = "You compare $p to itself.  It looks about the same.";
   }
   else if( obj1->item_type != obj2->item_type )
   {
      msg = "You can't compare $p and $P.";
   }
   else
   {
      switch ( obj1->item_type )
      {
         default:
            msg = "You can't compare $p and $P.";
            break;

         case ITEM_ARMOR:
            value1 = obj1->value[0];
            value2 = obj2->value[0];
            break;

         case ITEM_WEAPON:
            value1 = obj1->value[1] + obj1->value[2];
            value2 = obj2->value[1] + obj2->value[2];
            break;
      }
   }

   if( !msg )
   {
      if( value1 == value2 )
         msg = "$p and $P look about the same.";
      else if( value1 > value2 )
         msg = "$p looks better than $P.";
      else
         msg = "$p looks worse than $P.";
   }

   act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
   return;
}



void do_where( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   bool found;

   if( get_trust( ch ) < LEVEL_IMMORTAL )
   {
      send_to_char( "If only life were really that simple...\r\n", ch );
      return;
   }

   one_argument( argument, arg );

   set_pager_color( AT_PERSON, ch );
   if( arg[0] == '\0' )
   {
      if( get_trust( ch ) >= LEVEL_IMMORTAL )
         send_to_pager( "Players logged in:\r\n", ch );
      else
         pager_printf( ch, "Players near you in %s:\r\n", ch->in_room->area->name );
      found = FALSE;
      for( d = first_descriptor; d; d = d->next )
         if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
             && ( victim = d->character ) != NULL
             && !IS_NPC( victim )
             && victim->in_room
             && ( victim->in_room->area == ch->in_room->area || get_trust( ch ) >= LEVEL_IMMORTAL )
             && can_see( ch, victim ) )
         {
            found = TRUE;
            pager_printf( ch, "%-28s %-30s \r\n", victim->name, victim->in_room->name );
         }
      if( !found )
         send_to_char( "None\r\n", ch );
   }
   else
   {
      found = FALSE;
      for( victim = first_char; victim; victim = victim->next )
         if( victim->in_room
             && victim->in_room->area == ch->in_room->area
             && !IS_AFFECTED( victim, AFF_HIDE )
             && !IS_AFFECTED( victim, AFF_SNEAK ) && can_see( ch, victim ) && is_name( arg, victim->name ) )
         {
            found = TRUE;
            pager_printf( ch, "%-28s %s\r\n", PERS( victim, ch ), victim->in_room->name );
            break;
         }
      if( !found )
         act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   }

   return;
}




void do_consider( CHAR_DATA * ch, const char *argument )
{
   /*
    * New Concider code by Ackbar counts in a little bit more than
    * just their hp compared to yours :P Messages from godwars so leave this
    * in they deserve some credit :) - Ackbar
    */
   int diff;
   int con_hp;
   const char *msg;
   int overall;
   CHAR_DATA *victim;
   /*
    * It counts your difference's in combat, ac, and hp and then uses
    * * a simple enough formula to determine who's most likely to win :).
    */

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They are not here.\r\n", ch );
      return;
   }

//  act(AT_WHITE, "$n examines $N closely looking for any weaknesses.", ch, NULL, victim, TO_NOTVICT);
   act( AT_WHITE, "You examine $N closely looking for any weaknesses.", ch, NULL, victim, TO_CHAR );
//  act(AT_WHITE, "$n examines you closely looking for weaknesses.", ch, NULL, victim, TO_VICT);

   overall = 0;
   con_hp = victim->hit;


   diff = ch->hit * 100 / con_hp;
   if( diff <= 10 )
   {
      msg = "$E is currently FAR healthier than you are.";
      overall = overall - 3;
   }
   else if( diff <= 50 )
   {
      msg = "$E is currently much healthier than you are.";
      overall = overall - 2;
   }
   else if( diff <= 75 )
   {
      msg = "$E is currently slightly healthier than you are.";
      overall = overall - 1;
   }
   else if( diff <= 125 )
   {
      msg = "$E is currently about as healthy as you are.";
   }
   else if( diff <= 200 )
   {
      msg = "You are currently slightly healthier than $M.";
      overall = overall + 1;
   }
   else if( diff <= 500 )
   {
      msg = "You are currently much healthier than $M.";
      overall = overall + 2;
   }
   else
   {
      msg = "You are currently FAR healthier than $M.";
      overall = overall + 3;
   }
   act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

   diff = victim->armor - ch->armor;
   if( diff <= -100 )
   {
      msg = "$E is FAR better armoured than you.";
      overall = overall - 3;
   }
   else if( diff <= -50 )
   {
      msg = "$E looks much better armoured than you.";
      overall = overall - 2;
   }
   else if( diff <= -25 )
   {
      msg = "$E looks better armoured than you.";
      overall = overall - 1;
   }
   else if( diff <= 25 )
   {
      msg = "$E seems about as well armoured as you.";
   }
   else if( diff <= 50 )
   {
      msg = "You are better armoured than $M.";
      overall = overall + 1;
   }
   else if( diff <= 100 )
   {
      msg = "You are much better armoured than $M.";
      overall = overall + 2;
   }
   else
   {
      msg = "You are FAR better armoured than $M.";
      overall = overall + 3;
   }
   act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

   diff = victim->top_level - ch->top_level + GET_HITROLL( victim ) - GET_HITROLL( ch );
   if( diff <= -35 )
   {
      msg = "You are FAR more skilled than $M.";
      overall = overall + 3;
   }
   else if( diff <= -15 )
   {
      msg = "$E is not as skilled as you are.";
      overall = overall + 2;
   }
   else if( diff <= -5 )
   {
      msg = "$E doesn't seem quite as skilled as you.";
      overall = overall + 1;
   }
   else if( diff <= 5 )
   {
      msg = "You are about as skilled as $M.";
   }
   else if( diff <= 15 )
   {
      msg = "$E is slightly more skilled than you are.";
      overall = overall - 1;
   }
   else if( diff <= 35 )
   {
      msg = "$E seems more skilled than you are.";
      overall = overall - 2;
   }
   else
   {
      msg = "$E is FAR more skilled than you.";
      overall = overall - 3;
   }
   act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

   diff = victim->top_level - ch->top_level + GET_DAMROLL( victim ) - GET_DAMROLL( ch );
   if( diff <= -35 )
   {
      msg = "You are FAR more powerful than $M.";
      overall = overall + 3;
   }
   else if( diff <= -15 )
   {
      msg = "$E is not as powerful as you are.";
      overall = overall + 2;
   }
   else if( diff <= -5 )
   {
      msg = "$E doesn't seem quite as powerful as you.";
      overall = overall + 1;
   }
   else if( diff <= 5 )
   {
      msg = "You are about as powerful as $M.";
   }
   else if( diff <= 15 )
   {
      msg = "$E is slightly more powerful than you are.";
      overall = overall - 1;
   }
   else if( diff <= 35 )
   {
      msg = "$E seems more powerful than you are.";
      overall = overall - 2;
   }
   else
   {
      msg = "$E is FAR more powerful than you.";
      overall = overall - 3;
   }
   act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

   diff = overall;
   if( diff <= -11 )
   {
      msg = "Conclusion: $E would kill you in seconds.";
   }
   else if( diff <= -7 )
   {
      msg = "Conclusion: You would need a lot of luck to beat $M.";
   }
   else if( diff <= -3 )
   {
      msg = "Conclusion: You would need some luck to beat $N.";
   }
   else if( diff <= 2 )
   {
      msg = "Conclusion: It would be a very close fight.";
   }
   else if( diff <= 6 )
   {
      msg = "Conclusion: You shouldn't have a lot of trouble defeating $M.";
   }
   else if( diff <= 10 )
   {
      msg = "Conclusion: $N is no match for you.  You can easily beat $M.";
   }
   else
   {
      msg = "Conclusion: $E wouldn't last more than a few seconds against you.";
   }
   act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

   return;

}



/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

void do_practice( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int sn;

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      int col;
      short lasttype, cnt;

      col = cnt = 0;
      lasttype = SKILL_SPELL;

// set_pager_color( AT_MAGIC, ch );

//          send_to_pager("&G------------------------------------&B[&WSpells&B]&G------------------------------------\r\n", ch);

      for( sn = 0; sn < top_sn; sn++ )
      {
         if( !skill_table[sn]->name )
            break;

         if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
            continue;
         if( skill_table[sn]->guild == FORCE_ABILITY )
            continue;

         if( strcmp( skill_table[sn]->name, "reserved" ) == 0 && ( IS_IMMORTAL( ch ) ) )
         {
            if( col % 3 != 0 )
               send_to_pager( "\r\n", ch );
            col = 0;
         }
         if( skill_table[sn]->type != lasttype )
         {
            if( !cnt )
               send_to_pager( "", ch );
            else if( col % 3 != 0 )
               send_to_pager( "\r\n", ch );
            if( !str_cmp( skill_tname[skill_table[sn]->type], "weapon" ) )
               pager_printf( ch,
                             "&G-----------------------------------&B[&W%ss&B]&G------------------------------------\r\n",
                             skill_tname[skill_table[sn]->type] );
            else
               pager_printf( ch,
                             "&G------------------------------------&B[&W%s&B]&G-------------------------------------\r\n",
                             skill_tname[skill_table[sn]->type] );
            col = cnt = 0;
         }
         lasttype = skill_table[sn]->type;

         if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
            continue;

         if( ch->pcdata->learned[sn] <= 0 && ch->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
            continue;

         if( ch->pcdata->learned[sn] == 0 && SPELL_FLAG( skill_table[sn], SF_SECRETSKILL ) )
            continue;

         ++cnt;
         pager_printf( ch, "&B[&W%3d%%&B] %s%-18.18s ",
                       ch->pcdata->learned[sn],
                       skill_table[sn]->alignment == 0 ? "&G" : skill_table[sn]->alignment == -1001 ? "&R" : "&B",
                       skill_table[sn]->name );
         if( ++col % 3 == 0 )
            send_to_pager( "\r\n", ch );
      }

      if( col % 3 != 0 )
         send_to_pager( "\r\n", ch );

   }
   else
   {
      CHAR_DATA *mob;
      int adept;
      bool can_prac = TRUE;

      if( !IS_AWAKE( ch ) )
      {
         send_to_char( "In your dreams, or what?\r\n", ch );
         return;
      }

      for( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
         if( IS_NPC( mob ) && IS_SET( mob->act, ACT_PRACTICE ) )
            break;

      if( !mob )
      {
         send_to_char( "You can't do that here.\r\n", ch );
         return;
      }


      sn = skill_lookup( argument );

      if( sn == -1 )
      {
         act( AT_TELL, "&R&C$n says:&W I've never heard of that one.", mob, NULL, ch, TO_VICT );
         return;
      }

      if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
      {
         act( AT_TELL, "&R&C$n says:&W I can't teach you that.", mob, NULL, ch, TO_VICT );
         return;
      }

      if( can_prac && !IS_NPC( ch ) && ch->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
      {
         act( AT_TELL, "&R&C$n says:&W I don't think you're ready to learn that yet.", mob, NULL, ch, TO_VICT );
         return;
      }

      if( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
      {
         act( AT_TELL, "&R&C$n says:&W I don't know how to do that.", mob, NULL, ch, TO_VICT );
         return;
      }

      /*
       * Skill requires a special teacher
       */
      if( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
      {
         sprintf( buf, "%d", mob->pIndexData->vnum );
         if( !is_name( buf, skill_table[sn]->teachers ) )
         {
            act( AT_TELL, "&R&C$n says:&W I don't know how to do that.", mob, NULL, ch, TO_VICT );
            return;
         }
      }
      else
      {
         act( AT_TELL, "&R&C$n says:&W I don't know how to do that.", mob, NULL, ch, TO_VICT );
         return;
      }

      adept = 20;

      if( ch->pcdata->learned[sn] >= adept )
      {
         sprintf( buf, "$n tells you, 'I've taught you everything I can about %s.'", skill_table[sn]->name );
         act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
         act( AT_TELL, "&R&C$n says:&W You'll have to practice it on your own now.", mob, NULL, ch, TO_VICT );
      }
      else
      {
         ch->pcdata->learned[sn] += int_app[get_curr_int( ch )].learn;
         act( AT_ACTION, "You practice $T.", ch, NULL, skill_table[sn]->name, TO_CHAR );
         act( AT_ACTION, "$n practices $T.", ch, NULL, skill_table[sn]->name, TO_ROOM );
         if( ch->pcdata->learned[sn] >= adept )
         {
            ch->pcdata->learned[sn] = adept;
            act( AT_TELL, "&R&C$n says:&W You'll have to practice it on your own now.", mob, NULL, ch, TO_VICT );
         }
      }
   }
   return;
}

void do_viewskills( CHAR_DATA * ch, const char *argument )
{
   int sn;
   CHAR_DATA *victim;
   int col;
   short lasttype, cnt;

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: viewskills <player>\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, argument ) ) == NULL )
   {
      send_to_char( "No such player online.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You may only view the skills of a player.\r\n", ch );
      return;
   }

   col = cnt = 0;
   lasttype = SKILL_SPELL;

   set_pager_color( AT_MAGIC, ch );
   send_to_pager( "&G------------------------------------&B[&WSpells&B]&G------------------------------------\r\n", ch );

   for( sn = 0; sn < top_sn; sn++ )
   {
      if( !skill_table[sn]->name )
         break;

      if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
         continue;

      if( strcmp( skill_table[sn]->name, "reserved" ) == 0 && ( IS_IMMORTAL( ch ) ) )
      {
         if( col % 3 != 0 )
            send_to_pager( "\r\n", ch );
         col = 0;
      }
      if( skill_table[sn]->type != lasttype )
      {
         if( !cnt )
            send_to_pager( "                                (none)\r\n", ch );
         else if( col % 3 != 0 )
            send_to_pager( "\r\n", ch );
         if( !str_cmp( skill_tname[skill_table[sn]->type], "weapon" ) )
            pager_printf( ch, "&G-----------------------------------&B[&W%ss&B]&G------------------------------------\r\n",
                          skill_tname[skill_table[sn]->type] );
         else
            pager_printf( ch, "&G------------------------------------&B[&W%s&B]&G-------------------------------------\r\n",
                          skill_tname[skill_table[sn]->type] );
         col = cnt = 0;
      }
      lasttype = skill_table[sn]->type;

      if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
         continue;

      if( victim->pcdata->learned[sn] <= 0 && victim->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
         continue;

      if( victim->pcdata->learned[sn] == 0 && SPELL_FLAG( skill_table[sn], SF_SECRETSKILL ) )
         continue;

      ++cnt;
      pager_printf( ch, "&R[&W%3d%%&R] %s%-18.18s ",
                    victim->pcdata->learned[sn],
                    skill_table[sn]->alignment == 0 ? "&G" : skill_table[sn]->alignment == -1001 ? "&R" : "&B",
                    skill_table[sn]->name );
      if( ++col % 3 == 0 )
         send_to_pager( "\r\n", ch );
   }

   if( col % 3 != 0 )
      send_to_pager( "\r\n", ch );

   return;
}

void do_teach( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int sn;
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg );

   if( argument[0] == '\0' )
   {
      send_to_char( "Teach who, what?\r\n", ch );
      return;
   }
   else
   {
      CHAR_DATA *victim;
      int adept;

      if( !IS_AWAKE( ch ) )
      {
         send_to_char( "In your dreams, or what?\r\n", ch );
         return;
      }

      if( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
         send_to_char( "They don't seem to be here...\r\n", ch );
         return;
      }

      if( IS_NPC( victim ) )
      {
         send_to_char( "You can't teach that to them!\r\n", ch );
         return;
      }

      sn = skill_lookup( argument );

      if( sn == -1 )
      {
         act( AT_TELL, "You have no idea what that is.", victim, NULL, ch, TO_VICT );
         return;
      }

      if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
      {
         act( AT_TELL, "Thats just not going to happen.", victim, NULL, ch, TO_VICT );
         return;
      }

      if( victim->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
      {
         act( AT_TELL, "$n isn't ready to learn that yet.", victim, NULL, ch, TO_VICT );
         return;
      }

      if( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
      {
         act( AT_TELL, "You are unable to teach that skill.", victim, NULL, ch, TO_VICT );
         return;
      }

      adept = 20;

      if( victim->pcdata->learned[sn] >= adept )
      {
         act( AT_TELL, "$n must practice that on their own.", victim, NULL, ch, TO_VICT );
         return;
      }
      if( ch->pcdata->learned[sn] < 100 )
      {
         act( AT_TELL, "You must perfect that yourself before teaching others.", victim, NULL, ch, TO_VICT );
         return;
      }
      else
      {
         victim->pcdata->learned[sn] += int_app[get_curr_int( ch )].learn;
         sprintf( buf, "You teach %s $T.", PERS( victim, ch ) );
         act( AT_ACTION, buf, ch, NULL, skill_table[sn]->name, TO_CHAR );
         sprintf( buf, "%s teaches you $T.", PERS( ch, victim ) );
         act( AT_ACTION, buf, victim, NULL, skill_table[sn]->name, TO_CHAR );
      }
   }
   return;
}

//Deprecated in FotE, but left in in case we decide to use it again.
void do_wimpy( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int wimpy;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
      wimpy = ( int )ch->max_hit / 5;
   else
      wimpy = atoi( arg );

   if( wimpy < 0 )
   {
      send_to_char( "Your courage exceeds your wisdom.\r\n", ch );
      return;
   }

   if( wimpy > ch->max_hit )
   {
      send_to_char( "Such cowardice ill becomes you.\r\n", ch );
      return;
   }

   ch->wimpy = wimpy;
   ch_printf( ch, "Wimpy set to %d hit points.\r\n", wimpy );
   return;
}

void do_password( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   char *pArg, *pwdnew;
   char cEnd;

   if( IS_NPC( ch ) )
      return;

   /*
    * Can't use one_argument here because it smashes case.
    * So we just steal all its code.  Bleagh.
    */
   pArg = arg1;
   while( isspace( *argument ) )
      argument++;

   cEnd = ' ';
   if( *argument == '\'' || *argument == '"' )
      cEnd = *argument++;

   while( *argument != '\0' )
   {
      if( *argument == cEnd )
      {
         argument++;
         break;
      }
      *pArg++ = *argument++;
   }
   *pArg = '\0';

   pArg = arg2;
   while( isspace( *argument ) )
      argument++;

   cEnd = ' ';
   if( *argument == '\'' || *argument == '"' )
      cEnd = *argument++;

   while( *argument != '\0' )
   {
      if( *argument == cEnd )
      {
         argument++;
         break;
      }
      *pArg++ = *argument++;
   }
   *pArg = '\0';

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: password <old> <new>.\r\n", ch );
      return;
   }

   if( strcmp( sha256_crypt( arg1 ), ch->pcdata->pwd ) )
   {
      WAIT_STATE( ch, 40 );
      send_to_char( "Wrong password.  Wait 10 seconds.\r\n", ch );
      return;
   }

   if( strlen( arg2 ) < 5 )
   {
      send_to_char( "New password must be at least five characters long.\r\n", ch );
      return;
   }

   if( arg1[0] == '!' || arg2[0] == '!' )
   {
      send_to_char( "New password cannot begin with the '!' character.", ch );
      return;
   }

   pwdnew = sha256_crypt( arg2 );

   DISPOSE( ch->pcdata->pwd );
   ch->pcdata->pwd = str_dup( pwdnew );
   if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
      save_char_obj( ch );
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_socials( CHAR_DATA * ch, const char *argument )
{
   int iHash;
   int col = 0;
   SOCIALTYPE *social;

   set_pager_color( AT_PLAIN, ch );
   for( iHash = 0; iHash < 27; iHash++ )
      for( social = social_index[iHash]; social; social = social->next )
      {
         pager_printf( ch, "%-12s", social->name );
         if( ++col % 6 == 0 )
            send_to_pager( "\r\n", ch );
      }

   if( col % 6 != 0 )
      send_to_pager( "\r\n", ch );
   return;
}


void do_commands( CHAR_DATA * ch, const char *argument )
{
   int col;
   bool found;
   int hash;
   CMDTYPE *command;

   col = 0;
   set_pager_color( AT_PLAIN, ch );
   if( argument[0] == '\0' )
   {
      for( hash = 0; hash < 126; hash++ )
         for( command = command_hash[hash]; command; command = command->next )
            if( command->level < LEVEL_HERO
                && command->level <= get_trust( ch ) && !IS_CMD_MPROG( command ) )
            {
               pager_printf( ch, "[%-2d] %-12s", command->level, command->name );
               if( ++col % 4 == 0 )
                  send_to_pager( "\r\n", ch );
            }
      if( col % 4 != 0 )
         send_to_pager( "\r\n", ch );
   }
   else
   {
      found = FALSE;
      for( hash = 0; hash < 126; hash++ )
         for( command = command_hash[hash]; command; command = command->next )
            if( command->level < LEVEL_HERO
                && command->level <= get_trust( ch )
                && !str_prefix( argument, command->name ) && !IS_CMD_MPROG( command ) )
            {
               pager_printf( ch, "%-12s", command->name );
               found = TRUE;
               if( ++col % 4 == 0 )
                  send_to_pager( "\r\n", ch );
            }

      if( col % 4 != 0 )
         send_to_pager( "\r\n", ch );
      if( !found )
         ch_printf( ch, "No command found under %s.\r\n", argument );
   }
   return;
}


void do_channels( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_SILENCE ) )
      {
         send_to_char( "You are silenced.\r\n", ch );
         return;
      }

      send_to_char( "Channels:", ch );

      if( get_trust( ch ) > 2 && !NOT_AUTHED( ch ) )
      {
         send_to_char( !IS_SET( ch->deaf, CHANNEL_AUCTION ) ? " +AUCTION" : " -auction", ch );
      }

      send_to_char( !IS_SET( ch->deaf, CHANNEL_CHAT ) ? " +CHAT" : " -chat", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_OOC ) ? " +OOC" : " -ooc", ch );

      if( !IS_NPC( ch ) && ch->pcdata->clan )
      {
         send_to_char( !IS_SET( ch->deaf, CHANNEL_CLAN ) ? " +CLAN" : " -clan", ch );
      }


      send_to_char( !IS_SET( ch->deaf, CHANNEL_QUEST ) ? " +QUEST" : " -quest", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_TELLS ) ? " +TELLS" : " -tells", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_WARTALK ) ? " +WARTALK" : " -wartalk", ch );

      if( IS_HERO( ch ) )
      {
         send_to_char( !IS_SET( ch->deaf, CHANNEL_AVTALK ) ? " +AVATAR" : " -avatar", ch );
      }

      if( IS_IMMORTAL( ch ) )
      {
         send_to_char( !IS_SET( ch->deaf, CHANNEL_IMMTALK ) ? " +IMMTALK" : " -immtalk", ch );

         send_to_char( !IS_SET( ch->deaf, CHANNEL_PRAY ) ? " +PRAY" : " -pray", ch );
      }

      send_to_char( !IS_SET( ch->deaf, CHANNEL_MUSIC ) ? " +MUSIC" : " -music", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_ASK ) ? " +ASK" : " -ask", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_SHOUT ) ? " +SHOUT" : " -shout", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_YELL ) ? " +YELL" : " -yell", ch );

      if( IS_IMMORTAL( ch ) )
      {
         send_to_char( !IS_SET( ch->deaf, CHANNEL_MONITOR ) ? " +MONITOR" : " -monitor", ch );
      }

      send_to_char( !IS_SET( ch->deaf, CHANNEL_NEWBIE ) ? " +NEWBIE" : " -newbie", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_SPORTS ) ? " +SPORTS" : " -sports", ch );

      send_to_char( !IS_SET( ch->deaf, CHANNEL_HOLONET ) ? " +HOLONET" : " -HOLONET", ch );

      if( get_trust( ch ) >= sysdata.log_level )
      {
         send_to_char( !IS_SET( ch->deaf, CHANNEL_LOG ) ? " +LOG" : " -log", ch );

         send_to_char( !IS_SET( ch->deaf, CHANNEL_BUILD ) ? " +BUILD" : " -build", ch );

         send_to_char( !IS_SET( ch->deaf, CHANNEL_COMM ) ? " +COMM" : " -comm", ch );
      }
      send_to_char( ".\r\n", ch );
   }
   else
   {
      bool fClear;
      bool ClearAll;
      int bit;

      bit = 0;
      ClearAll = FALSE;

      if( arg[0] == '+' )
         fClear = TRUE;
      else if( arg[0] == '-' )
         fClear = FALSE;
      else
      {
         send_to_char( "Channels -channel or +channel?\r\n", ch );
         return;
      }

      if( !str_cmp( arg + 1, "auction" ) && get_trust( ch ) > 2 && !NOT_AUTHED( ch ) )
         bit = CHANNEL_AUCTION;
      else if( !str_cmp( arg + 1, "chat" ) )
         bit = CHANNEL_CHAT;
      else if( !str_cmp( arg + 1, "ooc" ) )
         bit = CHANNEL_OOC;
      else if( !str_cmp( arg + 1, "clan" ) && !IS_NPC( ch ) && ch->pcdata->clan
               && ch->pcdata->clan->clan_type != CLAN_GUILD )
         bit = CHANNEL_CLAN;
      else if( !str_cmp( arg + 1, "guild" ) && !IS_NPC( ch ) && ch->pcdata->clan
               && ch->pcdata->clan->clan_type == CLAN_GUILD )
         bit = CHANNEL_GUILD;
      else if( !str_cmp( arg + 1, "quest" ) )
         bit = CHANNEL_QUEST;
      else if( !str_cmp( arg + 1, "tells" ) )
         bit = CHANNEL_TELLS;
      else if( !str_cmp( arg + 1, "immtalk" ) && IS_IMMORTAL( ch ) )
         bit = CHANNEL_IMMTALK;
      else if( !str_cmp( arg + 1, "log" ) && get_trust( ch ) >= sysdata.log_level )
         bit = CHANNEL_LOG;
      else if( !str_cmp( arg + 1, "build" ) && get_trust( ch ) >= sysdata.log_level )
         bit = CHANNEL_BUILD;
      else if( !str_cmp( arg + 1, "pray" ) && IS_IMMORTAL( ch ) )
         bit = CHANNEL_PRAY;
      else if( !str_cmp( arg + 1, "avatar" ) && IS_HERO( ch ) )
         bit = CHANNEL_AVTALK;
      else if( !str_cmp( arg + 1, "monitor" ) && IS_IMMORTAL( ch ) )
         bit = CHANNEL_MONITOR;
      else if( !str_cmp( arg + 1, "newbie" ) && ( IS_IMMORTAL( ch ) || ch->top_level < 3 ) )
         bit = CHANNEL_NEWBIE;
      else if( !str_cmp( arg + 1, "music" ) )
         bit = CHANNEL_MUSIC;
      else if( !str_cmp( arg + 1, "ask" ) )
         bit = CHANNEL_ASK;
      else if( !str_cmp( arg + 1, "shout" ) )
         bit = CHANNEL_SHOUT;
      else if( !str_cmp( arg + 1, "yell" ) )
         bit = CHANNEL_YELL;
      else if( !str_cmp( arg + 1, "comm" ) && get_trust( ch ) >= sysdata.log_level )
         bit = CHANNEL_COMM;
      else if( !str_cmp( arg + 1, "order" ) && !IS_NPC( ch ) && ch->pcdata->clan )
         bit = CHANNEL_ORDER;
      else if( !str_cmp( arg + 1, "wartalk" ) )
         bit = CHANNEL_WARTALK;
      else if( !str_cmp( arg + 1, "sports" ) )
         bit = CHANNEL_SPORTS;
      else if( !str_cmp( arg + 1, "holonet" ) )
         bit = CHANNEL_HOLONET;
      else if( !str_cmp( arg + 1, "all" ) )
         ClearAll = TRUE;
      else
      {
         send_to_char( "Set or clear which channel?\r\n", ch );
         return;
      }

      if( ( fClear ) && ( ClearAll ) )
      {
         REMOVE_BIT( ch->deaf, CHANNEL_AUCTION );
         REMOVE_BIT( ch->deaf, CHANNEL_CHAT );
         REMOVE_BIT( ch->deaf, CHANNEL_QUEST );
         /*
          * REMOVE_BIT (ch->deaf, CHANNEL_IMMTALK); 
          */
         REMOVE_BIT( ch->deaf, CHANNEL_PRAY );
         REMOVE_BIT( ch->deaf, CHANNEL_MUSIC );
         REMOVE_BIT( ch->deaf, CHANNEL_ASK );
         REMOVE_BIT( ch->deaf, CHANNEL_SHOUT );
         REMOVE_BIT( ch->deaf, CHANNEL_YELL );
         REMOVE_BIT( ch->deaf, CHANNEL_SPORTS );
         REMOVE_BIT( ch->deaf, CHANNEL_HOLONET );

         /*
          * if (ch->pcdata->clan)
          * REMOVE_BIT (ch->deaf, CHANNEL_CLAN);
          * 
          * 
          * if (ch->pcdata->guild)
          * REMOVE_BIT (ch->deaf, CHANNEL_GUILD);
          */
         if( ch->top_level >= LEVEL_IMMORTAL )
            REMOVE_BIT( ch->deaf, CHANNEL_AVTALK );

         if( ch->top_level >= sysdata.log_level )
            REMOVE_BIT( ch->deaf, CHANNEL_COMM );

      }
      else if( ( !fClear ) && ( ClearAll ) )
      {
         SET_BIT( ch->deaf, CHANNEL_AUCTION );
         SET_BIT( ch->deaf, CHANNEL_CHAT );
         SET_BIT( ch->deaf, CHANNEL_QUEST );
         /*
          * SET_BIT (ch->deaf, CHANNEL_IMMTALK); 
          */
         SET_BIT( ch->deaf, CHANNEL_PRAY );
         SET_BIT( ch->deaf, CHANNEL_MUSIC );
         SET_BIT( ch->deaf, CHANNEL_ASK );
         SET_BIT( ch->deaf, CHANNEL_SHOUT );
         SET_BIT( ch->deaf, CHANNEL_YELL );

         if( ch->top_level >= LEVEL_IMMORTAL )
            SET_BIT( ch->deaf, CHANNEL_AVTALK );

         if( ch->top_level >= sysdata.log_level )
            SET_BIT( ch->deaf, CHANNEL_COMM );

      }
      else if( fClear )
      {
         REMOVE_BIT( ch->deaf, bit );
      }
      else
      {
         SET_BIT( ch->deaf, bit );
      }

      send_to_char( "Ok.\r\n", ch );
   }

   return;
}


/*
 * display WIZLIST file						-Thoric
 */
void do_wizlist( CHAR_DATA * ch, const char *argument )
{
   set_pager_color( AT_IMMORT, ch );
   show_file( ch, WIZLIST_FILE );
}

/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;

   one_argument( argument, arg );

   set_char_color( AT_WHITE, ch );
   if( arg[0] == '\0' )
   {
      send_to_char( "[ Keyword  ] Option\r\n", ch );

      send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_NORECALL )
                    ? "[+NORECALL ] You fight to the death, link-dead or not.\r\n"
                    : "[-norecall ] You try to recall if fighting link-dead.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_AUTOEXIT )
                    ? "[+AUTOEXIT ] You automatically see exits.\r\n"
                    : "[-autoexit ] You don't automatically see exits.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_AUTOLOOT )
                    ? "[+AUTOLOOT ] You automatically loot corpses.\r\n"
                    : "[-autoloot ] You don't automatically loot corpses.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_AUTOSAC )
                    ? "[+AUTOSAC  ] You automatically sacrifice corpses.\r\n"
                    : "[-autosac  ] You don't automatically sacrifice corpses.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_AUTOGOLD )
                    ? "[+AUTOCRED ] You automatically split credits from kills in groups.\r\n"
                    : "[-autocred ] You don't automatically split credits from kills in groups.\r\n", ch );

      send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_GAG )
                    ? "[+GAG      ] You see only necessary battle text.\r\n"
                    : "[-gag      ] You see full battle text.\r\n", ch );

      send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON )
                    ? "[+PAGER    ] Long output is page-paused.\r\n"
                    : "[-pager    ] Long output scrolls to the end.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_BLANK )
                    ? "[+BLANK    ] You have a blank line before your prompt.\r\n"
                    : "[-blank    ] You have no blank line before your prompt.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_BRIEF )
                    ? "[+BRIEF    ] You see brief descriptions.\r\n" : "[-brief    ] You see long descriptions.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_COMBINE )
                    ? "[+COMBINE  ] You see object lists in combined format.\r\n"
                    : "[-combine  ] You see object lists in single format.\r\n", ch );

      send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO )
                    ? "[+NOINTRO  ] You don't see the ascii intro screen on login.\r\n"
                    : "[-nointro  ] You see the ascii intro screen on login.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_PROMPT )
                    ? "[+PROMPT   ] You have a prompt.\r\n" : "[-prompt   ] You don't have a prompt.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_TELNET_GA )
                    ? "[+TELNETGA ] You receive a telnet GA sequence.\r\n"
                    : "[-telnetga ] You don't receive a telnet GA sequence.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_ANSI )
                    ? "[+ANSI     ] You receive ANSI color sequences.\r\n"
                    : "[-ansi     ] You don't receive receive ANSI colors.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_SOUND )
                    ? "[+SOUND     ] You have MSP support.\r\n" : "[-sound     ] You don't have MSP support.\r\n", ch );


      send_to_char( IS_SET( ch->act, PLR_SHOVEDRAG )
                    ? "[+SHOVEDRAG] You allow yourself to be shoved and dragged around.\r\n"
                    : "[-shovedrag] You'd rather not be shoved or dragged around.\r\n", ch );

      if( IS_IMMORTAL( ch ) )
         send_to_char( IS_SET( ch->act, PLR_ROOMVNUM )
                       ? "[+VNUM     ] You can see the VNUM of a room.\r\n"
                       : "[-vnum     ] You do not see the VNUM of a room.\r\n", ch );

      if( IS_IMMORTAL( ch ) ) /* Added 10/16 by Kuran of SWR */
         send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_ROOM )
                       ? "[+ROOMFLAGS] You will see room flags.\r\n" : "[-roomflags] You will not see room flags.\r\n", ch );

      send_to_char( IS_SET( ch->act, PLR_SILENCE ) ? "[+SILENCE  ] You are silenced.\r\n" : "", ch );

      send_to_char( !IS_SET( ch->act, PLR_NO_EMOTE ) ? "" : "[-emote    ] You can't emote.\r\n", ch );

      send_to_char( !IS_SET( ch->act, PLR_NO_TELL ) ? "" : "[-tell     ] You can't use 'tell'.\r\n", ch );

      send_to_char( !IS_SET( ch->act, PLR_LITTERBUG )
                    ? "" : "[-litter  ] A convicted litterbug. You cannot drop anything.\r\n", ch );

   }
   else
   {
      bool fSet;
      int bit = 0;

      if( arg[0] == '+' )
         fSet = TRUE;
      else if( arg[0] == '-' )
         fSet = FALSE;
      else
      {
         send_to_char( "Config -option or +option?\r\n", ch );
         return;
      }

      if( !str_prefix( arg + 1, "autoexit" ) )
         bit = PLR_AUTOEXIT;
      else if( !str_prefix( arg + 1, "autoloot" ) )
         bit = PLR_AUTOLOOT;
      else if( !str_prefix( arg + 1, "autosac" ) )
         bit = PLR_AUTOSAC;
      else if( !str_prefix( arg + 1, "autocred" ) )
         bit = PLR_AUTOGOLD;
      else if( !str_prefix( arg + 1, "blank" ) )
         bit = PLR_BLANK;
      else if( !str_prefix( arg + 1, "brief" ) )
         bit = PLR_BRIEF;
      else if( !str_prefix( arg + 1, "combine" ) )
         bit = PLR_COMBINE;
      else if( !str_prefix( arg + 1, "prompt" ) )
         bit = PLR_PROMPT;
      else if( !str_prefix( arg + 1, "telnetga" ) )
         bit = PLR_TELNET_GA;
      else if( !str_prefix( arg + 1, "ansi" ) )
         bit = PLR_ANSI;
      else if( !str_prefix( arg + 1, "sound" ) )
         bit = PLR_SOUND;
      else if( !str_prefix( arg + 1, "nice" ) )
         bit = PLR_NICE;
      else if( !str_prefix( arg + 1, "shovedrag" ) )
         bit = PLR_SHOVEDRAG;
      else if( IS_IMMORTAL( ch ) && !str_prefix( arg + 1, "vnum" ) )
         bit = PLR_ROOMVNUM;

      if( bit )
      {

         if( fSet )
            SET_BIT( ch->act, bit );
         else
            REMOVE_BIT( ch->act, bit );
         send_to_char( "Ok.\r\n", ch );
         return;
      }
      else
      {
         if( !str_prefix( arg + 1, "norecall" ) )
            bit = PCFLAG_NORECALL;
         else if( !str_prefix( arg + 1, "nointro" ) )
            bit = PCFLAG_NOINTRO;
         else if( !str_prefix( arg + 1, "gag" ) )
            bit = PCFLAG_GAG;
         else if( !str_prefix( arg + 1, "pager" ) )
            bit = PCFLAG_PAGERON;
         else if( !str_prefix( arg + 1, "roomflags" ) && ( IS_IMMORTAL( ch ) ) )
            bit = PCFLAG_ROOM;
         else
         {
            send_to_char( "Config which option?\r\n", ch );
            return;
         }

         if( fSet )
            SET_BIT( ch->pcdata->flags, bit );
         else
            REMOVE_BIT( ch->pcdata->flags, bit );

         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }

   return;
}


void do_credits( CHAR_DATA * ch, const char *argument )
{
   do_help( ch, "credits" );
}


extern int top_area;

/*
void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = first_area;
    pArea2    = first_area;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
	ch_printf( ch, "%-39s%-39s\r\n",
	    pArea1->name, pArea2 ? pArea2->name : "" );
	pArea1 = pArea1->next;
	if ( pArea2 )
	    pArea2 = pArea2->next;
    }

    return;
}
*/

/* 
 * New do_areas with soft/hard level ranges 
 */

void do_areas( CHAR_DATA * ch, const char *argument )
{
   AREA_DATA *pArea;

   set_pager_color( AT_PLAIN, ch );
   send_to_pager( "\r\n   Author    |             Area                     | Recommended |  Enforced\r\n", ch );
   send_to_pager( "-------------+--------------------------------------+-------------+-----------\r\n", ch );

   for( pArea = first_area; pArea; pArea = pArea->next )
      pager_printf( ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \r\n",
                    pArea->author, pArea->name, pArea->low_soft_range,
                    pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range );
   return;
}

void do_afk( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->act, PLR_AFK ) )
   {
      REMOVE_BIT( ch->act, PLR_AFK );
      send_to_char( "You are no longer afk.\r\n", ch );
      act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      SET_BIT( ch->act, PLR_AFK );
      send_to_char( "You are now afk.\r\n", ch );
      act( AT_GREY, "$n is now afk.", ch, NULL, NULL, TO_ROOM );
      return;
   }
}

void do_slist( CHAR_DATA * ch, const char *argument )
{
   int sn, i;
   char skn[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int lowlev, hilev;
   int col = 0;
   int ability;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   lowlev = 1;
   hilev = 100;

   if( arg1[0] != '\0' )
      lowlev = atoi( arg1 );

   if( ( lowlev < 1 ) || ( lowlev > LEVEL_IMMORTAL ) )
      lowlev = 1;

   if( arg2[0] != '\0' )
      hilev = atoi( arg2 );

   if( hilev > 100 )
      hilev = 100;

   if( lowlev > hilev )
      lowlev = hilev;

   set_pager_color( AT_MAGIC, ch );
   send_to_pager( "&G&WSPELL & SKILL LIST\r\n", ch );
   send_to_pager( "&W&B------------------\r\n", ch );

   for( ability = 0; ability < MAX_ABILITY; ability++ )
   {
      if( ability == FORCE_ABILITY )
         continue;

      if( ability >= 0 )
         sprintf( skn, "\r\n&G&W%s\r\n", ability_name[ability] );
      else
         sprintf( skn, "\r\n&G&WGeneral Skills\r\n" );

      send_to_pager( skn, ch );
      for( i = lowlev; i <= hilev; i++ )
      {
         for( sn = 0; sn < top_sn; sn++ )
         {
            if( !skill_table[sn]->name )
               break;

            if( skill_table[sn]->guild != ability )
               continue;

            if( ch->pcdata->learned[sn] == 0 && SPELL_FLAG( skill_table[sn], SF_SECRETSKILL ) )
               continue;

            if( i == skill_table[sn]->min_level )
            {
               pager_printf( ch, "&B(&W%2d&B)%s %-18.18s&W  ",
                             i, skill_table[sn]->alignment >= 1001 ? "&B" :
                             skill_table[sn]->alignment <= -1001 ? "&R" : "&W", skill_table[sn]->name );

               if( ++col == 3 )
               {
                  pager_printf( ch, "\r\n" );
                  col = 0;
               }
            }
         }
      }
      if( col != 0 )
      {
         pager_printf( ch, "\r\n" );
         col = 0;
      }
   }
   return;
}

void do_whois( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   buf[0] = '\0';

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      send_to_char( "You must input the name of a player online.\r\n", ch );
      return;
   }

   strcat( buf, "0." );
   strcat( buf, argument );
   if( ( ( victim = get_char_world_ooc( ch, buf ) ) == NULL ) )
   {
      send_to_char( "No such player online. Remember, you must input the whole name of the player.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "That's not a player!\r\n", ch );
      return;
   }

   if( strlen( argument ) != strlen( victim->name ) )
   {
      send_to_char( "No such player online. Remember, you must input the whole name of the player.\r\n", ch );
      return;
   }

   if( !IS_IMMORTAL( ch ) && IS_SET( victim->act, PLR_WHOINVIS ) )
   {
      send_to_char( "No such player online. Remember, you must input the whole name of the player.\r\n", ch );
      return;
   }

   ch_printf( ch, "%s is a %s %s",
              victim->name,
              victim->sex == SEX_MALE ? "male" : victim->sex == SEX_FEMALE ? "female" : "neutral", npc_race[victim->race] );
   if( IS_IMMORTAL( ch ) )
      ch_printf( ch, " in room %d.\r\n", victim->in_room->vnum );
   else
      ch_printf( ch, ".\r\n" );

/*
  if ( victim->pcdata->clan )
  {
	if ( victim->pcdata->clan->clan_type == CLAN_CRIME )
	   send_to_char( ", and belongs to the crime family ", ch );
	else if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
	   send_to_char( ", and belongs to the guild ", ch );
	else
	   send_to_char( ", and belongs to organization ", ch );
	send_to_char( victim->pcdata->clan->name, ch );
  }
  send_to_char( ".\r\n", ch );
*/

   if( victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0' )
      ch_printf( ch, "%s's homepage can be found at %s.\r\n", victim->name, victim->pcdata->homepage );

   if( victim->pcdata->bio && victim->pcdata->bio[0] != '\0' )
      ch_printf( ch, "%s's personal bio:\r\n%s", victim->name, victim->pcdata->bio );

   if( IS_IMMORTAL( ch ) )
   {
      send_to_char( "----------------------------------------------------\r\n", ch );

      send_to_char( "Info for immortals:\r\n", ch );

      if( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
         ch_printf( ch, "%s was authorized by %s.\r\n", victim->name, victim->pcdata->authed_by );

      ch_printf( ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\r\n",
                 victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );
      if( victim->pcdata->pkills || victim->pcdata->pdeaths )
         ch_printf( ch, "%s has killed %d players, and been killed by a player %d times.\r\n",
                    victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );
      if( victim->pcdata->illegal_pk )
         ch_printf( ch, "%s has committed %d illegal player kills.\r\n", victim->name, victim->pcdata->illegal_pk );

      ch_printf( ch, "%s is %shelled at the moment.\r\n",
                 victim->name, ( victim->pcdata->release_date == 0 ) ? "not " : "" );

      if( victim->pcdata->release_date != 0 )
         ch_printf( ch, "%s was helled by %s, and will be released on %24.24s.\r\n",
                    victim->sex == SEX_MALE ? "He" :
                    victim->sex == SEX_FEMALE ? "She" : "It",
                    victim->pcdata->helled_by, ctime( &victim->pcdata->release_date ) );

      if( get_trust( victim ) < get_trust( ch ) )
      {
         sprintf( buf2, "list %s", buf );
         do_comment( ch, buf2 );
      }

      if( IS_SET( victim->act, PLR_SILENCE ) || IS_SET( victim->act, PLR_NO_EMOTE ) || IS_SET( victim->act, PLR_NO_TELL ) )
      {
         sprintf( buf2, "This player has the following flags set:" );
         if( IS_SET( victim->act, PLR_SILENCE ) )
            strcat( buf2, " silence" );
         if( IS_SET( victim->act, PLR_NO_EMOTE ) )
            strcat( buf2, " noemote" );
         if( IS_SET( victim->act, PLR_NO_TELL ) )
            strcat( buf2, " notell" );
         strcat( buf2, ".\r\n" );
         send_to_char( buf2, ch );
      }
      if( victim->desc && victim->desc->host[0] != '\0' )   /* added by Gorog */
      {
         sprintf( buf2, "%s's IP info: %s ", victim->name, victim->desc->hostip );
         if( get_trust( ch ) >= LEVEL_GOD )
         {
            strcat( buf2, victim->desc->host );
         }
         strcat( buf2, "\r\n" );
         send_to_char( buf2, ch );
      }
      if( get_trust( ch ) >= LEVEL_GOD && get_trust( ch ) >= get_trust( victim ) && victim->pcdata )
      {
         sprintf( buf2, "Email: %s\r\n", victim->pcdata->email );
         send_to_char( buf2, ch );
      }
   }
}

void do_pager( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;
   argument = one_argument( argument, arg );
   if( !*arg )
   {
      if( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
         do_config( ch, "-pager" );
      else
         do_config( ch, "+pager" );
      return;
   }
   if( !is_number( arg ) )
   {
      send_to_char( "Set page pausing to how many lines?\r\n", ch );
      return;
   }
   ch->pcdata->pagerlen = atoi( arg );
   if( ch->pcdata->pagerlen < 5 )
      ch->pcdata->pagerlen = 5;
   ch_printf( ch, "Page pausing set to %d lines.\r\n", ch->pcdata->pagerlen );
   return;
}

bool is_online( char *argument )
{
   DESCRIPTOR_DATA *d;

   if( argument[0] == '\0' )
      return FALSE;

   for( d = last_descriptor; d; d = d->prev )
   {
      CHAR_DATA *wch;

      if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING ) || d->original )
         continue;
      wch = d->original ? d->original : d->character;

      if( !str_cmp( argument, wch->name ) )
         return TRUE;
   }

   return FALSE;

}

void do_whoinvis( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->act, PLR_WHOINVIS ) )
   {
      REMOVE_BIT( ch->act, PLR_WHOINVIS );
      send_to_char( "You now show up on the who list.\r\n", ch );
   }
   else
   {
      SET_BIT( ch->act, PLR_WHOINVIS );
      send_to_char( "You now will not show up on the who list.\r\n", ch );
      return;
   }
}

void do_introduce( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   CHAR_DATA *rch;
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   FELLOW_DATA *vfellow;
   FELLOW_DATA *fellow;
   int count;
   char arg[MIL];

   argument = one_argument( argument, arg1 );
   argument = remand( argument );

   if( IS_NPC( ch ) )
      return;

   if( arg1[0] == '\0' || argument[0] == '\0' || ( !isalpha( argument[0] ) && !isdigit( argument[0] ) ) )
   {
      send_to_char( "&RSyntax: introduce <person/all> <your preferred name>\r\n", ch );
      return;
   }

   arg1[0] = UPPER( arg1[0] );
   snprintf( arg, MIL, "%s", argument );
   arg[0] = UPPER( arg[0] );

   if( ( str_cmp( arg1, "all" ) ) && ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "Schizophrenic, eh?\r\n", ch );
      return;
   }

   if( ch->pcdata->disguise[0] != '\0' )
   {
      send_to_char( "Perhaps you should remove your disguise first?\r\n", ch );
      return;
   }

   if( strlen( arg ) < 3 )
   {
      send_to_char( "Introductions must be at least 3 characters long.\r\n", ch );
      return;
   }

   if( strlen( arg ) > 40 )
      arg[40] = '\0';

   if( arg[1] == '.' )
      arg[1] = 'x';

   if( !str_cmp( arg, "Someone" ) )
   {
      send_to_char( "Nice try, ass.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "all" ) )
   {
      count = 0;
      for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      {
         if( IS_NPC( rch ) || rch == ch )
            continue;

         if( can_see( ch, rch ) )
         {
            sprintf( buf, "__%s ", rch->name );
            strcat( buf, argument );
            do_introduce( ch, buf );
            count++;
         }
      }
      if( count == 0 )
      {
         send_to_char( "There's nobody here.\r\n", ch );
         return;
      }
      return;
   }


   if( IS_NPC( victim ) || IS_NPC( ch ) )
   {
      send_to_char( "They aren't worth your time.\r\n", ch );
      return;
   }

   smash_tilde( arg );

   for( vfellow = victim->first_fellow; vfellow; vfellow = vfellow->next )
   {
      if( !str_cmp( vfellow->victim, ch->name ) )
      {
         if( !str_cmp( vfellow->knownas, arg ) )
         {
            ch_printf( ch, "They already know you as %s.\r\n", arg );
            return;
         }

         ch_printf( ch, "&GYou reintroduce yourself to %s as %s.\r\n", PERS( victim, ch ), arg );
         ch_printf( victim, "&G%s reintroduces %sself as %s.\r\n", PERS( ch, victim ),
                    ch->sex == 2 ? "her" : ch->sex == 1 ? "him" : "it", arg );

         STRFREE( vfellow->knownas );
         vfellow->knownas = STRALLOC( arg );
         return;
      }
   }
   ch_printf( victim, "&G%s introduces %sself as %s.\r\n", PERS( ch, victim ),
              ch->sex == 2 ? "her" : ch->sex == 1 ? "him" : "it", arg );

   CREATE( fellow, FELLOW_DATA, 1 );
   fellow->victim = QUICKLINK( ch->name );
   fellow->knownas = STRALLOC( arg );
   LINK( fellow, victim->first_fellow, victim->last_fellow, next, prev );

   ch_printf( ch, "&GYou introduce yourself to %s as %s.\r\n", PERS( victim, ch ), arg );
   return;
}

void do_remember( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   FELLOW_DATA *fellow;
   FELLOW_DATA *nfellow;
   char arg[MIL];

   argument = one_argument( argument, arg1 );
   argument = remand( argument );

   if( IS_NPC( ch ) )
      return;

   if( arg1[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "&RSyntax: remember <person> <name>\r\n", ch );
      return;
   }

   arg1[0] = UPPER( arg1[0] );
   snprintf( arg, MIL, "%s", argument );
   arg[0] = UPPER( arg[0] );

   if( !( victim = get_char_room( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "Remembering yourself... RP a little memory loss, would you?\r\n", ch );
      return;
   }

   if( strlen( arg ) < 3 )
   {
      send_to_char( "You find it quite hard to remember them with such a short name.\r\n", ch );
      return;
   }

   if( strlen( arg ) > 40 )
      arg[40] = '\0';

   if( arg[1] == '.' )
      arg[1] = 'x';

   if( !str_cmp( arg, "Someone" ) )
   {
      send_to_char( "Perhaps a little more descriptive?\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || IS_NPC( ch ) )
   {
      send_to_char( "They aren't worth your time.\r\n", ch );
      return;
   }

   smash_tilde( arg );

   for( fellow = ch->first_fellow; fellow; fellow = fellow->next )
   {
      if( !str_cmp( fellow->victim, victim->name ) )
      {
         if( !str_cmp( fellow->knownas, argument ) )
         {
            ch_printf( ch, "You already know them as %s.\r\n", arg );
            return;
         }

         ch_printf( ch, "%s will be remembered as %s.\r\n", PERS( victim, ch ), arg );

         STRFREE( fellow->knownas );
         fellow->knownas = STRALLOC( arg );
         return;
      }
   }
   ch_printf( ch, "%s will be remembered as %s.\r\n", PERS( victim, ch ), arg );

   CREATE( nfellow, FELLOW_DATA, 1 );
   nfellow->victim = QUICKLINK( victim->name );
   nfellow->knownas = STRALLOC( arg );
   LINK( nfellow, ch->first_fellow, ch->last_fellow, next, prev );
   return;
}

void do_describe( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg[MIL];
   FELLOW_DATA *fellow;
   FELLOW_DATA *nfellow;
   FELLOW_DATA *vfellow;

   argument = one_argument( argument, arg1 );
   argument = remand( argument );

   if( IS_NPC( ch ) )
      return;

   if( arg1[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "&RSyntax: describe <to person> <person you are describing>\r\n", ch );
      return;
   }

   arg1[0] = UPPER( arg1[0] );
   snprintf( arg, MIL, "%s", argument );
   arg[0] = UPPER( arg[0] );

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "You cannot describe someone to yourself.\r\n", ch );
      return;
   }

   if( strlen( arg ) < 3 )
   {
      send_to_char( "You find it hard to describe someone based on such a short name.\r\n", ch );
      return;
   }

   if( strlen( arg ) > 40 )
      arg[40] = '\0';

   if( arg[1] == '.' )
      arg[1] = 'x';

   if( !str_cmp( arg, "Someone" ) )
   {
      send_to_char( "Perhaps a little more descriptive?\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || IS_NPC( ch ) )
   {
      send_to_char( "They aren't worth your time.\r\n", ch );
      return;
   }

   smash_tilde( arg );

   for( fellow = ch->first_fellow; fellow; fellow = fellow->next )
   {
      if( nifty_is_name( argument, fellow->knownas ) )
      {
         for( vfellow = victim->first_fellow; vfellow; vfellow = vfellow->next )
         {
            if( !str_cmp( fellow->victim, vfellow->victim ) )
            {
               if( !str_cmp( fellow->knownas, vfellow->knownas ) )
               {
                  ch_printf( ch, "They already know them as %s.\r\n", fellow->knownas );
                  return;
               }
               ch_printf( victim, "%s describes %s to you. They sound quite similar to %s.n\r", PERS( ch, victim ),
                          fellow->knownas, vfellow->knownas );
               ch_printf( ch, "You describe %s to %s.n\r", fellow->knownas, PERS( victim, ch ) );
               STRFREE( vfellow->knownas );
               vfellow->knownas = STRALLOC( fellow->knownas );
               return;
            }
         }
         ch_printf( ch, "You describe %s to %s.\r\n", fellow->knownas, PERS( victim, ch ) );
         ch_printf( victim, "%s describes %s to you.\r\n", PERS( ch, victim ), fellow->knownas );
         CREATE( nfellow, FELLOW_DATA, 1 );
         nfellow->victim = QUICKLINK( victim->name );
         nfellow->knownas = STRALLOC( fellow->knownas );
         LINK( nfellow, victim->first_fellow, victim->last_fellow, next, prev );
         return;
      }
   }
   send_to_char( "You don't seem to known anyone like that...\r\n", ch );
   return;
}
