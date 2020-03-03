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

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void add_reinforcements args( ( CHAR_DATA * ch ) );
ch_ret one_hit args( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) );
int xp_compute( CHAR_DATA * ch, CHAR_DATA * victim );
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA * in_room, EXIT_DATA ** pexit );
int ris_save( CHAR_DATA * ch, int schance, int ris );
void explode_emissile args( ( CHAR_DATA * ch, ROOM_INDEX_DATA * proom, int mindam, int maxdam, bool incendiary ) );
CHAR_DATA *get_char_room_mp( CHAR_DATA * ch, char *argument );

/* from shops.c */
CHAR_DATA *find_keeper args( ( CHAR_DATA * ch ) );

extern int top_affect;

const char *sector_name[SECT_MAX] = {
   "inside", "city", "field", "forest", "hills", "mountain", "water swim", "water noswim",
   "underwater", "air", "desert", "unknown", "ocean floor", "underground",
   "scrub", "rocky", "savanna", "tundra", "glacial", "rainforest", "jungle",
   "swamp", "wetlands", "brush", "steppe", "farmland", "volcanic"
};

void do_makeblade( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance, charge;
   bool checktool, checkdura, checkbatt, checkoven;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;
   AFFECT_DATA *paf;
   AFFECT_DATA *paf2;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makeblade <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbatt = FALSE;
         checkoven = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DURASTEEL )
               checkdura = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;

            if( obj->item_type == ITEM_OVEN )
               checkoven = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a vibro-blade.\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "&RYou need something to make it out of.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a power source for your blade.\n\r", ch );
            return;
         }

         if( !checkoven )
         {
            send_to_char( "&RYou need a small furnace to heat the metal.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeblade] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of crafting a vibroblade.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, do_makeblade, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makeblade );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeblade] );
   vnum = 66;

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

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_OVEN )
         checkoven = TRUE;
      if( obj->item_type == ITEM_DURASTEEL && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         charge = UMAX( 5, obj->value[0] );
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeblade] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven ) )
   {
      send_to_char( "&RYou activate your newly created vibroblade.\n\r", ch );
      send_to_char( "&RIt hums softly for a few seconds then begins to shake violently.\n\r", ch );
      send_to_char( "&RIt finally shatters breaking apart into a dozen pieces.\n\r", ch );
      learn_from_failure( ch, gsn_makeblade );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_WEAPON;
   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 3;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " vibro-blade blade " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was left here." );
   obj->description = STRALLOC( buf );
   CREATE( paf, AFFECT_DATA, 1 );
   paf->type = -1;
   paf->duration = -1;
   paf->location = get_atype( "backstab" );
   paf->modifier = level / 3;
   paf->bitvector = 0;
   paf->next = NULL;
   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
   CREATE( paf2, AFFECT_DATA, 1 );
   paf2->type = -1;
   paf2->duration = -1;
   paf2->location = get_atype( "hitroll" );
   paf2->modifier = -2;
   paf2->bitvector = 0;
   paf2->next = NULL;
   LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
   obj->value[0] = INIT_WEAPON_CONDITION;
   obj->value[1] = ( int )( level / 10 + 15 );  /* min dmg  */
   obj->value[2] = ( int )( level / 5 + 20 );   /* max dmg */
   obj->value[3] = WEAPON_VIBRO_BLADE;
   obj->value[4] = charge;
   obj->value[5] = charge;
   obj->cost = obj->value[2] * 10;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created blade.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes crafting a vibro-blade.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 200,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }

   learn_from_success( ch, gsn_makeblade );
}

void do_makeblaster( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checktool, checkdura, checkbatt, checkoven, checkcond, checkcirc, checkammo;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum, power, scope, ammo;
   AFFECT_DATA *paf;
   AFFECT_DATA *paf2;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makeblaster <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbatt = FALSE;
         checkoven = FALSE;
         checkcond = FALSE;
         checkcirc = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DURAPLAST )
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
            send_to_char( "&RYou need toolkit to make a blaster.\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "&RYou need something to make it out of.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a power source for your blaster.\n\r", ch );
            return;
         }

         if( !checkoven )
         {
            send_to_char( "&RYou need a small furnace to heat the plastics.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit board to control the firing mechanism.\n\r", ch );
            return;
         }

         if( !checkcond )
         {
            send_to_char( "&RYou still need a small superconductor.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeblaster] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a blaster.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, do_makeblaster, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makeblaster );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeblaster] );
   vnum = 50;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checkammo = FALSE;
   checktool = FALSE;
   checkdura = FALSE;
   checkbatt = FALSE;
   checkoven = FALSE;
   checkcond = FALSE;
   checkcirc = FALSE;
   power = 0;
   scope = 0;
   ammo = 0;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_OVEN )
         checkoven = TRUE;
      if( obj->item_type == ITEM_DURAPLAST && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_AMMO && checkammo == FALSE )
      {
         ammo = obj->value[0];
         checkammo = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }
      if( obj->item_type == ITEM_LENS && scope == 0 )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         scope++;
      }
      if( obj->item_type == ITEM_SUPERCONDUCTOR && power < 2 )
      {
         power++;
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
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeblaster] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )
       || ( !checkcond ) || ( !checkcirc ) )
   {
      send_to_char( "&RYou hold up your new blaster and aim at a leftover piece of plastic.\n\r", ch );
      send_to_char( "&RYou slowly squeeze the trigger hoping for the best...\n\r", ch );
      send_to_char( "&RYour blaster backfires destroying your weapon and burning your hand.\n\r", ch );
      learn_from_failure( ch, gsn_makeblaster );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_WEAPON;
   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 2 + level / 10;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " blaster " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   CREATE( paf, AFFECT_DATA, 1 );
   paf->type = -1;
   paf->duration = -1;
   paf->location = get_atype( "hitroll" );
   paf->modifier = URANGE( 0, 1 + scope, level / 30 );
   paf->bitvector = 0;
   paf->next = NULL;
   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
   CREATE( paf2, AFFECT_DATA, 1 );
   paf2->type = -1;
   paf2->duration = -1;
   paf2->location = get_atype( "damroll" );
   paf2->modifier = URANGE( 0, power, level / 30 );
   paf2->bitvector = 0;
   paf2->next = NULL;
   LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
   obj->value[0] = INIT_WEAPON_CONDITION; /* condition  */
   obj->value[1] = ( int )( level / 10 + 15 );  /* min dmg  */
   obj->value[2] = ( int )( level / 5 + 25 );   /* max dmg  */
   obj->value[3] = WEAPON_BLASTER;
   obj->value[4] = ammo;
   obj->value[5] = 2000;
   obj->cost = obj->value[2] * 50;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created blaster.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new blaster.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makeblaster );
}

void do_makelightsaber( CHAR_DATA * ch, char *argument )
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

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelightsaber] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of crafting a lightsaber.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, do_makelightsaber, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makelightsaber );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelightsaber] );
   vnum = 72;

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

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelightsaber] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )
       || ( !checkmirr ) || ( !checklens ) || ( !checkgems ) || ( !checkcond ) || ( !checkcirc ) )

   {
      send_to_char( "&RYou hold up your new lightsaber and press the switch hoping for the best.\n\r", ch );
      send_to_char( "&RInstead of a blade of light, smoke starts pouring from the handle.\n\r", ch );
      send_to_char( "&RYou drop the hot handle and watch as it melts on away on the floor.\n\r", ch );
      learn_from_failure( ch, gsn_makelightsaber );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_WEAPON;
   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   SET_BIT( obj->extra_flags, ITEM_ANTI_SOLDIER );
   SET_BIT( obj->extra_flags, ITEM_ANTI_THIEF );
   SET_BIT( obj->extra_flags, ITEM_ANTI_HUNTER );
   SET_BIT( obj->extra_flags, ITEM_ANTI_PILOT );
   SET_BIT( obj->extra_flags, ITEM_ANTI_CITIZEN );
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
   paf->modifier = URANGE( 0, gems, level / 10 );
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
   obj->value[1] = ( int )( level / 10 + gemtype * 2 );  /* min dmg  */
   obj->value[2] = ( int )( level / 5 + gemtype * 6 );   /* max dmg */
   obj->value[3] = WEAPON_LIGHTSABER;
   obj->value[4] = charge;
   obj->value[5] = charge;
   obj->cost = obj->value[2] * 75;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created lightsaber.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new lightsaber.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain = UMIN( obj->cost * 50, ( ch->pcdata->learned[gsn_makelightsaber] * 50 ) );
// Changed. -Tawnos
//         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[FORCE_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );
      gain_exp( ch, xpgain, FORCE_ABILITY );
      ch_printf( ch, "You gain %d force experience.", xpgain );
   }
   learn_from_success( ch, gsn_makelightsaber );
}


void do_makespice( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int schance;
   OBJ_DATA *obj;

   switch ( ch->substate )
   {
      default:
         strcpy( arg, argument );

         if( arg[0] == '\0' )
         {
            send_to_char( "&RFrom what?\n\r&w", ch );
            return;
         }

         if( !IS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
         {
            send_to_char( "&RYou need to be in a refinery to create drugs from spice.\n\r", ch );
            return;
         }

         if( ms_find_obj( ch ) )
            return;

         if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
         {
            send_to_char( "&RYou do not have that item.\n\r&w", ch );
            return;
         }

         if( obj->item_type != ITEM_RAWSPICE )
         {
            send_to_char( "&RYou can't make a drug out of that\n\r&w", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_spice_refining] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of refining spice into a drug.\n\r", ch );
            act( AT_PLAIN, "$n begins working on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 10, do_makespice, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out what to do with the stuff.\n\r", ch );
         learn_from_failure( ch, gsn_spice_refining );
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
         send_to_char( "&RYou are distracted and are unable to finish your work.\n\r&w", ch );
         return;
   }

   ch->substate = SUB_NONE;

   if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You seem to have lost your spice!\n\r", ch );
      return;
   }
   if( obj->item_type != ITEM_RAWSPICE )
   {
      send_to_char( "&RYou get your tools mixed up and can't finish your work.\n\r&w", ch );
      return;
   }

   obj->value[1] = URANGE( 10, obj->value[1], ( IS_NPC( ch ) ? ch->top_level
                                                : ( int )( ch->pcdata->learned[gsn_spice_refining] ) ) + 10 );
   strcpy( buf, obj->name );
   STRFREE( obj->name );
   strcat( buf, " drug spice " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, "a drug made from " );
   strcat( buf, obj->short_descr );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   strcat( buf, " was foolishly left lying around here." );
   STRFREE( obj->description );
   obj->description = STRALLOC( buf );
   obj->item_type = ITEM_SPICE;

   send_to_char( "&GYou finish your work.\n\r", ch );
   act( AT_PLAIN, "$n finishes $s work.", ch, NULL, argument, TO_ROOM );

   obj->cost += obj->value[1] * 10;
   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }

   learn_from_success( ch, gsn_spice_refining );

}

void do_makegrenade( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance, strength, weight;
   bool checktool, checkdrink, checkbatt, checkchem, checkcirc;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makegrenade <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdrink = FALSE;
         checkbatt = FALSE;
         checkchem = FALSE;
         checkcirc = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
               checkdrink = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_CHEMICAL )
               checkchem = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a grenade.\n\r", ch );
            return;
         }

         if( !checkdrink )
         {
            send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a small battery for the timer.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit for the timer.\n\r", ch );
            return;
         }

         if( !checkchem )
         {
            send_to_char( "&RSome explosive chemicals would come in handy!\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makegrenade] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a grenade.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, do_makegrenade, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makegrenade );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makegrenade] );
   vnum = 71;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkdrink = FALSE;
   checkbatt = FALSE;
   checkchem = FALSE;
   checkcirc = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE && obj->value[1] == 0 )
      {
         checkdrink = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }
      if( obj->item_type == ITEM_CHEMICAL )
      {
         strength = URANGE( 10, obj->value[0], level * 5 );
         weight = obj->weight;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkchem = TRUE;
      }
      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcirc = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makegrenade] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( !checkchem )
       || ( !checkcirc ) )
   {
      send_to_char
         ( "&RJust as you are about to finish your work,\n\ryour newly created grenade explodes in your hands...doh!\n\r",
           ch );
      learn_from_failure( ch, gsn_makegrenade );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_GRENADE;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = weight;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " grenade " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   obj->value[0] = strength * 2;
   obj->value[1] = strength * 3;
   obj->cost = obj->value[1] * 5;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created grenade.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new grenade.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makegrenade );
}

