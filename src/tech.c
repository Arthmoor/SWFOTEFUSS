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

/* This is for the new technician class. The technicians are going to be totally in control of ship related
   abilities. These include, creation of, and use of ship modules. Ship Maintanance, Custom Ship Design, and
   Ship Sabotage. Taking some of the abilities of engineers and getting some more.
*/

/* Modules are how ships are upgraded. Ships can have only so many modules, Depending on the type of ship. */
/* The effectiveness of the modules can vary depending on the level of the technitian's makemodule skill */
void do_makemodule( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int affecttype, affectammount;
   char name[MAX_STRING_LENGTH];
   int level, schance;
   bool checklens, checkbat, checksuper, checkcircuit, checktool;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;

   argument = one_argument( argument, arg );


   switch ( ch->substate )
   {
      default:

         if( str_cmp( arg, "hull" ) && str_cmp( arg, "torpedo" ) && str_cmp( arg, "rocket" ) && str_cmp( arg, "missile" )
             && str_cmp( arg, "primary" ) && str_cmp( arg, "secondary" ) && str_cmp( arg, "shield" )
             && str_cmp( arg, "speed" ) && str_cmp( arg, "hyperspeed" ) && str_cmp( arg, "energy" )
             && str_cmp( arg, "manuever" ) && str_cmp( arg, "chaff" ) && str_cmp( arg, "alarm" ) )
         {
            send_to_char
               ( "Modules may affect the following aspects of the ship:\n\rPrimary, Secondary, Missile, Rocket, Torpedo, Hull, Shield, Speed, Hyperspeed, Energy, Manuever, Chaff, and Alarm.\n\r",
                 ch );
            return;
         }
         checklens = FALSE;
         checkbat = FALSE;
         checksuper = FALSE;
         checkcircuit = FALSE;
         checktool = FALSE;
         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_LENS )
               checklens = TRUE;
            if( obj->item_type == ITEM_BATTERY )
               checkbat = TRUE;
            if( obj->item_type == ITEM_SUPERCONDUCTOR )
               checksuper = TRUE;
            if( obj->item_type == ITEM_CIRCUIT )
               checkcircuit = TRUE;
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;

         }

         if( !checklens )
         {
            send_to_char( "&RYou need a lens to control the energy.\n\r", ch );
            return;
         }

         if( !checkbat )
         {
            send_to_char( "&RYou need a battery to power the module.\n\r", ch );
            return;
         }

         if( !checksuper )
         {
            send_to_char( "&RYou need a superconductor to focus the energy.\n\r", ch );
            return;
         }

         if( !checkcircuit )
         {
            send_to_char( "&RYou need a circuit board to control the module.\n\r", ch );
            return;
         }

         if( !checktool )
         {
            send_to_char( "&RYou need a toolkit to build the module.\n\r", ch );
            return;
         }




         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makemodule] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of creating a module.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and begins to work.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 10, do_makemodule, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out what to do.\n\r", ch );
         learn_from_failure( ch, gsn_makemodule );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makemodule] );



   if( ( pObjIndex = get_obj_index( MODULE_VNUM ) ) == NULL )
   {
      send_to_char
         ( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
           ch );
      return;
   }

   checklens = FALSE;
   checkbat = FALSE;
   checksuper = FALSE;
   checkcircuit = FALSE;
   checktool = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;
      if( obj->item_type == ITEM_LENS && checklens == FALSE )
      {
         checklens = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }
      if( obj->item_type == ITEM_BATTERY && checkbat == FALSE )
      {
         checkbat = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }
      if( obj->item_type == ITEM_SUPERCONDUCTOR && checksuper == FALSE )
      {
         checksuper = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }
      if( obj->item_type == ITEM_CIRCUIT && checkcircuit == FALSE )
      {
         checkcircuit = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
      }

   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makemodule] );

   if( number_percent(  ) > schance * 2 || ( !checklens ) || ( !checktool ) || ( !checkbat ) || ( !checksuper )
       || ( !checkcircuit ) )
   {
      send_to_char( "&RYou hold up your newly created module.\n\r", ch );
      send_to_char( "&RThe module begins to shake violently turning red hot!\n\r", ch );
      send_to_char( "&RYou drop it as it begins to burn your hand and then.. It disintigrates!\n\r", ch );
      learn_from_failure( ch, gsn_makemodule );
      return;
   }

   if( !str_cmp( arg, "primary" ) )
   {
      affecttype = AFFECT_PRIMARY;
      affectammount = 1;
      strcpy( name, "A Primary Weapons Module" );
   }

   if( !str_cmp( arg, "secondary" ) )
   {
      affecttype = AFFECT_SECONDARY;
      affectammount = 1;
      strcpy( name, "A Secondary Weapons Module" );
   }

   if( !str_cmp( arg, "missile" ) )
   {
      affecttype = AFFECT_MISSILE;
      affectammount = ( level / 20 );
      strcpy( name, "A Missile Module" );
   }

   if( !str_cmp( arg, "rocket" ) )
   {
      affecttype = AFFECT_ROCKET;
      affectammount = ( level / 20 );
      strcpy( name, "A Rocket Module" );
   }

   if( !str_cmp( arg, "torpedo" ) )
   {
      affecttype = AFFECT_TORPEDO;
      affectammount = ( level / 20 );
      strcpy( name, "A Torpedo Module" );
   }

   if( !str_cmp( arg, "hull" ) )
   {
      affecttype = AFFECT_HULL;
      affectammount = ( level / 2 );
      strcpy( name, "A Hull Module" );
   }

   if( !str_cmp( arg, "shield" ) )
   {
      affecttype = AFFECT_SHIELD;
      affectammount = ( level / 5 );
      strcpy( name, "A Shield Module" );
   }
   if( !str_cmp( arg, "speed" ) )
   {
      affecttype = AFFECT_SPEED;
      affectammount = ( level / 10 );
      strcpy( name, "A Speed Module" );
   }
   if( !str_cmp( arg, "hyperspeed" ) )
   {
      affecttype = AFFECT_HYPER;
      affectammount = 1;
      strcpy( name, "A Hyperspeed Module" );
   }
   if( !str_cmp( arg, "energy" ) )
   {
      affecttype = AFFECT_ENERGY;
      affectammount = ( level * 5 );
      strcpy( name, "An Energy Module" );
   }
   if( !str_cmp( arg, "manuever" ) )
   {
      affecttype = AFFECT_MANUEVER;
      affectammount = ( level / 10 );
      strcpy( name, "A Manuever Module" );
   }
   if( !str_cmp( arg, "alarm" ) )
   {
      affecttype = AFFECT_ALARM;
      affectammount = 1;
      strcpy( name, "An Alarm Module" );
   }
   if( !str_cmp( arg, "chaff" ) )
   {
      affecttype = AFFECT_CHAFF;
      affectammount = URANGE( 1, ( level / 33 ), 3 );
      strcpy( name, "A Chaff Module" );
   }



   obj = create_object( pObjIndex, level );
   obj->item_type = ITEM_MODULE;
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   STRFREE( obj->name );
   obj->name = STRALLOC( name );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( name );
   STRFREE( obj->description );
   strcat( name, " was dropped here." );
   obj->description = STRALLOC( name );

   obj->value[0] = affecttype;
   obj->value[1] = affectammount;
   obj->value[2] = 0;
   obj->cost = ( level * affecttype * affectammount );

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created module.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes creating a new module.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain = ( ( ch->skill_level[TECHNICIAN_ABILITY] + 1 ) * 200 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, "You gain %d technician experience.", xpgain );
   }
   learn_from_success( ch, gsn_makemodule );
}


