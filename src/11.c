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

// 11.c. Tired of downloading huge files.

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

SHIP_DATA *first_ship;
SHIP_DATA *last_ship;

MISSILE_DATA *first_missile;
MISSILE_DATA *last_missile;

SPACE_DATA *first_starsystem;
SPACE_DATA *last_starsystem;

void explode_emissile args( ( CHAR_DATA * ch, ROOM_INDEX_DATA * proom, int mindam, int maxdam, bool incendiary ) );
extern int top_affect;
SHIP_DATA *ship_from_gunseat args( ( int vnum ) );

char *primary_beam_name( SHIP_DATA * ship )
{

   if( ship->primaryCount != 0 )
   {
      sprintf( bname, "%d %s", ship->primaryCount,
               ( ship->primaryType == SINGLE_LASER && ship->primaryCount == 1 ) ? "Single-laser cannon" :
               ( ship->primaryType == SINGLE_LASER && ship->primaryCount != 1 ) ? "Single-laser cannons" :
               ( ship->primaryType == DUAL_LASER && ship->primaryCount == 1 ) ? "Dual-laser cannon" :
               ( ship->primaryType == DUAL_LASER && ship->primaryCount != 1 ) ? "Dual-laser cannons" :
               ( ship->primaryType == TRI_LASER && ship->primaryCount == 1 ) ? "Triple-laser cannon" :
               ( ship->primaryType == TRI_LASER && ship->primaryCount != 1 ) ? "Triple-laser cannons" :
               ( ship->primaryType == QUAD_LASER && ship->primaryCount == 1 ) ? "Quad-laser cannon" :
               ( ship->primaryType == QUAD_LASER && ship->primaryCount != 1 ) ? "Quad-laser cannons" :
               ( ship->primaryType == AUTOBLASTER && ship->primaryCount == 1 ) ? "Autoblaster turret" :
               ( ship->primaryType == AUTOBLASTER && ship->primaryCount != 1 ) ? "Autoblaster turrets" :
               ( ship->primaryType == HEAVY_LASER && ship->primaryCount == 1 ) ? "Heavy laser cannon" :
               ( ship->primaryType == HEAVY_LASER && ship->primaryCount != 1 ) ? "Heavy laser cannons" :
               ( ship->primaryType == LIGHT_ION && ship->primaryCount == 1 ) ? "Light ion cannon" :
               ( ship->primaryType == LIGHT_ION && ship->primaryCount != 1 ) ? "Light ion cannons" :
               ( ship->primaryType == REPEATING_ION && ship->primaryCount == 1 ) ? "Repeating ion cannon" :
               ( ship->primaryType == REPEATING_ION && ship->primaryCount != 1 ) ? "Repeating ion cannons" :
               ( ship->primaryType == HEAVY_ION && ship->primaryCount == 1 ) ? "Heavy ion cannon" :
               ( ship->primaryType == HEAVY_ION && ship->primaryCount != 1 ) ? "Heavy ion cannons" : "unknown" );

      return bname;
   }
   else
      return "None.";
}

char *secondary_beam_name( SHIP_DATA * ship )
{

   if( ship->secondaryCount != 0 )
   {
      sprintf( bname, "%d %s", ship->secondaryCount,
               ( ship->secondaryType == SINGLE_LASER && ship->secondaryCount == 1 ) ? "Single-laser cannon" :
               ( ship->secondaryType == SINGLE_LASER && ship->secondaryCount != 1 ) ? "Single-laser cannons" :
               ( ship->secondaryType == DUAL_LASER && ship->secondaryCount == 1 ) ? "Dual-laser cannon" :
               ( ship->secondaryType == DUAL_LASER && ship->secondaryCount != 1 ) ? "Dual-laser cannons" :
               ( ship->secondaryType == TRI_LASER && ship->secondaryCount == 1 ) ? "Triple-laser cannon" :
               ( ship->secondaryType == TRI_LASER && ship->secondaryCount != 1 ) ? "Triple-laser cannons" :
               ( ship->secondaryType == QUAD_LASER && ship->secondaryCount == 1 ) ? "Quad-laser cannon" :
               ( ship->secondaryType == QUAD_LASER && ship->secondaryCount != 1 ) ? "Quad-laser cannons" :
               ( ship->secondaryType == AUTOBLASTER && ship->secondaryCount == 1 ) ? "Autoblaster turret" :
               ( ship->secondaryType == AUTOBLASTER && ship->secondaryCount != 1 ) ? "Autoblaster turrets" :
               ( ship->secondaryType == HEAVY_LASER && ship->secondaryCount == 1 ) ? "Heavy laser cannon" :
               ( ship->secondaryType == HEAVY_LASER && ship->secondaryCount != 1 ) ? "Heavy laser cannons" :
               ( ship->secondaryType == LIGHT_ION && ship->secondaryCount == 1 ) ? "Light ion cannon" :
               ( ship->secondaryType == LIGHT_ION && ship->secondaryCount != 1 ) ? "Light ion cannons" :
               ( ship->secondaryType == REPEATING_ION && ship->secondaryCount == 1 ) ? "Repeating ion cannon" :
               ( ship->secondaryType == REPEATING_ION && ship->secondaryCount != 1 ) ? "Repeating ion cannons" :
               ( ship->secondaryType == HEAVY_ION && ship->secondaryCount == 1 ) ? "Heavy ion cannon" :
               ( ship->secondaryType == HEAVY_ION && ship->secondaryCount != 1 ) ? "Heavy ion cannons" : "unknown" );

      return bname;
   }
   else
      return "None.";
}