void do_makelandmine( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance, strength, weight;
   bool checktool, checkdrink, checkbatt, checkchem, checkcirc;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makelandmine <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdrink = FALSE;
         checkbatt = FALSE;
         checkchem = FALSE;
         checkcirc = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
               checkdrink = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_CHEMICAL )
               checkchem = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a landmine.\n\r", ch );
            return;
         }

         if( !checkdrink )
         {
            send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a small battery for the detonator.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit for the detonator.\n\r", ch );
            return;
         }

         if( !checkchem )
         {
            send_to_char( "&RSome explosive chemicals would come in handy!\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelandmine] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a landmine.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, do_makelandmine, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makelandmine );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelandmine] );
   vnum = 70;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkdrink = FALSE;
   checkbatt = FALSE;
   checkchem = FALSE;
   checkcirc = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE && obj->value[1] == 0 )
      {
         checkdrink = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }
      if( obj->item_type == ITEM_CHEMICAL )
      {
         strength = URANGE( 10, obj->value[0], level * 5 );
         weight = obj->weight;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkchem = TRUE;
      }
      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcirc = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelandmine] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( !checkchem )
       || ( !checkcirc ) )
   {
      send_to_char
         ( "&RJust as you are about to finish your work,\n\ryour newly created landmine explodes in your hands...doh!\n\r",
           ch );
      learn_from_failure( ch, gsn_makelandmine );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_LANDMINE;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = weight;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " landmine " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   obj->value[0] = strength / 2;
   obj->value[1] = strength;
   obj->cost = obj->value[1] * 5;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created landmine.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new landmine.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makelandmine );
}
void do_makelight( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance, strength;
   bool checktool, checkbatt, checkchem, checkcirc, checklens;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makeflashlight <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkbatt = FALSE;
         checkchem = FALSE;
         checkcirc = FALSE;
         checklens = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_CHEMICAL )
               checkchem = TRUE;
            if( obj->item_type == ITEM_LENS )
               checklens = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a light.\n\r", ch );
            return;
         }

         if( !checklens )
         {
            send_to_char( "&RYou need a lens to make a light.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a battery for the light to work.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit.\n\r", ch );
            return;
         }

         if( !checkchem )
         {
            send_to_char( "&RSome chemicals to light would come in handy!\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelight] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a light.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 10, do_makelight, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makelight );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelight] );
   vnum = 65;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checklens = FALSE;
   checkbatt = FALSE;
   checkchem = FALSE;
   checkcirc = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         strength = obj->value[0];
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }
      if( obj->item_type == ITEM_CHEMICAL )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkchem = TRUE;
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
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makelight] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checklens ) || ( !checkbatt ) || ( !checkchem )
       || ( !checkcirc ) )
   {
      send_to_char
         ( "&RJust as you are about to finish your work,\n\ryour newly created light explodes in your hands...doh!\n\r",
           ch );
      learn_from_failure( ch, gsn_makelight );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_LIGHT;
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 3;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " light " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   obj->value[2] = strength;
   obj->cost = obj->value[2];

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created light.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new light.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 100,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makelight );
}

void do_makejewelry( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checktool, checkoven, checkmetal;
   OBJ_DATA *obj;
   OBJ_DATA *metal;
   AFFECT_DATA *paf;
   int value, cost;

   argument = one_argument( argument, arg );
   strcpy( arg2, argument );

   if( !str_cmp( arg, "body" )
       || !str_cmp( arg, "head" )
       || !str_cmp( arg, "legs" )
       || !str_cmp( arg, "arms" )
       || !str_cmp( arg, "about" )
       || !str_cmp( arg, "eyes" )
       || !str_cmp( arg, "waist" ) || !str_cmp( arg, "hold" ) || !str_cmp( arg, "feet" ) || !str_cmp( arg, "hands" ) )
   {
      send_to_char( "&RYou cannot make jewelry for that body part.\n\r&w", ch );
      send_to_char( "&RTry MAKEARMOR.\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "shield" ) )
   {
      send_to_char( "&RYou cannot make jewelry worn as a shield.\n\r&w", ch );
      send_to_char( "&RTry MAKESHIELD.\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "wield" ) )
   {
      send_to_char( "&RAre you going to fight with your jewelry?\n\r&w", ch );
      send_to_char( "&RTry MAKEBLADE...\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "holster1" ) || !str_cmp( arg, "holster2" ) )
   {
      send_to_char( "&RYou can't use jewelry there.&w\n\r", ch );
      send_to_char( "&RTry MAKECONTAINER...\n\r&w", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:

         if( arg2[0] == '\0' )
         {
            send_to_char( "&RUsage: Makejewelry <wearloc> <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkoven = FALSE;
         checkmetal = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_OVEN )
               checkoven = TRUE;
            if( obj->item_type == ITEM_RARE_METAL )
               checkmetal = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need a toolkit.\n\r", ch );
            return;
         }

         if( !checkoven )
         {
            send_to_char( "&RYou need an oven.\n\r", ch );
            return;
         }

         if( !checkmetal )
         {
            send_to_char( "&RYou need some precious metal.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makejewelry] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of creating some jewelry.\n\r", ch );
            act( AT_PLAIN, "$n takes $s toolkit and some metal and begins to work.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 15, do_makejewelry, 1 );
            ch->dest_buf = str_dup( arg );
            ch->dest_buf_2 = str_dup( arg2 );
            return;
         }
         send_to_char( "&RYou can't figure out what to do.\n\r", ch );
         learn_from_failure( ch, gsn_makejewelry );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         if( !ch->dest_buf_2 )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         strcpy( arg2, ch->dest_buf_2 );
         DISPOSE( ch->dest_buf_2 );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makejewelry] );

   checkmetal = FALSE;
   checkoven = FALSE;
   checktool = FALSE;
   value = 0;
   cost = 0;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_OVEN )
         checkoven = TRUE;
      if( obj->item_type == ITEM_RARE_METAL && checkmetal == FALSE )
      {
         checkmetal = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         metal = obj;
      }
      if( obj->item_type == ITEM_CRYSTAL )
      {
         cost += obj->cost;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makejewelry] );

   if( number_percent(  ) > schance * 2 || ( !checkoven ) || ( !checktool ) || ( !checkmetal ) )
   {
      send_to_char( "&RYou hold up your newly created jewelry.\n\r", ch );
      send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch );
      send_to_char( "&Rpiece of junk you've ever seen. You quickly hide your mistake...\n\r", ch );
      learn_from_failure( ch, gsn_makejewelry );
      return;
   }

   obj = metal;

   obj->item_type = ITEM_ARMOR;
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   value = get_wflag( arg );
   if( value < 0 || value > 31 )
      SET_BIT( obj->wear_flags, ITEM_WEAR_NECK );
   else
      SET_BIT( obj->wear_flags, 1 << value );
   obj->level = level;
   STRFREE( obj->name );
   sprintf( buf, "%s ", arg2 );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg2 );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was dropped here." );

// Create stat bonuses for Jewelry - Tawnos
   CREATE( paf, AFFECT_DATA, 1 );
   paf->type = -1;
   paf->duration = -1;
// Use smaller if check to minimize redundant code.
   if( IS_SET( obj->wear_flags, ITEM_WEAR_FINGER ) )
   {
      if( number_bits( 1 ) == 0 )
         paf->location = get_atype( "hitroll" );
      else
         paf->location = get_atype( "damroll" );

      paf->modifier = number_bits( 1 ) + 1;
   }
   else if( IS_SET( obj->wear_flags, ITEM_WEAR_EARS ) )
   {
      if( number_bits( 1 ) == 0 )
         paf->location = get_atype( "intelligence" );
      else
         paf->location = get_atype( "wisdom" );

      paf->modifier = number_bits( 1 ) + 1;
   }
   else if( IS_SET( obj->wear_flags, ITEM_WEAR_NECK ) )
   {
      if( number_bits( 1 ) == 0 )
         paf->location = get_atype( "charisma" );
      else
         paf->location = get_atype( "luck" );

      paf->modifier = 1;
   }
   else if( IS_SET( obj->wear_flags, ITEM_WEAR_WRIST ) )
   {
      if( number_bits( 1 ) == 0 )
         paf->location = get_atype( "strength" );
      else
         paf->location = get_atype( "dexterity" );

      paf->modifier = 1;
   }
   else
   {
      paf->location = get_atype( "strength" );
      paf->modifier = 1;
   }

   paf->bitvector = 0;
   paf->next = NULL;
   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
// End stat bonuses.

   obj->description = STRALLOC( buf );
   obj->value[1] = ( int )( ( ch->pcdata->learned[gsn_makejewelry] ) / 20 ) + metal->value[1];
   obj->value[0] = obj->value[1];
   obj->cost *= 10;
   obj->cost += cost;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created jewelry.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes sewing some new jewelry.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 100,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makejewelry );

}

void do_makearmor( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checksew, checkfab;
   OBJ_DATA *obj;
   OBJ_DATA *material;
   int value;

   argument = one_argument( argument, arg );
   strcpy( arg2, argument );

   if( !str_cmp( arg, "eyes" )
       || !str_cmp( arg, "ears" ) || !str_cmp( arg, "finger" ) || !str_cmp( arg, "neck" ) || !str_cmp( arg, "wrist" ) )
   {
      send_to_char( "&RYou cannot make clothing for that body part.\n\r&w", ch );
      send_to_char( "&RTry MAKEJEWELRY.\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "shield" ) )
   {
      send_to_char( "&RYou cannot make clothing worn as a shield.\n\r&w", ch );
      send_to_char( "&RTry MAKESHIELD.\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "wield" ) )
   {
      send_to_char( "&RAre you going to fight with your clothing?\n\r&w", ch );
      send_to_char( "&RTry MAKEBLADE...\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "holster1" ) || !str_cmp( arg, "holster2" ) )
   {
      send_to_char( "&RYou can't use armor there.&w\n\r", ch );
      send_to_char( "&RTry MAKECONTAINER...\n\r&w", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:

         if( arg2[0] == '\0' )
         {
            send_to_char( "&RUsage: Makearmor <wearloc> <name>\n\r&w", ch );
            return;
         }

         checksew = FALSE;
         checkfab = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_FABRIC )
               checkfab = TRUE;
            if( obj->item_type == ITEM_THREAD )
               checksew = TRUE;
         }

         if( !checkfab )
         {
            send_to_char( "&RYou need some sort of fabric or material.\n\r", ch );
            return;
         }

         if( !checksew )
         {
            send_to_char( "&RYou need a needle and some thread.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makearmor] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of creating some armor.\n\r", ch );
            act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 15, do_makearmor, 1 );
            ch->dest_buf = str_dup( arg );
            ch->dest_buf_2 = str_dup( arg2 );
            return;
         }
         send_to_char( "&RYou can't figure out what to do.\n\r", ch );
         learn_from_failure( ch, gsn_makearmor );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         if( !ch->dest_buf_2 )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         strcpy( arg2, ch->dest_buf_2 );
         DISPOSE( ch->dest_buf_2 );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makearmor] );

   checksew = FALSE;
   checkfab = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_THREAD )
         checksew = TRUE;
      if( obj->item_type == ITEM_FABRIC && checkfab == FALSE )
      {
         checkfab = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         material = obj;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makearmor] );

   if( number_percent(  ) > schance * 2 || ( !checkfab ) || ( !checksew ) )
   {
      send_to_char( "&RYou hold up your newly created armor.\n\r", ch );
      send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch );
      send_to_char( "&Rgarment you've ever seen. You quickly hide your mistake...\n\r", ch );
      learn_from_failure( ch, gsn_makearmor );
      return;
   }

   obj = material;

   obj->item_type = ITEM_ARMOR;
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   value = get_wflag( arg );
   if( value < 0 || value > 31 )
      SET_BIT( obj->wear_flags, ITEM_WEAR_BODY );
   else
      SET_BIT( obj->wear_flags, 1 << value );
   obj->level = level;
   STRFREE( obj->name );
   sprintf( buf, "%s ", arg2 );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg2 );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was dropped here." );
   obj->description = STRALLOC( buf );
   obj->value[0] = obj->value[1];
   obj->cost *= 10;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created garment.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes sewing some new armor.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 100,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makearmor );
}


void do_makecomlink( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int schance;
   bool checktool, checkgem, checkbatt, checkcirc;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;

   argument = one_argument( argument, arg );
   strcpy( arg2, argument );

   switch ( ch->substate )
   {
      default:

         if( arg2[0] == '\0' )
         {
            send_to_char( "&RUsage: Makecomlink <wearloc> <name>\n\r&w", ch );
            send_to_char( "&RAvailable wearlocs: hold, wrist, ears&w\n\r", ch );
            return;
         }

         checktool = FALSE;
         checkgem = FALSE;
         checkbatt = FALSE;
         checkcirc = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_CRYSTAL )
               checkgem = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a comlink.\n\r", ch );
            return;
         }

         if( !checkgem )
         {
            send_to_char( "&RYou need a small crystal.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a power source for your comlink.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecomlink] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a comlink.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 10, do_makecomlink, 1 );
            ch->dest_buf = str_dup( arg );
            ch->dest_buf_2 = str_dup( arg2 );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makecomlink );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         strcpy( arg2, ch->dest_buf_2 );
         DISPOSE( ch->dest_buf_2 );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   vnum = 64;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkgem = FALSE;
   checkbatt = FALSE;
   checkcirc = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_CRYSTAL && checkgem == FALSE )
      {
         checkgem = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         checkcirc = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecomlink] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkcirc ) || ( !checkbatt ) || ( !checkgem ) )
   {
      send_to_char( "&RYou hold up your newly created comlink....\n\r", ch );
      send_to_char( "&Rand it falls apart in your hands.\n\r", ch );
      learn_from_failure( ch, gsn_makecomlink );
      return;
   }

   obj = create_object( pObjIndex, ch->top_level );

   obj->item_type = ITEM_COMLINK;
   if( arg == NULL || !str_cmp( arg, "hold" ) )
      SET_BIT( obj->wear_flags, ITEM_HOLD );
   if( !str_cmp( arg, "ears" ) )
   {
      SET_BIT( obj->wear_flags, ITEM_WEAR_EARS );
      REMOVE_BIT( obj->wear_flags, ITEM_HOLD );
   }
   if( !str_cmp( arg, "wrist" ) )
   {
      SET_BIT( obj->wear_flags, ITEM_WEAR_WRIST );
      REMOVE_BIT( obj->wear_flags, ITEM_HOLD );
   }
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->weight = 3;
   STRFREE( obj->name );
   strcpy( buf, arg2 );
   strcat( buf, " comlink " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg2 );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was left here." );
   obj->description = STRALLOC( buf );
   obj->cost = 50;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created comlink.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes crafting a comlink.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 100,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makecomlink );

}