void do_showmodules( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   MODULE_DATA *mod;
   char buf[MAX_STRING_LENGTH];
   char str[MAX_STRING_LENGTH];
   int i;
   long xpgain;
   int schance;

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_showmodules] );

   if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be in the engine room of a ship.\n\r", ch );
      return;
   }

   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou fail to find the module control panel.\n\r", ch );
      learn_from_failure( ch, gsn_showmodules );
      return;
   }
   send_to_char( "&z+--------------------------------------+\n\r", ch );
   send_to_char( "&z| &RNum  Type                   Quantity &z|\n\r", ch );
   send_to_char( "&z| &r---  ----                   -------- &z|\n\r", ch );
   i = 0;
   for( mod = ship->first_module; mod; mod = mod->next )
   {
      i++;
      if( mod->affect == AFFECT_PRIMARY )
         strcpy( str, "Primary Weapon" );
      if( mod->affect == AFFECT_SECONDARY )
         strcpy( str, "Secondary Weapon" );
      if( mod->affect == AFFECT_MISSILE )
         strcpy( str, "Missile" );
      if( mod->affect == AFFECT_ROCKET )
         strcpy( str, "Rocket" );
      if( mod->affect == AFFECT_TORPEDO )
         strcpy( str, "Torpedo" );
      if( mod->affect == AFFECT_HULL )
         strcpy( str, "Hull" );
      if( mod->affect == AFFECT_SHIELD )
         strcpy( str, "Shields" );
      if( mod->affect == AFFECT_SPEED )
         strcpy( str, "Speed" );
      if( mod->affect == AFFECT_HYPER )
         strcpy( str, "Hyperspeed" );
      if( mod->affect == AFFECT_ENERGY )
         strcpy( str, "Energy" );
      if( mod->affect == AFFECT_MANUEVER )
         strcpy( str, "Manuever" );
      if( mod->affect == AFFECT_ALARM )
         strcpy( str, "Alarm" );
      if( mod->affect == AFFECT_CHAFF )
         strcpy( str, "Chaff" );

      sprintf( buf, "&z| &P%2d&p)  &G&W%-22.22s %-8.8d &z|\n\r", i, str, mod->ammount );
      send_to_char( buf, ch );
   }
   send_to_char( "&z+--------------------------------------+\n\r", ch );
   xpgain = UMAX( 100, i * 100 );
   gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
   ch_printf( ch, " You gain %d experience for being a Technician.\n\r", xpgain );
   learn_from_success( ch, gsn_showmodules );
}

