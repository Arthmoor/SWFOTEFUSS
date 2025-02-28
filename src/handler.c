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

#define BFS_MARK         BV01

extern int top_exit;
extern int top_ed;
extern int top_affect;
extern int cur_qobjs;
extern int cur_qchars;
extern CHAR_DATA *gch_prev;
extern OBJ_DATA *gobj_prev;
extern REL_DATA *first_relation;
extern REL_DATA *last_relation;

CHAR_DATA *cur_char;
ROOM_INDEX_DATA *cur_room;
bool cur_char_died;
ch_ret global_retcode;

int cur_obj;
int cur_obj_serial;
bool cur_obj_extracted;
obj_ret global_objcode;

OBJ_DATA *group_object( OBJ_DATA * obj1, OBJ_DATA * obj2 );
void update_room_reset( CHAR_DATA *ch, bool setting );
void delete_reset( RESET_DATA *pReset );

void room_explode_1( OBJ_DATA * obj, CHAR_DATA * xch, ROOM_INDEX_DATA * room, int blast )
{
   CHAR_DATA *rch;
   CHAR_DATA *rnext;
   OBJ_DATA *robj;
   OBJ_DATA *robj_next;
   int dam;

   if( IS_SET( room->room_flags, BFS_MARK ) )
      return;

   SET_BIT( room->room_flags, BFS_MARK );
   for( rch = room->first_person; rch; rch = rnext )
   {
      rnext = rch->next_in_room;
      act( AT_WHITE, "The shockwave from a massive explosion rips through your body!", rch, obj, NULL, TO_CHAR );

      dam = number_range( obj->value[0], obj->value[1] );
      damage( rch, rch, dam, TYPE_UNDEFINED );
      if( !char_died( rch ) )
      {
         if( IS_NPC( rch ) )
         {
            if( IS_SET( rch->act, ACT_SENTINEL ) )
            {
               rch->was_sentinel = rch->in_room;
               REMOVE_BIT( rch->act, ACT_SENTINEL );
            }
            if( obj->item_type != ITEM_SHIPBOMB )
            {
               start_hating( rch, xch );
               start_hunting( rch, xch );
            }
         }
      }
   }

   for( robj = room->first_content; robj; robj = robj_next )
   {
      robj_next = robj->next_content;
      if( robj != obj && robj->item_type != ITEM_SPACECRAFT && robj->item_type != ITEM_SCRAPS
          && robj->item_type != ITEM_CORPSE_NPC && robj->item_type != ITEM_CORPSE_PC
          && robj->item_type != ITEM_DROID_CORPSE )
         make_scraps( robj );
   }

   /*
    * other rooms 
    */
   {
      EXIT_DATA *pexit;

      for( pexit = room->first_exit; pexit; pexit = pexit->next )
      {
         if( pexit->to_room && pexit->to_room != room )
         {
            if( blast > 0 )
            {
               int roomblast;
               roomblast = blast - 1;
               room_explode_1( obj, xch, pexit->to_room, roomblast );
            }
            else
               echo_to_room( AT_WHITE, pexit->to_room, "You hear a loud EXPLOSION not to far from here." );
         }
      }
   }
}

void room_explode_2( ROOM_INDEX_DATA * room, int blast )
{
   if( !IS_SET( room->room_flags, BFS_MARK ) )
      return;

   REMOVE_BIT( room->room_flags, BFS_MARK );

   if( blast > 0 )
   {
      int roomblast;
      EXIT_DATA *pexit;

      for( pexit = room->first_exit; pexit; pexit = pexit->next )
      {
         if( pexit->to_room && pexit->to_room != room )
         {
            roomblast = blast - 1;
            room_explode_2( pexit->to_room, roomblast );
         }
      }
   }
}

void room_explode( OBJ_DATA * obj, CHAR_DATA * xch, ROOM_INDEX_DATA * room )
{
   int blast;
   blast = ( int )( obj->value[1] / 500 );
   room_explode_1( obj, xch, room, blast );
   room_explode_2( room, blast );
}

void explode( OBJ_DATA * obj )
{
   if( obj->armed_by )
   {
      ROOM_INDEX_DATA *room;
      CHAR_DATA *xch;
      bool held = FALSE;

      for( xch = first_char; xch; xch = xch->next )
         if( !IS_NPC( xch ) )
/* && nifty_is_name( obj->armed_by, xch->name ) Temp removed to see what happens*/
         {
            if( obj->carried_by == xch )
            {
               act( AT_WHITE, "$p EXPLODES in $n's hands!", obj->carried_by, obj, NULL, TO_ROOM );
               act( AT_WHITE, "$p EXPLODES in your hands!", obj->carried_by, obj, NULL, TO_CHAR );
               room = obj->carried_by->in_room;
               held = TRUE;
            }
            else if( obj->in_room )
               room = obj->in_room;
            else
               room = NULL;

            if( room )
            {
               if( !held && room->first_person )
                  act( AT_WHITE, "$p EXPLODES!", room->first_person, obj, NULL, TO_ROOM );
               room_explode( obj, xch, room );
               break;
            }
         }
   }
   make_scraps( obj );
}

bool is_wizvis( CHAR_DATA * ch, CHAR_DATA * victim )
{
   if( !IS_NPC( victim ) && IS_SET( victim->act, PLR_WIZINVIS ) && get_trust( ch ) < victim->pcdata->wizinvis )
      return FALSE;

   return TRUE;
}

/*
 * Return how much exp a char has
 */
int get_exp( CHAR_DATA * ch, int ability )
{
   if( ability >= MAX_ABILITY || ability < 0 )
      return 0;

   return ch->experience[ability];
}

int umin( int check, int ncheck )
{
   if( check < ncheck )
      return check;
   return ncheck;
}

int umax( int check, int ncheck )
{
   if( check > ncheck )
      return check;
   return ncheck;
}

int urange( int mincheck, int check, int maxcheck )
{
   if( check < mincheck )
      return mincheck;
   if( check > maxcheck )
      return maxcheck;
   return check;
}

/*
 * Calculate roughly how much experience a character is worth
 */
int get_exp_worth( CHAR_DATA * ch )
{
   int wexp;

   wexp = ch->skill_level[COMBAT_ABILITY] * ch->top_level * 50;
   wexp += ch->max_hit * 2;
   wexp -= ( ch->armor - 50 ) * 2;
   wexp += ( ch->barenumdie * ch->baresizedie + GET_DAMROLL( ch ) ) * 50;
   wexp += GET_HITROLL( ch ) * ch->top_level * 10;
   if( IS_AFFECTED( ch, AFF_SANCTUARY ) )
      wexp += ( int )( wexp * 1.5 );
   if( IS_AFFECTED( ch, AFF_FIRESHIELD ) )
      wexp += ( int )( wexp * 1.2 );
   if( IS_AFFECTED( ch, AFF_ICESHIELD ) )
      wexp += ( int )( wexp * 1.2 ); 
   if( IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
      wexp += ( int )( wexp * 1.2 );
   wexp = URANGE( MIN_EXP_WORTH, wexp, MAX_EXP_WORTH );

   return wexp;
}


/*								-Thoric
 * Return how much experience is required for ch to get to a certain level
 */
int exp_level( short level )
{
   int lvl;

   lvl = UMAX( 0, level - 1 );

   return ( lvl * lvl * 500 );
}

/*
 * Get what level ch is based on exp
 */
short level_exp( CHAR_DATA * ch, int lexp )
{
   int x, lastx, y, tmp;

   x = LEVEL_SUPREME;
   lastx = x;
   y = 0;
   while( !y )
   {
      tmp = exp_level( x );
      lastx = x;
      if( tmp > lexp )
         x /= 2;
      else if( lastx != x )
         x += ( x / 2 );
      else
         y = x;
   }
   if( y < 1 )
      y = 1;
   if( y > LEVEL_SUPREME )
      y = LEVEL_SUPREME;
   return y;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
short get_trust( CHAR_DATA * ch )
{
   if( !ch )
      return 0;

   if( ch->desc )
      if( ch->desc->original )
         ch = ch->desc->original;

   if( ch->trust != 0 )
      return ch->trust;

   if( IS_NPC( ch ) && ch->top_level >= LEVEL_AVATAR )
      return LEVEL_AVATAR;

   if( ch->top_level >= LEVEL_NEOPHYTE && IS_RETIRED( ch ) )
      return LEVEL_NEOPHYTE;

   return ch->top_level;
}


/*
 * Retrieve a character's age.
 */
short get_age( CHAR_DATA * ch )
{
   return 17 + ( ch->played + ( current_time - ch->logon ) ) / 14400;
}

/*
   get_curr_xxx have been changed to alot for stat bonuses, even
   after trained to max. If you feel this is incorrect, change
   at will.
   Old Format Was:

   short get_curr_str( CHAR_DATA *ch )
   {
       short max;

       max  = 20 + race_table[ch->race].str_plus;
       max = UMIN(max,25);
       return URANGE( 3, ch->perm_str + ch->mod_str, max );
   }
*/

/*
 * Retrieve character's current strength.
 */
short get_curr_str( CHAR_DATA * ch )
{
   return URANGE( 3, ch->perm_str + ch->mod_str, 25 );
}

/*
 * Retrieve character's current intelligence.
 */
short get_curr_int( CHAR_DATA * ch )
{
   return URANGE( 3, ch->perm_int + ch->mod_int, 25 );
}

/*
 * Retrieve character's current wisdom.
 */
short get_curr_wis( CHAR_DATA * ch )
{
   return URANGE( 3, ch->perm_wis + ch->mod_wis, 25 );
}

/*
 * Retrieve character's current dexterity.
 */
short get_curr_dex( CHAR_DATA * ch )
{
   return URANGE( 3, ch->perm_dex + ch->mod_dex, 25 );
}

/*
 * Retrieve character's current constitution.
 */
short get_curr_con( CHAR_DATA * ch )
{
   return URANGE( 3, ch->perm_con + ch->mod_con, 25 );
}

/*
 * Retrieve character's current charisma.
 */
short get_curr_cha( CHAR_DATA * ch )
{
   return URANGE( 3, ch->perm_cha + ch->mod_cha, 25 );
}

/*
 * Retrieve character's current luck.
 */
short get_curr_lck( CHAR_DATA * ch )
{
   return URANGE( 3, ch->perm_lck + ch->mod_lck, 25 );
}

/* Not changed for obvious reasons */
short get_curr_frc( CHAR_DATA * ch )
{
   short max;

   max = 20 + race_table[ch->race].frc_plus;
   max = UMIN( max, 25 );

   return URANGE( 0, ch->perm_frc + ch->mod_frc, max );
}

/*
 * Retrieve a character's carry capacity.
 * Vastly reduced (finally) due to containers		-Thoric
 */
int can_carry_n( CHAR_DATA * ch )
{
   int penalty = 0;

   if( !IS_NPC( ch ) && get_trust( ch ) >= LEVEL_IMMORTAL )
      return get_trust( ch ) * 200;

   if( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
      return 0;

   if( get_eq_char( ch, WEAR_WIELD ) )
      ++penalty;
   if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
      ++penalty;
   if( get_eq_char( ch, WEAR_MISSILE_WIELD ) )
      ++penalty;
   if( get_eq_char( ch, WEAR_HOLD ) )
      ++penalty;
   if( get_eq_char( ch, WEAR_SHIELD ) )
      ++penalty;
   return URANGE( 5, ( ch->top_level + 15 ) / 5 + get_curr_dex( ch ) - 13 - penalty, 20 );
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA * ch )
{
   if( !IS_NPC( ch ) && get_trust( ch ) >= LEVEL_IMMORTAL )
      return 1000000;

   if( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
      return 0;

   return str_app[get_curr_str( ch )].carry;
}

/*
 * See if a player/mob can take a piece of prototype eq		-Thoric
 */
bool can_take_proto( CHAR_DATA * ch )
{
   if( IS_IMMORTAL( ch ) )
      return TRUE;
   else if( IS_NPC( ch ) && IS_SET( ch->act, ACT_PROTOTYPE ) )
      return TRUE;
   else
      return FALSE;
}

/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *str, const char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_cmp( str, name ) )
         return TRUE;
   }
}

bool is_name_prefix( const char *str, const char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_prefix( str, name ) )
         return TRUE;
   }
}

/*
 * See if a string is one of the names of an object.		-Thoric
 * Treats a dash as a word delimiter as well as a space
 */
bool is_name2( const char *str, const char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument2( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_cmp( str, name ) )
         return TRUE;
   }
}

bool is_name2_prefix( const char *str, const char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument2( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_prefix( str, name ) )
         return TRUE;
   }
}