void do_makeshield( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int schance;
   bool checktool, checkbatt, checkcond, checkcirc, checkgems;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum, level, charge, gemtype;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makeshield <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkbatt = FALSE;
         checkcond = FALSE;
         checkcirc = FALSE;
         checkgems = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a workshop.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_CRYSTAL )
               checkgems = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_SUPERCONDUCTOR )
               checkcond = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make an energy shield.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a power source for your energy shield.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit board.\n\r", ch );
            return;
         }

         if( !checkcond )
         {
            send_to_char( "&RYou still need a small superconductor for your energy shield.\n\r", ch );
            return;
         }

         if( !checkgems )
         {
            send_to_char( "&RYou need a small crystal.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeshield] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of crafting an energy shield.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 20, do_makeshield, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makeshield );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeshield] );
   vnum = 28;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkbatt = FALSE;
   checkcond = FALSE;
   checkcirc = FALSE;
   checkgems = FALSE;
   charge = 0;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;

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
      if( obj->item_type == ITEM_CRYSTAL && checkgems == FALSE )
      {
         gemtype = obj->value[0];
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkgems = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeshield] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkbatt )
       || ( !checkgems ) || ( !checkcond ) || ( !checkcirc ) )

   {
      send_to_char( "&RYou hold up your new energy shield and press the switch hoping for the best.\n\r", ch );
      send_to_char( "&RInstead of a field of energy being created, smoke starts pouring from the device.\n\r", ch );
      send_to_char( "&RYou drop the hot device and watch as it melts on away on the floor.\n\r", ch );
      learn_from_failure( ch, gsn_makeshield );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_ARMOR;
   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_WEAR_SHIELD );
   obj->level = level;
   obj->weight = 2;
   STRFREE( obj->name );
   obj->name = STRALLOC( "energy shield" );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   obj->value[0] = ( int )( level / 10 + gemtype * 10 ); /* condition */
   obj->value[1] = ( int )( level / 10 + gemtype * 10 ); /* armor */
   obj->value[4] = charge;
   obj->value[5] = charge;
   obj->cost = obj->value[2] * 100;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created energy shield.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new energy shield.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makeshield );

}

void do_makecontainer( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checksew, checkfab;
   OBJ_DATA *obj;
   OBJ_DATA *material;
   int value;

   argument = one_argument( argument, arg );
   strcpy( arg2, argument );

   if( !str_cmp( arg, "eyes" )
       || !str_cmp( arg, "ears" ) || !str_cmp( arg, "finger" ) || !str_cmp( arg, "neck" ) || !str_cmp( arg, "wrist" ) )
   {
      send_to_char( "&RYou cannot make a container for that body part.\n\r&w", ch );
      send_to_char( "&RTry MAKEJEWELRY.\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "feet" ) || !str_cmp( arg, "hands" ) )
   {
      send_to_char( "&RYou cannot make a container for that body part.\n\r&w", ch );
      send_to_char( "&RTry MAKEARMOR.\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "shield" ) )
   {
      send_to_char( "&RYou cannot make a container a shield.\n\r&w", ch );
      send_to_char( "&RTry MAKESHIELD.\n\r&w", ch );
      return;
   }
   if( !str_cmp( arg, "wield" ) )
   {
      send_to_char( "&RAre you going to fight with a container?\n\r&w", ch );
      send_to_char( "&RTry MAKEBLADE...\n\r&w", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:

         if( arg2[0] == '\0' )
         {
            send_to_char( "&RUsage: Makecontainer <wearloc> <name>\n\r&w", ch );
            return;
         }

         checksew = FALSE;
         checkfab = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_FABRIC )
               checkfab = TRUE;
            if( obj->item_type == ITEM_THREAD )
               checksew = TRUE;
         }

         if( !checkfab )
         {
            send_to_char( "&RYou need some sort of fabric or material.\n\r", ch );
            return;
         }

         if( !checksew )
         {
            send_to_char( "&RYou need a needle and some thread.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecontainer] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of creating a bag.\n\r", ch );
            act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 10, do_makecontainer, 1 );
            ch->dest_buf = str_dup( arg );
            ch->dest_buf_2 = str_dup( arg2 );
            return;
         }
         send_to_char( "&RYou can't figure out what to do.\n\r", ch );
         learn_from_failure( ch, gsn_makecontainer );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         if( !ch->dest_buf_2 )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         strcpy( arg2, ch->dest_buf_2 );
         DISPOSE( ch->dest_buf_2 );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecontainer] );

   checksew = FALSE;
   checkfab = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_THREAD )
         checksew = TRUE;
      if( obj->item_type == ITEM_FABRIC && checkfab == FALSE )
      {
         checkfab = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         material = obj;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makecontainer] );

   if( number_percent(  ) > schance * 2 || ( !checkfab ) || ( !checksew ) )
   {
      send_to_char( "&RYou hold up your newly created container.\n\r", ch );
      send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch );
      send_to_char( "&Rcontainer you've ever seen. You quickly hide your mistake...\n\r", ch );
      learn_from_failure( ch, gsn_makecontainer );
      return;
   }

   obj = material;

   obj->item_type = ITEM_CONTAINER;
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   value = get_wflag( arg );
   if( value < 0 || value > 31 )
      SET_BIT( obj->wear_flags, ITEM_HOLD );
   else
      SET_BIT( obj->wear_flags, 1 << value );
   obj->level = level;
   STRFREE( obj->name );
   strcpy( buf, arg2 );
   strcat( buf, " " );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg2 );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was dropped here." );
   obj->description = STRALLOC( buf );
   obj->value[0] = level;
   obj->value[1] = 0;
   obj->value[2] = 0;
   obj->value[3] = 10;
   obj->cost *= 2;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created container.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes sewing a new container.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 100,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.\n\r", xpgain );
   }
   learn_from_success( ch, gsn_makecontainer );
}

void do_gemcutting( CHAR_DATA * ch, char *argument )
{
   send_to_char( "&RSorry, this skill isn't finished yet :(\n\r", ch );
}