void explode_emissile( CHAR_DATA * ch, ROOM_INDEX_DATA * proom, int mindam, int maxdam, bool incendiary )
{
   CHAR_DATA *rch;
   OBJ_INDEX_DATA *objindex;
   OBJ_INDEX_DATA *fObjIndex;
   OBJ_DATA *obj;
   OBJ_DATA *fobj;
   int dam;

// Make scraps of missile
   objindex = get_obj_index( 80 );
   obj = create_object( objindex, ch->top_level );
   obj->item_type = ITEM_TRASH;
   STRFREE( obj->name );
   STRFREE( obj->short_descr );
   STRFREE( obj->description );
   obj->name = STRALLOC( "scraps missile" );
   obj->short_descr = STRALLOC( "some scraps of a missile" );
   obj->description = STRALLOC( "The scraps of a missile are littering the area." );
   obj->timer = number_range( 5, 15 );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj_to_room( obj, proom );
// Damage
   for( rch = proom->first_person; rch; rch = rch->next_in_room )
   {
      if( incendiary )
         ch_printf( rch, "&RThe missile EXPLODES into a rain of fire!\n\r" );
      else
         ch_printf( rch, "&RThe massive shockwave from the EXPLOSION rips through your body!&w\n\r" );

      dam = number_range( mindam, maxdam );
      damage( ch, rch, dam, TYPE_MISSILE );
   }
   if( incendiary )
   {
//Incendiary round creates fire. Bwaha.
      fObjIndex = get_obj_index( 82 );
      fobj = create_object( fObjIndex, ch->top_level );
      fobj->timer = number_range( 3, 5 );
      fobj = obj_to_room( fobj, proom );
   }
}

void do_makegoggles( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checktool, checkduraplast, checkcirc, checkbatt, checklens;
   OBJ_DATA *obj;
   AFFECT_DATA *aff;

   argument = one_argument( argument, arg );
   strcpy( arg2, argument );

   switch ( ch->substate )
   {
      default:

         if( arg2[0] == '\0' || ( str_cmp( arg, "infrared" ) && str_cmp( arg, "magnifying" ) ) )
         {
            send_to_char( "&RUsage: Makegoggles <infrared/magnifying> <name>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkcirc = FALSE;
         checkduraplast = FALSE;
         checkbatt = FALSE;
         checklens = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_DURAPLAST )
               checkduraplast = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_LENS )
               checklens = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
         }

         if( !checkduraplast )
         {
            send_to_char( "&RYou need a piece of duraplast.\n\r", ch );
            return;
         }
         if( !checkcirc )
         {
            send_to_char( "&RYou'll need some circuitry.\n\r", ch );
            return;
         }
         if( !checkbatt )
         {
            send_to_char( "&RYou need a small battery.\n\r", ch );
            return;
         }
         if( !checklens )
         {
            send_to_char( "&RYou'll need a lens.\n\r", ch );
            return;
         }
         if( !checktool )
         {
            send_to_char( "&RYou'll need a toolkit.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makegoggles] );
         if( number_percent(  ) < schance )
         {
            if( !str_cmp( arg, "infrared" ) )
               send_to_char( "&GYou begin the long process of creating a pair of infrared goggles.\n\r", ch );
            else
               send_to_char( "&GYou begin the long process of creating a pair of magnifying goggles.\n\r", ch );
            act( AT_PLAIN, "$n takes $s toolkit and begins putting something together.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 12, do_makegoggles, 1 );
            ch->dest_buf = str_dup( arg );
            ch->dest_buf_2 = str_dup( arg2 );
            return;
         }
         send_to_char( "&RYou can't figure out what to do.\n\r", ch );
         learn_from_failure( ch, gsn_makegoggles );
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
         send_to_char( "&RYou are interrupted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makegoggles] );

   checktool = FALSE;
   checklens = FALSE;
   checkcirc = FALSE;
   checkbatt = FALSE;
   checkduraplast = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         checkcirc = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }
      if( obj->item_type == ITEM_LENS && checklens == FALSE )
      {
         checklens = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }
      if( obj->item_type == ITEM_DURAPLAST && checkduraplast == FALSE )
      {
         checkduraplast = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         checkbatt = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makegoggles] );

   if( number_percent(  ) > schance * 2 || ( !checkcirc ) || ( !checkbatt ) || ( !checkduraplast ) || ( !checklens ) )
   {
      send_to_char( "&RYou turn the goggles on, but the lens shatters!\n\r", ch );
      learn_from_failure( ch, gsn_makegoggles );
      return;
   }

   obj = create_object( get_obj_index( 85 ), level );
   obj->item_type = ITEM_GOGGLES;
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   SET_BIT( obj->wear_flags, ITEM_WEAR_EYES );

   obj->level = ch->top_level;
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
   obj->value[0] = ch->top_level;
   obj->value[1] = 0;
   obj->value[2] = 0;
   obj->value[3] = 0;
   if( !str_cmp( arg, "magnifying" ) )
   {
      if( !IS_NPC( ch ) )
         obj->value[4] = ch->pcdata->learned[gsn_makegoggles] / 10;
      else
         obj->value[4] = number_range( 2, 5 );
   }
   if( !str_cmp( arg, "infrared" ) )
      obj->value[5] = 1;
   else
      obj->value[5] = 0;
   if( !str_cmp( arg, "infrared" ) )
   {
      CREATE( aff, AFFECT_DATA, 1 );
      aff->type = -1;
      aff->duration = -1;
      aff->location = APPLY_AFFECT;
      aff->modifier = 512;
      aff->bitvector = 0;
      aff->next = NULL;
      LINK( aff, obj->first_affect, obj->last_affect, next, prev );
      ++top_affect;
   }

   obj->cost = number_range( 2500, 3500 );

   obj = obj_to_char( obj, ch );

   if( !str_cmp( arg, "infrared" ) )
      send_to_char( "&GYou finish making a pair of infrared goggles.&w\n\r", ch );
   else
      send_to_char( "&GYou finish making a pair of magnifying goggles.&w\n\r", ch );

   act( AT_PLAIN, "$n finishes contstructing a pair of goggles.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 100,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.\n\r", xpgain );
   }
   learn_from_success( ch, gsn_makegoggles );
}

void do_makemissile( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int level, schance;
   bool checktool, checkdura, checkbatt, checkcirc;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum, chemNum;

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makemissile <explosive/incendiary>\n\r&w", ch );
            return;
         }

         checktool = FALSE;
         checkdura = FALSE;
         checkbatt = FALSE;
         checkcirc = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }
         chemNum = 0;
         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_DURASTEEL )
               checkdura = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbatt = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcirc = TRUE;
            if( obj->item_type == ITEM_CHEMICAL )
               chemNum += obj->count;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit to make a missile.\n\r", ch );
            return;
         }

         if( !checkdura )
         {
            send_to_char( "&RYou'll need a chunk of durasteel to make the shell.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a small battery.\n\r", ch );
            return;
         }

         if( !checkcirc )
         {
            send_to_char( "&RYou need a small circuit.\n\r", ch );
            return;
         }

         if( chemNum < 3 )
         {
            send_to_char( "&RYou'll need two chemicals for the explosive, and one for propulsion.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makemissile] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a missile.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a few parts, beginning to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 22, do_makemissile, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makemissile );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makemissile] );
   vnum = 80;

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
      if( obj->item_type == ITEM_CHEMICAL && chemNum > 0 )
      {
         if( obj->count > 3 )
         {
            obj->count -= 3;
            chemNum -= 3;
         }
         else if( obj->count == 1 )
         {
            separate_obj( obj );
            obj_from_char( obj );
            extract_obj( obj );
            chemNum -= 1;
         }
         else if( obj->count == 3 )
         {
            obj_from_char( obj );
            extract_obj( obj );
            chemNum -= 3;
         }
         else
         {
            obj_from_char( obj );
            extract_obj( obj );
            chemNum -= 2;
         }
      }
      if( obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkcirc = TRUE;
      }
      if( obj->item_type == ITEM_DURASTEEL && checkdura == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkdura = TRUE;
      }
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makemissile] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkcirc ) )
   {
      if( number_range( 0, 1 ) == 1 )
         send_to_char( "&RYou finish the missile, but you forgot to install the propulsion primer!\n\r", ch );
      else if( number_range( 0, 1 ) == 1 )
         send_to_char( "&RYou finish the missile, but half the explosive is still on the worktable!\n\r", ch );
      else if( number_range( 0, 1 ) == 1 )
         send_to_char( "&RYou finish the missile, but the circuitry fails, and it starts to smoke.\n\r", ch );
      else
      {
         send_to_char( "&RYou finish the missile, only to figure out that you've made a grenade!\n\r", ch );
         send_to_char( "&RYou toss the grenade into the disposal.\n\r", ch );
      }
      learn_from_failure( ch, gsn_makemissile );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_MISSILE;
   SET_BIT( obj->wear_flags, ITEM_HOLD );
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = 2;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " missile " );
   sprintf( buf, "%s", remand( buf ) );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   if( !str_cmp( arg, "incendiary" ) )
      obj->short_descr = STRALLOC( "an incendiary missile" );
   else
      obj->short_descr = STRALLOC( "an explosive missile" );
   STRFREE( obj->description );
   strcat( buf, " was carelessly misplaced here." );
   obj->description = STRALLOC( buf );
   if( !IS_NPC( ch ) )
   {
      obj->value[1] = ch->pcdata->learned[gsn_makemissile];
      obj->value[2] = number_range( ch->pcdata->learned[gsn_makemissile], ch->pcdata->learned[gsn_makemissile] + 100 );
   }
   obj->cost = number_range( obj->value[1], obj->value[2] ) * 100;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created missile.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new missile.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makemissile );
}