/*								-Thoric
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name( const char *str, const char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   if( !str || str[0] == '\0' )
      return FALSE;

   for( ;; )
   {
      str = one_argument2( str, name );
      if( name[0] == '\0' )
         return TRUE;
      if( !is_name2( name, namelist ) )
         return FALSE;
   }
}

bool nifty_is_name_prefix( const char *str, const char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   if( !str || str[0] == '\0' )
      return FALSE;

   for( ;; )
   {
      str = one_argument2( str, name );
      if( name[0] == '\0' )
         return TRUE;
      if( !is_name2_prefix( name, namelist ) )
         return FALSE;
   }
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd )
{
   OBJ_DATA *wield;
   int mod;
   struct skill_type *skill;
   ch_ret retcode;

   mod = paf->modifier;

   if( fAdd )
   {
      SET_BIT( ch->affected_by, paf->bitvector );
   }
   else
   {
      REMOVE_BIT( ch->affected_by, paf->bitvector );
      /*
       * might be an idea to have a duration removespell which returns
       * the spell after the duration... but would have to store
       * the removed spell's information somewhere...    -Thoric
       */
      switch ( paf->location % REVERSE_APPLY )
      {
         case APPLY_AFFECT:
            REMOVE_BIT( ch->affected_by, mod );
            return;
         case APPLY_RESISTANT:
            REMOVE_BIT( ch->resistant, mod );
            return;
         case APPLY_IMMUNE:
            REMOVE_BIT( ch->immune, mod );
            return;
         case APPLY_SUSCEPTIBLE:
            REMOVE_BIT( ch->susceptible, mod );
            return;
         case APPLY_REMOVE:
            SET_BIT( ch->affected_by, mod );
            return;
         default:
            break;
      }
      mod = 0 - mod;
   }

   switch ( paf->location % REVERSE_APPLY )
   {
      default:
         bug( "%s: unknown location %d.", __func__, paf->location );
         return;

      case APPLY_NONE:
         break;
      case APPLY_STR:
         ch->mod_str += mod;
         break;
      case APPLY_DEX:
         ch->mod_dex += mod;
         break;
      case APPLY_INT:
         ch->mod_int += mod;
         break;
      case APPLY_WIS:
         ch->mod_wis += mod;
         break;
      case APPLY_CON:
         ch->mod_con += mod;
         break;
      case APPLY_CHA:
         ch->mod_cha += mod;
         break;
      case APPLY_LCK:
         ch->mod_lck += mod;
         break;
      case APPLY_SEX:
         ch->sex = ( ch->sex + mod ) % 3;
         if( ch->sex < 0 )
            ch->sex += 2;
         ch->sex = URANGE( 0, ch->sex, 2 );
         break;
      case APPLY_LEVEL:
         break;
      case APPLY_AGE:
         break;
      case APPLY_HEIGHT:
         ch->height += mod;
         break;
      case APPLY_WEIGHT:
         ch->weight += mod;
         break;
      case APPLY_MANA:
         ch->max_mana += mod;
         break;
      case APPLY_HIT:
         ch->max_hit += mod;
         break;
      case APPLY_MOVE:
         ch->max_move += mod;
         break;
      case APPLY_GOLD:
         break;
      case APPLY_EXP:
         break;
      case APPLY_AC:
         ch->armor += mod;
         break;
      case APPLY_HITROLL:
         ch->hitroll += mod;
         break;
      case APPLY_DAMROLL:
         ch->damroll += mod;
         break;
      case APPLY_SAVING_POISON:
         ch->saving_poison_death += mod;
         break;
      case APPLY_SAVING_ROD:
         ch->saving_wand += mod;
         break;
      case APPLY_SAVING_PARA:
         ch->saving_para_petri += mod;
         break;
      case APPLY_SAVING_BREATH:
         ch->saving_breath += mod;
         break;
      case APPLY_SAVING_SPELL:
         ch->saving_spell_staff += mod;
         break;
      case APPLY_AFFECT:
         SET_BIT( ch->affected_by, mod );
         break;
      case APPLY_RESISTANT:
         SET_BIT( ch->resistant, mod );
         break;
      case APPLY_IMMUNE:
         SET_BIT( ch->immune, mod );
         break;
      case APPLY_SUSCEPTIBLE:
         SET_BIT( ch->susceptible, mod );
         break;
      case APPLY_WEAPONSPELL:   /* see fight.c */
         break;
      case APPLY_REMOVE:
         REMOVE_BIT( ch->affected_by, mod );
         break;

      case APPLY_FULL:
         if( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_FULL] = URANGE( 0, ch->pcdata->condition[COND_FULL] + mod, 48 );
         break;

      case APPLY_THIRST:
         if( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_THIRST] = URANGE( 0, ch->pcdata->condition[COND_THIRST] + mod, 48 );
         break;

      case APPLY_DRUNK:
         if( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_DRUNK] = URANGE( 0, ch->pcdata->condition[COND_DRUNK] + mod, 48 );
         break;

      case APPLY_MENTALSTATE:
         ch->mental_state = URANGE( -100, ch->mental_state + mod, 100 );
         break;
      case APPLY_EMOTION:
         ch->emotional_state = URANGE( -100, ch->emotional_state + mod, 100 );
         break;

      case APPLY_STRIPSN:
         if( IS_VALID_SN( mod ) )
            affect_strip( ch, mod );
         else
            bug( "%s: APPLY_STRIPSN invalid sn %d", __func__, mod );
         break;

/* spell cast upon wear/removal of an object	-Thoric */
      case APPLY_WEARSPELL:
      case APPLY_REMOVESPELL:
         if( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) || IS_SET( ch->immune, RIS_MAGIC ) || ( ( paf->location % REVERSE_APPLY ) == APPLY_WEARSPELL && !fAdd ) || ( ( paf->location % REVERSE_APPLY ) == APPLY_REMOVESPELL && !fAdd ) || saving_char == ch /* so save/quit doesn't trigger */
             || loading_char == ch )   /* so loading doesn't trigger */
            return;

         mod = abs( mod );
         if( IS_VALID_SN( mod ) && ( skill = skill_table[mod] ) != NULL && skill->type == SKILL_SPELL )
            if( ( retcode = ( *skill->spell_fun ) ( mod, ch->skill_level[FORCE_ABILITY], ch, ch ) ) == rCHAR_DIED
                || char_died( ch ) )
               return;
         break;


/* skill apply types	-Thoric */

      case APPLY_PALM: /* not implemented yet */
         break;
      case APPLY_TRACK:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_track] > 0 )
            ch->pcdata->learned[gsn_track] = UMAX( 1, ch->pcdata->learned[gsn_track] + mod );
         break;
      case APPLY_HIDE:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_hide] > 0 )
            ch->pcdata->learned[gsn_hide] = UMAX( 1, ch->pcdata->learned[gsn_hide] + mod );
         break;
      case APPLY_STEAL:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_steal] > 0 )
            ch->pcdata->learned[gsn_steal] = UMAX( 1, ch->pcdata->learned[gsn_steal] + mod );
         break;
      case APPLY_SNEAK:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_sneak] > 0 )
            ch->pcdata->learned[gsn_sneak] = UMAX( 1, ch->pcdata->learned[gsn_sneak] + mod );
         break;
      case APPLY_PICK:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_pick_lock] > 0 )
            ch->pcdata->learned[gsn_pick_lock] = UMAX( 1, ch->pcdata->learned[gsn_pick_lock] + mod );
         break;
      case APPLY_BACKSTAB:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_backstab] > 0 )
            ch->pcdata->learned[gsn_backstab] = UMAX( 1, ch->pcdata->learned[gsn_backstab] + mod );
         break;
      case APPLY_DETRAP:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_detrap] > 0 )
            ch->pcdata->learned[gsn_detrap] = UMAX( 1, ch->pcdata->learned[gsn_detrap] + mod );
         break;
      case APPLY_DODGE:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_dodge] > 0 )
            ch->pcdata->learned[gsn_dodge] = UMAX( 1, ch->pcdata->learned[gsn_dodge] + mod );
         break;
      case APPLY_PEEK:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_peek] > 0 )
            ch->pcdata->learned[gsn_peek] = UMAX( 1, ch->pcdata->learned[gsn_peek] + mod );
         break;
      case APPLY_SCAN:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_scan] > 0 )
            ch->pcdata->learned[gsn_scan] = UMAX( 1, ch->pcdata->learned[gsn_scan] + mod );
         break;
      case APPLY_GOUGE:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_gouge] > 0 )
            ch->pcdata->learned[gsn_gouge] = UMAX( 1, ch->pcdata->learned[gsn_gouge] + mod );
         break;
      case APPLY_SEARCH:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_search] > 0 )
            ch->pcdata->learned[gsn_search] = UMAX( 1, ch->pcdata->learned[gsn_search] + mod );
         break;
      case APPLY_DIG:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_dig] > 0 )
            ch->pcdata->learned[gsn_dig] = UMAX( 1, ch->pcdata->learned[gsn_dig] + mod );
         break;
      case APPLY_MOUNT:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_mount] > 0 )
            ch->pcdata->learned[gsn_mount] = UMAX( 1, ch->pcdata->learned[gsn_mount] + mod );
         break;
      case APPLY_DISARM:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_disarm] > 0 )
            ch->pcdata->learned[gsn_disarm] = UMAX( 1, ch->pcdata->learned[gsn_disarm] + mod );
         break;
      case APPLY_KICK:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_kick] > 0 )
            ch->pcdata->learned[gsn_kick] = UMAX( 1, ch->pcdata->learned[gsn_kick] + mod );
         break;
      case APPLY_PARRY:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_parry] > 0 )
            ch->pcdata->learned[gsn_parry] = UMAX( 1, ch->pcdata->learned[gsn_parry] + mod );
         break;
      case APPLY_BASH:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_bash] > 0 )
            ch->pcdata->learned[gsn_bash] = UMAX( 1, ch->pcdata->learned[gsn_bash] + mod );
         break;
      case APPLY_STUN:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_stun] > 0 )
            ch->pcdata->learned[gsn_stun] = UMAX( 1, ch->pcdata->learned[gsn_stun] + mod );
         break;
      case APPLY_PUNCH:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_punch] > 0 )
            ch->pcdata->learned[gsn_punch] = UMAX( 1, ch->pcdata->learned[gsn_punch] + mod );
         break;
      case APPLY_CLIMB:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_climb] > 0 )
            ch->pcdata->learned[gsn_climb] = UMAX( 1, ch->pcdata->learned[gsn_climb] + mod );
         break;
      case APPLY_GRIP:
         if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_grip] > 0 )
            ch->pcdata->learned[gsn_grip] = UMAX( 1, ch->pcdata->learned[gsn_grip] + mod );
         break;
   }

   /*
    * Check for weapon wielding.
    * Guard against recursion (for weapons with affects).
    */
   if( !IS_NPC( ch )
       && saving_char != ch
       && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL && get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
   {
      static int depth;

      if( depth == 0 )
      {
         depth++;
         act( AT_ACTION, "You are too weak to wield $p any longer.", ch, wield, NULL, TO_CHAR );
         act( AT_ACTION, "$n stops wielding $p.", ch, wield, NULL, TO_ROOM );
         unequip_char( ch, wield );
         depth--;
      }
   }

   return;
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   AFFECT_DATA *paf_new;

   if( !ch )
   {
      bug( "%s: NULL ch!", __func__ );
      return;
   }

   if( !paf )
   {
      bug( "%s: NULL paf!", __func__ );
      return;
   }

   CREATE( paf_new, AFFECT_DATA, 1 );
   LINK( paf_new, ch->first_affect, ch->last_affect, next, prev );
   paf_new->type = paf->type;
   paf_new->duration = paf->duration;
   paf_new->location = paf->location;
   paf_new->modifier = paf->modifier;
   paf_new->bitvector = paf->bitvector;

   affect_modify( ch, paf_new, TRUE );
   return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   if( !ch->first_affect )
   {
      bug( "%s: no affect.", __func__ );
      return;
   }

   affect_modify( ch, paf, FALSE );

   UNLINK( paf, ch->first_affect, ch->last_affect, next, prev );
   DISPOSE( paf );
   return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA * ch, int sn )
{
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;

   for( paf = ch->first_affect; paf; paf = paf_next )
   {
      paf_next = paf->next;
      if( paf->type == sn )
         affect_remove( ch, paf );
   }

   return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA * ch, int sn )
{
   AFFECT_DATA *paf;

   for( paf = ch->first_affect; paf; paf = paf->next )
      if( paf->type == sn )
         return TRUE;

   return FALSE;
}

/*
 * Add or enhance an affect.
 * Limitations put in place by Thoric, they may be high... but at least
 * they're there :)
 */
void affect_join( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   AFFECT_DATA *paf_old;

   for( paf_old = ch->first_affect; paf_old; paf_old = paf_old->next )
      if( paf_old->type == paf->type )
      {
         paf->duration = UMIN( 1000000, paf->duration + paf_old->duration );
         if( paf->modifier )
            paf->modifier = UMIN( 5000, paf->modifier + paf_old->modifier );
         else
            paf->modifier = paf_old->modifier;
         affect_remove( ch, paf_old );
         break;
      }

   affect_to_char( ch, paf );
   return;
}