void do_reinforcements( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance, credits;

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( ch->backup_wait )
         {
            send_to_char( "&RYour reinforcements are already on the way.\n\r", ch );
            return;
         }

         if( !ch->pcdata->clan )
         {
            send_to_char( "&RYou need to be a member of an organization before you can call for reinforcements.\n\r", ch );
            return;
         }

         if( ch->gold < ch->skill_level[POLITICIAN_ABILITY] * 50 )
         {
            ch_printf( ch, "&RYou dont have enough credits to send for reinforcements.\n\r" );
            return;
         }

         schance = ( int )( ch->pcdata->learned[gsn_reinforcements] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch );
            act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 1, do_reinforcements, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou call for reinforcements but nobody answers.\n\r", ch );
         learn_from_failure( ch, gsn_reinforcements );
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
         send_to_char( "&RYou are interupted before you can finish your call.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   send_to_char( "&GYour reinforcements are on the way.\n\r", ch );
   credits = ch->skill_level[POLITICIAN_ABILITY] * 50;
   ch_printf( ch, "It cost you %d credits.\n\r", credits );
   ch->gold -= UMIN( credits, ch->gold );

   learn_from_success( ch, gsn_reinforcements );

   if( nifty_is_name( "empire", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_STORMTROOPER;
   else if( nifty_is_name( "republic", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_NR_TROOPER;
   else
      ch->backup_mob = MOB_VNUM_MERCINARY;

   ch->backup_wait = number_range( 1, 2 );

}

void do_postguard( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance, credits;

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( ch->backup_wait )
         {
            send_to_char( "&RYou already have backup coming.\n\r", ch );
            return;
         }

         if( !ch->pcdata->clan )
         {
            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
            return;
         }

         if( ch->gold < ch->skill_level[POLITICIAN_ABILITY] * 30 )
         {
            ch_printf( ch, "&RYou dont have enough credits.\n\r", ch );
            return;
         }

         schance = ( int )( ch->pcdata->learned[gsn_postguard] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch );
            act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 1, do_postguard, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou call for a guard but nobody answers.\n\r", ch );
         learn_from_failure( ch, gsn_postguard );
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
         send_to_char( "&RYou are interupted before you can finish your call.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   send_to_char( "&GYour guard is on the way.\n\r", ch );

   credits = ch->skill_level[POLITICIAN_ABILITY] * 30;
   ch_printf( ch, "It cost you %d credits.\n\r", credits );
   ch->gold -= UMIN( credits, ch->gold );

   learn_from_success( ch, gsn_postguard );

   if( nifty_is_name( "empire", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_IMP_GUARD;
   else if( nifty_is_name( "republic", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_NR_GUARD;
   else
      ch->backup_mob = MOB_VNUM_BOUNCER;

   ch->backup_wait = 1;

}

void add_reinforcements( CHAR_DATA * ch )
{
   MOB_INDEX_DATA *pMobIndex;
   OBJ_DATA *blaster;
   OBJ_INDEX_DATA *pObjIndex;
   int max = 1;

   if( ( pMobIndex = get_mob_index( ch->backup_mob ) ) == NULL )
      return;

   if( ch->backup_mob == MOB_VNUM_STORMTROOPER ||
       ch->backup_mob == MOB_VNUM_NR_TROOPER ||
       ch->backup_mob == MOB_VNUM_MERCINARY ||
       ch->backup_mob == MOB_VNUM_IMP_FORCES ||
       ch->backup_mob == MOB_VNUM_NR_FORCES || ch->backup_mob == MOB_VNUM_MERC_FORCES )
   {
      CHAR_DATA *mob[3];
      int mob_cnt;

      send_to_char( "Your reinforcements have arrived.\n\r", ch );
      if( ch->backup_mob == MOB_VNUM_STORMTROOPER || ch->backup_mob ==
          MOB_VNUM_NR_TROOPER || ch->backup_mob == MOB_VNUM_MERCINARY )
         max = 3;
      for( mob_cnt = 0; mob_cnt < max; mob_cnt++ )
      {
         int ability;
         mob[mob_cnt] = create_mobile( pMobIndex );
         char_to_room( mob[mob_cnt], ch->in_room );
         act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[mob_cnt], TO_ROOM );
         mob[mob_cnt]->top_level = ch->skill_level[POLITICIAN_ABILITY] / 3;
         for( ability = 0; ability < MAX_ABILITY; ability++ )
            mob[mob_cnt]->skill_level[ability] = mob[mob_cnt]->top_level;
         mob[mob_cnt]->hit = mob[mob_cnt]->top_level * 15;
         mob[mob_cnt]->max_hit = mob[mob_cnt]->hit;
         mob[mob_cnt]->armor = 100 - mob[mob_cnt]->top_level * 2.5;
         mob[mob_cnt]->damroll = mob[mob_cnt]->top_level / 5;
         mob[mob_cnt]->hitroll = mob[mob_cnt]->top_level / 5;
         if( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
         {
            blaster = create_object( pObjIndex, mob[mob_cnt]->top_level );
            obj_to_char( blaster, mob[mob_cnt] );
            equip_char( mob[mob_cnt], blaster, WEAR_WIELD );
         }
         if( mob[mob_cnt]->master )
            stop_follower( mob[mob_cnt] );
         add_follower( mob[mob_cnt], ch );
         SET_BIT( mob[mob_cnt]->affected_by, AFF_CHARM );
         do_setblaster( mob[mob_cnt], "full" );
      }
   }
   else
   {
      CHAR_DATA *mob;
      int ability;

      mob = create_mobile( pMobIndex );
      char_to_room( mob, ch->in_room );
      if( ch->pcdata && ch->pcdata->clan )
      {
         char tmpbuf[MAX_STRING_LENGTH];

         //STRFREE( mob->name );
         //mob->name = STRALLOC( ch->pcdata->clan->name );
         sprintf( tmpbuf, "(%s) %s", ch->pcdata->clan->name, mob->long_descr );
         STRFREE( mob->long_descr );
         mob->long_descr = STRALLOC( tmpbuf );
      }
      act( AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM );
      send_to_char( "Your guard has arrived.\n\r", ch );
      mob->top_level = UMIN( ch->skill_level[POLITICIAN_ABILITY], 30 );
      for( ability = 0; ability < MAX_ABILITY; ability++ )
         mob->skill_level[ability] = mob->top_level;
      mob->hit = mob->top_level * 15;
      mob->max_hit = mob->hit;
      mob->armor = 100 - mob->top_level * 2.5;
      mob->damroll = mob->top_level / 5;
      mob->hitroll = mob->top_level / 5;
      if( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
      {
         blaster = create_object( pObjIndex, mob->top_level );
         obj_to_char( blaster, mob );
         equip_char( mob, blaster, WEAR_WIELD );
      }

      /*
       * for making this more accurate in the future 
       */

      if( mob->mob_clan )
         STRFREE( mob->mob_clan );
      if( ch->pcdata && ch->pcdata->clan )
         mob->mob_clan = STRALLOC( ch->pcdata->clan->name );
   }
}

void do_torture( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int schance, dam;
   bool fail;

   if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_torture] <= 0 )
   {
      send_to_char( "Your mind races as you realize you have no idea how to do that.\n\r", ch );
      return;
   }

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      send_to_char( "You can't do that right now.\n\r", ch );
      return;
   }

   one_argument( argument, arg );

   if( ch->mount )
   {
      send_to_char( "You can't get close enough while mounted.\n\r", ch );
      return;
   }

   if( arg[0] == '\0' )
   {
      send_to_char( "Torture whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Are you masacistic or what...\n\r", ch );
      return;
   }

   if( !IS_AWAKE( victim ) )
   {
      send_to_char( "You need to wake them first.\n\r", ch );
      return;
   }

   if( is_safe( ch, victim ) )
      return;

   if( victim->fighting )
   {
      send_to_char( "You can't torture someone whos in combat.\n\r", ch );
      return;
   }

   ch->alignment = ch->alignment -= 100;
   ch->alignment = URANGE( -1000, ch->alignment, 1000 );

   WAIT_STATE( ch, skill_table[gsn_torture]->beats );

   fail = FALSE;
   schance = ris_save( victim, ch->skill_level[POLITICIAN_ABILITY], RIS_PARALYSIS );
   if( schance == 1000 )
      fail = TRUE;
   else
      fail = saves_para_petri( schance, victim );

   if( !IS_NPC( ch ) && !IS_NPC( victim ) )
      schance = sysdata.stun_plr_vs_plr;
   else
      schance = sysdata.stun_regular;
   if( ( !fail
         && ( IS_NPC( ch )
              || ( number_percent(  ) + schance ) < ch->pcdata->learned[gsn_torture] + 20 ) ) || ( ( !IS_NPC( victim ) )
                                                                                                   &&
                                                                                                   ( IS_SET
                                                                                                     ( victim->pcdata->act2,
                                                                                                       ACT_BOUND ) ) ) )
   {
      learn_from_success( ch, gsn_torture );
      WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      WAIT_STATE( victim, PULSE_VIOLENCE );
      act( AT_SKILL, "$N slowly tortures you. The pain is excruciating.", victim, NULL, ch, TO_CHAR );
      act( AT_SKILL, "You torture $N, leaving $M screaming in pain.", ch, NULL, victim, TO_CHAR );
      act( AT_SKILL, "$n tortures $N, leaving $M screaming in agony!", ch, NULL, victim, TO_NOTVICT );

      dam = dice( ch->skill_level[POLITICIAN_ABILITY] / 10, 4 );
      dam = URANGE( 0, victim->max_hit - 10, dam );
      victim->hit -= dam;
      victim->max_hit -= dam;

      ch_printf( victim, "You lose %d permanent hit points.", dam );
      ch_printf( ch, "They lose %d permanent hit points.", dam );

   }
   else
   {
      act( AT_SKILL, "$N tries to cut off your finger!", victim, NULL, ch, TO_CHAR );
      act( AT_SKILL, "You mess up big time.", ch, NULL, victim, TO_CHAR );
      act( AT_SKILL, "$n tries to painfully torture $N.", ch, NULL, victim, TO_NOTVICT );
      WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
   }
   return;

}

void do_disguise( CHAR_DATA * ch, char *argument )
{
   int schance;

   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      send_to_char( "You try but the Force resists you.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: disguise <disguise|clear>\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      STRFREE( ch->pcdata->disguise );
      ch->pcdata->disguise = STRALLOC( "" );
      send_to_char( "Disguise cleared.\n\r", ch );
      return;
   }

   schance = ( int )( ch->pcdata->learned[gsn_disguise] );

   if( number_percent(  ) > schance )
   {
      send_to_char( "You try to disguise yourself but fail.\n\r", ch );
      return;
   }

   if( strlen( argument ) > 40 )
      argument[40] = '\0';

   learn_from_success( ch, gsn_disguise );

   smash_tilde( argument );

   STRFREE( ch->pcdata->disguise );
   ch->pcdata->disguise = STRALLOC( argument );
   send_to_char( "Ok.\n\r", ch );
}


void do_deception( CHAR_DATA * ch, char *argument )
{
   int schance;

   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      send_to_char( "You try but the Force resists you.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: deception <name|clear>\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      STRFREE( ch->pcdata->disguise );
      ch->pcdata->disguise = STRALLOC( "" );
      send_to_char( "You cease your deception.\n\r", ch );
      return;
   }

   schance = ( int )( ch->pcdata->learned[gsn_deception] );

   if( number_percent(  ) > schance )
   {
      send_to_char( "You fail to deceive others.\n\r", ch );
      return;
   }

   if( strlen( argument ) > 40 )
      argument[40] = '\0';

   learn_from_success( ch, gsn_deception );

   smash_tilde( argument );

   STRFREE( ch->pcdata->disguise );
   ch->pcdata->disguise = STRALLOC( argument );

   send_to_char( "Ok.\n\r", ch );
}

void do_mine( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   bool shovel;
   short move;

   if( ch->pcdata->learned[gsn_mine] <= 0 )
   {
      ch_printf( ch, "You have no idea how to do that.\n\r" );
      return;
   }

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "And what will you mine the room with?\n\r", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   shovel = FALSE;
   for( obj = ch->first_carrying; obj; obj = obj->next_content )
      if( obj->item_type == ITEM_SHOVEL )
      {
         shovel = TRUE;
         break;
      }

   obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
   if( !obj )
   {
      send_to_char( "You don't see one here.\n\r", ch );
      return;
   }

   separate_obj( obj );
   if( obj->item_type != ITEM_LANDMINE )
   {
      act( AT_PLAIN, "That's not a landmine!", ch, obj, 0, TO_CHAR );
      return;
   }

   if( !CAN_WEAR( obj, ITEM_TAKE ) )
   {
      act( AT_PLAIN, "You cannot bury $p.", ch, obj, 0, TO_CHAR );
      return;
   }

   switch ( ch->in_room->sector_type )
   {
      case SECT_CITY:
      case SECT_INSIDE:
         send_to_char( "The floor is too hard to dig through.\n\r", ch );
         return;
      case SECT_WATER_SWIM:
      case SECT_WATER_NOSWIM:
      case SECT_UNDERWATER:
         send_to_char( "You cannot bury a mine in the water.\n\r", ch );
         return;
      case SECT_AIR:
         send_to_char( "What?  Bury a mine in the air?!\n\r", ch );
         return;
   }

   if( obj->weight > ( UMAX( 5, ( can_carry_w( ch ) / 10 ) ) ) && !shovel )
   {
      send_to_char( "You'd need a shovel to bury something that big.\n\r", ch );
      return;
   }

   move = ( obj->weight * 50 * ( shovel ? 1 : 5 ) ) / UMAX( 1, can_carry_w( ch ) );
   move = URANGE( 2, move, 1000 );
   if( move > ch->move )
   {
      send_to_char( "You don't have the energy to bury something of that size.\n\r", ch );
      return;
   }
   ch->move -= move;

   SET_BIT( obj->extra_flags, ITEM_BURRIED );
   WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );

   STRFREE( obj->armed_by );
   obj->armed_by = STRALLOC( ch->name );

   ch_printf( ch, "You arm and bury %s.\n\r", obj->short_descr );
   act( AT_PLAIN, "$n arms and buries $p.", ch, obj, NULL, TO_ROOM );

   learn_from_success( ch, gsn_mine );

   return;
}

void do_first_aid( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *medpac;
   CHAR_DATA *victim;
   int heal;
   char buf[MAX_STRING_LENGTH];

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "You can't do that while fighting!\n\r", ch );
      return;
   }

   medpac = get_eq_char( ch, WEAR_HOLD );
   if( !medpac || medpac->item_type != ITEM_MEDPAC )
   {
      send_to_char( "You need to be holding a medpac.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags2, ROOM_ARENA ) )
   {
      send_to_char( "Medpacs in the arena? There's no honor in that!\n\r", ch );
      return;
   }

   if( medpac->value[0] <= 0 )
   {
      send_to_char( "Your medpac seems to be empty.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
      victim = ch;
   else
      victim = get_char_room( ch, argument );

   if( !victim )
   {
      ch_printf( ch, "I don't see any %s here...\n\r", argument );
      return;
   }

   heal = number_range( 1, 150 );

   if( heal > ch->pcdata->learned[gsn_first_aid] * 2 )
   {
      ch_printf( ch, "You fail in your attempt at first aid.\n\r" );
      learn_from_failure( ch, gsn_first_aid );
      return;
   }

   if( victim == ch )
   {
      ch_printf( ch, "You tend to your wounds.\n\r" );
      sprintf( buf, "$n uses %s to help heal $s wounds.", medpac->short_descr );
      act( AT_ACTION, buf, ch, NULL, victim, TO_ROOM );
   }
   else
   {
      sprintf( buf, "You tend to $N's wounds." );
      act( AT_ACTION, buf, ch, NULL, victim, TO_CHAR );
      sprintf( buf, "$n uses %s to help heal $N's wounds.", medpac->short_descr );
      act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );
      sprintf( buf, "$n uses %s to help heal your wounds.", medpac->short_descr );
      act( AT_ACTION, buf, ch, NULL, victim, TO_VICT );
   }

   --medpac->value[0];
   victim->hit += URANGE( 0, heal, victim->max_hit - victim->hit );

   learn_from_success( ch, gsn_first_aid );
}

void do_snipe( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *wield;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   short dir, dist;
   short max_dist = 3;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *was_in_room;
   ROOM_INDEX_DATA *to_room;
   CHAR_DATA *victim;
   int schance;
   char buf[MAX_STRING_LENGTH];
   bool pfound = FALSE;

   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You'll have to do that elswhere.\n\r", ch );
      return;
   }

   if( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
   {
      send_to_char( "You can't do that while wielding two weapons.", ch );
      return;
   }

   wield = get_eq_char( ch, WEAR_WIELD );
   if( !wield || wield->item_type != ITEM_WEAPON || wield->value[3] != WEAPON_BLASTER )
   {
      send_to_char( "You don't seem to be holding a blaster", ch );
      return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( ( dir = get_door( arg ) ) == -1 || arg2[0] == '\0' )
   {
      send_to_char( "Usage: snipe <dir> <target>\n\r", ch );
      return;
   }

   if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
   {
      send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
      return;
   }

   if( IS_SET( pexit->exit_info, EX_CLOSED ) )
   {
      send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
      return;
   }

   was_in_room = ch->in_room;

   for( dist = 0; dist <= max_dist; dist++ )
   {
      if( IS_SET( pexit->exit_info, EX_CLOSED ) )
         break;

      if( !pexit->to_room )
         break;

      to_room = NULL;
      if( pexit->distance > 1 )
         to_room = generate_exit( ch->in_room, &pexit );

      if( to_room == NULL )
         to_room = pexit->to_room;

      char_from_room( ch );
      char_to_room( ch, to_room );


      if( IS_NPC( ch ) && ( victim = get_char_room_mp( ch, arg2 ) ) != NULL )
      {
         pfound = TRUE;
         break;
      }
      else if( !IS_NPC( ch ) && ( victim = get_char_room( ch, arg2 ) ) != NULL )
      {
         pfound = TRUE;
         break;
      }


      if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
         break;

   }

   char_from_room( ch );
   char_to_room( ch, was_in_room );

   if( !pfound )
   {
      ch_printf( ch, "You don't see that person to the %s!\n\r", dir_name[dir] );
      char_from_room( ch );
      char_to_room( ch, was_in_room );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Shoot yourself ... really?\n\r", ch );
      return;
   }

   if( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You can't shoot them there.\n\r", ch );
      return;
   }

   if( is_safe( ch, victim ) )
      return;

   if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
   {
      act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "You do the best you can!\n\r", ch );
      return;
   }

   if( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
   {
      send_to_char( "You feel too nice to do that!\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? 100 : ( int )( ch->pcdata->learned[gsn_snipe] );

   switch ( dir )
   {
      case 0:
      case 1:
         dir += 2;
         break;
      case 2:
      case 3:
         dir -= 2;
         break;
      case 4:
      case 7:
         dir += 1;
         break;
      case 5:
      case 8:
         dir -= 1;
         break;
      case 6:
         dir += 3;
         break;
      case 9:
         dir -= 3;
         break;
   }

   char_from_room( ch );
   char_to_room( ch, victim->in_room );

   if( number_percent(  ) < schance )
   {
      sprintf( buf, "A blaster shot fires at you from the %s.", dir_name[dir] );
      act( AT_ACTION, buf, victim, NULL, ch, TO_CHAR );
      act( AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR );
      sprintf( buf, "A blaster shot fires at $N from the %s.", dir_name[dir] );
      act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );

      one_hit( ch, victim, TYPE_UNDEFINED );

      if( char_died( ch ) )
         return;

      stop_fighting( ch, TRUE );

      learn_from_success( ch, gsn_snipe );
   }
   else
   {
      act( AT_ACTION, "You fire at $N but don't even come close.", ch, NULL, victim, TO_CHAR );
      sprintf( buf, "A blaster shot fired from the %s barely misses you.", dir_name[dir] );
      act( AT_ACTION, buf, ch, NULL, victim, TO_ROOM );
      learn_from_failure( ch, gsn_snipe );
   }

   char_from_room( ch );
   char_to_room( ch, was_in_room );

   if( IS_NPC( ch ) )
      WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
   else
   {
      if( number_percent(  ) < ch->pcdata->learned[gsn_third_attack] )
         WAIT_STATE( ch, 1 * PULSE_PER_SECOND );
      else if( number_percent(  ) < ch->pcdata->learned[gsn_second_attack] )
         WAIT_STATE( ch, 2 * PULSE_PER_SECOND );
      else
         WAIT_STATE( ch, 3 * PULSE_PER_SECOND );
   }
   if( IS_NPC( victim ) && !char_died( victim ) )
   {
      if( IS_SET( victim->act, ACT_SENTINEL ) )
      {
         victim->was_sentinel = victim->in_room;
         REMOVE_BIT( victim->act, ACT_SENTINEL );
      }

      start_hating( victim, ch );
      start_hunting( victim, ch );

   }

}

/* syntax throw <obj> [direction] [target] */

void do_throw( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   OBJ_DATA *tmpobj;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   short dir;
   SHIP_DATA *ship;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *was_in_room;
   ROOM_INDEX_DATA *to_room;
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];


   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   was_in_room = ch->in_room;

   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: throw <object> [direction] [target]\n\r", ch );
      return;
   }


   obj = get_eq_char( ch, WEAR_MISSILE_WIELD );
   if( !obj || !nifty_is_name( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_HOLD );
   if( !obj || !nifty_is_name( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_WIELD );
   if( !obj || !nifty_is_name( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_DUAL_WIELD );
   if( !obj || !nifty_is_name( arg, obj->name ) )
      if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
         obj = get_eq_char( ch, WEAR_HOLD );
   if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_WIELD );
   if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_DUAL_WIELD );
   if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
   {
      ch_printf( ch, "You don't seem to be holding or wielding %s.\n\r", arg );
      return;
   }

   if( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
   {
      act( AT_PLAIN, "You can't throw $p.", ch, obj, NULL, TO_CHAR );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      victim = who_fighting( ch );
      if( char_died( victim ) )
         return;
      act( AT_ACTION, "You throw $p at $N.", ch, obj, victim, TO_CHAR );
      act( AT_ACTION, "$n throws $p at $N.", ch, obj, victim, TO_NOTVICT );
      act( AT_ACTION, "$n throw $p at you.", ch, obj, victim, TO_VICT );
   }
   else if( arg2[0] == '\0' )
   {
      sprintf( buf, "$n throws %s at the floor.", obj->short_descr );
      act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
      ch_printf( ch, "You throw %s at the floor.\n\r", obj->short_descr );

      victim = NULL;
   }
   else if( ( dir = get_door( arg2 ) ) != -1 )
   {
      if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
      {
         send_to_char( "Are you expecting to throw it through a wall!?\n\r", ch );
         return;
      }


      if( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         send_to_char( "Are you expecting to throw it  through a door!?\n\r", ch );
         return;
      }


      switch ( dir )
      {
         case 0:
         case 1:
            dir += 2;
            break;
         case 2:
         case 3:
            dir -= 2;
            break;
         case 4:
         case 7:
            dir += 1;
            break;
         case 5:
         case 8:
            dir -= 1;
            break;
         case 6:
            dir += 3;
            break;
         case 9:
            dir -= 3;
            break;
      }

      to_room = NULL;
      if( pexit->distance > 1 )
         to_room = generate_exit( ch->in_room, &pexit );

      if( to_room == NULL )
         to_room = pexit->to_room;


      char_from_room( ch );
      char_to_room( ch, to_room );

      victim = get_char_room( ch, arg3 );

      if( victim )
      {
         if( is_safe( ch, victim ) )
            return;

         if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
         {
            act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
            return;
         }

         if( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
         {
            send_to_char( "You feel too nice to do that!\n\r", ch );
            return;
         }

         char_from_room( ch );
         char_to_room( ch, was_in_room );


         if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
         {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "You'll have to do that elswhere.\n\r", ch );
            return;
         }

         to_room = NULL;
         if( pexit->distance > 1 )
            to_room = generate_exit( ch->in_room, &pexit );

         if( to_room == NULL )
            to_room = pexit->to_room;


         char_from_room( ch );
         char_to_room( ch, to_room );

         sprintf( buf, "Someone throws %s at you from the %s.", obj->short_descr, dir_name[dir] );
         act( AT_ACTION, buf, victim, NULL, ch, TO_CHAR );
         act( AT_ACTION, "You throw $p at $N.", ch, obj, victim, TO_CHAR );
         sprintf( buf, "%s is thrown at $N from the %s.", obj->short_descr, dir_name[dir] );
         act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );


      }
      else
      {
         ch_printf( ch, "You throw %s %s.\n\r", obj->short_descr, dir_name[get_dir( arg2 )] );
         sprintf( buf, "%s is thrown from the %s.", obj->short_descr, dir_name[dir] );
         act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );

      }
   }
   else if( ( victim = get_char_room( ch, arg2 ) ) != NULL )
   {
      if( is_safe( ch, victim ) )
         return;

      if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
      {
         act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
         return;
      }

      if( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
      {
         send_to_char( "You feel too nice to do that!\n\r", ch );
         return;
      }

   }
   else
   {
      if( ( ship = ship_in_room( ch->in_room, arg2 ) ) == NULL )
      {
         act( AT_PLAIN, "I don't see that ship or person here.", ch, NULL, argument, TO_CHAR );
         return;
      }
      else
      {
         if( !ship->hatchopen )
         {
            send_to_char( "The hatch is closed!\n\r", ch );
            return;
         }

         if( !ship->entrance )
         {
            send_to_char( "That ship has no entrance!\n\r", ch );
            return;
         }

         unequip_char( ch, obj );
         separate_obj( obj );
         obj_from_char( obj );
         obj = obj_to_room( obj, get_room_index( ship->entrance ) );
         sprintf( buf, "You throw %s into %s.", obj->short_descr, ship->name );
         send_to_char( buf, ch );
         sprintf( buf, "%s throws %s into %s.\n\r", ch->name, obj->short_descr, ship->name );
         act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
         sprintf( buf, "%s is tossed into the ship with a *clink-clink-clink*.\n\r", obj->short_descr );
         echo_to_room( AT_WHITE, get_room_index( ship->entrance ), buf );
         return;
      }
   }


   if( obj == get_eq_char( ch, WEAR_WIELD ) && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
      tmpobj->wear_loc = WEAR_WIELD;

   unequip_char( ch, obj );
   separate_obj( obj );
   obj_from_char( obj );
   obj = obj_to_room( obj, ch->in_room );

   if( obj->item_type != ITEM_GRENADE )
      damage_obj( obj );

/* NOT NEEDED UNLESS REFERING TO OBJECT AGAIN 

   if( obj_extracted(obj) )
      return;
*/
   if( ch->in_room != was_in_room )
   {
      char_from_room( ch );
      char_to_room( ch, was_in_room );
   }

   if( !victim || char_died( victim ) )
      learn_from_failure( ch, gsn_throw );
   else
   {

      WAIT_STATE( ch, skill_table[gsn_throw]->beats );
      if( IS_NPC( ch ) || number_percent(  ) < ch->pcdata->learned[gsn_throw] )
      {
         learn_from_success( ch, gsn_throw );
         global_retcode =
            damage( ch, victim, number_range( obj->weight * 2, ( obj->weight * 2 + ch->perm_str ) ), TYPE_HIT );
      }
      else
      {
         learn_from_failure( ch, gsn_throw );
         global_retcode = damage( ch, victim, 0, TYPE_HIT );
      }

      if( IS_NPC( victim ) && !char_died( victim ) )
      {
         if( IS_SET( victim->act, ACT_SENTINEL ) )
         {
            victim->was_sentinel = victim->in_room;
            REMOVE_BIT( victim->act, ACT_SENTINEL );
         }

         start_hating( victim, ch );
         start_hunting( victim, ch );

      }

   }

   return;

}

void do_beg( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int percent, xp;
   int amount;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg1 );

   if( ch->mount )
   {
      send_to_char( "You can't do that while mounted.\n\r", ch );
      return;
   }

   if( arg1[0] == '\0' )
   {
      send_to_char( "Beg fo money from whom?\n\r", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "That's pointless.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "This isn't a good place to do that.\n\r", ch );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "Interesting combat technique.\n\r", ch );
      return;
   }

   if( victim->position == POS_FIGHTING )
   {
      send_to_char( "They're a little busy right now.\n\r", ch );
      return;
   }

   if( ch->position <= POS_SLEEPING )
   {
      send_to_char( "In your dreams or what?\n\r", ch );
      return;
   }

   if( victim->position <= POS_SLEEPING )
   {
      send_to_char( "You might want to wake them first...\n\r", ch );
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "You beg them for money.\n\r", ch );
      act( AT_ACTION, "$n begs you to give $s some change.\n\r", ch, NULL, victim, TO_VICT );
      act( AT_ACTION, "$n begs $N for change.\n\r", ch, NULL, victim, TO_NOTVICT );
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_beg]->beats );
   percent = number_percent(  ) + ch->skill_level[SMUGGLING_ABILITY] + victim->top_level;

   if( percent > ch->pcdata->learned[gsn_beg] )
   {
      /*
       * Failure.
       */
      send_to_char( "You beg them for money but don't get any!\n\r", ch );
      act( AT_ACTION, "$n is really getting on your nerves with all this begging!\n\r", ch, NULL, victim, TO_VICT );
      act( AT_ACTION, "$n begs $N for money.\n\r", ch, NULL, victim, TO_NOTVICT );

      if( victim->alignment < 0 && victim->top_level >= ch->top_level + 5 )
      {
         sprintf( buf, "%s is an annoying beggar and needs to be taught a lesson!", ch->name );
         do_yell( victim, buf );
         global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
      }

      learn_from_failure( ch, gsn_beg );

      return;
   }


   act( AT_ACTION, "$n begs $N for money.\n\r", ch, NULL, victim, TO_NOTVICT );
   act( AT_ACTION, "$n begs you for money!\n\r", ch, NULL, victim, TO_VICT );

   amount = UMIN( victim->gold, number_range( 1, 10 ) );
   if( amount <= 0 )
   {
      do_look( victim, ch->name );
      do_say( victim, "Sorry I have nothing to spare.\n\r" );
      learn_from_failure( ch, gsn_beg );
      return;
   }

   ch->gold += amount;
   victim->gold -= amount;
   ch_printf( ch, "%s gives you %d credits.\n\r", victim->short_descr, amount );
   learn_from_success( ch, gsn_beg );
   xp =
      UMIN( amount * 10,
            ( exp_level( ch->skill_level[SMUGGLING_ABILITY] + 1 ) - exp_level( ch->skill_level[SMUGGLING_ABILITY] ) ) );
   xp = UMIN( xp, xp_compute( ch, victim ) );
   gain_exp( ch, xp, SMUGGLING_ABILITY );
   ch_printf( ch, "&WYou gain %ld smuggling experience points!\n\r", xp );
   act( AT_ACTION, "$N gives $n some money.\n\r", ch, NULL, victim, TO_NOTVICT );
   act( AT_ACTION, "You give $n some money.\n\r", ch, NULL, victim, TO_VICT );

   return;

}

void do_pickshiplock( CHAR_DATA * ch, char *argument )
{
   do_pick( ch, argument );
}

void do_hijack( CHAR_DATA * ch, char *argument )
{
   int schance;
   int x;
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];
   bool uhoh = FALSE;
//    CHAR_DATA *guard;      <--- For the guard shits below
//    ROOM_INDEX_DATA *room;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }

   if( ship->class > SHIP_SPACE_STATION )
   {
      send_to_char( "&RThis isn't a spacecraft!\n\r", ch );
      return;
   }

   if( check_pilot( ch, ship ) )
   {
      send_to_char( "&RWhat would be the point of that!\n\r", ch );
      return;
   }

   if( ship->type == MOB_SHIP && get_trust( ch ) < 102 )
   {
      send_to_char( "&RThis ship isn't pilotable by mortals at this point in time...\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
   }

   if( ship->lastdoc != ship->location )
   {
      send_to_char( "&rYou don't seem to be docked right now.\n\r", ch );
      return;
   }

   if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
   {
      send_to_char( "The ship is not docked right now.\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "The ships drive is disabled .\n\r", ch );
      return;
   }
   /*
    * Remind to fix later, 'cause I'm sick of reading through all this fucking code  
    * 
    * for ( room = ship->firstroom ; room ; room = room->next_in_ship )
    * {
    * for ( guard = room->first_person; guard ; guard = guard->next_in_room )
    * if ( IS_NPC(guard) && guard->pIndexData && guard->pIndexData->vnum == MOB_VNUM_SHIP_GUARD 
    * && guard->position > POS_SLEEPING && !guard->fighting )
    * {
    * start_hating( guard, ch );
    * start_hunting( guard , ch );
    * uhoh = TRUE;
    * }   
    * }
    */

   if( uhoh )
   {
      send_to_char( "Uh oh....\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? 0 : ( int )( ch->pcdata->learned[gsn_hijack] );
   x = number_percent(  );
   if( x > schance )
   {
      send_to_char( "You fail to figure out the correct launch code.\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? 0 : ( int )( ch->pcdata->learned[gsn_shipsystems] );
   if( number_percent(  ) < schance )
   {

      if( ship->hatchopen )
      {
         ship->hatchopen = FALSE;
         sprintf( buf, "The hatch on %s closes.", ship->name );
         echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
         echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch slides shut." );
      }
      set_char_color( AT_GREEN, ch );
      send_to_char( "Launch sequence initiated.\n\r", ch );
      act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch, NULL, argument, TO_ROOM );
      echo_to_ship( AT_YELLOW, ship, "The ship hums as it lifts off the ground." );
      sprintf( buf, "%s begins to launch.", ship->name );
      echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
      ship->shipstate = SHIP_LAUNCH;
      ship->currspeed = ship->realspeed;
      learn_from_success( ch, gsn_shipsystems );
      learn_from_success( ch, gsn_hijack );
//                   sprintf( buf, "%s has been hijacked!", ship->name );
//             echo_to_all( AT_RED , buf, 0 );

      return;
   }
   set_char_color( AT_RED, ch );
   send_to_char( "You fail to work the controls properly!\n\r", ch );
   return;

}

void do_special_forces( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance, credits;

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( ch->backup_wait )
         {
            send_to_char( "&RYour reinforcements are already on the way.\n\r", ch );
            return;
         }

         if( !ch->pcdata->clan )
         {
            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
            return;
         }

         if( ch->gold < ch->skill_level[POLITICIAN_ABILITY] * 350 )
         {
            ch_printf( ch, "&RYou dont have enough credits to send for reinforcements.\n\r" );
            return;
         }

         schance = ( int )( ch->pcdata->learned[gsn_specialforces] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch );
            act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 1, do_special_forces, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou call for reinforcements but nobody answers.\n\r", ch );
         learn_from_failure( ch, gsn_specialforces );
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
         send_to_char( "&RYou are interupted before you can finish your call.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   send_to_char( "&GYour reinforcements are on the way.\n\r", ch );
   credits = ch->skill_level[POLITICIAN_ABILITY] * 175;
   ch_printf( ch, "It cost you %d credits.\n\r", credits );
   ch->gold -= UMIN( credits, ch->gold );

   learn_from_success( ch, gsn_specialforces );

   if( nifty_is_name( "empire", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_IMP_FORCES;
   else if( nifty_is_name( "republic", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_NR_FORCES;
   else
      ch->backup_mob = MOB_VNUM_MERC_FORCES;

   ch->backup_wait = number_range( 1, 2 );

}


void do_elite_guard( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance, credits;

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( ch->backup_wait )
         {
            send_to_char( "&RYou already have backup coming.\n\r", ch );
            return;
         }

         if( !ch->pcdata->clan )
         {
            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
            return;
         }

         if( ch->gold < ch->skill_level[POLITICIAN_ABILITY] * 200 )
         {
            ch_printf( ch, "&RYou dont have enough credits.\n\r", ch );
            return;
         }

         schance = ( int )( ch->pcdata->learned[gsn_eliteguard] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch );
            act( AT_PLAIN, "$n begins issuing orders into $s comlink.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 1, do_elite_guard, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou call for a guard but nobody answers.\n\r", ch );
         learn_from_failure( ch, gsn_eliteguard );
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
         send_to_char( "&RYou are interupted before you can finish your call.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   send_to_char( "&GYour guard is on the way.\n\r", ch );

   credits = ch->skill_level[POLITICIAN_ABILITY] * 200;
   ch_printf( ch, "It cost you %d credits.\n\r", credits );
   ch->gold -= UMIN( credits, ch->gold );

   learn_from_success( ch, gsn_eliteguard );

   if( nifty_is_name( "empire", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_IMP_ELITE;
   else if( nifty_is_name( "republic", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_NR_ELITE;
   else
      ch->backup_mob = MOB_VNUM_MERC_ELITE;

   ch->backup_wait = 1;

}

void do_add_patrol( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance, credits;

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( ch->backup_wait )
         {
            send_to_char( "&RYou already have backup coming.\n\r", ch );
            return;
         }

         if( !ch->pcdata->clan )
         {
            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
            return;
         }

         if( ch->gold < ch->skill_level[POLITICIAN_ABILITY] * 30 )
         {
            ch_printf( ch, "&RYou dont have enough credits.\n\r", ch );
            return;
         }

         schance = ( int )( ch->pcdata->learned[gsn_addpatrol] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch );
            act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 1, do_add_patrol, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou call for a guard but nobody answers.\n\r", ch );
         learn_from_failure( ch, gsn_addpatrol );
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
         send_to_char( "&RYou are interupted before you can finish your call.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   send_to_char( "&GYour guard is on the way.\n\r", ch );

   credits = ch->skill_level[POLITICIAN_ABILITY] * 30;
   ch_printf( ch, "It cost you %d credits.\n\r", credits );
   ch->gold -= UMIN( credits, ch->gold );

   learn_from_success( ch, gsn_addpatrol );

   if( nifty_is_name( "empire", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_IMP_PATROL;
   else if( nifty_is_name( "republic", ch->pcdata->clan->name ) )
      ch->backup_mob = MOB_VNUM_NR_PATROL;
   else
      ch->backup_mob = MOB_VNUM_MERC_PATROL;

   ch->backup_wait = 1;

}

void do_jail( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim = NULL;
   CLAN_DATA *clan = NULL;
   ROOM_INDEX_DATA *jail = NULL;

   if( IS_NPC( ch ) )
      return;

   if( !ch->pcdata || ( clan = ch->pcdata->clan ) == NULL )
   {
      send_to_char( "Only members of organizations can jail their enemies.\n\r", ch );
      return;
   }

   jail = get_room_index( clan->jail );
   if( !jail && clan->mainclan )
      jail = get_room_index( clan->mainclan->jail );

   if( !jail )
   {
      send_to_char( "Your orginization does not have a suitable prison.\n\r", ch );
      return;
   }

   if( ch->mount )
   {
      send_to_char( "You can't do that while mounted.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Jail who?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "That's pointless.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "That would be a waste of time.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "This isn't a good place to do that.\n\r", ch );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "Interesting combat technique.\n\r", ch );
      return;
   }

   if( ch->position <= POS_SLEEPING )
   {
      send_to_char( "In your dreams or what?\n\r", ch );
      return;
   }

   if( victim->position >= POS_SLEEPING )
   {
      send_to_char( "You will have to stun them first.\n\r", ch );
      return;
   }

   send_to_char( "You have them escorted off to jail.\n\r", ch );
   act( AT_ACTION, "You have a strange feeling that you've been moved.\n\r", ch, NULL, victim, TO_VICT );
   act( AT_ACTION, "$n has $N escorted away.\n\r", ch, NULL, victim, TO_NOTVICT );

   char_from_room( victim );
   char_to_room( victim, jail );

   act( AT_ACTION, "The door opens briefly as $n is shoved into the room.\n\r", victim, NULL, NULL, TO_ROOM );

   learn_from_success( ch, gsn_jail );

   return;
}

void do_smalltalk( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *victim = NULL;
   PLANET_DATA *planet = NULL;
   CLAN_DATA *clan = NULL;
   int percent;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "What would be the point of that.\n\r", ch );
   }

   argument = one_argument( argument, arg1 );

   if( ch->mount )
   {
      send_to_char( "You can't do that while mounted.\n\r", ch );
      return;
   }

   if( arg1[0] == '\0' )
   {
      send_to_char( "Create smalltalk with whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "That's pointless.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "This isn't a good place to do that.\n\r", ch );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "Interesting combat technique.\n\r", ch );
      return;
   }

   if( victim->position == POS_FIGHTING )
   {
      send_to_char( "They're a little busy right now.\n\r", ch );
      return;
   }


   if( !IS_NPC( victim ) || victim->vip_flags == 0 )
   {
      send_to_char( "Diplomacy would be wasted on them.\n\r", ch );
      return;
   }

   if( ch->position <= POS_SLEEPING )
   {
      send_to_char( "In your dreams or what?\n\r", ch );
      return;
   }

   if( victim->position <= POS_SLEEPING )
   {
      send_to_char( "You might want to wake them first...\n\r", ch );
      return;
   }

   percent = number_percent(  );

   WAIT_STATE( ch, skill_table[gsn_smalltalk]->beats );

   if( percent - ch->skill_level[POLITICIAN_ABILITY] + victim->top_level > ch->pcdata->learned[gsn_smalltalk] )
   {
      /*
       * Failure.
       */
      send_to_char( "You attempt to make smalltalk with them.. but are ignored.\n\r", ch );
      act( AT_ACTION, "$n is really getting on your nerves with all this chatter!\n\r", ch, NULL, victim, TO_VICT );
      act( AT_ACTION, "$n asks $N about the weather but is ignored.\n\r", ch, NULL, victim, TO_NOTVICT );

      if( victim->alignment < -500 && victim->top_level >= ch->top_level + 5 )
      {
         sprintf( buf, "SHUT UP %s!", ch->name );
         do_yell( victim, buf );
         global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
      }

      return;
   }

   send_to_char( "You strike up a short conversation with them.\n\r", ch );
   act( AT_ACTION, "$n smiles at you and says, 'hello'.\n\r", ch, NULL, victim, TO_VICT );
   act( AT_ACTION, "$n chats briefly with $N.\n\r", ch, NULL, victim, TO_NOTVICT );

   if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
      return;

   if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
      clan = ch->pcdata->clan;

   planet = ch->in_room->area->planet;

   if( clan != planet->governed_by )
      return;

   planet->pop_support += 0.2;
   send_to_char( "Popular support for your organization increases slightly.\n\r", ch );

   gain_exp( ch, victim->top_level * 10, POLITICIAN_ABILITY );
   ch_printf( ch, "You gain %d diplomacy experience.\n\r", victim->top_level * 10 );

   learn_from_success( ch, gsn_smalltalk );

   if( planet->pop_support > 100 )
      planet->pop_support = 100;
}

void do_propeganda( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   PLANET_DATA *planet;
   CLAN_DATA *clan;
   int percent;

   if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
   {
      send_to_char( "What would be the point of that.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg1 );

   if( ch->mount )
   {
      send_to_char( "You can't do that while mounted.\n\r", ch );
      return;
   }

   if( arg1[0] == '\0' )
   {
      send_to_char( "Spread propeganda to who?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "That's pointless.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "This isn't a good place to do that.\n\r", ch );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "Interesting combat technique.\n\r", ch );
      return;
   }

   if( victim->position == POS_FIGHTING )
   {
      send_to_char( "They're a little busy right now.\n\r", ch );
      return;
   }


   if( victim->vip_flags == 0 )
   {
      send_to_char( "Diplomacy would be wasted on them.\n\r", ch );
      return;
   }

   if( ch->position <= POS_SLEEPING )
   {
      send_to_char( "In your dreams or what?\n\r", ch );
      return;
   }

   if( victim->position <= POS_SLEEPING )
   {
      send_to_char( "You might want to wake them first...\n\r", ch );
      return;
   }

   if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
      clan = ch->pcdata->clan;

   planet = ch->in_room->area->planet;

   sprintf( buf, ", and the evils of %s", planet->governed_by ? planet->governed_by->name : "their current leaders" );
   ch_printf( ch, "You speak to them about the benifits of the %s%s.\n\r", ch->pcdata->clan->name,
              planet->governed_by == clan ? "" : buf );
   act( AT_ACTION, "$n speaks about his organization.\n\r", ch, NULL, victim, TO_VICT );
   act( AT_ACTION, "$n tells $N about their organization.\n\r", ch, NULL, victim, TO_NOTVICT );

   WAIT_STATE( ch, skill_table[gsn_propeganda]->beats );

   percent = number_percent(  );

   if( percent - get_curr_cha( ch ) + victim->top_level > ch->pcdata->learned[gsn_propeganda] )
   {

      if( planet->governed_by != clan )
      {
         sprintf( buf, "%s is a traitor!", ch->name );
         do_yell( victim, buf );
         global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
      }

      return;
   }

   if( planet->governed_by == clan )
   {
      planet->pop_support += .5 + ch->top_level / 15;
      send_to_char( "Popular support for your organization increases.\n\r", ch );
   }
   else
   {
      planet->pop_support -= .5 + ch->top_level / 15;
      send_to_char( "Popular support for the current government decreases.\n\r", ch );
   }

   gain_exp( ch, victim->top_level * 100, POLITICIAN_ABILITY );
   ch_printf( ch, "You gain %d diplomacy experience.\n\r", victim->top_level * 100 );

   learn_from_success( ch, gsn_propeganda );

   if( planet->pop_support > 100 )
      planet->pop_support = 100;
   if( planet->pop_support < -100 )
      planet->pop_support = -100;
   save_planet( planet );
}

void do_bribe( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   PLANET_DATA *planet;
   CLAN_DATA *clan;
   int percent, amount;

   if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
   {
      send_to_char( "What would be the point of that.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg1 );

   if( ch->mount )
   {
      send_to_char( "You can't do that while mounted.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Bribe who how much?\n\r", ch );
      return;
   }

   amount = atoi( argument );

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "That's pointless.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "This isn't a good place to do that.\n\r", ch );
      return;
   }

   if( amount > ch->gold )
   {
      send_to_char( "Perhaps if you had that many credits...\n\r", ch );
      return;
   }

   if( amount <= 0 )
   {
      send_to_char( "A little bit more money would be a good plan.\n\r", ch );
      return;
   }

   if( amount > 10000 )
   {
      send_to_char( "You cannot bribe for more than 10000 credits at a time.\n\r", ch );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "Interesting combat technique.\n\r", ch );
      return;
   }

   if( victim->position == POS_FIGHTING )
   {
      send_to_char( "They're a little busy right now.\n\r", ch );
      return;
   }

   if( ch->position <= POS_SLEEPING )
   {
      send_to_char( "In your dreams or what?\n\r", ch );
      return;
   }

   if( victim->position <= POS_SLEEPING )
   {
      send_to_char( "You might want to wake them first...\n\r", ch );
      return;
   }

   if( victim->vip_flags == 0 )
   {
      send_to_char( "Diplomacy would be wasted on them.\n\r", ch );
      return;
   }

   ch->gold -= amount;
   victim->gold += amount;

   ch_printf( ch, "You give them a small gift on behalf of %s.\n\r", ch->pcdata->clan->name );
   act( AT_ACTION, "$n offers you a small bribe.\n\r", ch, NULL, victim, TO_VICT );
   act( AT_ACTION, "$n gives $N some money.\n\r", ch, NULL, victim, TO_NOTVICT );

   if( !IS_NPC( victim ) )
      return;

   WAIT_STATE( ch, skill_table[gsn_bribe]->beats );

   if( percent - amount + victim->top_level > ch->pcdata->learned[gsn_bribe] )
      return;

   if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
      clan = ch->pcdata->clan;

   planet = ch->in_room->area->planet;


   if( clan == planet->governed_by )
   {
      planet->pop_support += URANGE( 0.1, amount / 1000, 2 );
      send_to_char( "Popular support for your organization increases slightly.\n\r", ch );

      amount =
         UMIN( amount,
               ( exp_level( ch->skill_level[POLITICIAN_ABILITY] + 1 ) - exp_level( ch->skill_level[POLITICIAN_ABILITY] ) ) );

      gain_exp( ch, amount, POLITICIAN_ABILITY );
      ch_printf( ch, "You gain %d diplomacy experience.\n\r", amount );

      learn_from_success( ch, gsn_bribe );
   }

   if( planet->pop_support > 100 )
      planet->pop_support = 100;
}

void do_seduce( CHAR_DATA * ch, char *argument )
{
   AFFECT_DATA af;
   int schance;
   int level;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   CHAR_DATA *rch;

   if( argument[0] == '\0' )
   {
      send_to_char( "Seduce who?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You like yourself even better!\n\r", ch );
      return;
   }

   if( IS_SET( victim->immune, RIS_CHARM ) )
   {
      send_to_char( "They seem to be immune to such acts.\n\r", ch );
      sprintf( buf, "%s is trying to seduce you but just looks sleazy.\n\r", ch->name );
      act( AT_MAGIC, buf, ch, NULL, victim, TO_ROOM );
      return;
   }

   if( find_keeper( victim ) != NULL )
   {
      send_to_char( "They have been trained against such things!\n\r", ch );
      return;
   }

   if( !IS_NPC( victim ) && !IS_NPC( ch ) )
   {
      send_to_char( "I don't think so...\n\r", ch );
      sprintf( buf, "%s is trying to seduce you but just looks sleazy.\n\r", ch->name );
      act( AT_MAGIC, buf, ch, NULL, victim, TO_ROOM );
      return;
   }

   level = !IS_NPC( ch ) ? ( int )ch->pcdata->learned[gsn_seduce] : ch->top_level;
   schance = ris_save( victim, level, RIS_CHARM );

   if( IS_AFFECTED( victim, AFF_CHARM ) || schance == 1000 || IS_AFFECTED( ch, AFF_CHARM )
       || ch->top_level < victim->top_level || circle_follow( victim, ch ) || saves_spell_staff( schance, victim )
       || ch->sex == victim->sex )
   {
      send_to_char( "&w&BYou failed.\n\r", ch );
      sprintf( buf, "%s is trying to seduce you but just looks sleazy.\n\r", ch->name );
      act( AT_MAGIC, buf, ch, NULL, victim, TO_ROOM );
      learn_from_failure( ch, gsn_seduce );
      return;
   }

   if( victim->master )
      stop_follower( victim );
   add_follower( victim, ch );
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( rch->master == ch && IS_AFFECTED( rch, AFF_CHARM ) && rch != victim )
      {
         send_to_char( "You snap out of it.\n\r", rch );
         ch_printf( ch, "&B%s becomes less dazed.\n\r", PERS( rch, ch ) );
         stop_follower( rch );
      }
   }
   af.type = gsn_seduce;
   af.duration = ( number_fuzzy( ( level + 1 ) / 3 ) + 1 ) * DUR_CONV;
   af.location = 0;
   af.modifier = 0;
   af.bitvector = AFF_CHARM;
   affect_to_char( victim, &af );
   act( AT_MAGIC, "$n just seems so attractive...", ch, NULL, victim, TO_VICT );
   act( AT_MAGIC, "$N's eyes glaze over...", ch, NULL, victim, TO_ROOM );
   if( ch != victim )
      send_to_char( "Ok.\n\r", ch );

   learn_from_success( ch, gsn_seduce );
   sprintf( buf, "%s has seduced %s.", ch->name, victim->name );
   log_string_plus( buf, LOG_NORMAL, ch->top_level );

   return;

}

void do_mass_propeganda( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *rch;
   PLANET_DATA *planet;
   CLAN_DATA *clan;
   int victims = 0;

   if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
   {
      send_to_char( "What would be the point of that.\n\r", ch );
      return;
   }

   if( ch->mount )
   {
      send_to_char( "You can't do that while mounted.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "This isn't a good place to do that.\n\r", ch );
      return;
   }

   if( ch->position == POS_FIGHTING )
   {
      send_to_char( "Interesting combat technique.\n\r", ch );
      return;
   }

   if( ch->position <= POS_SLEEPING )
   {
      send_to_char( "In your dreams or what?\n\r", ch );
      return;
   }

   if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
      clan = ch->pcdata->clan;

   planet = ch->in_room->area->planet;

   sprintf( buf, ", and the evils of %s", planet->governed_by ? planet->governed_by->name : "their current leaders" );
   ch_printf( ch, "You speak to the people about the benefits of the %s%s.\n\r", ch->pcdata->clan->name,
              planet->governed_by == clan ? "" : buf );
   act( AT_ACTION, "$n speaks about their organization.\n\r", ch, NULL, NULL, TO_ROOM );

   WAIT_STATE( ch, skill_table[gsn_masspropeganda]->beats );

   if( number_percent(  ) < ch->pcdata->learned[gsn_masspropeganda] )
   {
      for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      {
         if( rch == ch )
            continue;

         if( !IS_NPC( rch ) )
            continue;

         if( rch->vip_flags == 0 )
            continue;

         if( can_see( ch, rch ) )
            victims++;
         else
            continue;
      }

      if( planet->governed_by == clan )
      {
         planet->pop_support += ( .5 + ch->top_level / 10 ) * victims;
         send_to_char( "Popular support for your organization increases.\n\r", ch );
      }
      else
      {
         planet->pop_support -= ( ch->top_level / 10 ) * victims;
         send_to_char( "Popular support for the current government decreases.\n\r", ch );
      }

      gain_exp( ch, ch->top_level * 100, POLITICIAN_ABILITY );
      ch_printf( ch, "You gain %d diplomacy experience.\n\r", ch->top_level * 100 );

      learn_from_success( ch, gsn_masspropeganda );

      if( planet->pop_support > 100 )
         planet->pop_support = 100;
      if( planet->pop_support < -100 )
         planet->pop_support = -100;
      save_planet( planet );
      return;
   }
   else
   {
      send_to_char( "They don't even seem interested in what you have to say.\n\r", ch );
      return;
   }
   return;
}

void do_gather_intelligence( CHAR_DATA * ch, char *argument )
{
}

void do_repair( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj, *cobj;
   char arg[MAX_STRING_LENGTH];
   int schance;
   bool checktool, checksew;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "Repair what?\n\r", ch );
            return;
         }

         if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
         {
            send_to_char( "&RYou do not have that item.\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checksew = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
         {
            send_to_char( "&RYou can only repair weapons and armor.&w\n\r", ch );
            return;
         }

         if( obj->item_type == ITEM_WEAPON && obj->value[0] == INIT_WEAPON_CONDITION )
         {
            send_to_char( "&WIt does not appear to be in need of repair.\n\r", ch );
            return;
         }
         else if( obj->item_type == ITEM_ARMOR && obj->value[0] == obj->value[1] )
         {
            send_to_char( "&WIt does not appear to be in need of repair.\n\r", ch );
            return;
         }

         for( cobj = ch->last_carrying; cobj; cobj = cobj->prev_content )
         {
            if( cobj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( cobj->item_type == ITEM_THREAD )
               checksew = TRUE;
         }

         if( !checktool && obj->item_type == ITEM_WEAPON )
         {
            send_to_char( "&w&RYou need a toolkit to repair weapons.\n\r", ch );
            return;
         }

         if( !checksew && obj->item_type == ITEM_ARMOR )
         {
            send_to_char( "&w&RYou need a needle and thread to repair armor.\n\r", ch );
            return;
         }

         send_to_char( "&W&GYou begin to repair your equipment...&W\n\r", ch );
         act( AT_PLAIN, "$n takes $s tools and begins to repair something.", ch, NULL, argument, TO_ROOM );
         add_timer( ch, TIMER_DO_FUN, 5, do_repair, 1 );
         ch->dest_buf = str_dup( arg );
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
         send_to_char( "&RYou are interrupted and fail to finish your work.\n\r", ch );
         return;

   }

   ch->substate = SUB_NONE;

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_repair] );

   if( number_percent(  ) > schance * 2 )
   {
      send_to_char( "&RYou realize your attempts to repair your equipment have had no effect.\n\r", ch );
      learn_from_failure( ch, gsn_repair );
      return;
   }

   if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "&RError S3. Report to Administration\n\r&w", ch );
      return;
   }

   switch ( obj->item_type )
   {
      default:
         send_to_char( "Error S4. Contact Administration.\n\r", ch );
         return;
      case ITEM_ARMOR:
         obj->value[0] = obj->value[1];
         break;
      case ITEM_WEAPON:
         obj->value[0] = INIT_WEAPON_CONDITION;
         break;
      case ITEM_DEVICE:
         obj->value[2] = obj->value[1];
         break;
   }

   send_to_char( "&GYou repair your equipment back to fine condition.&W\n\r", ch );

   {
      long xpgain;

      xpgain = ( number_percent(  ) * 6 ) * ch->skill_level[ENGINEERING_ABILITY];
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.\n\r", xpgain );
   }

   learn_from_success( ch, gsn_repair );
}

void do_makeduallightsaber( CHAR_DATA * ch, char *argument )
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

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: makeduallightsaber <name>\n\r&w", ch );
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

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeduallightsaber] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of crafting a lightsaber.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, do_makeduallightsaber, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makeduallightsaber );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeduallightsaber] );
   vnum = 72;

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

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeduallightsaber] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )
       || ( !checkmirr ) || ( !checklens ) || ( !checkgems ) || ( !checkcond ) || ( !checkcirc ) )

   {
      send_to_char( "&RYou hold up your new lightsaber and press the switch hoping for the best.\n\r", ch );
      send_to_char( "&RInstead of a blade of light, smoke starts pouring from the handle.\n\r", ch );
      send_to_char( "&RYou drop the hot handle and watch as it melts on away on the floor.\n\r", ch );
      learn_from_failure( ch, gsn_makeduallightsaber );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_WEAPON;
   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   SET_BIT( obj->extra_flags, ITEM_ANTI_SOLDIER );
   SET_BIT( obj->extra_flags, ITEM_ANTI_THIEF );
   SET_BIT( obj->extra_flags, ITEM_ANTI_HUNTER );
   SET_BIT( obj->extra_flags, ITEM_ANTI_PILOT );
   SET_BIT( obj->extra_flags, ITEM_ANTI_CITIZEN );
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
   paf->modifier = URANGE( 0, gems, level / 8 );
   paf->bitvector = 0;
   paf->next = NULL;
   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
   CREATE( paf2, AFFECT_DATA, 1 );
   paf2->type = -1;
   paf2->duration = -1;
   paf2->location = get_atype( "parry" );
   paf2->modifier = ( 100 );
   paf2->bitvector = 0;
   paf2->next = NULL;
   LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
   obj->value[0] = INIT_WEAPON_CONDITION; /* condition  */
   obj->value[1] = ( int )( level / 10 + gemtype * 2 );  /* min dmg  */
   obj->value[2] = ( int )( level / 5 + gemtype * 6 );   /* max dmg */
   obj->value[3] = WEAPON_DUAL_LIGHTSABER;
   obj->value[4] = charge;
   obj->value[5] = charge;
   obj->cost = obj->value[2] * 75;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created lightsaber.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new lightsaber.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[FORCE_ABILITY] + 1 ) - exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, FORCE_ABILITY );
      ch_printf( ch, "You gain %d force experience.", xpgain );
   }
   learn_from_success( ch, gsn_makeduallightsaber );
}

void do_makepike( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance, charge;
   bool checktool, checkdura, checkbatt, checkoven;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;
   AFFECT_DATA *paf;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makepike <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbatt = FALSE;
         checkoven = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DURASTEEL )
               checkdura = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;

            if( obj->item_type == ITEM_OVEN )
               checkoven = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a force pike.\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "&RYou need something to make it out of.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a power source for your pike.\n\r", ch );
            return;
         }

         if( !checkoven )
         {
            send_to_char( "&RYou need a small furnace to heat the metal.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makepike] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of crafting a force pike.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 30, do_makepike, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makepike );
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
         send_to_char( "&RYou are interrupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makepike] );
   vnum = 74;

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

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_OVEN )
         checkoven = TRUE;
      if( obj->item_type == ITEM_DURASTEEL && checkdura == FALSE )
      {
         checkdura = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         charge = UMAX( 5, obj->value[0] );
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makepike] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven ) )
   {
      send_to_char( "&RYou activate your newly created force pike.\n\r", ch );
      send_to_char( "&RIt hums softly for a few seconds then begins to shake violently.\n\r", ch );
      send_to_char( "&RIt finally shatters breaking apart into a dozen pieces.\n\r", ch );
      learn_from_failure( ch, gsn_makepike );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_WEAPON;
   SET_BIT( obj->wear_flags, ITEM_WIELD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 3;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " force pike" );
   strcat( buf, remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was left here." );
   CREATE( paf, AFFECT_DATA, 1 );
   paf->type = -1;
   paf->duration = -1;
   paf->location = get_atype( "parry" );
   paf->modifier = level / 3;
   paf->bitvector = 0;
   paf->next = NULL;
   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   ++top_affect;
   obj->description = STRALLOC( buf );
   obj->value[0] = INIT_WEAPON_CONDITION;
   obj->value[1] = ( int )( level / 10 + 10 );  /* min dmg  */
   obj->value[2] = ( int )( level / 2 + 20 );   /* max dmg */
   obj->value[3] = WEAPON_FORCE_PIKE;
   obj->value[4] = charge;
   obj->value[5] = charge;
   obj->cost = obj->value[2] * 10;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created force pike.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes crafting a force pike.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 200,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }

   learn_from_success( ch, gsn_makepike );
}

void do_makebug( CHAR_DATA * ch, char *argument )
{
   int level, schance;
   bool checktool, checkbatt, checkcirc;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;

   switch ( ch->substate )
   {
      default:
         checktool = FALSE;
         checkbatt = FALSE;
         checkcirc = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a bug.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a small battery to power the bug.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou're going to need some circuitry.\n\r", ch );
            return;
         }


         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebug] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the process of making a bug.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 15, do_makebug, 1 );
            ch->dest_buf = str_dup( "blah" );
            return;
         }
         send_to_char( "&RYou're not quite sure how to do it...\n\r", ch );
         learn_from_failure( ch, gsn_makebug );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         ch->dest_buf = str_dup( "blah" );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interrupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebug] );
   vnum = 77;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkbatt = FALSE;
   checkcirc = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;

      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }

      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcirc = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebug] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkbatt ) || ( !checkcirc ) )
   {
      send_to_char( "&RYou finish and activate the bug, but sadly, it melts into nothing.\n\r", ch );
      learn_from_failure( ch, gsn_makebug );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_BUG;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 1;
   STRFREE( obj->name );
   obj->name = STRALLOC( "device bug" );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( "a small bug" );
   STRFREE( obj->description );
   obj->description = STRALLOC( "A small electronic device was dropped here." );

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish and activate the bug. It works beautifully.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making a bug.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain = number_range( 1550, 2100 );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makebug );
}

void do_makebeacon( CHAR_DATA * ch, char *argument )
{
   int level, schance;
   bool checktool, checkbatt, checkcirc;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;

   switch ( ch->substate )
   {
      default:
         checktool = FALSE;
         checkbatt = FALSE;
         checkcirc = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a beacon.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a small battery to power the beacon.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou're going to need some circuitry.\n\r", ch );
            return;
         }


         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebeacon] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the process of making a beacon.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 15, do_makebeacon, 1 );
            ch->dest_buf = str_dup( "blah" );
            return;
         }
         send_to_char( "&RYou're not quite sure how to do it...\n\r", ch );
         learn_from_failure( ch, gsn_makebeacon );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         ch->dest_buf = str_dup( "blah" );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interrupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebeacon] );
   vnum = 78;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkbatt = FALSE;
   checkcirc = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;

      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;
      }

      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcirc = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebeacon] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkbatt ) || ( !checkcirc ) )
   {
      send_to_char( "&RYou finish and activate the beacon, but sadly, it melts into nothing.\n\r", ch );
      learn_from_failure( ch, gsn_makebeacon );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_BEACON;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 1;
   STRFREE( obj->name );
   obj->name = STRALLOC( "device beacon" );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( "a locator beacon" );
   STRFREE( obj->description );
   obj->description = STRALLOC( "A small electronic device was dropped here." );

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish and activate the beacon. It works beautifully.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making a beacon.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain = number_range( 2500, 3750 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, "You gain %d technician experience.", xpgain );
   }
   learn_from_success( ch, gsn_makebeacon );
}

