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

BMARKET_DATA *first_market_ship;
BMARKET_DATA *last_market_ship;

typedef enum
{
   SP_NOT_SET, SP_COCKPIT, SP_ENTRANCE, SP_HANGER1, SP_HANGER2, SP_HANGER3, SP_HANGER4, SP_ENGINEROOM,
   SP_PILOT, SP_COPILOT, SP_NAVIGATOR, SP_GUNNER, SP_TURRET1, SP_TURRET2, SP_TURRET3, SP_TURRET4, SP_TURRET5,
   SP_TURRET6, SP_TURRET7, SP_TURRET8, SP_TURRET9, SP_TURRET10, SP_OTHER
} ship_proto_room_types;

struct ship_prototypes_struct
{
   char *name;
   char *sname;
   char *clan;
   int num_rooms;
   int cost;
   int class;
   int tractor;
   int primaryType;
   int secondaryType;
   int primaryCount;
   int secondaryCount;
   int range_weapons[3];
   int hull;
   int shields;
   int energy;
   int chaff;
   int maxbombs;
   int speed;
   int hyperspeed;
   int manuever;
   int turrets;
   int mods;
   //int         plasma;
};

struct ship_prototypes_struct ship_prototypes[256];
int NUM_PROTOTYPES;


typedef struct prototype_room PROTO_ROOM;

struct prototype_room
{
   int what_prototype;
   PROTO_ROOM *next;
   PROTO_ROOM *prev;
   char *name;
   char *desc;
   char *flags;
   int tunnel;
   int exits[10];
   int exflags[10];
   int keys[10];
   int room_num;
   int room_type;
   char *rprog[10];
   char *reset[10];
};

PROTO_ROOM *first_prototype_room;
PROTO_ROOM *last_prototype_room;

void instaroom( AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, bool dodoors );
void shiplist( CHAR_DATA * ch );
char *primary_beam_name_proto( int shiptype );
char *secondary_beam_name_proto( int shiptype );