/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA * ch )
{
   OBJ_DATA *obj;

   if( !ch )
   {
      bug( "%s: NULL char.", __func__ );
      return;
   }

   if( !ch->in_room )
   {
      bug( "%s: NULL room: %s", __func__, ch->name );
      return;
   }

   if( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
       && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room->light > 0 )
      --ch->in_room->light;

   if( ch->in_room->area )
      --ch->in_room->area->nplayer;

   UNLINK( ch, ch->in_room->first_person, ch->in_room->last_person, next_in_room, prev_in_room );
   ch->in_room = NULL;
   ch->next_in_room = NULL;
   ch->prev_in_room = NULL;

   if( !IS_NPC( ch ) && get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
      remove_timer( ch, TIMER_SHOVEDRAG );

   return;
}

/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex )
{
   OBJ_DATA *obj;

   if( !ch )
   {
      bug( "%s: NULL ch!", __func__ );
      return;
   }

   if( !pRoomIndex || !get_room_index( pRoomIndex->vnum ) )
   {
      bug( "%s: %s -> NULL room!  Putting char in limbo (%d)", __func__, ch->name, ROOM_VNUM_LIMBO );
      /*
       * This used to just return, but there was a problem with crashing
       * and I saw no reason not to just put the char in limbo.  -Narn
       */
      pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
   }

   ch->in_room = pRoomIndex;
   if( ch->home_vnum < 1 )
      ch->home_vnum = ch->in_room->vnum;
   LINK( ch, pRoomIndex->first_person, pRoomIndex->last_person, next_in_room, prev_in_room );

   if( ch->in_room->area )
      if( ++ch->in_room->area->nplayer > ch->in_room->area->max_players )
         ch->in_room->area->max_players = ch->in_room->area->nplayer;

   if( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
      ++ch->in_room->light;

   if( !IS_NPC( ch ) && IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && get_timer( ch, TIMER_SHOVEDRAG ) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );   /*-30 Seconds-*/

   return;
}


/*
 * Give an obj to a char.
 */
OBJ_DATA *obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch )
{
   OBJ_DATA *otmp;
   OBJ_DATA *oret = obj;
   bool skipgroup, grouped;
   int oweight = get_obj_weight( obj );
   int onum = get_obj_number( obj );
   int wear_loc = obj->wear_loc;
   int extra_flags = obj->extra_flags;

   skipgroup = FALSE;
   grouped = FALSE;

   if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
   {
      if( !IS_IMMORTAL( ch ) && ( !IS_NPC( ch ) && !IS_SET( ch->act, ACT_PROTOTYPE ) ) )
         return obj_to_room( obj, ch->in_room );
   }

   if( loading_char == ch )
   {
      int x, y;
      for( x = 0; x < MAX_WEAR; x++ )
         for( y = 0; y < MAX_LAYERS; y++ )
            if( save_equipment[x][y] == obj )
            {
               skipgroup = TRUE;
               break;
            }
   }

   if( IS_NPC( ch ) && ch->pIndexData->pShop )
      skipgroup = TRUE;

   if( !skipgroup )
      for( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
         if( ( oret = group_object( otmp, obj ) ) == otmp )
         {
            grouped = TRUE;
            break;
         }

   if( !grouped )
   {
      LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
      obj->carried_by = ch;
      obj->in_room = NULL;
      obj->in_obj = NULL;
   }
   if( wear_loc == WEAR_NONE )
   {
      ch->carry_number += onum;
      ch->carry_weight += oweight;
   }
   else if( !IS_SET( extra_flags, ITEM_MAGIC ) )
      ch->carry_weight += oweight;
   return ( oret ? oret : obj );
}

/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA * obj )
{
   CHAR_DATA *ch;

   if( ( ch = obj->carried_by ) == NULL )
   {
      bug( "%s: null ch.", __func__ );
      return;
   }

   if( obj->wear_loc != WEAR_NONE )
      unequip_char( ch, obj );

   /*
    * obj may drop during unequip... 
    */
   if( !obj->carried_by )
      return;

   UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

   if( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
      empty_obj( obj, NULL, NULL );

   obj->in_room = NULL;
   obj->carried_by = NULL;
   ch->carry_number -= get_obj_number( obj );
   ch->carry_weight -= get_obj_weight( obj );
   return;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA * obj, int iWear )
{
   if( obj->item_type != ITEM_ARMOR )
      return 0;

   switch ( iWear )
   {
      case WEAR_BODY:
         return 3 * obj->value[0];
      case WEAR_HEAD:
         return 2 * obj->value[0];
      case WEAR_LEGS:
         return 2 * obj->value[0];
      case WEAR_FEET:
         return obj->value[0];
      case WEAR_HANDS:
         return obj->value[0];
      case WEAR_ARMS:
         return obj->value[0];
      case WEAR_SHIELD:
         return obj->value[0];
      case WEAR_FINGER_L:
         return obj->value[0];
      case WEAR_FINGER_R:
         return obj->value[0];
      case WEAR_NECK_1:
         return obj->value[0];
      case WEAR_NECK_2:
         return obj->value[0];
      case WEAR_ABOUT:
         return 2 * obj->value[0];
      case WEAR_WAIST:
         return obj->value[0];
      case WEAR_WRIST_L:
         return obj->value[0];
      case WEAR_WRIST_R:
         return obj->value[0];
      case WEAR_HOLD:
         return obj->value[0];
      case WEAR_EYES:
         return obj->value[0];
      case WEAR_BACK:
         return obj->value[0];
   }

   return 0;
}

/*
 * Find a piece of eq on a character.
 * Will pick the top layer if clothing is layered.		-Thoric
 */
OBJ_DATA *get_eq_char( CHAR_DATA * ch, int iWear )
{
   OBJ_DATA *obj, *maxobj = NULL;

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
      if( obj->wear_loc == iWear )
      {
         if( !obj->pIndexData->layers )
            return obj;
         else if( !maxobj || obj->pIndexData->layers > maxobj->pIndexData->layers )
            maxobj = obj;
      }

   return maxobj;
}

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA * ch, OBJ_DATA * obj, int iWear )
{
   AFFECT_DATA *paf;
   OBJ_DATA *otmp;

   if( obj->carried_by != ch )
   {
      bug( "%s: obj not being carried by ch!", __func__ );
      return;
   }

   if( ( otmp = get_eq_char( ch, iWear ) ) != NULL && ( !otmp->pIndexData->layers || !obj->pIndexData->layers ) )
   {
      bug( "%s: already equipped (%s, %d).", __func__, ch->name, iWear );
      return;
   }

   separate_obj( obj ); /* just in case */
   if( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_EVIL( ch ) )
       || ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) && IS_GOOD( ch ) )
       || ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL( ch ) ) )
   {
      /*
       * Thanks to Morgenes for the bug fix here!
       */
      if( loading_char != ch )
      {
         act( AT_MAGIC, "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
         act( AT_MAGIC, "$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM );
      }
      if( obj->carried_by )
         obj_from_char( obj );
      obj_to_room( obj, ch->in_room );
// oprog_zap_trigger( ch, obj);
      if( IS_SET( sysdata.save_flags, SV_ZAPDROP ) && !char_died( ch ) )
         save_char_obj( ch );
      return;
   }

   ch->armor -= apply_ac( obj, iWear );
   obj->wear_loc = iWear;

   ch->carry_number -= get_obj_number( obj );
   if( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
      ch->carry_weight -= get_obj_weight( obj );

   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      affect_modify( ch, paf, TRUE );
   for( paf = obj->first_affect; paf; paf = paf->next )
      affect_modify( ch, paf, TRUE );

   if( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room )
      ++ch->in_room->light;

   return;
}

/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA * ch, OBJ_DATA * obj )
{
   AFFECT_DATA *paf;

   if( obj->wear_loc == WEAR_NONE )
   {
      bug( "%s: already unequipped.", __func__ );
      return;
   }

   ch->carry_number += get_obj_number( obj );
   if( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
      ch->carry_weight += get_obj_weight( obj );

   ch->armor += apply_ac( obj, obj->wear_loc );
   obj->wear_loc = -1;

   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      affect_modify( ch, paf, FALSE );
   if( obj->carried_by )
      for( paf = obj->first_affect; paf; paf = paf->next )
         affect_modify( ch, paf, FALSE );

   if( !obj->carried_by )
      return;

   if( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room && ch->in_room->light > 0 )
      --ch->in_room->light;

   return;
}

/*
 * Move an obj out of a room.
 */
int falling;

void obj_from_room( OBJ_DATA * obj )
{
   ROOM_INDEX_DATA *in_room;

   if( ( in_room = obj->in_room ) == NULL )
   {
      bug( "%s: NULL.", __func__ );
      return;
   }

   UNLINK( obj, in_room->first_content, in_room->last_content, next_content, prev_content );

   if( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
      empty_obj( obj, NULL, obj->in_room );

   if( obj->item_type == ITEM_FIRE )
      obj->in_room->light -= obj->count;

   obj->carried_by = NULL;
   obj->in_obj = NULL;
   obj->in_room = NULL;
   if( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling == 0 )
      write_corpses( NULL, obj->short_descr + 14 );
   return;
}

/*
 * Move an obj into a room.
 */
OBJ_DATA *obj_to_room( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex )
{
   OBJ_DATA *otmp, *oret;
   short count = obj->count;
   short item_type = obj->item_type;

   for( otmp = pRoomIndex->first_content; otmp; otmp = otmp->next_content )
      if( ( oret = group_object( otmp, obj ) ) == otmp )
      {
         if( item_type == ITEM_FIRE )
            pRoomIndex->light += count;
         return oret;
      }

   LINK( obj, pRoomIndex->first_content, pRoomIndex->last_content, next_content, prev_content );
   obj->in_room = pRoomIndex;
   obj->carried_by = NULL;
   obj->in_obj = NULL;
   obj->room_vnum = pRoomIndex->vnum; /* hotboot tracker */
   if( item_type == ITEM_FIRE )
      pRoomIndex->light += count;
   falling++;
   obj_fall( obj, FALSE );
   falling--;
   if( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling == 0 )
      write_corpses( NULL, obj->short_descr + 14 );
   return obj;
}

/*
 * Move an object into an object.
 */
OBJ_DATA *obj_to_obj( OBJ_DATA * obj, OBJ_DATA * obj_to )
{
   OBJ_DATA *otmp, *oret;

   if( obj == obj_to )
   {
      bug( "%s: trying to put object inside itself: vnum %d", __func__, obj->pIndexData->vnum );
      return obj;
   }
   /*
    * Big carry_weight bug fix here by Thoric 
    */
   if( obj->carried_by != obj_to->carried_by )
   {
      if( obj->carried_by )
         obj->carried_by->carry_weight -= get_obj_weight( obj );
      if( obj_to->carried_by )
         obj_to->carried_by->carry_weight += get_obj_weight( obj );
   }

   for( otmp = obj_to->first_content; otmp; otmp = otmp->next_content )
      if( ( oret = group_object( otmp, obj ) ) == otmp )
         return oret;

   LINK( obj, obj_to->first_content, obj_to->last_content, next_content, prev_content );
   obj->in_obj = obj_to;
   obj->in_room = NULL;
   obj->carried_by = NULL;

   return obj;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA * obj )
{
   OBJ_DATA *obj_from;

   if( ( obj_from = obj->in_obj ) == NULL )
   {
      bug( "%s: null obj_from.", __func__ );
      return;
   }

   UNLINK( obj, obj_from->first_content, obj_from->last_content, next_content, prev_content );

   if( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
      empty_obj( obj, obj->in_obj, NULL );

   obj->in_obj = NULL;
   obj->in_room = NULL;
   obj->carried_by = NULL;

   for( ; obj_from; obj_from = obj_from->in_obj )
      if( obj_from->carried_by )
         obj_from->carried_by->carry_weight -= get_obj_weight( obj );

   return;
}

/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA * obj )
{
   OBJ_DATA *obj_content;
   REL_DATA *RQueue, *rq_next;

   if( !obj )
   {
      bug( "%s: !obj", __func__ );
      return;
   }

   if( obj_extracted( obj ) )
   {
      bug( "%s: obj %d already extracted!", __func__, obj->pIndexData->vnum );
      return;
   }

   if( obj->item_type == ITEM_PORTAL )
      remove_portal( obj );

   if( obj->carried_by )
      obj_from_char( obj );
   else if( obj->in_room )
      obj_from_room( obj );
   else if( obj->in_obj )
      obj_from_obj( obj );

   while( ( obj_content = obj->last_content ) != NULL )
      extract_obj( obj_content );

   {
      AFFECT_DATA *paf;
      AFFECT_DATA *paf_next;

      for( paf = obj->first_affect; paf; paf = paf_next )
      {
         paf_next = paf->next;
         DISPOSE( paf );
      }
      obj->first_affect = obj->last_affect = NULL;
   }

   {
      EXTRA_DESCR_DATA *ed;
      EXTRA_DESCR_DATA *ed_next;

      for( ed = obj->first_extradesc; ed; ed = ed_next )
      {
         ed_next = ed->next;
         STRFREE( ed->description );
         STRFREE( ed->keyword );
         DISPOSE( ed );
      }
      obj->first_extradesc = obj->last_extradesc = NULL;
   }

   if( obj == gobj_prev )
      gobj_prev = obj->prev;

   for( RQueue = first_relation; RQueue; RQueue = rq_next )
   {
      rq_next = RQueue->next;
      if( RQueue->Type == relOSET_ON )
      {
         if( obj == RQueue->Subject )
            ( ( CHAR_DATA * ) RQueue->Actor )->dest_buf = NULL;
         else
            continue;
         UNLINK( RQueue, first_relation, last_relation, next, prev );
         DISPOSE( RQueue );
      }
   }

   UNLINK( obj, first_object, last_object, next, prev );
   /*
    * shove onto extraction queue 
    */
   queue_extracted_obj( obj );

   obj->pIndexData->count -= obj->count;
   numobjsloaded -= obj->count;
   --physicalobjects;
   if( obj->serial == cur_obj )
   {
      cur_obj_extracted = TRUE;
      if( global_objcode == rNONE )
         global_objcode = rOBJ_EXTRACTED;
   }
   return;
}

/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA * ch, bool fPull )
{
   CHAR_DATA *wch;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   REL_DATA *RQueue, *rq_next;

   if( !ch )
   {
      bug( "%s: NULL ch.", __func__ );
      return;  /* who removed this line? */
   }

   if( !ch->in_room )
   {
      bug( "%s: NULL room.", __func__ );
      return;
   }

   if( ch == supermob )
   {
      bug( "%s: ch == supermob!", __func__ );
      return;
   }

   if( char_died( ch ) )
   {
      bug( "%s: %s already died!", __func__, ch->name );
      return;
   }

   if( ch == cur_char )
      cur_char_died = TRUE;
   /*
    * shove onto extraction queue 
    */
   queue_extracted_char( ch, fPull );

   for( RQueue = first_relation; RQueue; RQueue = rq_next )
   {
      rq_next = RQueue->next;
      if( fPull && RQueue->Type == relMSET_ON )
      {
         if( ch == RQueue->Subject )
            ( ( CHAR_DATA * ) RQueue->Actor )->dest_buf = NULL;
         else if( ch != RQueue->Actor )
            continue;
         UNLINK( RQueue, first_relation, last_relation, next, prev );
         DISPOSE( RQueue );
      }
   }

   if( gch_prev == ch )
      gch_prev = ch->prev;

   if( fPull && !IS_SET( ch->act, ACT_POLYMORPHED ) )
      die_follower( ch );

   stop_fighting( ch, TRUE );

   if( ch->mount )
   {
      update_room_reset( ch, TRUE );
      REMOVE_BIT( ch->mount->act, ACT_MOUNTED );
      ch->mount = NULL;
      ch->position = POS_STANDING;
   }

   if( IS_NPC( ch ) )
      update_room_reset( ch, TRUE );

   if( IS_SET( ch->in_room->room_flags2, ROOM_ARENA ) )
   {
      ch->hit = ch->max_hit;
      ch->mana = ch->max_mana;
      ch->move = ch->max_move;
   }

   if( IS_NPC( ch ) && IS_SET( ch->act, ACT_MOUNTED ) )
   {
      for( wch = first_char; wch; wch = wch->next )
      {
         if( wch->mount == ch )
         {
            wch->mount = NULL;
            wch->position = POS_STANDING;
         }
      }
      REMOVE_BIT( ch->act, ACT_MOUNTED );
   }

   REMOVE_BIT( ch->affected_by, AFF_SANCTUARY );

   while( ( obj = ch->last_carrying ) != NULL )
      extract_obj( obj );

   char_from_room( ch );

   if( !fPull )
   {
      location = NULL;

      if( !location )
         location = get_room_index( wherehome( ch ) );

      if( !location )
         location = get_room_index( 1 );

      char_to_room( ch, location );

      act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
      ch->position = POS_RESTING;
      return;
   }

   if( IS_NPC( ch ) )
   {
      --ch->pIndexData->count;
      --nummobsloaded;
   }

   if( ch->desc && ch->desc->original && IS_SET( ch->act, ACT_POLYMORPHED ) )
      do_revert( ch, "" );

   if( ch->desc && ch->desc->original )
      do_return( ch, "" );

   if( ch->switched && ch->switched->desc )
      do_return( ch->switched, "" );

   for( wch = first_char; wch; wch = wch->next )
      if( wch->reply == ch )
         wch->reply = NULL;

   UNLINK( ch, first_char, last_char, next, prev );

   if( ch->desc )
   {
      if( ch->desc->character != ch )
         bug( "%s: char's descriptor points to another char", __func__ );
      else
      {
         ch->desc->character = NULL;
         close_socket( ch->desc, FALSE );
         ch->desc = NULL;
      }
   }
   return;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char fix[MAX_STRING_LENGTH];
   CHAR_DATA *rch;
   int number, count, vnum;

   number = number_argument( argument, arg );
   if( !str_cmp( arg, "self" ) )
      return ch;

   if( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;

   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( ch == rch )
         continue;
      snprintf( fix, MAX_STRING_LENGTH, "__%s", rch->name );
      if( ( can_see( ch, rch ) && ( nifty_is_name( arg, PERS( rch, ch ) ) || !str_cmp( arg, fix ) ) )
          || ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) || ( IS_NPC( ch ) && ( nifty_is_name( arg, rch->name ) ) )
          || ( IS_NPC( rch ) && nifty_is_name( arg, rch->name ) ) )
      {
         if( number == 0 && !IS_NPC( rch ) )
            return rch;
         else if( ++count == number )
            return rch;
      }
   }

   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of characters
    * again looking for prefix matching, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( !can_see( ch, rch ) || !nifty_is_name_prefix( arg, PERS( rch, ch ) ) || ch == rch
          || ( IS_NPC( ch ) && ( !nifty_is_name_prefix( arg, rch->name ) ) ) )
         continue;
      if( number == 0 && !IS_NPC( rch ) )
         return rch;
      else if( ++count == number )
         return rch;
   }

   return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *wch;
   int number, count, vnum;

   number = number_argument( argument, arg );
   count = 0;
   if( !str_cmp( arg, "self" ) )
      return ch;

   /*
    * Allow reference by vnum for saints+         -Thoric
    */
   if( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   /*
    * check the room for an exact match 
    */
   for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
      if( ( nifty_is_name( arg, PERS( wch, ch ) ) ||
            ( IS_NPC( ch ) && ( nifty_is_name( arg, wch->name ) ) )
            || ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) && is_wizvis( ch, wch ) )
      {
         if( number == 0 && !IS_NPC( wch ) )
            return wch;
         else if( ++count == number )
            return wch;
      }

   count = 0;

   /*
    * check the world for an exact match 
    */
   for( wch = first_char; wch; wch = wch->next )
      if( ( nifty_is_name( arg, PERS( wch, ch ) ) || ( IS_NPC( ch ) && ( nifty_is_name( arg, wch->name ) ) )
            || ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) && is_wizvis( ch, wch ) )
      {
         if( number == 0 && !IS_NPC( wch ) )
            return wch;
         else if( ++count == number )
            return wch;
      }

   /*
    * bail out if looking for a vnum match 
    */
   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, check the room for
    * for a prefix match, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
   {
      if( !nifty_is_name_prefix( arg, PERS( wch, ch ) ) || ( IS_NPC( ch ) && ( !nifty_is_name_prefix( arg, wch->name ) ) ) )
         continue;
      if( number == 0 && !IS_NPC( wch ) && is_wizvis( ch, wch ) )
         return wch;
      else if( ++count == number && is_wizvis( ch, wch ) )
         return wch;
   }

   /*
    * If we didn't find a prefix match in the room, run through the full list
    * of characters looking for prefix matching, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( wch = first_char; wch; wch = wch->next )
   {
      if( !nifty_is_name_prefix( arg, PERS( wch, ch ) ) || ( IS_NPC( ch ) && ( !nifty_is_name( arg, wch->name ) ) ) )
         continue;
      if( number == 0 && !IS_NPC( wch ) && is_wizvis( ch, wch ) )
         return wch;
      else if( ++count == number && is_wizvis( ch, wch ) )
         return wch;
   }

   return NULL;
}

/* Find a character by name, no PERS */
CHAR_DATA *get_char_world_ooc( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *wch;
   int number, count, vnum;

   number = number_argument( argument, arg );
   count = 0;
   if( !str_cmp( arg, "self" ) )
      return ch;

   /*
    * Allow reference by vnum for saints+         -Thoric
    */
   if( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   /*
    * check the room for an exact match 
    */
   for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
      if( ( nifty_is_name( arg, wch->name ) ||
            ( IS_NPC( ch ) && ( nifty_is_name( arg, wch->name ) ) )
            || ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) && is_wizvis( ch, wch ) )
      {
         if( number == 0 && !IS_NPC( wch ) )
            return wch;
         else if( ++count == number )
            return wch;
      }

   count = 0;

   /*
    * check the world for an exact match 
    */
   for( wch = first_char; wch; wch = wch->next )
      if( ( nifty_is_name( arg, wch->name ) || ( IS_NPC( ch ) && ( nifty_is_name( arg, wch->name ) ) )
            || ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) && is_wizvis( ch, wch ) )
      {
         if( number == 0 && !IS_NPC( wch ) )
            return wch;
         else if( ++count == number )
            return wch;
      }

   /*
    * bail out if looking for a vnum match 
    */
   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, check the room for
    * for a prefix match, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
   {
      if( !nifty_is_name_prefix( arg, wch->name ) || ( IS_NPC( ch ) && ( !nifty_is_name_prefix( arg, wch->name ) ) ) )
         continue;
      if( number == 0 && !IS_NPC( wch ) && is_wizvis( ch, wch ) )
         return wch;
      else if( ++count == number && is_wizvis( ch, wch ) )
         return wch;
   }

   /*
    * If we didn't find a prefix match in the room, run through the full list
    * of characters looking for prefix matching, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( wch = first_char; wch; wch = wch->next )
   {
      if( !nifty_is_name_prefix( arg, wch->name ) || ( IS_NPC( ch ) && ( !nifty_is_name( arg, wch->name ) ) ) )
         continue;
      if( number == 0 && !IS_NPC( wch ) && is_wizvis( ch, wch ) )
         return wch;
      else if( ++count == number && is_wizvis( ch, wch ) )
         return wch;
   }

   return NULL;
}

/*
 * Find a char by comfreq
 */

CHAR_DATA *get_char_from_comfreq( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *wch;

   if( !str_cmp( argument, ch->comfreq ) || !str_cmp( argument, "self" ) )
      return ch;

   /*
    * check mud for matching comfreq, or immortal name 
    */
   for( wch = first_char; wch; wch = wch->next )
   {
      if( IS_IMMORTAL( ch ) )
      {
         if( nifty_is_name_prefix( argument, wch->name ) )
            return wch;
      }
      else
      {
         if( !str_cmp( argument, wch->comfreq ) || ( IS_IMMORTAL( wch ) && nifty_is_name_prefix( argument, wch->name ) ) )
         {
            if( !IS_NPC( wch ) || ( IS_IMMORTAL( wch ) && can_see( ch, wch ) ) )
               return wch;
         }
      }
   }

   return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA * ch, const char *argument, OBJ_DATA * list )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number;
   int count;

   number = number_argument( argument, arg );
   count = 0;
   for( obj = list; obj; obj = obj->next_content )
      if( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = list; obj; obj = obj->next_content )
      if( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}

/*
 * Find an obj in a list...going the other way			-Thoric
 */
OBJ_DATA *get_obj_list_rev( CHAR_DATA * ch, const char *argument, OBJ_DATA * list )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number;
   int count;

   number = number_argument( argument, arg );
   count = 0;
   for( obj = list; obj; obj = obj->prev_content )
      if( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = list; obj; obj = obj->prev_content )
      if( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number, count, vnum;

   number = number_argument( argument, arg );
   if( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc == WEAR_NONE
          && can_see_obj( ch, obj ) && ( nifty_is_name( arg, obj->name ) || obj->pIndexData->vnum == vnum ) )
         if( ( count += obj->count ) >= number )
            return obj;

   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number, count, vnum;

   if( !ch )
   {
      bug( "%s: null ch", __func__ );
   }

   number = number_argument( argument, arg );

   if( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc != WEAR_NONE
          && can_see_obj( ch, obj ) && ( nifty_is_name( arg, obj->name ) || obj->pIndexData->vnum == vnum ) )
         if( ++count == number )
            return obj;

   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc != WEAR_NONE && can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ++count == number )
            return obj;

   return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;

   if( !ch || !ch->in_room )
      return NULL;

   obj = get_obj_list_rev( ch, argument, ch->in_room->last_content );
   if( obj )
      return obj;

   if( ( obj = get_obj_carry( ch, argument ) ) != NULL )
      return obj;

   if( ( obj = get_obj_wear( ch, argument ) ) != NULL )
      return obj;

   return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number, count, vnum;

   if( !ch )
      return NULL;

   if( ( obj = get_obj_here( ch, argument ) ) != NULL )
      return obj;

   number = number_argument( argument, arg );

   /*
    * Allow reference by vnum for saints+         -Thoric
    */
   if( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;
   for( obj = first_object; obj; obj = obj->next )
      if( can_see_obj( ch, obj ) && ( nifty_is_name( arg, obj->name ) || vnum == obj->pIndexData->vnum ) )
         if( ( count += obj->count ) >= number )
            return obj;

   /*
    * bail out if looking for a vnum 
    */
   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = first_object; obj; obj = obj->next )
      if( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}


/*
 * How mental state could affect finding an object		-Thoric
 * Used by get/drop/put/quaff/recite/etc
 * Increasingly freaky based on mental state and drunkeness
 */
bool ms_find_obj( CHAR_DATA * ch )
{
   int ms = ch->mental_state;
   int drunk = IS_NPC( ch ) ? 0 : ch->pcdata->condition[COND_DRUNK];
   const char *t;

   /*
    * we're going to be nice and let nothing weird happen unless
    * you're a tad messed up
    */
   drunk = UMAX( 1, drunk );
   if( abs( ms ) + ( drunk / 3 ) < 30 )
      return FALSE;
   if( ( number_percent(  ) + ( ms < 0 ? 15 : 5 ) ) > abs( ms ) / 2 + drunk / 4 )
      return FALSE;
   if( ms > 15 )  /* range 1 to 20 */
      switch ( number_range( UMAX( 1, ( ms / 5 - 15 ) ), ( ms + 4 ) / 5 ) )
      {
         default:
         case 1:
            t = "As you reach for it, you forgot what it was...\r\n";
            break;
         case 2:
            t = "As you reach for it, something inside stops you...\r\n";
            break;
         case 3:
            t = "As you reach for it, it seems to move out of the way...\r\n";
            break;
         case 4:
            t = "You grab frantically for it, but can't seem to get a hold of it...\r\n";
            break;
         case 5:
            t = "It disappears as soon as you touch it!\r\n";
            break;
         case 6:
            t = "You would if it would stay still!\r\n";
            break;
         case 7:
            t = "Whoa!  It's covered in blood!  Ack!  Ick!\r\n";
            break;
         case 8:
            t = "Wow... trails!\r\n";
            break;
         case 9:
            t = "You reach for it, then notice the back of your hand is growing something!\r\n";
            break;
         case 10:
            t = "As you grasp it, it shatters into tiny shards which bite into your flesh!\r\n";
            break;
         case 11:
            t = "What about that huge dragon flying over your head?!?!?\r\n";
            break;
         case 12:
            t = "You stratch yourself instead...\r\n";
            break;
         case 13:
            t = "You hold the universe in the palm of your hand!\r\n";
            break;
         case 14:
            t = "You're too scared.\r\n";
            break;
         case 15:
            t = "Your mother smacks your hand... 'NO!'\r\n";
            break;
         case 16:
            t = "Your hand grasps the worse pile of revoltingness than you could ever imagine!\r\n";
            break;
         case 17:
            t = "You stop reaching for it as it screams out at you in pain!\r\n";
            break;
         case 18:
            t = "What about the millions of burrow-maggots feasting on your arm?!?!\r\n";
            break;
         case 19:
            t = "That doesn't matter anymore... you've found the true answer to everything!\r\n";
            break;
         case 20:
            t = "A supreme entity has no need for that.\r\n";
            break;
      }
   else
   {
      int sub = URANGE( 1, abs( ms ) / 2 + drunk, 60 );
      switch ( number_range( 1, sub / 10 ) )
      {
         default:
         case 1:
            t = "In just a second...\r\n";
            break;
         case 2:
            t = "You can't find that...\r\n";
            break;
         case 3:
            t = "It's just beyond your grasp...\r\n";
            break;
         case 4:
            t = "...but it's under a pile of other stuff...\r\n";
            break;
         case 5:
            t = "You go to reach for it, but pick your nose instead.\r\n";
            break;
         case 6:
            t = "Which one?!?  I see two... no three...\r\n";
            break;
      }
   }
   send_to_char( t, ch );
   return TRUE;
}


/*
 * Generic get obj function that supports optional containers.	-Thoric
 * currently only used for "eat" and "quaff".
 */
OBJ_DATA *find_obj( CHAR_DATA * ch, const char *argument, bool carryonly )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
      argument = one_argument( argument, arg2 );

   if( arg2[0] == '\0' )
   {
      if( carryonly && ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
      {
         send_to_char( "You do not have that item.\r\n", ch );
         return NULL;
      }
      else if( !carryonly && ( obj = get_obj_here( ch, arg1 ) ) == NULL )
      {
         act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
         return NULL;
      }
      return obj;
   }
   else
   {
      OBJ_DATA *container;

      if( carryonly
          && ( container = get_obj_carry( ch, arg2 ) ) == NULL && ( container = get_obj_wear( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not have that item.\r\n", ch );
         return NULL;
      }
      if( !carryonly && ( container = get_obj_here( ch, arg2 ) ) == NULL )
      {
         act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
         return NULL;
      }

      if( !IS_OBJ_STAT( container, ITEM_COVERING ) && IS_SET( container->value[1], CONT_CLOSED ) )
      {
         act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
         return NULL;
      }

      obj = get_obj_list( ch, arg1, container->first_content );
      if( !obj )
         act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
              "I see nothing like that beneath $p." : "I see nothing like that in $p.", ch, container, NULL, TO_CHAR );
      return obj;
   }
   return NULL;
}

int get_obj_number( OBJ_DATA * obj )
{
   return obj->count;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA * obj )
{
   int weight;

   weight = obj->count * obj->weight;
   for( obj = obj->first_content; obj; obj = obj->next_content )
      weight += get_obj_weight( obj );

   return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA * pRoomIndex )
{
   if( !pRoomIndex )
   {
      bug( "%s: NULL pRoomIndex", __func__ );
      return TRUE;
   }

   if( pRoomIndex->light > 0 )
      return FALSE;

   if( IS_SET( pRoomIndex->room_flags, ROOM_DARK ) )
      return TRUE;

   if( pRoomIndex->sector_type == SECT_INSIDE || pRoomIndex->sector_type == SECT_CITY )
      return FALSE;

   if( weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK )
      return TRUE;

   return FALSE;
}

/*
 * True if room is private.
 */
bool room_is_private( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex )
{
   CHAR_DATA *rch;
   int count;

   if( !ch )
   {
      bug( "%s: NULL ch", __func__ );
      return FALSE;
   }

   if( !pRoomIndex )
   {
      bug( "%s: NULL pRoomIndex", __func__ );
      return FALSE;
   }

   if( IS_SET( pRoomIndex->room_flags, ROOM_PLR_HOME ) && ch->plr_home != pRoomIndex )
      return TRUE;

   count = 0;

   for( rch = pRoomIndex->first_person; rch; rch = rch->next_in_room )
      count++;

   if( IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) && count >= 2 )
      return TRUE;

   if( IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && count >= 1 )
      return TRUE;

   return FALSE;
}

/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA * ch, CHAR_DATA * victim )
{
   if( !victim )
      return FALSE;

   if( victim->position == POS_FIGHTING || victim->position < POS_SLEEPING )
      return TRUE;

   if( !ch )
   {
      if( IS_AFFECTED( victim, AFF_INVISIBLE ) || IS_AFFECTED( victim, AFF_HIDE ) || IS_SET( victim->act, PLR_WIZINVIS ) )
         return FALSE;
      else
         return TRUE;
   }

   if( ch == victim )
      return TRUE;

   if( !IS_NPC( victim ) && IS_SET( victim->act, PLR_WIZINVIS ) && get_trust( ch ) < victim->pcdata->wizinvis )
      return FALSE;

   if( victim->position == POS_FIGHTING || victim->position < POS_SLEEPING )
      return TRUE;

   /*
    * SB 
    */
   if( IS_NPC( victim ) && IS_SET( victim->act, ACT_MOBINVIS ) && get_trust( ch ) < victim->mobinvis )
      return FALSE;

   if( !IS_IMMORTAL( ch ) && !IS_NPC( victim ) && !victim->desc
       && get_timer( victim, TIMER_RECENTFIGHT ) > 0
       && ( !victim->switched || !IS_AFFECTED( victim->switched, AFF_POSSESS ) ) )
      return FALSE;

   if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;

   /*
    * The miracle cure for blindness? -- Altrag 
    */
   if( !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
   {
      if( IS_AFFECTED( ch, AFF_BLIND ) )
         return FALSE;

      if( room_is_dark( ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
         return FALSE;

      if( IS_AFFECTED( victim, AFF_HIDE )
          && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) && !victim->fighting && !IS_AFFECTED( ch, AFF_INFRARED ) )
         return FALSE;

      if( ch->race == RACE_DEFEL && victim->race == RACE_DEFEL )
         return TRUE;

      if( IS_AFFECTED( victim, AFF_INVISIBLE ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
         return FALSE;

   }

   return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;

   if( IS_OBJ_STAT( obj, ITEM_BURRIED ) )
      return FALSE;

   if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return TRUE;

   if( IS_AFFECTED( ch, AFF_BLIND ) )
      return FALSE;

   if( IS_OBJ_STAT( obj, ITEM_HIDDEN ) )
      return FALSE;

   if( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
      return TRUE;

   if( room_is_dark( ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
      return FALSE;

   if( IS_OBJ_STAT( obj, ITEM_INVIS ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
      return FALSE;

   return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( !IS_OBJ_STAT( obj, ITEM_NODROP ) )
      return TRUE;

   if( !IS_NPC( ch ) && get_trust( ch ) >= LEVEL_IMMORTAL )
      return TRUE;

   if( IS_NPC( ch ) && ch->pIndexData->vnum == MOB_VNUM_SUPERMOB )
      return TRUE;

   return FALSE;
}

/*
 * Return ascii name of an item type.
 */
const char *item_type_name( OBJ_DATA * obj )
{
   if( obj->item_type < 1 || obj->item_type > MAX_ITEM_TYPE )
   {
      bug( "%s: unknown type %d.", __func__, obj->item_type );
      return "(unknown)";
   }

   return o_types[obj->item_type];
}

/*
 * Return ascii name of an affect location.
 */
const char *affect_loc_name( int location )
{
   switch ( location )
   {
      case APPLY_NONE:
         return "none";
      case APPLY_STR:
         return "strength";
      case APPLY_DEX:
         return "dexterity";
      case APPLY_INT:
         return "intelligence";
      case APPLY_WIS:
         return "wisdom";
      case APPLY_CON:
         return "constitution";
      case APPLY_CHA:
         return "charisma";
      case APPLY_LCK:
         return "luck";
      case APPLY_SEX:
         return "sex";
      case APPLY_LEVEL:
         return "level";
      case APPLY_AGE:
         return "age";
      case APPLY_MANA:
         return "mana";
      case APPLY_HIT:
         return "hp";
      case APPLY_MOVE:
         return "moves";
      case APPLY_GOLD:
         return "gold";
      case APPLY_EXP:
         return "experience";
      case APPLY_AC:
         return "armor class";
      case APPLY_HITROLL:
         return "hit roll";
      case APPLY_DAMROLL:
         return "damage roll";
      case APPLY_SAVING_POISON:
         return "save vs poison";
      case APPLY_SAVING_ROD:
         return "save vs rod";
      case APPLY_SAVING_PARA:
         return "save vs paralysis";
      case APPLY_SAVING_BREATH:
         return "save vs breath";
      case APPLY_SAVING_SPELL:
         return "save vs spell";
      case APPLY_HEIGHT:
         return "height";
      case APPLY_WEIGHT:
         return "weight";
      case APPLY_AFFECT:
         return "affected_by";
      case APPLY_RESISTANT:
         return "resistant";
      case APPLY_IMMUNE:
         return "immune";
      case APPLY_SUSCEPTIBLE:
         return "susceptible";
      case APPLY_BACKSTAB:
         return "backstab";
      case APPLY_PICK:
         return "pick";
      case APPLY_TRACK:
         return "track";
      case APPLY_STEAL:
         return "steal";
      case APPLY_SNEAK:
         return "sneak";
      case APPLY_HIDE:
         return "hide";
      case APPLY_PALM:
         return "palm";
      case APPLY_DETRAP:
         return "detrap";
      case APPLY_DODGE:
         return "dodge";
      case APPLY_PEEK:
         return "peek";
      case APPLY_SCAN:
         return "scan";
      case APPLY_GOUGE:
         return "gouge";
      case APPLY_SEARCH:
         return "search";
      case APPLY_MOUNT:
         return "mount";
      case APPLY_DISARM:
         return "disarm";
      case APPLY_KICK:
         return "kick";
      case APPLY_PARRY:
         return "parry";
      case APPLY_BASH:
         return "bash";
      case APPLY_STUN:
         return "stun";
      case APPLY_PUNCH:
         return "punch";
      case APPLY_CLIMB:
         return "climb";
      case APPLY_GRIP:
         return "grip";
      case APPLY_SCRIBE:
         return "scribe";
      case APPLY_COVER_TRAIL:
         return "cover trail";
      case APPLY_WEAPONSPELL:
         return "weapon spell";
      case APPLY_WEARSPELL:
         return "wear spell";
      case APPLY_REMOVESPELL:
         return "remove spell";
      case APPLY_MENTALSTATE:
         return "mental state";
      case APPLY_EMOTION:
         return "emotional state";
      case APPLY_STRIPSN:
         return "dispel";
      case APPLY_REMOVE:
         return "remove";
      case APPLY_DIG:
         return "dig";
      case APPLY_FULL:
         return "hunger";
      case APPLY_THIRST:
         return "thirst";
      case APPLY_DRUNK:
         return "drunk";
      case APPLY_BLOOD:
         return "blood";
   }

   bug( "%s: unknown location %d.", __func__, location );
   return "(unknown)";
}

/*
 * Return ascii name of an affect bit vector.
 */
const char *affect_bit_name( int vector )
{
   static char buf[512];

   buf[0] = '\0';
   if( vector & AFF_BLIND )
      strlcat( buf, " blind", 512 );
   if( vector & AFF_INVISIBLE )
      strlcat( buf, " invisible", 512 );
   if( vector & AFF_DETECT_EVIL )
      strlcat( buf, " detect_evil", 512 );
   if( vector & AFF_DETECT_INVIS )
      strlcat( buf, " detect_invis", 512 );
   if( vector & AFF_DETECT_MAGIC )
      strlcat( buf, " detect_magic", 512 );
   if( vector & AFF_DETECT_HIDDEN )
      strlcat( buf, " detect_hidden", 512 );
   if( vector & AFF_WEAKEN )
      strlcat( buf, " weaken", 512 );
   if( vector & AFF_SANCTUARY )
      strlcat( buf, " sanctuary", 512 );
   if( vector & AFF_FAERIE_FIRE )
      strlcat( buf, " faerie_fire", 512 );
   if( vector & AFF_INFRARED )
      strlcat( buf, " infrared", 512 );
   if( vector & AFF_CURSE )
      strlcat( buf, " curse", 512 );
   if( vector & AFF_COVER_TRAIL )
      strlcat( buf, " cover trail", 512 );
   if( vector & AFF_POISON )
      strlcat( buf, " poison", 512 );
   if( vector & AFF_PROTECT )
      strlcat( buf, " protect", 512 );
   if( vector & AFF_PARALYSIS )
      strlcat( buf, " paralysis", 512 );
   if( vector & AFF_SLEEP )
      strlcat( buf, " sleep", 512 );
   if( vector & AFF_SNEAK )
      strlcat( buf, " sneak", 512 );
   if( vector & AFF_HIDE )
      strlcat( buf, " hide", 512 );
   if( vector & AFF_CHARM )
      strlcat( buf, " charm", 512 );
   if( vector & AFF_POSSESS )
      strlcat( buf, " possess", 512 );
   if( vector & AFF_FLYING )
      strlcat( buf, " flying", 512 );
   if( vector & AFF_PASS_DOOR )
      strlcat( buf, " pass_door", 512 );
   if( vector & AFF_FLOATING )
      strlcat( buf, " floating", 512 );
   if( vector & AFF_TRUESIGHT )
      strlcat( buf, " true_sight", 512 );
   if( vector & AFF_DETECTTRAPS )
      strlcat( buf, " detect_traps", 512 );
   if( vector & AFF_SCRYING )
      strlcat( buf, " scrying", 512 );
   if( vector & AFF_FIRESHIELD )
      strlcat( buf, " fireshield", 512 );
   if( vector & AFF_SHOCKSHIELD )
      strlcat( buf, " shockshield", 512 );
   if( vector & AFF_ICESHIELD )
      strlcat( buf, " iceshield", 512 );
   if( vector & AFF_POSSESS )
      strlcat( buf, " possess", 512 );
   if( vector & AFF_BERSERK )
      strlcat( buf, " berserk", 512 );
   if( vector & AFF_AQUA_BREATH )
      strlcat( buf, " aqua_breath", 512 );
   return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/*
 * Return ascii name of extra flags vector.
 */
const char *extra_bit_name( int extra_flags )
{
   static char buf[512];

   buf[0] = '\0';
   if( extra_flags & ITEM_GLOW )
      strlcat( buf, " glow", 512 );
   if( extra_flags & ITEM_HUM )
      strlcat( buf, " hum", 512 );
   if( extra_flags & ITEM_DARK )
      strlcat( buf, " dark", 512 );
   if( extra_flags & ITEM_HUTT_SIZE )
      strlcat( buf, " hutt_size", 512 );
   if( extra_flags & ITEM_CONTRABAND )
      strlcat( buf, " contraband", 512 );
   if( extra_flags & ITEM_INVIS )
      strlcat( buf, " invis", 512 );
   if( extra_flags & ITEM_MAGIC )
      strlcat( buf, " magic", 512 );
   if( extra_flags & ITEM_NODROP )
      strlcat( buf, " nodrop", 512 );
   if( extra_flags & ITEM_BLESS )
      strlcat( buf, " bless", 512 );
   if( extra_flags & ITEM_ANTI_GOOD )
      strlcat( buf, " anti-good", 512 );
   if( extra_flags & ITEM_ANTI_EVIL )
      strlcat( buf, " anti-evil", 512 );
   if( extra_flags & ITEM_ANTI_NEUTRAL )
      strlcat( buf, " anti-neutral", 512 );
   if( extra_flags & ITEM_NOREMOVE )
      strlcat( buf, " noremove", 512 );
   if( extra_flags & ITEM_INVENTORY )
      strlcat( buf, " inventory", 512 );
   if( extra_flags & ITEM_DEATHROT )
      strlcat( buf, " deathrot", 512 );
   if( extra_flags & ITEM_ANTI_SOLDIER )
      strlcat( buf, " anti-soldier", 512 );
   if( extra_flags & ITEM_ANTI_THIEF )
      strlcat( buf, " anti-thief", 512 );
   if( extra_flags & ITEM_ANTI_HUNTER )
      strlcat( buf, " anti-hunter", 512 );
   if( extra_flags & ITEM_ANTI_JEDI )
      strlcat( buf, " anti-jedi", 512 );
   if( extra_flags & ITEM_ANTI_SITH )
      strlcat( buf, " anti-sith", 512 );
   if( extra_flags & ITEM_ANTI_PILOT )
      strlcat( buf, " anti-pilot", 512 );
   if( extra_flags & ITEM_SMALL_SIZE )
      strlcat( buf, " small_size", 512 );
   if( extra_flags & ITEM_LARGE_SIZE )
      strlcat( buf, " large_size", 512 );
   if( extra_flags & ITEM_DONATION )
      strlcat( buf, " donation", 512 );
   if( extra_flags & ITEM_CLANOBJECT )
      strlcat( buf, " clan", 512 );
   if( extra_flags & ITEM_ANTI_CITIZEN )
      strlcat( buf, " anti-citizen", 512 );
   if( extra_flags & ITEM_PROTOTYPE )
      strlcat( buf, " prototype", 512 );
   if( extra_flags & ITEM_HUMAN_SIZE )
      strlcat( buf, " human_size", 512 );
   return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/*
 * Return ascii name of magic flags vector. - Scryn
 */
const char *magic_bit_name( int magic_flags )
{
   static char buf[512];

   buf[0] = '\0';
   if( magic_flags & ITEM_RETURNING )
      strlcat( buf, " returning", 512 );
   return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/*
 * Set off a trap (obj) upon character (ch)			-Thoric
 */
ch_ret spring_trap( CHAR_DATA * ch, OBJ_DATA * obj )
{
   int dam;
   int typ;
   int lev;
   const char *txt;
   char buf[MAX_STRING_LENGTH];
   ch_ret retcode;

   typ = obj->value[1];
   lev = obj->value[2];

   retcode = rNONE;

   switch ( typ )
   {
      default:
         txt = "hit by a trap";
         break;
      case TRAP_TYPE_POISON_GAS:
         txt = "surrounded by a green cloud of gas";
         break;
      case TRAP_TYPE_POISON_DART:
         txt = "hit by a dart";
         break;
      case TRAP_TYPE_POISON_NEEDLE:
         txt = "pricked by a needle";
         break;
      case TRAP_TYPE_POISON_DAGGER:
         txt = "stabbed by a dagger";
         break;
      case TRAP_TYPE_POISON_ARROW:
         txt = "struck with an arrow";
         break;
      case TRAP_TYPE_BLINDNESS_GAS:
         txt = "surrounded by a red cloud of gas";
         break;
      case TRAP_TYPE_SLEEPING_GAS:
         txt = "surrounded by a yellow cloud of gas";
         break;
      case TRAP_TYPE_FLAME:
         txt = "struck by a burst of flame";
         break;
      case TRAP_TYPE_EXPLOSION:
         txt = "hit by an explosion";
         break;
      case TRAP_TYPE_ACID_SPRAY:
         txt = "covered by a spray of acid";
         break;
      case TRAP_TYPE_ELECTRIC_SHOCK:
         txt = "suddenly shocked";
         break;
      case TRAP_TYPE_BLADE:
         txt = "sliced by a razor sharp blade";
         break;
      case TRAP_TYPE_SEX_CHANGE:
         txt = "surrounded by a mysterious aura";
         break;
   }

   dam = number_range( obj->value[2], obj->value[2] * 2 );
   snprintf( buf, MAX_STRING_LENGTH, "You are %s!", txt );
   act( AT_HITME, buf, ch, NULL, NULL, TO_CHAR );
   snprintf( buf, MAX_STRING_LENGTH, "$n is %s.", txt );
   act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
   --obj->value[0];
   if( obj->value[0] <= 0 )
      extract_obj( obj );
   switch ( typ )
   {
      default:
      case TRAP_TYPE_POISON_DART:
      case TRAP_TYPE_POISON_NEEDLE:
      case TRAP_TYPE_POISON_DAGGER:
      case TRAP_TYPE_POISON_ARROW:
         /*
          * hmm... why not use spell_poison() here? 
          */
         retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
         if( retcode == rNONE )
            retcode = damage( ch, ch, dam, TYPE_UNDEFINED );
         break;
      case TRAP_TYPE_POISON_GAS:
         retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_BLINDNESS_GAS:
         retcode = obj_cast_spell( gsn_blindness, lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_SLEEPING_GAS:
         retcode = obj_cast_spell( skill_lookup( "sleep" ), lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_ACID_SPRAY:
         retcode = obj_cast_spell( skill_lookup( "acid blast" ), lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_SEX_CHANGE:
         retcode = obj_cast_spell( skill_lookup( "change sex" ), lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_FLAME:
      case TRAP_TYPE_EXPLOSION:
         retcode = obj_cast_spell( gsn_fireball, lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_ELECTRIC_SHOCK:
      case TRAP_TYPE_BLADE:
         retcode = damage( ch, ch, dam, TYPE_UNDEFINED );
   }
   return retcode;
}

/*
 * Check an object for a trap					-Thoric
 */
ch_ret check_for_trap( CHAR_DATA * ch, OBJ_DATA * obj, int flag )
{
   OBJ_DATA *check;
   ch_ret retcode;

   if( !obj->first_content )
      return rNONE;

   retcode = rNONE;

   for( check = obj->first_content; check; check = check->next_content )
      if( check->item_type == ITEM_TRAP && IS_SET( check->value[3], flag ) )
      {
         retcode = spring_trap( ch, check );
         if( retcode != rNONE )
            return retcode;
      }
   return retcode;
}

/*
 * Check the room for a trap					-Thoric
 */
ch_ret check_room_for_traps( CHAR_DATA * ch, int flag )
{
   OBJ_DATA *check;
   ch_ret retcode;

   retcode = rNONE;

   if( !ch )
      return rERROR;
   if( !ch->in_room || !ch->in_room->first_content )
      return rNONE;

   for( check = ch->in_room->first_content; check; check = check->next_content )
   {
      if( check->item_type == ITEM_LANDMINE && flag == TRAP_ENTER_ROOM )
      {
         explode( check );
         return rNONE;
      }
      else if( check->item_type == ITEM_TRAP && IS_SET( check->value[3], flag ) )
      {
         retcode = spring_trap( ch, check );
         if( retcode != rNONE )
            return retcode;
      }
   }
   return retcode;
}

/*
 * return TRUE if an object contains a trap			-Thoric
 */
bool is_trapped( OBJ_DATA * obj )
{
   OBJ_DATA *check;

   if( !obj->first_content )
      return FALSE;

   for( check = obj->first_content; check; check = check->next_content )
      if( check->item_type == ITEM_TRAP )
         return TRUE;

   return FALSE;
}

/*
 * If an object contains a trap, return the pointer to the trap	-Thoric
 */
OBJ_DATA *get_trap( OBJ_DATA * obj )
{
   OBJ_DATA *check;

   if( !obj->first_content )
      return NULL;

   for( check = obj->first_content; check; check = check->next_content )
      if( check->item_type == ITEM_TRAP )
         return check;

   return NULL;
}

/*
 * Remove an exit from a room					-Thoric
 */
void extract_exit( ROOM_INDEX_DATA * room, EXIT_DATA * pexit )
{
   UNLINK( pexit, room->first_exit, room->last_exit, next, prev );
   if( pexit->rexit )
      pexit->rexit->rexit = NULL;
   STRFREE( pexit->keyword );
   STRFREE( pexit->description );
   DISPOSE( pexit );
}

/*
 * Remove a room
 */
void extract_room( ROOM_INDEX_DATA * room )
{
   bug( "%s: not implemented", __func__ );
   /*
    * (remove room from hash table)
    * clean_room( room )
    * DISPOSE( room );
    */
   return;
}

/*
 * clean out a room (leave list pointers intact )		-Thoric
 */
void clean_room( ROOM_INDEX_DATA * room )
{
   EXTRA_DESCR_DATA *ed, *ed_next;
   EXIT_DATA *pexit, *pexit_next;

   STRFREE( room->description );
   STRFREE( room->name );
   for( ed = room->first_extradesc; ed; ed = ed_next )
   {
      ed_next = ed->next;
      STRFREE( ed->description );
      STRFREE( ed->keyword );
      DISPOSE( ed );
      top_ed--;
   }
   room->first_extradesc = NULL;
   room->last_extradesc = NULL;
   for( pexit = room->first_exit; pexit; pexit = pexit_next )
   {
      pexit_next = pexit->next;
      STRFREE( pexit->keyword );
      STRFREE( pexit->description );
      DISPOSE( pexit );
      top_exit--;
   }
   room->first_exit = NULL;
   room->last_exit = NULL;
   room->room_flags = 0;
   room->sector_type = 0;
   room->light = 0;
}

/*
 * clean out an object (index) (leave list pointers intact )	-Thoric
 */
void clean_obj( OBJ_INDEX_DATA * obj )
{
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;
   EXTRA_DESCR_DATA *ed;
   EXTRA_DESCR_DATA *ed_next;

   STRFREE( obj->name );
   STRFREE( obj->short_descr );
   STRFREE( obj->description );
   STRFREE( obj->action_desc );
   obj->item_type = 0;
   obj->extra_flags = 0;
   obj->wear_flags = 0;
   obj->count = 0;
   obj->weight = 0;
   obj->cost = 0;
   obj->value[0] = 0;
   obj->value[1] = 0;
   obj->value[2] = 0;
   obj->value[3] = 0;
   obj->value[4] = 0;
   obj->value[5] = 0;
   for( paf = obj->first_affect; paf; paf = paf_next )
   {
      paf_next = paf->next;
      DISPOSE( paf );
      top_affect--;
   }
   obj->first_affect = NULL;
   obj->last_affect = NULL;
   for( ed = obj->first_extradesc; ed; ed = ed_next )
   {
      ed_next = ed->next;
      STRFREE( ed->description );
      STRFREE( ed->keyword );
      DISPOSE( ed );
      top_ed--;
   }
   obj->first_extradesc = NULL;
   obj->last_extradesc = NULL;
}

/*
 * clean out a mobile (index) (leave list pointers intact )	-Thoric
 */
void clean_mob( MOB_INDEX_DATA * mob )
{
   MPROG_DATA *mprog, *mprog_next;

   STRFREE( mob->player_name );
   STRFREE( mob->short_descr );
   STRFREE( mob->long_descr );
   STRFREE( mob->description );
   mob->spec_fun = NULL;
   mob->spec_2 = NULL;
   mob->pShop = NULL;
   mob->rShop = NULL;
   mob->progtypes = 0;

   for( mprog = mob->mudprogs; mprog; mprog = mprog_next )
   {
      mprog_next = mprog->next;
      STRFREE( mprog->arglist );
      STRFREE( mprog->comlist );
      DISPOSE( mprog );
   }
   mob->count = 0;
   mob->killed = 0;
   mob->sex = 0;
   mob->level = 0;
   mob->act = 0;
   mob->affected_by = 0;
   mob->alignment = 0;
   mob->mobthac0 = 0;
   mob->ac = 0;
   mob->hitnodice = 0;
   mob->hitsizedice = 0;
   mob->hitplus = 0;
   mob->damnodice = 0;
   mob->damsizedice = 0;
   mob->damplus = 0;
   mob->gold = 0;
   mob->position = 0;
   mob->defposition = 0;
   mob->height = 0;
   mob->weight = 0;
}

extern int top_reset;

/*
 * Remove all resets from a room -Thoric
 */
void clean_resets( ROOM_INDEX_DATA *room )
{
    RESET_DATA *pReset, *pReset_next;

    for( pReset = room->first_reset; pReset; pReset = pReset_next )
    {
	pReset_next = pReset->next;
	delete_reset( pReset );
	--top_reset;
    }
    room->first_reset	= NULL;
    room->last_reset	= NULL;
}

/* no more silliness */
void name_stamp_stats( CHAR_DATA * ch )
{
}

/*
 * "Fix" a character's stats					-Thoric
 */
void fix_char( CHAR_DATA * ch )
{
   AFFECT_DATA *aff;
   OBJ_DATA *obj;

   de_equip_char( ch );

   for( aff = ch->first_affect; aff; aff = aff->next )
      affect_modify( ch, aff, FALSE );

   ch->affected_by = race_table[ch->race].affected;
   ch->mental_state = -10;
   ch->hit = UMAX( 1, ch->hit );
   ch->mana = UMAX( 1, ch->mana );
   ch->move = UMAX( 1, ch->move );
   ch->armor = 100;
   ch->mod_str = 0;
   ch->mod_dex = 0;
   ch->mod_wis = 0;
   ch->mod_int = 0;
   ch->mod_con = 0;
   ch->mod_cha = 0;
   ch->mod_lck = 0;
   ch->damroll = 0;
   ch->hitroll = 0;
   ch->alignment = URANGE( -1000, ch->alignment, 1000 );
   ch->saving_breath = 0;
   ch->saving_wand = 0;
   ch->saving_para_petri = 0;
   ch->saving_spell_staff = 0;
   ch->saving_poison_death = 0;

   for( aff = ch->first_affect; aff; aff = aff->next )
      affect_modify( ch, aff, TRUE );

   ch->carry_weight = 0;
   ch->carry_number = 0;

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      if( obj->wear_loc == WEAR_NONE )
         ch->carry_number += get_obj_number( obj );
      if( !IS_SET( obj->extra_flags, ITEM_MAGIC ) )
         ch->carry_weight += get_obj_weight( obj );
   }

   re_equip_char( ch );
}

/*
 * Show an affect verbosely to a character			-Thoric
 */
void showaffect( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   char buf[MAX_STRING_LENGTH];
   int x;

   if( !paf )
   {
      bug( "%s: NULL paf", __func__ );
      return;
   }
   if( paf->location != APPLY_NONE && paf->modifier != 0 )
   {
      switch ( paf->location )
      {
         default:
            snprintf( buf, MAX_STRING_LENGTH, "Affects %s by %d.\r\n", affect_loc_name( paf->location ), paf->modifier );
            break;
         case APPLY_AFFECT:
            snprintf( buf, MAX_STRING_LENGTH, "Affects %s by", affect_loc_name( paf->location ) );
            for( x = 0; x < 32; x++ )
               if( IS_SET( paf->modifier, 1 << x ) )
               {
                  strlcat( buf, " ", MAX_STRING_LENGTH );
                  strlcat( buf, a_flags[x], MAX_STRING_LENGTH );
               }
            strlcat( buf, "\r\n", MAX_STRING_LENGTH );
            break;
         case APPLY_WEAPONSPELL:
         case APPLY_WEARSPELL:
         case APPLY_REMOVESPELL:
            snprintf( buf, MAX_STRING_LENGTH, "Casts spell '%s'\r\n",
                     IS_VALID_SN( paf->modifier ) ? skill_table[paf->modifier]->name : "unknown" );
            break;
         case APPLY_RESISTANT:
         case APPLY_IMMUNE:
         case APPLY_SUSCEPTIBLE:
            snprintf( buf, MAX_STRING_LENGTH, "Affects %s by", affect_loc_name( paf->location ) );
            for( x = 0; x < 32; x++ )
               if( IS_SET( paf->modifier, 1 << x ) )
               {
                  strlcat( buf, " ", MAX_STRING_LENGTH );
                  strlcat( buf, ris_flags[x], MAX_STRING_LENGTH );
               }
            strlcat( buf, "\r\n", MAX_STRING_LENGTH );
            break;
      }
      send_to_char( buf, ch );
   }
}

/*
 * Set the current global object to obj				-Thoric
 */
void set_cur_obj( OBJ_DATA * obj )
{
   cur_obj = obj->serial;
   cur_obj_extracted = FALSE;
   global_objcode = rNONE;
}

/*
 * Check the recently extracted object queue for obj		-Thoric
 */
bool obj_extracted( OBJ_DATA * obj )
{
   OBJ_DATA *cod;

   if( !obj )
      return TRUE;

   if( obj->serial == cur_obj && cur_obj_extracted )
      return TRUE;

   for( cod = extracted_obj_queue; cod; cod = cod->next )
      if( obj == cod )
         return TRUE;
   return FALSE;
}

/*
 * Stick obj onto extraction queue
 */
void queue_extracted_obj( OBJ_DATA * obj )
{

   ++cur_qobjs;
   obj->next = extracted_obj_queue;
   extracted_obj_queue = obj;
}

/* Deallocates the memory used by a single object after it's been extracted. */
void free_obj( OBJ_DATA * obj )
{
   AFFECT_DATA *paf, *paf_next;
   EXTRA_DESCR_DATA *ed, *ed_next;
   REL_DATA *RQueue, *rq_next;
   MPROG_ACT_LIST *mpact, *mpact_next;

   for( mpact = obj->mpact; mpact; mpact = mpact_next )
   {
      mpact_next = mpact->next;
      DISPOSE( mpact->buf );
      DISPOSE( mpact );
   }

   /*
    * remove affects 
    */
   for( paf = obj->first_affect; paf; paf = paf_next )
   {
      paf_next = paf->next;
      DISPOSE( paf );
   }
   obj->first_affect = obj->last_affect = NULL;

   /*
    * remove extra descriptions 
    */
   for( ed = obj->first_extradesc; ed; ed = ed_next )
   {
      ed_next = ed->next;
      STRFREE( ed->description );
      STRFREE( ed->keyword );
      DISPOSE( ed );
   }
   obj->first_extradesc = obj->last_extradesc = NULL;

   for( RQueue = first_relation; RQueue; RQueue = rq_next )
   {
      rq_next = RQueue->next;
      if( RQueue->Type == relOSET_ON )
      {
         if( obj == RQueue->Subject )
            ( ( CHAR_DATA * ) RQueue->Actor )->dest_buf = NULL;
         else
            continue;
         UNLINK( RQueue, first_relation, last_relation, next, prev );
         DISPOSE( RQueue );
      }
   }
   STRFREE( obj->name );
   STRFREE( obj->description );
   STRFREE( obj->short_descr );
   STRFREE( obj->action_desc );
   STRFREE( obj->armed_by );
   DISPOSE( obj->killer );
   DISPOSE( obj );
   return;
}

/*
 * Clean out the extracted object queue
 */
void clean_obj_queue( void )
{
   OBJ_DATA *obj;

   while( extracted_obj_queue )
   {
      obj = extracted_obj_queue;
      extracted_obj_queue = extracted_obj_queue->next;
      free_obj( obj );
      --cur_qobjs;
   }
}

/*
 * Set the current global character to ch			-Thoric
 */
void set_cur_char( CHAR_DATA * ch )
{
   cur_char = ch;
   cur_char_died = FALSE;
   cur_room = ch->in_room;
   global_retcode = rNONE;
}

/*
 * Check to see if ch died recently				-Thoric
 */
bool char_died( CHAR_DATA * ch )
{
   EXTRACT_CHAR_DATA *ccd;

   if( ch == cur_char && cur_char_died )
      return TRUE;

   for( ccd = extracted_char_queue; ccd; ccd = ccd->next )
      if( ccd->ch == ch )
         return TRUE;
   return FALSE;
}

/*
 * Add ch to the queue of recently extracted characters		-Thoric
 */
void queue_extracted_char( CHAR_DATA * ch, bool extract )
{
   EXTRACT_CHAR_DATA *ccd;

   if( !ch )
   {
      bug( "%s: ch = NULL", __func__ );
      return;
   }
   CREATE( ccd, EXTRACT_CHAR_DATA, 1 );
   ccd->ch = ch;
   ccd->room = ch->in_room;
   ccd->extract = extract;
   if( ch == cur_char )
      ccd->retcode = global_retcode;
   else
      ccd->retcode = rCHAR_DIED;
   ccd->next = extracted_char_queue;
   extracted_char_queue = ccd;
   cur_qchars++;
}

/*
 * clean out the extracted character queue
 */
void clean_char_queue(  )
{
   EXTRACT_CHAR_DATA *ccd;

   for( ccd = extracted_char_queue; ccd; ccd = extracted_char_queue )
   {
      extracted_char_queue = ccd->next;
      if( ccd->extract )
         free_char( ccd->ch );
      DISPOSE( ccd );
      --cur_qchars;
   }
}

/*
 * Add a timer to ch						-Thoric
 * Support for "call back" time delayed commands
 */
void add_timer( CHAR_DATA * ch, short type, short count, DO_FUN * fun, int value )
{
   TIMER *timer;

   for( timer = ch->first_timer; timer; timer = timer->next )
      if( timer->type == type )
      {
         timer->count = count;
         timer->do_fun = fun;
         timer->value = value;
         break;
      }
   if( !timer )
   {
      CREATE( timer, TIMER, 1 );
      timer->count = count;
      timer->type = type;
      timer->do_fun = fun;
      timer->value = value;
      LINK( timer, ch->first_timer, ch->last_timer, next, prev );
   }
}

TIMER *get_timerptr( CHAR_DATA * ch, short type )
{
   TIMER *timer;

   for( timer = ch->first_timer; timer; timer = timer->next )
      if( timer->type == type )
         return timer;
   return NULL;
}

short get_timer( CHAR_DATA * ch, short type )
{
   TIMER *timer;

   if( ( timer = get_timerptr( ch, type ) ) != NULL )
      return timer->count;
   else
      return 0;
}

void extract_timer( CHAR_DATA * ch, TIMER * timer )
{
   if( !timer )
   {
      bug( "%s: NULL timer", __func__ );
      return;
   }

   UNLINK( timer, ch->first_timer, ch->last_timer, next, prev );
   DISPOSE( timer );
   return;
}

void remove_timer( CHAR_DATA * ch, short type )
{
   TIMER *timer;

   for( timer = ch->first_timer; timer; timer = timer->next )
      if( timer->type == type )
         break;

   if( timer )
      extract_timer( ch, timer );
}

bool in_soft_range( CHAR_DATA * ch, AREA_DATA * tarea )
{
   if( IS_IMMORTAL( ch ) )
      return TRUE;
   else if( IS_NPC( ch ) )
      return TRUE;
   else if( ( tarea ) && ( ch->top_level >= tarea->low_soft_range || ch->top_level <= tarea->hi_soft_range ) )
      return TRUE;
   else
      return FALSE;
}

bool in_hard_range( CHAR_DATA * ch, AREA_DATA * tarea )
{
   if( IS_IMMORTAL( ch ) )
      return TRUE;
   else if( IS_NPC( ch ) )
      return TRUE;
   else if( ( tarea ) && ( ch->top_level >= tarea->low_hard_range && ch->top_level <= tarea->hi_hard_range ) )
      return TRUE;
   else
      return FALSE;
}

/*
 * Scryn, standard luck check 2/2/96
 */
bool chance( CHAR_DATA * ch, short percent )
{
/*  short clan_factor, ms;*/
   short deity_factor, ms;

   if( !ch )
   {
      bug( "%s: null ch!", __func__ );
      return FALSE;
   }

/* Code for clan stuff put in by Narn, Feb/96.  The idea is to punish clan
members who don't keep their alignment in tune with that of their clan by
making it harder for them to succeed at pretty much everything.  Clan_factor
will vary from 1 to 3, with 1 meaning there is no effect on the player's
change of success, and with 3 meaning they have half the chance of doing
whatever they're trying to do. 

Note that since the neutral clannies can only be off by 1000 points, their
maximum penalty will only be half that of the other clan types.

  if ( IS_CLANNED( ch ) )
    clan_factor = 1 + abs( ch->alignment - ch->pcdata->clan->alignment ) / 1000; 
  else
    clan_factor = 1;
*/
/* Mental state bonus/penalty:  Your mental state is a ranged value with
 * zero (0) being at a perfect mental state (bonus of 10).
 * negative values would reflect how sedated one is, and
 * positive values would reflect how stimulated one is.
 * In most circumstances you'd do best at a perfectly balanced state.
 */

   deity_factor = 0;

   ms = 10 - abs( ch->mental_state );

   if( ( number_percent(  ) - get_curr_lck( ch ) + 13 - ms ) + deity_factor <= percent )
      return TRUE;
   else
      return FALSE;
}

/*
 * Make a simple clone of an object (no extras...yet)		-Thoric
 */
OBJ_DATA *clone_object( OBJ_DATA * obj )
{
   OBJ_DATA *clone;

   CREATE( clone, OBJ_DATA, 1 );
   clone->pIndexData = obj->pIndexData;
   clone->name = QUICKLINK( obj->name );
   clone->short_descr = QUICKLINK( obj->short_descr );
   clone->description = QUICKLINK( obj->description );
   clone->action_desc = QUICKLINK( obj->action_desc );
   clone->item_type = obj->item_type;
   clone->extra_flags = obj->extra_flags;
   clone->magic_flags = obj->magic_flags;
   clone->wear_flags = obj->wear_flags;
   clone->wear_loc = obj->wear_loc;
   clone->weight = obj->weight;
   clone->cost = obj->cost;
   clone->level = obj->level;
   clone->timer = obj->timer;
   clone->value[0] = obj->value[0];
   clone->value[1] = obj->value[1];
   clone->value[2] = obj->value[2];
   clone->value[3] = obj->value[3];
   clone->value[4] = obj->value[4];
   clone->value[5] = obj->value[5];
   clone->count = 1;
   ++obj->pIndexData->count;
   ++numobjsloaded;
   ++physicalobjects;
   cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
   clone->serial = clone->pIndexData->serial = cur_obj_serial;
   LINK( clone, first_object, last_object, next, prev );
   return clone;
}

/*
 * If possible group obj2 into obj1				-Thoric
 * This code, along with clone_object, obj->count, and special support
 * for it implemented throughout handler.c and save.c should show improved
 * performance on MUDs with players that hoard tons of potions and scrolls
 * as this will allow them to be grouped together both in memory, and in
 * the player files.
 */
OBJ_DATA *group_object( OBJ_DATA * obj1, OBJ_DATA * obj2 )
{
   if( !obj1 || !obj2 )
      return NULL;
   if( obj1 == obj2 )
      return obj1;

   if( obj1->pIndexData == obj2->pIndexData
/*
    &&	!obj1->pIndexData->mudprogs
    &&  !obj2->pIndexData->mudprogs
*/
       && !str_cmp( obj1->name, obj2->name )
       && !str_cmp( obj1->short_descr, obj2->short_descr )
       && !str_cmp( obj1->description, obj2->description )
       && !str_cmp( obj1->action_desc, obj2->action_desc )
       && obj1->item_type == obj2->item_type
       && obj1->extra_flags == obj2->extra_flags
       && obj1->magic_flags == obj2->magic_flags
       && obj1->wear_flags == obj2->wear_flags
       && obj1->wear_loc == obj2->wear_loc
       && obj1->weight == obj2->weight
       && obj1->cost == obj2->cost
       && obj1->level == obj2->level
       && obj1->timer == obj2->timer
       && obj1->value[0] == obj2->value[0]
       && obj1->value[1] == obj2->value[1]
       && obj1->value[2] == obj2->value[2]
       && obj1->value[3] == obj2->value[3]
       && obj1->value[4] == obj2->value[4]
       && obj1->value[5] == obj2->value[5]
       && !obj1->first_extradesc && !obj2->first_extradesc
       && !obj1->first_affect && !obj2->first_affect && !obj1->first_content && !obj2->first_content )
   {
      obj1->count += obj2->count;
      obj1->pIndexData->count += obj2->count;   /* to be decremented in */
      numobjsloaded += obj2->count; /* extract_obj */
      extract_obj( obj2 );
      return obj1;
   }
   return obj2;
}

/*
 * Split off a grouped object					-Thoric
 * decreased obj's count to num, and creates a new object containing the rest
 */
void split_obj( OBJ_DATA * obj, int num )
{
   int count;
   OBJ_DATA *rest;

   if( !obj )
      return;

   count = obj->count;

   if( count <= num || num == 0 )
      return;

   rest = clone_object( obj );
   --obj->pIndexData->count;  /* since clone_object() ups this value */
   --numobjsloaded;
   rest->count = obj->count - num;
   obj->count = num;

   if( obj->carried_by )
   {
      LINK( rest, obj->carried_by->first_carrying, obj->carried_by->last_carrying, next_content, prev_content );
      rest->carried_by = obj->carried_by;
      rest->in_room = NULL;
      rest->in_obj = NULL;
   }
   else if( obj->in_room )
   {
      LINK( rest, obj->in_room->first_content, obj->in_room->last_content, next_content, prev_content );
      rest->carried_by = NULL;
      rest->in_room = obj->in_room;
      rest->in_obj = NULL;
   }
   else if( obj->in_obj )
   {
      LINK( rest, obj->in_obj->first_content, obj->in_obj->last_content, next_content, prev_content );
      rest->in_obj = obj->in_obj;
      rest->in_room = NULL;
      rest->carried_by = NULL;
   }
}

void separate_obj( OBJ_DATA * obj )
{
   split_obj( obj, 1 );
}

/*
 * Empty an obj's contents... optionally into another obj, or a room
 */
bool empty_obj( OBJ_DATA * obj, OBJ_DATA * destobj, ROOM_INDEX_DATA * destroom )
{
   OBJ_DATA *otmp, *otmp_next;
   CHAR_DATA *ch = obj->carried_by;
   bool movedsome = FALSE;

   if( !obj )
   {
      bug( "%s: NULL obj", __func__ );
      return FALSE;
   }
   if( destobj || ( !destroom && !ch && ( destobj = obj->in_obj ) != NULL ) )
   {
      for( otmp = obj->first_content; otmp; otmp = otmp_next )
      {
         otmp_next = otmp->next_content;
         if( destobj->item_type == ITEM_CONTAINER && get_obj_weight( otmp ) + get_obj_weight( destobj ) > destobj->value[0] )
            continue;
         obj_from_obj( otmp );
         obj_to_obj( otmp, destobj );
         movedsome = TRUE;
      }
      return movedsome;
   }
   if( destroom || ( !ch && ( destroom = obj->in_room ) != NULL ) )
   {
      for( otmp = obj->first_content; otmp; otmp = otmp_next )
      {
         otmp_next = otmp->next_content;
         if( ch && ( otmp->pIndexData->progtypes & DROP_PROG ) && otmp->count > 1 )
         {
            separate_obj( otmp );
            obj_from_obj( otmp );
            if( !otmp_next )
               otmp_next = obj->first_content;
         }
         else
            obj_from_obj( otmp );
         otmp = obj_to_room( otmp, destroom );
         if( ch )
         {
            oprog_drop_trigger( ch, otmp );  /* mudprogs */
            if( char_died( ch ) )
               ch = NULL;
         }
         movedsome = TRUE;
      }
      return movedsome;
   }
   if( ch )
   {
      for( otmp = obj->first_content; otmp; otmp = otmp_next )
      {
         otmp_next = otmp->next_content;
         obj_from_obj( otmp );
         obj_to_char( otmp, ch );
         movedsome = TRUE;
      }
      return movedsome;
   }
   bug( "%s: could not determine a destination for vnum %d", __func__, obj->pIndexData->vnum );
   return FALSE;
}

/*
 * Improve mental state						-Thoric
 */
void better_mental_state( CHAR_DATA * ch, int mod )
{
   int c = URANGE( 0, abs( mod ), 20 );
   int con = get_curr_con( ch );

   c += number_percent(  ) < con ? 1 : 0;

   if( ch->mental_state < 0 )
      ch->mental_state = URANGE( -100, ch->mental_state + c, 0 );
   else if( ch->mental_state > 0 )
      ch->mental_state = URANGE( 0, ch->mental_state - c, 100 );
}

/*
 * Deteriorate mental state					-Thoric
 */
void worsen_mental_state( CHAR_DATA * ch, int mod )
{
   int c = URANGE( 0, abs( mod ), 20 );
   int con = get_curr_con( ch );


   c -= number_percent(  ) < con ? 1 : 0;
   if( c < 1 )
      return;

   if( ch->mental_state < 0 )
      ch->mental_state = URANGE( -100, ch->mental_state - c, 100 );
   else if( ch->mental_state > 0 )
      ch->mental_state = URANGE( -100, ch->mental_state + c, 100 );
   else
      ch->mental_state -= c;
}


/*
 * Add gold to an area's economy				-Thoric
 */
void boost_economy( AREA_DATA * tarea, int gold )
{
   if( !tarea )
      return;
   while( gold >= 1000000000 )
   {

      ++tarea->high_economy;
      gold -= 1000000000;
   }
   tarea->low_economy += gold;
   while( tarea->low_economy >= 1000000000 )
   {
      ++tarea->high_economy;
      tarea->low_economy -= 1000000000;
   }
}

/*
 * Take gold from an area's economy				-Thoric
 */
void lower_economy( AREA_DATA * tarea, int gold )
{
   while( gold >= 1000000000 )
   {
      tarea->high_economy -= 1;
      gold -= 1000000000;
   }
   tarea->low_economy -= gold;
   while( tarea->low_economy < 0 )
   {
      tarea->high_economy -= 1;
      tarea->low_economy += 1000000000;
   }
}

/*
 * Check to see if economy has at least this much gold		   -Thoric
 */
bool economy_has( AREA_DATA * tarea, int gold )
{
   int hasgold = ( ( tarea->high_economy > 0 ) ? 1 : 0 ) * 1000000000 + tarea->low_economy;

   if( hasgold >= gold )
      return TRUE;
   return FALSE;
}

/*
 * Used in db.c when resetting a mob into an area		    -Thoric
 * Makes sure mob doesn't get more than 10% of that area's gold,
 * and reduces area economy by the amount of gold given to the mob
 */
void economize_mobgold( CHAR_DATA * mob )
{
   int gold;
   AREA_DATA *tarea;

   /*
    * make sure it isn't way too much 
    */
   mob->gold = UMIN( mob->gold, mob->top_level * mob->top_level * 400 );
   if( !mob->in_room )
      return;
   tarea = mob->in_room->area;

   gold = ( ( tarea->high_economy > 0 ) ? 1 : 0 ) * 1000000000 + tarea->low_economy;
   mob->gold = URANGE( 0, mob->gold, gold / 10 );
   if( mob->gold )
      lower_economy( tarea, mob->gold );
}


/*
 * Add another notch on that there belt... ;)
 * Keep track of the last so many kills by vnum			-Thoric
 */
void add_kill( CHAR_DATA * ch, CHAR_DATA * mob )
{
   int vnum, x;
   short track;

   if( IS_NPC( ch ) )
      return;

   if( !IS_NPC( mob ) )
      return;

   vnum = mob->pIndexData->vnum;
   track = URANGE( 2, ( ( ch->skill_level[COMBAT_ABILITY] + 3 ) * MAX_KILLTRACK ) / LEVEL_AVATAR, MAX_KILLTRACK );
   for( x = 0; x < track; x++ )
      if( ch->pcdata->killed[x].vnum == vnum )
      {
         if( ch->pcdata->killed[x].count < 50 )
            ++ch->pcdata->killed[x].count;
         return;
      }
      else if( ch->pcdata->killed[x].vnum == 0 )
         break;
   memmove( ( char * )ch->pcdata->killed + sizeof( KILLED_DATA ),
            ch->pcdata->killed, ( track - 1 ) * sizeof( KILLED_DATA ) );
   ch->pcdata->killed[0].vnum = vnum;
   ch->pcdata->killed[0].count = 1;
   if( track < MAX_KILLTRACK )
      ch->pcdata->killed[track].vnum = 0;
}

/*
 * Return how many times this player has killed this mob	-Thoric
 * Only keeps track of so many (MAX_KILLTRACK), and keeps track by vnum
 */
int times_killed( CHAR_DATA * ch, CHAR_DATA * mob )
{
   int vnum, x;
   short track;

   if( IS_NPC( ch ) )
      return 0;

   if( !IS_NPC( mob ) )
      return 0;

   vnum = mob->pIndexData->vnum;
   track = URANGE( 2, ( ( ch->skill_level[COMBAT_ABILITY] + 3 ) * MAX_KILLTRACK ) / LEVEL_AVATAR, MAX_KILLTRACK );
   for( x = 0; x < track; x++ )
      if( ch->pcdata->killed[x].vnum == vnum )
         return ch->pcdata->killed[x].count;
      else if( ch->pcdata->killed[x].vnum == 0 )
         break;
   return 0;
}

void check_switches( bool possess )
{
   CHAR_DATA *ch;

   for( ch = first_char; ch; ch = ch->next )
      check_switch( ch, possess );
}

void check_switch( CHAR_DATA *ch, bool possess )
{
   AFFECT_DATA *paf;
   CMDTYPE *cmd;
   int hash, trust = get_trust(ch);

   if( !ch->switched )
      return;

   if( !possess )
   {
      for( paf = ch->switched->first_affect; paf; paf = paf->next )
      {
         if( paf->duration == -1 )
            continue;
         if( paf->type != -1 && skill_table[paf->type]->spell_fun == spell_possess )
            return;
      }
   }

   for( hash = 0; hash < 126; hash++ )
   {
      for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
      {
         if( cmd->do_fun != do_switch )
            continue;
         if( cmd->level <= trust )
            return;

         if( !IS_NPC(ch) && ch->pcdata->bestowments && is_name( cmd->name, ch->pcdata->bestowments )
          && cmd->level <= trust )
            return;
      }
   }

   if( !possess )
   {
      set_char_color( AT_BLUE, ch->switched );
      send_to_char( "You suddenly forfeit the power to switch!\n\r", ch->switched );
   }
   do_return( ch->switched, "" );
}