void do_launch2( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *rch;
   CHAR_DATA *zch;
   OBJ_DATA *wield;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char *dtxt;
   char *ftxt;
   short dir, dist = 0, max_dist;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *proom;
   int schance, missroom;
   char buf[MAX_STRING_LENGTH];
   bool exfound;
   bool msgsent;

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   wield = get_eq_char( ch, WEAR_WIELD );


   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You'll have to do that elsewhere.\n\r", ch );
      return;
   }

   if( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
   {
      send_to_char( "You can't do that while wielding two weapons.\n\r", ch );
      return;
   }
   if( !wield )
   {
      send_to_char( "You must have a launcher equipped first.\n\r", ch );
      return;
   }

   if( wield->item_type != ITEM_GLAUNCHER && wield->item_type != ITEM_RLAUNCHER )
   {
      send_to_char( "You must have a launcher equipped first.\n\r", ch );
      return;
   }

   if( ( arg[0] == '\0' ) )
   {
      send_to_char( "Missile Launcher usage:  launch <dir> <optional target>.\n\r", ch );
      send_to_char( "Grenade Launcher usage:  launch <dir> <how far>.\n\r", ch );
      return;
   }
   dir = get_door( arg );
   if( wield->item_type == ITEM_RLAUNCHER )
   {
      if( wield->value[5] == 0 )
      {
         send_to_char( "You must load a missile into it first.\n\r", ch );
         return;
      }
      if( wield->value[2] == 1 && !ch->aiming_at && arg2[0] != '\0' )
      {
         send_to_char( "This launcher is equipped with a guidance system...you must AIM first.\n\r", ch );
         return;
      }
      max_dist = wield->value[0];
      if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
      {
         send_to_char( "Your ignorance of common sense shows as you fire the missile into a wall!\n\r", ch );
         act( AT_ACTION, "$n fires a missile into the wall!", ch, NULL, NULL, TO_ROOM );

         if( wield->value[1] == 1 )
            explode_emissile( ch, ch->in_room, wield->value[3], wield->value[4], 1 );
         else
            explode_emissile( ch, ch->in_room, wield->value[3], wield->value[4], 0 );


//Unload launcher
         wield->value[0] = 0;
         wield->value[1] = 0;
         wield->value[3] = 0;
         wield->value[4] = 0;
         wield->value[5] = 0;
         return;
      }  // end if firing into wall


      switch ( dir )
      {
         default:
            dtxt = "somewhere";
            break;
         case 0:
            dtxt = "north";
            ftxt = "the south";
            break;
         case 1:
            dtxt = "east";
            ftxt = "the west";
            break;
         case 2:
            dtxt = "south";
            ftxt = "the north";
            break;
         case 3:
            dtxt = "west";
            ftxt = "the east";
            break;
         case 4:
            dtxt = "up";
            ftxt = "below";
            break;
         case 5:
            dtxt = "down";
            ftxt = "above";
            break;
         case 6:
            dtxt = "southwest";
            ftxt = "the northeast";
            break;
         case 7:
            dtxt = "southeast";
            ftxt = "the northwest";
            break;
         case 8:
            dtxt = "northwest";
            ftxt = "the southeast";
            break;
         case 9:
            dtxt = "northeast";
            ftxt = "the southwest";
            break;
      }

      ch_printf( ch, "You launch the missile %s.\n\r", dtxt );

      for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
      {
         if( pexit->vdir == dir )
         {
            if( IS_SET( pexit->exit_info, EX_CLOSED ) )  // Door in the way?
            {
               if( !IS_SET( pexit->exit_info, EX_BASHPROOF ) )
               {
                  REMOVE_BIT( pexit->exit_info, EX_CLOSED );
                  if( IS_SET( pexit->exit_info, EX_LOCKED ) )
                     REMOVE_BIT( pexit->exit_info, EX_LOCKED );
                  SET_BIT( pexit->exit_info, EX_BASHED );
                  for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
                  {
                     if( rch == ch )
                        continue;
                     sprintf( buf, "%s launches a missile %s, and it EXPLODES upon impact of the doorway!", PERS( ch, rch ),
                              dtxt );
                     send_to_char( buf, rch );
                  }
                  send_to_char( "&RThe missile flies directly into the doorway, and EXPLODES!\n\r", ch );
                  send_to_char( "&wThe doorway is reduced to nothing but debris.\n\r", ch );
                  act( AT_ACTION, "&RThe doorway is reduced to nothing but debris.&w\n\r", ch, NULL, NULL, TO_ROOM );
                  if( wield->value[1] == 1 )
                     explode_emissile( ch, ch->in_room, wield->value[3], wield->value[4], 1 );
                  else
                     explode_emissile( ch, ch->in_room, wield->value[3], wield->value[4], 0 );
// Knock char down if close to door (1 in 4 schance)           
                  for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
                  {
                     if( number_range( 1, 4 ) == 4 )
                     {
                        ch_printf( rch,
                                   "&RYou are too close to the door, and the EXPLOSION knocks you to the ground!&w\n\r" );
                        act( AT_RED, "$n is too close to the door, and the EXPLOSION knocks them to the ground!", rch, NULL,
                             NULL, TO_ROOM );
                        rch->position = POS_SITTING;
                     }
                  }

               }
               else  // is bashproof
               {
                  for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
                  {
                     if( rch == ch )
                        continue;
                     sprintf( buf, "&w%s fires a missile %s, and it explodes upon impact of the doorway!", PERS( ch, rch ),
                              dtxt );
                     send_to_char( buf, rch );
                  }
                  send_to_char( "&RThe missile flies directly into the doorway, and EXPLODES!\n\r", ch );
                  send_to_char( "&wThe door remains undamaged, aside from some carbon scoring.\n\r", ch );
                  act( AT_ACTION, "&RThe door remains undamaged, aside from some carbon scoring.", ch, NULL, NULL, TO_ROOM );
                  if( wield->value[1] == 1 )
                     explode_emissile( ch, ch->in_room, wield->value[3], wield->value[4], 1 );
                  else
                     explode_emissile( ch, ch->in_room, wield->value[3], wield->value[4], 0 );

               }  //end bashproof check
//Unload launcher
               wield->value[0] = 0;
               wield->value[1] = 0;
               wield->value[3] = 0;
               wield->value[4] = 0;
               wield->value[5] = 0;
               return;
            }  //end if door in way

            act( AT_WHITE, "&w$n launches a missile $d!", ch, NULL, dtxt, TO_ROOM );
            missroom = pexit->vnum;
            proom = get_room_index( missroom );
            dist = 1;
            break;
         }
      }  //end missile-in-room

      for( ;; )   // next room, and all rooms after
      {
         msgsent = FALSE;
         for( pexit = proom->first_exit; pexit; pexit = pexit->next )
         {
            if( pexit->vdir == dir )   // get next room information
            {
               missroom = pexit->vnum;
               exfound = TRUE;
               break;
            }
            else
               exfound = FALSE;
         }

         //check target, return if hit
         for( rch = proom->first_person; rch; rch = rch->next_in_room ) // Target?
         {
            if( arg2[0] == '\0' )
               break;
            if( ch->aiming_at == rch ) // Laser guidance on launcher, like having 100% launchers
            {
               if( !IS_NPC( rch ) )
                  schance = 100 - rch->pcdata->learned[gsn_dodge];
               else
                  schance = 10;
            }
            else
            {
               if( !IS_NPC( rch ) )
                  schance = ch->pcdata->learned[gsn_launchers] - rch->pcdata->learned[gsn_dodge];
               else
                  schance = 10;
            }
            if( number_range( -15, schance + ( rch->perm_lck - 12 ) ) >= 0 )  // Direct hit
            {
               ch_printf( ch, "Your missile scores a direct hit on %s!\n\r", PERS( rch, ch ) );
               ch_printf( rch, "A missile fired from %s flies into you!\n\r", ftxt );
               for( zch = proom->first_person; zch; zch = zch->next_in_room )
               {
                  if( zch == rch )
                     continue;
                  ch_printf( zch, "A missile fired from the %s flies right into %s!\n\r", ftxt, PERS( rch, zch ) );
               }
               if( wield->value[1] == 1 )
                  explode_emissile( ch, proom, wield->value[3], wield->value[4], 1 );
               else
                  explode_emissile( ch, proom, wield->value[3], wield->value[4], 0 );

// Unload launcher
               wield->value[0] = 0;
               wield->value[1] = 0;
               wield->value[3] = 0;
               wield->value[4] = 0;
               wield->value[5] = 0;
               return;
            }
            else if( number_range( -15, schance + ( rch->perm_lck - 12 ) ) >= -5 )  // Miss, but still damages
            {
               ch_printf( ch, "%s narrowly dodges your missile, but it explodes nearby.\n\r", PERS( rch, ch ) );
               ch_printf( rch, "You barely dodge a missile fired from %s, but it explodes near you!\n\r", ftxt );
               for( zch = proom->first_person; zch; zch = zch->next_in_room )
               {
                  if( zch == rch )
                     continue;
                  ch_printf( zch, "%s barely dodges a missile fired at them from %s, but it still explodes nearby!\n\r",
                             PERS( rch, zch ), ftxt );
               }
               if( wield->value[1] == 1 )
                  explode_emissile( ch, proom, wield->value[3], wield->value[4], 1 );
               else
                  explode_emissile( ch, proom, wield->value[3], wield->value[4], 0 );
// Unload launcher
               wield->value[0] = 0;
               wield->value[1] = 0;
               wield->value[3] = 0;
               wield->value[4] = 0;
               wield->value[5] = 0;

               return;
            }
            else
            {
               ch_printf( ch, "%s dodges the missile fired at them.\n\r", PERS( rch, ch ) );
               ch_printf( rch, "A missile flies in from %s! Luckily, you dodge it.\n\r", ftxt );
               for( zch = proom->first_person; zch; zch = zch->next_in_room )
               {
                  if( zch == rch )
                     continue;
                  ch_printf( zch, "A missile flies in from %s at %s! They quickly dodge it.\n\r", ftxt, PERS( rch, zch ) );
               }
               msgsent = TRUE;
            }
            if( ch->aiming_at )
            {
               send_to_char( "&G*&gbeep&G* Target lost. Target acquisition necessary for relaunch.\n\r", ch );
               ch->aiming_at = NULL;
            }

         }
         //end ?target

         //check door, return if hit
         if( exfound == TRUE )
         {
            if( IS_SET( pexit->exit_info, EX_CLOSED ) )  // Door in the way?
            {
               if( !IS_SET( pexit->exit_info, EX_BASHPROOF ) )
               {
                  REMOVE_BIT( pexit->exit_info, EX_CLOSED );
                  if( IS_SET( pexit->exit_info, EX_LOCKED ) )
                     REMOVE_BIT( pexit->exit_info, EX_LOCKED );
                  SET_BIT( pexit->exit_info, EX_BASHED );

                  for( rch = proom->first_person; rch; rch = rch->next_in_room )
                  {
                     ch_printf( rch, "&wA missile flies in from the %s, and explodes upon impact of the doorway %s!\n\r",
                                ftxt, dtxt );
                     ch_printf( rch, "&RThe doorway is reduced to nothing but debris.&w\n\r" );
                     ch_printf( ch, "&WThe missile travels %d room%s away, and hits a door, which is destroyed!\n\r",
                                dist, dist > 1 ? "s" : "" );
                  }
               }
               else
               {
                  for( rch = proom->first_person; rch; rch = rch->next_in_room )
                  {
                     ch_printf( rch, "&wA missile flies in from the %s, and explodes upon impact of the doorway %s!\n\r",
                                ftxt, dtxt );
                     ch_printf( rch, "&RThe door remains undamaged, aside from some carbon scoring.\n\r" );
                     ch_printf( ch, "&WThe missile travels %d room%s, and hits a door, which remains undamaged!\n\r",
                                dist, dist > 1 ? "s" : "" );
                  }
               }
               if( wield->value[1] == 1 )
                  explode_emissile( ch, proom, wield->value[3], wield->value[4], 1 );
               else
                  explode_emissile( ch, proom, wield->value[3], wield->value[4], 0 );
// Knock char down if close to door (1 in 4 schance)           
               for( rch = proom->first_person; rch; rch = rch->next_in_room )
               {
                  if( number_range( 1, 4 ) == 4 )
                  {
                     ch_printf( rch, "&wYou are too close to the door, and the EXPLOSION knocks you to the ground!&w\n\r" );
                     for( zch = proom->first_person; zch; zch = zch->next_in_room )
                     {
                        if( zch == rch )
                           continue;
                        ch_printf( zch, "&w%s is too close to the door, and the EXPLOSION knocks them to the ground!",
                                   PERS( zch, rch ) );
                     }
                     rch->position = POS_SITTING;
                  }
               }

//Unload launcher
               wield->value[0] = 0;
               wield->value[1] = 0;
               wield->value[3] = 0;
               wield->value[4] = 0;
               wield->value[5] = 0;

               return;
            }
         }
         // end check for door

         //check distance, return if drop
         if( dist == max_dist )  //Missile reaches range
         {
            if( msgsent == FALSE )
               sprintf( buf, "A missile flies in from %s, loses speed, and drops, exploding!\n\r", ftxt );
            else
               sprintf( buf, "The missile loses speed and drops, exploding!\n\r" );
            for( zch = proom->first_person; zch; zch = zch->next_in_room )
               send_to_char( buf, zch );

            ch_printf( ch, "The missile travels %d room%s away, loses speed, and explodes as it drops!\n\r",
                       dist, dist > 1 ? "s" : "" );
            if( wield->value[1] == 1 )
               explode_emissile( ch, proom, wield->value[3], wield->value[4], 1 );
            else
               explode_emissile( ch, proom, wield->value[3], wield->value[4], 0 );

//Unload launcher
            wield->value[0] = 0;
            wield->value[1] = 0;
            wield->value[3] = 0;
            wield->value[4] = 0;
            wield->value[5] = 0;

            return;
         }  //End reaches range

         if( exfound == FALSE )
            break;
         else
         {
            if( msgsent == FALSE )
               sprintf( buf, "A missile flies in from %s, and continues %s!\n\r", ftxt, dtxt );
            else
               sprintf( buf, "The missile continues %s. That was close!\n\r", dtxt );
            for( zch = proom->first_person; zch; zch = zch->next_in_room )
               send_to_char( buf, zch );
            dist++;
            missroom = pexit->vnum;
            proom = get_room_index( missroom );
         }
      }

      if( exfound == FALSE )
      {
         //msgsent, boom, return
         if( msgsent == FALSE )
            sprintf( buf, "A missile flies in from %s, and explodes, hitting a wall!\n\r", ftxt );
         else
            sprintf( buf, "The missile flies into a wall, and explodes!\n\r" );
         for( zch = proom->first_person; zch; zch = zch->next_in_room )
            send_to_char( buf, zch );
         ch_printf( ch, "The missile flies %d room%s away, and hits a wall, exploding!\n\r", dist, dist > 1 ? "s" : "" );

         if( wield->value[1] == 1 )
            explode_emissile( ch, proom, wield->value[3], wield->value[4], 1 );
         else
            explode_emissile( ch, proom, wield->value[3], wield->value[4], 0 );
      }

      // unload launcher
      wield->value[0] = 0;
      wield->value[1] = 0;
      wield->value[3] = 0;
      wield->value[4] = 0;
      wield->value[5] = 0;
      return;
   }  //end if rlauncher
/*
  if( wield->item_type == ITEM_GLAUNCHER )
  {
    if(wield->value[1] + wield->value[2] == 0)
    {
     send_to_char("You don't have any grenades loaded.\n\r", ch);
     return;
    }
    if(!isdigit(atoi(arg2))
    {
     ch_printf("Invalid distance. It must be a number between 1 and %d.\n\r", wield->value[5])
    if(atoi(arg2) > wield->value[5])
    {
     send_to_char("This launcher isn't capable of firing a grenade that far.\n\r", ch);
     return;
    }
    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
    }
	*/
}