void do_removemodule( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   bool checktool;
   OBJ_DATA *obj;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   MODULE_DATA *mod;
   int schance;
   int num, i;

   strcpy( arg, argument );
   checktool = FALSE;
   switch ( ch->substate )
   {
      default:
         if( ( ship = ship_from_engine( ch->in_room->vnum ) ) != NULL )
         {
            ship = ship_from_engine( ch->in_room->vnum );
         }

         if( !ship )
         {
            send_to_char( "You need to be in the ships engine room.\n\r", ch );
            return;
         }

         if( arg[0] == '\0' || atoi( arg ) == 0 )
         {
            send_to_char( "Remove WHICH module? Use the number next to it on showmodules.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
         }

         if( checktool == FALSE )
         {
            send_to_char( "You need a toolkit to remove a module.\n\r", ch );
            return;
         }
         i = 0;
         num = atoi( argument );
         for( mod = ship->first_module; mod; mod = mod->next )
            i++;

         if( i < num || i == 0 )
         {
            send_to_char( "No such module installed.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_removemodule] );
         if( number_percent(  ) < schance )
         {
            strcpy( arg, argument );
            ch->dest_buf = str_dup( arg );
            send_to_char( "&GYou begin the long process of removing a module.\n\r", ch );
            sprintf( buf, "$n takes out $s toolkit and begins to work.\n\r" );
            act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );

            add_timer( ch, TIMER_DO_FUN, 5, do_removemodule, 1 );
            return;
         }

         send_to_char( "&RYou are unable to figure out what to do.\n\r", ch );
         learn_from_failure( ch, gsn_removemodule );
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
         send_to_char( "&RYou are interupted and fail to finish.\n\r", ch );
         return;
   }
   ch->substate = SUB_NONE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checktool = TRUE;

   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_removemodule] );

   ship = ship_from_engine( ch->in_room->vnum );
   if( !ship )
   {
      send_to_char( "Error: Something went wrong. Contact an Admin!\n\r", ch );
      return;
   }

   if( number_percent(  ) > schance * 2 )
   {
      send_to_char( "&RYou finish removing the module and everything's looking good...\n\r", ch );
      send_to_char( "&RThen you realize you removed the hyperdrive energy core. OOPS!\n\r", ch );
      learn_from_failure( ch, gsn_removemodule );
      return;
   }
   num = atoi( arg );
   i = 0;
   for( mod = ship->first_module; mod; mod = mod->next )
   {
      i++;
      if( i == num )
      {
         if( mod->affect == AFFECT_PRIMARY )
            ship->primaryCount -= mod->ammount;
         if( mod->affect == AFFECT_SECONDARY )
            ship->secondaryCount -= mod->ammount;
         if( mod->affect == AFFECT_MISSILE )
         {
            ship->maxmissiles -= mod->ammount;
            ship->missiles = ship->maxmissiles;
         }
         if( mod->affect == AFFECT_ROCKET )
         {
            ship->maxrockets -= mod->ammount;
            ship->rockets = ship->maxrockets;
         }
         if( mod->affect == AFFECT_TORPEDO )
         {
            ship->maxtorpedos -= mod->ammount;
            ship->torpedos = ship->maxtorpedos;
         }
         if( mod->affect == AFFECT_HULL )
            ship->maxhull -= mod->ammount;
         if( mod->affect == AFFECT_SHIELD )
            ship->maxshield -= mod->ammount;
         if( mod->affect == AFFECT_SPEED )
            ship->realspeed -= mod->ammount;
         if( mod->affect == AFFECT_HYPER )
            ship->hyperspeed += mod->ammount;
         if( mod->affect == AFFECT_ENERGY )
            ship->maxenergy -= mod->ammount;
         if( mod->affect == AFFECT_MANUEVER )
            ship->manuever -= mod->ammount;
         if( mod->affect == AFFECT_ALARM )
            ship->alarm -= mod->ammount;
         if( mod->affect == AFFECT_CHAFF )
            ship->maxchaff -= mod->ammount;
         UNLINK( mod, ship->first_module, ship->last_module, next, prev );
         DISPOSE( mod );
         save_ship( ship );
         break;
      }
   }

   send_to_char( "You finish removing the module and toss the extra scraps away.\n\r", ch );

   {
      long xpgain;
      xpgain = ( ( ch->skill_level[TECHNICIAN_ABILITY] + 1 ) * 300 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, " You gain %d experience for being a Technician.\n\r", xpgain );
      learn_from_success( ch, gsn_removemodule );
   }
   return;
}