void do_plantbeacon( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   OBJ_DATA *obj;
   bool checkbeacon = FALSE;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;
   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: plantbeacon <ship>\n\r", ch );
      return;
   }

   if( ( ship = ship_in_room( ch->in_room, argument ) ) == NULL )
   {
      send_to_char( "That ship isn't here.\n\r", ch );
      return;
   }
   if( is_name( ch->name, ship->pbeacon ) )
   {
      send_to_char( "You have already planted a beacon on that ship.\n\r", ch );
      return;
   }

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->item_type == ITEM_BEACON )
         checkbeacon = TRUE;
   if( checkbeacon == FALSE )
   {
      send_to_char( "You don't have any beacons to plant.\n\r", ch );
      return;
   }

   if( number_percent(  ) < ch->pcdata->learned[gsn_plantbeacon] )
   {
      sprintf( buf, "You place a locating beacon on the hull of %s.\n\r", ship->name );
      send_to_char( buf, ch );
      sprintf( buf, "%s places a device on the hull of %s.", ch->name, ship->name );
      act( AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM );
      sprintf( buf, "%s %s", ship->pbeacon, ch->name );
      STRFREE( ship->pbeacon );
      ship->pbeacon = STRALLOC( buf );
      save_ship( ship );

      for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      {
         if( obj->item_type == ITEM_BEACON )
         {
            separate_obj( obj );
            obj_from_char( obj );
            extract_obj( obj );
            break;
         }
      }

      learn_from_success( ch, gsn_plantbeacon );
      return;
   }
   else
   {
      send_to_char( "&RYou're not quite sure where to place the beacon.\n\r", ch );
      learn_from_failure( ch, gsn_plantbeacon );
      return;
   }
}