void do_buymobship( CHAR_DATA * ch, char *argument )
{
   int x, size, ship_type, vnum, count, caps;
   SHIP_DATA *ship;
   SHIP_DATA *sship;
   char arg[MAX_STRING_LENGTH];
   char shipname[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   bool found_proto = FALSE;
   AREA_DATA *tarea;
   CLAN_DATA *clan;
   CLAN_DATA *mainclan;
   SPACE_DATA *ssystem;
   PLANET_DATA *planet;
   bool fsys, fplan, fcap;

   argument = one_argument( argument, arg );

   if( !arg || !argument )
   {
      send_to_char( "Usage: buymobship <ship type> <starsystem to be sent to>\n\r", ch );
      return;
   }
   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "&ROnly players can do that!\r\n", ch );
      return;
   }
   if( !ch->pcdata->clan )
   {
      send_to_char( "You do not belong to any organization.\r\n", ch );
      return;
   }
   clan = ch->pcdata->clan;
   mainclan = ch->pcdata->clan->mainclan ? ch->pcdata->clan->mainclan : clan;

   if( ( ch->pcdata->bestowments
         && is_name( "clanbuyship", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
      ;
   else
   {
      send_to_char( "Your organization hasn't seen fit to bestow you with that ability.\r\n", ch );
      return;
   }

   if( arg == NULL || !arg || arg[0] == '\0' )
   {
      send_to_char( "\n\r&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w                               Available ships                               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );

      for( x = 0; x < NUM_PROTOTYPES; x++ )
      {
         if( !ch->pcdata->clan && ( !str_cmp( ship_prototypes[x].clan, "The Empire" )
                                    || !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name )
             && str_cmp( ship_prototypes[x].clan, "" ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name ) )
            continue;
         if( ship_prototypes[x].class != 0 && ship_prototypes[x].class != 1 && ship_prototypes[x].class != 2 )
            continue;

         ch_printf( ch, "&W|&w %s%-39.39s&W Type:  &w%-13.13s &WCost:&w %8d &W|\r\n",
                    !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&R" :
                    !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&G" : "&w",
                    ship_prototypes[x].name,
                    ship_prototypes[x].class == 0 ? "Fighter" :
                    ship_prototypes[x].class == 1 ? "Bomber" :
                    ship_prototypes[x].class == 2 ? "Midtarget" : "Spacecraft", ship_prototypes[x].cost );
      }

      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w               Refer to 'shiplist' for a complete ship listing               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\n\r", ch );
      return;
   }

   size = NUM_PROTOTYPES;
   for( x = 0; x < size; x++ )
   {
      if( nifty_is_name_prefix( arg, ship_prototypes[x].name ) )
      {
         found_proto = TRUE;
         break;
      }
   }
   if( !found_proto )
   {
      send_to_char( "\n\r&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w                               Available ships                               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );

      for( x = 0; x < NUM_PROTOTYPES; x++ )
      {
         if( !ch->pcdata->clan && ( !str_cmp( ship_prototypes[x].clan, "The Empire" )
                                    || !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name )
             && str_cmp( ship_prototypes[x].clan, "" ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name ) )
            continue;
         if( ship_prototypes[x].class != 0 && ship_prototypes[x].class != 1 && ship_prototypes[x].class != 2 )
            continue;

         ch_printf( ch, "&W|&w %s%-39.39s&W Type:  &w%-13.13s &WCost:&w %8d &W|\r\n",
                    !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&R" :
                    !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&G" : "&w",
                    ship_prototypes[x].name,
                    ship_prototypes[x].class == 0 ? "Fighter" :
                    ship_prototypes[x].class == 1 ? "Bomber" :
                    ship_prototypes[x].class == 2 ? "Midtarget" : "Spacecraft", ship_prototypes[x].cost );
      }
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w               Refer to 'shiplist' for a complete ship listing               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\n\r", ch );
      return;
   }
   ship_type = x;

   // Modify later. -||

   if( ( clan->clan_type == 0 ) && str_cmp( ship_prototypes[ship_type].clan, ch->pcdata->clan->name ) )
   {
      send_to_char( "Your organization may only purchase its own ship models.\n\r", ch );
      return;
   }
   if( ship_prototypes[x].class != 0 && ship_prototypes[x].class != 1 && ship_prototypes[x].class != 2 )
   {
      send_to_char( "You may only purchase fighters, bombers, or midtargets for mobile ships.\n\r", ch );
      return;
   }
   vnum = find_vnum_block( ship_prototypes[ship_type].num_rooms );
   if( vnum == -1 )
   {
      send_to_char( "There was a problem with your ship: free vnums. Notify an administrator.\r\n", ch );
      bug( "Ship area is low on vnums.", 0 );
      return;
   }

   switch ( ship_type )
   {
      default:
         sprintf( shipname, "Mobile Ship MS" );
         break;
         // NR
      case 0:
         sprintf( shipname, "X-Wing Snubfighter MXW" );
         break;
      case 1:
         sprintf( shipname, "A-Wing Scout MAW" );
         break;
      case 2:
         sprintf( shipname, "B-Wing Heavy Fighter MBW" );
         break;
      case 3:
         sprintf( shipname, "Y-Wing Bomber MYB" );
         break;
      case 4:
         sprintf( shipname, "K-Wing Heavy Bomber MKW" );
         break;
         // Imp
      case 6:
         sprintf( shipname, "TIE Fighter MTF" );
         break;
      case 7:
         sprintf( shipname, "TIE Bomber MTB" );
         break;
      case 8:
         sprintf( shipname, "TIE Defender MTD" );
         break;
      case 9:
         sprintf( shipname, "XM-1 Missileboat MXM" );
         break;
      case 10:
         sprintf( shipname, "XG-1 Assault Gunboat MXG" );
         break;
   }

   sprintf( shipname, "%s%d (%s)", shipname, number_range( 1111, 9999 ), ship_prototypes[ship_type].sname );

   if( ch->pcdata->clan->funds < ship_prototypes[ship_type].cost )
   {
      send_to_char( "Your organization cannot pay for that ship.\r\n", ch );
      return;
   }
   clan = ch->pcdata->clan;
   mainclan = ch->pcdata->clan->mainclan ? ch->pcdata->clan->mainclan : clan;

   if( ( ch->pcdata->bestowments
         && is_name( "clanbuyship", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
      ;
   else
   {
      send_to_char( "Your organization hasn't seen fit to bestow you with that ability.\r\n", ch );
      return;
   }

   fsys = fplan = fcap = FALSE;
   for( ssystem = first_starsystem; ssystem; ssystem = ssystem->next )
   {
      if( nifty_is_name( argument, ssystem->name ) )
      {
         fsys = TRUE;
         break;
      }
   }
   if( !fsys )
   {
      send_to_char( "No such starsystem.\n\r", ch );
      return;
   }
   for( planet = ssystem->first_planet; planet; planet = planet->next_in_system )
   {
      if( ch->pcdata->clan == planet->governed_by )
         fplan = TRUE;
   }
   if( !fplan )
   {
      send_to_char( "Your organization does not control any planets in that starsystem.\n\r", ch );
      return;
   }
   count = 0;
   for( sship = ssystem->first_ship; sship; sship = sship->next_in_starsystem )
   {
      if( sship->type == MOB_SHIP )
         count++;
      if( sship->class >= SHIP_CRUISER )
      {
         fcap = TRUE;
         caps++;
      }
   }
   if( !fcap )
   {
      send_to_char( "There isn't a capital class ship in that starsystem.\n\r", ch );
      return;
   }
   if( count > 3 * caps )
   {
      send_to_char( "You can only have 3 mobile ships per capital-class ship in a system.\n\r", ch );
      return;
   }
   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( SHIP_AREA, tarea->filename ) )
         break;
   if( make_prototype_rooms( ship_type, vnum, tarea, shipname ) == -1 )
   {
      send_to_char( "There was a problem with your ship: unable to create a room. Notify an administrator.\r\n", ch );
      bug( "Ship area unable to make_room.", 0 );
      return;
   }

   ch->pcdata->clan->funds -= ship_prototypes[ship_type].cost * 1.3;

   ch_printf( ch, "It costs %d to build the ship and %d to train a pilot.\n\r",
              ship_prototypes[ship_type].cost, ship_prototypes[ship_type].cost / 3 );
   ch_printf( ch, "%s is quickly dispatched to the %s system.\n\r", shipname, ssystem->name );

   ship = make_prototype_ship( ship_type, vnum, ch, shipname );
   ship->owner = STRALLOC( ch->pcdata->clan->name );
   save_ship( ship );
   write_ship_list(  );
   extract_ship( ship );
   ship_to_starsystem( ship, ssystem );
   ship->location = 0;
   ship->inship = NULL;
   ship->type = MOB_SHIP;
   if( ship->home )
      STRFREE( ship->home );
   ship->home = STRALLOC( ssystem->name );
   ship->hx = ship->hy = ship->hz = 1;
   ship->vx = number_range( -3000, 3000 );
   ship->vy = number_range( -3000, 3000 );
   ship->vz = number_range( -3000, 3000 );
   ship->autopilot = TRUE;
   sprintf( buf, "%s enters the starsystem at %.0f %.0f %.0f", ship->name, ship->vx, ship->vy, ship->vz );
   echo_to_system( AT_YELLOW, ship, buf, NULL );
   return;
}

void do_orderclanship( CHAR_DATA * ch, char *argument )
{
   int x, size, ship_type, vnum;
   SHIP_DATA *ship;
   char arg[MAX_STRING_LENGTH];
   bool found_proto = FALSE;
   AREA_DATA *tarea;
   CLAN_DATA *clan;
   CLAN_DATA *mainclan;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "&ROnly players can do that!\r\n", ch );
      return;
   }
   if( !ch->pcdata->clan )
   {
      send_to_char( "You do not belong to any organization.\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;
   mainclan = ch->pcdata->clan->mainclan ? ch->pcdata->clan->mainclan : clan;

   if( ( ch->pcdata->bestowments
         && is_name( "clanbuyship", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
      ;
   else
   {
      send_to_char( "Your organization hasn't seen fit to bestow you with that ability.\r\n", ch );
      return;
   }
   if( !IS_SET( ch->in_room->room_flags2, ROOM_SHIPYARD ) )
   {
      send_to_char( "You can only purchase a ship at a shipyard.\r\n", ch );
      return;
   }
   if( argument == NULL || !argument || argument[0] == '\0' )
   {
      send_to_char( "\n\r&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w                               Available ships                               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );

      for( x = 0; x < NUM_PROTOTYPES; x++ )
      {
         if( !ch->pcdata->clan && ( !str_cmp( ship_prototypes[x].clan, "The Empire" )
                                    || !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name )
             && str_cmp( ship_prototypes[x].clan, "" ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name ) )
            continue;

         ch_printf( ch, "&W|&w %s%-39.39s&W Type:  &w%-13.13s &WCost:&w %8d &W|\r\n",
                    !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&R" :
                    !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&G" : "&w",
                    ship_prototypes[x].name,
                    ship_prototypes[x].class == SHIP_FIGHTER ? "Starfighter" :
                    ship_prototypes[x].class == SHIP_BOMBER ? "Bomber" :
                    ship_prototypes[x].class == SHIP_SHUTTLE ? "Shuttle" :
                    ship_prototypes[x].class == SHIP_FREIGHTER ? "Freighter" :
                    ship_prototypes[x].class == SHIP_FRIGATE ? "Frigate" :
                    ship_prototypes[x].class == SHIP_TT ? "Troop Transport" :
                    ship_prototypes[x].class == SHIP_CORVETTE ? "Corvette" :
                    ship_prototypes[x].class == SHIP_CRUISER ? "Cruiser" :
                    ship_prototypes[x].class == SHIP_DREADNAUGHT ? "Dreadnaught" :
                    ship_prototypes[x].class == SHIP_DESTROYER ? "Star Destroyer" :
                    ship_prototypes[x].class == SHIP_SPACE_STATION ? "Space Station" :
                    ship_prototypes[x].class == LAND_VEHICLE ? "Land vehicle" : "Unknown", ship_prototypes[x].cost );
      }

      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w               Refer to 'shiplist' for a complete ship listing               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg );
   size = NUM_PROTOTYPES;
   for( x = 0; x < size; x++ )
   {
      if( nifty_is_name_prefix( arg, ship_prototypes[x].name ) )
      {
         found_proto = TRUE;
         break;
      }
   }
   if( !found_proto )
   {
      send_to_char( "\n\r&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w                               Available ships                               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );

      for( x = 0; x < NUM_PROTOTYPES; x++ )
      {
         if( !ch->pcdata->clan && ( !str_cmp( ship_prototypes[x].clan, "The Empire" )
                                    || !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name )
             && str_cmp( ship_prototypes[x].clan, "" ) )
            continue;
         if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name ) )
            continue;

         ch_printf( ch, "&W|&w %s%-39.39s&W Type:  &w%-13.13s &WCost:&w %8d &W|\r\n",
                    !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&R" :
                    !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&G" : "&w",
                    ship_prototypes[x].name,
                    ship_prototypes[x].class == SHIP_FIGHTER ? "Starfighter" :
                    ship_prototypes[x].class == SHIP_BOMBER ? "Bomber" :
                    ship_prototypes[x].class == SHIP_SHUTTLE ? "Shuttle" :
                    ship_prototypes[x].class == SHIP_FREIGHTER ? "Freighter" :
                    ship_prototypes[x].class == SHIP_FRIGATE ? "Frigate" :
                    ship_prototypes[x].class == SHIP_TT ? "Troop Transport" :
                    ship_prototypes[x].class == SHIP_CORVETTE ? "Corvette" :
                    ship_prototypes[x].class == SHIP_CRUISER ? "Cruiser" :
                    ship_prototypes[x].class == SHIP_DREADNAUGHT ? "Dreadnaught" :
                    ship_prototypes[x].class == SHIP_DESTROYER ? "Star Destroyer" :
                    ship_prototypes[x].class == SHIP_SPACE_STATION ? "Space Station" :
                    ship_prototypes[x].class == LAND_VEHICLE ? "Land vehicle" : "Unknown", ship_prototypes[x].cost );
      }
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
      send_to_char( "&W|&w               Refer to 'shiplist' for a complete ship listing               &W|\r\n", ch );
      send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\n\r", ch );
      return;
   }
   ship_type = x;
   if( argument[0] == '\0' || argument == NULL || !argument )
   {
      send_to_char( "What would you like to name your ship?\r\n", ch );
      return;
   }
   // Modify later. -||

   if( ( clan->clan_type == 0 ) && str_cmp( ship_prototypes[ship_type].clan, ch->pcdata->clan->name ) )
   {
      send_to_char( "Your organization may only purchase its own ship models.\n\r", ch );
      return;
   }
   vnum = find_vnum_block( ship_prototypes[ship_type].num_rooms );
   if( vnum == -1 )
   {
      send_to_char( "There was a problem with your ship: free vnums. Notify an administrator.\r\n", ch );
      bug( "Ship area is low on vnums.", 0 );
      return;
   }
   argument[0] = UPPER( argument[0] );
   sprintf( argument, "%s (%s)", argument, ship_prototypes[ship_type].sname );
   for( ship = first_ship; ship; ship = ship->next )
   {
      if( !str_cmp( ship->name, argument ) )
      {
         send_to_char( "That ship name is already in use. Choose another.\r\n", ch );
         return;
      }
   }
   if( ch->pcdata->clan->funds < ship_prototypes[ship_type].cost )
   {
      send_to_char( "Your organization cannot pay for that ship.\r\n", ch );
      return;
   }
   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( SHIP_AREA, tarea->filename ) )
         break;
   if( make_prototype_rooms( ship_type, vnum, tarea, argument ) == -1 )
   {
      send_to_char( "There was a problem with your ship: unable to create a room. Notify an administrator.\r\n", ch );
      bug( "Ship area unable to make_room.", 0 );
      return;
   }

   ch->pcdata->clan->funds -= ship_prototypes[ship_type].cost;

   send_to_char( "&wA shipyard salesman enters some information into a datapad.\r\n", ch );
   send_to_char( "&R&CThe salesman says:&R&W You're all set. Thanks.\n\r", ch );
   ship = make_prototype_ship( ship_type, vnum, ch, argument );
   ship->owner = ch->pcdata->clan->name;
   save_ship( ship );
   write_ship_list(  );
   return;
}

void do_ordership( CHAR_DATA * ch, char *argument )
{
   int x, size, ship_type, vnum, count;
   SHIP_DATA *ship;
   char arg[MAX_STRING_LENGTH];
   bool found_proto = FALSE;
   AREA_DATA *tarea;
   BMARKET_DATA *marketship;
   char *bmshipname;
   int bmshipcost;

   count = 0;
   if( !IS_SET( ch->in_room->room_flags2, ROOM_SHIPYARD ) && !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
   {
      send_to_char( "You can only purchase ships at a shipyard.\r\n", ch );
      return;
   }
   if( !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
   {
      if( argument == NULL || !argument || argument[0] == '\0' )
      {
         send_to_char( "\n\r&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
         send_to_char( "&W|&w                               Available ships                               &W|\r\n", ch );
         send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );

         for( x = 0; x < NUM_PROTOTYPES; x++ )
         {
            if( !ch->pcdata->clan && ( !str_cmp( ship_prototypes[x].clan, "The Empire" )
                                       || !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ) )
               continue;
            if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name )
                && str_cmp( ship_prototypes[x].clan, "" ) )
               continue;

            ch_printf( ch, "&W|&w %s%-39.39s&W Type:  &w%-13.13s &WCost:&w %8d &W|\r\n",
                       !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&R" :
                       !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&G" : "&w",
                       ship_prototypes[x].name,
                       ship_prototypes[x].class == SHIP_FIGHTER ? "Starfighter" :
                       ship_prototypes[x].class == SHIP_BOMBER ? "Bomber" :
                       ship_prototypes[x].class == SHIP_SHUTTLE ? "Shuttle" :
                       ship_prototypes[x].class == SHIP_FREIGHTER ? "Freighter" :
                       ship_prototypes[x].class == SHIP_FRIGATE ? "Frigate" :
                       ship_prototypes[x].class == SHIP_TT ? "Troop Transport" :
                       ship_prototypes[x].class == SHIP_CORVETTE ? "Corvette" :
                       ship_prototypes[x].class == SHIP_CRUISER ? "Cruiser" :
                       ship_prototypes[x].class == SHIP_DREADNAUGHT ? "Dreadnaught" :
                       ship_prototypes[x].class == SHIP_DESTROYER ? "Star Destroyer" :
                       ship_prototypes[x].class == SHIP_SPACE_STATION ? "Space Station" :
                       ship_prototypes[x].class == LAND_VEHICLE ? "Land vehicle" : "Unknown", ship_prototypes[x].cost );

         }
         send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
         send_to_char( "&W|&w               Refer to 'shiplist' for a complete ship listing               &W|\r\n", ch );
         send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\n\r", ch );
         return;
      }
   }
   else
   {
      if( argument == NULL || !argument || argument[0] == '\0' )
      {
         ch_printf( ch, "&z+&W------------------------&z The Black Market &W--------------------------&z+\n\r" );
         for( marketship = first_market_ship; marketship; marketship = marketship->next )
         {
            for( x = 0; x < NUM_PROTOTYPES; x++ )
            {
               if( !str_cmp( marketship->filename, ship_prototypes[x].sname ) )
               {
                  bmshipname = ship_prototypes[x].name;
                  bmshipcost = ship_prototypes[x].cost * 3;
                  break;
               }
            }

            ch_printf( ch, "&W|&z %-35.35s&W |&z Quantity: %d&W |&z Cost: %-8d&W |\n\r", bmshipname, marketship->quantity,
                       bmshipcost );
            count++;
         }
         if( count == 0 )
            ch_printf( ch, "&W|&z                         No ships available.                        &W|\n\r" );
         ch_printf( ch, "&z+&W--------------------------------------------------------------------&z+\n\r" );
         return;
      }
   }

   argument = one_argument( argument, arg );
   size = NUM_PROTOTYPES;
   for( x = 0; x < size; x++ )
   {
      if( !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
      {
         if( nifty_is_name_prefix( arg, ship_prototypes[x].name ) )
         {
            found_proto = TRUE;
            break;
         }
      }
      else
      {
         for( marketship = first_market_ship; marketship; marketship = marketship->next )
         {
            if( nifty_is_name_prefix( arg, ship_prototypes[x].name )
                && !str_cmp( ship_prototypes[x].sname, marketship->filename ) )
            {
               found_proto = 1;
               break;
            }
         }
         if( found_proto == 1 )
            break;
      }
   }

   if( !found_proto )
   {
      if( !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
      {
         send_to_char( "\n\r&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
         send_to_char( "&W|&w                               Available ships                               &W|\r\n", ch );
         send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );

         for( x = 0; x < NUM_PROTOTYPES; x++ )
         {
            if( !ch->pcdata->clan && ( !str_cmp( ship_prototypes[x].clan, "The Empire" )
                                       || !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ) )
               continue;
            if( ch->pcdata->clan && str_cmp( ship_prototypes[x].clan, ch->pcdata->clan->name )
                && str_cmp( ship_prototypes[x].clan, "" ) )
               continue;

            ch_printf( ch, "&W|&w %s%-39.39s&W Type:  &w%-13.13s &WCost:&w %8d &W|\r\n",
                       !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&R" :
                       !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&G" : "&w",
                       ship_prototypes[x].name,
                       ship_prototypes[x].class == SHIP_FIGHTER ? "Starfighter" :
                       ship_prototypes[x].class == SHIP_BOMBER ? "Bomber" :
                       ship_prototypes[x].class == SHIP_SHUTTLE ? "Shuttle" :
                       ship_prototypes[x].class == SHIP_FREIGHTER ? "Freighter" :
                       ship_prototypes[x].class == SHIP_FRIGATE ? "Frigate" :
                       ship_prototypes[x].class == SHIP_TT ? "Troop Transport" :
                       ship_prototypes[x].class == SHIP_CORVETTE ? "Corvette" :
                       ship_prototypes[x].class == SHIP_CRUISER ? "Cruiser" :
                       ship_prototypes[x].class == SHIP_DREADNAUGHT ? "Dreadnaught" :
                       ship_prototypes[x].class == SHIP_DESTROYER ? "Star Destroyer" :
                       ship_prototypes[x].class == SHIP_SPACE_STATION ? "Space Station" :
                       ship_prototypes[x].class == LAND_VEHICLE ? "Land vehicle" : "Unknown", ship_prototypes[x].cost );

         }
         send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
         send_to_char( "&W|&w               Refer to 'shiplist' for a complete ship listing               &W|\r\n", ch );
         send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\n\r", ch );
         return;
      }
      else
      {
         ch_printf( ch, "&z+&W------------------------&z The Black Market &W--------------------------&z+\n\r" );
         for( marketship = first_market_ship; marketship; marketship = marketship->next )
         {
            for( x = 0; x < NUM_PROTOTYPES; x++ )
            {
               if( !str_cmp( marketship->filename, ship_prototypes[x].sname ) )
               {
                  bmshipname = ship_prototypes[x].name;
                  if( marketship->quantity == 1 )
                     bmshipcost = ship_prototypes[x].cost * 2.5;
                  else if( marketship->quantity == 2 )
                     bmshipcost = ship_prototypes[x].cost * 2;
                  else
                     bmshipcost = ship_prototypes[x].cost * 1.5;
               }
            }
            ch_printf( ch, "&W|&z %-35.35s&W |&z Quantity: %d&W |&z Cost: %-8d&W |\n\r", bmshipname, marketship->quantity,
                       bmshipcost );
            count++;
         }
         if( count == 0 )
            ch_printf( ch, "&W|&z                         No ships available.                        &W|\n\r" );
         ch_printf( ch, "&z+&W--------------------------------------------------------------------&z+\n\r" );
         return;
      }
   }

   ship_type = x;
   if( argument[0] == '\0' || argument == NULL || !argument )
   {
      send_to_char( "What do you want to name your ship?\r\n", ch );
      return;
   }
   if( ship_prototypes[ship_type].class == 3 && !IS_IMMORTAL( ch ) )
   {
      send_to_char( "At the moment, capital ships may only be purchased by clans.\n\r", ch );
      return;
   }

   if( !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
   {
      if( !str_cmp( ship_prototypes[ship_type].clan, "The Empire" ) ||
          !str_cmp( ship_prototypes[ship_type].clan, "The New Republic" ) )
      {
         if( !ch->pcdata->clan )
         {
            send_to_char( "You must find a black market to purchase clanned ships.\n\r", ch );
            return;
         }
         else if( str_cmp( ch->pcdata->clan->name, ship_prototypes[ship_type].clan ) )
         {
            send_to_char( "You must find a black market to purchase other clan's ships.\n\r", ch );
            return;
         }
      }
   }

   vnum = find_vnum_block( ship_prototypes[ship_type].num_rooms );
   if( vnum == -1 )
   {
      send_to_char( "There was a problem with your ship: free vnums. Notify an administrator.\r\n", ch );
      bug( "Ship Shop area is low on vnums.", 0 );
      return;
   }

   argument[0] = UPPER( argument[0] );

   strcpy( log_buf, ship_prototypes[ship_type].sname );
//    sprintf(argument,"%s (%s)",argument, ship_prototypes[ship_type].sname);
   for( ship = first_ship; ship; ship = ship->next )
   {
//        if(!str_cmp(ship->name,argument))
      if( !str_cmp( ship->name, log_buf ) )
      {
         send_to_char( "&CThat ship name is already in use. Choose another.\r\n", ch );
         return;
      }
   }
   if( !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
   {
      if( ch->gold < ship_prototypes[ship_type].cost )
      {
         send_to_char( "You can't afford that ship.\r\n", ch );
         return;
      }
   }
   else
   {
      if( ch->gold < ship_prototypes[ship_type].cost * 3 )
      {
         send_to_char( "You can't afford that ship.\n\r", ch );
         return;
      }
   }

   if( !str_cmp( argument, "&" ) )
   {
      send_to_char( "No color codes in ship names.\n\r", ch );
      return;
   }

   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( SHIP_AREA, tarea->filename ) )
         break;
   if( make_prototype_rooms( ship_type, vnum, tarea, argument ) == -1 )
   {
      send_to_char( "There was a problem with your ship: unable to create a room. Notify an administrator.\n\r", ch );
      bug( "Ship Shop unable to make_room.", 0 );
      return;
   }

   if( !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
   {
      ch->gold -= ship_prototypes[ship_type].cost;
      send_to_char( "A shipyard salesman enters some information into a datapad.\r\n", ch );
      send_to_char( "&R&CA salesman says:&R&W You're all set. Thanks.\n\r", ch );
   }
   else
   {
      ch->gold -= ship_prototypes[ship_type].cost * 3;

      for( marketship = first_market_ship; marketship; marketship = marketship->next )
      {
         if( !str_cmp( ship_prototypes[ship_type].sname, marketship->filename ) )
         {
            remove_market_ship( marketship );
            break;
         }
      }

      send_to_char( "A smuggler gives a grin and shows you to the ship.\n\r", ch );
      send_to_char( "&R&CA smuggler says:&R&W There ya go, she's all yours.\n\r", ch );
   }
   ship = make_prototype_ship( ship_type, vnum, ch, argument );
   return;
}

SHIP_DATA *make_prototype_ship( int ship_type, int vnum, CHAR_DATA * ch, char *ship_name )
{
   SHIP_DATA *ship;
   PROTO_ROOM *proom;
   ROOM_INDEX_DATA *room;
   char sp_filename[MAX_STRING_LENGTH];
   CREATE( ship, SHIP_DATA, 1 );
   LINK( ship, first_ship, last_ship, next, prev );
   ship->name = STRALLOC( ship_name );
   ship->owner = STRALLOC( "" );
   ship->protoname = STRALLOC( ship_prototypes[ship_type].sname );
   ship->clanowner = STRALLOC( ship_prototypes[ship_type].clan );
   ship->description = STRALLOC( "" );
   ship->copilot = STRALLOC( "" );
   ship->pilot = STRALLOC( "" );
   if( ch->in_room->area && ch->in_room->area->planet )
      ship->home = STRALLOC( ch->in_room->area->planet->name );
   else
      ship->home = STRALLOC( "" );
   ship->pbeacon = STRALLOC( "" );
   ship->type = PLAYER_SHIP;
   ship->class = ship_prototypes[ship_type].class;
   ship->cost = ship_prototypes[ship_type].cost;
   ship->starsystem = NULL;
   ship->in_room = NULL;
   ship->next_in_room = NULL;
   ship->prev_in_room = NULL;
   ship->currjump = NULL;
   ship->target0 = NULL;
   ship->target1 = NULL;
   ship->target2 = NULL;
   ship->target3 = NULL;
   ship->target4 = NULL;
   ship->target5 = NULL;
   ship->target6 = NULL;
   ship->target7 = NULL;
   ship->target8 = NULL;
   ship->target9 = NULL;
   ship->target10 = NULL;
   ship->password = number_range( 1111, 9999 );;
   ship->maxmods = ship_prototypes[ship_type].mods;
   sprintf( sp_filename, "ship_%d.sss", vnum );
   ship->filename = str_dup( sp_filename );
   ship->pilotseat = vnum;
   ship->coseat = vnum;
   ship->navseat = vnum;
   ship->gunseat = vnum;
   ship->entrance = vnum;
   ship->engineroom = vnum;
   for( proom = first_prototype_room; proom; proom = proom->next )
   {
      if( proom->what_prototype == ship_type )
      {
         if( proom->room_num == 1 )
            ship->firstroom = vnum;
         if( proom->room_num == ship_prototypes[ship_type].num_rooms )
            ship->lastroom = proom->room_num + vnum - 1;
      }
   }
   for( proom = first_prototype_room; proom; proom = proom->next )
   {
      if( proom->what_prototype == ship_type )
      {
         room = get_room_index( proom->room_num + vnum - 1 );
         switch ( proom->room_type )
         {
            case SP_COCKPIT:
               ship->cockpit = proom->room_num + vnum - 1;
               break;
            case SP_ENTRANCE:
               ship->entrance = proom->room_num + vnum - 1;
               break;
            case SP_HANGER1:
               ship->hanger1 = proom->room_num + vnum - 1;
               break;
            case SP_HANGER2:
               ship->hanger2 = proom->room_num + vnum - 1;
               break;
            case SP_HANGER3:
               ship->hanger3 = proom->room_num + vnum - 1;
               break;
            case SP_HANGER4:
               ship->hanger4 = proom->room_num + vnum - 1;
               break;
            case SP_ENGINEROOM:
               ship->engineroom = proom->room_num + vnum - 1;
               break;
            case SP_PILOT:
               ship->pilotseat = proom->room_num + vnum - 1;
               break;
            case SP_COPILOT:
               ship->coseat = proom->room_num + vnum - 1;
               break;
            case SP_NAVIGATOR:
               ship->navseat = proom->room_num + vnum - 1;
               break;
            case SP_GUNNER:
               ship->gunseat = proom->room_num + vnum - 1;
               break;
            case SP_TURRET1:
               ship->turret1 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET2:
               ship->turret2 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET3:
               ship->turret3 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET4:
               ship->turret4 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET5:
               ship->turret5 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET6:
               ship->turret6 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET7:
               ship->turret7 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET8:
               ship->turret8 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET9:
               ship->turret9 = proom->room_num + vnum - 1;
               break;
            case SP_TURRET10:
               ship->turret10 = proom->room_num + vnum - 1;
               break;
            default:
               break;
         }
      }
   }
   ship->tractorbeam = ship_prototypes[ship_type].tractor;
   ship->primaryType = ship_prototypes[ship_type].primaryType;
   ship->secondaryType = ship_prototypes[ship_type].secondaryType;
   ship->primaryCount = ship_prototypes[ship_type].primaryCount;
   ship->secondaryCount = ship_prototypes[ship_type].secondaryCount;
   ship->missiles = ship_prototypes[ship_type].range_weapons[0];
   ship->torpedos = ship_prototypes[ship_type].range_weapons[1];
   ship->rockets = ship_prototypes[ship_type].range_weapons[2];
   ship->maxmissiles = ship_prototypes[ship_type].range_weapons[0];
   ship->maxtorpedos = ship_prototypes[ship_type].range_weapons[1];
   ship->maxrockets = ship_prototypes[ship_type].range_weapons[2];
   ship->maxhull = ship_prototypes[ship_type].hull;
   ship->hull = ship_prototypes[ship_type].hull;
   ship->shield = ship_prototypes[ship_type].shields;
   ship->maxshield = ship_prototypes[ship_type].shields;
   //ship->plasmashield = ship_prototypes[ship_type].plasma;
   //ship->maxplasmashield = ship_prototypes[ship_type].plasma;
   ship->energy = ship_prototypes[ship_type].energy;
   ship->maxenergy = ship_prototypes[ship_type].energy;
   ship->chaff = ship_prototypes[ship_type].chaff;
   ship->maxchaff = ship_prototypes[ship_type].chaff;
   ship->bombs = ship_prototypes[ship_type].maxbombs;
   ship->maxbombs = ship_prototypes[ship_type].maxbombs;
   ship->realspeed = ship_prototypes[ship_type].speed;
   ship->hyperspeed = ship_prototypes[ship_type].hyperspeed;
   ship->manuever = ship_prototypes[ship_type].manuever;
   //MARKER
   ship->shipyard = ch->in_room->vnum;
   resetship( ship );
   ship->owner = STRALLOC( ch->name );
   save_ship( ship );
   write_ship_list(  );
   return ship;
}

int make_prototype_rooms( int ship_type, int vnum, AREA_DATA * tarea, char *Sname )
{
   PROTO_ROOM *proom;
   ROOM_INDEX_DATA *newroom, *rfrom, *rto;
   int value;
   char *arg;
   char arg2[MAX_STRING_LENGTH];
   EXIT_DATA *xit;
   int x, y;
   int *exits;
   int *keys;
   int *exflags;
   char buf[MAX_STRING_LENGTH];
   char newdesc[MAX_STRING_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   CHAR_DATA *victim;
   MOB_INDEX_DATA *pMobIndex;
   int rvnum;
   char *rarg;
   char rtype[MAX_STRING_LENGTH];

   for( proom = first_prototype_room; proom; proom = proom->next )
   {
      if( proom->what_prototype == ship_type )
      {
         if( ( newroom = get_room_index( vnum + proom->room_num - 1 ) ) == NULL )
         {
            newroom = make_room( vnum + proom->room_num - 1, tarea );
            if( !newroom )
               return -1;
         }
         newroom->area = tarea;
         if( proom->room_type == SP_NOT_SET )
         {
            if( newroom->name )
               STRFREE( newroom->name );
            sprintf( buf, "SPARE ROOM FOR: %d", vnum );
            newroom->name = STRALLOC( buf );
            if( newroom->description )
               STRFREE( newroom->description );
            continue;
         }
         strcpy( newdesc, strlinwrp( proom->desc, 60 ) );
         if( newroom->name )
            STRFREE( newroom->name );
         newroom->name = STRALLOC( strrep( proom->name, "$SN$", Sname ) );
         if( newroom->description )
            STRFREE( newroom->description );
         newroom->description = STRALLOC( strrep( newdesc, "$SN$", Sname ) );
         arg = STRALLOC( proom->flags );
         newroom->tunnel = proom->tunnel;
         while( arg[0] != '\0' )
         {
            arg = one_argument( arg, arg2 );
            value = get_sp_rflag( arg2 );
            TOGGLE_BIT( newroom->room_flags, 1 << value );
         }
         for( y = 0; y < 10; y++ )
         {
            if( proom->reset[y] != NULL && proom->reset[y][0] != '\0' )
            {
               rarg = STRALLOC( proom->reset[y] );
               rarg = one_argument( rarg, rtype );
               rvnum = atoi( rarg );
               switch ( UPPER( rtype[0] ) )
               {
                  case 'O':
                     if( ( pObjIndex = get_obj_index( rvnum ) ) == NULL )
                        break;
                     obj = create_object( pObjIndex, 60 );
                     obj = obj_to_room( obj, newroom );
                     break;
                  case 'M':
                     if( ( pMobIndex = get_mob_index( rvnum ) ) == NULL )
                        break;
                     victim = create_mobile( pMobIndex );
                     char_to_room( victim, newroom );
                     break;
                  default:
                     break;
               }
            }
         }
         instaroom( tarea, newroom, TRUE );
      }
   }
   for( proom = first_prototype_room; proom; proom = proom->next )
   {
      if( proom->what_prototype == ship_type )
      {
         exits = proom->exits;
         keys = proom->keys;
         exflags = proom->exflags;
         for( x = 0; x < 10; x++ )
         {
            if( exits[x] > 0 )
            {
               rfrom = get_room_index( vnum + proom->room_num - 1 );
               rto = get_room_index( vnum + exits[x] - 1 );
               xit = make_exit( rfrom, rto, x );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = keys[x] == 0 ? -1 : vnum + keys[x] - 1;
               xit->exit_info = exflags[x];
            }
         }
      }
   }
   make_rprogs( ship_type, vnum );
   fold_area( tarea, tarea->filename, FALSE );
   return 1;
}



int find_vnum_block( int num_needed )
{
   bool counting = FALSE;
   int count = 0;
   AREA_DATA *tarea;
   int lrange;
   int trange;
   int vnum;
   int startvnum = -1;
   ROOM_INDEX_DATA *room;

   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( SHIP_AREA, tarea->filename ) )
         break;
   lrange = tarea->low_r_vnum;
   trange = tarea->hi_r_vnum;
   for( vnum = lrange; vnum <= trange; vnum++ )
   {
//        if ( (room = get_room_index( vnum )) == NULL || !strcmp(room->name,"VNUM_AVAILABLE\0"))
      if( ( room = get_room_index( vnum ) ) == NULL )
      {
         if( !counting )
         {
            counting = TRUE;
            startvnum = vnum;
         }
         count++;
         if( count == num_needed + 1 )
            break;
      }
      else if( counting )
      {
         counting = FALSE;
         count = 0;
         startvnum = -1;
      }
   }
   return startvnum;
}

int get_sp_rflag( char *flag )
{
   int x;

   for( x = 0; x < 32; x++ )
      if( !str_cmp( flag, r_flags[x] ) )
         return x;
   return -1;
}

void make_rprogs( int ship_type, int vnum )
{
   PROTO_ROOM *proom;
   char *argument;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   MPROG_DATA *mprog, *mprg;
   ROOM_INDEX_DATA *room;
   int mptype, x, size;
   for( proom = first_prototype_room; proom; proom = proom->next )
   {
      if( proom->what_prototype == ship_type )
      {
         size = 10;
         for( x = 0; x < size; x++ )
         {
            argument = str_dup( proom->rprog[x] );
            if( argument[0] == '\0' )
               continue;
            argument = one_argument( argument, arg1 );
            argument = one_argument( argument, arg2 );
            sprintf( argument, "%s", parse_prog_string( argument, ship_type, vnum ) );
            room = get_room_index( proom->room_num + vnum - 1 );
            mprog = room->mudprogs;
            mptype = get_mpflag( arg1 );
            if( mprog )
               for( ; mprog->next; mprog = mprog->next );
            CREATE( mprg, MPROG_DATA, 1 );
            if( mprog )
               mprog->next = mprg;
            else
               room->mudprogs = mprg;
            room->progtypes |= ( 1 << mptype );


            if( mptype != -1 )
            {
               mprg->type = 1 << mptype;
               if( mprg->arglist )
                  STRFREE( mprg->arglist );
               mprg->arglist = STRALLOC( arg2 );
            }
            mprg->comlist = STRALLOC( argument );
            mprg->next = NULL;
         }
      }
   }
}

char *parse_prog_string( char *inp, int ship_type, int vnum )
{
   char sch[MAX_STRING_LENGTH];
   char rep[MAX_STRING_LENGTH];
   char newinp[MAX_STRING_LENGTH];
   int x, size;
   strcpy( newinp, inp );
   size = ship_prototypes[ship_type].num_rooms;
   for( x = 0; x < size; x++ )
   {
      sprintf( sch, "$RNUM:%d$", x + 1 );
      sprintf( rep, "%d", x + vnum );
      sprintf( newinp, "%s", strrep( newinp, sch, rep ) );
   }
   return str_dup( newinp );
}

void save_prototype( int prototype )
{
   PROTO_ROOM *proom;
   FILE *fpout;
   char filename[MAX_STRING_LENGTH];
   int x;

   sprintf( filename, "%s%s.proto", SHIP_PROTOTYPE_DIR, ship_prototypes[prototype].sname );
   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      perror( filename );
      return;
   }
   fprintf( fpout, "#HEADER\n\n" );
   fprintf( fpout, "Name           %s~\n", ship_prototypes[prototype].name );
   fprintf( fpout, "Sname          %s~\n", ship_prototypes[prototype].sname );
   fprintf( fpout, "NumRooms       %d\n", ship_prototypes[prototype].num_rooms );
   fprintf( fpout, "Cost           %d\n", ship_prototypes[prototype].cost );
   fprintf( fpout, "Class          %d\n", ship_prototypes[prototype].class );
   fprintf( fpout, "Tractor        %d\n", ship_prototypes[prototype].tractor );
   fprintf( fpout, "primaryType         %d\n", ship_prototypes[prototype].primaryType );
   fprintf( fpout, "secondaryType		 %d\n", ship_prototypes[prototype].secondaryType );
   fprintf( fpout, "primaryCount		 %d\n", ship_prototypes[prototype].primaryCount );
   fprintf( fpout, "secondaryCount		 %d\n", ship_prototypes[prototype].secondaryCount );
   fprintf( fpout, "RangeWeapons0  %d\n", ship_prototypes[prototype].range_weapons[0] );
   fprintf( fpout, "RangeWeapons1  %d\n", ship_prototypes[prototype].range_weapons[1] );
   fprintf( fpout, "RangeWeapons2  %d\n", ship_prototypes[prototype].range_weapons[2] );
   fprintf( fpout, "Hull           %d\n", ship_prototypes[prototype].hull );
   fprintf( fpout, "Shields        %d\n", ship_prototypes[prototype].shields );
   fprintf( fpout, "Energy         %d\n", ship_prototypes[prototype].energy );
   fprintf( fpout, "Chaff          %d\n", ship_prototypes[prototype].chaff );
   fprintf( fpout, "Maxbombs       %d\n", ship_prototypes[prototype].maxbombs );
   fprintf( fpout, "Speed          %d\n", ship_prototypes[prototype].speed );
   fprintf( fpout, "Hyperspeed     %d\n", ship_prototypes[prototype].hyperspeed );
   fprintf( fpout, "Manuever       %d\n", ship_prototypes[prototype].manuever );
   fprintf( fpout, "Turrets        %d\n", ship_prototypes[prototype].turrets );
   fprintf( fpout, "Mods	    %d\n", ship_prototypes[prototype].mods );
   fprintf( fpout, "Clan           %s~\n", ship_prototypes[prototype].clan );
   //fprintf( fpout, "Plasma         %d\n",  ship_prototypes[prototype].plasma );
   fprintf( fpout, "End\n" );
   fprintf( fpout, "\n#ROOMS\n\n" );
   for( proom = first_prototype_room; proom; proom = proom->next )
   {
      if( proom->what_prototype == prototype )
      {
         fprintf( fpout, "#ROOM\n\n" );
         fprintf( fpout, "RoomNum           %d\n", proom->room_num );
         fprintf( fpout, "Name              %s~\n", proom->name );
         fprintf( fpout, "Desc              %s~\n", proom->desc );
         fprintf( fpout, "Flags             %s~\n", proom->flags );
         fprintf( fpout, "Tunnel            %d\n", proom->tunnel );
         for( x = 0; x < 10; x++ )
            fprintf( fpout, "Exits%d       %d\n", x, proom->exits[x] );
         for( x = 0; x < 10; x++ )
            fprintf( fpout, "Exflags%d     %d\n", x, proom->exflags[x] );
         for( x = 0; x < 10; x++ )
            fprintf( fpout, "Keys%d        %d\n", x, proom->keys[x] );
         fprintf( fpout, "RoomType          %d\n", proom->room_type );
         for( x = 0; x < 10; x++ )
            fprintf( fpout, "Rprog%d        %s~\n", x, proom->rprog[x] );
         for( x = 0; x < 10; x++ )
            fprintf( fpout, "Reset%d        %s~\n", x, proom->reset[x] );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "\n#ENDROOMS\n\n" );

   fprintf( fpout, "#END\n" );
   fclose( fpout );
   return;
}

void write_prototype_list(  )
{
   FILE *fpout;
   char filename[MAX_STRING_LENGTH];
   int x;

   sprintf( filename, "%sprototype.lst", SHIP_PROTOTYPE_DIR );
   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      perror( filename );
      return;
   }
   for( x = 0; x < NUM_PROTOTYPES; x++ )
      fprintf( fpout, "%s.proto\n", ship_prototypes[x].sname );
   fprintf( fpout, "$\n" );
   fclose( fpout );
   return;
}

void write_all_prototypes(  )
{
   int x;
   for( x = 0; x < NUM_PROTOTYPES; x++ )
      save_prototype( x );
   write_prototype_list(  );
}

bool load_prototype_header( FILE * fp, int prototype )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool done = FALSE;
   bool fMatch;
   while( !done )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case 'C':
            KEY( "Cost", ship_prototypes[prototype].cost, fread_number( fp ) );
            KEY( "Class", ship_prototypes[prototype].class, fread_number( fp ) );
            KEY( "Chaff", ship_prototypes[prototype].chaff, fread_number( fp ) );
            KEY( "Clan", ship_prototypes[prototype].clan, fread_string( fp ) );
         case 'E':
            KEY( "Energy", ship_prototypes[prototype].energy, fread_number( fp ) );
            if( !str_cmp( word, "End" ) )
            {
               done = TRUE;
               fMatch = TRUE;
               break;
            }
         case 'H':
            KEY( "Hull", ship_prototypes[prototype].hull, fread_number( fp ) );
            KEY( "Hyperspeed", ship_prototypes[prototype].hyperspeed, fread_number( fp ) );

         case 'M':
            KEY( "Maxbombs", ship_prototypes[prototype].maxbombs, fread_number( fp ) );
            KEY( "Manuever", ship_prototypes[prototype].manuever, fread_number( fp ) );
            KEY( "Mods", ship_prototypes[prototype].mods, fread_number( fp ) );
         case 'N':
            KEY( "Name", ship_prototypes[prototype].name, fread_string( fp ) );
            KEY( "NumRooms", ship_prototypes[prototype].num_rooms, fread_number( fp ) );
         case 'P':
            KEY( "primaryType", ship_prototypes[prototype].primaryType, fread_number( fp ) );
            KEY( "primaryCount", ship_prototypes[prototype].primaryCount, fread_number( fp ) );
         case 'R':
            KEY( "RangeWeapons0", ship_prototypes[prototype].range_weapons[0], fread_number( fp ) );
            KEY( "RangeWeapons1", ship_prototypes[prototype].range_weapons[1], fread_number( fp ) );
            KEY( "RangeWeapons2", ship_prototypes[prototype].range_weapons[2], fread_number( fp ) );
         case 'S':
            KEY( "secondaryType", ship_prototypes[prototype].secondaryType, fread_number( fp ) );
            KEY( "secondaryCount", ship_prototypes[prototype].secondaryCount, fread_number( fp ) );
            KEY( "Sname", ship_prototypes[prototype].sname, fread_string( fp ) );
            KEY( "Shields", ship_prototypes[prototype].shields, fread_number( fp ) );
            KEY( "Speed", ship_prototypes[prototype].speed, fread_number( fp ) );
         case 'T':
            KEY( "Tractor", ship_prototypes[prototype].tractor, fread_number( fp ) );
            KEY( "Turrets", ship_prototypes[prototype].turrets, fread_number( fp ) );
      }
      if( !fMatch )
      {
         sprintf( buf, "Load_prototype_header: no match: %s", word );
         bug( buf, 0 );
      }
   }
   return TRUE;
}

bool fread_prototype_room( FILE * fp, int prototype )
{
   PROTO_ROOM *proom;
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool done = FALSE;
   bool fMatch;
   CREATE( proom, PROTO_ROOM, 1 );
   proom->what_prototype = prototype;
   while( !done )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case 'D':
            KEY( "Desc", proom->desc, fread_string( fp ) );
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               done = TRUE;
               fMatch = TRUE;
               break;
            }
            KEY( "Exits0", proom->exits[0], fread_number( fp ) );
            KEY( "Exits1", proom->exits[1], fread_number( fp ) );
            KEY( "Exits2", proom->exits[2], fread_number( fp ) );
            KEY( "Exits3", proom->exits[3], fread_number( fp ) );
            KEY( "Exits4", proom->exits[4], fread_number( fp ) );
            KEY( "Exits5", proom->exits[5], fread_number( fp ) );
            KEY( "Exits6", proom->exits[6], fread_number( fp ) );
            KEY( "Exits7", proom->exits[7], fread_number( fp ) );
            KEY( "Exits8", proom->exits[8], fread_number( fp ) );
            KEY( "Exits9", proom->exits[9], fread_number( fp ) );
            KEY( "Exflags0", proom->exflags[0], fread_number( fp ) );
            KEY( "Exflags1", proom->exflags[1], fread_number( fp ) );
            KEY( "Exflags2", proom->exflags[2], fread_number( fp ) );
            KEY( "Exflags3", proom->exflags[3], fread_number( fp ) );
            KEY( "Exflags4", proom->exflags[4], fread_number( fp ) );
            KEY( "Exflags5", proom->exflags[5], fread_number( fp ) );
            KEY( "Exflags6", proom->exflags[6], fread_number( fp ) );
            KEY( "Exflags7", proom->exflags[7], fread_number( fp ) );
            KEY( "Exflags8", proom->exflags[8], fread_number( fp ) );
            KEY( "Exflags9", proom->exflags[9], fread_number( fp ) );
         case 'F':
            KEY( "Flags", proom->flags, fread_string( fp ) );
         case 'K':
            KEY( "Keys0", proom->keys[0], fread_number( fp ) );
            KEY( "Keys1", proom->keys[1], fread_number( fp ) );
            KEY( "Keys2", proom->keys[2], fread_number( fp ) );
            KEY( "Keys3", proom->keys[3], fread_number( fp ) );
            KEY( "Keys4", proom->keys[4], fread_number( fp ) );
            KEY( "Keys5", proom->keys[5], fread_number( fp ) );
            KEY( "Keys6", proom->keys[6], fread_number( fp ) );
            KEY( "Keys7", proom->keys[7], fread_number( fp ) );
            KEY( "Keys8", proom->keys[8], fread_number( fp ) );
            KEY( "Keys9", proom->keys[9], fread_number( fp ) );
         case 'N':
            KEY( "Name", proom->name, fread_string( fp ) );
         case 'R':
            KEY( "RoomType", proom->room_type, fread_number( fp ) );
            KEY( "RoomNum", proom->room_num, fread_number( fp ) );
            KEY( "Rprog0", proom->rprog[0], fread_string( fp ) );
            KEY( "Rprog1", proom->rprog[1], fread_string( fp ) );
            KEY( "Rprog2", proom->rprog[2], fread_string( fp ) );
            KEY( "Rprog3", proom->rprog[3], fread_string( fp ) );
            KEY( "Rprog4", proom->rprog[4], fread_string( fp ) );
            KEY( "Rprog5", proom->rprog[5], fread_string( fp ) );
            KEY( "Rprog6", proom->rprog[6], fread_string( fp ) );
            KEY( "Rprog7", proom->rprog[7], fread_string( fp ) );
            KEY( "Rprog8", proom->rprog[8], fread_string( fp ) );
            KEY( "Rprog9", proom->rprog[9], fread_string( fp ) );
            KEY( "Reset0", proom->reset[0], fread_string( fp ) );
            KEY( "Reset1", proom->reset[1], fread_string( fp ) );
            KEY( "Reset2", proom->reset[2], fread_string( fp ) );
            KEY( "Reset3", proom->reset[3], fread_string( fp ) );
            KEY( "Reset4", proom->reset[4], fread_string( fp ) );
            KEY( "Reset5", proom->reset[5], fread_string( fp ) );
            KEY( "Reset6", proom->reset[6], fread_string( fp ) );
            KEY( "Reset7", proom->reset[7], fread_string( fp ) );
            KEY( "Reset8", proom->reset[8], fread_string( fp ) );
            KEY( "Reset9", proom->reset[9], fread_string( fp ) );
         case 'T':
            KEY( "Tunnel", proom->tunnel, fread_number( fp ) );
      }
      if( !fMatch )
      {
         sprintf( buf, "Fread_prototype_room: no match: %s", word );
         bug( buf, 0 );
      }
   }
   LINK( proom, first_prototype_room, last_prototype_room, next, prev );
   return TRUE;
}

bool load_prototype_rooms( FILE * fp, int prototype )
{
   char letter;
   char *word;
   bool done = FALSE;
   while( !done )
   {
      letter = fread_letter( fp );
      if( letter == '#' )
      {
         word = fread_word( fp );
         if( !strcmp( "ROOM", word ) )
         {
            if( !fread_prototype_room( fp, prototype ) )
               return FALSE;
         }
         if( !strcmp( "ENDROOMS", word ) )
            done = TRUE;
      }
      else
      {
         bug( "Load_prototype_rooms, unknown prefix: %s", letter );
         return FALSE;
      }
   }
   return TRUE;
}

int load_prototype( char *prototypefile, int prototype )
{
   char filename[256];
   FILE *fp;
   bool found = FALSE;
   int stage = -1;
   bool ok = TRUE;
   char letter;
   char *word;

   sprintf( filename, "%s%s", SHIP_PROTOTYPE_DIR, prototypefile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      found = TRUE;
      prototype++;
      while( ok )
      {
         letter = fread_letter( fp );
         if( letter != '#' )
         {
            bug( "Load_prototype: # not found.", 0 );
            break;
         }
         stage++;
         word = fread_word( fp );
         if( strcmp( word, "END" ) )
         {
            switch ( stage )
            {
               case 0:
                  if( strcmp( word, "HEADER" ) )
                  {
                     bug( "Load_prototype: HEADER not found.", 0 );
                     break;
                  }
                  if( !load_prototype_header( fp, prototype ) )
                  {
                     ok = FALSE;
                     prototype--;
                  }
                  break;
               case 1:
                  if( strcmp( word, "ROOMS" ) )
                  {
                     bug( "Load_prototype: ROOMS not found.", 0 );
                     break;
                  }
                  if( !load_prototype_rooms( fp, prototype ) )
                  {
                     ok = FALSE;
                     prototype--;
                  }
                  break;
               default:
                  bug( "Load_prototype: Unknown stage: %d", stage );
            }
         }
         else
            ok = FALSE;
      }
      fclose( fp );
   }
   return prototype;
}

void load_ship_prototypes(  )
{
   FILE *fpList;
   char *filename;
   char prototypeslist[256];
   int prototype;
   first_prototype_room = NULL;
   last_prototype_room = NULL;

   log_string( "Loading ship prototypes..." );

   sprintf( prototypeslist, "%sprototype.lst", SHIP_PROTOTYPE_DIR );
   if( ( fpList = fopen( prototypeslist, "r" ) ) == NULL )
   {
      perror( prototypeslist );
      exit( 1 );
   }

   for( prototype = -1;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;


      prototype = load_prototype( filename, prototype );
   }
   fclose( fpList );
   log_string( " Done ship prototypes " );
   NUM_PROTOTYPES = prototype + 1;
   // write_all_prototypes();
   return;
}

void do_makeprototypeship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int prototype;
   int cost;
   int count;
   char ship_name[MAX_STRING_LENGTH];
   char name[MAX_STRING_LENGTH];
   char sname[MAX_STRING_LENGTH];
   char scost[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int x, len;
   ROOM_INDEX_DATA *room;
   MPROG_DATA *mprg;
   EXIT_DATA *eroom;
   int room_count = 1;
   PROTO_ROOM *proom;

   argument = one_argument( argument, ship_name );
   argument = one_argument( argument, scost );
   argument = one_argument( argument, sname );
   strcpy( name, argument );
   if( !ship_name || ship_name[0] == '\0' )
   {
      send_to_char( "You must specify a valid ship name.\r\n", ch );
      send_to_char( "USAGE: makeprototypeship <ship name> <cost> <short name> <long name>\r\n", ch );
      return;
   }
   cost = atoi( scost );
   if( cost <= 0 )
   {
      send_to_char( "The cost must be greater than 0.\r\n", ch );
      send_to_char( "USAGE: makeprototypeship <ship name> <cost> <short name> <long name>\r\n", ch );
      return;
   }
   if( !sname || sname[0] == '\0' )
   {
      send_to_char( "You must specify a short name for the ship.\r\n", ch );
      send_to_char( "USAGE: makeprototypeship <ship name> <cost> <short name> <long name>\r\n", ch );
      return;
   }
   len = strlen( sname );
   for( x = 0; x < len; x++ )
      sname[x] = UPPER( sname[x] );
   if( !name || name[0] == '\0' )
   {
      send_to_char( "You must specify a long name for the ship.\r\n", ch );
      send_to_char( "USAGE: makeprototypeship <ship name> <cost> <short name> <long name>\r\n", ch );
      return;
   }
   if( !scost || scost[0] == '\0' )
   {
      send_to_char( "You must specify a cost for the ship.\r\n", ch );
      send_to_char( "USAGE: makeprototypeship <ship name> <cost> <short name> <long name>\r\n", ch );
      return;
   }
   if( ( ship = get_ship( ship_name ) ) == NULL )
   {
      send_to_char( "You must specify a valid ship name.\r\n", ch );
      send_to_char( "USAGE: makeprototypeship <ship name> <short name> <long name> <cost>\r\n", ch );
      return;
   }

   prototype = NUM_PROTOTYPES++;

   ship_prototypes[prototype].tractor = ship->tractorbeam;
   ship_prototypes[prototype].primaryType = ship->primaryType;
   ship_prototypes[prototype].secondaryType = ship->secondaryType;
   ship_prototypes[prototype].primaryCount = ship->primaryCount;
   ship_prototypes[prototype].secondaryCount = ship->secondaryCount;
   ship_prototypes[prototype].range_weapons[0] = ship->maxmissiles;
   ship_prototypes[prototype].range_weapons[1] = ship->maxtorpedos;
   ship_prototypes[prototype].range_weapons[2] = ship->maxrockets;
   ship_prototypes[prototype].hull = ship->maxhull;
   ship_prototypes[prototype].shields = ship->maxshield;
   //ship_prototypes[prototype].plasma = ship->maxplasmashield;
   ship_prototypes[prototype].energy = ship->maxenergy;
   ship_prototypes[prototype].chaff = ship->maxchaff;
   ship_prototypes[prototype].maxbombs = ship->maxbombs;
   ship_prototypes[prototype].speed = ship->realspeed;
   ship_prototypes[prototype].hyperspeed = ship->hyperspeed;
   ship_prototypes[prototype].manuever = ship->manuever;
   ship_prototypes[prototype].cost = cost;
   sprintf( buf, "%s: '%s'", sname, name );
   ship_prototypes[prototype].name = STRALLOC( buf );
   ship_prototypes[prototype].sname = STRALLOC( sname );
   ship_prototypes[prototype].class = ship->class;
   ship_prototypes[prototype].num_rooms = ship->lastroom - ship->firstroom + 1;
   if( ship->turret1 != 0 )
      count++;
   if( ship->turret2 != 0 )
      count++;
   if( ship->turret3 != 0 )
      count++;
   if( ship->turret4 != 0 )
      count++;
   if( ship->turret5 != 0 )
      count++;
   if( ship->turret6 != 0 )
      count++;
   if( ship->turret7 != 0 )
      count++;
   if( ship->turret8 != 0 )
      count++;
   if( ship->turret9 != 0 )
      count++;
   if( ship->turret10 != 0 )
      count++;
   ship_prototypes[prototype].turrets = count;

   for( room = get_room_index( ship->firstroom ); room; room = room->next_in_ship )
   {
      CREATE( proom, PROTO_ROOM, 1 );
      proom->room_num = room_count;
      proom->name = STRALLOC( room->name );
      proom->desc = STRALLOC( room->description );
      proom->flags = STRALLOC( flag_string( room->room_flags, r_flags ) );
      proom->tunnel = room->tunnel;

      for( x = 0; x < 10; x++ )
      {
         eroom = get_exit_num( room, x );
         if( eroom )
         {
            proom->exits[x] = eroom->to_room->vnum;
            proom->exflags[x] = eroom->exit_info;
            proom->keys[x] = eroom->key;
         }
      }

      if( ship->cockpit == room->vnum )
         proom->room_type = 1;
      if( ship->entrance == room->vnum )
         proom->room_type = 2;
      if( ship->hanger1 == room->vnum )
         proom->room_type = 3;
      if( ship->hanger2 == room->vnum )
         proom->room_type = 4;
      if( ship->hanger3 == room->vnum )
         proom->room_type = 5;
      if( ship->hanger4 == room->vnum )
         proom->room_type = 6;
      if( ship->engineroom == room->vnum )
         proom->room_type = 7;
      if( ship->pilotseat == room->vnum )
         proom->room_type = 8;
      if( ship->coseat == room->vnum )
         proom->room_type = 9;
      if( ship->navseat == room->vnum )
         proom->room_type = 8;
      if( ship->gunseat == room->vnum )
         proom->room_type = 9;
      if( ship->turret1 == room->vnum )
         proom->room_type = 10;
      if( ship->turret2 == room->vnum )
         proom->room_type = 11;
      if( ship->turret3 == room->vnum )
         proom->room_type = 12;
      if( ship->turret4 == room->vnum )
         proom->room_type = 13;
      if( ship->turret5 == room->vnum )
         proom->room_type = 14;
      if( ship->turret6 == room->vnum )
         proom->room_type = 15;
      if( ship->turret7 == room->vnum )
         proom->room_type = 16;
      if( ship->turret8 == room->vnum )
         proom->room_type = 17;
      if( ship->turret9 == room->vnum )
         proom->room_type = 18;
      if( ship->turret10 == room->vnum )
         proom->room_type = 19;

      x = 1;
      for( mprg = room->mudprogs; mprg; mprg = mprg->next )
      {
         sprintf( buf, "%s %s %s ~", mprog_flags[mprg->type], mprg->arglist, mprg->comlist );
         proom->rprog[x] = STRALLOC( buf );
         x++;
      }

/* Remind me to add resets later        
	for(x=0;x<10;x++)
            fprintf( fpout, "Reset%d        %s~\n", x, proom->reset[x] );
        fprintf( fpout,"End\n\n");
*/

      LINK( proom, first_prototype_room, last_prototype_room, next, prev );
      room_count++;
   }

   send_to_char( "Done.\r\n", ch );
   save_prototype( prototype );
   return;
}

char pbname[MAX_STRING_LENGTH];

char *primary_beam_name_proto( int shiptype )
{
   if( ship_prototypes[shiptype].primaryCount != 0 )
   {
      sprintf( pbname, "%d %s", ship_prototypes[shiptype].primaryCount,
               ( ship_prototypes[shiptype].primaryType == SINGLE_LASER
                 && ship_prototypes[shiptype].primaryCount ==
                 1 ) ? "Single-laser cannon" : ( ship_prototypes[shiptype].primaryType == SINGLE_LASER
                                                 && ship_prototypes[shiptype].primaryCount !=
                                                 1 ) ? "Single-laser cannons" : ( ship_prototypes[shiptype].primaryType ==
                                                                                  DUAL_LASER
                                                                                  && ship_prototypes[shiptype].
                                                                                  primaryCount ==
                                                                                  1 ) ? "Dual-laser cannon"
               : ( ship_prototypes[shiptype].primaryType == DUAL_LASER
                   && ship_prototypes[shiptype].primaryCount !=
                   1 ) ? "Dual-laser cannons" : ( ship_prototypes[shiptype].primaryType == TRI_LASER
                                                  && ship_prototypes[shiptype].primaryCount ==
                                                  1 ) ? "Triple-laser cannon" : ( ship_prototypes[shiptype].primaryType ==
                                                                                  TRI_LASER
                                                                                  && ship_prototypes[shiptype].
                                                                                  primaryCount !=
                                                                                  1 ) ? "Triple-laser cannons"
               : ( ship_prototypes[shiptype].primaryType == QUAD_LASER
                   && ship_prototypes[shiptype].primaryCount ==
                   1 ) ? "Quad-laser cannon" : ( ship_prototypes[shiptype].primaryType == QUAD_LASER
                                                 && ship_prototypes[shiptype].primaryCount !=
                                                 1 ) ? "Quad-laser cannons" : ( ship_prototypes[shiptype].primaryType ==
                                                                                AUTOBLASTER
                                                                                && ship_prototypes[shiptype].primaryCount ==
                                                                                1 ) ? "Autoblaster turret"
               : ( ship_prototypes[shiptype].primaryType == AUTOBLASTER
                   && ship_prototypes[shiptype].primaryCount !=
                   1 ) ? "Autoblaster turrets" : ( ship_prototypes[shiptype].primaryType == HEAVY_LASER
                                                   && ship_prototypes[shiptype].primaryCount ==
                                                   1 ) ? "Heavy laser cannon" : ( ship_prototypes[shiptype].primaryType ==
                                                                                  HEAVY_LASER
                                                                                  && ship_prototypes[shiptype].
                                                                                  primaryCount !=
                                                                                  1 ) ? "Heavy laser cannons"
               : ( ship_prototypes[shiptype].primaryType == LIGHT_ION
                   && ship_prototypes[shiptype].primaryCount ==
                   1 ) ? "Light ion cannon" : ( ship_prototypes[shiptype].primaryType == LIGHT_ION
                                                && ship_prototypes[shiptype].primaryCount !=
                                                1 ) ? "Light ion cannons" : ( ship_prototypes[shiptype].primaryType ==
                                                                              REPEATING_ION
                                                                              && ship_prototypes[shiptype].primaryCount ==
                                                                              1 ) ? "Repeating ion cannon"
               : ( ship_prototypes[shiptype].primaryType == REPEATING_ION
                   && ship_prototypes[shiptype].primaryCount !=
                   1 ) ? "Repeating ion cannons" : ( ship_prototypes[shiptype].primaryType == HEAVY_ION
                                                     && ship_prototypes[shiptype].primaryCount ==
                                                     1 ) ? "Heavy ion cannon" : ( ship_prototypes[shiptype].primaryType ==
                                                                                  HEAVY_ION
                                                                                  && ship_prototypes[shiptype].
                                                                                  primaryCount !=
                                                                                  1 ) ? "Heavy ion cannons" : "unknown" );

      return pbname;
   }
   else
      return "None.";
}
char *secondary_beam_name_proto( int shiptype )
{
   if( ship_prototypes[shiptype].secondaryCount != 0 )
   {
      sprintf( pbname, "%d %s", ship_prototypes[shiptype].secondaryCount,
               ( ship_prototypes[shiptype].secondaryType == SINGLE_LASER
                 && ship_prototypes[shiptype].secondaryCount ==
                 1 ) ? "Single-laser cannon" : ( ship_prototypes[shiptype].secondaryType == SINGLE_LASER
                                                 && ship_prototypes[shiptype].secondaryCount !=
                                                 1 ) ? "Single-laser cannons" : ( ship_prototypes[shiptype].secondaryType ==
                                                                                  DUAL_LASER
                                                                                  && ship_prototypes[shiptype].
                                                                                  secondaryCount ==
                                                                                  1 ) ? "Dual-laser cannon"
               : ( ship_prototypes[shiptype].secondaryType == DUAL_LASER
                   && ship_prototypes[shiptype].secondaryCount !=
                   1 ) ? "Dual-laser cannons" : ( ship_prototypes[shiptype].secondaryType == TRI_LASER
                                                  && ship_prototypes[shiptype].secondaryCount ==
                                                  1 ) ? "Triple-laser cannon" : ( ship_prototypes[shiptype].secondaryType ==
                                                                                  TRI_LASER
                                                                                  && ship_prototypes[shiptype].
                                                                                  secondaryCount !=
                                                                                  1 ) ? "Triple-laser cannons"
               : ( ship_prototypes[shiptype].secondaryType == QUAD_LASER
                   && ship_prototypes[shiptype].secondaryCount ==
                   1 ) ? "Quad-laser cannon" : ( ship_prototypes[shiptype].secondaryType == QUAD_LASER
                                                 && ship_prototypes[shiptype].secondaryCount !=
                                                 1 ) ? "Quad-laser cannons" : ( ship_prototypes[shiptype].secondaryType ==
                                                                                AUTOBLASTER
                                                                                && ship_prototypes[shiptype].
                                                                                secondaryCount ==
                                                                                1 ) ? "Autoblaster turret"
               : ( ship_prototypes[shiptype].secondaryType == AUTOBLASTER
                   && ship_prototypes[shiptype].secondaryCount !=
                   1 ) ? "Autoblaster turrets" : ( ship_prototypes[shiptype].secondaryType == HEAVY_LASER
                                                   && ship_prototypes[shiptype].secondaryCount ==
                                                   1 ) ? "Heavy laser cannon" : ( ship_prototypes[shiptype].secondaryType ==
                                                                                  HEAVY_LASER
                                                                                  && ship_prototypes[shiptype].
                                                                                  secondaryCount !=
                                                                                  1 ) ? "Heavy laser cannons"
               : ( ship_prototypes[shiptype].secondaryType == LIGHT_ION
                   && ship_prototypes[shiptype].secondaryCount ==
                   1 ) ? "Light ion cannon" : ( ship_prototypes[shiptype].secondaryType == LIGHT_ION
                                                && ship_prototypes[shiptype].secondaryCount !=
                                                1 ) ? "Light ion cannons" : ( ship_prototypes[shiptype].secondaryType ==
                                                                              REPEATING_ION
                                                                              && ship_prototypes[shiptype].secondaryCount ==
                                                                              1 ) ? "Repeating ion cannon"
               : ( ship_prototypes[shiptype].secondaryType == REPEATING_ION
                   && ship_prototypes[shiptype].secondaryCount !=
                   1 ) ? "Repeating ion cannons" : ( ship_prototypes[shiptype].secondaryType == HEAVY_ION
                                                     && ship_prototypes[shiptype].secondaryCount ==
                                                     1 ) ? "Heavy ion cannon" : ( ship_prototypes[shiptype].secondaryType ==
                                                                                  HEAVY_ION
                                                                                  && ship_prototypes[shiptype].
                                                                                  secondaryCount !=
                                                                                  1 ) ? "Heavy ion cannons" : "unknown" );

      return pbname;
   }
   else
      return "None.";
}
void do_shipstat( CHAR_DATA * ch, char *argument )
{
   int shiptype;
   char buf1[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH];
   char buf4[MAX_STRING_LENGTH];
   char buf5[MAX_STRING_LENGTH];
   char buf6[MAX_STRING_LENGTH];
   char buf7[MAX_STRING_LENGTH];
   char buf8[MAX_STRING_LENGTH];
   char buf9[MAX_STRING_LENGTH];
   char buf10[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: shipstat <ship number>\n\r", ch );
      return;
   }
   shiptype = atoi( argument ) - 1;
   if( shiptype < 0 || shiptype > NUM_PROTOTYPES - 1 )
   {
      send_to_char( "Invalid number.\n\r", ch );
      return;
   }

   if( ship_prototypes[shiptype].primaryType > 0 )
      sprintf( buf1, "%d", ship_prototypes[shiptype].primaryType );
   else
      sprintf( buf1, "&RNone." );
   if( ship_prototypes[shiptype].secondaryType > 0 )
      sprintf( buf2, "%d", ship_prototypes[shiptype].secondaryType );
   else
      sprintf( buf2, "&RNone." );
   if( ship_prototypes[shiptype].range_weapons[0] > 0 )
      sprintf( buf3, "%d", ship_prototypes[shiptype].range_weapons[0] );
   else
      sprintf( buf3, "&RNone." );
   if( ship_prototypes[shiptype].shields > 0 )
      sprintf( buf4, "%d", ship_prototypes[shiptype].shields );
   else
      sprintf( buf4, "&RNone." );
   if( ship_prototypes[shiptype].range_weapons[1] > 0 )
      sprintf( buf5, "%d", ship_prototypes[shiptype].range_weapons[1] );
   else
      sprintf( buf5, "&RNone." );
   if( ship_prototypes[shiptype].hyperspeed > 0 )
      sprintf( buf6, "Class %d", ship_prototypes[shiptype].hyperspeed );
   else
      sprintf( buf6, "&RNone." );
   if( ship_prototypes[shiptype].range_weapons[2] > 0 )
      sprintf( buf7, "%d", ship_prototypes[shiptype].range_weapons[2] );
   else
      sprintf( buf7, "&RNone." );
   if( ship_prototypes[shiptype].maxbombs > 0 )
      sprintf( buf8, "%d", ship_prototypes[shiptype].maxbombs );
   else
      sprintf( buf8, "&RNone." );
   if( ship_prototypes[shiptype].turrets > 0 )
      sprintf( buf9, "%d", ship_prototypes[shiptype].turrets );
   else
      sprintf( buf9, "&RNone." );
   if( ship_prototypes[shiptype].chaff > 0 )
      sprintf( buf10, "%d", ship_prototypes[shiptype].chaff );
   else
      sprintf( buf10, "&RNone." );

   ch_printf( ch, "&R&z+&W---------------------------------------------------------------&z+\n\r" );
   ch_printf( ch, "&W| Name: &w%-35.35s      &WCost: &w%8d &W|\r\n",
              ship_prototypes[shiptype].name, ship_prototypes[shiptype].cost );
   ch_printf( ch, "&z+&W---------------------------------------------------------------&z+\n\r\n\r" );

   ch_printf( ch, "&W         Primary Weapon System:&w %-30.30s\n\r", primary_beam_name_proto( shiptype ) );
   ch_printf( ch, "&W                     Secondary:&w %-30.30s\n\r\n\r", secondary_beam_name_proto( shiptype ) );
   ch_printf( ch, "&W          Missiles:&w %-5s&W   Torpedos:&w %-5s&W   Rockets:&w %-5s\n\r", buf3, buf5, buf7 );
   ch_printf( ch, "&W   Planetary bombs:&w %-5s&W      Chaff:&w %-5s\n\r", buf8, buf10 );
   ch_printf( ch, "\n\r&W   Hull:&w %-5d&W   Shields:&w %-5s&W   Speed:&w %-5d&W   Energy:&w %d\n\r",
              ship_prototypes[shiptype].hull, buf4, ship_prototypes[shiptype].speed, ship_prototypes[shiptype].energy );
   ch_printf( ch, "&W           Maneuverability:&w %d   &WHyperdrive:&w %-5s\n\r", ship_prototypes[shiptype].manuever,
              buf6 );
   ch_printf( ch, "&W                        Turrets:&w %-5s\n\r", buf9 );

   return;
}

void load_market_list(  )
{
   FILE *fpList;
   char *filename;
   char list[256];
   BMARKET_DATA *marketship;
   int quantity;

   first_market_ship = NULL;
   last_market_ship = NULL;

   log_string( "Loading black market." );

   sprintf( list, "%s%s", SHIP_PROTOTYPE_DIR, "blackmarket.lst" );
   if( ( fpList = fopen( list, "r" ) ) == NULL )
   {
      perror( list );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;
      CREATE( marketship, BMARKET_DATA, 1 );
      LINK( marketship, first_market_ship, last_market_ship, next, prev );
      marketship->filename = STRALLOC( filename );
      quantity = fread_number( fpList );
      marketship->quantity = quantity;
   }
   fclose( fpList );
   log_string( "Finished loading blackmarket." );
   return;
}

void save_market_list(  )
{
   BMARKET_DATA *marketship;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", SHIP_PROTOTYPE_DIR, "blackmarket.lst" );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open blackmarket.lst for writing.\n\r", 0 );
      return;
   }
   for( marketship = first_market_ship; marketship; marketship = marketship->next )
   {
      fprintf( fpout, "%s\n", marketship->filename );
      fprintf( fpout, "%d\n", marketship->quantity );
   }
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

void add_market_ship( SHIP_DATA * ship )
{
   BMARKET_DATA *marketship;
   bool found;

   if( !ship )
      return;

   for( marketship = first_market_ship; marketship; marketship = marketship->next )
   {
      if( !str_cmp( marketship->filename, ship->protoname ) )
      {
         //Debugging  bug("Found, adding quantity", 0);
         found = TRUE;
         marketship->quantity++;
         return;
      }
   }


   //Debugging  bug("Not found, adding to .lst", 0);
   CREATE( marketship, BMARKET_DATA, 1 );
   LINK( marketship, first_market_ship, last_market_ship, next, prev );

   marketship->filename = STRALLOC( ship->protoname );
   marketship->quantity = 1;

   save_market_list(  );
}

void remove_market_ship( BMARKET_DATA * marketship )
{
   if( marketship->quantity > 1 )
      marketship->quantity--;
   else
   {
      UNLINK( marketship, first_market_ship, last_market_ship, next, prev );
      STRFREE( marketship->filename );
      DISPOSE( marketship );
   }

   save_market_list(  );
}

void make_random_marketlist(  )
{
// BMARKET_DATA *marketship;
   BMARKET_DATA *nmarketship;
   FILE *fpout;
   char filename[256];
   int x, priority, count;

   // Clear previously loaded data *** Change of plans, delete file and rebuild from scratch
   // *** db.c does not load from file anymore
/* for( marketship = first_market_ship; marketship; marketship = marketship->next )
 {
	UNLINK( marketship, first_market_ship, last_market_ship, next, prev );
	STRFREE( marketship->filename );
	DISPOSE( marketship );
 }*/

   sprintf( filename, "%s%s", SHIP_PROTOTYPE_DIR, "blackmarket.lst" );
   fpout = fopen( filename, "w" );
   fprintf( fpout, "$\n" );
   fclose( fpout );

   // Make a new list
   count = 0;
   while( 1 )
   {
      for( x = 0; x < NUM_PROTOTYPES; x++ )
      {

         if( str_cmp( ship_prototypes[x].clan, "The Empire" ) && str_cmp( ship_prototypes[x].clan, "The New Republic" ) )
            continue;
         if( count == 1 )
         {
            if( number_range( 1, 10 ) == 1 )
            {
               save_market_list(  );
               return;
            }
         }
         if( count == 2 )
         {
            if( number_range( 1, 10 ) < 3 )
            {
               save_market_list(  );
               return;
            }
         }
         if( count == 3 )
         {
            if( number_range( 1, 10 ) != 3 )
            {
               save_market_list(  );
               return;
            }
         }
         if( count == 4 )
         {
            save_market_list(  );
            return;
         }

         priority = 0;
         if( !str_cmp( ship_prototypes[x].sname, "TIE" ) )
            priority = 10;
         if( !str_cmp( ship_prototypes[x].sname, "TIE.D" ) )
            priority = 1;
         if( !str_cmp( ship_prototypes[x].sname, "TIE.B" ) )
            priority = 5;
         if( !str_cmp( ship_prototypes[x].sname, "XM-1" ) )
            priority = 3;
         if( !str_cmp( ship_prototypes[x].sname, "XG-1" ) )
            priority = 4;
         if( !str_cmp( ship_prototypes[x].sname, "X-Wing" ) )
            priority = 8;
         if( !str_cmp( ship_prototypes[x].sname, "Y-Wing" ) )
            priority = 8;
         if( !str_cmp( ship_prototypes[x].sname, "B-Wing" ) )
            priority = 2;
         if( !str_cmp( ship_prototypes[x].sname, "A-Wing" ) )
            priority = 6;
         if( !str_cmp( ship_prototypes[x].sname, "K-Wing" ) )
            priority = 3;

         if( priority != 0 )
         {
            if( priority == 1 || priority == 2 )
            {
               if( number_range( 1, 25 ) >= 24 )
               {
/*    	for(marketship = first_market_ship; marketship; marketship = marketship->next)
    	{
    	 if(str_cmp(marketship->filename, ship_prototypes[x].sname))
   	 {*/
                  CREATE( nmarketship, BMARKET_DATA, 1 );
                  LINK( nmarketship, first_market_ship, last_market_ship, next, prev );

                  nmarketship->filename = STRALLOC( ship_prototypes[x].sname );
                  nmarketship->quantity = 1;
                  count++;
/*   	 }
   	} */
               }
            }
            if( priority <= 5 && priority >= 3 )
            {
               if( number_range( 1, 25 ) >= 22 )
               {
/*    	for(marketship = first_market_ship; marketship; marketship = marketship->next)
    	{
    		if(str_cmp(ship_prototypes[x].sname, marketship->filename))
    		{*/
                  CREATE( nmarketship, BMARKET_DATA, 1 );
                  LINK( nmarketship, first_market_ship, last_market_ship, next, prev );

                  nmarketship->filename = STRALLOC( ship_prototypes[x].sname );
                  if( priority >= 4 && ( number_range( 1, 10 ) == 10 ) )
                     nmarketship->quantity = number_range( 1, 3 );
                  else
                     nmarketship->quantity = 1;
                  count++;
/*        	}
        }*/
               }
            }
            if( priority > 5 && priority < 9 )
            {
               if( number_range( 1, 25 ) >= 20 )
               {
/*    	for(marketship = first_market_ship; marketship; marketship = marketship->next)
    	{
    		if(str_cmp(ship_prototypes[x].sname, marketship->filename))
    		{*/
                  CREATE( nmarketship, BMARKET_DATA, 1 );
                  LINK( nmarketship, first_market_ship, last_market_ship, next, prev );

                  nmarketship->filename = STRALLOC( ship_prototypes[x].sname );
                  if( ( priority == 5 || priority == 6 ) && ( number_range( 1, 10 ) == 10 ) )
                     nmarketship->quantity = number_range( 1, 3 );
                  else
                     nmarketship->quantity = 1;
                  count++;
                  /*
                   * }
                   * }
                   */
               }
            }
            if( priority >= 9 )
            {
               if( number_range( 1, 25 ) >= 18 )
               {
/*    	for(marketship = first_market_ship; marketship; marketship = marketship->next)
    	{
    		if(str_cmp(ship_prototypes[x].sname, marketship->filename))
    		{*/
                  CREATE( nmarketship, BMARKET_DATA, 1 );
                  LINK( nmarketship, first_market_ship, last_market_ship, next, prev );

                  nmarketship->filename = STRALLOC( ship_prototypes[x].sname );
                  if( number_range( 1, 10 ) == 10 )
                     nmarketship->quantity = number_range( 1, 3 );
                  else
                     nmarketship->quantity = 1;
                  count++;
/*   		}
   	}*/
               }
            }
         }
      }  // End for prototypes
   }
   save_market_list(  );
}

void do_generate_market( CHAR_DATA * ch, char *argument )
{
   BMARKET_DATA *marketship;


   if( IS_NPC( ch ) )
      return;

   if( !first_market_ship )
   {
      bug( "Error 0." );
      return;
   }

   marketship = first_market_ship;
   if( !( marketship ) )
   {
      bug( "Error 2." );
      return;
   }


   // Clear list
   for( marketship = first_market_ship; marketship; marketship = marketship->next )
   {
      UNLINK( marketship, first_market_ship, last_market_ship, next, prev );
      STRFREE( marketship->filename );
// Pretty sure you don't need this, especially after its been unlinked.
// DISPOSE( marketship );
   }
   // Generate new
   make_random_marketlist(  );
   return;
}



void do_installmodule( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   MODULE_DATA *mod;
   int i = 0, maxmod;
   bool checktool, checkmod;
   OBJ_DATA *obj;
   OBJ_DATA *modobj;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   int x;
   int schance;
//Max slots per mod.
   int maxslot = 2;
//Damned Slots.
   int primary = 0;
   int secondary = 0;
   int missile = 0;
   int rocket = 0;
   int torpedo = 0;
   int hull = 0;
   int shields = 0;
   int speed = 0;
   int hyperspeed = 0;
   int energy = 0;
   int manuever = 0;
   int salarm = 0;
   int chaff = 0;

   strcpy( arg, argument );
   checktool = FALSE;
   checkmod = FALSE;
   switch ( ch->substate )
   {
      default:
         if( ( ship = ship_from_engine( ch->in_room->vnum ) ) != NULL )
         {
            ship = ship_from_engine( ch->in_room->vnum );
            strcpy( arg, ship->name );
         }

         if( !ship )
         {
            send_to_char( "You need to be in the ships engine room.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checktool = TRUE;
            if( obj->item_type == ITEM_MODULE )
            {
               checkmod = TRUE;
               modobj = obj;
            }
         }

         if( checktool == FALSE )
         {
            send_to_char( "You need a toolkit to install a module.\n\r", ch );
            return;
         }
         if( checkmod == FALSE )
         {
            send_to_char( "You need a module to install!\n\r", ch );
            return;
         }

         for( i = 0, mod = ship->first_module; mod; mod = mod->next )
         {
            ++i;
            if( mod->affect == AFFECT_PRIMARY )
               ++primary;
            if( mod->affect == AFFECT_SECONDARY )
               ++secondary;
            if( mod->affect == AFFECT_MISSILE )
               ++missile;
            if( mod->affect == AFFECT_ROCKET )
               ++rocket;
            if( mod->affect == AFFECT_TORPEDO )
               ++torpedo;
            if( mod->affect == AFFECT_HULL )
               ++hull;
            if( mod->affect == AFFECT_SHIELD )
               ++shields;
            if( mod->affect == AFFECT_SPEED )
               ++speed;
            if( mod->affect == AFFECT_HYPER )
               ++hyperspeed;
            if( mod->affect == AFFECT_ENERGY )
               ++energy;
            if( mod->affect == AFFECT_MANUEVER )
               ++manuever;
            if( mod->affect == AFFECT_ALARM )
               ++salarm;
            if( mod->affect == AFFECT_CHAFF )
               ++chaff;
         }

//Holy ifchecks batman!
         if( modobj->value[0] == AFFECT_ALARM && ( ship->alarm == 1 ) )
         {
            send_to_char( "This ship already has an alarm system!\n\r", ch );
            return;
         }
         if( modobj->value[0] == AFFECT_HYPER && ( ship->hyperspeed == 1 ) )
         {
            send_to_char( "This ship already has a first class hyperspeed drive.\n\r", ch );
            return;
         }
         if( ( modobj->value[0] == AFFECT_PRIMARY && primary >= maxslot ) ||
             ( modobj->value[0] == AFFECT_SECONDARY && secondary >= maxslot ) ||
             ( modobj->value[0] == AFFECT_MISSILE && missile >= maxslot ) ||
             ( modobj->value[0] == AFFECT_ROCKET && rocket >= maxslot ) ||
             ( modobj->value[0] == AFFECT_TORPEDO && torpedo >= maxslot ) ||
             ( modobj->value[0] == AFFECT_HULL && hull >= maxslot ) ||
             ( modobj->value[0] == AFFECT_SHIELD && shields >= maxslot ) ||
             ( modobj->value[0] == AFFECT_SPEED && speed >= maxslot ) ||
             ( modobj->value[0] == AFFECT_HYPER && hyperspeed >= maxslot ) ||
             ( modobj->value[0] == AFFECT_ENERGY && energy >= maxslot ) ||
             ( modobj->value[0] == AFFECT_MANUEVER && manuever >= maxslot ) ||
             ( modobj->value[0] == AFFECT_ALARM && salarm >= maxslot ) ||
             ( modobj->value[0] == AFFECT_CHAFF && chaff >= maxslot ) )
         {
            send_to_char( "&RYou've already filled that slot to its maximum.\n\r", ch );
            return;
         }

         for( x = 0; x < NUM_PROTOTYPES; x++ )
         {
            if( !str_cmp( ship->protoname, ship_prototypes[x].sname ) )
            {
               maxmod = ship_prototypes[x].mods;
               break;
            }
         }

         if( i >= maxmod )
         {
            send_to_char( "This ship is already at it's module limit!\n\r", ch );
            return;
         }

         if( IS_SET( ship->flags, SHIP_SIMULATOR ) )
         {
            send_to_char( "Simulators can't have modules, fool!\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_installmodule] );
         if( number_percent(  ) < schance )
         {
            ch->dest_buf = str_dup( arg );
            send_to_char( "&GYou begin the long process of installing a new module.\n\r", ch );
            sprintf( buf, "$n takes out $s toolkit and a module and begins to work.\n\r" );
            act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 5, do_installmodule, 1 );
            return;
         }

         send_to_char( "&RYou are unable to figure out what to do.\n\r", ch );
         learn_from_failure( ch, gsn_installmodule );
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
      if( obj->item_type == ITEM_MODULE )
      {
         checkmod = TRUE;
         modobj = obj;
      }

   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_installmodule] );

   ship = ship_from_engine( ch->in_room->vnum );
   if( !ship )
   {
      send_to_char( "Error: Something went wrong. Contact an Admin!\n\r", ch );
      return;
   }

   if( number_percent(  ) > schance * 2 )
   {
      send_to_char( "&RYou finish installing the new module and everything's looking good...\n\r", ch );
      send_to_char( "&RThen it turns bright red and melts!\n\r", ch );
      learn_from_failure( ch, gsn_installmodule );
      return;
   }

   CREATE( mod, MODULE_DATA, 1 );
   LINK( mod, ship->first_module, ship->last_module, next, prev );
   mod->affect = modobj->value[0];
   mod->ammount = modobj->value[1];
   if( mod->affect == AFFECT_PRIMARY )
      ship->primaryCount += mod->ammount;
   if( mod->affect == AFFECT_SECONDARY )
      ship->secondaryCount += mod->ammount;
   if( mod->affect == AFFECT_MISSILE )
   {
      ship->maxmissiles += mod->ammount;
      ship->missiles = ship->maxmissiles;
   }
   if( mod->affect == AFFECT_ROCKET )
   {
      ship->maxrockets += mod->ammount;
      ship->rockets = ship->maxrockets;
   }
   if( mod->affect == AFFECT_TORPEDO )
   {
      ship->maxtorpedos += mod->ammount;
      ship->torpedos = ship->maxtorpedos;
   }
   if( mod->affect == AFFECT_HULL )
      ship->maxhull += mod->ammount;
   if( mod->affect == AFFECT_SHIELD )
      ship->maxshield += mod->ammount;
   if( mod->affect == AFFECT_SPEED )
      ship->realspeed += mod->ammount;
   if( mod->affect == AFFECT_HYPER )
      ship->hyperspeed -= mod->ammount;
   if( mod->affect == AFFECT_ENERGY )
      ship->maxenergy += mod->ammount;
   if( mod->affect == AFFECT_MANUEVER )
      ship->manuever += mod->ammount;
   if( mod->affect == AFFECT_ALARM )
      ship->alarm += mod->ammount;
   if( mod->affect == AFFECT_CHAFF )
      ship->maxchaff += mod->ammount;
   save_ship( ship );
   separate_obj( modobj );
   obj_from_char( modobj );
   extract_obj( modobj );
   send_to_char( "You finish installing your new module.\n\r", ch );

   {
      long xpgain;
      xpgain = ( ( ch->skill_level[TECHNICIAN_ABILITY] + 1 ) * 300 );
      gain_exp( ch, xpgain, TECHNICIAN_ABILITY );
      ch_printf( ch, " You gain %d experience for being a Technician.\n\r", xpgain );
      learn_from_success( ch, gsn_installmodule );
   }
   return;
}

void do_shiplist( CHAR_DATA * ch, char *argument )
{
   int x;

   send_to_char( "\n\r&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
   send_to_char( "&W|&w                             Complete ship listing                           &W|\r\n", ch );
   send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );

   for( x = 0; x < NUM_PROTOTYPES; x++ )
      ch_printf( ch, "&W|&w  &w(&W%2d&w) %s%-35.35s&W Type: %s %-15.15s &W   |\r\n", x + 1,
                 !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&R" :
                 !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&G" : "&w",
                 ship_prototypes[x].name,
                 !str_cmp( ship_prototypes[x].clan, "The Empire" ) ? "&RImperial" :
                 !str_cmp( ship_prototypes[x].clan, "The New Republic" ) ? "&GRepublic" :
                 "&wCivilian",
                 ship_prototypes[x].class == SHIP_FIGHTER ? "Starfighter" :
                 ship_prototypes[x].class == SHIP_BOMBER ? "Bomber" :
                 ship_prototypes[x].class == SHIP_SHUTTLE ? "Shuttle" :
                 ship_prototypes[x].class == SHIP_FREIGHTER ? "Freighter" :
                 ship_prototypes[x].class == SHIP_FRIGATE ? "Frigate" :
                 ship_prototypes[x].class == SHIP_TT ? "Troop Transport" :
                 ship_prototypes[x].class == SHIP_CORVETTE ? "Corvette" :
                 ship_prototypes[x].class == SHIP_CRUISER ? "Cruiser" :
                 ship_prototypes[x].class == SHIP_DREADNAUGHT ? "Dreadnaught" :
                 ship_prototypes[x].class == SHIP_DESTROYER ? "Star Destroyer" :
                 ship_prototypes[x].class == SHIP_SPACE_STATION ? "Space Station" :
                 ship_prototypes[x].class == LAND_VEHICLE ? "Land vehicle" : "Unknown" );


   send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\r\n", ch );
   send_to_char( "&W|&w               Use 'shipstat (number)' to get ship information               &W|\r\n", ch );
   send_to_char( "&z+&W-----------------------------------------------------------------------------&z+\n\r", ch );
   return;
}