void do_shipmaintenance( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance, change, bombs = 0;
   long xp;
   SHIP_DATA *ship;
   OBJ_DATA *obj;
   int oldbombs;

   strcpy( arg, argument );

   if( ( ch->pcdata->learned[gsn_shipmaintenance] ) <= 0 )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
         {
            send_to_char( "&RYou must be in the engine room of a ship to do that!\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_shipmaintenance] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou start performing basic maintenance on your ship...\n\r", ch );
            act( AT_PLAIN, "$n begins some basic ship maintenance.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 15, do_shipmaintenance, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou fail to perform even the most basic of ship maintenance skills.\n\r", ch );
         learn_from_failure( ch, gsn_shipmaintenance );
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
         send_to_char( "&RYou are distracted and fail to finish your maintenance.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
   {
      return;
   }

   change = URANGE( 0,
                    number_range( ( int )( ch->pcdata->learned[gsn_shipmaintenance] / 2 ),
                                  ( int )( ch->pcdata->learned[gsn_shipmaintenance] ) ), ( ship->maxhull - ship->hull ) );
   ship->hull += change;
   ship->chaff = ship->maxchaff;
   ship->missiles = ship->maxmissiles;
   ship->torpedos = ship->maxtorpedos;
   ship->rockets = ship->maxrockets;

   oldbombs = ship->bombs;


   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( ship->maxbombs - bombs == 0 )
         break;

      if( obj->item_type == ITEM_SHIPBOMB )
      {

         if( ship->bombs > 0 && ship->lowbombstr > 0 && ship->hibombstr > 0 )
         {
            ship->lowbombstr = ( ( ( ship->lowbombstr * ship->bombs ) + obj->value[0] ) / ( ship->bombs + 1 ) );
            sprintf( log_buf, "Ships lowbombstr is %d", ship->lowbombstr );
            log_string( log_buf );

            ship->hibombstr = ( ( ship->hibombstr * ship->bombs ) + ( int )obj->value[1] ) / ( ship->bombs + 1 );
            sprintf( log_buf, "Ships hibombstr is %d", ship->hibombstr );
            log_string( log_buf );
         }
         else
         {
            ship->lowbombstr = obj->value[0];
            ship->hibombstr = obj->value[1];
         }

         if( obj->count > ( ship->maxbombs - bombs ) )
         {
            obj->count -= ( ship->maxbombs - bombs );
            ship->bombs += ( ship->maxbombs - bombs );
            break;
         }
         else
         {
            ship->bombs += obj->count;
            obj_from_char( obj );
            extract_obj( obj );
         }
      }  //if check
   }  // for loop   

   ch_printf( ch, "&GRepairs complete.. Hull raised %d points, ship weaponry and chaff restocked.\n\r", change );
   if( ship->bombs > oldbombs )
      ch_printf( ch, "&G%d bombs loaded into ship from inventory.\n\r", ship->bombs - oldbombs );

   act( AT_PLAIN, "$n finishes patching the hull and restocking the ship.", ch, NULL, argument, TO_ROOM );

   xp = get_ship_value( ship ) / 100;
   ch_printf( ch, "&WYou gain %ld points of technician experience!\n\r", ( get_ship_value( ship ) / 100 ) );
   gain_exp( ch, xp, TECHNICIAN_ABILITY );

   learn_from_success( ch, gsn_shipmaintenance );
   return;


}

void do_scanbugs( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;

   char arg[MAX_STRING_LENGTH];

   int schance;

   BUG_DATA *bugs;
   int i;

   argument = one_argument( argument, arg );

   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "Syntax: checkbugs <person>\n\r", ch );
            return;
         }


         victim = get_char_room( ch, arg );

         if( !victim )
         {
            send_to_char( "They aren't here.\n\r", ch );
            return;
         }

         if( IS_NPC( victim ) )
         {
            send_to_char( "You can only do that to players.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_scanbugs] );

         if( number_percent(  ) - 20 < schance )
         {
            send_to_char( "&w&GScanning...\n\r", ch );
            act( AT_PLAIN, "$n takes a scanner and begins to scan $N.", ch, NULL, victim, TO_NOTVICT );
            act( AT_PLAIN, "$n takes a scanner and begins to scan you for bugs.", ch, NULL, victim, TO_VICT );
            add_timer( ch, TIMER_DO_FUN, 10, do_scanbugs, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "You punch random buttons on the scanner, unsure of what you are doing.\n\r", ch );
         learn_from_failure( ch, gsn_scanbugs );
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
         send_to_char( "&RYou are interupted and fail to finish scanning.\n\r", ch );
         return;

   }

   ch->substate = SUB_NONE;

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_scanbugs] );

   if( number_percent(  ) > schance * 2 + 20 )
   {
      send_to_char( "&w&RYou struggle with the scanner, furious in your ignorance of its abilities.\n\r", ch );
      learn_from_failure( ch, gsn_scanbugs );
      return;
   }

   victim = get_char_room( ch, arg );

   if( !victim )
   {
      send_to_char( "Your victim left before you finished scanning!\n\r", ch );
      return;
   }

   i = 0;
   for( bugs = victim->first_bug; bugs; bugs = bugs->next_in_bug )
      i++;

   if( i >= 1 )
      ch_printf( ch, "&w&GScan Complete. %d bugs apparent.\n\r", i );
   else
      send_to_char( "&w&GScan Complete. No bugs apparent.\n\r", ch );

   learn_from_success( ch, gsn_scanbugs );

   {
      long xpgain;

      xpgain = ( ch->experience[TECHNICIAN_ABILITY] / 30 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, "You gain %d technician experience.", xpgain );
   }

   return;
}