void do_showbeacons( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   SHIP_DATA *ship2;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH];
   int count = 0;

   if( IS_NPC( ch ) )
      return;
   if( number_percent(  ) > ch->pcdata->learned[gsn_showbeacons] )
   {
      send_to_char( "You can't figure out what to do.\n\r", ch );
      learn_from_failure( ch, gsn_showbeacons );
      return;
   }

   send_to_char( "\n\r", ch );
   send_to_char( "&zRequesting response from active locator beacons...\n\r", ch );
   send_to_char( "&w__________________________________________________\n\r\n\r", ch );

   for( ship = first_ship; ship; ship = ship->next )
   {
      if( is_name( ch->name, ship->pbeacon ) )
      {
         if( !ship->in_room && ship->shipstate != SHIP_HYPERSPACE )
            sprintf( buf3, "&w%.0f&z, &w%.0f&z, &w%.0f", ship->vx, ship->vy, ship->vz );
         if( ship->shipstate == SHIP_HYPERSPACE )
            sprintf( buf3, "In Hyperspace" );

         for( ship2 = first_ship; ship2; ship2 = ship2->next )
         {
            if( ship->in_room )
            {
               if( ship->in_room->vnum == ship2->hanger1 )
               {
                  sprintf( buf, "%s", ship2->name );
                  sprintf( buf2, "%s", ship->in_room->name );
                  break;
               }
               if( ship->in_room->vnum == ship2->hanger2 )
               {
                  sprintf( buf, "%s", ship2->name );
                  sprintf( buf2, "%s", ship->in_room->name );
                  break;
               }
               if( ship->in_room->vnum == ship2->hanger3 )
               {
                  sprintf( buf, "%s", ship2->name );
                  sprintf( buf2, "%s", ship->in_room->name );
                  break;
               }
               if( ship->in_room->vnum == ship2->hanger4 )
               {
                  sprintf( buf, "%s", ship2->name );
                  sprintf( buf2, "%s", ship->in_room->name );
                  break;
               }
            }
         }

         ch_printf( ch, "^g&xACTIVE^x&z: &w%-15.15s&z Location:&w %s &z(&w%s&z)&w\n\r", ship->name,
                    ( ship->in_room && ship->in_room->area->planet ) ?
                    ship->in_room->area->planet->name :
                    ( ship->in_room && !ship->in_room->area->planet ) ?
                    buf :
                    ( !ship->in_room && ship->starsystem ) ? ship->starsystem->name :
                    ship->shipstate == SHIP_HYPERSPACE ? "Unknown" : "Unknown",
                    ( ship->in_room && ship->in_room->area->planet ) ?
                    ship->in_room->name :
                    ( ship->in_room && !ship->in_room->area->planet ) ?
                    buf2 : ship->shipstate == SHIP_HYPERSPACE ? "In Hyperspace" : ( !ship->in_room ) ? buf3 : "Unknown" );
         count++;
      }
   }
   if( count == 0 )
      send_to_char( "            &zNo active beacons found.\n\r", ch );
   learn_from_success( ch, gsn_showbeacons );
}