void do_load( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *rch;
   OBJ_DATA *obj;
   OBJ_DATA *launcher;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Usage: load <object> <launcher>.\n\r", ch );
      return;
   }

   launcher = get_obj_carry( ch, arg2 );

   if( !launcher )
   {
      ch_printf( ch, "You don't seem to be carrying a '%s'.\n\r", arg2 );
      return;
   }
   if( launcher->item_type != ITEM_RLAUNCHER && launcher->item_type != ITEM_GLAUNCHER )
   {
      ch_printf( ch, "&w%s&w isn't a launcher.\n\r", launcher->short_descr );
      return;
   }
   if( launcher->item_type == ITEM_RLAUNCHER )
   {
      if( launcher->value[5] == 1 )
      {
         ch_printf( ch, "&w%s&w is already loaded with an %s missile.\n\r",
                    launcher->short_descr, launcher->value[1] == 0 ? "explosive" : "incendiary" );
         return;
      }
      if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
      {
         ch_printf( ch, "You don't seem to be carrying a '%s'.\n\r", arg1 );
         return;
      }
      else
      {
         if( obj->item_type != ITEM_MISSILE )
         {
            ch_printf( ch, "&w%s&w isn't a missile.\n\r", obj->short_descr );
            return;
         }
      }

      launcher->value[0] = obj->value[0];
      launcher->value[1] = obj->value[1];
      launcher->value[3] = obj->value[3];
      launcher->value[4] = obj->value[4];
      launcher->value[5] = 1;

      obj_from_char( obj );
      extract_obj( obj );

      WAIT_STATE( ch, 12 );
      ch_printf( ch, "&wYou load &w%s&w into &w%s&w.\n\r", obj->short_descr, launcher->short_descr );
      for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      {
         if( ch == rch )
            continue;
         ch_printf( rch, "&w%s loads &w%s&w into &w%s&w.\n\r", PERS( ch, rch ), obj->short_descr, launcher->short_descr );
      }
      return;
   }

}