void do_removebug( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_STRING_LENGTH];
   int schance;
   BUG_DATA *bugs;

   argument = one_argument( argument, arg );

   switch ( ch->substate )
   {
      default:

         if( arg[0] == '\0' )
         {
            send_to_char( "Syntax: removebug <person>\n\r", ch );
            return;
         }


         victim = get_char_room( ch, arg );

         if( !victim )
         {
            send_to_char( "They aren't here.\n\r", ch );
            return;
         }

         if( IS_NPC( victim ) )
         {
            send_to_char( "You can only do that to players.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_scanbugs] );

         if( number_percent(  ) < schance + 20 )
         {
            send_to_char( "&w&GYou begin to pick at a bug, trying to remove it without notifying the owner...\n\r", ch );
            act( AT_PLAIN, "$n takes $s toolkit and begins removing a bug from $N.", ch, NULL, victim, TO_NOTVICT );
            act( AT_PLAIN, "$n takes $s toolkit and begins removing a bug from you.", ch, NULL, victim, TO_VICT );
            add_timer( ch, TIMER_DO_FUN, 1, do_removebug, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "You look curiously at the bug, unsure of how to remove it.\n\r", ch );
         learn_from_failure( ch, gsn_removebug );
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
         send_to_char( "&RYou are interrupted and fail to finish removing the bug.\n\r", ch );
         return;

   }

   ch->substate = SUB_NONE;

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_scanbugs] );


   victim = get_char_room( ch, arg );

   if( !victim )
   {
      send_to_char( "Your victim left before you finished removing the bug!\n\r", ch );
      return;
   }

   if( number_percent(  ) > schance * 2 || !victim->first_bug )
   {
      send_to_char( "&w&RYou fiddle with the bug, and suddenly a small beeper goes off...\n\r", ch );
      send_to_char( "&w&RIt appears you've failed.\n\r", ch );
      learn_from_failure( ch, gsn_scanbugs );
      return;
   }

   if( victim->first_bug )
      bugs = victim->first_bug;

   UNLINK( bugs, victim->first_bug, victim->last_bug, next_in_bug, prev_in_bug );
   STRFREE( bugs->name );
   DISPOSE( bugs );
   send_to_char( "&w&GWith a satisfying *click*, you detach the bug and smash it.\n\r", ch );
   learn_from_success( ch, gsn_removebug );
   {
      long xpgain;

      xpgain = ( ch->experience[TECHNICIAN_ABILITY] / 25 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, "You gain %d technician experience.", xpgain );
   }

   return;
}