void do_checkbeacons( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char arg[MAX_STRING_LENGTH];
   int schance;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg );

   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "Usage: checkbeacons <ship>\n\r", ch );
            return;
         }

         if( ( ship = ship_in_room( ch->in_room, arg ) ) == NULL )
         {
            send_to_char( "That ship isn't here.\n\r", ch );
            return;
         }
         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_checkbeacons] );

         if( number_percent(  ) - 20 < schance )
         {
            send_to_char( "&w&GYou take a scanner and begin to check over the ship.\n\r", ch );
            act( AT_PLAIN, "$n punches a few instructions into a scanner.", ch, NULL, NULL, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 6, do_checkbeacons, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "You punch random buttons on the scanner, unsure of what you are doing.\n\r", ch );
         learn_from_failure( ch, gsn_checkbeacons );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         ch->dest_buf = NULL;
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->dest_buf = NULL;
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interrupted and fail to finish scanning.\n\r", ch );
         return;

   }

   ch->substate = SUB_NONE;

   if( ( ship = ship_in_room( ch->in_room, arg ) ) == NULL )
   {
      send_to_char( "The ship left before you could complete the scan.\n\r", ch );
      return;
   }

   if( ship->pbeacon[0] == '\0' )
      send_to_char( "&w&GScan Complete. No unknown broadcast signals detected.\n\r", ch );
   else
      send_to_char( "&w&GScan Complete. You've detected a strange signal being broadcast...\n\r", ch );

   learn_from_success( ch, gsn_checkbeacons );
   {
      long xpgain;

      xpgain = ( ch->experience[TECHNICIAN_ABILITY] / 30 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, "You gain %d technician experience.", xpgain );
   }

   return;
}