void do_unload( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *rch;
   OBJ_DATA *obj;
   OBJ_DATA *launcher;

   if( argument == '\0' )
   {
      send_to_char( "Usage: unload <launcher>.\n\r", ch );
      return;
   }

   launcher = get_obj_carry( ch, argument );

   if( !launcher )
   {
      ch_printf( ch, "You don't seem to be carrying a '%s'.\n\r", argument );
      return;
   }
   if( launcher->item_type != ITEM_RLAUNCHER && launcher->item_type != ITEM_GLAUNCHER )
   {
      ch_printf( ch, "&w%s&w isn't a launcher.\n\r", launcher->short_descr );
      return;
   }

   if( launcher->item_type == ITEM_RLAUNCHER )
   {
      if( launcher->value[5] == 0 )
      {
         ch_printf( ch, "&w%s&w is already unloaded.\n\r", launcher->short_descr );
         return;
      }

      obj = create_object( get_obj_index( 80 ), ch->top_level );
      obj->item_type = ITEM_MISSILE;
      SET_BIT( obj->wear_flags, ITEM_HOLD );
      SET_BIT( obj->wear_flags, ITEM_TAKE );
      obj->level = ch->top_level;
      obj->weight = 2;
      STRFREE( obj->name );
      STRFREE( obj->short_descr );
      STRFREE( obj->description );
      if( launcher->value[1] == 0 )
      {
         obj->name = STRALLOC( "explosive missile" );
         obj->short_descr = STRALLOC( "an explosive missile" );
         obj->description = STRALLOC( "A missile has been carelessly left here." );
         obj->value[1] = 0;
      }
      else
      {
         obj->name = STRALLOC( "incendiary missile" );
         obj->short_descr = STRALLOC( "an incendiary missile" );
         obj->value[1] = 1;
      }
      obj->value[0] = launcher->value[0];
      obj->value[2] = 0;
      obj->description = STRALLOC( "A missile has been carelessly left here." );
      obj->cost = number_range( obj->value[3], obj->value[4] ) * 25;
      obj->value[3] = launcher->value[3];
      obj->value[4] = launcher->value[4];

      obj = obj_to_char( obj, ch );

      launcher->value[0] = 0;
      launcher->value[1] = 0;
      launcher->value[3] = 0;
      launcher->value[4] = 0;
      launcher->value[5] = 0;

      WAIT_STATE( ch, 12 );
      ch_printf( ch, "&wYou unload %s&w.\n\r", launcher->short_descr );
      for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      {
         if( ch == rch )
            continue;
         ch_printf( rch, "&w%s&w unloads %s&w.\n\r", PERS( ch, rch ), launcher->short_descr );
      }
      return;
   }
}