void do_nullifybeacons( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char arg[MAX_STRING_LENGTH];
   int schance;

   if( IS_NPC( ch ) )
      return;
   argument = one_argument( argument, arg );

   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "Syntax: nullifybeacons <ship>\n\r", ch );
            return;
         }

         if( ( ship = ship_in_room( ch->in_room, arg ) ) == NULL )
         {
            send_to_char( "That ship isn't here.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_nullifybeacons] );

         if( number_percent(  ) < schance + 20 )
         {
            send_to_char( "&w&GYou place a small device on the ship, and input a few commands.\n\r", ch );
            act( AT_PLAIN, "$n places a device on a ship, and inputs a few commands.", ch, NULL, NULL, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 1, do_nullifybeacons, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "You look over the ship, trying to find the correct spot to place the nullifier.\n\r", ch );
         learn_from_failure( ch, gsn_nullifybeacons );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         strcpy( arg, ch->dest_buf );
         DISPOSE( ch->dest_buf );
         ch->dest_buf = NULL;
         break;
   }

   ch->substate = SUB_NONE;

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_nullifybeacons] );

   if( ( ship = ship_in_room( ch->in_room, arg ) ) == NULL )
   {
      send_to_char( "The ship left before the nullifier could work.\n\r", ch );
      return;
   }

   STRFREE( ship->pbeacon );
   ship->pbeacon = STRALLOC( "" );
   save_ship( ship );
   send_to_char( "&w&GThe nullifier emits several beeps, and shuts off.\n\r", ch );
   send_to_char( "&GChecking your scanner, no foreign signals are actively broadcasting.\n\r", ch );
   send_to_char( "&wYou remove the nullifier.\n\r", ch );
   act( AT_PLAIN, "A device on a ship emits several beeps.\n\r", ch, NULL, NULL, TO_ROOM );
   act( AT_PLAIN, "$n removes the device, and checks a scanner.\n\r", ch, NULL, NULL, TO_ROOM );
   learn_from_success( ch, gsn_nullifybeacons );
   {
      long xpgain;

      xpgain = ( ch->experience[TECHNICIAN_ABILITY] / 25 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, "You gain %d technician experience.", xpgain );
   }

   return;
}

void do_makebinders( CHAR_DATA * ch, char *argument )
{
   int level, schance;
   bool checktool, checkoven, checkdura;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;

   switch ( ch->substate )
   {
      default:
         checktool = FALSE;
         checkdura = FALSE;
         checkoven = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_OVEN )
               checkoven = TRUE;
            if( obj->item_type == ITEM_DURASTEEL )
               checkdura = TRUE;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need a toolkit.\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "&RYou'll need some durasteel to mold.\n\r", ch );
            return;
         }

         if( !checkoven )
         {
            send_to_char( "&RYou're going to need an oven to heat the durasteel.\n\r", ch );
            return;
         }


         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebinders] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the process of making a pair of binders.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 15, do_makebinders, 1 );
            ch->dest_buf = str_dup( "blah" );
            return;
         }
         send_to_char( "&RYou're not quite sure how to do it...\n\r", ch );
         learn_from_failure( ch, gsn_makebinders );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         ch->dest_buf = str_dup( "blah" );
         DISPOSE( ch->dest_buf );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         ch->substate = SUB_NONE;
         send_to_char( "&RYou are interrupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebinders] );
   vnum = 79;

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checktool = FALSE;
   checkoven = FALSE;
   checkdura = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_OVEN )
         checkoven = TRUE;
      if( obj->item_type == ITEM_DURASTEEL && checkdura == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkdura = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makebinders] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkoven ) )
   {
      send_to_char( "&RYou finish making your binders, but the locking mechanism doesn't work.\n\r", ch );
      learn_from_failure( ch, gsn_makebinders );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_BINDERS;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 1;
   STRFREE( obj->name );
   obj->name = STRALLOC( "binders" );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( "&Ya pair of binders&w" );
   STRFREE( obj->description );
   obj->description = STRALLOC( "A pair of binders was discarded here." );

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish constructing a pair of binders.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making a pair of binders.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain = number_range( 2500, 3750 );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makebinders );
}


void do_setinfrared( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( !IS_DROID( ch ) || IS_NPC( ch ) )
   {
      send_to_char( "You turn your 'infrared knob'.. and, well, uh..Hm.\n\r", ch );
      return;
   }

   if( arg[0] == '\0' )
   {
      send_to_char( "Setinfrared ON or OFF?\n\r", ch );
      return;
   }

   if( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
   {
      SET_BIT( ch->affected_by, AFF_INFRARED );
      send_to_char( "You activate your infrared vision unit.\n\r", ch );
      return;
   }

   if( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
   {
      REMOVE_BIT( ch->affected_by, AFF_INFRARED );
      send_to_char( "You deactivate your infrared vision unit.\n\r", ch );
      return;
   }
}


void do_battle_command( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *gch;
   AFFECT_DATA af;
   int schance;

   if( is_affected( ch, gsn_battle_command ) )
   {
      send_to_char( "&RYou've already taken command of a group.\n\r", ch );
      return;
   }

   schance = ch->pcdata->learned[gsn_battle_command];

   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou attempt to take command of the group, only to be ignored.\n\r", ch );
      learn_from_failure( ch, gsn_battle_command );
      return;
   }

   for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
   {
      if( is_same_group( gch, ch ) )
      {
         if( is_affected( gch, gsn_battle_command ) )
            continue;

         if( gch != ch )
            ch_printf( gch, "&G%s takes command of the group, you feel more confident and stronger in battle.\n\r",
                       PERS( ch, gch ) );
         else
            send_to_char( "&GYou take command of the group, you feel more confident and stronger in battle.\n\r", ch );

         af.type = gsn_battle_command;
         af.duration = ( number_fuzzy( ( ch->pcdata->learned[gsn_battle_command] + 1 ) / 3 ) + 1 ) * DUR_CONV;
         af.location = APPLY_AC;
         af.modifier = -( ch->pcdata->learned[gsn_battle_command] / 2 );
         af.bitvector = 0;
         affect_to_char( gch, &af );
         af.location = APPLY_HIT;
         af.modifier = ch->pcdata->learned[gsn_battle_command] / 2;
         affect_to_char( gch, &af );
      }
   }
   learn_from_success( ch, gsn_battle_command );

   return;
}