void do_link( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int x;

   if( IS_NPC( ch ) )
      return;

   if( ( ship = ship_from_gunseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the gunseat of a ship to link weapon systems.\n\r", ch );
      return;
   }

   if( str_cmp( argument, "primary" ) && str_cmp( argument, "secondary" ) && str_cmp( argument, "launchers" ) )
   {
      send_to_char( "Usage: link <primary/secondary/launchers>\n\r", ch );
      return;
   }

   if( !str_prefix( argument, "primary" ) )
   {
      if( ship->primaryType == B_NONE )
      {
         send_to_char( "&RThis ship is not equipped with a primary weapon system.\n\r", ch );
         return;
      }

      switch ( ship->primaryType )
      {
         case 0:
            x = 0;
            break;
         case 1:
            x = ship->primaryCount;
            break;
         case 2:
            x = 2 * ship->primaryCount;
            break;
         case 3:
            x = 3 * ship->primaryCount;
            break;
         case 4:
            x = 4 * ship->primaryCount;
            break;
         case 5:
            x = ship->primaryCount;
            break;
         case 6:
            x = ship->primaryCount;
            break;
         case 7:
            x = ship->primaryCount;
            break;
         case 8:
            x = ship->primaryCount;
            break;
         case 9:
            x = ship->primaryCount;
            break;
         default:
            x = 1;
            break;
      }

      if( x <= 1 )
      {
         send_to_char( "The current primary weapon can only fire once.\n\r", ch );
         return;
      }

      ship->primaryLinked = TRUE;
      ch_printf( ch, "Primary weapon systems are linked. All available will fire.\n\r" );
      return;
   }
   if( !str_prefix( argument, "secondary" ) )
   {
      if( ship->secondaryType == B_NONE )
      {
         send_to_char( "&RThis ship is not equipped with a secondary weapon system.\n\r", ch );
         return;
      }

      switch ( ship->secondaryType )
      {
         case 0:
            x = 0;
            break;
         case 1:
            x = ship->secondaryCount;
            break;
         case 2:
            x = 2 * ship->secondaryCount;
            break;
         case 3:
            x = 3 * ship->secondaryCount;
            break;
         case 4:
            x = 4 * ship->secondaryCount;
            break;
         case 5:
            x = ship->secondaryCount;
            break;
         case 6:
            x = ship->secondaryCount;
            break;
         case 7:
            x = ship->secondaryCount;
            break;
         case 8:
            x = ship->secondaryCount;
            break;
         case 9:
            x = ship->secondaryCount;
            break;
         default:
            x = 1;
            break;
      }

      if( x <= 1 )
      {
         send_to_char( "The current secondary weapon can only fire once.\n\r", ch );
         return;
      }

      ship->secondaryLinked = TRUE;
      ch_printf( ch, "Secondary weapon systems are linked. All available will fire.\n\r" );
      return;
   }
   if( !str_prefix( argument, "launchers" ) )
   {
      if( !ship->maxmissiles && !ship->maxtorpedos && !ship->maxrockets )
      {
         send_to_char( "This ship does not have a launcher system installed.\n\r", ch );
         return;
      }

      if( ( ship->maxmissiles && ship->maxtorpedos ) ||
          ( ship->maxmissiles && ship->maxrockets ) ||
          ( ship->maxtorpedos && ship->maxmissiles ) ||
          ( ship->maxtorpedos && ship->maxrockets ) ||
          ( ship->maxrockets && ship->maxmissiles ) || ( ship->maxrockets && ship->maxtorpedos ) )
      {
         ship->warheadLinked = TRUE;
         ch_printf( ch, "Launcher system linked. All available will fire.\n\r" );
         return;
      }
      else
      {
         ch_printf( ch, "You only have one launcher type, linking is unnecessary.\n\r" );
         return;
      }
   }
}

void do_unlink( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;

   if( ( ship = ship_from_gunseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the gunseat of a ship to unlink weapon systems.\n\r", ch );
      return;
   }

   if( str_cmp( argument, "primary" ) && str_cmp( argument, "secondary" )
       && str_cmp( argument, "launchers" ) && str_cmp( argument, "all" ) )
   {
      send_to_char( "Usage: unlink <primary/secondary/launchers/all>\n\r", ch );
      return;
   }

   if( !str_prefix( argument, "primary" ) )
   {
      if( ship->primaryCount == 0 )
      {
         send_to_char( "This ship doesn't have a primary weapon system.\n\r", ch );
         return;
      }
      if( ship->primaryLinked == FALSE )
      {
         send_to_char( "The primary weapon systems aren't linked.\n\r", ch );
         return;
      }
      ship->primaryLinked = FALSE;
      send_to_char( "You unlink the primary weapon systems.\n\r", ch );
      act( AT_PLAIN, "$n presses a few buttons on the console.\n\r", ch, NULL, NULL, TO_ROOM );
      return;
   }
   if( !str_prefix( argument, "secondary" ) )
   {
      if( ship->secondaryCount == 0 )
      {
         send_to_char( "This ship doesn't have a secondary weapon system.\n\r", ch );
         return;
      }
      if( ship->secondaryLinked == FALSE )
      {
         send_to_char( "The secondary weapon systems aren't linked.\n\r", ch );
         return;
      }
      ship->secondaryLinked = FALSE;
      send_to_char( "You unlink the secondary weapon systems.\n\r", ch );
      act( AT_PLAIN, "$n presses a few buttons on the console.\n\r", ch, NULL, NULL, TO_ROOM );
      return;
   }
   if( !str_prefix( argument, "launchers" ) )
   {
      if( ship->warheadLinked == FALSE )
      {
         send_to_char( "The launcher systems aren't linked.\n\r", ch );
         return;
      }
      ship->warheadLinked = FALSE;
      send_to_char( "You unlink the launcher systems.\n\r", ch );
      act( AT_PLAIN, "$n presses a few buttons on the console.\n\r", ch, NULL, NULL, TO_ROOM );
      return;
   }
   if( !str_prefix( argument, "all" ) )
   {
      if( ship->warheadLinked == FALSE && ship->primaryLinked == FALSE && ship->secondaryLinked == FALSE )
      {
         send_to_char( "No ship system is linked.\n\r", ch );
         return;
      }
      if( ship->primaryLinked == TRUE )
      {
         ship->primaryLinked = FALSE;
         send_to_char( "Primary weapon systems unlinked.\n\r", ch );
      }
      if( ship->secondaryLinked == TRUE )
      {
         ship->secondaryLinked = FALSE;
         send_to_char( "Secondary weapon systems unlinked.\n\r", ch );
      }
      if( ship->warheadLinked == TRUE )
      {
         ship->warheadLinked = FALSE;
         send_to_char( "Launcher systems unlinked.\n\r", ch );
      }
      return;
   }
}

void do_barrel_roll( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];
   int schance;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to perform a maneuver.\n\r", ch );
      return;
   }

   if( ship->manuever < 70 )
   {
      send_to_char( "This ship isn't maneuverable enough to perform a barrel roll.\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace.\n\r", ch );
      return;
   }
   if( !ship->starsystem )
   {
      send_to_char( "&RYou can only do that in realspace.\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         schance = IS_NPC( ch ) ? ch->top_level * 2 : ch->pcdata->learned[gsn_barrelroll];
         if( number_percent(  ) > schance )
         {
            send_to_char( "&RYou attempt to do a barrel roll, but fail horribly.\n\r", ch );
            return;
         }

         if( ship->juking == TRUE )
         {
            send_to_char( "&YYou stop juking from side to side, and perform a barrel roll.\n\r", ch );
            sprintf( buf, "%s stops juking from side to side.", ship->name );
            echo_to_system( AT_YELLOW, ship, buf, NULL );
            ship->juking = FALSE;
         }
         ship->rolling = TRUE;
         send_to_char( "&GThe ship starts rolling...\n\r", ch );
         act( AT_PLAIN, "$n performs a barrel roll.", ch, NULL, NULL, TO_ROOM );
         sprintf( buf, "%s performs a barrel roll.", ship->name );
         echo_to_system( AT_YELLOW, ship, buf, NULL );
         learn_from_success( ch, gsn_barrelroll );
         add_timer( ch, TIMER_DO_FUN, number_range( 3, 6 ), do_barrel_roll, 1 );
         return;

      case 1:
         break;

      case SUB_TIMER_DO_ABORT:
         ch->substate = SUB_NONE;
         if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
            return;
         send_to_char( "&YYou level out your flight pattern.\n\r", ch );
         act( AT_PLAIN, "$n levels out $s flight pattern.\n\r", ch, NULL, NULL, TO_ROOM );
         sprintf( buf, "%s levels out its flight pattern.", ship->name );
         echo_to_system( AT_YELLOW, ship, buf, NULL );
         ship->rolling = FALSE;
         return;
   }

   ch->substate = SUB_NONE;
   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
      return;
   send_to_char( "&YYou level out your flight pattern.\n\r", ch );
   act( AT_PLAIN, "$n levels out $s flight pattern.\n\r", ch, NULL, NULL, TO_ROOM );
   sprintf( buf, "%s levels out its flight pattern.", ship->name );
   echo_to_system( AT_YELLOW, ship, buf, NULL );
   ship->rolling = FALSE;
   return;
}

void do_juke( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];
   int schance;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to perform a maneuver.\n\r", ch );
      return;
   }

   if( ship->manuever < 75 )
   {
      send_to_char( "This ship isn't maneuverable enough to perform a juke.\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace.\n\r", ch );
      return;
   }
   if( !ship->starsystem )
   {
      send_to_char( "&RYou can only do that in realspace.\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         schance = IS_NPC( ch ) ? ch->top_level * 2 : ch->pcdata->learned[gsn_juke];
         if( number_percent(  ) > schance )
         {
            send_to_char( "&RYou attempt to do a juke, but fail horribly.\n\r", ch );
            return;
         }

         if( ship->rolling == TRUE )
         {
            send_to_char( "&YYou break out of the barrel roll, and start juking.\n\r", ch );
            sprintf( buf, "%s stops rolling.", ship->name );
            echo_to_system( AT_YELLOW, ship, buf, NULL );
            ship->rolling = FALSE;
         }
         ship->juking = TRUE;
         send_to_char( "&GYou perform the maneuver, and the ship jukes from side to side.\n\r", ch );
         act( AT_PLAIN, "$n jukes the ship from side to side.", ch, NULL, NULL, TO_ROOM );
         sprintf( buf, "%s starts juking from side to side.", ship->name );
         echo_to_system( AT_YELLOW, ship, buf, NULL );
         learn_from_success( ch, gsn_juke );
         add_timer( ch, TIMER_DO_FUN, number_range( 3, 6 ), do_juke, 1 );
         return;

      case 1:
         break;

      case SUB_TIMER_DO_ABORT:
         ch->substate = SUB_NONE;
         if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
            return;
         send_to_char( "&YYou level out your flight pattern.\n\r", ch );
         act( AT_PLAIN, "$n levels out $s flight pattern.\n\r", ch, NULL, NULL, TO_ROOM );
         sprintf( buf, "%s levels out its flight pattern.", ship->name );
         echo_to_system( AT_YELLOW, ship, buf, NULL );
         ship->juking = FALSE;
         return;
   }

   ch->substate = SUB_NONE;
   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
      return;
   send_to_char( "You level out your flight pattern.\n\r", ch );
   act( AT_PLAIN, "$n levels out $s flight pattern.\n\r", ch, NULL, NULL, TO_ROOM );
   sprintf( buf, "%s levels out its flight pattern.", ship->name );
   echo_to_system( AT_YELLOW, ship, buf, NULL );
   ship->juking = FALSE;
   return;
}
