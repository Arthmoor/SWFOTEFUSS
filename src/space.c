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

SHIP_DATA *first_ship;
SHIP_DATA *last_ship;

MISSILE_DATA *first_missile;
MISSILE_DATA *last_missile;

SPACE_DATA *first_starsystem;
SPACE_DATA *last_starsystem;

char *primary_beam_name args( ( SHIP_DATA * ship ) );
char *secondary_beam_name args( ( SHIP_DATA * ship ) );

int bus_pos = 0;
int bus_planet = 0;
int bus2_planet = 5;
int bus3_planet = 3;
int bus4_planet = 8;
int turbocar_stop = 0;
int corus_shuttle = 0;

#define MAX_STATION    10
#define MAX_BUS_STOP 10

#define STOP_PLANET     1057
#define STOP_SHIPYARD   1057


int const station_vnum[MAX_STATION] = {
   215, 216, 217, 218, 219, 220, 221, 222, 223, 224
};

char *const station_name[MAX_STATION] = {
   "Menari Spaceport", "Skydome Botanical Gardens", "Grand Towers",
   "Grandis Mon Theater", "Palace Station", "Great Galactic Museum",
   "College Station", "Holographic Zoo of Extinct Animals",
   "Dometown Station ", "Monument Plaza"
};

int const bus_vnum[MAX_BUS_STOP] = {
   1053, 1850, 250, 4556, 28613, 9701, 7801, 9000, 10154, 18003
};

char *const bus_stop[MAX_BUS_STOP + 1] = {
   "Coruscant", "Yavin IV", "Tatooine", "Endor",
   "Kashyyyk", "Mon Calamari", "Nal Hutta", "Ord Mantell", "Corellia",
   "Bakura", "Coruscant"   /* last should always be same as first */
};

char *const ship_flags[] = {
   "nohijack", "shield_boost", "torp_boost", "chaff_boost", "hull_boost",
   "laser_boost", "missile_boost", "rocket_boost", "simulator", "nodestroy", "noslicer", "ion_lasers",
   "ion_drive", "ion_ion", "ion_turret1", "ion_turret2", "ion_turret3",
   "ion_turret4", "ion_turret5", "ion_turret6", "ion_turret7", "ion_turret8",
   "ion_turret9", "ion_turret10", "respawn", "ion_misiles", "ion_hyper", "cloak", "r24",
   "r25", "r26", "r27", "r28", "r29",
   "r30", "r31"
};

int get_shipflag( char *flag )
{
   int x;

   for( x = 0; x < 32; x++ )
      if( !str_cmp( flag, ship_flags[x] ) )
         return x;
   return -1;
}

/* local routines */
void fread_ship args( ( SHIP_DATA * ship, FILE * fp ) );
bool load_ship_file args( ( char *shipfile ) );
void fread_starsystem args( ( SPACE_DATA * starsystem, FILE * fp ) );
bool load_starsystem args( ( char *starsystemfile ) );
void write_starsystem_list args( ( void ) );
void landship args( ( SHIP_DATA * ship, char *arg ) );
void launchship args( ( SHIP_DATA * ship ) );
bool land_bus args( ( SHIP_DATA * ship, int destination ) );
void launch_bus args( ( SHIP_DATA * ship ) );
void echo_to_room_dnr args( ( int ecolor, ROOM_INDEX_DATA * room, char *argument ) );
ch_ret drive_ship( CHAR_DATA * ch, SHIP_DATA * ship, EXIT_DATA * pexit, int fall );
bool autofly( SHIP_DATA * ship );
bool is_facing( SHIP_DATA * ship, SHIP_DATA * target );
void sound_to_ship( SHIP_DATA * ship, char *argument );

/* from comm.c */
bool write_to_descriptor args( ( int desc, char *txt, int length ) );

ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA * in_room, EXIT_DATA ** pexit );

void echo_to_room_dnr( int ecolor, ROOM_INDEX_DATA * room, char *argument )
{
   CHAR_DATA *vic;

   if( room == NULL )
      return;

   for( vic = room->first_person; vic; vic = vic->next_in_room )
   {
      set_char_color( ecolor, vic );
      send_to_char( argument, vic );
   }
}



bool land_bus( SHIP_DATA * ship, int destination )
{
   char buf[MAX_STRING_LENGTH];

   if( ship )
      extract_ship( ship );

   if( !ship_to_room( ship, destination ) )
   {
      return FALSE;
   }

   echo_to_ship( AT_YELLOW, ship, "You feel a slight thud as the ship sets down on the ground." );
   ship->location = destination;
   ship->lastdoc = ship->location;
   ship->shipstate = SHIP_DOCKED;
   if( ship->starsystem )
      ship_from_starsystem( ship, ship->starsystem );
   sprintf( buf, "%s lands on the platform.", ship->name );
   echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
   sprintf( buf, "The hatch on %s opens.", ship->name );
   echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
   echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch opens." );
   ship->hatchopen = TRUE;
   sound_to_room( get_room_index( ship->entrance ), "!!SOUND(door)" );
   sound_to_room( get_room_index( ship->location ), "!!SOUND(door)" );
   return TRUE;
}

void launch_bus( SHIP_DATA * ship )
{
   char buf[MAX_STRING_LENGTH];

   sound_to_room( get_room_index( ship->entrance ), "!!SOUND(door)" );
   sound_to_room( get_room_index( ship->location ), "!!SOUND(door)" );
   sprintf( buf, "The hatch on %s closes and it begins to launch.", ship->name );
   echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
   echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch slides shut." );
   ship->hatchopen = FALSE;
   extract_ship( ship );
   echo_to_ship( AT_YELLOW, ship, "The ship begins to launch." );
   ship->location = 0;
   ship->shipstate = SHIP_READY;
}

void update_traffic(  )
{
   SHIP_DATA *shuttle;
   SHIP_DATA *turbocar;
   char buf[MAX_STRING_LENGTH];

   shuttle = ship_from_cockpit( ROOM_CORUSCANT_SHUTTLE );
   if( shuttle != NULL )
   {
      switch ( corus_shuttle )
      {
         default:
            corus_shuttle++;
            break;

         case 0:
            land_bus( shuttle, STOP_PLANET );
            corus_shuttle++;
            echo_to_ship( AT_CYAN, shuttle, "Welcome to Bulova Spaceport." );
            break;

         case 4:
            launch_bus( shuttle );
            corus_shuttle++;
            break;

         case 5:
            land_bus( shuttle, STOP_SHIPYARD );
            echo_to_ship( AT_CYAN, shuttle, "Welcome to Coruscant Shipyard." );
            corus_shuttle++;
            break;

         case 9:
            launch_bus( shuttle );
            corus_shuttle++;
            break;

      }

      if( corus_shuttle >= 10 )
         corus_shuttle = 0;
   }

   turbocar = ship_from_cockpit( ROOM_CORUSCANT_TURBOCAR );
   if( turbocar != NULL )
   {
      sprintf( buf, "The turbocar doors close and it speeds out of the station." );
      echo_to_room( AT_YELLOW, get_room_index( turbocar->location ), buf );
      extract_ship( turbocar );
      turbocar->location = 0;
      ship_to_room( turbocar, station_vnum[turbocar_stop] );
      echo_to_ship( AT_YELLOW, turbocar, "The turbocar makes a quick journey to the next station." );
      turbocar->location = station_vnum[turbocar_stop];
      turbocar->lastdoc = turbocar->location;
      turbocar->shipstate = SHIP_DOCKED;
      if( turbocar->starsystem )
         ship_from_starsystem( turbocar, turbocar->starsystem );
      sprintf( buf, "A turbocar pulls into the platform and the doors slide open." );
      echo_to_room( AT_YELLOW, get_room_index( turbocar->location ), buf );
      sprintf( buf, "Welcome to %s.", station_name[turbocar_stop] );
      echo_to_ship( AT_CYAN, turbocar, buf );
      turbocar->hatchopen = TRUE;

      turbocar_stop++;
      if( turbocar_stop >= MAX_STATION )
         turbocar_stop = 0;
   }

}

void update_bus(  )
{
   SHIP_DATA *ship;
   SHIP_DATA *ship2;
   SHIP_DATA *ship3;
   SHIP_DATA *ship4;
   SHIP_DATA *target;
   int destination;
   char buf[MAX_STRING_LENGTH];

   ship = ship_from_cockpit( ROOM_SHUTTLE_BUS );
   ship2 = ship_from_cockpit( ROOM_SHUTTLE_BUS_2 );
   ship3 = ship_from_cockpit( ROOM_SHUTTLE_BUS_3 );
   ship4 = ship_from_cockpit( ROOM_SHUTTLE_BUS_4 );

   if( ship == NULL )
   {
      bug( "update_bus: Ship is NULL" );
      return;
   }
   else if( ship2 == NULL )
   {
      bug( "update_bus: Ship2 is NULL" );
      return;
   }
   if( ship3 == NULL )
   {
      bug( "update_bus: Ship3 is NULL" );
      return;
   }
   if( ship4 == NULL )
   {
      bug( "update_bus: Ship4 is NULL" );
      return;
   }

   switch ( bus_pos )
   {

      case 0:
         target = ship_from_hanger( bus_vnum[bus_planet] );
         if( target != NULL && !target->starsystem )
         {
            sprintf( buf, "An electronic voice says, 'Cannot land at %s ... it seems to have dissapeared.'",
                     bus_stop[bus_planet] );
            echo_to_ship( AT_CYAN, ship, buf );
            bus_pos = 5;
         }

         target = ship_from_hanger( bus_vnum[bus2_planet] );
         if( target != NULL && !target->starsystem )
         {
            sprintf( buf, "An electronic voice says, 'Cannot land at %s ... it seems to have dissapeared.'",
                     bus_stop[bus_planet] );
            echo_to_ship( AT_CYAN, ship2, buf );
            bus_pos = 5;
         }
         target = ship_from_hanger( bus_vnum[bus3_planet] );
         if( target != NULL && !target->starsystem )
         {
            sprintf( buf, "An electronic voice says, 'Cannot land at %s ... it seems to have disappeared.'",
                     bus_stop[bus_planet] );
            echo_to_ship( AT_CYAN, ship3, buf );
            bus_pos = 5;
         }
         target = ship_from_hanger( bus_vnum[bus4_planet] );
         if( target != NULL && !target->starsystem )
         {
            sprintf( buf, "An electronic voice says, 'Cannot land at %s ... it seems to have disappeared.'",
                     bus_stop[bus_planet] );
            echo_to_ship( AT_CYAN, ship4, buf );
            bus_pos = 5;
         }
         bus_pos++;
         break;

      case 6:
         launch_bus( ship );
         launch_bus( ship2 );
         launch_bus( ship3 );
         launch_bus( ship4 );
         bus_pos++;
         break;

      case 7:
         echo_to_ship( AT_YELLOW, ship, "The ship lurches slightly as it makes the jump to lightspeed." );
         echo_to_ship( AT_YELLOW, ship2, "The ship lurches slightly as it makes the jump to lightspeed." );
         echo_to_ship( AT_YELLOW, ship3, "The ship lurches slightly as it makes the jump to lightspeed." );
         echo_to_ship( AT_YELLOW, ship4, "The ship lurches slightly as it makes the jump to lightspeed." );

         bus_pos++;
         break;

      case 9:

         echo_to_ship( AT_YELLOW, ship, "The ship lurches slightly as it comes out of hyperspace.." );
         echo_to_ship( AT_YELLOW, ship2, "The ship lurches slightly as it comes out of hyperspace.." );
         echo_to_ship( AT_YELLOW, ship3, "The ship lurches slightly as it comes out of hyperspace.." );
         echo_to_ship( AT_YELLOW, ship4, "The ship lurches slightly as it comes out of hyperspace.." );
         bus_pos++;
         break;

      case 1:
         destination = bus_vnum[bus_planet];
         if( !land_bus( ship, destination ) )
         {
            sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'", bus_stop[bus_planet] );
            echo_to_ship( AT_CYAN, ship, buf );
            echo_to_ship( AT_CYAN, ship, "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'" );
         }
         else
         {
            sprintf( buf, "An electronic voice says, 'Welcome to %s'", bus_stop[bus_planet] );
            echo_to_ship( AT_CYAN, ship, buf );
            echo_to_ship( AT_CYAN, ship, "It continues, 'Please exit through the main ramp. Enjoy your stay.'" );
         }

         destination = bus_vnum[bus2_planet];
         if( !land_bus( ship2, destination ) )
         {
            sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'", bus_stop[bus_planet] );
            echo_to_ship( AT_CYAN, ship2, buf );
            echo_to_ship( AT_CYAN, ship2, "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'" );
         }
         else
         {
            sprintf( buf, "An electronic voice says, 'Welcome to %s'", bus_stop[bus2_planet] );
            echo_to_ship( AT_CYAN, ship2, buf );
            echo_to_ship( AT_CYAN, ship2, "It continues, 'Please exit through the main ramp. Enjoy your stay.'" );
         }

         destination = bus_vnum[bus3_planet];
         if( !land_bus( ship3, destination ) )
         {
            sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'", bus_stop[bus3_planet] );
            echo_to_ship( AT_CYAN, ship3, buf );
            echo_to_ship( AT_CYAN, ship3, "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'" );
         }
         else
         {
            sprintf( buf, "An electronic voice says, 'Welcome to %s'", bus_stop[bus3_planet] );
            echo_to_ship( AT_CYAN, ship3, buf );
            echo_to_ship( AT_CYAN, ship3, "It continues, 'Please exit through the main ramp. Enjoy your stay.'" );
         }

         destination = bus_vnum[bus4_planet];
         if( !land_bus( ship4, destination ) )
         {
            sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'", bus_stop[bus4_planet] );
            echo_to_ship( AT_CYAN, ship4, buf );
            echo_to_ship( AT_CYAN, ship4, "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'" );
         }
         else
         {
            sprintf( buf, "An electronic voice says, 'Welcome to %s'", bus_stop[bus4_planet] );
            echo_to_ship( AT_CYAN, ship4, buf );
            echo_to_ship( AT_CYAN, ship4, "It continues, 'Please exit through the main ramp. Enjoy your stay.'" );
         }

         bus_pos++;
         break;

      case 5:
         sprintf( buf, "It continues, 'Next stop, %s'", bus_stop[bus_planet + 1] );
         echo_to_ship( AT_CYAN, ship, "An electronic voice says, 'Preparing for launch.'" );
         echo_to_ship( AT_CYAN, ship, buf );

         sprintf( buf, "It continues, 'Next stop, %s'", bus_stop[bus2_planet + 1] );
         echo_to_ship( AT_CYAN, ship2, "An electronic voice says, 'Preparing for launch.'" );
         echo_to_ship( AT_CYAN, ship2, buf );

         sprintf( buf, "It continues, 'Next stop, %s'", bus_stop[bus3_planet + 1] );
         echo_to_ship( AT_CYAN, ship3, "An electronic voice says, 'Preparing for launch.'" );
         echo_to_ship( AT_CYAN, ship3, buf );

         sprintf( buf, "It continues, 'Next stop, %s'", bus_stop[bus4_planet + 1] );
         echo_to_ship( AT_CYAN, ship4, "An electronic voice says, 'Preparing for launch.'" );
         echo_to_ship( AT_CYAN, ship4, buf );

         bus_pos++;
         break;

      default:
         bus_pos++;
         break;
   }

   if( bus_pos >= 10 )
   {
      bus_pos = 0;
      bus_planet++;
      bus2_planet++;
      bus3_planet++;
      bus4_planet++;
   }

   if( bus_planet >= MAX_BUS_STOP )
      bus_planet = 0;
   if( bus2_planet >= MAX_BUS_STOP )
      bus2_planet = 0;
   if( bus3_planet >= MAX_BUS_STOP )
      bus3_planet = 0;
   if( bus4_planet >= MAX_BUS_STOP )
      bus4_planet = 0;

}

void move_ships(  )
{
   SHIP_DATA *ship;
   MISSILE_DATA *missile;
   MISSILE_DATA *m_next;
   SHIP_DATA *target;
   float dx, dy, dz, change;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch;
   bool ch_found = FALSE;

   for( missile = first_missile; missile; missile = m_next )
   {
      m_next = missile->next;

      ship = missile->fired_from;
      target = missile->target;
      if( !target )
      {
         extract_missile( missile );
         continue;
      }

      if( target->starsystem && target->starsystem == missile->starsystem )
      {

         if( missile->mx < target->vx )
            missile->mx += UMIN( missile->speed / 5, target->vx - missile->mx );
         else if( missile->mx > target->vx )
            missile->mx -= UMIN( missile->speed / 5, missile->mx - target->vx );
         if( missile->my < target->vy )
            missile->my += UMIN( missile->speed / 5, target->vy - missile->my );
         else if( missile->my > target->vy )
            missile->my -= UMIN( missile->speed / 5, missile->my - target->vy );
         if( missile->mz < target->vz )
            missile->mz += UMIN( missile->speed / 5, target->vz - missile->mz );
         else if( missile->mz > target->vz )
            missile->mz -= UMIN( missile->speed / 5, missile->mz - target->vz );

         if( abs( missile->mx ) - abs( target->vx ) <= 20 && abs( missile->mx ) - abs( target->vx ) >= -20
             && abs( missile->my ) - abs( target->vy ) <= 20 && abs( missile->my ) - abs( target->vy ) >= -20
             && abs( missile->mz ) - abs( target->vz ) <= 20 && abs( missile->mz ) - abs( target->vz ) >= -20 )
         {
            if( target->chaff_released <= 0 )
            {
               if( target->manuever / 2 < number_range( 1, 100 ) )
               {
                  if( target->juking == TRUE )
                  {
                     if( number_range( 1, 11 ) != 11 )
                     {
                        echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ), "Your missile barely misses its target." );
                        echo_to_cockpit( AT_ORANGE, target, "An incoming missile is barely evaded." );
                        sprintf( buf, "%s quickly evades an incoming missile.", target->name );
                        echo_to_system( AT_ORANGE, target, buf, ship );
                        extract_missile( missile );
                     }
                  }
                  else
                  {

                     echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ), "Your missile hits its target dead on!" );
                     echo_to_cockpit( AT_BLOOD, target, "The ship is hit by a missile." );
                     echo_to_ship( AT_RED, target, "A loud explosion shakes the ship violently!" );
                     sprintf( buf, "You see a small explosion as %s is hit by a missile", target->name );
                     echo_to_system( AT_ORANGE, target, buf, ship );
                     for( ch = first_char; ch; ch = ch->next )
                        if( !IS_NPC( ch ) && nifty_is_name( missile->fired_by, ch->name ) )
                        {
                           ch_found = TRUE;
                           damage_ship_ch( target, 20 + missile->missiletype * missile->missiletype * 20,
                                           30 + missile->missiletype * missile->missiletype * missile->missiletype * 30,
                                           ch );
                        }
                     if( !ch_found )
                        damage_ship( target, 20 + missile->missiletype * missile->missiletype * 20,
                                     30 + missile->missiletype * missile->missiletype * ship->missiletype * 30 );
                     extract_missile( missile );
                  }
               }
            }
            else
            {
               echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ),
                             "Your missile explodes harmlessly in a cloud of chaff!" );
               echo_to_cockpit( AT_YELLOW, target, "A missile explodes in your chaff." );
               extract_missile( missile );
            }
            continue;
         }

         missile->age++;
         if( missile->age >= 50 )
         {
            extract_missile( missile );
            continue;
         }
      }
      else
      {
         extract_missile( missile );
         continue;
      }

   }

   for( ship = first_ship; ship; ship = ship->next )
   {

      if( !ship || !ship->starsystem )
         continue;

      if( ship->currspeed > 0 )
      {

         change = sqrt( ship->hx * ship->hx + ship->hy * ship->hy + ship->hz * ship->hz );

         if( change > 0 )
         {
            dx = ship->hx / change;
            dy = ship->hy / change;
            dz = ship->hz / change;
            ship->vx += ( dx * ship->currspeed / 5 );
            ship->vy += ( dy * ship->currspeed / 5 );
            ship->vz += ( dz * ship->currspeed / 5 );
         }

      }

      if( autofly( ship ) )
         continue;

/*           
          if ( ship->class != SHIP_SPACE_STATION && !autofly(ship) )
          {
            if ( ship->starsystem->star1 && strcmp(ship->starsystem->star1,"") )
            {
              if (ship->vx >= ship->starsystem->s1x + 1 || ship->vx <= ship->starsystem->s1x - 1 )
                ship->vx -= URANGE(-3,(ship->starsystem->gravitys1)/(ship->vx - ship->starsystem->s1x)/2,3);
              if (ship->vy >= ship->starsystem->s1y + 1 || ship->vy <= ship->starsystem->s1y - 1 )
                ship->vy -= URANGE(-3,(ship->starsystem->gravitys1)/(ship->vy - ship->starsystem->s1y)/2,3);
              if (ship->vz >= ship->starsystem->s1z + 1 || ship->vz <= ship->starsystem->s1z - 1 )
                ship->vz -= URANGE(-3,(ship->starsystem->gravitys1)/(ship->vz - ship->starsystem->s1z)/2,3);
            }
          
            if ( ship->starsystem->star2 && strcmp(ship->starsystem->star2,"") )
            {
              if (ship->vx >= ship->starsystem->s2x + 1 || ship->vx <= ship->starsystem->s2x - 1 )
                ship->vx -= URANGE(-3,(ship->starsystem->gravitys2)/(ship->vx - ship->starsystem->s2x)/2,3);
              if (ship->vy >= ship->starsystem->s2y + 1 || ship->vy <= ship->starsystem->s2y - 1 )
                ship->vy -= URANGE(-3,(ship->starsystem->gravitys2)/(ship->vy - ship->starsystem->s2y)/2,3);
              if (ship->vz >= ship->starsystem->s2z + 1 || ship->vz <= ship->starsystem->s2z - 1 )
                ship->vz -= URANGE(-3,(ship->starsystem->gravitys2)/(ship->vz - ship->starsystem->s2z)/2,3);
            }
          
            if ( ship->starsystem->planet1 && strcmp(ship->starsystem->planet1,"") )
            {
              if (ship->vx >= ship->starsystem->p1x + 1 || ship->vx <= ship->starsystem->p1x - 1 )
                ship->vx -= URANGE(-3,(ship->starsystem->gravityp1)/(ship->vx - ship->starsystem->p1x)/2,3);
              if (ship->vy >= ship->starsystem->p1y + 1 || ship->vy <= ship->starsystem->p1y - 1 )
                ship->vy -= URANGE(-3,(ship->starsystem->gravityp1)/(ship->vy - ship->starsystem->p1y)/2,3);
              if (ship->vz >= ship->starsystem->p1z + 1 || ship->vz <= ship->starsystem->p1z - 1 )
                ship->vz -= URANGE(-3,(ship->starsystem->gravityp1)/(ship->vz - ship->starsystem->p1z)/2,3);
            }
          
            if ( ship->starsystem->planet2 && strcmp(ship->starsystem->planet2,"") )
            {
              if (ship->vx >= ship->starsystem->p2x + 1 || ship->vx <= ship->starsystem->p2x - 1 )
                ship->vx -= URANGE(-3,(ship->starsystem->gravityp2)/(ship->vx - ship->starsystem->p2x)/2,3);
              if (ship->vy >= ship->starsystem->p2y + 1 || ship->vy <= ship->starsystem->p2y - 1 )
                ship->vy -= URANGE(-3,(ship->starsystem->gravityp2)/(ship->vy - ship->starsystem->p2y)/2,3);
              if (ship->vz >= ship->starsystem->p2z + 1 || ship->vz <= ship->starsystem->p2z - 1 )
                ship->vz -= URANGE(-3,(ship->starsystem->gravityp2)/(ship->vz - ship->starsystem->p2z)/2,3);
            }
          
            if ( ship->starsystem->planet3 && strcmp(ship->starsystem->planet3,"") )
            {
              if (ship->vx >= ship->starsystem->p3x + 1 || ship->vx <= ship->starsystem->p3x - 1 )
                ship->vx -= URANGE(-3,(ship->starsystem->gravityp3)/(ship->vx - ship->starsystem->p3x)/2,3);
              if (ship->vy >= ship->starsystem->p3y + 1 || ship->vy <= ship->starsystem->p3y - 1 )
                ship->vy -= URANGE(-3,(ship->starsystem->gravityp3)/(ship->vy - ship->starsystem->p3y)/2,3);
              if (ship->vz >= ship->starsystem->p3z + 1 || ship->vz <= ship->starsystem->p3z - 1 )
                ship->vz -= URANGE(-3,(ship->starsystem->gravityp3)/(ship->vz - ship->starsystem->p3z)/2,3);
            }
          }

*/
/*
          for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem)
          { 
                if ( target != ship &&
                    abs(ship->vx - target->vx) < 1 &&
                    abs(ship->vy - target->vy) < 1 &&
                    abs(ship->vz - target->vz) < 1 )
                {
                    ship->collision = target->maxhull;
                    target->collision = ship->maxhull;
                }                   
          }
*/
      if( ship->starsystem->star1 && strcmp( ship->starsystem->star1, "" ) &&
          abs( ship->vx - ship->starsystem->s1x ) < 10 &&
          abs( ship->vy - ship->starsystem->s1y ) < 10 && abs( ship->vz - ship->starsystem->s1z ) < 10 )
      {
         echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "You fly directly into the sun." );
         sprintf( buf, "%s flies directly into %s!", ship->name, ship->starsystem->star1 );
         echo_to_system( AT_ORANGE, ship, buf, NULL );
         destroy_ship( ship, NULL, "flew into sun" );
         continue;
      }
      if( ship->starsystem->star2 && strcmp( ship->starsystem->star2, "" ) &&
          abs( ship->vx - ship->starsystem->s2x ) < 10 &&
          abs( ship->vy - ship->starsystem->s2y ) < 10 && abs( ship->vz - ship->starsystem->s2z ) < 10 )
      {
         echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "You fly directly into the sun." );
         sprintf( buf, "%s flys directly into %s!", ship->name, ship->starsystem->star2 );
         echo_to_system( AT_ORANGE, ship, buf, NULL );
         destroy_ship( ship, NULL, "flew into sun" );
         continue;
      }

      if( ship->currspeed > 0 )
      {
         if( ship->starsystem->planet1 && strcmp( ship->starsystem->planet1, "" ) &&
             abs( ship->vx - ship->starsystem->p1x ) < 10 &&
             abs( ship->vy - ship->starsystem->p1y ) < 10 && abs( ship->vz - ship->starsystem->p1z ) < 10 )
         {
            sprintf( buf, "You begin orbitting %s.", ship->starsystem->planet1 );
            echo_to_cockpit( AT_YELLOW, ship, buf );
            sprintf( buf, "%s begins orbitting %s.", ship->name, ship->starsystem->planet1 );
            echo_to_system( AT_ORANGE, ship, buf, NULL );
            ship->currspeed = 0;
            continue;
         }
         if( ship->starsystem->planet2 && strcmp( ship->starsystem->planet2, "" ) &&
             abs( ship->vx - ship->starsystem->p2x ) < 10 &&
             abs( ship->vy - ship->starsystem->p2y ) < 10 && abs( ship->vz - ship->starsystem->p2z ) < 10 )
         {
            sprintf( buf, "You begin orbitting %s.", ship->starsystem->planet2 );
            echo_to_cockpit( AT_YELLOW, ship, buf );
            sprintf( buf, "%s begins orbitting %s.", ship->name, ship->starsystem->planet2 );
            echo_to_system( AT_ORANGE, ship, buf, NULL );
            ship->currspeed = 0;
            continue;
         }
         if( ship->starsystem->planet3 && strcmp( ship->starsystem->planet3, "" ) &&
             abs( ship->vx - ship->starsystem->p3x ) < 10 &&
             abs( ship->vy - ship->starsystem->p3y ) < 10 && abs( ship->vz - ship->starsystem->p3z ) < 10 )
         {
            sprintf( buf, "You begin orbitting %s.", ship->starsystem->planet2 );
            echo_to_cockpit( AT_YELLOW, ship, buf );
            sprintf( buf, "%s begins orbitting %s.", ship->name, ship->starsystem->planet2 );
            echo_to_system( AT_ORANGE, ship, buf, NULL );
            ship->currspeed = 0;
            continue;
         }
      }
   }

   for( ship = first_ship; ship; ship = ship->next )
      if( ship->collision )
      {
         echo_to_cockpit( AT_WHITE + AT_BLINK, ship, "You have collided with another ship!" );
         echo_to_ship( AT_RED, ship, "A loud explosion shakes the ship violently!" );
         damage_ship( ship, ship->collision, ship->collision );
         ship->collision = 0;
      }
}

void recharge_ships(  )
{
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];

   for( ship = first_ship; ship; ship = ship->next )
   {

      if( ship->primaryState > 0 )
      {
         if( ship->primaryType != AUTOBLASTER )
            ship->energy -= ship->primaryState;

         ship->primaryState = 0;
      }
      if( ship->secondaryState > 0 )
      {
         if( ship->secondaryType != AUTOBLASTER )
            ship->energy -= ship->secondaryState;

         ship->secondaryState = 0;
      }
      if( ship->statet1 > 0 )
      {
         ship->energy -= ship->statet1;
         ship->statet1 = 0;
      }
      if( ship->statet2 > 0 )
      {
         ship->energy -= ship->statet2;
         ship->statet2 = 0;
      }
      if( ship->statet3 > 0 )
      {
         ship->energy -= ship->statet3;
         ship->statet3 = 0;
      }
      if( ship->statet4 > 0 )
      {
         ship->energy -= ship->statet4;
         ship->statet4 = 0;
      }
      if( ship->statet5 > 0 )
      {
         ship->energy -= ship->statet5;
         ship->statet5 = 0;
      }
      if( ship->statet6 > 0 )
      {
         ship->energy -= ship->statet6;
         ship->statet6 = 0;
      }
      if( ship->statet7 > 0 )
      {
         ship->energy -= ship->statet7;
         ship->statet7 = 0;
      }
      if( ship->statet8 > 0 )
      {
         ship->energy -= ship->statet8;
         ship->statet8 = 0;
      }
      if( ship->statet9 > 0 )
      {
         ship->energy -= ship->statet9;
         ship->statet9 = 0;
      }
      if( ship->statet10 > 0 )
      {
         ship->energy -= ship->statet10;
         ship->statet10 = 0;
      }


      if( ship->missilestate == MISSILE_RELOAD_2 )
      {
         ship->missilestate = MISSILE_READY;
         if( ship->missiles > 0 )
            echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ), "Missile launcher reloaded." );
      }

      if( ship->missilestate == MISSILE_RELOAD )
      {
         ship->missilestate = MISSILE_RELOAD_2;
      }

      if( ship->missilestate == MISSILE_FIRED )
         ship->missilestate = MISSILE_RELOAD;

      if( ship->torpedostate == MISSILE_RELOAD_2 )
      {
         ship->torpedostate = MISSILE_READY;
         if( ship->missiles > 0 )
            echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ), "Torpedo launcher reloaded." );
      }

      if( ship->torpedostate == MISSILE_RELOAD )
      {
         ship->torpedostate = MISSILE_RELOAD_2;
      }

      if( ship->torpedostate == MISSILE_FIRED )
         ship->torpedostate = MISSILE_RELOAD;

      if( ship->rocketstate == MISSILE_RELOAD_2 )
      {
         ship->rocketstate = MISSILE_READY;
         if( ship->rockets > 0 )
            echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ), "Rocket launcher reloaded." );
      }

      if( ship->rocketstate == MISSILE_RELOAD )
      {
         ship->rocketstate = MISSILE_RELOAD_2;
      }

      if( ship->rocketstate == MISSILE_FIRED )
         ship->rocketstate = MISSILE_RELOAD;

      if( autofly( ship ) )
      {
         if( ship->starsystem )
         {
            if( ship->target0 && ship->primaryState != LASER_DAMAGED )
            {
               int schance = 50;
               SHIP_DATA *target = ship->target0;
               int shots;

               for( shots = 0; shots <= ship->primaryCount && ship->target0; shots++ )
               {
                  if( ship->shipstate != SHIP_HYPERSPACE && ship->energy > 25
                      && ship->target0->starsystem == ship->starsystem
                      && abs( target->vx - ship->vx ) <= 1000
                      && abs( target->vy - ship->vy ) <= 1000
                      && abs( target->vz - ship->vz ) <= 1000 && ship->primaryState < ship->primaryCount )
                  {
                     if( ship->class >= SHIP_SHUTTLE || is_facing( ship, target ) )
                     {
                        schance += target->class * 25;
                        schance -= target->manuever / 10;
                        schance -= target->currspeed / 20;
                        schance -= ( abs( target->vx - ship->vx ) / 70 );
                        schance -= ( abs( target->vy - ship->vy ) / 70 );
                        schance -= ( abs( target->vz - ship->vz ) / 70 );
                        schance = URANGE( 10, schance, 90 );
                        if( number_percent(  ) > schance )
                        {
                           sprintf( buf, "%s fires at you but misses.", ship->name );
                           echo_to_cockpit( AT_ORANGE, target, buf );
                           sprintf( buf, "Laserfire from %s barely misses %s.", ship->name, target->name );
                           echo_to_system( AT_ORANGE, target, buf, NULL );
                        }
                        else
                        {
                           sprintf( buf, "Laserfire from %s hits %s.", ship->name, target->name );
                           echo_to_system( AT_ORANGE, target, buf, NULL );
                           sprintf( buf, "You are hit by lasers from %s!", ship->name );
                           echo_to_cockpit( AT_BLOOD, target, buf );
                           echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
                           damage_ship( target, 5, 10 );
                        }
                        ship->primaryState++;
                     }
                  }
               }
            }
         }
      }

   }
}

void update_space(  )
{
   SHIP_DATA *ship;
   SHIP_DATA *target;
   char buf[MAX_STRING_LENGTH];
   int too_close, target_too_close;
   int recharge;

   for( ship = first_ship; ship; ship = ship->next )
   {
      if( ship->starsystem )
      {
         if( ship->energy > 0 && ship->shipstate == SHIP_DISABLED && ship->class != SHIP_SPACE_STATION )
            ship->energy -= 100;
         else if( ship->energy > 0 )
            ship->energy += ( 5 + ship->class * 5 );
         else if( ship->type == MOB_SHIP )
            ship->energy = ship->maxenergy;
         else
            destroy_ship( ship, NULL, "destroyed by lack of energy" );
      }

      if( ship->chaff_released > 0 )
         ship->chaff_released--;

      if( ship->shipstate == SHIP_HYPERSPACE )
      {
         switch ( ship->hyperspeed )
         {
            case 1:
               ship->hyperdistance -= number_range( 170, 225 );
               break;
            case 2:
               ship->hyperdistance -= number_range( 135, 180 );
               break;
            case 3:
               ship->hyperdistance -= number_range( 80, 110 );
               break;
            case 4:
               ship->hyperdistance -= number_range( 60, 100 );
               break;
            case 5:
               ship->hyperdistance -= number_range( 40, 60 );
               break;
            case 6:
               ship->hyperdistance -= number_range( 20, 45 );
               break;
            default:
               ship->hyperdistance -= 20;
               break;
         }
         if( ship->hyperdistance <= 0 )
         {
            ship_to_starsystem( ship, ship->currjump );

            if( ship->starsystem == NULL )
            {
               echo_to_cockpit( AT_RED, ship, "Ship lost in Hyperspace. Make new calculations." );
            }
            else
            {
               echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Hyperjump complete." );
               echo_to_ship( AT_YELLOW, ship, "The ship lurches slightly as it comes out of hyperspace." );
               sprintf( buf, "%s enters the starsystem at %.0f %.0f %.0f", ship->name, ship->vx, ship->vy, ship->vz );
               echo_to_system( AT_YELLOW, ship, buf, NULL );
               ship->shipstate = SHIP_READY;
               STRFREE( ship->home );
               ship->home = STRALLOC( ship->starsystem->name );
               if( str_cmp( "Public", ship->owner ) )
                  save_ship( ship );

            }
         }
         else
         {
            sprintf( buf, "%d", ship->hyperdistance );
            echo_to_room_dnr( AT_YELLOW, get_room_index( ship->pilotseat ), "Remaining jump distance: " );
            echo_to_room( AT_WHITE, get_room_index( ship->pilotseat ), buf );

         }
      }

      /*
       * following was originaly to fix ships that lost their pilot 
       * in the middle of a manuever and are stuck in a busy state 
       * but now used for timed manouevers such as turning 
       */

      if( ship->shipstate == SHIP_BUSY_3 )
      {
         echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Manuever complete." );
         ship->shipstate = SHIP_READY;
      }
      if( ship->shipstate == SHIP_BUSY_2 )
         ship->shipstate = SHIP_BUSY_3;
      if( ship->shipstate == SHIP_BUSY )
         ship->shipstate = SHIP_BUSY_2;

      if( ship->shipstate == SHIP_LAND_2 )
         landship( ship, ship->dest );
      if( ship->shipstate == SHIP_LAND )
         ship->shipstate = SHIP_LAND_2;

      if( ship->shipstate == SHIP_LAUNCH_2 )
         launchship( ship );
      if( ship->shipstate == SHIP_LAUNCH )
         ship->shipstate = SHIP_LAUNCH_2;


      ship->shield = UMAX( 0, ship->shield - 1 - ship->class );

      if( ship->autorecharge && ship->maxshield > ship->shield && ship->energy > 100 )
      {
         recharge = UMIN( ship->maxshield - ship->shield, 10 + ship->class * 10 );
         recharge = UMIN( recharge, ship->energy / 2 - 100 );
         recharge = UMAX( 1, recharge );
         ship->shield += recharge;
         ship->energy -= recharge;
      }

      if( ship->shield > 0 )
      {
         if( ship->energy < 200 )
         {
            ship->shield = 0;
            echo_to_cockpit( AT_RED, ship, "The ships shields fizzle and die." );
            ship->autorecharge = FALSE;
         }
      }

      if( ship->starsystem && ship->currspeed > 0 )
      {
         sprintf( buf, "%d  ", ship->currspeed );
         echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), "Speed: " );
         echo_to_room_dnr( AT_LBLUE, get_room_index( ship->pilotseat ), buf );
         if( ship->maxhull > 0 )
         {

            sprintf( buf, "%d%%  ", ( ship->hull * 100 ) / ship->maxhull );
            echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), "Hull: " );
            echo_to_room_dnr( AT_LBLUE, get_room_index( ship->pilotseat ), buf );
         }

         if( ship->maxshield > 0 )
         {
            sprintf( buf, "%d%%  ", ( ship->shield * 100 ) / ship->maxshield );
            echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), "Shields: " );
            echo_to_room_dnr( AT_LBLUE, get_room_index( ship->pilotseat ), buf );
         }
         if( ship->maxenergy > 0 )
         {
            sprintf( buf, "%d%%  ", ( ship->energy * 100 ) / ship->maxenergy );
            echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), "Energy: " );
            echo_to_room( AT_LBLUE, get_room_index( ship->pilotseat ), buf );
         }
         echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), "Coordinates in " );
         if( ship->starsystem )
            sprintf( buf, "&C%s&B system: ", ship->starsystem->name );
         else
            sprintf( buf, "system: " );
         echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), buf );
         sprintf( buf, "%.0f %.0f %.0f", ship->vx, ship->vy, ship->vz );
         echo_to_room( AT_LBLUE, get_room_index( ship->pilotseat ), buf );


         if( ship->pilotseat != ship->coseat )
         {
            sprintf( buf, "%d  ", ship->currspeed );
            echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), "Speed: " );
            echo_to_room_dnr( AT_LBLUE, get_room_index( ship->coseat ), buf );
            if( ship->maxhull > 0 )
            {

               sprintf( buf, "%d%%  ", ( ship->hull * 100 ) / ship->maxhull );
               echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), "Hull: " );
               echo_to_room_dnr( AT_LBLUE, get_room_index( ship->coseat ), buf );
            }

            if( ship->maxshield > 0 )
            {
               sprintf( buf, "%d%%  ", ( ship->shield * 100 ) / ship->maxshield );
               echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), "Shields: " );
               echo_to_room_dnr( AT_LBLUE, get_room_index( ship->coseat ), buf );
            }
            if( ship->maxenergy > 0 )
            {
               sprintf( buf, "%d%%  ", ( ship->energy * 100 ) / ship->maxenergy );
               echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), "Energy: " );
               echo_to_room( AT_LBLUE, get_room_index( ship->coseat ), buf );
            }
            echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), "Coordinates in " );
            if( ship->starsystem )
               sprintf( buf, "&C%s&B system: ", ship->starsystem->name );
            else
               sprintf( buf, "system: " );
            echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), buf );
            sprintf( buf, "%.0f %.0f %.0f", ship->vx, ship->vy, ship->vz );
            echo_to_room( AT_LBLUE, get_room_index( ship->coseat ), buf );
         }
      }

      if( ship->starsystem )
      {
         too_close = ship->currspeed + 50;
         for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
         {
            target_too_close = too_close + target->currspeed;
            if( target != ship &&
                abs( ship->vx - target->vx ) < target_too_close &&
                abs( ship->vy - target->vy ) < target_too_close && abs( ship->vz - target->vz ) < target_too_close )
            {
               sprintf( buf, "Proximity alert: %s  %.0f %.0f %.0f", target->name, target->vx, target->vy, target->vz );
               echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
            }
         }
         too_close = ship->currspeed + 100;
         if( ship->starsystem->star1 && strcmp( ship->starsystem->star1, "" ) &&
             abs( ship->vx - ship->starsystem->s1x ) < too_close &&
             abs( ship->vy - ship->starsystem->s1y ) < too_close && abs( ship->vz - ship->starsystem->s1z ) < too_close )
         {
            sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->star1,
                     ship->starsystem->s1x, ship->starsystem->s1y, ship->starsystem->s1z );
            echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
         }
         if( ship->starsystem->star2 && strcmp( ship->starsystem->star2, "" ) &&
             abs( ship->vx - ship->starsystem->s2x ) < too_close &&
             abs( ship->vy - ship->starsystem->s2y ) < too_close && abs( ship->vz - ship->starsystem->s2z ) < too_close )
         {
            sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->star2,
                     ship->starsystem->s2x, ship->starsystem->s2y, ship->starsystem->s2z );
            echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
         }
         if( ship->starsystem->planet1 && strcmp( ship->starsystem->planet1, "" ) &&
             abs( ship->vx - ship->starsystem->p1x ) < too_close &&
             abs( ship->vy - ship->starsystem->p1y ) < too_close && abs( ship->vz - ship->starsystem->p1z ) < too_close )
         {
            sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->planet1,
                     ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
            echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
         }
         if( ship->starsystem->planet2 && strcmp( ship->starsystem->planet2, "" ) &&
             abs( ship->vx - ship->starsystem->p2x ) < too_close &&
             abs( ship->vy - ship->starsystem->p2y ) < too_close && abs( ship->vz - ship->starsystem->p2z ) < too_close )
         {
            sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->planet2,
                     ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
            echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
         }
         if( ship->starsystem->planet3 && strcmp( ship->starsystem->planet3, "" ) &&
             abs( ship->vx - ship->starsystem->p3x ) < too_close &&
             abs( ship->vy - ship->starsystem->p3y ) < too_close && abs( ship->vz - ship->starsystem->p3z ) < too_close )
         {
            sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->planet3,
                     ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
            echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
         }
      }


      if( ship->target0 )
      {
         sprintf( buf, "%s   %.0f %.0f %.0f", ship->target0->name, ship->target0->vx, ship->target0->vy, ship->target0->vz );
         echo_to_room_dnr( AT_BLUE, get_room_index( ship->gunseat ), "Target: " );
         echo_to_room( AT_LBLUE, get_room_index( ship->gunseat ), buf );
         if( ship->starsystem != ship->target0->starsystem )
            ship->target0 = NULL;
      }

      if( ship->target1 )
      {
         sprintf( buf, "%s   %.0f %.0f %.0f", ship->target1->name, ship->target1->vx, ship->target1->vy, ship->target1->vz );
         echo_to_room_dnr( AT_BLUE, get_room_index( ship->turret1 ), "Target: " );
         echo_to_room( AT_LBLUE, get_room_index( ship->turret1 ), buf );
         if( ship->starsystem != ship->target1->starsystem )
            ship->target1 = NULL;
      }

      if( ship->target2 )
      {
         sprintf( buf, "%s   %.0f %.0f %.0f", ship->target2->name, ship->target2->vx, ship->target2->vy, ship->target2->vz );
         echo_to_room_dnr( AT_BLUE, get_room_index( ship->turret2 ), "Target: " );
         echo_to_room( AT_LBLUE, get_room_index( ship->turret2 ), buf );
         if( ship->starsystem != ship->target2->starsystem )
            ship->target2 = NULL;
      }

      if( ship->energy < 100 && ship->starsystem )
      {
         echo_to_cockpit( AT_RED, ship, "Warning: Ship fuel low." );
      }

      ship->energy = URANGE( 0, ship->energy, ship->maxenergy );
   }

   for( ship = first_ship; ship; ship = ship->next )
   {

      if( ship->autotrack && ship->target0 && ship->class < SHIP_CRUISER )
      {
         target = ship->target0;
         too_close = ship->currspeed + 10;
         target_too_close = too_close + target->currspeed;
         if( target != ship && ship->shipstate == SHIP_READY &&
             abs( ship->vx - target->vx ) < target_too_close &&
             abs( ship->vy - target->vy ) < target_too_close && abs( ship->vz - target->vz ) < target_too_close )
         {
            ship->hx = 0 - ( ship->target0->vx - ship->vx );
            ship->hy = 0 - ( ship->target0->vy - ship->vy );
            ship->hz = 0 - ( ship->target0->vz - ship->vz );
            ship->energy -= ship->currspeed / 10;
            echo_to_room( AT_RED, get_room_index( ship->pilotseat ), "Autotrack: Evading to avoid collision!\n\r" );
            if( ( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
                || ( ( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE ) && ship->manuever > 50 ) )
               ship->shipstate = SHIP_BUSY_3;
            else if( ship->class >= SHIP_CRUISER || ( ship->class == SHIP_DREADNAUGHT && ship->manuever > 50 ) )
               ship->shipstate = SHIP_BUSY_2;
            else
               ship->shipstate = SHIP_BUSY;
         }
         else if( !is_facing( ship, ship->target0 ) )
         {
            ship->hx = ship->target0->vx - ship->vx;
            ship->hy = ship->target0->vy - ship->vy;
            ship->hz = ship->target0->vz - ship->vz;
            ship->energy -= ship->currspeed / 10;
            echo_to_room( AT_BLUE, get_room_index( ship->pilotseat ), "Autotracking target ... setting new course.\n\r" );
            if( ( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
                || ( ( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE ) && ship->manuever > 50 ) )
               ship->shipstate = SHIP_BUSY_3;
            else if( ship->class >= SHIP_CRUISER || ( ship->class == SHIP_DREADNAUGHT && ship->manuever > 50 ) )
               ship->shipstate = SHIP_BUSY_2;
            else
               ship->shipstate = SHIP_BUSY;
         }
      }

      if( autofly( ship ) )
      {
         if( ship->starsystem )
         {
            if( ship->target0 )
            {
               int schance = 50;

               /*
                * auto assist ships 
                */

               for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
               {
                  if( autofly( target ) )
                  {
                     if( IS_SET( target->flags, SHIP_SIMULATOR ) )
                        continue;
                     if( !str_cmp( target->owner, ship->owner ) && target != ship )
                        if( target->target0 == NULL && ship->target0 != target )
                        {
                           target->target0 = ship->target0;
                           sprintf( buf, "You are being targetted by %s.", target->name );
                           echo_to_cockpit( AT_BLOOD, target->target0, buf );
                           break;
                        }
                  }
               }

               target = ship->target0;
               ship->autotrack = TRUE;
               if( ship->class != SHIP_SPACE_STATION )
                  ship->currspeed = ship->realspeed;
               if( ship->energy > 200 )
                  ship->autorecharge = TRUE;


               if( ship->shipstate != SHIP_HYPERSPACE && ship->energy > 25
                   && ship->missilestate == MISSILE_READY && ship->target0->starsystem == ship->starsystem
                   && abs( target->vx - ship->vx ) <= 1200
                   && abs( target->vy - ship->vy ) <= 1200 && abs( target->vz - ship->vz ) <= 1200 && ship->missiles > 0 )
               {
                  if( ship->class >= SHIP_CRUISER || is_facing( ship, target ) )
                  {
                     schance -= target->manuever / 5;
                     schance -= target->currspeed / 20;
                     schance += target->class * target->class * 25;
                     schance -= ( abs( target->vx - ship->vx ) / 100 );
                     schance -= ( abs( target->vy - ship->vy ) / 100 );
                     schance -= ( abs( target->vz - ship->vz ) / 100 );
                     schance += ( 30 );
                     schance = URANGE( 10, schance, 90 );

                     if( number_percent(  ) > schance )
                     {
                     }
                     else
                     {
                        new_missile( ship, target, NULL, CONCUSSION_MISSILE );
                        ship->missiles--;
                        sprintf( buf, "Incoming missile from %s.", ship->name );
                        echo_to_cockpit( AT_BLOOD, target, buf );
                        sprintf( buf, "%s fires a missile towards %s.", ship->name, target->name );
                        echo_to_system( AT_ORANGE, target, buf, NULL );

                        if( ship->class >= SHIP_CRUISER )
                           ship->missilestate = MISSILE_RELOAD_2;
                        else
                           ship->missilestate = MISSILE_FIRED;
                     }
                  }
               }

               if( ship->missilestate == MISSILE_DAMAGED )
                  ship->missilestate = MISSILE_READY;
               if( ship->primaryState == LASER_DAMAGED )
                  ship->primaryState = LASER_READY;
               if( ship->secondaryState == LASER_DAMAGED )
                  ship->secondaryState = LASER_READY;
               if( ship->shipstate == SHIP_DISABLED )
                  ship->shipstate = SHIP_READY;

            }
            else
            {
               ship->currspeed = 0;

               if( !str_cmp( ship->owner, "The Empire" ) )
                  for( target = first_ship; target; target = target->next )
                     if( ship->starsystem == target->starsystem )
                        if( !str_cmp( target->owner, "The New Republic" ) )
                        {
                           ship->target0 = target;
                           sprintf( buf, "You are being targetted by %s.", ship->name );
                           echo_to_cockpit( AT_BLOOD, target, buf );
                           break;
                        }
               if( !str_cmp( ship->owner, "The New Republic" ) )
                  for( target = first_ship; target; target = target->next )
                     if( ship->starsystem == target->starsystem )
                        if( !str_cmp( target->owner, "The Empire" ) )
                        {
                           sprintf( buf, "You are being targetted by %s.", ship->name );
                           echo_to_cockpit( AT_BLOOD, target, buf );
                           ship->target0 = target;
                           break;
                        }

               if( !str_cmp( ship->owner, "Pirates" ) )
                  for( target = first_ship; target; target = target->next )
                     if( ship->starsystem == target->starsystem )
                     {
                        sprintf( buf, "You are being targetted by %s.", ship->name );
                        echo_to_cockpit( AT_BLOOD, target, buf );
                        ship->target0 = target;
                        break;
                     }

            }
         }
         else
         {
            if( number_range( 1, 25 ) == 25 )
            {
               ship_to_starsystem( ship, starsystem_from_name( ship->home ) );
               ship->vx = number_range( -5000, 5000 );
               ship->vy = number_range( -5000, 5000 );
               ship->vz = number_range( -5000, 5000 );
               ship->hx = 1;
               ship->hy = 1;
               ship->hz = 1;
            }
         }
      }

      if( ( ship->class >= SHIP_CRUISER ) && ship->target0 == NULL )
      {
         if( ship->missiles < ship->maxmissiles )
            ship->missiles++;
         if( ship->torpedos < ship->maxtorpedos )
            ship->torpedos++;
         if( ship->rockets < ship->maxrockets )
            ship->rockets++;
      }
   }

}



void write_starsystem_list(  )
{
   SPACE_DATA *tstarsystem;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", SPACE_DIR, SPACE_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open starsystem.lst for writing!\n\r", 0 );
      return;
   }
   for( tstarsystem = first_starsystem; tstarsystem; tstarsystem = tstarsystem->next )
      fprintf( fpout, "%s\n", tstarsystem->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}


/*
 * Get pointer to space structure from starsystem name.
 */
SPACE_DATA *starsystem_from_name( char *name )
{
   SPACE_DATA *starsystem;

   for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
      if( !str_cmp( name, starsystem->name ) )
         return starsystem;

   for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
      if( !str_prefix( name, starsystem->name ) )
         return starsystem;

   return NULL;
}

/*
 * Get pointer to space structure from the dock vnun.
 */
SPACE_DATA *starsystem_from_vnum( int vnum )
{
   SPACE_DATA *starsystem;
   SHIP_DATA *ship;

   for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
      if( vnum == starsystem->doc1a || vnum == starsystem->doc2a || vnum == starsystem->doc3a ||
          vnum == starsystem->doc1b || vnum == starsystem->doc2b || vnum == starsystem->doc3b ||
          vnum == starsystem->doc1c || vnum == starsystem->doc2c || vnum == starsystem->doc3c )
         return starsystem;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->hanger1 || vnum == ship->hanger2 || vnum == ship->hanger3 || vnum == ship->hanger4 )
         return ship->starsystem;

   return NULL;
}

SPACE_DATA *starsystem_from_room( ROOM_INDEX_DATA * room )
{
   SHIP_DATA *ship;
//    ROOM_INDEX_DATA  *sRoom;

   if( room == NULL )
      return NULL;

   if( room->area != NULL && room->area->planet != NULL )
      return room->area->planet->starsystem;

   for( ship = first_ship; ship; ship = ship->next )

      //for ( sRoom->vnum = ship->firstroom ; sRoom ; sRoom = sRoom->next_in_ship )
      //if ( room == sRoom )
   {
      if( room->vnum == ship->hanger1 || room->vnum == ship->hanger2
          || room->vnum == ship->hanger3 || room->vnum == ship->hanger4 )
         return ship->starsystem;
   }

   return NULL;
}



/*
 * Save a starsystem's data to its data file
 */
void save_starsystem( SPACE_DATA * starsystem )
{
   FILE *fp;
   char filename[256];
   char buf[MAX_STRING_LENGTH];

   if( !starsystem )
   {
      bug( "save_starsystem: null starsystem pointer!", 0 );
      return;
   }

   if( !starsystem->filename || starsystem->filename[0] == '\0' )
   {
      sprintf( buf, "save_starsystem: %s has no filename", starsystem->name );
      bug( buf, 0 );
      return;
   }

   sprintf( filename, "%s%s", SPACE_DIR, starsystem->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_starsystem: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#SPACE\n" );
      fprintf( fp, "Name         %s~\n", starsystem->name );
      fprintf( fp, "Filename     %s~\n", starsystem->filename );
      fprintf( fp, "Planet1      %s~\n", starsystem->planet1 );
      fprintf( fp, "Planet2      %s~\n", starsystem->planet2 );
      fprintf( fp, "Planet3      %s~\n", starsystem->planet3 );
      fprintf( fp, "Star1        %s~\n", starsystem->star1 );
      fprintf( fp, "Star2        %s~\n", starsystem->star2 );
      fprintf( fp, "Location1a      %s~\n", starsystem->location1a );
      fprintf( fp, "Location1b      %s~\n", starsystem->location1b );
      fprintf( fp, "Location1c      %s~\n", starsystem->location1c );
      fprintf( fp, "Location2a       %s~\n", starsystem->location2a );
      fprintf( fp, "Location2b      %s~\n", starsystem->location2b );
      fprintf( fp, "Location2c      %s~\n", starsystem->location2c );
      fprintf( fp, "Location3a      %s~\n", starsystem->location3a );
      fprintf( fp, "Location3b      %s~\n", starsystem->location3b );
      fprintf( fp, "Location3c      %s~\n", starsystem->location3c );
      fprintf( fp, "Doc1a          %d\n", starsystem->doc1a );
      fprintf( fp, "Doc2a          %d\n", starsystem->doc2a );
      fprintf( fp, "Doc3a          %d\n", starsystem->doc3a );
      fprintf( fp, "Doc1b          %d\n", starsystem->doc1b );
      fprintf( fp, "Doc2b          %d\n", starsystem->doc2b );
      fprintf( fp, "Doc3b          %d\n", starsystem->doc3b );
      fprintf( fp, "Doc1c          %d\n", starsystem->doc1c );
      fprintf( fp, "Doc2c          %d\n", starsystem->doc2c );
      fprintf( fp, "Doc3c          %d\n", starsystem->doc3c );
      fprintf( fp, "P1x          %d\n", starsystem->p1x );
      fprintf( fp, "P1y          %d\n", starsystem->p1y );
      fprintf( fp, "P1z          %d\n", starsystem->p1z );
      fprintf( fp, "P2x          %d\n", starsystem->p2x );
      fprintf( fp, "P2y          %d\n", starsystem->p2y );
      fprintf( fp, "P2z          %d\n", starsystem->p2z );
      fprintf( fp, "P3x          %d\n", starsystem->p3x );
      fprintf( fp, "P3y          %d\n", starsystem->p3y );
      fprintf( fp, "P3z          %d\n", starsystem->p3z );
      fprintf( fp, "S1x          %d\n", starsystem->s1x );
      fprintf( fp, "S1y          %d\n", starsystem->s1y );
      fprintf( fp, "S1z          %d\n", starsystem->s1z );
      fprintf( fp, "S2x          %d\n", starsystem->s2x );
      fprintf( fp, "S2y          %d\n", starsystem->s2y );
      fprintf( fp, "S2z          %d\n", starsystem->s2z );
      fprintf( fp, "Gravitys1     %d\n", starsystem->gravitys1 );
      fprintf( fp, "Gravitys2     %d\n", starsystem->gravitys2 );
      fprintf( fp, "Gravityp1     %d\n", starsystem->gravityp1 );
      fprintf( fp, "Gravityp2     %d\n", starsystem->gravityp2 );
      fprintf( fp, "Gravityp3     %d\n", starsystem->gravityp3 );
      fprintf( fp, "Xpos          %d\n", starsystem->xpos );
      fprintf( fp, "Ypos          %d\n", starsystem->ypos );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}


/*
 * Read in actual starsystem data.
 */
void fread_starsystem( SPACE_DATA * starsystem, FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;


   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'D':
            KEY( "Doc1a", starsystem->doc1a, fread_number( fp ) );
            KEY( "Doc2a", starsystem->doc2a, fread_number( fp ) );
            KEY( "Doc3a", starsystem->doc3a, fread_number( fp ) );
            KEY( "Doc1b", starsystem->doc1b, fread_number( fp ) );
            KEY( "Doc2b", starsystem->doc2b, fread_number( fp ) );
            KEY( "Doc3b", starsystem->doc3b, fread_number( fp ) );
            KEY( "Doc1c", starsystem->doc1c, fread_number( fp ) );
            KEY( "Doc2c", starsystem->doc2c, fread_number( fp ) );
            KEY( "Doc3c", starsystem->doc3c, fread_number( fp ) );
            break;


         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !starsystem->name )
                  starsystem->name = STRALLOC( "" );
               if( !starsystem->location1a )
                  starsystem->location1a = STRALLOC( "" );
               if( !starsystem->location2a )
                  starsystem->location2a = STRALLOC( "" );
               if( !starsystem->location3a )
                  starsystem->location3a = STRALLOC( "" );
               if( !starsystem->location1b )
                  starsystem->location1b = STRALLOC( "" );
               if( !starsystem->location2b )
                  starsystem->location2b = STRALLOC( "" );
               if( !starsystem->location3b )
                  starsystem->location3b = STRALLOC( "" );
               if( !starsystem->location1c )
                  starsystem->location1c = STRALLOC( "" );
               if( !starsystem->location2c )
                  starsystem->location2c = STRALLOC( "" );
               if( !starsystem->location3c )
                  starsystem->location3c = STRALLOC( "" );
               if( !starsystem->planet1 )
                  starsystem->planet1 = STRALLOC( "" );
               if( !starsystem->planet2 )
                  starsystem->planet2 = STRALLOC( "" );
               if( !starsystem->planet3 )
                  starsystem->planet3 = STRALLOC( "" );
               if( !starsystem->star1 )
                  starsystem->star1 = STRALLOC( "" );
               if( !starsystem->star2 )
                  starsystem->star2 = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Filename", starsystem->filename, fread_string_nohash( fp ) );
            break;

         case 'G':
            KEY( "Gravitys1", starsystem->gravitys1, fread_number( fp ) );
            KEY( "Gravitys2", starsystem->gravitys2, fread_number( fp ) );
            KEY( "Gravityp1", starsystem->gravityp1, fread_number( fp ) );
            KEY( "Gravityp2", starsystem->gravityp2, fread_number( fp ) );
            KEY( "Gravityp3", starsystem->gravityp3, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Location1a", starsystem->location1a, fread_string( fp ) );
            KEY( "Location2a", starsystem->location2a, fread_string( fp ) );
            KEY( "Location3a", starsystem->location3a, fread_string( fp ) );
            KEY( "Location1b", starsystem->location1b, fread_string( fp ) );
            KEY( "Location2b", starsystem->location2b, fread_string( fp ) );
            KEY( "Location3b", starsystem->location3b, fread_string( fp ) );
            KEY( "Location1c", starsystem->location1c, fread_string( fp ) );
            KEY( "Location2c", starsystem->location2c, fread_string( fp ) );
            KEY( "Location3c", starsystem->location3c, fread_string( fp ) );
            break;

         case 'N':
            KEY( "Name", starsystem->name, fread_string( fp ) );
            break;

         case 'P':
            KEY( "Planet1", starsystem->planet1, fread_string( fp ) );
            KEY( "Planet2", starsystem->planet2, fread_string( fp ) );
            KEY( "Planet3", starsystem->planet3, fread_string( fp ) );
            KEY( "P1x", starsystem->p1x, fread_number( fp ) );
            KEY( "P1y", starsystem->p1y, fread_number( fp ) );
            KEY( "P1z", starsystem->p1z, fread_number( fp ) );
            KEY( "P2x", starsystem->p2x, fread_number( fp ) );
            KEY( "P2y", starsystem->p2y, fread_number( fp ) );
            KEY( "P2z", starsystem->p2z, fread_number( fp ) );
            KEY( "P3x", starsystem->p3x, fread_number( fp ) );
            KEY( "P3y", starsystem->p3y, fread_number( fp ) );
            KEY( "P3z", starsystem->p3z, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Star1", starsystem->star1, fread_string( fp ) );
            KEY( "Star2", starsystem->star2, fread_string( fp ) );
            KEY( "S1x", starsystem->s1x, fread_number( fp ) );
            KEY( "S1y", starsystem->s1y, fread_number( fp ) );
            KEY( "S1z", starsystem->s1z, fread_number( fp ) );
            KEY( "S2x", starsystem->s2x, fread_number( fp ) );
            KEY( "S2y", starsystem->s2y, fread_number( fp ) );
            KEY( "S2z", starsystem->s2z, fread_number( fp ) );

         case 'X':
            KEY( "Xpos", starsystem->xpos, fread_number( fp ) );

         case 'Y':
            KEY( "Ypos", starsystem->ypos, fread_number( fp ) );

      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_starsystem: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

/*
 * Load a starsystem file
 */

bool load_starsystem( char *starsystemfile )
{
   char filename[256];
   SPACE_DATA *starsystem;
   FILE *fp;
   bool found;

   CREATE( starsystem, SPACE_DATA, 1 );

   found = FALSE;
   sprintf( filename, "%s%s", SPACE_DIR, starsystemfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      LINK( starsystem, first_starsystem, last_starsystem, next, prev );
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_starsystem_file: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SPACE" ) )
         {
            fread_starsystem( starsystem, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "Load_starsystem_file: bad section: %s.", word );
            bug( buf, 0 );
            break;
         }
      }
      fclose( fp );
   }

   if( !( found ) )
      DISPOSE( starsystem );

   return found;
}

/*
 * Load in all the starsystem files.
 */
void load_space(  )
{
   FILE *fpList;
   char *filename;
   char starsystemlist[256];
   char buf[MAX_STRING_LENGTH];


   first_starsystem = NULL;
   last_starsystem = NULL;

   log_string( "Loading space..." );

   sprintf( starsystemlist, "%s%s", SPACE_DIR, SPACE_LIST );
   if( ( fpList = fopen( starsystemlist, "r" ) ) == NULL )
   {
      perror( starsystemlist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;


      if( !load_starsystem( filename ) )
      {
         sprintf( buf, "Cannot load starsystem file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   log_string( " Done starsystems " );
   return;
}

void do_setstarsystem( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   SPACE_DATA *starsystem;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg2[0] == '\0' || arg1[0] == '\0' )
   {
      send_to_char( "Usage: setstarsystem <starsystem> <field> <values>\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( "name filename xpos ypos,\n\r", ch );
      send_to_char( "star1 s1x s1y s1z gravitys1\n\r", ch );
      send_to_char( "star2 s2x s2y s2z gravitys2\n\r", ch );
      send_to_char( "planet1 p1x p1y p1z gravityp1\n\r", ch );
      send_to_char( "planet2 p2x p2y p2z gravityp2\n\r", ch );
      send_to_char( "planet3 p3x p3y p3z gravityp3\n\r", ch );
      send_to_char( "location1a location1b location1c doc1a doc1b doc1c\n\r", ch );
      send_to_char( "location2a location2b location2c doc2a doc2b doc2c\n\r", ch );
      send_to_char( "location3a location3b location3c doc3a doc3b doc3c\n\r", ch );
      send_to_char( "", ch );
      return;
   }

   starsystem = starsystem_from_name( arg1 );
   if( !starsystem )
   {
      send_to_char( "No such starsystem.\n\r", ch );
      return;
   }


   if( !str_cmp( arg2, "doc1a" ) )
   {
      starsystem->doc1a = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "doc1b" ) )
   {
      starsystem->doc1b = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "doc1c" ) )
   {
      starsystem->doc1c = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "doc2a" ) )
   {
      starsystem->doc2a = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "doc2b" ) )
   {
      starsystem->doc2b = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "doc2c" ) )
   {
      starsystem->doc2c = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "doc3a" ) )
   {
      starsystem->doc3a = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "doc3b" ) )
   {
      starsystem->doc3b = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "doc3c" ) )
   {
      starsystem->doc3c = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "s1x" ) )
   {
      starsystem->s1x = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "s1y" ) )
   {
      starsystem->s1y = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "s1z" ) )
   {
      starsystem->s1z = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "s2x" ) )
   {
      starsystem->s2x = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "s2y" ) )
   {
      starsystem->s2y = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "s2z" ) )
   {
      starsystem->s2z = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "p1x" ) )
   {
      starsystem->p1x = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "p1y" ) )
   {
      starsystem->p1y = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "p1z" ) )
   {
      starsystem->p1z = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "p2x" ) )
   {
      starsystem->p2x = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "p2y" ) )
   {
      starsystem->p2y = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "p2z" ) )
   {
      starsystem->p2z = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "p3x" ) )
   {
      starsystem->p3x = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "p3y" ) )
   {
      starsystem->p3y = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "p3z" ) )
   {
      starsystem->p3z = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "xpos" ) )
   {
      starsystem->xpos = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "ypos" ) )
   {
      starsystem->ypos = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "gravitys1" ) )
   {
      starsystem->gravitys1 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "gravitys2" ) )
   {
      starsystem->gravitys2 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "gravityp1" ) )
   {
      starsystem->gravityp1 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "gravityp2" ) )
   {
      starsystem->gravityp2 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "gravityp3" ) )
   {
      starsystem->gravityp3 = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      STRFREE( starsystem->name );
      starsystem->name = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "star1" ) )
   {
      STRFREE( starsystem->star1 );
      starsystem->star1 = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "star2" ) )
   {
      STRFREE( starsystem->star2 );
      starsystem->star2 = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "planet1" ) )
   {
      STRFREE( starsystem->planet1 );
      starsystem->planet1 = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "planet2" ) )
   {
      STRFREE( starsystem->planet2 );
      starsystem->planet2 = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "planet3" ) )
   {
      STRFREE( starsystem->planet3 );
      starsystem->planet3 = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "location1a" ) )
   {
      STRFREE( starsystem->location1a );
      starsystem->location1a = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "location1b" ) )
   {
      STRFREE( starsystem->location1b );
      starsystem->location1b = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "location1c" ) )
   {
      STRFREE( starsystem->location1c );
      starsystem->location1c = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "location2a" ) )
   {
      STRFREE( starsystem->location2a );
      starsystem->location2a = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "location2b" ) )
   {
      STRFREE( starsystem->location2a );
      starsystem->location2b = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "location2c" ) )
   {
      STRFREE( starsystem->location2c );
      starsystem->location2c = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }

   if( !str_cmp( arg2, "location3a" ) )
   {
      STRFREE( starsystem->location3a );
      starsystem->location3a = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "location3b" ) )
   {
      STRFREE( starsystem->location3b );
      starsystem->location3b = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }
   if( !str_cmp( arg2, "location3c" ) )
   {
      STRFREE( starsystem->location3c );
      starsystem->location3c = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_starsystem( starsystem );
      return;
   }


   do_setstarsystem( ch, "" );
   return;
}

void showstarsystem( CHAR_DATA * ch, SPACE_DATA * starsystem )
{
   ch_printf( ch, "Starsystem:%s     Filename: %s    Xpos: %d   Ypos: %d\n\r",
              starsystem->name, starsystem->filename, starsystem->xpos, starsystem->ypos );
   ch_printf( ch, "Star1: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
              starsystem->star1, starsystem->gravitys1, starsystem->s1x, starsystem->s1y, starsystem->s1z );
   ch_printf( ch, "Star2: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
              starsystem->star2, starsystem->gravitys2, starsystem->s2x, starsystem->s2y, starsystem->s2z );
   ch_printf( ch, "Planet1: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
              starsystem->planet1, starsystem->gravityp1, starsystem->p1x, starsystem->p1y, starsystem->p1z );
   ch_printf( ch, "     Doc1a: %5d (%s)\n\r", starsystem->doc1a, starsystem->location1a );
   ch_printf( ch, "     Doc1b: %5d (%s)\n\r", starsystem->doc1b, starsystem->location1b );
   ch_printf( ch, "     Doc1c: %5d (%s)\n\r", starsystem->doc1c, starsystem->location1c );
   ch_printf( ch, "Planet2: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
              starsystem->planet2, starsystem->gravityp2, starsystem->p2x, starsystem->p2y, starsystem->p2z );
   ch_printf( ch, "     Doc2a: %5d (%s)\n\r", starsystem->doc2a, starsystem->location2a );
   ch_printf( ch, "     Doc2b: %5d (%s)\n\r", starsystem->doc2b, starsystem->location2b );
   ch_printf( ch, "     Doc2c: %5d (%s)\n\r", starsystem->doc2c, starsystem->location2c );
   ch_printf( ch, "Planet3: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
              starsystem->planet3, starsystem->gravityp3, starsystem->p3x, starsystem->p3y, starsystem->p3z );
   ch_printf( ch, "     Doc3a: %5d (%s)\n\r", starsystem->doc3a, starsystem->location3a );
   ch_printf( ch, "     Doc3b: %5d (%s)\n\r", starsystem->doc3b, starsystem->location3b );
   ch_printf( ch, "     Doc3c: %5d (%s)\n\r", starsystem->doc3c, starsystem->location3c );
   return;
}

void do_showstarsystem( CHAR_DATA * ch, char *argument )
{
   SPACE_DATA *starsystem;

   starsystem = starsystem_from_name( argument );

   if( starsystem == NULL )
      send_to_char( "&RNo such starsystem.\n\r", ch );
   else
      showstarsystem( ch, starsystem );

}

void do_makestarsystem( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char filename[256];
   SPACE_DATA *starsystem;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makestarsystem <starsystem name>\n\r", ch );
      return;
   }


   CREATE( starsystem, SPACE_DATA, 1 );
   LINK( starsystem, first_starsystem, last_starsystem, next, prev );

   starsystem->name = STRALLOC( argument );

   starsystem->location1a = STRALLOC( "" );
   starsystem->location2a = STRALLOC( "" );
   starsystem->location3a = STRALLOC( "" );
   starsystem->location1b = STRALLOC( "" );
   starsystem->location2b = STRALLOC( "" );
   starsystem->location3b = STRALLOC( "" );
   starsystem->location1c = STRALLOC( "" );
   starsystem->location2c = STRALLOC( "" );
   starsystem->location3c = STRALLOC( "" );
   starsystem->planet1 = STRALLOC( "" );
   starsystem->planet2 = STRALLOC( "" );
   starsystem->planet3 = STRALLOC( "" );
   starsystem->star1 = STRALLOC( "" );
   starsystem->star2 = STRALLOC( "" );

   argument = one_argument( argument, arg );
   sprintf( filename, "%s.system", strlower( arg ) );
   starsystem->filename = str_dup( filename );
   save_starsystem( starsystem );
   write_starsystem_list(  );
}

void do_starsystems( CHAR_DATA * ch, char *argument )
{
   SPACE_DATA *starsystem;
   int count = 0;

   set_char_color( AT_NOTE, ch );
   ch_printf( ch, "\n\r&R&w+&R&W--------------------------------&R&w+&R&W--------------&R&w+\n\r" );
   ch_printf( ch, "&R&W|&Y Starsystem Name                &R&W|   &YLocation   &R&W|\n\r" );
   ch_printf( ch, "&R&w+&R&W--------------------------------&R&w+&R&W--------------&R&w+\n\r" );

   for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
   {
      if( !str_cmp( starsystem->name, "Simulator" ) )
         continue;

      ch_printf( ch, "&R&W| &O%-30s&R&W | &Y%5d&O, &Y%-5d&R&W |\n\r", starsystem->name, starsystem->xpos, starsystem->ypos );
      count++;
   }
   ch_printf( ch, "&R&w+&R&W--------------------------------&R&w+&R&W--------------&R&w+\n\r" );


   if( !count )
   {
      send_to_char( "There are no starsystems currently formed.\n\r", ch );
      return;
   }
}

void echo_to_ship( int color, SHIP_DATA * ship, char *argument )
{
   int room;

   for( room = ship->firstroom; room <= ship->lastroom; room++ )
   {
      echo_to_room( color, get_room_index( room ), argument );
   }

}

void sound_to_ship( SHIP_DATA * ship, char *argument )
{
   int roomnum;
   ROOM_INDEX_DATA *room;
   CHAR_DATA *vic;

   for( roomnum = ship->firstroom; roomnum <= ship->lastroom; roomnum++ )
   {
      room = get_room_index( roomnum );
      if( room == NULL )
         continue;

      for( vic = room->first_person; vic; vic = vic->next_in_room )
      {
         if( !IS_NPC( vic ) && IS_SET( vic->act, PLR_SOUND ) )
            send_to_char( argument, vic );
      }
   }

}

void echo_to_cockpit( int color, SHIP_DATA * ship, char *argument )
{
   int room;

   for( room = ship->firstroom; room <= ship->lastroom; room++ )
   {
      if( room == ship->cockpit || room == ship->navseat
          || room == ship->pilotseat || room == ship->coseat
          || room == ship->gunseat || room == ship->engineroom
          || room == ship->turret1 || room == ship->turret2
          || room == ship->turret3 || room == ship->turret4
          || room == ship->turret5 || room == ship->turret6
          || room == ship->turret7 || room == ship->turret8 || room == ship->turret9 || room == ship->turret10 )
         echo_to_room( color, get_room_index( room ), argument );
   }

}

void echo_to_system( int color, SHIP_DATA * ship, char *argument, SHIP_DATA * ignore )
{
   SHIP_DATA *target;

   if( !ship->starsystem )
      return;

   for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
   {
      if( target != ship && target != ignore )
         echo_to_cockpit( color, target, argument );
   }

}

bool is_facing( SHIP_DATA * ship, SHIP_DATA * target )
{
   float dy, dx, dz, hx, hy, hz;
   float cosofa;

   hx = ship->hx;
   hy = ship->hy;
   hz = ship->hz;

   dx = target->vx - ship->vx;
   dy = target->vy - ship->vy;
   dz = target->vz - ship->vz;

   cosofa = ( hx * dx + hy * dy + hz * dz ) / ( sqrt( hx * hx + hy * hy + hz * hz ) + sqrt( dx * dx + dy * dy + dz * dz ) );

   if( cosofa > 0.75 )
      return TRUE;

   return FALSE;
}


long int get_ship_value( SHIP_DATA * ship )
{
   long int price;

   price = ship->cost;

   return price;
}

void write_ship_list(  )
{
   SHIP_DATA *tship;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", SHIP_DIR, SHIP_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open ship.lst for writing!\n\r", 0 );
      return;
   }
   for( tship = first_ship; tship; tship = tship->next )
      fprintf( fpout, "%s\n", tship->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

SHIP_DATA *ship_in_room( ROOM_INDEX_DATA * room, char *name )
{
   SHIP_DATA *ship;

   if( !room )
      return NULL;

   for( ship = room->first_ship; ship; ship = ship->next_in_room )
      if( !str_cmp( name, ship->name ) )
         return ship;

   for( ship = room->first_ship; ship; ship = ship->next_in_room )
      if( nifty_is_name_prefix( name, ship->name ) )
         return ship;

   return NULL;
}

/*
 * Get pointer to ship structure from ship name.
 */
SHIP_DATA *get_ship( char *name )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( !str_cmp( name, ship->name ) )
         return ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( nifty_is_name_prefix( name, ship->name ) )
         return ship;

   return NULL;
}

/*
 * Checks if ships in a starsystem and returns poiner if it is.
 */
SHIP_DATA *get_ship_here( char *name, SPACE_DATA * starsystem )
{
   SHIP_DATA *ship;

   if( starsystem == NULL )
      return NULL;

   for( ship = starsystem->first_ship; ship; ship = ship->next_in_starsystem )
      if( !str_cmp( name, ship->name ) )
         return ship;

   for( ship = starsystem->first_ship; ship; ship = ship->next_in_starsystem )
      if( nifty_is_name_prefix( name, ship->name ) )
         return ship;

   return NULL;
}


/*
 * Get pointer to ship structure from ship name.
 */
SHIP_DATA *ship_from_pilot( char *name )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( !str_cmp( name, ship->pilot ) )
         return ship;
   if( !str_cmp( name, ship->copilot ) )
      return ship;
   if( !str_cmp( name, ship->owner ) )
      return ship;
   return NULL;
}


/*
 * Get pointer to ship structure from cockpit, turret, or entrance ramp vnum.
 */

SHIP_DATA *ship_from_cockpit( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->cockpit || vnum == ship->turret1 || vnum == ship->turret2
          || vnum == ship->turret3 || vnum == ship->turret4 || vnum == ship->turret5
          || vnum == ship->turret6 || vnum == ship->turret7 || vnum == ship->turret8
          || vnum == ship->turret9 || vnum == ship->turret10
          || vnum == ship->pilotseat || vnum == ship->coseat || vnum == ship->navseat
          || vnum == ship->gunseat || vnum == ship->engineroom )
         return ship;
   return NULL;
}

SHIP_DATA *ship_from_pilotseat( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->pilotseat || vnum == ship->cockpit )
         return ship;
   return NULL;
}

SHIP_DATA *ship_from_coseat( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->coseat )
         return ship;
   return NULL;
}

SHIP_DATA *ship_from_navseat( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->navseat )
         return ship;
   return NULL;
}

SHIP_DATA *ship_from_gunseat( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->gunseat )
         return ship;
   return NULL;
}

SHIP_DATA *ship_from_engine( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
   {
      if( ship->engineroom )
      {
         if( vnum == ship->engineroom )
            return ship;
      }
      else
      {
         if( vnum == ship->cockpit )
            return ship;
      }
   }

   return NULL;
}



SHIP_DATA *ship_from_turret( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->gunseat || vnum == ship->turret1 || vnum == ship->turret2
          || vnum == ship->turret3 || vnum == ship->turret4 || vnum == ship->turret5
          || vnum == ship->turret6 || vnum == ship->turret7 || vnum == ship->turret8
          || vnum == ship->turret9 || vnum == ship->turret10 )
         return ship;
   return NULL;
}

SHIP_DATA *ship_from_entrance( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->entrance )
         return ship;
   return NULL;
}

SHIP_DATA *ship_from_hanger( int vnum )
{
   SHIP_DATA *ship;

   for( ship = first_ship; ship; ship = ship->next )
      if( vnum == ship->hanger1 || vnum == ship->hanger2 || vnum == ship->hanger3 || vnum == ship->hanger4 )
         return ship;
   return NULL;
}


void save_ship( SHIP_DATA * ship )
{
   FILE *fp;
   char filename[256];
   char buf[MAX_STRING_LENGTH];
   MODULE_DATA *mod;

   if( !ship )
   {
      bug( "save_ship: null ship pointer!", 0 );
      return;
   }

   if( !ship->filename || ship->filename[0] == '\0' )
   {
      sprintf( buf, "save_ship: %s has no filename", ship->name );
      bug( buf, 0 );
      return;
   }

   sprintf( filename, "%s%s", SHIP_DIR, ship->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_ship: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#SHIP\n" );
      fprintf( fp, "Name         %s~\n", ship->name );
      fprintf( fp, "Filename     %s~\n", ship->filename );
      fprintf( fp, "Protoname    %s~\n", ship->protoname );
      fprintf( fp, "PBeacon      %s~\n", ship->pbeacon );
      fprintf( fp, "ClanOwner    %s~\n", ship->clanowner );
      fprintf( fp, "Description  %s~\n", ship->description );
      fprintf( fp, "Owner        %s~\n", ship->owner );
      fprintf( fp, "Pilot        %s~\n", ship->pilot );
      fprintf( fp, "Copilot      %s~\n", ship->copilot );
      fprintf( fp, "Class        %d\n", ship->class );
      fprintf( fp, "primaryType        %d\n", ship->primaryType );
      fprintf( fp, "primaryCount       %d\n", ship->primaryCount );
      fprintf( fp, "secondaryType      %d\n", ship->secondaryType );
      fprintf( fp, "secondaryCount	 %d\n", ship->secondaryCount );
      fprintf( fp, "Alarm        %d\n", ship->alarm );
      fprintf( fp, "Maxmod       %d\n", ship->maxmod );
      fprintf( fp, "Tractorbeam  %d\n", ship->tractorbeam );
      fprintf( fp, "Shipyard     %d\n", ship->shipyard );
      fprintf( fp, "Turret1      %d\n", ship->turret1 );
      fprintf( fp, "Turret2      %d\n", ship->turret2 );
      fprintf( fp, "Turret3      %d\n", ship->turret3 );
      fprintf( fp, "Turret4	   %d\n", ship->turret4 );
      fprintf( fp, "Turret5	   %d\n", ship->turret5 );
      fprintf( fp, "Turret6      %d\n", ship->turret6 );
      fprintf( fp, "Turret7      %d\n", ship->turret7 );
      fprintf( fp, "Turret8      %d\n", ship->turret8 );
      fprintf( fp, "Turret9      %d\n", ship->turret9 );
      fprintf( fp, "Turret10     %d\n", ship->turret10 );
      fprintf( fp, "Statet1      %d\n", ship->statet1 );
      fprintf( fp, "Statet2      %d\n", ship->statet2 );
      fprintf( fp, "Statet3      %d\n", ship->statet3 );
      fprintf( fp, "Statet4      %d\n", ship->statet4 );
      fprintf( fp, "Statet5      %d\n", ship->statet5 );
      fprintf( fp, "Statet6      %d\n", ship->statet6 );
      fprintf( fp, "Statet7      %d\n", ship->statet7 );
      fprintf( fp, "Statet8      %d\n", ship->statet8 );
      fprintf( fp, "Statet9      %d\n", ship->statet9 );
      fprintf( fp, "Statet10     %d\n", ship->statet10 );
      fprintf( fp, "Bombs	   %d\n", ship->bombs );
      fprintf( fp, "Hibombstr	   %d\n", ship->hibombstr );
      fprintf( fp, "lowbombstr   %d\n", ship->lowbombstr );
      fprintf( fp, "Maxbombs     %d\n", ship->maxbombs );
      fprintf( fp, "Missiles     %d\n", ship->missiles );
      fprintf( fp, "Maxmissiles  %d\n", ship->maxmissiles );
      fprintf( fp, "Rockets     %d\n", ship->rockets );
      fprintf( fp, "Maxrockets  %d\n", ship->maxrockets );
      fprintf( fp, "Torpedos     %d\n", ship->torpedos );
      fprintf( fp, "Maxtorpedos  %d\n", ship->maxtorpedos );
      fprintf( fp, "Lastdoc      %d\n", ship->lastdoc );
      fprintf( fp, "Firstroom    %d\n", ship->firstroom );
      fprintf( fp, "Lastroom     %d\n", ship->lastroom );
      fprintf( fp, "Hanger1      %d\n", ship->hanger1 );
      fprintf( fp, "Hanger2      %d\n", ship->hanger2 );
      fprintf( fp, "Hanger3      %d\n", ship->hanger3 );
      fprintf( fp, "Hanger4      %d\n", ship->hanger4 );
      fprintf( fp, "Shield       %d\n", ship->shield );
      fprintf( fp, "Maxshield    %d\n", ship->maxshield );
      fprintf( fp, "Hull         %d\n", ship->hull );
      fprintf( fp, "Maxhull      %d\n", ship->maxhull );
      fprintf( fp, "Maxenergy    %d\n", ship->maxenergy );
      fprintf( fp, "Hyperspeed   %d\n", ship->hyperspeed );
      fprintf( fp, "Comm         %d\n", ship->comm );
      fprintf( fp, "Cost	   %d\n", ship->cost );
      fprintf( fp, "Chaff        %d\n", ship->chaff );
      fprintf( fp, "Maxchaff     %d\n", ship->maxchaff );
      fprintf( fp, "Sensor       %d\n", ship->sensor );
      fprintf( fp, "Astro_array  %d\n", ship->astro_array );
      fprintf( fp, "Realspeed    %d\n", ship->realspeed );
      fprintf( fp, "Type         %d\n", ship->type );
      fprintf( fp, "Cockpit      %d\n", ship->cockpit );
      fprintf( fp, "Coseat       %d\n", ship->coseat );
      fprintf( fp, "Pilotseat    %d\n", ship->pilotseat );
      fprintf( fp, "Gunseat      %d\n", ship->gunseat );
      fprintf( fp, "Navseat      %d\n", ship->navseat );
      fprintf( fp, "Engineroom   %d\n", ship->engineroom );
      fprintf( fp, "Entrance     %d\n", ship->entrance );
      fprintf( fp, "Shipstate    %d\n", ship->shipstate );
      fprintf( fp, "Missilestate %d\n", ship->missilestate );
      fprintf( fp, "Energy       %d\n", ship->energy );
      fprintf( fp, "Manuever     %d\n", ship->manuever );
      fprintf( fp, "Password     %d\n", ship->password );
      fprintf( fp, "Home         %s~\n", ship->home );
      for( mod = ship->first_module; mod; mod = mod->next )
         fprintf( fp, "Module     %d %d\n", mod->affect, mod->ammount );
      fprintf( fp, "Simulator    %d %d\n",
               IS_SET( ship->flags, SHIP_SIMULATOR ) ? 1 : 0, IS_SET( ship->flags, SHIP_SIMULATOR ) ? ship->sim_vnum : 0 );

      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}


/*
 * Read in actual ship data.
 */
void fread_ship( SHIP_DATA * ship, FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   int dummy_number;
   int x1, x2;
   char *line;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Alarm", ship->alarm, fread_number( fp ) );
            KEY( "Astro_array", ship->astro_array, fread_number( fp ) );
            break;

         case 'B':
            KEY( "Bombs", ship->bombs, fread_number( fp ) );
            break;

         case 'C':
            KEY( "Cockpit", ship->cockpit, fread_number( fp ) );
            KEY( "Coseat", ship->coseat, fread_number( fp ) );
            KEY( "ClanOwner", ship->clanowner, fread_string( fp ) );
            KEY( "Class", ship->class, fread_number( fp ) );
            KEY( "Copilot", ship->copilot, fread_string( fp ) );
            KEY( "Comm", ship->comm, fread_number( fp ) );
            KEY( "Cost", ship->cost, fread_number( fp ) );
            KEY( "Chaff", ship->chaff, fread_number( fp ) );
            break;


         case 'D':
            KEY( "Description", ship->description, fread_string( fp ) );
            break;

         case 'E':
            KEY( "Engineroom", ship->engineroom, fread_number( fp ) );
            KEY( "Entrance", ship->entrance, fread_number( fp ) );
            KEY( "Energy", ship->energy, fread_number( fp ) );
            if( !str_cmp( word, "End" ) )
            {
               if( !ship->home )
                  ship->home = STRALLOC( "" );
               if( !ship->name )
                  ship->name = STRALLOC( "" );
               if( !ship->owner )
                  ship->owner = STRALLOC( "" );
               if( !ship->description )
                  ship->description = STRALLOC( "" );
               if( !ship->copilot )
                  ship->copilot = STRALLOC( "" );
               if( !ship->pilot )
                  ship->pilot = STRALLOC( "" );
               if( ship->shipstate != SHIP_DISABLED )
                  ship->shipstate = SHIP_DOCKED;

               ship->primaryState = LASER_READY;
               ship->secondaryState = LASER_READY;
               if( ship->statet1 != LASER_DAMAGED )
                  ship->statet1 = LASER_READY;
               if( ship->statet2 != LASER_DAMAGED )
                  ship->statet2 = LASER_READY;
               if( ship->missilestate != MISSILE_DAMAGED )
                  ship->missilestate = MISSILE_READY;
               if( ship->shipyard <= 0 )
                  ship->shipyard = ROOM_LIMBO_SHIPYARD;
               if( ship->lastdoc <= 0 )
                  ship->lastdoc = ship->shipyard;
               ship->bayopen = FALSE;
               if( ship->class >= SHIP_CRUISER )
                  ship->autopilot = TRUE;
               else
                  ship->autopilot = FALSE;

               ship->hatchopen = FALSE;
               if( ship->navseat <= 0 )
                  ship->navseat = ship->cockpit;
               if( ship->gunseat <= 0 )
                  ship->gunseat = ship->cockpit;
               if( ship->coseat <= 0 )
                  ship->coseat = ship->cockpit;
               if( ship->pilotseat <= 0 )
                  ship->pilotseat = ship->cockpit;
               if( ship->missiletype == 1 )
               {
                  ship->torpedos = ship->missiles; /* for back compatability */
                  ship->missiles = 0;
               }
               ship->starsystem = NULL;
               ship->energy = ship->maxenergy;
               ship->hull = ship->maxhull;
               ship->in_room = NULL;
               ship->next_in_room = NULL;
               ship->prev_in_room = NULL;

               return;
            }
            break;

         case 'F':
            KEY( "Filename", ship->filename, fread_string_nohash( fp ) );
            KEY( "Firstroom", ship->firstroom, fread_number( fp ) );
            break;

         case 'G':
            KEY( "Gunseat", ship->gunseat, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Home", ship->home, fread_string( fp ) );
            KEY( "Hyperspeed", ship->hyperspeed, fread_number( fp ) );
            KEY( "Hibombstr", ship->hibombstr, fread_number( fp ) );
            KEY( "Hull", ship->hull, fread_number( fp ) );
            KEY( "Hanger1", ship->hanger1, fread_number( fp ) );
            KEY( "Hanger2", ship->hanger2, fread_number( fp ) );
            KEY( "Hanger3", ship->hanger3, fread_number( fp ) );
            KEY( "Hanger4", ship->hanger4, fread_number( fp ) );
            break;
         case 'L':
            KEY( "Lastdoc", ship->lastdoc, fread_number( fp ) );
            KEY( "Lastroom", ship->lastroom, fread_number( fp ) );
            KEY( "Lowbombstr", ship->lowbombstr, fread_number( fp ) );
            break;

         case 'M':
            KEY( "Manuever", ship->manuever, fread_number( fp ) );
            KEY( "Maxbombs", ship->maxbombs, fread_number( fp ) );
            KEY( "Maxmissiles", ship->maxmissiles, fread_number( fp ) );
            KEY( "Maxmod", ship->maxmod, fread_number( fp ) );
            KEY( "Maxtorpedos", ship->maxtorpedos, fread_number( fp ) );
            KEY( "Maxrockets", ship->maxrockets, fread_number( fp ) );
            KEY( "Missiles", ship->missiles, fread_number( fp ) );
            KEY( "Missiletype", ship->missiletype, fread_number( fp ) );
            KEY( "Maxshield", ship->maxshield, fread_number( fp ) );
            KEY( "Maxenergy", ship->maxenergy, fread_number( fp ) );
            KEY( "Missilestate", ship->missilestate, fread_number( fp ) );
            KEY( "Maxhull", ship->maxhull, fread_number( fp ) );
            KEY( "Maxchaff", ship->maxchaff, fread_number( fp ) );
            if( !str_cmp( word, "Module" ) )
            {
               MODULE_DATA *mod;
               line = fread_line( fp );
               CREATE( mod, MODULE_DATA, 1 );
               LINK( mod, ship->first_module, ship->last_module, next, prev );
               x1 = x2 = 0;
               sscanf( line, "%d %d", &x1, &x2 );
               mod->affect = x1;
               mod->ammount = x2;
            }
            break;

         case 'N':
            KEY( "Name", ship->name, fread_string( fp ) );
            KEY( "Navseat", ship->navseat, fread_number( fp ) );
            break;

         case 'O':
            KEY( "Owner", ship->owner, fread_string( fp ) );
            KEY( "Objectnum", dummy_number, fread_number( fp ) );
            break;

         case 'P':
            KEY( "Password", ship->password, fread_number( fp ) );
            KEY( "Pilot", ship->pilot, fread_string( fp ) );
            KEY( "Pilotseat", ship->pilotseat, fread_number( fp ) );
            KEY( "Protoname", ship->protoname, fread_string( fp ) );
            KEY( "primaryCount", ship->primaryCount, fread_number( fp ) );
            KEY( "primaryType", ship->primaryType, fread_number( fp ) );
            KEY( "PBeacon", ship->pbeacon, fread_string( fp ) );
            break;

         case 'R':
            KEY( "Realspeed", ship->realspeed, fread_number( fp ) );
            KEY( "Rockets", ship->rockets, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Shipyard", ship->shipyard, fread_number( fp ) );
            KEY( "Sensor", ship->sensor, fread_number( fp ) );
            KEY( "secondaryType", ship->secondaryType, fread_number( fp ) );
            KEY( "secondaryCount", ship->secondaryCount, fread_number( fp ) );
            KEY( "Shield", ship->shield, fread_number( fp ) );
            KEY( "Shipstate", ship->shipstate, fread_number( fp ) );
            KEY( "Statet1", ship->statet1, fread_number( fp ) );
            KEY( "Statet2", ship->statet2, fread_number( fp ) );
            KEY( "Statet3", ship->statet3, fread_number( fp ) );
            KEY( "Statet4", ship->statet4, fread_number( fp ) );
            KEY( "Statet5", ship->statet5, fread_number( fp ) );
            KEY( "Statet6", ship->statet6, fread_number( fp ) );
            KEY( "Statet7", ship->statet7, fread_number( fp ) );
            KEY( "Statet8", ship->statet8, fread_number( fp ) );
            KEY( "Statet9", ship->statet9, fread_number( fp ) );
            KEY( "Statet10", ship->statet10, fread_number( fp ) );
            if( !str_cmp( word, "Simulator" ) )
            {
               line = fread_line( fp );
               x1 = x2 = 0;
               sscanf( line, "%d %d", &x1, &x2 );
               if( x1 == 1 )
               {
                  SET_BIT( ship->flags, SHIP_SIMULATOR );
                  ship->sim_vnum = x2;
               }
               else
                  ship->sim_vnum = 0;
               fMatch = TRUE;
            }

            break;

         case 'T':
            KEY( "Type", ship->type, fread_number( fp ) );
            KEY( "Tractorbeam", ship->tractorbeam, fread_number( fp ) );
            KEY( "Turret1", ship->turret1, fread_number( fp ) );
            KEY( "Turret2", ship->turret2, fread_number( fp ) );
            KEY( "Turret3", ship->turret3, fread_number( fp ) );
            KEY( "Turret4", ship->turret4, fread_number( fp ) );
            KEY( "Turret5", ship->turret5, fread_number( fp ) );
            KEY( "Turret6", ship->turret6, fread_number( fp ) );
            KEY( "Turret7", ship->turret7, fread_number( fp ) );
            KEY( "Turret8", ship->turret8, fread_number( fp ) );
            KEY( "Turret9", ship->turret9, fread_number( fp ) );
            KEY( "Turret10", ship->turret10, fread_number( fp ) );
            KEY( "Torpedos", ship->torpedos, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_ship: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

/*
 * Load a ship file
 */

bool load_ship_file( char *shipfile )
{
   char filename[256];
   SHIP_DATA *ship;
   FILE *fp;
   bool found;
   ROOM_INDEX_DATA *pRoomIndex;
   CLAN_DATA *clan;


   CREATE( ship, SHIP_DATA, 1 );

   found = FALSE;
   sprintf( filename, "%s%s", SHIP_DIR, shipfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_ship_file: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SHIP" ) )
         {
            fread_ship( ship, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "Load_ship_file: bad section: %s.", word );
            bug( buf, 0 );
            break;
         }
      }
      fclose( fp );
   }
   if( !( found ) )
      DISPOSE( ship );
   else
   {
      LINK( ship, first_ship, last_ship, next, prev );
      if( !str_cmp( "Public", ship->owner ) || ship->type == MOB_SHIP )
      {

         if( ship->class <= SHIP_CORVETTE && ship->type != MOB_SHIP )
         {
            extract_ship( ship );
            ship_to_room( ship, ship->shipyard );

            ship->location = ship->shipyard;
            ship->lastdoc = ship->shipyard;
            ship->shipstate = SHIP_DOCKED;
         }

         if( IS_SET( ship->flags, XSHIP_ION_DRIVE ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_DRIVE );
            ship->shipstate = SHIP_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_HYPER ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_HYPER );
            ship->shipstate = LASER_READY;

         }

         if( IS_SET( ship->flags, XSHIP_ION_ION ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_ION );
            ship->secondaryState = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_LASERS ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_LASERS );
            ship->primaryState = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_MISSILES ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_MISSILES );
            ship->missilestate = MISSILE_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET1 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET1 );
            ship->turret1 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET2 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET2 );
            ship->turret2 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET3 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET3 );
            ship->turret3 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET4 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET4 );
            ship->turret4 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET5 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET5 );
            ship->turret5 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET6 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET6 );
            ship->turret6 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET7 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET7 );
            ship->turret7 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET8 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET8 );
            ship->turret8 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET9 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET9 );
            ship->turret9 = LASER_READY;
         }

         if( IS_SET( ship->flags, XSHIP_ION_TURRET10 ) )
         {
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET10 );
            ship->turret10 = LASER_READY;
         }

         ship->currspeed = 0;
         ship->energy = ship->maxenergy;
         ship->chaff = ship->maxchaff;
         ship->hull = ship->maxhull;
         ship->shield = 0;

         ship->statet1 = LASER_READY;
         ship->statet2 = LASER_READY;
         ship->primaryState = LASER_READY;
         ship->secondaryState = LASER_READY;
         ship->missilestate = LASER_READY;

         ship->currjump = NULL;
         ship->target0 = NULL;
         ship->target1 = NULL;
         ship->target2 = NULL;

         ship->hatchopen = FALSE;
         ship->bayopen = FALSE;
         ship->bayopen = FALSE;

         ship->missiles = ship->maxmissiles;
         ship->torpedos = ship->maxtorpedos;
         ship->rockets = ship->maxrockets;
         ship->autorecharge = FALSE;
         ship->autotrack = FALSE;
         ship->autospeed = FALSE;


      }

      else if( ship->cockpit == ROOM_SHUTTLE_BUS ||
               ship->cockpit == ROOM_SHUTTLE_BUS_2 ||
               ship->cockpit == ROOM_SENATE_SHUTTLE ||
               ship->cockpit == ROOM_CORUSCANT_TURBOCAR || ship->cockpit == ROOM_CORUSCANT_SHUTTLE )
      {
      }
      else if( ( pRoomIndex = get_room_index( ship->lastdoc ) ) != NULL && ship->class <= SHIP_CORVETTE )
      {
         LINK( ship, pRoomIndex->first_ship, pRoomIndex->last_ship, next_in_room, prev_in_room );
         ship->in_room = pRoomIndex;
         ship->location = ship->lastdoc;
      }


      if( ship->class >= SHIP_CRUISER || ship->type == MOB_SHIP )
      {
         ship_to_starsystem( ship, starsystem_from_name( ship->home ) );
         if( ship->class >= SHIP_CRUISER )
         {

            if( !str_cmp( ship->home, "Coruscant" ) || !str_cmp( ship->home, "coruscant" ) )
            {
               ship->vx = -345;
               ship->vy = -987;
               ship->vz = 1562;
            }
            else if( !str_cmp( ship->home, "Calamari" ) || !str_cmp( ship->home, "calamari" ) )
            {
               ship->vx = 3300;
               ship->vy = 1300;
               ship->vz = 1700;
            }
            else
            {
               ship->vx = number_range( -5000, 5000 );
               ship->vy = number_range( -5000, 5000 );
               ship->vz = number_range( -5000, 5000 );
            }
         }

         ship->hx = 1;
         ship->hy = 1;
         ship->hz = 1;
         ship->shipstate = SHIP_READY;
         ship->autopilot = TRUE;
         ship->autorecharge = TRUE;
         ship->shield = ship->maxshield;

         if( ship->type == MOB_SHIP )
         {
            if( !str_cmp( ship->home, "Coruscant" ) || !str_cmp( ship->home, "coruscant" ) )
            {
               if( number_range( 1, 2 ) == 1 )
                  ship->vx = ( -345 ) + ( number_range( 50, 150 ) );
               else
                  ship->vx = ( -345 ) - ( number_range( 50, 150 ) );
               if( number_range( 1, 2 ) == 1 )
                  ship->vy = ( -987 ) + ( number_range( 50, 150 ) );
               else
                  ship->vy = ( -987 ) - ( number_range( 50, 150 ) );
               if( number_range( 1, 2 ) == 1 )
                  ship->vz = ( 1562 ) + ( number_range( 50, 150 ) );
               else
                  ship->vz = ( 1562 ) - ( number_range( 50, 150 ) );
            }
            else if( !str_cmp( ship->home, "Calamari" ) || !str_cmp( ship->home, "calamari" ) )
            {
               if( number_range( 1, 2 ) == 1 )
                  ship->vx = ( 3300 ) + ( number_range( 50, 150 ) );
               else
                  ship->vx = ( 3300 ) - ( number_range( 50, 150 ) );
               if( number_range( 1, 2 ) == 1 )
                  ship->vy = ( 1300 ) + ( number_range( 50, 150 ) );
               else
                  ship->vy = ( 1300 ) - ( number_range( 50, 150 ) );
               if( number_range( 1, 2 ) == 1 )
                  ship->vz = ( 1700 ) + ( number_range( 50, 150 ) );
               else
                  ship->vz = ( 1700 ) - ( number_range( 50, 150 ) );

            }
            else
            {
               ship->vx = number_range( -5000, 5000 );
               ship->vy = number_range( -5000, 5000 );
               ship->vz = number_range( -5000, 5000 );
            }

            ship->hx = 1;
            ship->hy = 1;
            ship->hz = 1;
            ship->shipstate = SHIP_READY;
            ship->autopilot = TRUE;
            ship->autorecharge = TRUE;
            ship->shield = ship->maxshield;
         }
      }

      if( !ship->password )
         ship->password = number_range( 1111, 9999 );

      if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
      {
         if( ship->class != LAND_VEHICLE )
            clan->spacecraft++;
         else
            clan->vehicles++;
      }

   }

   return found;
}

/*
 * Load in all the ship files.
 */
void load_ships(  )
{
   FILE *fpList;
   char *filename;
   char shiplist[256];
   char buf[MAX_STRING_LENGTH];


   first_ship = NULL;
   last_ship = NULL;
   first_missile = NULL;
   last_missile = NULL;

   log_string( "Loading ships..." );

   sprintf( shiplist, "%s%s", SHIP_DIR, SHIP_LIST );
   if( ( fpList = fopen( shiplist, "r" ) ) == NULL )
   {
      perror( shiplist );
      exit( 1 );
   }

   for( ;; )
   {

      filename = feof( fpList ) ? "$" : fread_word( fpList );

      if( filename[0] == '$' )
         break;

      if( !load_ship_file( filename ) )
      {
         sprintf( buf, "Cannot load ship file: %s", filename );
         bug( buf, 0 );
      }

   }
   fclose( fpList );
   log_string( " Done ships " );
   return;
}

void untargetship( SHIP_DATA * target )
{
   SHIP_DATA *ship;
   MISSILE_DATA *missile;

   for( ship = first_ship; ship; ship = ship->next )
   {
      if( ship == target )
         continue;
      if( ship->target0 == target )
         ship->target0 = NULL;
      if( ship->target1 == target )
         ship->target1 = NULL;
      if( ship->target2 == target )
         ship->target2 = NULL;
      if( ship->target3 == target )
         ship->target3 = NULL;
      if( ship->target4 == target )
         ship->target4 = NULL;
      if( ship->target5 == target )
         ship->target5 = NULL;
      if( ship->target6 == target )
         ship->target6 = NULL;
      if( ship->target7 == target )
         ship->target7 = NULL;
      if( ship->target8 == target )
         ship->target8 = NULL;
      if( ship->target9 == target )
         ship->target9 = NULL;
      if( ship->target10 == target )
         ship->target10 = NULL;
   }
   for( missile = first_missile; missile; missile = missile->next )
      if( missile->target == target )
         missile->target = NULL;
}


void resetship( SHIP_DATA * ship )
{
   untargetship( ship );
   ship->shipstate = SHIP_READY;

   if( ship->class != SHIP_SPACE_STATION && ship->type != MOB_SHIP )
   {
      extract_ship( ship );
      ship_to_room( ship, ship->shipyard );

      ship->location = ship->shipyard;
      ship->lastdoc = ship->shipyard;
      ship->shipstate = SHIP_DOCKED;
   }

   if( ship->starsystem )
      ship_from_starsystem( ship, ship->starsystem );

   ship->currspeed = 0;
   ship->energy = ship->maxenergy;
   ship->chaff = ship->maxchaff;
   ship->hull = ship->maxhull;
   ship->shield = 0;

   ship->statet1 = LASER_READY;
   ship->statet2 = LASER_READY;
   ship->primaryState = LASER_READY;
   ship->secondaryState = LASER_READY;
   ship->missilestate = LASER_READY;

   ship->currjump = NULL;
   ship->target0 = NULL;
   ship->target1 = NULL;
   ship->target2 = NULL;

   ship->hatchopen = FALSE;
   ship->bayopen = FALSE;

   ship->missiles = ship->maxmissiles;
   ship->torpedos = ship->maxtorpedos;
   ship->rockets = ship->maxrockets;
   ship->autorecharge = FALSE;
   ship->autotrack = FALSE;
   ship->autospeed = FALSE;

   if( IS_SET( ship->flags, XSHIP_ION_DRIVE ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_DRIVE );
      ship->shipstate = SHIP_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_HYPER ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_HYPER );
      ship->shipstate = LASER_READY;

   }

   if( IS_SET( ship->flags, XSHIP_ION_ION ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_ION );
      ship->secondaryState = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_LASERS ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_LASERS );
      ship->primaryState = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_MISSILES ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_MISSILES );
      ship->missilestate = MISSILE_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET1 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET1 );
      ship->turret1 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET2 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET2 );
      ship->turret2 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET3 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET3 );
      ship->turret3 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET4 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET4 );
      ship->turret4 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET5 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET5 );
      ship->turret5 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET6 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET6 );
      ship->turret6 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET7 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET7 );
      ship->turret7 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET8 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET8 );
      ship->turret8 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET9 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET9 );
      ship->turret9 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET10 ) )
   {
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET10 );
      ship->turret10 = LASER_READY;
   }

   if( str_cmp( "Public", ship->owner ) && ship->type != MOB_SHIP )
   {
      CLAN_DATA *clan;

      if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
      {
         if( ship->class != LAND_VEHICLE )
            clan->spacecraft--;
         else
            clan->vehicles--;
      }

      STRFREE( ship->owner );
      ship->owner = STRALLOC( "" );
      STRFREE( ship->pilot );
      ship->pilot = STRALLOC( "" );
      STRFREE( ship->copilot );
      ship->copilot = STRALLOC( "" );
   }


   save_ship( ship );
}

void do_resetship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;

   ship = get_ship( argument );
   if( ship == NULL )
   {
      send_to_char( "&RNo such ship!", ch );
      return;
   }

   resetship( ship );

   if( ( ship->class >= SHIP_CRUISER || ship->type == MOB_SHIP ) && ship->home )
   {
      ship_to_starsystem( ship, starsystem_from_name( ship->home ) );
      ship->vx = number_range( -5000, 5000 );
      ship->vy = number_range( -5000, 5000 );
      ship->vz = number_range( -5000, 5000 );
      ship->shipstate = SHIP_READY;
      ship->autopilot = TRUE;
      ship->autorecharge = TRUE;
      ship->shield = ship->maxshield;
   }

}

void do_setship( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   SHIP_DATA *ship;
   int tempnum;
   ROOM_INDEX_DATA *roomindex;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
   {
      send_to_char( "Usage: setship <ship> <field> <values>\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( "filename name owner copilot pilot description home\n\r", ch );
      send_to_char( "cockpit entrance turret1 turret2 turret3 turret4\n\r", ch );
      send_to_char( "turret5 turret6 turret7 turret8 turret9 turret10\n\r", ch );
      send_to_char( "hanger engineroom firstroom lastroom shipyard\n\r", ch );
      send_to_char( "manuever speed hyperspeed tractorbeam\n\r", ch );
      send_to_char( "pcount primary scount secondary missiles shield hull energy chaff\n\r", ch );
      send_to_char( "comm sensor astroarray class torpedos bombs\n\r", ch );
      send_to_char( "pilotseat coseat gunseat navseat rockets simvnum\n\r", ch );
      return;
   }

   ship = get_ship( arg1 );
   if( !ship )
   {
      send_to_char( "No such ship.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "owner" ) )
   {
      CLAN_DATA *clan;
      if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
      {
         if( ship->class != LAND_VEHICLE )
            clan->spacecraft--;
         else
            clan->vehicles--;
      }
      STRFREE( ship->owner );
      ship->owner = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
      {
         if( ship->class != LAND_VEHICLE )
            clan->spacecraft++;
         else
            clan->vehicles++;
      }
      return;
   }

   if( !str_cmp( arg2, "primary" ) )
   {
      ship->primaryType = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "secondary" ) )
   {
      ship->secondaryType = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "home" ) )
   {
      STRFREE( ship->home );
      ship->home = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "pilot" ) )
   {
      STRFREE( ship->pilot );
      ship->pilot = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "copilot" ) )
   {
      STRFREE( ship->copilot );
      ship->copilot = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "firstroom" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }
      ship->firstroom = tempnum;
      ship->lastroom = tempnum;
      ship->cockpit = tempnum;
      ship->coseat = tempnum;
      ship->pilotseat = tempnum;
      ship->gunseat = tempnum;
      ship->navseat = tempnum;
      ship->entrance = tempnum;
      ship->turret1 = 0;
      ship->turret2 = 0;
      ship->turret3 = 0;
      ship->turret4 = 0;
      ship->turret5 = 0;
      ship->turret6 = 0;
      ship->turret7 = 0;
      ship->turret8 = 0;
      ship->turret9 = 0;
      ship->turret10 = 0;
      ship->hanger1 = 0;
      ship->hanger2 = 0;
      ship->hanger3 = 0;
      ship->hanger4 = 0;
      send_to_char( "You will now need to set the other rooms in the ship.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "lastroom" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }
      if( tempnum < ship->firstroom )
      {
         send_to_char( "The last room on a ship must be greater than or equal to the first room.\n\r", ch );
         return;
      }
      ship->lastroom = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "cockpit" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->cockpit = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "pilotseat" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->pilotseat = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "coseat" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->coseat = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "navseat" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->navseat = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "gunseat" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->gunseat = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "entrance" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->entrance = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "turret1" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret1 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "turret2" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret2 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret3" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret3 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret4" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret4 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret5" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret5 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret6" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret6 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret7" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }
      if( tempnum < ship->firstroom || tempnum > ship->lastroom )
      {
         send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r", ch );
         return;
      }

      ship->turret7 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret8" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret8 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret9" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret9 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "turret10" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->turret10 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "hanger" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->hanger1 = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "engineroom" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.\n\r", ch );
         return;
      }

      ship->engineroom = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "shipyard" ) )
   {
      tempnum = atoi( argument );
      roomindex = get_room_index( tempnum );
      if( roomindex == NULL )
      {
         send_to_char( "That room doesn't exist.", ch );
         return;
      }
      ship->shipyard = tempnum;
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( !str_cmp( argument, "republic" ) )
         ship->type = SHIP_REPUBLIC;
      else if( !str_cmp( argument, "imperial" ) )
         ship->type = SHIP_IMPERIAL;
      else if( !str_cmp( argument, "player" ) )
         ship->type = PLAYER_SHIP;
      else if( !str_cmp( argument, "mob" ) )
         ship->type = MOB_SHIP;
      else
      {
         send_to_char( "Ship type must be either: republic, imperial, player or mob.\n\r", ch );
         return;
      }
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      STRFREE( ship->name );
      ship->name = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "filename" ) )
   {
      DISPOSE( ship->filename );
      ship->filename = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      write_ship_list(  );
      return;
   }

   if( !str_cmp( arg2, "desc" ) )
   {
      STRFREE( ship->description );
      ship->description = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "manuever" ) )
   {
      ship->manuever = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "pcount" ) )
   {
      ship->primaryCount = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "scount" ) )
   {
      ship->secondaryCount = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "class" ) )
   {
      ship->class = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "missiles" ) )
   {
      ship->maxmissiles = atoi( argument );
      ship->missiles = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "torpedos" ) )
   {
      ship->maxtorpedos = atoi( argument );
      ship->torpedos = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "rockets" ) )
   {
      ship->maxrockets = atoi( argument );
      ship->rockets = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "speed" ) )
   {
      ship->realspeed = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "tractorbeam" ) )
   {
      ship->tractorbeam = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "hyperspeed" ) )
   {
      ship->hyperspeed = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "shield" ) )
   {
      ship->maxshield = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "hull" ) )
   {
      ship->hull = atoi( argument );
      ship->maxhull = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "energy" ) )
   {
      ship->energy = atoi( argument );
      ship->maxenergy = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "sensor" ) )
   {
      ship->sensor = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "astroarray" ) )
   {
      ship->astro_array = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "comm" ) )
   {
      ship->comm = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "chaff" ) )
   {
      ship->chaff = URANGE( 0, atoi( argument ), 25 );
      ship->maxchaff = URANGE( 0, atoi( argument ), 25 );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( arg2, "bombs" ) )
   {
      ship->bombs = atoi( argument );
      ship->maxbombs = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      save_ship( ship );
      return;
   }
   if( !str_cmp( arg2, "simvnum" ) )
   {
      ship->sim_vnum = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "flags" ) )
   {
      char arg3[MAX_INPUT_LENGTH];
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setship <ship> flags <flag> [flag]...\n\r", ch );
         send_to_char( "Valid flags are: simulator\n\r", ch );
         return;
      }

      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         tempnum = get_shipflag( arg3 );

         if( tempnum < 0 || tempnum > 31 )
         {
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            return;
         }
         TOGGLE_BIT( ship->flags, 1 << tempnum );
      }
      save_ship( ship );
      return;
   }

   do_setship( ch, "" );
   return;
}

void do_showship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showship <ship>\n\r", ch );
      return;
   }

   ship = get_ship( argument );
   if( !ship )
   {
      send_to_char( "No such ship.\n\r", ch );
      return;
   }
   set_char_color( AT_YELLOW, ch );
   ch_printf( ch, "%s %s : %s\n\rFilename: %s\n\r",
              ship->type == SHIP_REPUBLIC ? "New Republic" :
              ( ship->type == SHIP_IMPERIAL ? "Imperial" :
                ( ship->type == PLAYER_SHIP ? "Civilian" : "Mob" ) ),
              ship->class == SHIP_FIGHTER ? "Starfighter" :
              ship->class == SHIP_BOMBER ? "Bomber" :
              ship->class == SHIP_SHUTTLE ? "Shuttle" :
              ship->class == SHIP_FREIGHTER ? "Freighter" :
              ship->class == SHIP_FRIGATE ? "Frigate" :
              ship->class == SHIP_TT ? "Troop Transport" :
              ship->class == SHIP_CORVETTE ? "Corvette" :
              ship->class == SHIP_CRUISER ? "Cruiser" :
              ship->class == SHIP_DREADNAUGHT ? "Dreadnaught" :
              ship->class == SHIP_DESTROYER ? "Star Destroyer" :
              ship->class == SHIP_SPACE_STATION ? "Space Station" :
              ship->class == LAND_VEHICLE ? "Land vehicle" : "Unknown", ship->name, ship->filename );
   ch_printf( ch, "Home: %s   Description: %s  Current system: %s\n\rOwner: %s   Pilot: %s   Copilot: %s\n\r",
              ship->home, ship->description,
              ship->starsystem ? ship->starsystem->name : "None", ship->owner, ship->pilot, ship->copilot );
   ch_printf( ch, "Firstroom: %d   Lastroom: %d ", ship->firstroom, ship->lastroom );
   ch_printf( ch, "Cockpit: %d   Entrance: %d\n\rEngineroom: %d       Bay Doors: %s\n\r",
              ship->cockpit, ship->entrance, ship->engineroom, ship->bayopen == TRUE ? "Open" : "Closed" );
   ch_printf( ch, "Hanger1: %d   Hanger2: %d   Hanger3: %d  Hanger4: %d\n\r",
              ship->hanger1, ship->hanger2, ship->hanger3, ship->hanger4 );
   ch_printf( ch, "Pilotseat: %d   Coseat: %d   Navseat: %d  Gunseat: %d\n\r",
              ship->pilotseat, ship->coseat, ship->navseat, ship->gunseat );
   ch_printf( ch, "Location: %d   Lastdoc: %d   Shipyard: %d\n\r", ship->location, ship->lastdoc, ship->shipyard );
   ch_printf( ch, "Tractor Beam: %d   Comm: %d   Sensor: %d   Astro Array: %d\n\r",
              ship->tractorbeam, ship->comm, ship->sensor, ship->astro_array );
   ch_printf( ch, "Primary Count: %d Type: %d Condition: %s\n\r",
              ship->primaryCount, ship->primaryType, ship->primaryState == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Secondary Count: %d Type: %d Condition: %s\n\r",
              ship->primaryCount, ship->primaryType, ship->primaryState == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret One: %d  Condition: %s\n\r", ship->turret1, ship->statet1 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Two: %d  Condition: %s\n\r", ship->turret2, ship->statet2 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Three: %d  Condition: %s\n\r",
              ship->turret3, ship->statet3 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Four: %d  Condition: %s\n\r", ship->turret4, ship->statet4 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Five: %d  Condition: %s\n\r", ship->turret5, ship->statet5 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Six: %d  Condition: %s\n\r", ship->turret6, ship->statet6 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Seven: %d  Condition: %s\n\r",
              ship->turret7, ship->statet7 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Eight: %d  Condition: %s\n\r",
              ship->turret8, ship->statet8 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Nine: %d  Condition: %s\n\r", ship->turret9, ship->statet9 == LASER_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Turret Ten: %d  Condition: %s\n\r",
              ship->turret10, ship->statet10 == LASER_DAMAGED ? "Damaged" : "Good" );

   ch_printf( ch, "Missiles: %d/%d  Torpedos: %d/%d  Rockets: %d/%d  Condition: %s\n\r",
              ship->missiles,
              ship->maxmissiles,
              ship->torpedos,
              ship->maxtorpedos,
              ship->rockets, ship->maxrockets, ship->missilestate == MISSILE_DAMAGED ? "Damaged" : "Good" );
   ch_printf( ch, "Hull: %d/%d  Ship Condition: %s\n\r",
              ship->hull, ship->maxhull, ship->shipstate == SHIP_DISABLED ? "Disabled" : "Running" );

   ch_printf( ch, "Shields: %d/%d   Energy(fuel): %d/%d\n\rBombs: %d/%d  Chaff: %d/%d\n\r",
              ship->shield,
              ship->maxshield, ship->energy, ship->maxenergy, ship->bombs, ship->maxbombs, ship->chaff, ship->maxchaff );
   ch_printf( ch, "Current Coordinates: %.0f %.0f %.0f\n\r", ship->vx, ship->vy, ship->vz );
   ch_printf( ch, "Current Heading: %.0f %.0f %.0f\n\r", ship->hx, ship->hy, ship->hz );
   ch_printf( ch, "Speed: %d/%d   Hyperspeed: %d\n\r  Manueverability: %d\n\r",
              ship->currspeed, ship->realspeed, ship->hyperspeed, ship->manuever );
   ch_printf( ch, "Flags: %s", flag_string( ship->flags, ship_flags ) );
   return;
}

void do_makeship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char arg[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makeship <filename> <ship name>\n\r", ch );
      return;
   }

   CREATE( ship, SHIP_DATA, 1 );
   LINK( ship, first_ship, last_ship, next, prev );

   ship->name = STRALLOC( argument );
   ship->description = STRALLOC( "" );
   ship->owner = STRALLOC( "" );
   ship->copilot = STRALLOC( "" );
   ship->pilot = STRALLOC( "" );
   ship->home = STRALLOC( "" );
   ship->type = PLAYER_SHIP;
   ship->starsystem = NULL;
   ship->energy = ship->maxenergy;
   ship->hull = ship->maxhull;
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


   ship->filename = str_dup( arg );
   save_ship( ship );
   write_ship_list(  );

}

void do_copyship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   SHIP_DATA *old;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: copyship <oldshipname> <filename> <newshipname>\n\r", ch );
      return;
   }

   old = get_ship( arg );

   if( !old )
   {
      send_to_char( "Thats not a ship!\n\r", ch );
      return;
   }

   CREATE( ship, SHIP_DATA, 1 );
   LINK( ship, first_ship, last_ship, next, prev );

   ship->name = STRALLOC( argument );
   ship->description = STRALLOC( "" );
   ship->owner = STRALLOC( "" );
   ship->copilot = STRALLOC( "" );
   ship->pilot = STRALLOC( "" );
   ship->home = STRALLOC( "" );
   ship->type = old->type;
   ship->class = old->class;
   ship->primaryCount = old->primaryCount;
   ship->maxmissiles = old->maxmissiles;
   ship->maxrockets = old->maxrockets;
   ship->maxtorpedos = old->maxtorpedos;
   ship->maxshield = old->maxshield;
   ship->maxhull = old->maxhull;
   ship->maxenergy = old->maxenergy;
   ship->hyperspeed = old->hyperspeed;
   ship->maxchaff = old->maxchaff;
   ship->maxbombs = old->maxbombs;
   ship->realspeed = old->realspeed;
   ship->manuever = old->manuever;
   ship->in_room = NULL;
   ship->next_in_room = NULL;
   ship->prev_in_room = NULL;
   ship->currjump = NULL;
   ship->target0 = NULL;
   ship->target1 = NULL;
   ship->target2 = NULL;

   ship->filename = str_dup( arg2 );
   save_ship( ship );
   write_ship_list(  );
}

void do_ships( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int count;

   if( !IS_NPC( ch ) )
   {
      count = 0;
      send_to_char( "&YThe following ships are owned by you or by your organization:\n\r", ch );
      send_to_char( "\n\r&WShip                               Owner\n\r", ch );
      for( ship = first_ship; ship; ship = ship->next )
      {
         if( str_cmp( ship->owner, ch->name ) )
         {
            if( !ch->pcdata || !ch->pcdata->clan || str_cmp( ship->owner, ch->pcdata->clan->name )
                || ship->class > SHIP_SPACE_STATION )
               continue;
         }

         if( ship->type == MOB_SHIP )
            continue;
         else if( ship->type == SHIP_REPUBLIC )
            set_char_color( AT_BLOOD, ch );
         else if( ship->type == SHIP_IMPERIAL )
            set_char_color( AT_DGREEN, ch );
         else
            set_char_color( AT_BLUE, ch );

         if( ship->in_room )
            ch_printf( ch, "%s (%s) - %s\n\r", ship->name, ship->in_room->name, ship->owner );
         else
            ch_printf( ch, "%s - %s\n\r", ship->name, ship->owner );

         count++;
      }

      if( !count )
      {
         send_to_char( "There are no ships owned by you.\n\r", ch );
      }

   }


/*    count =0;
    send_to_char( "&Y\n\rThe following ships are docked here:\n\r", ch );
    
    send_to_char( "\n\r&WShip                               Owner          Cost/Rent\n\r", ch );
    for ( ship = first_ship; ship; ship = ship->next )
    {   
        if ( ship->location != ch->in_room->vnum || ship->class > SHIP_SPACE_STATION)
               continue;

        if (ship->type == MOB_SHIP)
           continue;
        else if (ship->type == SHIP_REPUBLIC)
           set_char_color( AT_BLOOD, ch );
        else if (ship->type == SHIP_IMPERIAL)
           set_char_color( AT_DGREEN, ch );
        else
          set_char_color( AT_BLUE, ch );
        
        ch_printf( ch, "%-35s %-15s", ship->name, ship->owner );
        if (ship->type == MOB_SHIP || ship->class == SHIP_SPACE_STATION )
        {
          ch_printf( ch, "\n\r");
          continue;
        }
        if ( !str_cmp(ship->owner, "Public") )
        { 
          ch_printf( ch, "%ld to rent.\n\r", get_ship_value(ship)/100 ); 
        }
        else if ( str_cmp(ship->owner, "") )
          ch_printf( ch, "%s", "\n\r" );
        else
           ch_printf( ch, "%ld to buy.\n\r", get_ship_value(ship) ); 
        
        count++;
    }

    if ( !count )
    {
        send_to_char( "There are no ships docked here.\n\r", ch );
    } */
}


void do_speeders( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int count;

   if( !IS_NPC( ch ) )
   {
      count = 0;
      send_to_char( "&YThe following are owned by you or by your organization:\n\r", ch );
      send_to_char( "\n\r&WVehicle                            Owner\n\r", ch );
      for( ship = first_ship; ship; ship = ship->next )
      {
         if( str_cmp( ship->owner, ch->name ) )
         {
            if( !ch->pcdata || !ch->pcdata->clan || str_cmp( ship->owner, ch->pcdata->clan->name )
                || ship->class <= SHIP_SPACE_STATION )
               continue;
         }
         if( ship->location != ch->in_room->vnum || ship->class != LAND_VEHICLE )
            continue;

         if( ship->type == MOB_SHIP )
            continue;
         else if( ship->type == SHIP_REPUBLIC )
            set_char_color( AT_BLOOD, ch );
         else if( ship->type == SHIP_IMPERIAL )
            set_char_color( AT_DGREEN, ch );
         else
            set_char_color( AT_BLUE, ch );

         ch_printf( ch, "%-35s %-15s\n\r", ship->name, ship->owner );

         count++;
      }

      if( !count )
      {
         send_to_char( "There are no land or air vehicles owned by you.\n\r", ch );
      }

   }


   count = 0;
   send_to_char( "&Y\n\rThe following vehicles are parked here:\n\r", ch );

   send_to_char( "\n\r&WVehicle                            Owner          Cost/Rent\n\r", ch );
   for( ship = first_ship; ship; ship = ship->next )
   {
      if( ship->location != ch->in_room->vnum || ship->class != LAND_VEHICLE )
         continue;

      if( ship->type == MOB_SHIP )
         continue;
      else if( ship->type == SHIP_REPUBLIC )
         set_char_color( AT_BLOOD, ch );
      else if( ship->type == SHIP_IMPERIAL )
         set_char_color( AT_DGREEN, ch );
      else
         set_char_color( AT_BLUE, ch );


      ch_printf( ch, "%-35s %-15s", ship->name, ship->owner );

      if( !str_cmp( ship->owner, "Public" ) )
      {
         ch_printf( ch, "%ld to rent.\n\r", get_ship_value( ship ) / 100 );
      }
      else if( str_cmp( ship->owner, "" ) )
         ch_printf( ch, "%s", "\n\r" );
      else
         ch_printf( ch, "%ld to buy.\n\r", get_ship_value( ship ) );

      count++;
   }

   if( !count )
   {
      send_to_char( "There are no sea air or land vehicles here.\n\r", ch );
   }
}

void do_allspeeders( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int count = 0;

   count = 0;
   send_to_char( "&Y\n\rThe following sea/land/air vehicles are currently formed:\n\r", ch );

   send_to_char( "\n\r&WVehicle                            Owner\n\r", ch );
   for( ship = first_ship; ship; ship = ship->next )
   {
      if( ship->class != LAND_VEHICLE )
         continue;

      if( ship->type == MOB_SHIP )
         continue;
      else if( ship->type == SHIP_REPUBLIC )
         set_char_color( AT_BLOOD, ch );
      else if( ship->type == SHIP_IMPERIAL )
         set_char_color( AT_DGREEN, ch );
      else
         set_char_color( AT_BLUE, ch );


      ch_printf( ch, "%-35s %-15s ", ship->name, ship->owner );

      if( !str_cmp( ship->owner, "Public" ) )
      {
         ch_printf( ch, "%ld to rent.\n\r", get_ship_value( ship ) / 100 );
      }
      else if( str_cmp( ship->owner, "" ) )
         ch_printf( ch, "%s", "\n\r" );
      else
         ch_printf( ch, "%ld to buy.\n\r", get_ship_value( ship ) );

      count++;
   }

   if( !count )
   {
      send_to_char( "There are none currently formed.\n\r", ch );
      return;
   }

}

void do_allships( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int count = 0;

   count = 0;
   send_to_char( "&Y\n\rThe following ships are currently formed:\n\r", ch );

   send_to_char( "\n\r&WShip                               Owner\n\r", ch );

   if( IS_IMMORTAL( ch ) )
      for( ship = first_ship; ship; ship = ship->next )
         if( ship->type == MOB_SHIP )
            ch_printf( ch, "&w%-35s %-15s\n\r", ship->name, ship->owner );

   for( ship = first_ship; ship; ship = ship->next )
   {
      if( ship->class == LAND_VEHICLE )
         continue;

      if( ship->type == MOB_SHIP )
         continue;
      else if( ship->type == SHIP_REPUBLIC )
         set_char_color( AT_BLOOD, ch );
      else if( ship->type == SHIP_IMPERIAL )
         set_char_color( AT_DGREEN, ch );
      else
         set_char_color( AT_BLUE, ch );

      ch_printf( ch, "%-35s %-15s ", ship->name, ship->owner );
      if( ship->type == MOB_SHIP || ship->class == SHIP_SPACE_STATION )
      {
         ch_printf( ch, "\n\r" );
         continue;
      }
      if( !str_cmp( ship->owner, "Public" ) )
      {
         ch_printf( ch, "%ld to rent.\n\r", get_ship_value( ship ) / 100 );
      }
      else if( str_cmp( ship->owner, "" ) )
         ch_printf( ch, "%s", "\n\r" );
      else
         ch_printf( ch, "%ld to buy.\n\r", get_ship_value( ship ) );

      count++;
   }

   if( !count )
   {
      send_to_char( "There are no ships currently formed.\n\r", ch );
      return;
   }

}


void ship_to_starsystem( SHIP_DATA * ship, SPACE_DATA * starsystem )
{
   if( starsystem == NULL )
      return;

   if( ship == NULL )
      return;

   if( starsystem->first_ship == NULL )
      starsystem->first_ship = ship;

   if( starsystem->last_ship )
   {
      starsystem->last_ship->next_in_starsystem = ship;
      ship->prev_in_starsystem = starsystem->last_ship;
      ship->next_in_starsystem = NULL;
   }

   starsystem->last_ship = ship;

   ship->starsystem = starsystem;

}

void new_missile( SHIP_DATA * ship, SHIP_DATA * target, CHAR_DATA * ch, int missiletype )
{
   SPACE_DATA *starsystem;
   MISSILE_DATA *missile;

   if( ship == NULL )
      return;

   if( target == NULL )
      return;

   if( ( starsystem = ship->starsystem ) == NULL )
      return;

   CREATE( missile, MISSILE_DATA, 1 );
   LINK( missile, first_missile, last_missile, next, prev );

   missile->target = target;
   missile->fired_from = ship;
   if( ch )
      missile->fired_by = STRALLOC( ch->name );
   else
      missile->fired_by = STRALLOC( "" );
   missile->missiletype = missiletype;
   missile->age = 0;
   if( missile->missiletype == HEAVY_BOMB )
      missile->speed = 250;
   else if( missile->missiletype == PROTON_TORPEDO )
      missile->speed = 200;
   else if( missile->missiletype == CONCUSSION_MISSILE )
      missile->speed = 300;
   else
      missile->speed = 50;

   missile->mx = ship->vx;
   missile->my = ship->vy;
   missile->mz = ship->vz;

   if( starsystem->first_missile == NULL )
      starsystem->first_missile = missile;

   if( starsystem->last_missile )
   {
      starsystem->last_missile->next_in_starsystem = missile;
      missile->prev_in_starsystem = starsystem->last_missile;
   }

   starsystem->last_missile = missile;

   missile->starsystem = starsystem;

}

void ship_from_starsystem( SHIP_DATA * ship, SPACE_DATA * starsystem )
{

   if( starsystem == NULL )
      return;

   if( ship == NULL )
      return;

   if( starsystem->last_ship == ship )
      starsystem->last_ship = ship->prev_in_starsystem;

   if( starsystem->first_ship == ship )
      starsystem->first_ship = ship->next_in_starsystem;

   if( ship->prev_in_starsystem )
      ship->prev_in_starsystem->next_in_starsystem = ship->next_in_starsystem;

   if( ship->next_in_starsystem )
      ship->next_in_starsystem->prev_in_starsystem = ship->prev_in_starsystem;

   ship->starsystem = NULL;
   ship->next_in_starsystem = NULL;
   ship->prev_in_starsystem = NULL;

}

void extract_missile( MISSILE_DATA * missile )
{
   SPACE_DATA *starsystem;

   if( missile == NULL )
      return;

   if( ( starsystem = missile->starsystem ) != NULL )
   {

      if( starsystem->last_missile == missile )
         starsystem->last_missile = missile->prev_in_starsystem;

      if( starsystem->first_missile == missile )
         starsystem->first_missile = missile->next_in_starsystem;

      if( missile->prev_in_starsystem )
         missile->prev_in_starsystem->next_in_starsystem = missile->next_in_starsystem;

      if( missile->next_in_starsystem )
         missile->next_in_starsystem->prev_in_starsystem = missile->prev_in_starsystem;

      missile->starsystem = NULL;
      missile->next_in_starsystem = NULL;
      missile->prev_in_starsystem = NULL;

   }

   UNLINK( missile, first_missile, last_missile, next, prev );

   missile->target = NULL;
   missile->fired_from = NULL;
   if( missile->fired_by )
      STRFREE( missile->fired_by );

   DISPOSE( missile );

}

bool is_rental( CHAR_DATA * ch, SHIP_DATA * ship )
{
   if( !str_cmp( "Public", ship->owner ) )
      return TRUE;

   return FALSE;
}

bool check_pilot( CHAR_DATA * ch, SHIP_DATA * ship )
{
   if( !str_cmp( ch->name, ship->owner ) || !str_cmp( ch->name, ship->pilot )
       || !str_cmp( ch->name, ship->copilot ) || !str_cmp( "Public", ship->owner ) )
      return TRUE;

   if( !IS_NPC( ch ) && ch->pcdata && ch->pcdata->clan )
   {
      if( !str_cmp( ch->pcdata->clan->name, ship->owner ) )
      {
         if( !str_cmp( ch->pcdata->clan->leader, ch->name ) )
            return TRUE;
         if( !str_cmp( ch->pcdata->clan->number1, ch->name ) )
            return TRUE;
         if( !str_cmp( ch->pcdata->clan->number2, ch->name ) )
            return TRUE;
         if( ch->pcdata->bestowments && is_name( "pilot", ch->pcdata->bestowments ) )
            return TRUE;
      }
   }

   return FALSE;
}

bool extract_ship( SHIP_DATA * ship )
{
   ROOM_INDEX_DATA *room;

   if( ( room = ship->in_room ) != NULL )
   {
      UNLINK( ship, room->first_ship, room->last_ship, next_in_room, prev_in_room );
      ship->in_room = NULL;
   }
   return TRUE;
}

void damage_ship_ch( SHIP_DATA * ship, int min, int max, CHAR_DATA * ch )
{
   int sdamage, shield_dmg;
   long xp;

   sdamage = number_range( min, max );

   xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 25;
   xp = UMIN( get_ship_value( ship ) / 100, xp );
   gain_exp( ch, xp, PILOTING_ABILITY );

   if( ship->shield > 0 )
   {
      shield_dmg = UMIN( ship->shield, sdamage );
      sdamage -= shield_dmg;
      ship->shield -= shield_dmg;
      if( ship->shield == 0 )
         echo_to_cockpit( AT_BLOOD, ship, "&G[&gShip Computer&G] &wShields down..." );
   }

   if( sdamage > 0 )
   {
      if( number_range( 1, 100 ) <= 5 && ship->shipstate != SHIP_DISABLED )
      {
         echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G] &wDrive disabled." );
         ship->shipstate = SHIP_DISABLED;
      }

      if( number_range( 1, 100 ) <= 5 && ship->missilestate != MISSILE_DAMAGED && ship->maxmissiles > 0 )
      {
         echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->gunseat ),
                       "&G[&gShip Computer&G] &wMissile launcher offline." );
         ship->missilestate = MISSILE_DAMAGED;
      }

      if( number_range( 1, 100 ) <= 2 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_LASERS ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_LASERS );
         if( ship->primaryState != LASER_DAMAGED )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->gunseat ),
                          "&G[&gShip Computer&G] &wPrimary weapon system offline." );

            ship->primaryState = LASER_DAMAGED;
         }
      }
      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET1 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET1 );
         if( ship->statet1 != LASER_DAMAGED && ship->turret1 )
         {
            ship->statet1 = LASER_DAMAGED;
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret1 ), "&G[&gShip Computer&G] &wTurret offline." );
         }
      }

      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET2 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET2 );
         if( ship->statet2 != LASER_DAMAGED && ship->turret2 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret2 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet2 = LASER_DAMAGED;
         }
      }

      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET3 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET3 );
         if( ship->statet3 != LASER_DAMAGED && ship->turret3 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret3 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet3 = LASER_DAMAGED;
         }
      }

      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET4 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET4 );
         if( ship->statet4 != LASER_DAMAGED && ship->turret4 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret4 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet4 = LASER_DAMAGED;
         }
      }

      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET5 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET5 );
         if( ship->statet5 != LASER_DAMAGED && ship->turret5 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret5 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet5 = LASER_DAMAGED;
         }
      }

      if( number_range( 1, 100 ) <= 5 )
      {

         if( IS_SET( ship->flags, XSHIP_ION_TURRET6 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET6 );
         if( ship->statet6 != LASER_DAMAGED && ship->turret6 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret6 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet6 = LASER_DAMAGED;
         }
      }

      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET7 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET7 );

         if( ship->statet7 != LASER_DAMAGED && ship->turret7 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret7 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet7 = LASER_DAMAGED;
         }
      }
      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET8 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET8 );
         if( ship->statet8 != LASER_DAMAGED && ship->turret8 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret8 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet8 = LASER_DAMAGED;
         }
      }

      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET9 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET9 );
         if( ship->statet9 != LASER_DAMAGED && ship->turret9 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret9 ), "&G[&gShip Computer&G] &wTurret offline." );

            ship->statet9 = LASER_DAMAGED;
         }
      }
      if( number_range( 1, 100 ) <= 5 )
      {
         if( IS_SET( ship->flags, XSHIP_ION_TURRET10 ) )
            REMOVE_BIT( ship->flags, XSHIP_ION_TURRET10 );
         if( ship->statet10 != LASER_DAMAGED && ship->turret10 )
         {
            echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret10 ), "&G[&gShip Computer&G] &wTurret offline." );
            ship->statet10 = LASER_DAMAGED;
         }
      }
   }

   if( ch->pcdata->learned[gsn_spacecombat] > 1 && ch->pcdata->learned[gsn_spacecombat2] < 1
       && ch->pcdata->learned[gsn_spacecombat3] < 1 )
      ship->hull -= sdamage * 4;
   else if( ch->pcdata->learned[gsn_spacecombat] > 1 && ch->pcdata->learned[gsn_spacecombat2] > 1
            && ch->pcdata->learned[gsn_spacecombat3] < 1 )
      ship->hull -= sdamage * 5;
   else if( ch->pcdata->learned[gsn_spacecombat] > 1 && ch->pcdata->learned[gsn_spacecombat2] > 1
            && ch->pcdata->learned[gsn_spacecombat3] > 1 )
      ship->hull -= sdamage * 6;
   else
      ship->hull -= sdamage * 5;

   if( ship->hull <= 0 )
   {
      destroy_ship( ship, ch, "shot down" );

      xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) );
      xp = UMIN( get_ship_value( ship ), xp );
      gain_exp( ch, xp, PILOTING_ABILITY );
      ch_printf( ch, "&WYou gain %ld piloting experience!\n\r", xp );
      return;
   }

   if( ship->hull <= ship->maxhull / 20 )
      echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G] &wShip hull severely damaged." );

}

void damage_ship_ch_ion( SHIP_DATA * ship, int min, int max, CHAR_DATA * ch )
{
   int sdamage, numloop, shields, range, schance;
   long xp;
   //Used to store all systems damaged to then be displayed when done.

   sdamage = number_range( min, max );

   /*
    * Random # used for a loop. The loop will run through accordingly disabling a random system each time.
    * Could disable as many as all 11 systems in one hit though highly unlikely.
    *
    */
   numloop = number_range( 1, 7 );


   xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 25;
   xp = UMIN( get_ship_value( ship ) / 100, xp );
   gain_exp( ch, xp, PILOTING_ABILITY );

   if( ship->shield > 0 )
   {
      //Small formula to determine how much damage to cause to shields but shields will be ignored just damaged.
      shields = ( ( ship->shield - sdamage ) / ( sdamage / 2 ) );

      ship->shield -= shields;

      if( ship->shield < 0 )
         ship->shield = 0;
      if( ship->shield == 0 )
         echo_to_cockpit( AT_BLOOD, ship, "&G[&gShip Computer&G]&w Warning. Shields down.\n\r" );



   }
/*    
    if (ship->plasmashield > 0)
    {
      //The ship has plasma shielding which will protect it from those nasty ion cannons :P
      int plasmashields;
      plasmashields = ( ( ship->maxplasmashield - damage) / (damage / 2) );
      ship->plasmashield -= plasmashields;
      if (ship->plasmashield < 0)
        ship->plasmashield = 0;
      if (ship->plasmashield == 0)
        echo_to_cockpit( AT_BLOOD, ship, "&R[&WALERT&R] &WWarning Plasma Shielding Down!\n\r");

      //Exit out of the damage function with no damage recieved to the ship for now :)  
      
      return;
   }  
*/
   //This is what picks out 
   range = number_range( 1, 7 );

   /*
    * Update for new space/ion types
    * * Low damage has much less chance to disable a system, high damage is increased.
    * * If no shield protection, much higher chance to disable a system
    * * -||
    */
   if( sdamage < 5 && ship->shield <= 0 )
      schance = 15;  //
   else if( sdamage < 5 && ship->shield > 0 )
      schance = 5;   // Repeating ions
   else if( sdamage > 15 && ship->shield <= 0 )
      schance = 40;  //
   else if( sdamage > 15 && ship->shield > 0 )
      schance = 18;  // Heavy ions
   else if( ( sdamage >= 6 && sdamage <= 14 ) && ship->shield <= 0 )
      schance = 20;  //
   else if( ( sdamage >= 6 && sdamage <= 14 ) && ship->shield > 0 )
      schance = 12;  // Light ions
   else
      schance = 11;

   if( number_range( 1, 100 ) < schance )
   {
      switch ( range )
      {
         case 1:
            if( ship->shipstate != SHIP_DISABLED )
            {
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Ship drive disabled." );
               ship->shipstate = SHIP_DISABLED;
               SET_BIT( ship->flags, XSHIP_ION_DRIVE );
               if( ship->tractoring )
               {
                  ship->tractoring->tractored_by = NULL;
                  ship->tractoring = NULL;
               }
            }
            break;
         case 2:
            if( ship->missilestate != MISSILE_DAMAGED && ship->maxmissiles > 0 )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->gunseat ),
                             "&G[&gShip Computer&G]&w Warning. Ship missile launcher offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship,
                                "&G[&gShip Computer&G]&w Warning. Ship missile launcher disabled." );
               ship->missilestate = MISSILE_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_MISSILES );
            }
            break;
         case 3:
            if( ship->secondaryState != LASER_DAMAGED )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->gunseat ),
                             "&G[&gShip Computer&G]&w Warning. Secondary weapon system offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship,
                                "&G[&gShip Computer&G]&w Warning. Secondary weapon system offline." );
               ship->secondaryState = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_ION );
            }
            break;
         case 4:
            if( ship->primaryState != LASER_DAMAGED )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->gunseat ),
                             "&G[&gShip Computer&G]&w Warning. Primary weapon system offline.." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship,
                                "&G[&gShip Computer&G]&w Warning. Primary weapon system offline." );
               ship->primaryState = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_LASERS );
            }
            break;
         case 5:
            if( ship->statet1 != LASER_DAMAGED && ship->turret1 )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret1 ),
                             "&G[&gShip Computer&G]&w Warning. Turret offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Turret offline." );
               ship->statet1 = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_TURRET1 );
            }
            break;
         case 6:
            if( number_range( 1, 100 ) <= 80 && ship->statet2 != LASER_DAMAGED && ship->turret2 )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret2 ),
                             "&G[&gShip Computer&G]&w Warning. Turret offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Turret offline." );
               ship->statet2 = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_TURRET2 );
            }
            break;
         case 7:
            if( number_range( 1, 100 ) <= 80 && ship->hyperstate != LASER_DAMAGED )
            {
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Hyperdrive disabled." );
               ship->hyperstate = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_HYPER );
            }
            break;
         case 8:
            if( ship->statet3 != LASER_DAMAGED && ship->turret3 )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret3 ),
                             "&G[&gShip Computer&G]&w Warning. Turret offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Turret offline." );
               ship->statet3 = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_TURRET3 );
            }
            break;
         case 9:
            if( ship->statet4 != LASER_DAMAGED && ship->turret4 )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret4 ),
                             "&G[&gShip Computer&G]&w Warning. Turret offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Turret offline." );
               ship->statet4 = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_TURRET4 );
            }
            break;
         case 10:
            if( ship->statet5 != LASER_DAMAGED && ship->turret5 )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret5 ),
                             "&G[&gShip Computer&G]&w Warning. Turret offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Turret offline." );
               ship->statet5 = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_TURRET5 );
            }
            break;
         case 11:
            if( ship->statet6 != LASER_DAMAGED && ship->turret6 )
            {
               echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret6 ),
                             "&G[&gShip Computer&G]&w Warning. Turret offline." );
               echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Turret offline." );
               ship->statet6 = LASER_DAMAGED;
               SET_BIT( ship->flags, XSHIP_ION_TURRET6 );
            }
            break;
         default:
            bug( "Reached 'default' in damage_ship_ch_ion" );
            break;
      }
   }
}

void damage_ship( SHIP_DATA * ship, int min, int max )
{
   int sdamage, shield_dmg;

   sdamage = number_range( min, max );

   if( ship->shield > 0 )
   {
      shield_dmg = UMIN( ship->shield, sdamage );
      sdamage -= shield_dmg;
      ship->shield -= shield_dmg;
      if( ship->shield == 0 )
         echo_to_cockpit( AT_BLOOD, ship, "&G[&gShip Computer&G]&w Warning. Shields down." );
   }

   if( sdamage > 0 )
   {

      if( number_range( 1, 100 ) <= 5 && ship->shipstate != SHIP_DISABLED )
      {
         echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Ship drive disabled." );
         ship->shipstate = SHIP_DISABLED;
      }

      if( number_range( 1, 100 ) <= 5 && ship->missilestate != MISSILE_DAMAGED && ship->maxmissiles > 0 )
      {
         echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->gunseat ),
                       "&G[&gShip Computer&G]&w Warning. Missile launcher offline." );
         ship->missilestate = MISSILE_DAMAGED;
      }

      if( number_range( 1, 100 ) <= 2 && ship->statet1 != LASER_DAMAGED && ship->turret1 )
      {
         echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret1 ),
                       "&G[&gShip Computer&G]&w Warning. Turret offline." );
         ship->statet1 = LASER_DAMAGED;
      }

      if( number_range( 1, 100 ) <= 2 && ship->statet2 != LASER_DAMAGED && ship->turret2 )
      {
         echo_to_room( AT_BLOOD + AT_BLINK, get_room_index( ship->turret2 ),
                       "&G[&gShip Computer&G]&w Warning. Turret offline." );
         ship->statet2 = LASER_DAMAGED;
      }

   }

   ship->hull -= sdamage * 5;

   if( ship->hull <= 0 )
   {
      destroy_ship( ship, NULL, "excessive hull damage" );
      return;
   }

   if( ship->hull <= ship->maxhull / 20 )
      echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "&G[&gShip Computer&G]&w Warning. Ship hull critically damaged." );

}

void destroy_ship( SHIP_DATA * ship, CHAR_DATA * ch, char *reason )
{
   char buf[MAX_STRING_LENGTH];
   int roomnum;
   ROOM_INDEX_DATA *room;
   OBJ_DATA *robj;
   CHAR_DATA *rch;
   int vnum, size;
   ROOM_INDEX_DATA *sroom;
   char shiplog[MAX_STRING_LENGTH];
   AREA_DATA *tarea;
   SHIP_DATA *hship;

   sprintf( buf, "%s explodes in a blinding flash of light!", ship->name );
   echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );
   if( !IS_SET( ship->flags, SHIP_SIMULATOR ) )
   {
      sprintf( buf, "%s destroyed by %s: %s.\n\r", ship->name, ch ? ch->name : "No CH", reason );
      log_string( buf );
      echo_to_ship( AT_WHITE + AT_BLINK, ship, "A blinding flash of light burns your eyes..." );
      echo_to_ship( AT_WHITE, ship,
                    "But before you have a chance to scream...\n\rYou are ripped apart as your spacecraft explodes..." );
   }
   else
   {
      if( ship->type != MOB_SHIP )
         echo_to_ship( AT_WHITE, ship, "The ship has sustained critical damage, and was destroyed.\n\r" );
      else
      {
         ship->shipstate = SHIP_READY;
         ship->currspeed = 0;
         ship->energy = ship->maxenergy;
         ship->chaff = ship->maxchaff;
         ship->hull = ship->maxhull;
         ship->shield = ship->maxshield;

         ship->statet1 = LASER_READY;
         ship->statet2 = LASER_READY;
         ship->statet3 = LASER_READY;
         ship->statet4 = LASER_READY;
         ship->statet5 = LASER_READY;
         ship->statet6 = LASER_READY;
         ship->statet7 = LASER_READY;
         ship->statet8 = LASER_READY;
         ship->statet9 = LASER_READY;
         ship->statet10 = LASER_READY;

         ship->primaryState = LASER_READY;
         ship->secondaryState = LASER_READY;
         ship->missilestate = LASER_READY;

         ship->currjump = NULL;
         ship->target0 = NULL;
         ship->target1 = NULL;
         ship->target2 = NULL;

         ship->hatchopen = FALSE;
         ship->bayopen = FALSE;

         ship->missiles = ship->maxmissiles;
         ship->torpedos = ship->maxtorpedos;
         ship->rockets = ship->maxrockets;
         ship->autorecharge = FALSE;
         ship->autotrack = FALSE;
         ship->autospeed = FALSE;
         ship->autopilot = TRUE;
         ship->vx = number_range( -2000, 2000 );
         ship->vy = number_range( -2000, 2000 );
         ship->vz = number_range( -2000, 2000 );
         return;
      }
   }

   resetship( ship );

   for( roomnum = ship->firstroom; roomnum <= ship->lastroom; roomnum++ )
   {
      room = get_room_index( roomnum );
      if( room != NULL )
      {
         rch = room->first_person;
         while( rch )
         {
            if( IS_SET( ship->flags, SHIP_SIMULATOR ) )
            {
               resetship( ship );
               ship->shipyard = ship->sim_vnum;
               ship->shipstate = SHIP_READY;
               extract_ship( ship );
               ship_to_room( ship, ship->shipyard );
               ship->location = ship->shipyard;
               ship->lastdoc = ship->shipyard;
               ship->shipstate = SHIP_DOCKED;
               if( ship->starsystem )
                  ship_from_starsystem( ship, ship->starsystem );
               save_ship( ship );
               send_to_char( "Red lights flash around the cockpit, and the simulation ends.\n\r", rch );

               return;
            }
            else if( IS_IMMORTAL( rch ) )
            {
               send_to_char( "You are saved from the explosion because of your Immortality.\r\n", rch );
               char_from_room( rch );
               char_to_room( rch, get_room_index( 100 ) );
            }
            else
            {
               if( ch )
                  raw_kill( ch, rch );
               else
                  raw_kill( rch, rch );
            }
            sprintf( buf, "%s killed in space, onboard %s.", rch->name, ship->name );
            log_string( buf );
            rch = room->first_person;
         }
         for( robj = room->first_content; robj; robj = robj->next_content )
         {
            separate_obj( robj );
            extract_obj( robj );
         }
      }

   }
   /*
    * Ships in the hanger of the ship 
    */
   for( hship = first_ship; hship; hship = hship->next )
   {
      if( hship->location >= ship->firstroom && hship->location <= ship->lastroom && hship->location != 0 )
         destroy_ship( hship, NULL, "in hanger of destroyed ship." );
   }

   if( !IS_SET( ship->flags, SHIP_RESPAWN ) && !IS_SET( ship->flags, SHIP_NODESTROY ) )
   {
      resetship( ship );
      size = ship->lastroom + 1;
      sroom = get_room_index( ship->firstroom );
      if( sroom != NULL )
      {
         tarea = sroom->area;
         for( vnum = ship->firstroom; vnum < size; vnum++ )
         {
            sroom = get_room_index( vnum );
            delete_room( sroom );
         }
         fold_area( tarea, tarea->filename, FALSE );
      }
      sprintf( shiplog, "Ship Deleted: %s", ship->name );
      log_string( shiplog );
      extract_ship( ship );
      ship_to_room( ship, 46 );
      ship->location = 46;
      ship->shipyard = 46;
      if( ship->starsystem )
         ship_from_starsystem( ship, ship->starsystem );

      sprintf( buf, "%s%s", SHIP_DIR, ship->filename );
      remove( buf );

      UNLINK( ship, first_ship, last_ship, next, prev );
      DISPOSE( ship );

      write_ship_list(  );
   }
}


bool ship_to_room( SHIP_DATA * ship, int vnum )
{
   ROOM_INDEX_DATA *shipto;

   if( ( shipto = get_room_index( vnum ) ) == NULL )
      return FALSE;

   LINK( ship, shipto->first_ship, shipto->last_ship, next_in_room, prev_in_room );
   ship->in_room = shipto;
   return TRUE;
}


void do_board( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *fromroom;
   ROOM_INDEX_DATA *toroom;
   SHIP_DATA *ship;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Board what?\n\r", ch );
      return;
   }

   if( ( ship = ship_in_room( ch->in_room, argument ) ) == NULL )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
      return;
   }

   if( IS_SET( ch->act, ACT_MOUNTED ) )
   {
      act( AT_PLAIN, "You can't go in there riding THAT.", ch, NULL, argument, TO_CHAR );
      return;
   }

   fromroom = ch->in_room;

   if( ( toroom = get_room_index( ship->entrance ) ) != NULL )
   {
      if( !ship->hatchopen )
      {
         send_to_char( "&RThe hatch is closed!\n\r", ch );
         return;
      }

      if( toroom->tunnel > 0 )
      {
         CHAR_DATA *ctmp;
         int count = 0;

         for( ctmp = toroom->first_person; ctmp; ctmp = ctmp->next_in_room )
            if( ++count >= toroom->tunnel )
            {
               send_to_char( "There is no room for you in there.\n\r", ch );
               return;
            }
      }
      if( ship->shipstate == SHIP_LAUNCH || ship->shipstate == SHIP_LAUNCH_2 )
      {
         send_to_char( "&rThat ship has already started launching!\n\r", ch );
         return;
      }

      act( AT_PLAIN, "$n enters $T.", ch, NULL, ship->name, TO_ROOM );
      act( AT_PLAIN, "You enter $T.", ch, NULL, ship->name, TO_CHAR );
      char_from_room( ch );
      char_to_room( ch, toroom );
      act( AT_PLAIN, "$n enters the ship.", ch, NULL, argument, TO_ROOM );
      do_look( ch, "auto" );

   }
   else
      send_to_char( "That ship has no entrance!\n\r", ch );
}

bool rent_ship( CHAR_DATA * ch, SHIP_DATA * ship )
{

   long price;

   if( IS_NPC( ch ) )
      return FALSE;

   price = get_ship_value( ship ) / 100;

   if( IS_SET( ship->flags, SHIP_SIMULATOR ) )
   {
      ch_printf( ch, "&wThe simulation begins.\n\r" );
      return TRUE;
   }

   if( ch->gold < price )
   {
      ch_printf( ch, "&RRenting this ship costs %ld. You don't have enough credits!\n\r", price );
      return FALSE;
   }

   ch->gold -= price;
   ch_printf( ch, "&GYou pay %ld credits to rent the ship.\n\r", price );
   return TRUE;

}

void do_leaveship( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *fromroom;
   ROOM_INDEX_DATA *toroom;
   SHIP_DATA *ship;

   fromroom = ch->in_room;

   if( ( ship = ship_from_entrance( fromroom->vnum ) ) == NULL )
   {
      send_to_char( "I see no exit here.\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
   }

   if( ship->lastdoc != ship->location )
   {
      send_to_char( "&rMaybe you should wait until the ship lands.\n\r", ch );
      return;
   }

   if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
   {
      send_to_char( "&rPlease wait till the ship is properly docked.\n\r", ch );
      return;
   }

   if( !ship->hatchopen )
   {
      send_to_char( "&RYou need to open the hatch first.\n\r", ch );
      return;
   }

   if( ( toroom = get_room_index( ship->location ) ) != NULL )
   {
      act( AT_PLAIN, "$n exits the ship.", ch, NULL, argument, TO_ROOM );
      act( AT_PLAIN, "You exit the ship.", ch, NULL, argument, TO_CHAR );
      char_from_room( ch );
      char_to_room( ch, toroom );
      act( AT_PLAIN, "$n steps out of a ship.", ch, NULL, argument, TO_ROOM );
      do_look( ch, "auto" );
   }
   else
      send_to_char( "The exit doesn't seem to be working properly.\n\r", ch );
}

void do_launch( CHAR_DATA * ch, char *argument )
{
   int schance, password;
   long price = 0;
   SHIP_DATA *ship, *ship2;
   char buf[MAX_STRING_LENGTH];

   password = atoi( argument );
   if( argument[0] != '\0' && ( password < 1000 || password > 9999 ) )
   {
      send_to_char( "This is not within the valid range of lock codes.\n\r", ch );
      return;
   }
   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }

   if( ship->class == LAND_VEHICLE )
   {
      send_to_char( "&RThis isn't a spacecraft!\n\r", ch );
      return;
   }

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou don't seem to be in the pilot seat!\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RThe ship is set on autopilot, you'll have to turn it off first.\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
   }
   if( argument[0] != '\0' && password != ship->password )
   {
      send_to_char( "&RYou enter in the wrong code and the computer gives you a small &Yelectric shock&W.\r\n", ch );
      ch->hit -= number_range( 0, 50 );
      return;
   }
   else if( !check_pilot( ch, ship ) && password != ship->password )
   {
      send_to_char( "&RHey, thats not your ship! Try renting a public one.\n\r", ch );
      return;
   }

   if( ship->tractored_by != NULL )
   {
      send_to_char( "&RYou're being held in by a tractor beam!\n\r", ch );
      return;
   }

   if( ( ship2 = ship_from_room( ship->location ) ) && !ship2->bayopen )
   {
      send_to_char( "&RThe bay doors are closed.\n\r", ch );
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

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_smallspace] );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_mediumspace] );
   if( ship->class >= SHIP_CRUISER )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_largespace] );
   if( number_percent(  ) < schance )
   {
      if( is_rental( ch, ship ) )
         if( !rent_ship( ch, ship ) )
            return;
      if( !is_rental( ch, ship ) )
      {
         if( ship->class <= SHIP_SHUTTLE )
            price = 20;
         else if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
            price = 50;
         else
            price = 500;

         price += ( ship->maxhull - ship->hull );
         if( ship->missiles )
            price += ( 50 * ( ship->maxmissiles - ship->missiles ) );
         else if( ship->torpedos )
            price += ( 75 * ( ship->maxtorpedos - ship->torpedos ) );
         else if( ship->rockets )
            price += ( 150 * ( ship->maxrockets - ship->rockets ) );

         if( ship->shipstate == SHIP_DISABLED )
            price += 200;
         if( ship->missilestate == MISSILE_DAMAGED )
            price += 100;
         if( ship->primaryState == LASER_DAMAGED )
            price += 50;
         if( ship->secondaryState == LASER_DAMAGED )
            price += 50;
         if( ship->statet1 == LASER_DAMAGED )
            price += 50;
         if( ship->statet2 == LASER_DAMAGED )
            price += 50;
         if( ship->statet3 == LASER_DAMAGED )
            price += 50;
         if( ship->statet4 == LASER_DAMAGED )
            price += 50;
         if( ship->statet5 == LASER_DAMAGED )
            price += 500;
         if( ship->statet6 == LASER_DAMAGED )
            price += 500;
         if( ship->statet7 == LASER_DAMAGED )
            price += 500;
         if( ship->statet8 == LASER_DAMAGED )
            price += 500;
         if( ship->statet9 == LASER_DAMAGED )
            price += 500;
         if( ship->statet10 == LASER_DAMAGED )
            price += 500;
      }


      if( ch->pcdata && ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, ship->owner ) )
      {
         if( ch->pcdata->clan->funds < price )
         {
            ch_printf( ch, "&R%s doesn't have enough funds to prepare this ship for launch.\n\r", ch->pcdata->clan->name );
            return;
         }

         ch->pcdata->clan->funds -= price;
         ch_printf( ch, "&GIt costs %s %ld credits to ready this ship for launch.\n\r", ch->pcdata->clan->name, price );
      }
      else if( str_cmp( ship->owner, "Public" ) )
      {
         if( ch->gold < price )
         {
            ch_printf( ch, "&RYou don't have enough funds to prepare this ship for launch.\n\r" );
            return;
         }

         ch->gold -= price;
         ch_printf( ch, "&GYou pay %ld credits to ready the ship for launch.\n\r", price );
      }

      ship->energy = ship->maxenergy;
      ship->chaff = ship->maxchaff;
      ship->missiles = ship->maxmissiles;
      ship->torpedos = ship->maxtorpedos;
      ship->rockets = ship->maxrockets;
      ship->shield = 0;
      ship->autorecharge = FALSE;
      ship->autotrack = FALSE;
      ship->autospeed = FALSE;
      ship->hull = ship->maxhull;

      ship->missilestate = MISSILE_READY;
      ship->primaryState = LASER_READY;
      ship->secondaryState = LASER_READY;
      ship->statet1 = LASER_READY;
      ship->statet2 = LASER_READY;
      ship->statet3 = LASER_READY;
      ship->statet4 = LASER_READY;
      ship->statet5 = LASER_READY;
      ship->statet6 = LASER_READY;
      ship->statet7 = LASER_READY;
      ship->statet8 = LASER_READY;
      ship->statet9 = LASER_READY;
      ship->statet10 = LASER_READY;
      ship->shipstate = SHIP_DOCKED;

      if( ship->hatchopen )
      {
         ship->hatchopen = FALSE;
         sprintf( buf, "The hatch on %s closes.", ship->name );
         echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
         echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch slides shut." );
         sound_to_room( get_room_index( ship->entrance ), "!!SOUND(door)" );
         sound_to_room( get_room_index( ship->location ), "!!SOUND(door)" );
      }
      set_char_color( AT_GREEN, ch );
      send_to_char( "&G[&gShip Computer&G]&w Launch sequence initiated.\n\r", ch );
      act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch, NULL, argument, TO_ROOM );
      echo_to_ship( AT_YELLOW, ship, "The ship hums as it lifts off the ground." );
      sprintf( buf, "%s begins to launch.", ship->name );
      echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
      ship->shipstate = SHIP_LAUNCH;
      ship->currspeed = ship->realspeed;

      if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
         learn_from_success( ch, gsn_smallspace );
      if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
         learn_from_success( ch, gsn_mediumspace );
      if( ship->class >= SHIP_CRUISER )
         learn_from_success( ch, gsn_largespace );
      sound_to_ship( ship, "!!SOUND(xwing)" );
      return;
   }
   set_char_color( AT_RED, ch );
   send_to_char( "You fail to work the controls properly!\n\r", ch );
   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      learn_from_failure( ch, gsn_smallspace );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      learn_from_failure( ch, gsn_mediumspace );
   if( ship->class >= SHIP_CRUISER )
      learn_from_failure( ch, gsn_largespace );
   return;

}

void launchship( SHIP_DATA * ship )
{
   char buf[MAX_STRING_LENGTH];
   SHIP_DATA *source;
   int plusminus;
   ROOM_INDEX_DATA *room;
   SPACE_DATA *simul;
   for( simul = first_starsystem; simul; simul = simul->next )
   {
      if( !strcmp( simul->name, "Simulator\0" ) )
         break;
   }
   if( IS_SET( ship->flags, SHIP_SIMULATOR ) && simul )
   {
      ship_to_starsystem( ship, simul );
      ship->currjump = simul;
      ship->jx = 11;
      ship->jy = -11;
      ship->jz = 11;
   }
   else
   {
      ship_to_starsystem( ship, starsystem_from_room( ship->in_room ) );
   }
   if( ship->starsystem == NULL )
   {
      echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ),
                    "&G[&gShip Computer&G]&w Launch path blocked .. Launch aborted." );
      echo_to_ship( AT_YELLOW, ship, "The ship slowly sets back back down on the landing pad." );
      sprintf( buf, "%s slowly sets back down.", ship->name );
      echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
      ship->shipstate = SHIP_DOCKED;
      return;
   }

   source = ship_from_room( ship->in_room->vnum );
   extract_ship( ship );

   ship->location = 0;

   if( ship->shipstate != SHIP_DISABLED )
      ship->shipstate = SHIP_READY;

   plusminus = number_range( -1, 2 );
   if( plusminus > 0 )
      ship->hx = 1;
   else
      ship->hx = -1;

   plusminus = number_range( -1, 2 );
   if( plusminus > 0 )
      ship->hy = 1;
   else
      ship->hy = -1;

   plusminus = number_range( -1, 2 );
   if( plusminus > 0 )
      ship->hz = 1;
   else
      ship->hz = -1;

   if( ( room = get_room_index( ship->lastdoc ) ) != NULL &&
       room->area && room->area->planet && room->area->planet->starsystem
       && room->area->planet->starsystem == ship->starsystem )
   {
      ship->vx = room->area->planet->x;
      ship->vy = room->area->planet->y;
      ship->vz = room->area->planet->z;
   }
   else if( source )
   {
      ship->vx = source->vx;
      ship->vy = source->vy;
      ship->vz = source->vz;
   }

   ship->energy -= 100;

   ship->vx += ( ship->hx * ship->currspeed * 2 );
   ship->vy += ( ship->hy * ship->currspeed * 2 );
   ship->vz += ( ship->hz * ship->currspeed * 2 );

   echo_to_room( AT_GREEN, get_room_index( ship->location ), "Launch complete.\n\r" );
   echo_to_ship( AT_YELLOW, ship, "The ship leaves the platform far behind as it flies into space." );
   sprintf( buf, "%s enters the starsystem at %.0f %.0f %.0f", ship->name, ship->vx, ship->vy, ship->vz );
   echo_to_system( AT_YELLOW, ship, buf, NULL );
   sprintf( buf, "%s lifts off into space.", ship->name );
   echo_to_room( AT_YELLOW, get_room_index( ship->lastdoc ), buf );
   ship->inship = NULL;

}

void showland( AREA_DATA * tarea, CHAR_DATA * ch )
{
   ROOM_INDEX_DATA *room;
   int vnum;
   int num = 0;
   for( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) != NULL )
      {
         if( IS_SET( room->room_flags2, ROOM_HIDDENPAD ) )
            continue;
         if( IS_SET( room->room_flags, ROOM_CAN_LAND ) )
         {
            num += 1;
            ch_printf( ch, "&c| &c(&C%2.2d&c) &Y%-43.43s &c|\n\r", num, room->name );
         }
      }
   }
   return;

}

void do_land( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   int schance;
   SHIP_DATA *ship;
   SHIP_DATA *target;
   PLANET_DATA *planet;
   bool pfound = FALSE;
   ROOM_INDEX_DATA *room;
   bool rfound = FALSE;
   int vnum;

   strcpy( arg, argument );
   argument = one_argument( argument, arg1 );

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the pilots seat of a ship to do that!\n\r", ch );
      return;
   }

   if( ship->class > SHIP_SPACE_STATION )
   {
      send_to_char( "&RThis isn't a spacecraft!\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first.\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RYou can't land space stations.\n\r", ch );
      return;
   }

   if( ship->class >= SHIP_CRUISER )
   {
      send_to_char( "&RCapital ships are far to large to land.\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RThe ships drive is disabled. Unable to land.\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RThe ship is already docked!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }

   if( ship->shipstate != SHIP_READY )
   {
      send_to_char( "&RPlease wait until the ship has finished its current maneuver.\n\r", ch );
      return;
   }
   if( ship->starsystem == NULL )
   {
      send_to_char( "&RThere's nowhere to land around here!", ch );
      return;
   }

   if( ship->energy < 25 )
   {
      send_to_char( "&RTheres not enough fuel!\n\r", ch );
      return;
   }
   if( ship->tractored_by )
   {
      send_to_char( "&RYou can't land while locked in a tractor beam!\n\r", ch );
      return;
   }

   if( ship->tractoring )
   {
      send_to_char( "&RYou can't land while you have a ship in tow!\n\r", ch );
      return;
   }
   if( arg[0] == '\0' )
   {
      set_char_color( AT_CYAN, ch );
      ch_printf( ch, "%s", "Land where?\n\r\n\rChoices: \n\r" );

      for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
      {
         /*
          * for(hanger=target->first_hanger;hanger;hanger=hanger->next)
          */
         if( target->hanger1 || target->hanger2 )
            ch_printf( ch, "%s%s    %.0f %.0f %.0f\n\r         ",
                       target->class == 11 ? "Asteroid" : "", target->name, target->vx, target->vy, target->vz );
      }
      for( planet = ship->starsystem->first_planet; planet; planet = planet->next_in_system )
         ch_printf( ch, "%s%s   %d %d %d\n\r",
                    planet->controls != 0 ? "Asteroid " : "", planet->name, planet->x, planet->y, planet->z );

      ch_printf( ch, "\n\rYour Coordinates: %.0f %.0f %.0f\n\r", ship->vx, ship->vy, ship->vz );
      return;
   }


   for( planet = ship->starsystem->first_planet; planet; planet = planet->next_in_system )
   {
      if( !str_prefix( arg1, planet->name ) )
      {
         pfound = TRUE;

         if( !planet->area )
         {
            send_to_char( "&RThat planet doesn't have any landing areas.\n\r", ch );
            return;
         }

         if( ( planet->x > ship->vx + 200 ) || ( planet->x < ship->vx - 200 ) ||
             ( planet->y > ship->vy + 200 ) || ( planet->y < ship->vy - 200 ) ||
             ( planet->z > ship->vz + 200 ) || ( planet->z < ship->vz - 200 ) )
         {
            send_to_char( "&RThat planet is too far away! You'll have to fly a little closer.\n\r", ch );
            return;
         }

         if( argument[0] != '\0' )
         {
            AREA_DATA *pArea;
            for( pArea = planet->first_area; pArea; pArea = pArea->next_on_planet )
            {
               for( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
               {
                  if( ( room = get_room_index( vnum ) ) == NULL )
                     continue;
                  if( IS_SET( room->room_flags, ROOM_CAN_LAND ) && !str_prefix( argument, room->name ) )
                  {
                     if( IS_SET( room->room_flags2, ROOM_CLANLAND ) )
                     {
                        if( room->area
                            && room->area->planet
                            && room->area->planet->governed_by
                            && ch->pcdata->clan
                            && str_cmp( room->area->planet->governed_by->name, ship->owner )
                            && str_cmp( room->area->planet->governed_by->name, ch->pcdata->clan->name ) )
                        {
                           act( AT_RED,
                                "Just as you're about to land, several planetary defense ships appear from the clouds and order you to fall back.",
                                ch, NULL, NULL, TO_CHAR );
                           act( AT_RED,
                                "Just as $n is about to land, several planetary defense ships appear from the clouds and order $m to fall back.",
                                ch, NULL, NULL, TO_ROOM );
                           return;
                        }
                        if( !ch->pcdata->clan )
                        {
                           act( AT_RED,
                                "Just as you're about to land, several planetary defense ships appear from the clouds and order you to fall back.",
                                ch, NULL, NULL, TO_CHAR );
                           act( AT_RED,
                                "Just as $n is about to land, several planetary defense ships appear from the clouds and order $m to fall back.",
                                ch, NULL, NULL, TO_ROOM );
                           return;
                        }
                     }


                     rfound = TRUE;
                     break;
                  }
               }
            }
         }

         if( !rfound )
         {
            AREA_DATA *pArea;
            send_to_char( "&c+--------------------------------------------------+\n\r", ch );
            send_to_char( "&c| &CPlease type the location after the planet name.  &c|\n\r", ch );
            ch_printf( ch, "&c| &CPossible choices for %-26.26s  &c|\n\r", planet->name );
            send_to_char( "&c+--------------------------------------------------+\n\r", ch );
            for( pArea = planet->first_area; pArea; pArea = pArea->next_on_planet )
            {
               showland( pArea, ch );
            }
            send_to_char( "&c+--------------------------------------------------+\n\r", ch );
            return;
         }

         break;
      }
   }

   if( !pfound )
   {
      target = get_ship_here( arg, ship->starsystem );

      if( target != NULL )
      {
         if( target == ship )
         {
            send_to_char( "&RYou can't land your ship inside itself!\n\r", ch );
            return;
         }
         if( ( target->hanger1 &&
               ( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE ) )
             || ( target->hanger2 && ( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE ) ) )
         {
            if( ( target->hanger1 && !target->hanger2 )
                && ( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE ) )
            {
               send_to_char( "&RThat ship only has a small-sized ship hanger.\n\r", ch );
               return;
            }
            if( ship->class >= SHIP_FREIGHTER && target->hanger2 == 0 )
            {
               send_to_char( "&RThat ship doesn't have a midsized ship hanger.\n\r", ch );
               return;
            }
            if( !target->hanger1 && !target->hanger2 )
            {
               send_to_char( "&RThat ship has no hanger to land in!\n\r", ch );
               return;
            }

         }
         if( ( target->vx > ship->vx + 200 ) || ( target->vx < ship->vx - 200 ) ||
             ( target->vy > ship->vy + 200 ) || ( target->vy < ship->vy - 200 ) ||
             ( target->vz > ship->vz + 200 ) || ( target->vz < ship->vz - 200 ) )
         {
            send_to_char( "&R That ship is too far away! You'll have to fly a little closer.\n\r", ch );
            return;
         }


         if( target->bayopen == FALSE )
         {
            send_to_char( "&RThe bay doors on that ship are closed.\n\r", ch );
            return;
         }

      }
      else
      {
         send_to_char( "&RI don't see that here.\n\r&W", ch );
         do_land( ch, "" );
         return;
      }
   }

   schance = IS_NPC( ch ) ? 100 : ( int )( ch->pcdata->learned[gsn_shipsystems] );
   if( number_percent(  ) < schance )
   {
      set_char_color( AT_GREEN, ch );
      send_to_char( "&G[&gShip Computer&G] &wLanding sequence initiated.\n\r", ch );
      act( AT_PLAIN, "$n begins the landing sequence.", ch, NULL, "", TO_ROOM );
      echo_to_ship( AT_YELLOW, ship, "The ship slowly begins its landing approach." );
      ship->dest = STRALLOC( arg );
      ship->shipstate = SHIP_LAND;
      ship->currspeed = 0;

      if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
         learn_from_success( ch, gsn_smallspace );
      if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
         learn_from_success( ch, gsn_mediumspace );


      if( get_room_index( ship->lastdoc ) && starsystem_from_room( get_room_index( ship->lastdoc ) ) != ship->starsystem )
      {
         int xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) );
         xp = UMIN( get_ship_value( ship ), xp );
         gain_exp( ch, xp, PILOTING_ABILITY );
         ch_printf( ch, "&WYou gain %ld points of flight experience!\n\r", UMIN( get_ship_value( ship ), xp ) );
      }

      return;
   }
   send_to_char( "You fail to work the controls properly.\n\r", ch );
   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      learn_from_failure( ch, gsn_smallspace );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      learn_from_failure( ch, gsn_mediumspace );;
   return;
}

void landship( SHIP_DATA * ship, char *argument )
{
   SHIP_DATA *target;
   char buf[MAX_STRING_LENGTH];
   int destination = 0;
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   //HANGER_DATA *hanger;
   PLANET_DATA *planet;
   ROOM_INDEX_DATA *room;
   AREA_DATA *pArea;
   int vnum;

   strcpy( arg, argument );
   argument = one_argument( argument, arg1 );

   for( planet = ship->starsystem->first_planet; planet; planet = planet->next_in_system )
   {
      if( !str_prefix( arg1, planet->name ) )
      {
         for( pArea = planet->first_area; pArea; pArea = pArea->next_on_planet )
         {
            for( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
            {
               if( ( room = get_room_index( vnum ) ) == NULL )
                  continue;
               if( IS_SET( room->room_flags, ROOM_CAN_LAND ) && !str_prefix( argument, room->name ) )
               {
                  destination = room->vnum;
                  break;
               }
            }
         }

         break;
      }
   }

   if( destination != 0 )
   {

      if( !ship_to_room( ship, destination ) )
      {
         echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Could not complete approach. Landing aborted." );
         echo_to_ship( AT_YELLOW, ship, "The ship pulls back up out of its landing sequence." );
         if( ship->shipstate != SHIP_DISABLED )
            ship->shipstate = SHIP_READY;
         return;
      }

      echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "&G[&gShip Computer&G] &wLanding sequence complete." );
      echo_to_ship( AT_YELLOW, ship, "You feel a slight thud as the ship sets down on the ground." );
      sprintf( buf, "%s disappears from your scanner.", ship->name );
      echo_to_system( AT_YELLOW, ship, buf, NULL );

      extract_ship( ship );
      ship_to_room( ship, destination );
      ship->location = destination;
      ship->lastdoc = ship->location;
      if( ship->shipstate != SHIP_DISABLED )
         ship->shipstate = SHIP_DOCKED;
      ship_from_starsystem( ship, ship->starsystem );

      sprintf( buf, "%s lands on the platform.", ship->name );
      echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );

      ship->energy = ship->maxenergy;

      save_ship( ship );
      return;

   }
   else
   {
      ROOM_INDEX_DATA *droom;
      target = get_ship_here( arg, ship->starsystem );

      if( ( ship->type == 0 || ship->type == 1 ) && target->hanger1 )
         droom = get_room_index( target->hanger1 );
      if( ship->type == 2 )
         droom = get_room_index( target->hanger2 );
      if( !ship_to_room( ship, room->vnum ) )
      {
         echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Could not complete approach. Landing aborted." );
         echo_to_ship( AT_YELLOW, ship, "The ship pulls back up out of its landing sequence." );
         if( ship->shipstate != SHIP_DISABLED )
            ship->shipstate = SHIP_READY;
         return;
      }
      extract_ship( ship );
      ship_to_room( ship, droom->vnum );
      echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Landing sequence complete." );
      echo_to_ship( AT_YELLOW, ship, "You feel a slight thud as the ship sets down on the ground." );
      sprintf( buf, "%s disappears from your scanner.", ship->name );
      echo_to_system( AT_YELLOW, ship, buf, NULL );
      ship_from_starsystem( ship, ship->starsystem );
      ship->energy = ship->energy - 25;
      ship->shipstate = SHIP_DOCKED;
      ship->location = droom->vnum;
      ship->lastdoc = ship->location;
      save_ship( ship );
      return;
   }
}

void do_accelerate( CHAR_DATA * ch, char *argument )
{
   int schance;
   int change;
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];

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

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RThe controls must be at the pilots chair...\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first.\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RSpace stations can't move.\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RThe ships drive is disabled. Unable to accelerate.\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RYou can't do that until after you've launched!\n\r", ch );
      return;
   }
   if( ship->energy < abs( ( atoi( argument ) - abs( ship->currspeed ) ) / 10 ) )
   {
      send_to_char( "&RTheres not enough fuel!\n\r", ch );
      return;
   }

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_smallspace] );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_mediumspace] );
   if( ship->class >= SHIP_CRUISER )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_largespace] );

   if( number_percent(  ) >= schance )
   {
      send_to_char( "&RYou fail to work the controls properly.\n\r", ch );
      if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
         learn_from_failure( ch, gsn_smallspace );
      if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
         learn_from_failure( ch, gsn_mediumspace );
      if( ship->class >= SHIP_CRUISER )
         learn_from_failure( ch, gsn_largespace );
      return;
   }

   change = atoi( argument );

   act( AT_PLAIN, "$n manipulates the ships controls.", ch, NULL, argument, TO_ROOM );

   if( change > ship->currspeed )
   {
      send_to_char( "&GAccelerating.\n\r", ch );
      echo_to_cockpit( AT_YELLOW, ship, "The ship begins to accelerate." );
      sprintf( buf, "%s begins to speed up.", ship->name );
      echo_to_system( AT_ORANGE, ship, buf, NULL );
   }

   if( change < ship->currspeed )
   {
      send_to_char( "&GDecelerating.\n\r", ch );
      echo_to_cockpit( AT_YELLOW, ship, "The ship begins to slow down." );
      sprintf( buf, "%s begins to slow down.", ship->name );
      echo_to_system( AT_ORANGE, ship, buf, NULL );
   }

   ship->energy -= abs( ( change - abs( ship->currspeed ) ) / 10 );

   ship->currspeed = URANGE( 0, change, ship->realspeed );

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      learn_from_success( ch, gsn_smallspace );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      learn_from_success( ch, gsn_mediumspace );
   if( ship->class >= SHIP_CRUISER )
      learn_from_success( ch, gsn_largespace );

}

void do_trajectory( CHAR_DATA * ch, char *argument )
{
   PLANET_DATA *planet;
   bool foundplanet = FALSE;
   char buf[MAX_STRING_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   int schance;
   float vx, vy, vz;
   SHIP_DATA *ship;


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

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYour not in the pilots seat.\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first.\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RThe ships drive is disabled. Unable to manuever.\n\r", ch );
      return;
   }
   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RSpace stations can't turn.\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RYou can't do that until after you've launched!\n\r", ch );
      return;
   }
   if( ship->shipstate != SHIP_READY )
   {
      send_to_char( "&RPlease wait until the ship has finished its current manouver.\n\r", ch );
      return;
   }
   if( ship->energy < ( ship->currspeed / 10 ) )
   {
      send_to_char( "&RTheres not enough fuel!\n\r", ch );
      return;
   }

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_smallspace] );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_mediumspace] );
   if( ship->class >= SHIP_CRUISER )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_largespace] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou fail to work the controls properly.\n\r", ch );
      if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
         learn_from_failure( ch, gsn_smallspace );
      if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
         learn_from_failure( ch, gsn_mediumspace );
      if( ship->class >= SHIP_CRUISER )
         learn_from_failure( ch, gsn_largespace );
      return;
   }

   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( arg3[0] == '\0' )
   {
      for( planet = ship->starsystem->first_planet; planet; planet = planet->next_in_system )
      {
         if( !str_prefix( arg2, planet->name ) )
         {
            foundplanet = TRUE;
            vx = planet->x;
            vy = planet->y;
            vz = planet->z;
            break;
         }
      }
   }

   if( !foundplanet )
   {
      if( !str_cmp( arg2, "inf" ) )
         vx = 0;
      else
         vx = atof( arg2 );
      if( !str_cmp( arg3, "inf" ) )
         vy = 0;
      else
         vy = atof( arg3 );
      if( !str_cmp( argument, "inf" ) )
         vz = 0;
      else
         vz = atof( argument );
   }

   if( vx == ship->vx && vy == ship->vy && vz == ship->vz )
   {
      ch_printf( ch, "The ship is already at %.0f %.0f %.0f !", vx, vy, vz );
   }

   ship->hx = vx - ship->vx;
   ship->hy = vy - ship->vy;
   ship->hz = vz - ship->vz;

   ship->energy -= ( ship->currspeed / 10 );

   ch_printf( ch, "&GNew course set, aproaching %.0f %.0f %.0f.\n\r", vx, vy, vz );
   act( AT_PLAIN, "$n manipulates the ships controls.", ch, NULL, argument, TO_ROOM );

   echo_to_cockpit( AT_YELLOW, ship, "The ship begins to turn.\n\r" );
   sprintf( buf, "%s turns altering its present course.", ship->name );
   echo_to_system( AT_ORANGE, ship, buf, NULL );

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      ship->shipstate = SHIP_BUSY_3;
   else if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      ship->shipstate = SHIP_BUSY_2;
   else
      ship->shipstate = SHIP_BUSY;

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      learn_from_success( ch, gsn_smallspace );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      learn_from_success( ch, gsn_mediumspace );
   if( ship->class >= SHIP_CRUISER )
      learn_from_success( ch, gsn_largespace );

}

void do_buyship( CHAR_DATA * ch, char *argument )
{
   long price;
   SHIP_DATA *ship;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "&ROnly players can do that!\n\r", ch );
      return;
   }

   ship = ship_in_room( ch->in_room, argument );
   if( !ship )
   {
      ship = ship_from_cockpit( ch->in_room->vnum );

      if( !ship )
      {
         act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
         return;
      }
   }

   if( str_cmp( ship->owner, "" ) || ship->type == MOB_SHIP )
   {
      send_to_char( "&RThat ship isn't for sale!", ch );
      return;
   }


   if( ship->type == SHIP_IMPERIAL )
   {
      if( !ch->pcdata->clan || str_cmp( ch->pcdata->clan->name, "the empire" ) )
      {
         if( !ch->pcdata->clan || !ch->pcdata->clan->mainclan || str_cmp( ch->pcdata->clan->mainclan->name, "The Empire" ) )
         {
            send_to_char( "&RThat ship may only be purchased by the Empire!\n\r", ch );
            return;
         }
      }
   }
   else if( ship->type == SHIP_REPUBLIC )
   {
      if( !ch->pcdata->clan || str_cmp( ch->pcdata->clan->name, "the new republic" ) )
      {
         if( !ch->pcdata->clan || !ch->pcdata->clan->mainclan
             || str_cmp( ch->pcdata->clan->mainclan->name, "The New Republic" ) )
         {
            send_to_char( "&RThat ship may only be purchased by The New Republic!\n\r", ch );
            return;
         }
      }
   }

   price = get_ship_value( ship );

   if( ch->gold < price )
   {
      ch_printf( ch, "&RThis ship costs %ld. You don't have enough credits!\n\r", price );
      return;
   }

   ch->gold -= price;
   ch_printf( ch, "&GYou pay %ld credits to purchace the ship.\n\r", price );

   act( AT_PLAIN, "$n walks over to a terminal and makes a credit transaction.", ch, NULL, argument, TO_ROOM );

   STRFREE( ship->owner );
   ship->owner = STRALLOC( ch->name );
   save_ship( ship );

}

void do_clanbuyship( CHAR_DATA * ch, char *argument )
{
   long price;
   SHIP_DATA *ship;
   CLAN_DATA *clan;
   CLAN_DATA *mainclan;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "&ROnly players can do that!\n\r", ch );
      return;
   }
   if( !ch->pcdata->clan )
   {
      send_to_char( "&RYou aren't a member of any organizations!\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;
   mainclan = ch->pcdata->clan->mainclan ? ch->pcdata->clan->mainclan : clan;

   if( ( ch->pcdata->bestowments
         && is_name( "clanbuyship", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
      ;
   else
   {
      send_to_char( "&RYour organization hasn't seen fit to bestow you with that ability.\n\r", ch );
      return;
   }

   ship = ship_in_room( ch->in_room, argument );
   if( !ship )
   {
      ship = ship_from_cockpit( ch->in_room->vnum );

      if( !ship )
      {
         act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
         return;
      }
   }

   if( str_cmp( ship->owner, "" ) || ship->type == MOB_SHIP )
   {
      send_to_char( "&RThat ship isn't for sale!\n\r", ch );
      return;
   }

   if( str_cmp( mainclan->name, "The Empire" ) && ship->type == SHIP_IMPERIAL )
   {
      send_to_char( "&RThat ship may only be purchaced by the Empire!\n\r", ch );
      return;
   }

   if( str_cmp( mainclan->name, "The New Republic" ) && ship->type == SHIP_REPUBLIC )
   {
      send_to_char( "&RThat ship may only be purchaced by The New Republic!\n\r", ch );
      return;
   }

   if( !str_cmp( mainclan->name, "The Empire" ) && ship->type != SHIP_IMPERIAL )
   {
      send_to_char( "&RDue to contractual agreements that ship may not be purchaced by the empire!\n\r", ch );
      return;
   }

   if( !str_cmp( mainclan->name, "The New Republic" ) && ship->type != SHIP_REPUBLIC )
   {
      send_to_char( "&RBecause of contractual agreements, the NR can only purchase NR ships!\n\r", ch );
      return;
   }

   price = get_ship_value( ship );

   if( ch->pcdata->clan->funds < price )
   {
      ch_printf( ch, "&RThis ship costs %ld. You don't have enough credits!\n\r", price );
      return;
   }

   clan->funds -= price;
   ch_printf( ch, "&G%s pays %ld credits to purchace the ship.\n\r", clan->name, price );

   act( AT_PLAIN, "$n walks over to a terminal and makes a credit transaction.", ch, NULL, argument, TO_ROOM );

   STRFREE( ship->owner );
   ship->owner = STRALLOC( clan->name );
   save_ship( ship );

   if( ship->class != LAND_VEHICLE )
      clan->spacecraft++;
   else
      clan->vehicles++;
}

void do_sellship( CHAR_DATA * ch, char *argument )
{
   long price;
   SHIP_DATA *ship;

   ship = ship_in_room( ch->in_room, argument );
   if( !ship )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
      return;
   }

   if( str_cmp( ship->owner, ch->name ) )
   {
      send_to_char( "&RThat isn't your ship!", ch );
      return;
   }

   if( !IS_NPC( ch ) && ch->top_level < 11 )
   {
      send_to_char( "Due to cheating, players under level 11 are not allowed to sell ships.\n\r", ch );
      return;
   }

   price = get_ship_value( ship );

   ch->gold += ( price - price / 10 );
   ch_printf( ch, "&GYou receive %ld credits from selling your ship.\n\r", price - price / 10 );

   act( AT_PLAIN, "$n walks over to a terminal and makes a credit transaction.", ch, NULL, argument, TO_ROOM );

   STRFREE( ship->owner );
   ship->owner = STRALLOC( "" );
   save_ship( ship );

}

void do_info( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   SHIP_DATA *target;
   char buf[MAX_STRING_LENGTH];

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      if( argument[0] == '\0' )
      {
         act( AT_PLAIN, "Which ship do you want info on?.", ch, NULL, NULL, TO_CHAR );
         return;
      }

      ship = ship_in_room( ch->in_room, argument );
      if( !ship )
      {
         act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
         return;
      }

      target = ship;
   }
   else if( argument[0] == '\0' )
      target = ship;
   else
      target = get_ship_here( argument, ship->starsystem );

   if( target == NULL )
   {
      send_to_char( "&RI don't see that here.\n\rTry the radar, or type info by itself for info on this ship.\n\r", ch );
      return;
   }

   if( abs( target->vx - ship->vx ) > 500 + ship->sensor * 2 ||
       abs( target->vy - ship->vy ) > 500 + ship->sensor * 2 || abs( target->vz - ship->vz ) > 500 + ship->sensor * 2 )
   {
      send_to_char( "&RThat ship is to far away to scan.\n\r", ch );
      return;
   }
   send_to_char( "You take a look at the ship...\n\r\n\r", ch );

   ch_printf( ch, "&G| &W%s %s:&w %s &G|\n\r\n\r",
              target->type == SHIP_REPUBLIC ? "New Republic" :
              target->type == SHIP_IMPERIAL ? "Imperial" :
              target->type == PLAYER_SHIP ? "Civilian" : "Mob",
              target->class == SHIP_FIGHTER ? "Starfighter" :
              target->class == SHIP_BOMBER ? "Bomber" :
              target->class == SHIP_SHUTTLE ? "Shuttle" :
              target->class == SHIP_FREIGHTER ? "Freighter" :
              target->class == SHIP_FRIGATE ? "Frigate" :
              target->class == SHIP_TT ? "Troop Transport" :
              target->class == SHIP_CORVETTE ? "Corvette" :
              target->class == SHIP_CRUISER ? "Cruiser" :
              target->class == SHIP_DREADNAUGHT ? "Dreadnaught" :
              target->class == SHIP_DESTROYER ? "Star Destroyer" :
              target->class == SHIP_SPACE_STATION ? "Space Station" :
              target->class == LAND_VEHICLE ? "Land vehicle" : "Unknown", target->name );

   if( !str_cmp( target->owner, ch->name ) || !str_cmp( target->pilot, ch->name ) || !str_cmp( target->copilot, ch->name ) )
      ch_printf( ch, "&G| &WOwner: &w%s&G || &WPilot: &w%s&G || &WCoPilot: &w%s&G |\n\r",
                 target->owner != '\0' ? target->owner : "None.", target->pilot != '\0' ? target->pilot : "None.",
                 target->copilot != '\0' ? target->copilot : "None." );
   else if( !str_cmp( target->owner, "Public" ) || !str_cmp( target->owner, "public" ) )
      ch_printf( ch, "&G| &WOwner: &wPublic &G || &WPilot: &w---------&G || &WCoPilot: &w---------&G |\n\r" );
   else if( ch->pcdata->clan )
   {
      if( !str_cmp( target->owner, ch->pcdata->clan->name ) )
         ch_printf( ch, "&G| &WOwner: &w%s&G || &WPilot: &w%s&G || &WCoPilot: &w%s&G |\n\r",
                    target->owner != '\0' ? target->owner : "None.", target->pilot != '\0' ? target->pilot : "None.",
                    target->copilot != '\0' ? target->copilot : "None." );
   }
   else
      ch_printf( ch, "&G| &WOwner: &w%s&G || &WPilot: &w%s&G || &WCoPilot: &w%s&G |\n\r\n\r",
                 ship->owner != '\0' ? "---------" : "None.", "---------", "---------" );

   sprintf( buf, "$n checks over %s thoroughly.", target->name );
   act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );

}

void do_autorecharge( CHAR_DATA * ch, char *argument )
{
   int schance;
   SHIP_DATA *ship;
   int recharge;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }

   if( ( ship = ship_from_coseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the co-pilots seat!\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first.\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_shipsystems] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou fail to work the controls properly.\n\r", ch );
      learn_from_failure( ch, gsn_shipsystems );
      return;
   }

   act( AT_PLAIN, "$n flips a switch on the control panell.", ch, NULL, argument, TO_ROOM );

   if( !str_cmp( argument, "on" ) )
   {
      ship->autorecharge = TRUE;
      send_to_char( "&GYou power up the shields.\n\r", ch );
      echo_to_cockpit( AT_YELLOW, ship, "Shields ON. Autorecharge ON." );
   }
   else if( !str_cmp( argument, "off" ) )
   {
      ship->autorecharge = FALSE;
      send_to_char( "&GYou shutdown the shields.\n\r", ch );
      echo_to_cockpit( AT_YELLOW, ship, "Shields OFF. Shield strength set to 0. Autorecharge OFF." );
      ship->shield = 0;
   }
   else if( !str_cmp( argument, "idle" ) )
   {
      ship->autorecharge = FALSE;
      send_to_char( "&GYou let the shields idle.\n\r", ch );
      echo_to_cockpit( AT_YELLOW, ship, "Autorecharge OFF. Shields IDLEING." );
   }
   else
   {
      if( ship->autorecharge == TRUE )
      {
         ship->autorecharge = FALSE;
         send_to_char( "&GYou toggle the shields.\n\r", ch );
         echo_to_cockpit( AT_YELLOW, ship, "Autorecharge OFF. Shields IDLEING." );
      }
      else
      {
         ship->autorecharge = TRUE;
         send_to_char( "&GYou toggle the shields.\n\r", ch );
         echo_to_cockpit( AT_YELLOW, ship, "Shields ON. Autorecharge ON" );
      }
   }

   if( ship->autorecharge )
   {
      recharge = URANGE( 1, ship->maxshield - ship->shield, 25 + ship->class * 25 );
      recharge = UMIN( recharge, ship->energy * 5 + 100 );
      ship->shield += recharge;
      ship->energy -= ( recharge * 2 + recharge * ship->class );
   }
   if( ship->shield > ship->maxshield )
      ship->shield = ship->maxshield;
   learn_from_success( ch, gsn_shipsystems );
}

void do_autopilot( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the pilots seat!\n\r", ch );
      return;
   }

   if( !check_pilot( ch, ship ) )
   {
      send_to_char( "&RHey! Thats not your ship!\n\r", ch );
      return;
   }

   if( ship->target0 || ship->target1 || ship->target2 )
   {
      send_to_char( "&RNot while the ship is enganged with an enemy!\n\r", ch );
      return;
   }


   act( AT_PLAIN, "$n flips a switch on the control panell.", ch, NULL, argument, TO_ROOM );

   if( ship->autopilot == TRUE )
   {
      ship->autopilot = FALSE;
      send_to_char( "&GYou toggle the autopilot.\n\r", ch );
      echo_to_cockpit( AT_YELLOW, ship, "Autopilot OFF." );
   }
   else
   {
      ship->autopilot = TRUE;
      ship->autorecharge = TRUE;
      send_to_char( "&GYou toggle the autopilot.\n\r", ch );
      echo_to_cockpit( AT_YELLOW, ship, "Autopilot ON." );
   }

}

void do_openhatch( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   int password;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( !arg1 || arg1[0] == '\0' || !str_cmp( argument, "hatch" ) )
   {
      ship = ship_from_entrance( ch->in_room->vnum );
      if( ship == NULL )
      {
         send_to_char( "&ROpen what?\n\r", ch );
         return;
      }
      else
      {
         if( !ship->hatchopen )
         {

            if( ship->class >= SHIP_CRUISER )
            {
               send_to_char( "&RTry one of the docking bays!\n\r", ch );
               return;
            }
            if( ship->location != ship->lastdoc || ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED ) )
            {
               send_to_char( "&RPlease wait till the ship lands!\n\r", ch );
               return;
            }
            ship->hatchopen = TRUE;
            send_to_char( "&GYou open the hatch.\n\r", ch );
            act( AT_PLAIN, "$n opens the hatch.", ch, NULL, argument, TO_ROOM );
            sprintf( buf, "The hatch on %s opens.", ship->name );
            echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
            sound_to_room( get_room_index( ship->entrance ), "!!SOUND(door)" );
            sound_to_room( get_room_index( ship->location ), "!!SOUND(door)" );
            return;
         }
         else
         {
            send_to_char( "&RIt's already open.\n\r", ch );
            return;
         }
      }
   }

   password = atoi( arg2 );
   if( ( arg2[0] != '\0' ) && ( password < 1000 || password > 9999 ) )
   {
      send_to_char( "This is not within the valid range of lock codes.\n\r", ch );
      return;
   }
   ship = ship_in_room( ch->in_room, arg1 );
   if( !ship )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
      return;
   }

   if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
   {
      send_to_char( "&RThat ship has already started to launch.\n\r", ch );
      return;
   }

   if( arg2[0] != '\0' && password != ship->password )
   {
      send_to_char( "&RYou enter in the wrong code and the computer gives you a small &Yelectric shock&R.\r\n", ch );
      ch->hit -= number_range( 0, 75 );
      return;
   }

   else if( !check_pilot( ch, ship ) && password != ship->password )
   {
      send_to_char( "&RHey! Thats not your ship!\n\r", ch );
      return;
   }
   if( !ship->hatchopen )
   {
      ship->hatchopen = TRUE;
      act( AT_PLAIN, "You open the hatch on $T.", ch, NULL, ship->name, TO_CHAR );
      act( AT_PLAIN, "$n opens the hatch on $T.", ch, NULL, ship->name, TO_ROOM );
      echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch opens from the outside." );
      sound_to_room( get_room_index( ship->entrance ), "!!SOUND(door)" );
      sound_to_room( get_room_index( ship->location ), "!!SOUND(door)" );
      return;
   }

   send_to_char( "&GIts already open!\n\r", ch );

}

void do_closehatch( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];

   if( !argument || argument[0] == '\0' || !str_cmp( argument, "hatch" ) )
   {
      ship = ship_from_entrance( ch->in_room->vnum );
      if( ship == NULL )
      {
         send_to_char( "&RClose what?\n\r", ch );
         return;
      }
      else
      {

         if( ship->class == SHIP_SPACE_STATION )
         {
            send_to_char( "&RTry one of the docking bays!\n\r", ch );
            return;
         }
         if( ship->hatchopen )
         {
            ship->hatchopen = FALSE;
            send_to_char( "&GYou close the hatch.\n\r", ch );
            act( AT_PLAIN, "$n closes the hatch.", ch, NULL, argument, TO_ROOM );
            sprintf( buf, "The hatch on %s closes.", ship->name );
            echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
            sound_to_room( get_room_index( ship->entrance ), "!!SOUND(door)" );
            sound_to_room( get_room_index( ship->location ), "!!SOUND(door)" );
            return;
         }
         else
         {
            send_to_char( "&RIt's already closed.\n\r", ch );
            return;
         }
      }
   }

   ship = ship_in_room( ch->in_room, argument );
   if( !ship )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
      return;
   }

   if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
   {
      send_to_char( "&RThat ship has already started to launch", ch );
      return;
   }
   else
   {
      if( ship->hatchopen )
      {
         ship->hatchopen = FALSE;
         act( AT_PLAIN, "You close the hatch on $T.", ch, NULL, ship->name, TO_CHAR );
         act( AT_PLAIN, "$n closes the hatch on $T.", ch, NULL, ship->name, TO_ROOM );
         echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch is closed from outside." );
         sound_to_room( get_room_index( ship->entrance ), "!!SOUND(door)" );
         sound_to_room( get_room_index( ship->location ), "!!SOUND(door)" );

         return;
      }
      else
      {
         send_to_char( "&RIts already closed.\n\r", ch );
         return;
      }
   }


}

void do_status( CHAR_DATA * ch, char *argument )
{
   int schance, x;
   SHIP_DATA *ship;
   SHIP_DATA *target;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit, turret or engineroom of a ship to do that!\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
      target = ship;
   else
      target = get_ship_here( argument, ship->starsystem );

   if( target == NULL )
   {
      send_to_char( "&RI don't see that here.\n\rTry the radar, or type status by itself for your ships status.\n\r", ch );
      return;
   }

   if( abs( target->vx - ship->vx ) > 500 + ship->sensor * 2 ||
       abs( target->vy - ship->vy ) > 500 + ship->sensor * 2 || abs( target->vz - ship->vz ) > 500 + ship->sensor * 2 )
   {
      send_to_char( "&RThat ship is to far away to scan.\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_shipsystems] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou cant figure out what the readout means.\n\r", ch );
      learn_from_failure( ch, gsn_shipsystems );
      return;
   }

   act( AT_PLAIN, "$n checks various gauges and displays on the control panel.", ch, NULL, argument, TO_ROOM );

   ch_printf( ch, "&w[&W %s &w]   &zShip Condition: %s\n\r\n\r", target->name,
              target->shipstate == SHIP_DISABLED ? "&RDisabled" : "&GRunning" );
   ch_printf( ch, "&zCurrent Coordinates:&w %.0f %.0f %.0f   &zCurrent Heading:&w %.0f %.0f %.0f\n\r",
              target->vx, target->vy, target->vz, target->hx, target->hy, target->hz );
   ch_printf( ch, "&zSpeed:&w %d&W/%d   &zHyperdrive: &wClass %d   %s %s\n\r",
              target->currspeed, target->realspeed, target->hyperspeed,
              ship->class >= SHIP_CRUISER ? "&zBay doors:" : "",
              ( ship->class >= SHIP_CRUISER && ship->bayopen == TRUE ) ? "&wOpen" :
              ( ship->class >= SHIP_CRUISER && ship->bayopen == FALSE ) ? "&wClosed" : "" );

   ch_printf( ch, "&zHull:&w %d&W/%d   &zShields:&w %d&W/%d   &zFuel:&w %d&W/%d\n\r\n\r",
              target->hull, target->maxhull, target->shield, target->maxshield, target->energy, target->maxenergy );

   ch_printf( ch, "&zPrimary Weapon System:   &w%-7s&W::  &w%s\n\r",
              target->primaryState == LASER_DAMAGED ? "&ROffline" : "&GOnline", primary_beam_name( target ) );

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

   ch_printf( ch, "&z   Linked fire: &w%s\n\r",
              ( x <= 1 ) ? "Unavailable" : ship->primaryLinked == TRUE ? "&GON" : "&ROFF" );

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
   if( target->secondaryCount != 0 )
   {
      ch_printf( ch, "&zSecondary Weapon System: &w%-7s&W::  &w%s\n\r",
                 target->secondaryState == LASER_DAMAGED ? "&ROffline" : "&GOnline", secondary_beam_name( target ) );
      ch_printf( ch, "&z   Linked fire: &w%s\n\r",
                 ( x <= 1 ) ? "Unavailable" : ship->secondaryLinked == TRUE ? "&GON" : "&ROFF" );
   }
   if( target->turret1 )
      ch_printf( ch, "\n\r&zTurret One:  &w %s  &zCurrent Target:&w %s\n\r",
                 target->statet1 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target1 ? target->target1->name : "none" );
   if( target->turret2 )
      ch_printf( ch, "&zTurret Two:  &w %s  &zCurrent Target:&w %s\n\r",
                 target->statet2 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target2 ? target->target2->name : "none" );
   if( target->turret3 )
      ch_printf( ch, "&zTurret Three:&w %s  &zCurrent Target:&w %s\n\r",
                 target->statet3 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target3 ? target->target3->name : "none" );
   if( target->turret4 )
      ch_printf( ch, "&zTurret Four: &w %s  &zCurrent Target:&w %s\n\r",
                 target->statet4 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target4 ? target->target4->name : "none" );
   if( target->turret5 )
      ch_printf( ch, "&zTurret Five: &w %s  &zCurrent Target:&w %s\n\r",
                 target->statet5 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target5 ? target->target5->name : "none" );
   if( target->turret6 )
      ch_printf( ch, "&zTurret Six:  &w %s  &zCurrent Target:&w %s\n\r",
                 target->statet6 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target6 ? target->target6->name : "none" );
   if( target->turret7 )
      ch_printf( ch, "&zTurret Seven:&w %s  &zCurrent Target:&w %s\n\r",
                 target->statet7 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target7 ? target->target7->name : "none" );
   if( target->turret8 )
      ch_printf( ch, "&zTurret Eight:&w %s  &zCurrent Target:&w %s\n\r",
                 target->statet8 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target8 ? target->target8->name : "none" );
   if( target->turret9 )
      ch_printf( ch, "&zTurret Nine: &w %s  &zCurrent Target:&w %s\n\r",
                 target->statet9 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target9 ? target->target9->name : "none" );
   if( target->turret10 )
      ch_printf( ch, "&zTurret Ten:  &w %s  &zCurrent Target:&w %s\n\r",
                 target->statet10 == LASER_DAMAGED ? "Damaged" : "Operational",
                 target->target10 ? target->target10->name : "none" );


   ch_printf( ch, "\n\r&w          | &zMissile launcher:&w %-15s   &zPayload: &w%d&W/%d\n\r",
              !ship->maxmissiles ? "&RNot installed" :
              ship->missilestate == LASER_DAMAGED ? "&ROffline" : "&GOnline", ship->missiles, ship->maxmissiles );
   ch_printf( ch, " %-10s&w | &zTorpedo launcher:&w %-15s   &zPayload: &w%d&W/%d\n\r",
              ship->warheadLinked == TRUE ? "&GLinked" : "&RUnlinked",
              !ship->maxtorpedos ? "&RNot installed" :
              ship->torpedostate == LASER_DAMAGED ? "&ROffline" : "&GOnline", ship->torpedos, ship->maxtorpedos );
   ch_printf( ch, "          &w| &zRocket launcher: &w %-15s   &zPayload: &w%d&W/%d\n\r",
              !ship->maxrockets ? "&RNot installed" :
              ship->rocketstate == LASER_DAMAGED ? "&ROffline" : "&GOnline", ship->rockets, ship->maxrockets );
   if( ship->maxbombs > 0 )
      ch_printf( ch, "&zBomb Payload: &w%d&W/%d\n\r", ship->bombs, ship->maxbombs );

   learn_from_success( ch, gsn_shipsystems );
}

void do_hyperspace( CHAR_DATA * ch, char *argument )
{
   int schance;
   SHIP_DATA *ship;
   SHIP_DATA *eShip;
   char buf[MAX_STRING_LENGTH];

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


   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou aren't in the pilots seat.\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first.\n\r", ch );
      return;
   }


   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RSpace Stations can't move!\n\r", ch );
      return;
   }
   if( ship->hyperspeed == 0 )
   {
      send_to_char( "&RThis ship is not equipped with a hyperdrive!\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou are already travelling lightspeed!\n\r", ch );
      return;
   }
   if( ship->hyperstate == LASER_DAMAGED )
   {
      send_to_char( "&RThe ships hyperdrive is disabled. Unable to jump to lightspeed.\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RThe ships drive is disabled. Unable to manuever.\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RYou can't do that until after you've launched!\n\r", ch );
      return;
   }
   if( ship->shipstate != SHIP_READY )
   {
      send_to_char( "&RPlease wait until the ship has finished its current maneuver.\n\r", ch );
      return;
   }
   if( !ship->currjump )
   {
      send_to_char( "&RYou need to calculate your jump first!\n\r", ch );
      return;
   }

   if( ship->energy < ( ( 66 * ship->hyperspeed ) + ( ( 1 + ship->hyperdistance ) / 6 ) ) )
   {
      send_to_char( "&RTheres not enough fuel!\n\r", ch );
      return;
   }

   if( ship->currspeed <= 0 )
   {
      send_to_char( "&RYou need to speed up a little first!\n\r", ch );
      return;
   }

   for( eShip = ship->starsystem->first_ship; eShip; eShip = eShip->next_in_starsystem )
   {
      if( eShip == ship )
         continue;

      if( abs( eShip->vx - ship->vx ) < 500
          && abs( eShip->vy - ship->vy ) < 500 && abs( eShip->vz - ship->vz ) < 500 && eShip->class > SHIP_CRUISER )
      {
         ch_printf( ch, "&RYou are too close to %s to make the jump to lightspeed.\n\r", eShip->name );
         return;
      }
   }

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_smallspace] );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_mediumspace] );
   if( ship->class >= SHIP_CRUISER )
      schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_largespace] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou can't figure out which lever to use.\n\r", ch );
      if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
         learn_from_failure( ch, gsn_smallspace );
      if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
         learn_from_failure( ch, gsn_mediumspace );
      if( ship->class >= SHIP_CRUISER )
         learn_from_failure( ch, gsn_largespace );
      return;
   }
   sprintf( buf, "%s disappears from your scanner.", ship->name );
   echo_to_system( AT_YELLOW, ship, buf, NULL );

   ship_from_starsystem( ship, ship->starsystem );
   ship->shipstate = SHIP_HYPERSPACE;

   send_to_char( "&GYou push forward the hyperspeed lever.\n\r", ch );
   act( AT_PLAIN, "$n pushes a lever forward on the control panel.", ch, NULL, argument, TO_ROOM );
   echo_to_ship( AT_YELLOW, ship, "The ship lurches slightly as it makes the jump to lightspeed." );
   echo_to_cockpit( AT_YELLOW, ship, "The stars become streaks of light as you enter hyperspace." );

   ship->energy -= ( ( 66 * ship->hyperspeed ) + ( ( 1 + ship->hyperdistance ) / 6 ) );

   ship->vx = ship->jx;
   ship->vy = ship->jy;
   ship->vz = ship->jz;

   if( ship->class >= SHIP_FIGHTER && ship->class <= SHIP_SHUTTLE )
      learn_from_success( ch, gsn_smallspace );
   if( ship->class >= SHIP_FREIGHTER && ship->class <= SHIP_CORVETTE )
      learn_from_success( ch, gsn_mediumspace );
   if( ship->class >= SHIP_CRUISER )
      learn_from_success( ch, gsn_largespace );

}

void do_target( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance;
   SHIP_DATA *ship;
   SHIP_DATA *target;
   char buf[MAX_STRING_LENGTH];

   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
         {
            send_to_char( "&RYou must be in the gunners seat or turret of a ship to do that!\n\r", ch );
            return;
         }

         if( ship->class > SHIP_SPACE_STATION )
         {
            send_to_char( "&RThis isn't a spacecraft!\n\r", ch );
            return;
         }

         if( ship->shipstate == SHIP_HYPERSPACE )
         {
            send_to_char( "&RYou can only do that in realspace!\n\r", ch );
            return;
         }
         if( !ship->starsystem )
         {
            send_to_char( "&RYou can't do that until you've finished launching!\n\r", ch );
            return;
         }

         if( autofly( ship ) )
         {
            send_to_char( "&RYou'll have to turn off the ships autopilot first....\n\r", ch );
            return;
         }

         if( arg[0] == '\0' )
         {
            send_to_char( "&RYou need to specify a target!\n\r", ch );
            return;
         }

         if( !str_cmp( arg, "none" ) )
         {
            send_to_char( "&GTarget set to none.\n\r", ch );
            if( ch->in_room->vnum == ship->gunseat )
               ship->target0 = NULL;
            if( ch->in_room->vnum == ship->turret1 )
               ship->target1 = NULL;
            if( ch->in_room->vnum == ship->turret2 )
               ship->target2 = NULL;
            if( ch->in_room->vnum == ship->turret3 )
               ship->target3 = NULL;
            if( ch->in_room->vnum == ship->turret4 )
               ship->target4 = NULL;
            if( ch->in_room->vnum == ship->turret5 )
               ship->target5 = NULL;
            if( ch->in_room->vnum == ship->turret6 )
               ship->target6 = NULL;
            if( ch->in_room->vnum == ship->turret7 )
               ship->target7 = NULL;
            if( ch->in_room->vnum == ship->turret8 )
               ship->target8 = NULL;
            if( ch->in_room->vnum == ship->turret9 )
               ship->target9 = NULL;
            if( ch->in_room->vnum == ship->turret10 )
               ship->target10 = NULL;
            return;
         }

         target = get_ship_here( arg, ship->starsystem );
         if( target == NULL )
         {
            send_to_char( "&RThat ship isn't here!\n\r", ch );
            return;
         }

         if( target == ship )
         {
            send_to_char( "&RYou can't target your own ship!\n\r", ch );
            return;
         }

         if( !str_cmp( target->owner, ship->owner ) && str_cmp( target->owner, "Public" ) )
         {
            send_to_char( "&RThat ship has the same owner... try targetting an enemy ship instead!\n\r", ch );
            return;
         }

         if( abs( ship->vx - target->vx ) > 5000 ||
             abs( ship->vy - target->vy ) > 5000 || abs( ship->vz - target->vz ) > 5000 )
         {
            send_to_char( "&RThat ship is too far away to target.\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_weaponsystems] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GTracking target.\n\r", ch );
            act( AT_PLAIN, "$n makes some adjustments on the targeting computer.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 1, do_target, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou fail to work the controls properly.\n\r", ch );
         learn_from_failure( ch, gsn_weaponsystems );
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
         if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
            return;
         send_to_char( "&RYour concentration is broken. You fail to lock onto your target.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
   {
      return;
   }

   target = get_ship_here( arg, ship->starsystem );
   if( target == NULL || target == ship )
   {
      send_to_char( "&RThe ship has left the starsystem. Targetting aborted.\n\r", ch );
      return;
   }

   if( ch->in_room->vnum == ship->gunseat )
      ship->target0 = target;

   if( ch->in_room->vnum == ship->turret1 )
      ship->target1 = target;

   if( ch->in_room->vnum == ship->turret2 )
      ship->target2 = target;
   if( ch->in_room->vnum == ship->turret3 )
      ship->target3 = target;

   if( ch->in_room->vnum == ship->turret4 )
      ship->target4 = target;

   if( ch->in_room->vnum == ship->turret5 )
      ship->target5 = target;

   if( ch->in_room->vnum == ship->turret6 )
      ship->target6 = target;

   if( ch->in_room->vnum == ship->turret7 )
      ship->target7 = target;

   if( ch->in_room->vnum == ship->turret8 )
      ship->target8 = target;

   if( ch->in_room->vnum == ship->turret9 )
      ship->target9 = target;

   if( ch->in_room->vnum == ship->turret10 )
      ship->target10 = target;

   send_to_char( "&GTarget Locked.\n\r", ch );
   sprintf( buf, "You are being targetted by %s.", ship->name );
   echo_to_cockpit( AT_BLOOD, target, buf );

   sound_to_room( ch->in_room, "!!SOUND(targetlock)" );
   learn_from_success( ch, gsn_weaponsystems );

   if( autofly( target ) && !target->target0 )
   {
      sprintf( buf, "You are being targetted by %s.", target->name );
      echo_to_cockpit( AT_BLOOD, ship, buf );
      target->target0 = ship;
   }
}

/* Fire update by ||, primary/secondary usage, autoblasters, repeating ions... */

void do_fire( CHAR_DATA * ch, char *argument )
{
   int schance;
   int range = 1500;
   int iTurret;
   int sdamage;
   int p, times, size;
   char arg[MAX_STRING_LENGTH];
   SHIP_DATA *ship;
   SHIP_DATA *target;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];

   obj = get_eq_char( ch, WEAR_WIELD );
   if( obj )
   {
      if( obj->item_type == ITEM_RLAUNCHER || obj->item_type == ITEM_GLAUNCHER )
      {
         do_launch2( ch, argument );
         return;
      }
   }
   if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the gunners chair or turret of a ship to do that!\n\r", ch );
      return;
   }

   if( ship->class > SHIP_SPACE_STATION )
   {
      send_to_char( "&RThis isn't a spacecraft!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }
   if( ship->starsystem == NULL )
   {
      send_to_char( "&RYou can't do that until after you've finished launching!\n\r", ch );
      return;
   }
   if( ship->energy < 5 )
   {
      send_to_char( "&RTheres not enough energy left to fire!\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first.\n\r", ch );
      return;
   }


   schance = IS_NPC( ch ) ? ch->top_level
      : ( int )( ch->perm_dex * 2 + ch->pcdata->learned[gsn_spacecombat] / 3
                 + ch->pcdata->learned[gsn_spacecombat2] / 3 + ch->pcdata->learned[gsn_spacecombat3] / 3 );

   if( ch->in_room->vnum == ship->gunseat && !str_prefix( arg, "primary" ) )
   {
      if( ship->primaryType == B_NONE )
      {
         send_to_char( "&RThis ship doesn't have a primary weapon system.\n\r", ch );
         return;
      }
      if( ship->primaryState == LASER_DAMAGED )
      {
         send_to_char( "&RThe ship's primary weapon system is damaged.\n\r", ch );
         return;
      }
      if( ship->primaryLinked == TRUE )
      {
         switch ( ship->primaryType )
         {
            case 0:
               p = 0;
               break;
            case 1:
               p = ship->primaryCount;
               break;
            case 2:
               p = 2 * ship->primaryCount;
               break;
            case 3:
               p = 3 * ship->primaryCount;
               break;
            case 4:
               p = 4 * ship->primaryCount;
               break;
            case 5:
               p = 3 * ship->primaryCount;
               break;
            case 6:
               p = ship->primaryCount;
               break;
            case 7:
               p = ship->primaryCount;
               break;
            case 8:
               p = 3 * ship->primaryCount;
               break;
            case 9:
               p = ship->primaryCount;
               break;
            default:
               p = 1;
               break;
         }
      }
      else
      {
         if( ship->primaryType == AUTOBLASTER )
            p = 3;
         else
            p = 1;
      }

      if( ship->primaryState >= p && ship->primaryType != AUTOBLASTER )
      {
         send_to_char( "&RThe primary weapon systems are still recharging.\n\r", ch );
         return;
      }

      if( ship->primaryType == AUTOBLASTER && ship->primaryState >= 12 )
      {
         send_to_char( "&RYou need to let the autoblaster cool down.\n\r", ch );
         return;
      }
      if( ship->target0 == NULL )
      {
         send_to_char( "&RYou need to choose a target first.\n\r", ch );
         return;
      }
      target = ship->target0;
      if( ship->target0->starsystem != ship->starsystem )
      {
         send_to_char( "&RYour target seems to have left.\n\r", ch );
         ship->target0 = NULL;
         return;
      }
      if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
      {
         send_to_char( "&RThat ship is out of range.\n\r", ch );
         return;
      }
      if( ship->class < SHIP_CRUISER && !is_facing( ship, target ) )
      {
         send_to_char( "&RYou can only fire in a forward direction. You'll need to turn your ship!\n\r", ch );
         return;
      }
      ship->primaryState++;

      switch ( target->class )
      {
         case 0:
            size = 10;
            break;
         case 1:
            size = 10;
            break;
         case 2:
            size = 10;
            break;
         case 3:
            size = 12;
            break;
         case 4:
            size = 18;
            break;
         case 5:
            size = 20;
            break;
         case 6:
            size = 25;
            break;
         case 7:
            size = 40;
            break;
         case 8:
            size = 55;
            break;
         case 9:
            size = 65;
            break;
         case 10:
            size = 70;
            break;
         default:
            size = 70;
            break;
      }

      schance = 100 - ( ( target->manuever - ship->manuever ) + 65 );
      schance -= ( abs( target->vx - ship->vx ) / 100 );
      schance -= ( abs( target->vy - ship->vy ) / 100 );
      schance -= ( abs( target->vz - ship->vz ) / 100 );
      schance += size - 10;
      if( target->rolling == FALSE )
         schance = URANGE( 5, schance, 95 );
      else
         schance = number_range( 5, 8 );

      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      if( number_percent(  ) > schance )
      {
         if( ship->primaryType == HEAVY_LASER )
         {
            sprintf( buf, "&YHeavy lasers&O fire from %s at you but miss.", ship->name );
            echo_to_cockpit( AT_ORANGE, target, buf );
            sprintf( buf, "The ship's &Yheavy lasers&O fire at %s but miss.", target->name );
            echo_to_cockpit( AT_ORANGE, ship, buf );
            sprintf( buf, "&YHeavy laserfire&O from %s barely misses %s.", ship->name, target->name );
            echo_to_system( AT_ORANGE, ship, buf, target );
         }
         else if( ship->primaryType == AUTOBLASTER )
         {
            sprintf( buf, "&YAutoblaster fire&O from %s comes at you but misses.", ship->name );
            echo_to_cockpit( AT_ORANGE, target, buf );
            sprintf( buf, "The &Yautoblaster&O fires at %s but misses.", target->name );
            echo_to_cockpit( AT_ORANGE, ship, buf );
            sprintf( buf, "&YAutoblaster fire&O from %s barely misses %s.", ship->name, target->name );
            echo_to_system( AT_ORANGE, ship, buf, target );
         }
         else
         {
            sprintf( buf, "&YLasers&O fire from %s at you but miss.", ship->name );
            echo_to_cockpit( AT_ORANGE, target, buf );
            sprintf( buf, "The ships &Ylasers&O fire at %s but miss.", target->name );
            echo_to_cockpit( AT_ORANGE, ship, buf );
            sprintf( buf, "&YLaserfire&O from %s barely misses %s.", ship->name, target->name );
            echo_to_system( AT_ORANGE, ship, buf, target );
         }
         learn_from_failure( ch, gsn_spacecombat );
         learn_from_failure( ch, gsn_spacecombat2 );
         learn_from_failure( ch, gsn_spacecombat3 );

         if( p > 1 && str_cmp( argument, "noloop" ) ) // Infinite recursion == bad
            for( times = 1; times < p; times++ )
               do_fire( ch, "primary noloop" );

         return;
      }
      if( ship->primaryType == HEAVY_LASER )
      {
         sprintf( buf, "&YHeavy laserfire&O from %s hits %s.", ship->name, target->name );
         echo_to_system( AT_ORANGE, ship, buf, target );
         sprintf( buf, "You are hit by &Rheavy lasers&r from %s!", ship->name );
         echo_to_cockpit( AT_BLOOD, target, buf );
         sprintf( buf, "Your ships lasers hit %s!", target->name );
         echo_to_cockpit( AT_YELLOW, ship, buf );
      }
      else if( ship->primaryType == AUTOBLASTER )
      {
         sprintf( buf, "&YAutoblaster fire&O from %s hits %s.", ship->name, target->name );
         echo_to_system( AT_ORANGE, ship, buf, target );
         sprintf( buf, "You are hit by &Rautoblaster fire&r from %s!", ship->name );
         echo_to_cockpit( AT_BLOOD, target, buf );
         sprintf( buf, "Your ship's autoblaster hits %s!", target->name );
         echo_to_cockpit( AT_YELLOW, ship, buf );
      }
      else
      {
         sprintf( buf, "&YLaserfire&O from %s hits %s.", ship->name, target->name );
         echo_to_system( AT_ORANGE, ship, buf, target );
         sprintf( buf, "You are hit by &Rlasers&r from %s!", ship->name );
         echo_to_cockpit( AT_BLOOD, target, buf );
         sprintf( buf, "Your ships lasers hit %s!.", target->name );
         echo_to_cockpit( AT_YELLOW, ship, buf );
      }

      learn_from_success( ch, gsn_spacecombat );
      learn_from_success( ch, gsn_spacecombat2 );
      learn_from_success( ch, gsn_spacecombat3 );
      //echo_to_ship( AT_RED , target , "The ship shakes with vibration." );           

      if( ship->primaryType == HEAVY_LASER )
         damage_ship_ch( target, 15, 30, ch );
      else if( ship->primaryType == AUTOBLASTER )
         damage_ship_ch( target, 1, 2, ch );
      else
         damage_ship_ch( target, 3, 10, ch );

      if( autofly( target ) && target->target0 != ship )
      {
         target->target0 = ship;
         sprintf( buf, "You are being targetted by %s.", target->name );
         echo_to_cockpit( AT_BLOOD, ship, buf );
      }

      if( p > 1 && str_cmp( argument, "noloop" ) ) // Infinite recursion == bad
         for( times = 1; times < p; times++ )
            do_fire( ch, "primary noloop" );

      return;
   }  // End firing primary

   if( ch->in_room->vnum == ship->gunseat && !str_prefix( arg, "secondary" ) )
   {
      if( ship->secondaryType == B_NONE )
      {
         send_to_char( "&RThis ship doesn't have a secondary weapon system.\n\r", ch );
         return;
      }
      if( ship->secondaryState == LASER_DAMAGED )
      {
         send_to_char( "&RThe ship's secondary weapon systems are damaged.\n\r", ch );
         return;
      }
      if( ship->secondaryType == AUTOBLASTER && ship->secondaryState >= 15 )
      {
         send_to_char( "&RYou need to let the autoblaster cool down.\n\r", ch );
         return;
      }
      if( ship->secondaryType == REPEATING_ION && ship->secondaryState >= 15 )
      {
         send_to_char( "&RYou need to let the repeating ion cannon cool down.\n\r", ch );
         return;
      }
      if( ship->secondaryLinked == TRUE )
      {
         switch ( ship->secondaryType )
         {
            case 0:
               p = 0;
               break;
            case 5:
               p = 3 * ship->secondaryCount;
               break;
            case 7:
               p = 1 * ship->secondaryCount;
               break;
            case 8:
               p = 3 * ship->secondaryCount;
               break;
            case 9:
               p = 1 * ship->secondaryCount;
               break;
            default:
               p = 1;
               break;
         }
      }
      else
      {
         if( ship->secondaryType == AUTOBLASTER || ship->secondaryType == REPEATING_ION )
            p = 3;
         else
            p = 1;
      }
      if( ship->secondaryState >= ship->secondaryCount * p && ship->secondaryType != AUTOBLASTER
          && ship->secondaryType != REPEATING_ION )
      {
         send_to_char( "&RThe secondary weapon systems are still recharging.\n\r", ch );
         return;
      }

      if( ship->target0 == NULL )
      {
         send_to_char( "&RYou need to choose a target first.\n\r", ch );
         return;
      }
      target = ship->target0;
      if( ship->target0->starsystem != ship->starsystem )
      {
         send_to_char( "&RYour target seems to have left.\n\r", ch );
         ship->target0 = NULL;
         return;
      }
      if( abs( target->vx - ship->vx ) > range ||
          abs( target->vy - ship->vy ) > range || abs( target->vz - ship->vz ) > range )
      {
         send_to_char( "&RThat ship is out of range.\n\r", ch );
         return;
      }
      if( ship->class < SHIP_CRUISER && !is_facing( ship, target ) )
      {
         send_to_char( "&RYou can only fire in a forward direction. You'll need to turn your ship!\n\r", ch );
         return;
      }
      ship->secondaryState++;
      switch ( target->class )
      {
         case 0:
            size = 10;
            break;
         case 1:
            size = 10;
            break;
         case 2:
            size = 10;
            break;
         case 3:
            size = 12;
            break;
         case 4:
            size = 18;
            break;
         case 5:
            size = 20;
            break;
         case 6:
            size = 25;
            break;
         case 7:
            size = 40;
            break;
         case 8:
            size = 55;
            break;
         case 9:
            size = 65;
            break;
         case 10:
            size = 70;
            break;
         default:
            size = 70;
            break;
      }
      schance = 100 - ( ( target->manuever - ship->manuever ) + 65 );
      schance -= ( abs( target->vx - ship->vx ) / 100 );
      schance -= ( abs( target->vy - ship->vy ) / 100 );
      schance -= ( abs( target->vz - ship->vz ) / 100 );
      schance += size - 10;
      schance = URANGE( 5, schance, 95 );
      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      if( number_percent(  ) > schance )
      {
         if( ship->secondaryType == AUTOBLASTER )
         {
            sprintf( buf, "&YAutoblaster fire&O from %s comes at you but misses.", ship->name );
            echo_to_cockpit( AT_ORANGE, target, buf );
            sprintf( buf, "The &Yautoblaster&O fires at %s but misses.", target->name );
            echo_to_cockpit( AT_ORANGE, ship, buf );
            sprintf( buf, "&YAutoblaster fire&O from %s barely misses %s.", ship->name, target->name );
            echo_to_system( AT_ORANGE, ship, buf, target );
         }
         else if( ship->secondaryType == LIGHT_ION )
         {
            sprintf( buf, "&CLight ionfire&c from %s speeds toward you but misses.", ship->name );
            echo_to_cockpit( AT_PLAIN, target, buf );
            sprintf( buf, "&cThe &Clight ionfire&c speeds toward %s but misses.", target->name );
            echo_to_cockpit( AT_PLAIN, ship, buf );
            sprintf( buf, "&CLight ionfire&c from %s barely misses %s.", ship->name, target->name );
            echo_to_system( AT_PLAIN, ship, buf, target );
         }
         else if( ship->secondaryType == REPEATING_ION )
         {
            sprintf( buf, "&CRepeating ionfire&c from %s speeds toward you but misses.", ship->name );
            echo_to_cockpit( AT_PLAIN, target, buf );
            sprintf( buf, "&cThe &Crepeating ionfire&c speeds toward %s but misses.", target->name );
            echo_to_cockpit( AT_PLAIN, ship, buf );
            sprintf( buf, "&CRepeating ionfire&c from %s barely misses %s.", ship->name, target->name );
            echo_to_system( AT_PLAIN, ship, buf, target );
         }
         else if( ship->secondaryType == HEAVY_ION )
         {
            sprintf( buf, "&CHeavy ionfire&c from %s comes at you but misses.", ship->name );
            echo_to_cockpit( AT_PLAIN, target, buf );
            sprintf( buf, "&cThe &Cheavy ionfire&c speeds toward %s but misses.", target->name );
            echo_to_cockpit( AT_PLAIN, ship, buf );
            sprintf( buf, "&CHeavy ionfire&c from %s barely misses %s.", ship->name, target->name );
            echo_to_system( AT_PLAIN, ship, buf, target );
         }
         learn_from_failure( ch, gsn_spacecombat );
         learn_from_failure( ch, gsn_spacecombat2 );
         learn_from_failure( ch, gsn_spacecombat3 );

         if( p > 1 && str_cmp( argument, "noloop" ) ) // Infinite recursion == bad
            for( times = 1; times < p; times++ )
               do_fire( ch, "secondary noloop" );
         return;
      }
      if( ship->secondaryType == LIGHT_ION )
      {
         sprintf( buf, "&cYou are hit with &Clight ionfire&c from %s!", ship->name );
         echo_to_cockpit( AT_PLAIN, target, buf );
         sprintf( buf, "&cThe &Clight ionfire&c hits %s!", target->name );
         echo_to_cockpit( AT_PLAIN, ship, buf );
         sprintf( buf, "&CLight ionfire&c from %s hits %s.", ship->name, target->name );
         echo_to_system( AT_PLAIN, ship, buf, target );
      }
      else if( ship->secondaryType == REPEATING_ION )
      {
         sprintf( buf, "&cYou are hit with &Crepeating ionfire&c from %s!", ship->name );
         echo_to_cockpit( AT_PLAIN, target, buf );
         sprintf( buf, "&cThe &Crepeating ionfire&c hits %s!", target->name );
         echo_to_cockpit( AT_PLAIN, ship, buf );
         sprintf( buf, "&CRepeating ionfire&c from %s hits %s.", ship->name, target->name );
         echo_to_system( AT_PLAIN, ship, buf, target );
      }
      else if( ship->secondaryType == HEAVY_ION )
      {
         sprintf( buf, "&cYou are hit with &Cheavy ionfire&c from %s!", ship->name );
         echo_to_cockpit( AT_PLAIN, target, buf );
         sprintf( buf, "&cThe &Cheavy ionfire&c hits %s!", target->name );
         echo_to_cockpit( AT_PLAIN, target, buf );
         sprintf( buf, "&CHeavy ionfire&c from %s hits %s.", ship->name, target->name );
         echo_to_system( AT_PLAIN, ship, buf, target );
      }
      else if( ship->secondaryType == AUTOBLASTER )
      {
         sprintf( buf, "&YAutoblaster fire&O from %s hits %s.", ship->name, target->name );
         echo_to_system( AT_ORANGE, ship, buf, target );
         sprintf( buf, "You are hit by &Rautoblaster fire&r from %s!", ship->name );
         echo_to_cockpit( AT_BLOOD, target, buf );
         sprintf( buf, "Your ship's autoblaster hits %s!", target->name );
         echo_to_cockpit( AT_YELLOW, ship, buf );
      }


      learn_from_success( ch, gsn_spacecombat );
      learn_from_success( ch, gsn_spacecombat2 );
      learn_from_success( ch, gsn_spacecombat3 );

      //echo_to_ship( AT_RED , target , "&RThe ship vibrates and the lights dim momentarily." );

      if( ship->secondaryType == LIGHT_ION )
         damage_ship_ch_ion( target, 10, 15, ch );
      else if( ship->secondaryType == REPEATING_ION )
         damage_ship_ch_ion( target, 2, 4, ch );
      else if( ship->secondaryType == HEAVY_ION )
         damage_ship_ch_ion( target, 15, 25, ch );
      else
         damage_ship_ch( target, 1, 4, ch );

      if( autofly( target ) && target->target0 != ship )
      {
         target->target0 = ship;
         sprintf( buf, "You are being targetted by %s.", target->name );
         echo_to_cockpit( AT_BLOOD, ship, buf );
      }

      if( p > 1 && str_cmp( argument, "noloop" ) ) // Infinite recursion == bad
         for( times = 1; times < p; times++ )
            do_fire( ch, "secondary noloop" );
      return;
   }  // End secondary fire

   if( ch->in_room->vnum == ship->gunseat && !str_prefix( arg, "missile" ) )
   {
      if( ship->missilestate == MISSILE_DAMAGED )
      {
         send_to_char( "&RThe ships missile launchers are damaged.\n\r", ch );
         return;
      }
      if( ship->missiles <= 0 )
      {
         send_to_char( "&RYou have no missiles to fire!\n\r", ch );
         return;
      }
      if( ship->missilestate != MISSILE_READY )
      {
         send_to_char( "&RThe missiles are still reloading.\n\r", ch );
         return;
      }
      if( ship->target0 == NULL )
      {
         send_to_char( "&RYou need to choose a target first.\n\r", ch );
         return;
      }
      target = ship->target0;
      if( ship->target0->starsystem != ship->starsystem )
      {
         send_to_char( "&RYour target seems to have left.\n\r", ch );
         ship->target0 = NULL;
         return;
      }
      if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
      {
         send_to_char( "&RThat ship is out of missile range.\n\r", ch );
         return;
      }
      if( ship->class < SHIP_CRUISER && !is_facing( ship, target ) )
      {
         send_to_char( "&RMissiles can only fire forward. You'll need to turn your ship!\n\r", ch );
         return;
      }
      schance = 100 - ( ( target->manuever - ship->manuever ) + 50 );
      schance -= ( abs( target->vx - ship->vx ) / 100 );
      schance -= ( abs( target->vy - ship->vy ) / 100 );
      schance -= ( abs( target->vz - ship->vz ) / 100 );
      //Extra chance for missiles due to computer compensation.
      schance += 20;
      schance = URANGE( 5, schance, 95 );
      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      if( number_percent(  ) > schance )
      {
         send_to_char( "&RYou fail to lock onto your target, and the missile goes off on a tangent.\n\r", ch );
         ship->missilestate = MISSILE_RELOAD_2;
         return;
      }
      new_missile( ship, target, ch, CONCUSSION_MISSILE );
      ship->missiles--;
      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      echo_to_cockpit( AT_YELLOW, ship, "Missiles launched." );
      sprintf( buf, "Incoming missile from %s.", ship->name );
      echo_to_cockpit( AT_BLOOD, target, buf );
      sprintf( buf, "%s fires a missile towards %s.", ship->name, target->name );
      echo_to_system( AT_ORANGE, ship, buf, target );
      learn_from_success( ch, gsn_weaponsystems );
      if( ship->class >= SHIP_CRUISER )
         ship->missilestate = MISSILE_RELOAD;
      else
         ship->missilestate = MISSILE_FIRED;

      if( autofly( target ) && target->target0 != ship )
      {
         target->target0 = ship;
         sprintf( buf, "You are being targetted by %s.", target->name );
         echo_to_cockpit( AT_BLOOD, ship, buf );
      }

      if( ship->warheadLinked == TRUE && str_cmp( argument, "noloop" ) )
      {
         if( ship->maxtorpedos > 0 && ship->torpedos > 0 )
            do_fire( ch, "torpedo noloop" );
         if( ship->maxrockets > 0 && ship->rockets > 0 )
            do_fire( ch, "rocket noloop" );
      }
      return;
   }
   if( ch->in_room->vnum == ship->gunseat && !str_prefix( arg, "torpedo" ) )
   {
      if( ship->torpedostate == MISSILE_DAMAGED )
      {
         send_to_char( "&RThe ship's torpedo launchers are damaged.\n\r", ch );
         return;
      }
      if( ship->torpedos <= 0 )
      {
         send_to_char( "&RYou have no torpedos to fire!\n\r", ch );
         return;
      }
      if( ship->torpedostate != MISSILE_READY )
      {
         send_to_char( "&RThe torpedos are still reloading.\n\r", ch );
         return;
      }
      if( ship->target0 == NULL )
      {
         send_to_char( "&RYou need to choose a target first.\n\r", ch );
         return;
      }
      target = ship->target0;
      if( ship->target0->starsystem != ship->starsystem )
      {
         send_to_char( "&RYour target seems to have left.\n\r", ch );
         ship->target0 = NULL;
         return;
      }
      if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
      {
         send_to_char( "&RThat ship is out of torpedo range.\n\r", ch );
         return;
      }
      if( ship->class < SHIP_CRUISER && !is_facing( ship, target ) )
      {
         send_to_char( "&RTorpedos can only fire forward. You'll need to turn your ship!\n\r", ch );
         return;
      }
      schance = 100 - ( ( target->manuever - ship->manuever ) + 50 );
      schance -= ( abs( target->vx - ship->vx ) / 100 );
      schance -= ( abs( target->vy - ship->vy ) / 100 );
      schance -= ( abs( target->vz - ship->vz ) / 100 );
      //Extra chance for missiles due to computer compensation.
      schance += 20;
      schance = URANGE( 5, schance, 95 );
      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      if( number_percent(  ) > schance )
      {
         send_to_char( "&RYou fail to lock onto your target, and the torpedo goes off on a tangent.\n\r", ch );
         ship->torpedostate = MISSILE_RELOAD_2;
         return;
      }
      new_missile( ship, target, ch, PROTON_TORPEDO );
      ship->torpedos--;
      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      echo_to_cockpit( AT_YELLOW, ship, "Torpedo launched." );
      sprintf( buf, "Incoming torpedo from %s.", ship->name );
      echo_to_cockpit( AT_BLOOD, target, buf );
      sprintf( buf, "%s fires a torpedo towards %s.", ship->name, target->name );
      echo_to_system( AT_ORANGE, ship, buf, target );
      learn_from_success( ch, gsn_weaponsystems );
      if( ship->class >= SHIP_CRUISER )
         ship->torpedostate = MISSILE_RELOAD;
      else
         ship->torpedostate = MISSILE_FIRED;

      if( autofly( target ) && target->target0 != ship )
      {
         target->target0 = ship;
         sprintf( buf, "You are being targetted by %s.", target->name );
         echo_to_cockpit( AT_BLOOD, ship, buf );
      }

      if( ship->warheadLinked == TRUE && str_cmp( argument, "noloop" ) )
      {
         if( ship->maxmissiles > 0 && ship->missiles > 0 )
            do_fire( ch, "missile noloop" );
         if( ship->maxrockets > 0 && ship->rockets > 0 )
            do_fire( ch, "rocket noloop" );
      }
      return;
   }

   if( ch->in_room->vnum == ship->gunseat && !str_prefix( arg, "rocket" ) )
   {
      if( ship->rocketstate == MISSILE_DAMAGED )
      {
         send_to_char( "&RThe ship's rocket launchers are damaged.\n\r", ch );
         return;
      }
      if( ship->rockets <= 0 )
      {
         send_to_char( "&RYou have no rockets to fire!\n\r", ch );
         return;
      }
      if( ship->rocketstate != MISSILE_READY )
      {
         send_to_char( "&RThe rockets are still reloading.\n\r", ch );
         return;
      }
      if( ship->target0 == NULL )
      {
         send_to_char( "&RYou need to choose a target first.\n\r", ch );
         return;
      }
      target = ship->target0;
      if( ship->target0->starsystem != ship->starsystem )
      {
         send_to_char( "&RYour target seems to have left.\n\r", ch );
         ship->target0 = NULL;
         return;
      }
      if( abs( target->vx - ship->vx ) > 800 || abs( target->vy - ship->vy ) > 800 || abs( target->vz - ship->vz ) > 800 )
      {
         send_to_char( "&RThat ship is out of rocket range.\n\r", ch );
         return;
      }
      if( ship->class < SHIP_CRUISER && !is_facing( ship, target ) )
      {
         send_to_char( "&RRockets can only fire forward. You'll need to turn your ship!\n\r", ch );
         return;
      }
      schance = 100 - ( ( target->manuever - ship->manuever ) + 50 );
      schance -= ( abs( target->vx - ship->vx ) / 100 );
      schance -= ( abs( target->vy - ship->vy ) / 100 );
      schance -= ( abs( target->vz - ship->vz ) / 100 );
      //Extra chance for missiles due to computer compensation.
      schance += 20;
      schance = URANGE( 5, schance, 95 );
      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      if( number_percent(  ) > schance )
      {
         send_to_char( "&RYou fail to lock onto your target, and the rocket goes off on a tangent.\n\r", ch );
         ship->rocketstate = MISSILE_RELOAD_2;
         return;
      }
      new_missile( ship, target, ch, HEAVY_ROCKET );
      ship->rockets--;
      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
      echo_to_cockpit( AT_YELLOW, ship, "Rocket launched." );
      sprintf( buf, "Incoming rocket from %s.", ship->name );
      echo_to_cockpit( AT_BLOOD, target, buf );
      sprintf( buf, "%s fires a heavy rocket towards %s.", ship->name, target->name );
      echo_to_system( AT_ORANGE, ship, buf, target );
      learn_from_success( ch, gsn_weaponsystems );
      if( ship->class >= SHIP_CRUISER )
         ship->rocketstate = MISSILE_RELOAD;
      else
         ship->rocketstate = MISSILE_FIRED;

      if( autofly( target ) && target->target0 != ship )
      {
         target->target0 = ship;
         sprintf( buf, "You are being targetted by %s.", target->name );
         echo_to_cockpit( AT_BLOOD, ship, buf );
      }

      if( ship->warheadLinked == TRUE && str_cmp( argument, "noloop" ) )
      {
         if( ship->maxmissiles > 0 && ship->missiles > 0 )
            do_fire( ch, "missile noloop" );
         if( ship->maxtorpedos > 0 && ship->torpedos > 0 )
            do_fire( ch, "torpedo noloop" );
      }
      return;
   }


   /*
    * Turrets ....... Shit. A lot of bullshit :) 
    */

   if( ( ch->in_room->vnum == ship->turret1 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret2 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret3 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret4 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret5 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret5 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret6 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret7 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret8 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret9 && !str_prefix( argument, "lasers" ) ) ||
       ( ch->in_room->vnum == ship->turret10 && !str_prefix( argument, "lasers" ) ) )
   {

      if( ch->in_room->vnum == ship->turret1 )
         iTurret = 1;
      else if( ch->in_room->vnum == ship->turret2 )
         iTurret = 2;
      else if( ch->in_room->vnum == ship->turret3 )
         iTurret = 3;
      else if( ch->in_room->vnum == ship->turret4 )
         iTurret = 4;
      else if( ch->in_room->vnum == ship->turret5 )
         iTurret = 5;
      else if( ch->in_room->vnum == ship->turret6 )
         iTurret = 6;
      else if( ch->in_room->vnum == ship->turret7 )
         iTurret = 7;
      else if( ch->in_room->vnum == ship->turret8 )
         iTurret = 8;
      else if( ch->in_room->vnum == ship->turret9 )
         iTurret = 9;
      else if( ch->in_room->vnum == ship->turret10 )
         iTurret = 10;
      else
      {
         send_to_char( "Error: do_fire: turrets. Tell someone important.\n\r", ch );
         return;
      }

      switch ( iTurret )
      {
         case 1:
            if( ship->statet1 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet1 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target1 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target1;
            if( ship->target1->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target1 = NULL;
               return;
            }
            break;
         case 2:
            if( ship->statet2 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet2 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target2 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target2;
            if( ship->target2->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target2 = NULL;
               return;
            }
            break;
         case 3:
            if( ship->statet3 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet3 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target3 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target3;
            if( ship->target3->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target3 = NULL;
               return;
            }
            break;
         case 4:
            if( ship->statet4 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet4 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target4 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target4;
            if( ship->target4->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target4 = NULL;
               return;
            }
            break;
         case 5:
            if( ship->statet5 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet5 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target5 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target5;
            if( ship->target5->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target5 = NULL;
               return;
            }
            break;
         case 6:
            if( ship->statet6 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet6 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target6 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target6;
            if( ship->target6->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target6 = NULL;
               return;
            }
            break;
         case 7:
            if( ship->statet7 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet7 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target7 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target7;
            if( ship->target7->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target7 = NULL;
               return;
            }
            break;
         case 8:
            if( ship->statet8 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet8 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target8 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target8;
            if( ship->target8->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target8 = NULL;
               return;
            }
            break;
         case 9:
            if( ship->statet9 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet9 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target9 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target9;
            if( ship->target9->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target9 = NULL;
               return;
            }
            break;
         case 10:
            if( ship->statet10 == LASER_DAMAGED )
            {
               send_to_char( "&RThe ships turret is damaged.\n\r", ch );
               return;
            }
            if( ship->statet10 > ship->class )
            {
               send_to_char( "&RThe turbolaser is recharging.\n\r", ch );
               return;
            }
            if( ship->target10 == NULL )
            {
               send_to_char( "&RYou need to choose a target first.\n\r", ch );
               return;
            }
            target = ship->target10;
            if( ship->target10->starsystem != ship->starsystem )
            {
               send_to_char( "&RYour target seems to have left.\n\r", ch );
               ship->target10 = NULL;
               return;
            }
      }  /* end switch */

      if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
      {
         send_to_char( "&RThat ship is out of laser range.\n\r", ch );
         return;
      }

      switch ( iTurret )
      {
         case 1:
            ship->statet1++;
            break;
         case 2:
            ship->statet2++;
            break;
         case 3:
            ship->statet3++;
            break;
         case 4:
            ship->statet4++;
            break;
         case 5:
            ship->statet5++;
            break;
         case 6:
            ship->statet6++;
            break;
         case 7:
            ship->statet7++;
            break;
         case 8:
            ship->statet8++;
            break;
         case 9:
            ship->statet9++;
            break;
         case 10:
            ship->statet10++;
            break;
         default:
            send_to_char( "Something bad happened.\n\r", ch );
            return;
      }

      act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );

      schance = 100 - ( ( target->manuever - ship->manuever ) + 50 );
      schance -= ( abs( target->vx - ship->vx ) / 100 );
      schance -= ( abs( target->vy - ship->vy ) / 100 );
      schance -= ( abs( target->vz - ship->vz ) / 100 );
      schance = URANGE( 5, schance, 95 );

      if( number_percent(  ) > schance )
      {
         sprintf( buf, "Turbolasers fire from %s at you but miss.", ship->name );
         echo_to_cockpit( AT_ORANGE, target, buf );
         sprintf( buf, "Turbolasers fire from the ships turret at %s but miss.", target->name );
         echo_to_cockpit( AT_ORANGE, ship, buf );
         sprintf( buf, "%s fires at %s but misses.", ship->name, target->name );
         echo_to_system( AT_ORANGE, ship, buf, target );

         learn_from_failure( ch, gsn_spacecombat );
         learn_from_failure( ch, gsn_spacecombat2 );
         learn_from_failure( ch, gsn_spacecombat3 );

         return;
      }
      sdamage = number_range( 3, 18 );
      sprintf( buf, "Turbolasers fire from %s, hitting %s.", ship->name, target->name );
      echo_to_system( AT_ORANGE, ship, buf, target );
      if( sdamage > 0 )
         sprintf( buf, "You are hit by turbolasers from %s!", ship->name );
      else
         sprintf( buf, "You are hit by turbolasers from %s, but the shields hold.", ship->name );

      echo_to_cockpit( AT_BLOOD, target, buf );
      sprintf( buf, "Turbolasers fire from the turret, hitting %s!.", target->name );
      echo_to_cockpit( AT_YELLOW, ship, buf );

      learn_from_success( ch, gsn_spacecombat );
      learn_from_success( ch, gsn_spacecombat2 );
      learn_from_success( ch, gsn_spacecombat3 );


      echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
      damage_ship_ch( target, 2, sdamage, ch );

      if( autofly( target ) && target->target0 != ship )
      {
         target->target0 = ship;
         sprintf( buf, "You are being targetted by %s.", target->name );
         echo_to_cockpit( AT_BLOOD, ship, buf );
      }

      return;
   }

   send_to_char( "&RYou can't fire that!\n\r", ch );

}

void do_calculate( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   int schance, count;
   SHIP_DATA *ship;
   SPACE_DATA *starsystem;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

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

   if( ( ship = ship_from_navseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be at a nav computer to calculate jumps.\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first....\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RAnd what exactly are you going to calculate...?\n\r", ch );
      return;
   }
   if( ship->hyperspeed == 0 )
   {
      send_to_char( "&RThis ship is not equipped with a hyperdrive!\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RYou can't do that until after you've launched!\n\r", ch );
      return;
   }
   if( ship->starsystem == NULL )
   {
      send_to_char( "&RYou can only do that in realspace.\n\r", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      if( IS_SET( ship->flags, SHIP_SIMULATOR ) )
      {
         send_to_char( "&WFormat: Calculate simulator <x> <y> <z>\n\r", ch );
         return;
      }
      send_to_char( "&WFormat: Calculate <starsystem> <entry x> <entry y> <entry z>\n\r&wPossible destinations:\n\r", ch );
      for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
      {
         if( !str_cmp( starsystem->name, "Simulator" ) )
            continue;
         set_char_color( AT_NOTE, ch );
         ch_printf( ch, "%-30s %d\n\r", starsystem->name,
                    ( abs( starsystem->xpos - ship->starsystem->xpos ) +
                      abs( starsystem->ypos - ship->starsystem->ypos ) ) / 2 );
         count++;
      }
      if( !count )
      {
         send_to_char( "No Starsystems found.\n\r", ch );
      }
      return;
   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_navigation] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou cant seem to figure the charts out today.\n\r", ch );
      learn_from_failure( ch, gsn_navigation );
      return;
   }


   ship->currjump = starsystem_from_name( arg1 );
   ship->jx = atoi( arg2 );
   ship->jy = atoi( arg3 );
   ship->jz = atoi( argument );

   if( ship->currjump == NULL )
   {
      send_to_char( "&RYou can't seem to find that starsytsem on your charts.\n\r", ch );
      return;
   }
   else if( IS_SET( ship->flags, SHIP_SIMULATOR ) && ship->currjump != starsystem_from_name( "Simulator" ) )
   {
      send_to_char( "&RYou can't seem to find that starsytsem on your charts.\n\r", ch );
      ship->currjump = NULL;
      return;
   }
   else if( !IS_SET( ship->flags, SHIP_SIMULATOR ) && ship->currjump == starsystem_from_name( "Simulator" ) )
   {
      send_to_char( "&RYou can't seem to find that starsytsem on your charts.\n\r", ch );
      ship->currjump = NULL;
      return;
   }
   else
   {
      SPACE_DATA *starsystem2;

      starsystem2 = ship->currjump;

      if( starsystem2->star1 && strcmp( starsystem2->star1, "" ) &&
          abs( ship->jx - starsystem2->s1x ) < 300 &&
          abs( ship->jy - starsystem2->s1y ) < 300 && abs( ship->jz - starsystem2->s1z ) < 300 )
      {
         echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
         echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
         ship->currjump = NULL;
         return;
      }
      else if( starsystem2->star2 && strcmp( starsystem2->star2, "" ) &&
               abs( ship->jx - starsystem2->s2x ) < 300 &&
               abs( ship->jy - starsystem2->s2y ) < 300 && abs( ship->jz - starsystem2->s2z ) < 300 )
      {
         echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
         echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
         ship->currjump = NULL;
         return;
      }
      else if( starsystem2->planet1 && strcmp( starsystem2->planet1, "" ) &&
               abs( ship->jx - starsystem2->p1x ) < 300 &&
               abs( ship->jy - starsystem2->p1y ) < 300 && abs( ship->jz - starsystem2->p1z ) < 300 )
      {
         echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
         echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
         ship->currjump = NULL;
         return;
      }
      else if( starsystem2->planet2 && strcmp( starsystem2->planet2, "" ) &&
               abs( ship->jx - starsystem2->p2x ) < 300 &&
               abs( ship->jy - starsystem2->p2y ) < 300 && abs( ship->jz - starsystem2->p2z ) < 300 )
      {
         echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
         echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
         ship->currjump = NULL;
         return;
      }
      else if( starsystem2->planet3 && strcmp( starsystem2->planet3, "" ) &&
               abs( ship->jx - starsystem2->p3x ) < 300 &&
               abs( ship->jy - starsystem2->p3y ) < 300 && abs( ship->jz - starsystem2->p3z ) < 300 )
      {
         echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
         echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
         ship->currjump = NULL;
         return;
      }
      else
      {
         ship->jx += number_range( -250, 250 );
         ship->jy += number_range( -250, 250 );
         ship->jz += number_range( -250, 250 );
      }
   }

   ship->hyperdistance = abs( ship->starsystem->xpos - ship->currjump->xpos );
   ship->hyperdistance += abs( ship->starsystem->ypos - ship->currjump->ypos );
   ship->hyperdistance /= 5;

   if( ship->hyperdistance < 100 )
      ship->hyperdistance = 100;

   ship->hyperdistance += number_range( 0, 200 );

   sound_to_room( ch->in_room, "!!SOUND(computer)" );

   send_to_char( "&GHyperspace course set. Ready for the jump to lightspeed.\n\r", ch );
   act( AT_PLAIN, "$n does some calculations using the ships computer.", ch, NULL, argument, TO_ROOM );

   learn_from_success( ch, gsn_navigation );

   WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
}

void do_recharge( CHAR_DATA * ch, char *argument )
{
   int recharge;
   int schance;
   SHIP_DATA *ship;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }
   if( ( ship = ship_from_coseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RThe controls must be at the co-pilot station.\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&R...\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RThe ships drive is disabled. Unable to manuever.\n\r", ch );
      return;
   }

   if( ship->energy < 100 )
   {
      send_to_char( "&RTheres not enough energy!\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_shipsystems] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou fail to work the controls properly.\n\r", ch );
      learn_from_failure( ch, gsn_shipsystems );
      return;
   }
   if( ship->shield < ship->maxshield )
   {
      send_to_char( "&GRecharging shields..\n\r", ch );
      act( AT_PLAIN, "$n pulls back a lever on the control panel.", ch, NULL, argument, TO_ROOM );

      learn_from_success( ch, gsn_shipsystems );

      recharge = UMIN( ship->maxshield - ship->shield, ship->energy * 5 + 100 );
      recharge = URANGE( 1, recharge, 25 + ship->class * 25 );
      ship->shield += recharge;
      ship->energy -= recharge * 3;
   }
   else
      send_to_char( "&GShip shields are at maximum.\n\r", ch );
}

void do_refuel( CHAR_DATA * ch, char *argument )
{
}

void do_addpilot( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RYou can't do that here.\n\r", ch );
      return;
   }

   if( str_cmp( ship->owner, ch->name ) )
   {

      if( !IS_NPC( ch ) && ch->pcdata && ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, ship->owner ) )
         if( !str_cmp( ch->pcdata->clan->leader, ch->name ) )
            ;
         else if( !str_cmp( ch->pcdata->clan->number1, ch->name ) )
            ;
         else if( !str_cmp( ch->pcdata->clan->number2, ch->name ) )
            ;
         else
         {
            send_to_char( "&RThat isn't your ship!", ch );
            return;
         }
      else
      {
         send_to_char( "&RThat isn't your ship!", ch );
         return;
      }

   }

   if( argument[0] == '\0' )
   {
      send_to_char( "&RAdd which pilot?\n\r", ch );
      return;
   }

   if( str_cmp( ship->pilot, "" ) )
   {
      if( str_cmp( ship->copilot, "" ) )
      {
         send_to_char( "&RYou are ready have a pilot and copilot..\n\r", ch );
         send_to_char( "&RTry rempilot first.\n\r", ch );
         return;
      }

      STRFREE( ship->copilot );
      ship->copilot = STRALLOC( argument );
      send_to_char( "Copilot Added.\n\r", ch );
      save_ship( ship );
      return;

      return;
   }

   STRFREE( ship->pilot );
   ship->pilot = STRALLOC( argument );
   send_to_char( "Pilot Added.\n\r", ch );
   save_ship( ship );

}

void do_rempilot( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }

   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RYou can't do that here.\n\r", ch );
      return;
   }

   if( str_cmp( ship->owner, ch->name ) )
   {

      if( !IS_NPC( ch ) && ch->pcdata && ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, ship->owner ) )
         if( !str_cmp( ch->pcdata->clan->leader, ch->name ) )
            ;
         else if( !str_cmp( ch->pcdata->clan->number1, ch->name ) )
            ;
         else if( !str_cmp( ch->pcdata->clan->number2, ch->name ) )
            ;
         else
         {
            send_to_char( "&RThat isn't your ship!", ch );
            return;
         }
      else
      {
         send_to_char( "&RThat isn't your ship!", ch );
         return;
      }

   }

   if( argument[0] == '\0' )
   {
      send_to_char( "&RRemove which pilot?\n\r", ch );
      return;
   }

   if( !str_cmp( ship->pilot, argument ) )
   {
      STRFREE( ship->pilot );
      ship->pilot = STRALLOC( "" );
      send_to_char( "Pilot Removed.\n\r", ch );
      save_ship( ship );
      return;
   }

   if( !str_cmp( ship->copilot, argument ) )
   {
      STRFREE( ship->copilot );
      ship->copilot = STRALLOC( "" );
      send_to_char( "Copilot Removed.\n\r", ch );
      save_ship( ship );
      return;
   }

   send_to_char( "&RThat person isn't listed as one of the ships pilots.\n\r", ch );

}

void do_radar( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *target;
   PLANET_DATA *planet;
   int schance;
   SHIP_DATA *ship;
   MISSILE_DATA *missile;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit or turret of a ship to do that!\n\r", ch );
      return;
   }

   if( ship->class > SHIP_SPACE_STATION )
   {
      send_to_char( "&RThis isn't a spacecraft!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RWait until after you launch!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }

   if( ship->starsystem == NULL )
   {
      send_to_char( "&RYou can't do that unless the ship is flying in realspace!\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_navigation] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou fail to work the controls properly.\n\r", ch );
      learn_from_failure( ch, gsn_navigation );
      return;
   }

   act( AT_PLAIN, "$n checks the radar.", ch, NULL, argument, TO_ROOM );

   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "&W%s Starsystem\n\r\n\r", ship->starsystem->name );
   set_char_color( AT_LBLUE, ch );
   if( ship->starsystem->star1 && str_cmp( ship->starsystem->star1, "" ) )
      ch_printf( ch, "&YThe star, %s   %d %d %d\n\r",
                 ship->starsystem->star1, ship->starsystem->s1x, ship->starsystem->s1y, ship->starsystem->s1z );
   if( ship->starsystem->star2 && str_cmp( ship->starsystem->star2, "" ) )
      ch_printf( ch, "&YThe star, %s   %d %d %d\n\r",
                 ship->starsystem->star2, ship->starsystem->s2x, ship->starsystem->s2y, ship->starsystem->s2z );

   if( ship->starsystem->planet1 && str_cmp( ship->starsystem->planet1, "" ) )
   {

      for( planet = first_planet; planet; planet = planet->next )
      {

         if( !str_cmp( planet->name, ship->starsystem->planet1 ) )
            break;
      }

      if( planet->controls == 0 )
         ch_printf( ch, "&GThe planet, %s   %d %d %d\n\r",
                    ship->starsystem->planet1, ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
      else
         ch_printf( ch, "&OA fairly large asteroid, designation %s    %d %d %d\n\r",
                    ship->starsystem->planet1, ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
   }

   if( ship->starsystem->planet2 && str_cmp( ship->starsystem->planet2, "" ) )
   {

      for( planet = first_planet; planet; planet = planet->next )
      {

         if( !str_cmp( planet->name, ship->starsystem->planet2 ) )
            break;
      }

      if( planet->controls == 0 )
         ch_printf( ch, "&GThe planet, %s   %d %d %d\n\r",
                    ship->starsystem->planet2, ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
      else
         ch_printf( ch, "&OA fairly large asteroid, designation %s    %d %d %d\n\r",
                    ship->starsystem->planet2, ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
   }
   if( ship->starsystem->planet3 && str_cmp( ship->starsystem->planet3, "" ) )
   {

      for( planet = first_planet; planet; planet = planet->next )
      {

         if( !str_cmp( planet->name, ship->starsystem->planet3 ) )
            break;
      }

      if( planet->controls == 0 )
         ch_printf( ch, "&GThe planet, %s   %d %d %d\n\r",
                    ship->starsystem->planet3, ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
      else
         ch_printf( ch, "&OA fairly large asteroid, designation %s    %d %d %d\n\r",
                    ship->starsystem->planet3, ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
   }
   ch_printf( ch, "\n\r" );
   for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
   {
      if( target != ship )
      {
         if( ship->class != 11 )
            ch_printf( ch, "&R&C%s    %.0f %.0f %.0f\n\r", target->name, target->vx, target->vy, target->vz );
         else
            ch_printf( ch, "&R&O%s     %.0f %.0f %.0f\n\r", target->name, target->vx, target->vy, target->vz );
      }
   }
   ch_printf( ch, "\n\r" );
   for( missile = ship->starsystem->first_missile; missile; missile = missile->next_in_starsystem )
   {
      ch_printf( ch, "&G&R%s    %d %d %d\n\r",
                 missile->missiletype == CONCUSSION_MISSILE ? "A Concussion Missile" :
                 ( missile->missiletype == PROTON_TORPEDO ? "A Torpedo" :
                   ( missile->missiletype == HEAVY_ROCKET ? "A Heavy Rocket" : "A Heavy Bomb" ) ),
                 missile->mx, missile->my, missile->mz );
   }

   ch_printf( ch, "\n\r&WYour coordinates: %.0f %.0f %.0f\n\r", ship->vx, ship->vy, ship->vz );


   learn_from_success( ch, gsn_navigation );

}

void do_autotrack( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int schance;

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


   if( ship->class == SHIP_SPACE_STATION )
   {
      send_to_char( "&RPlatforms don't have autotracking systems!\n\r", ch );
      return;
   }
   if( ship->class >= SHIP_CRUISER )
   {
      send_to_char( "&RThis ship is too big for autotracking!\n\r", ch );
      return;
   }

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou aren't in the pilots chair!\n\r", ch );
      return;
   }
   if( !ship->target0 )
   {
      send_to_char( "&RYou don't have a target to track.\n\r", ch );
      return;
   }
   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn off the ships autopilot first....\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_shipsystems] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou're not sure which switch to flip.\n\r", ch );
      learn_from_failure( ch, gsn_shipsystems );
      return;
   }

   act( AT_PLAIN, "$n flips a switch on the control panel.", ch, NULL, argument, TO_ROOM );
   if( ship->autotrack )
   {
      ship->autotrack = FALSE;
      echo_to_cockpit( AT_YELLOW, ship, "Autotracking off." );
   }
   else
   {
      ship->autotrack = TRUE;
      echo_to_cockpit( AT_YELLOW, ship, "Autotracking on." );
   }

   learn_from_success( ch, gsn_shipsystems );

}

void do_jumpvector( CHAR_DATA * ch, char *argument )
{
}
void do_reload( CHAR_DATA * ch, char *argument )
{
}
void do_closebay( CHAR_DATA * ch, char *argument )
{
}
void do_openbay( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be at the pilot controls of a ship to manipulate the bay doors.\n\r", ch );
      return;
   }

   if( ship->bayopen == FALSE )
   {
      echo_to_room( AT_WHITE, get_room_index( ship->pilotseat ), "&G[&gShip Computer&G] &wOpening bay doors." );
      sprintf( buf, "The bay doors on %s slowly slide open.", ship->name );
      echo_to_system( AT_YELLOW, ship, buf, NULL );
      if( ship->hanger1 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger1 ), "&R&YThe bay doors slowly slide open." );
      if( ship->hanger2 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger2 ), "&R&YThe bay doors slowly slide open." );
      if( ship->hanger3 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger3 ), "&R&YThe bay doors slowly slide open." );
      if( ship->hanger4 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger4 ), "&R&YThe bay doors slowly slide open." );

      ship->bayopen = TRUE;
      return;
   }
   else
   {
      echo_to_room( AT_WHITE, get_room_index( ship->pilotseat ), "&G[&gShip Computer&G] &wClosing bay doors." );
      sprintf( buf, "The bay doors on %s slowly slide closed.", ship->name );
      echo_to_system( AT_YELLOW, ship, buf, NULL );
      if( ship->hanger1 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger1 ), "&R&YThe bay doors slowly slide closed." );
      if( ship->hanger2 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger2 ), "&R&YThe bay doors slowly slide closed." );
      if( ship->hanger3 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger3 ), "&R&YThe bay doors slowly slide closed." );
      if( ship->hanger4 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger4 ), "&R&YThe bay doors slowly slide closed." );

      ship->bayopen = FALSE;
      return;
   }
}

void do_tractorbeam( CHAR_DATA * ch, char *argument )
{

   char arg[MAX_INPUT_LENGTH];
   int schance;
   SHIP_DATA *ship;
   SHIP_DATA *target;
   char buf[MAX_STRING_LENGTH];

   strcpy( arg, argument );

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the cockpit of a ship to do that!\n\r", ch );
      return;
   }

   if( !check_pilot( ch, ship ) )
   {
      send_to_char( "This isn't your ship!\n\r", ch );
      return;
   }

   if( ship->tractorbeam == 0 )
   {
      send_to_char( "You might want to install a tractorbeam!\n\r", ch );
      return;
   }

   if( ship->hanger1 == 0 && ship->hanger2 == 0 )
   {
      send_to_char( "No hanger available.\n\r", ch );
      return;
   }

   if( !ship->bayopen )
   {
      send_to_char( "Your hanger is closed.\n\r", ch );
      return;
   }

   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou need to be in the pilot seat!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RThe ships drive is disabled. No power available.\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RYour ship is docked!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {

      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }

   if( ship->shipstate != SHIP_READY )
   {
      send_to_char( "&RPlease wait until the ship has finished its current manouver.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "&RCapture what? (use none to clear)\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      send_to_char( "&RShip tractoring cleared.\n\r", ch );
      if( ship->tractoring != NULL )
      {
         ship->tractoring->tractored_by = NULL;
         ship->tractoring = NULL;
      }
      return;
   }

   target = get_ship_here( argument, ship->starsystem );

   if( target == NULL )
   {
      send_to_char( "&RI don't see that here. (use 'none' to clear)\n\r", ch );
      return;
   }

   if( target == ship )
   {
      send_to_char( "&RTractor your own ship?\n\r", ch );
      return;
   }

   if( target->shipstate == SHIP_LAND )
   {
      send_to_char( "&RThat ship is already in a landing sequence.\n\r", ch );
      return;
   }

   if( ( target->vx > ship->vx + 200 ) || ( target->vx < ship->vx - 200 ) ||
       ( target->vy > ship->vy + 200 ) || ( target->vy < ship->vy - 200 ) ||
       ( target->vz > ship->vz + 200 ) || ( target->vz < ship->vz - 200 ) )
   {
      send_to_char( "&RThat ship is too far away! You'll have to fly a little closer.\n\r", ch );
      return;
   }

   if( ship->class <= target->class )
   {
      send_to_char( "&RThat ship is too big for your hanger.\n\r", ch );
      return;
   }

   if( target->class >= SHIP_CRUISER )
   {
      send_to_char( "&RYou can't capture capital ships.\n\r", ch );
      return;
   }


   if( ship->energy < ( 25 + 25 * target->class ) )
   {
      send_to_char( "&RThere's not enough fuel!\n\r", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_tractorbeams] );

   if( number_percent(  ) < schance )
   {
      send_to_char( "&GCapture sequence initiated.\n\r", ch );
      act( AT_PLAIN, "$n begins the capture sequence.", ch, NULL, argument, TO_ROOM );
      echo_to_ship( AT_YELLOW, target, "The ship shudders as a tractorbeam locks on." );
      sprintf( buf, "You are being captured by %s.", ship->name );
      echo_to_cockpit( AT_BLOOD, target, buf );

      if( autofly( target ) && !target->target0 )
         target->target0 = ship;

      target->dest = STRALLOC( ship->name );
      target->shipstate = SHIP_LAND;
      target->autopilot = FALSE;
      target->currspeed = 0;
      target->tractored_by = ship;
      ship->tractoring = target;
      learn_from_success( ch, gsn_tractorbeams );
      return;

   }
   send_to_char( "You fail to work the controls properly.\n\r", ch );
   echo_to_ship( AT_YELLOW, target, "The ship shudders and then stops as a tractorbeam attempts to lock on." );
   sprintf( buf, "The %s attempted to capture your ship!", ship->name );
   echo_to_cockpit( AT_BLOOD, target, buf );
   if( autofly( target ) && !target->target0 )
      target->target0 = ship;


   learn_from_failure( ch, gsn_tractorbeams );
   return;
}

void do_pluogus( CHAR_DATA * ch, char *argument )
{
   bool ch_comlink = FALSE;
   OBJ_DATA *obj;
   int next_planet, itt;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->pIndexData->item_type == ITEM_COMLINK )
         ch_comlink = TRUE;
   }

   if( !ch_comlink )
   {
      send_to_char( "You need a comlink to do that!\n\r", ch );
      return;
   }

   send_to_char( "Serin Sol Schedule Information:\n\r", ch );
   /*
    * current port 
    */
   if( bus_pos < 7 && bus_pos > 1 )
      ch_printf( ch, "The shuttle is currently docked at %s.\n\r", bus_stop[bus_planet] );
   /*
    * destinations 
    */
   next_planet = bus_planet;
   send_to_char( "Next stops: ", ch );
   if( bus_pos <= 1 )
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   for( itt = 0; itt < 3; itt++ )
   {
      next_planet++;
      if( next_planet >= MAX_BUS_STOP )
         next_planet = 0;
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   }
   ch_printf( ch, "\n\r\n\r" );

   send_to_char( "Serin Monir Schedule Information:\n\r", ch );
   /*
    * current port 
    */
   if( bus_pos < 7 && bus_pos > 1 )
      ch_printf( ch, "The shuttle is currently docked at %s.\n\r", bus_stop[bus2_planet] );
   /*
    * destinations 
    */
   next_planet = bus2_planet;
   send_to_char( "Next stops: ", ch );
   if( bus_pos <= 1 )
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   for( itt = 0; itt < 3; itt++ )
   {
      next_planet++;
      if( next_planet >= MAX_BUS_STOP )
         next_planet = 0;
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   }
   ch_printf( ch, "\n\r\n\r" );

   send_to_char( "Serin Faun Schedule Information:\n\r", ch );
   /*
    * current port 
    */
   if( bus_pos < 7 && bus_pos > 1 )
      ch_printf( ch, "The shuttle is currently docked at %s.\n\r", bus_stop[bus3_planet] );
   /*
    * destinations 
    */
   next_planet = bus3_planet;
   send_to_char( "Next stops: ", ch );
   if( bus_pos <= 1 )
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   for( itt = 0; itt < 3; itt++ )
   {
      next_planet++;
      if( next_planet >= MAX_BUS_STOP )
         next_planet = 0;
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   }
   ch_printf( ch, "\n\r\n\r" );

   send_to_char( "Serin Taw Schedule Information:\n\r", ch );
   /*
    * current port 
    */
   if( bus_pos < 7 && bus_pos > 1 )
      ch_printf( ch, "The shuttle is currently docked at %s.\n\r", bus_stop[bus4_planet] );
   /*
    * destinations 
    */
   next_planet = bus4_planet;
   send_to_char( "Next stops: ", ch );
   if( bus_pos <= 1 )
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   for( itt = 0; itt < 3; itt++ )
   {
      next_planet++;
      if( next_planet >= MAX_BUS_STOP )
         next_planet = 0;
      ch_printf( ch, "%s  ", bus_stop[next_planet] );
   }
   ch_printf( ch, "\n\r" );

}

void do_fly( CHAR_DATA * ch, char *argument )
{
}

void do_drive( CHAR_DATA * ch, char *argument )
{
   int dir;
   SHIP_DATA *ship;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RYou must be in the drivers seat of a land vehicle to do that!\n\r", ch );
      return;
   }

   if( ship->class != LAND_VEHICLE )
   {
      send_to_char( "&RThis isn't a land vehicle!\n\r", ch );
      return;
   }


   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RThe drive is disabled.\n\r", ch );
      return;
   }

   if( ship->energy < 1 )
   {
      send_to_char( "&RTheres not enough fuel!\n\r", ch );
      return;
   }

   if( ( dir = get_door( argument ) ) == -1 )
   {
      send_to_char( "Usage: drive <direction>\n\r", ch );
      return;
   }

   drive_ship( ch, ship, get_exit( get_room_index( ship->location ), dir ), 0 );

}

ch_ret drive_ship( CHAR_DATA * ch, SHIP_DATA * ship, EXIT_DATA * pexit, int fall )
{
   ROOM_INDEX_DATA *in_room;
   ROOM_INDEX_DATA *to_room;
   ROOM_INDEX_DATA *from_room;
   ROOM_INDEX_DATA *original;
   char buf[MAX_STRING_LENGTH];
   char *txt;
   char *dtxt;
   ch_ret retcode;
   short door, distance;
   bool drunk = FALSE;
   CHAR_DATA *rch;
   CHAR_DATA *next_rch;

   if( !IS_NPC( ch ) )
      if( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE ) && ( ch->position != POS_DRAG ) )
         drunk = TRUE;

   if( drunk && !fall )
   {
      door = number_door(  );
      pexit = get_exit( get_room_index( ship->location ), door );
   }

#ifdef DEBUG
   if( pexit )
   {
      sprintf( buf, "drive_ship: %s to door %d", ch->name, pexit->vdir );
      log_string( buf );
   }
#endif

   retcode = rNONE;
   txt = NULL;

   in_room = get_room_index( ship->location );
   from_room = in_room;
   if( !pexit || ( to_room = pexit->to_room ) == NULL )
   {
      if( drunk )
         send_to_char( "You drive into a wall in your drunken state.\n\r", ch );
      else
         send_to_char( "Alas, you cannot go that way.\n\r", ch );
      return rNONE;
   }

   door = pexit->vdir;
   distance = pexit->distance;

   if( IS_SET( pexit->exit_info, EX_WINDOW ) && !IS_SET( pexit->exit_info, EX_ISDOOR ) )
   {
      send_to_char( "Alas, you cannot go that way.\n\r", ch );
      return rNONE;
   }

   if( IS_SET( pexit->exit_info, EX_PORTAL ) && IS_NPC( ch ) )
   {
      act( AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL, TO_CHAR );
      return rNONE;
   }

   if( IS_SET( pexit->exit_info, EX_NOMOB ) && IS_NPC( ch ) )
   {
      act( AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR );
      return rNONE;
   }

   if( IS_SET( pexit->exit_info, EX_CLOSED ) && ( IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
   {
      if( !IS_SET( pexit->exit_info, EX_SECRET ) && !IS_SET( pexit->exit_info, EX_DIG ) )
      {
         if( drunk )
         {
            act( AT_PLAIN, "$n drives into the $d in $s drunken state.", ch, NULL, pexit->keyword, TO_ROOM );
            act( AT_PLAIN, "You drive into the $d in your drunken state.", ch, NULL, pexit->keyword, TO_CHAR );
         }
         else
            act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
      }
      else
      {
         if( drunk )
            send_to_char( "You hit a wall in your drunken state.\n\r", ch );
         else
            send_to_char( "Alas, you cannot go that way.\n\r", ch );
      }

      return rNONE;
   }

/*
    if ( distance > 1 )
	if ( (to_room=generate_exit(in_room, &pexit)) == NULL )
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );
*/
   if( room_is_private( ch, to_room ) )
   {
      send_to_char( "That room is private right now.\n\r", ch );
      return rNONE;
   }

   if( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && ch->in_room->area != to_room->area )
   {
      if( ch->top_level < to_room->area->low_hard_range )
      {
         set_char_color( AT_TELL, ch );
         switch ( to_room->area->low_hard_range - ch->top_level )
         {
            case 1:
               send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'", ch );
               break;
            case 2:
               send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'", ch );
               break;
            case 3:
               send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch );
               break;
            default:
               send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch );
         }
         return rNONE;
      }
      else if( ch->top_level > to_room->area->hi_hard_range )
      {
         set_char_color( AT_TELL, ch );
         send_to_char( "A voice in your mind says, 'There is nothing more for you down that path.'", ch );
         return rNONE;
      }
   }

   if( !fall )
   {
      if( IS_SET( to_room->room_flags, ROOM_INDOORS )
          || IS_SET( to_room->room_flags, ROOM_SPACECRAFT ) || to_room->sector_type == SECT_INSIDE )
      {
         send_to_char( "You can't drive indoors!\n\r", ch );
         return rNONE;
      }

      if( IS_SET( to_room->room_flags, ROOM_NO_DRIVING ) )
      {
         send_to_char( "You can't take a vehicle through there!\n\r", ch );
         return rNONE;
      }

      if( in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR || IS_SET( pexit->exit_info, EX_FLY ) )
      {

         send_to_char( "You can't go there.\n\r", ch );
         return rNONE;

      }

      if( in_room->sector_type == SECT_WATER_NOSWIM
          || to_room->sector_type == SECT_WATER_NOSWIM
          || to_room->sector_type == SECT_WATER_SWIM
          || to_room->sector_type == SECT_UNDERWATER || to_room->sector_type == SECT_OCEANFLOOR )
      {

         send_to_char( "You'd need a boat to go there.\n\r", ch );
         return rNONE;


      }

      if( IS_SET( pexit->exit_info, EX_CLIMB ) )
      {

         send_to_char( "You need to fly or climb to get up there.\n\r", ch );
         return rNONE;

      }

   }

   if( to_room->tunnel > 0 )
   {
      CHAR_DATA *ctmp;
      int count = 0;

      for( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
         if( ++count >= to_room->tunnel )
         {
            send_to_char( "There is no room for you in there.\n\r", ch );
            return rNONE;
         }
   }

   if( fall )
      txt = "falls";
   else if( !txt )
   {
      if( ship->class == LAND_VEHICLE )
      {
         txt = "drive";
      }
   }
   sprintf( buf, "$n %ss the vehicle $T.", txt );
   act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM );
   sprintf( buf, "You %s the vehicle $T.", txt );
   act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_CHAR );
   sprintf( buf, "%s %ss %s.", ship->name, txt, dir_name[door] );
   echo_to_room( AT_ACTION, get_room_index( ship->location ), buf );

   extract_ship( ship );
   ship_to_room( ship, to_room->vnum );

   ship->location = to_room->vnum;
   ship->lastdoc = ship->location;

   if( fall )
      txt = "falls";
   if( ship->class == LAND_VEHICLE )
   {
      txt = "drives in";
   }

   switch ( door )
   {
      default:
         dtxt = "somewhere";
         break;
      case 0:
         dtxt = "the south";
         break;
      case 1:
         dtxt = "the west";
         break;
      case 2:
         dtxt = "the north";
         break;
      case 3:
         dtxt = "the east";
         break;
      case 4:
         dtxt = "below";
         break;
      case 5:
         dtxt = "above";
         break;
      case 6:
         dtxt = "the south-west";
         break;
      case 7:
         dtxt = "the south-east";
         break;
      case 8:
         dtxt = "the north-west";
         break;
      case 9:
         dtxt = "the north-east";
         break;
   }

   sprintf( buf, "%s %s from %s.", ship->name, txt, dtxt );
   echo_to_room( AT_ACTION, get_room_index( ship->location ), buf );

   for( rch = ch->in_room->last_person; rch; rch = next_rch )
   {
      next_rch = rch->prev_in_room;
      original = rch->in_room;
      char_from_room( rch );
      char_to_room( rch, to_room );
      do_look( rch, "auto" );
      char_from_room( rch );
      char_to_room( rch, original );
   }

/*
    if (  CHECK FOR FALLING HERE
    &&   fall > 0 )
    {
	if (!IS_AFFECTED( ch, AFF_FLOATING )
	|| ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLOATING ) ) )
	{
	  set_char_color( AT_HURT, ch );
	  send_to_char( "OUCH! You hit the ground!\n\r", ch );
	  WAIT_STATE( ch, 20 );
	  retcode = damage( ch, ch, 50 * fall, TYPE_UNDEFINED );
	}
	else
	{
	  set_char_color( AT_MAGIC, ch );
	  send_to_char( "You lightly float down to the ground.\n\r", ch );
	}
    }

*/
   return retcode;

}

void do_bomb( CHAR_DATA * ch, char *argument )
{
   int vnum, i, hibomb, lowbomb;
   SHIP_DATA *ship;
   OBJ_INDEX_DATA *pobj;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *room;
   PLANET_DATA *planet;
   int bombs;
   bool foundplanet = FALSE;
   char buf[MAX_STRING_LENGTH];
   int schance;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be in the cockpit of the bomber to do this.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' || isalpha( argument[0] ) )
   {
      send_to_char( "&RDrop how many bombs?\n\r", ch );
      return;
   }
   bombs = atoi( argument );


   if( ship->class != SHIP_BOMBER )
   {
      send_to_char( "&RYou must be in the cockpit of the bomber to do this.\n\r", ch );
      return;
   }

   if( ship->maxbombs <= 0 )
   {
      send_to_char( "&RThis ship doesn't have the ability to carry bombs!\n\r", ch );
      return;
   }

   if( ship->bombs == 0 )
   {
      send_to_char( "&RYou have no bombs left!\n\r", ch );
      return;
   }

   if( ship->bombs < bombs )
   {
      send_to_char( "&RYou don't have that many bombs left!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_DISABLED )
   {
      send_to_char( "&RYou can't bomb while your ship is disabled.\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RYou can't do that while the ship is docked!\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }

   if( ship->shipstate != SHIP_READY )
   {
      send_to_char( "&RPlease wait until the ship has finished its current maneuver.\n\r", ch );
      return;
   }

   if( ship->starsystem == NULL )
   {
      send_to_char( "&RThere's no planets around here!", ch );
      return;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_bomb] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou can't seem to maneuver into the right position to unleash your payload.\n\r", ch );
      return;
   }

   for( planet = ship->starsystem->first_planet; planet; planet = planet->next_in_system )
      if( ( ship->vx > planet->x - 20 ) && ( ship->vx < planet->x + 20 ) && ( ship->vy > planet->y - 20 )
          && ( ship->vy < planet->y + 20 ) && ( ship->vz > planet->z - 20 ) && ( ship->vz < planet->z + 20 ) )
      {
         foundplanet = TRUE;
         break;
      }

   if( !foundplanet )
   {
      send_to_char( "&RYou must be directly orbitting a planet in order to bomb it.\n\r", ch );
      return;
   }

   if( !planet->first_area )
   {
      send_to_char( "&RThat planet has no place to bomb!\n\r", ch );
      return;
   }

   if( !ship->lowbombstr || ship->lowbombstr == 0 )
      lowbomb = 75;
   else
      lowbomb = ship->lowbombstr;

   if( !ship->hibombstr || ship->hibombstr == 0 )
      hibomb = 150;
   else
      hibomb = ship->hibombstr;

   for( i = 1; i <= bombs; i++ )
   {
      vnum = number_range( planet->first_area->low_r_vnum, planet->first_area->hi_r_vnum );
      if( ( room = get_room_index( vnum ) ) == NULL || !str_cmp( room->name, "Floating in a void" ) )
         --i;
      else if( ( pobj = get_obj_index( OBJ_VNUM_SHIPBOMB ) ) != NULL )
      {
         if( !IS_SET( room->room_flags, ROOM_INDOORS ) )
         {
            sprintf( buf, "Bomb dropped in room %s", room->name );
            log_string( buf );
            obj = create_object( pobj, 30 );
            obj->value[0] = lowbomb;
            obj->value[1] = hibomb;
            obj->armed_by = ch->name;
            obj_to_room( obj, room );
            echo_to_room( AT_WHITE, room, "A bomb falls from the sky..." );
            explode( obj );
         }
         else
            --i;
      }
   }
   sprintf( buf,
            "&GYou unleash your payload of %d bombs onto the lands of %s, and watch as the bombs make small explosions viewable from orbit.\n\r",
            bombs, planet->name );
   ship->bombs -= bombs;
   echo_to_cockpit( AT_GREEN, ship, buf );
   learn_from_success( ch, gsn_bomb );
}

void do_chaff( CHAR_DATA * ch, char *argument )
{
   int schance;
   SHIP_DATA *ship;


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


   if( ( ship = ship_from_coseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "&RThe controls are at the copilots seat!\n\r", ch );
      return;
   }

   if( autofly( ship ) )
   {
      send_to_char( "&RYou'll have to turn the autopilot off first...\n\r", ch );
      return;
   }

   if( ship->shipstate == SHIP_HYPERSPACE )
   {
      send_to_char( "&RYou can only do that in realspace!\n\r", ch );
      return;
   }
   if( ship->shipstate == SHIP_DOCKED )
   {
      send_to_char( "&RYou can't do that until after you've launched!\n\r", ch );
      return;
   }
   if( ship->chaff <= 0 )
   {
      send_to_char( "&RYou don't have any chaff to release!\n\r", ch );
      return;
   }
   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_weaponsystems] );
   if( number_percent(  ) > schance )
   {
      send_to_char( "&RYou can't figure out which switch it is.\n\r", ch );
      learn_from_failure( ch, gsn_weaponsystems );
      return;
   }

   ship->chaff--;

   ship->chaff_released++;

   send_to_char( "You flip the chaff release switch.\n\r", ch );
   act( AT_PLAIN, "$n flips a switch on the control pannel", ch, NULL, argument, TO_ROOM );
   echo_to_cockpit( AT_YELLOW, ship, "A burst of chaff is released from the ship." );

   learn_from_success( ch, gsn_weaponsystems );

}

bool autofly( SHIP_DATA * ship )
{

   if( !ship )
      return FALSE;

   if( ship->type == MOB_SHIP )
      return TRUE;

   if( ship->autopilot )
      return TRUE;

   return FALSE;

}


SHIP_DATA *ship_from_room( int vnum )
{
   SHIP_DATA *ship;
   int sRoom;

   for( ship = first_ship; ship; ship = ship->next )
      for( sRoom = ship->firstroom; sRoom < ship->lastroom + 1; sRoom++ )
         if( vnum == sRoom )
            return ship;
   return NULL;
}


void do_salvage( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   int value;
   char buf[MAX_STRING_LENGTH];
   ship = ship_in_room( ch->in_room, argument );
   if( !ship )
   {
      send_to_char( "No such ship.\n\r", ch );
      return;
   }
   if( str_cmp( ship->owner, ch->name ) )
   {
      send_to_char( "You do not own this ship!\n\r", ch );
      return;
   }
   if( !IS_NPC( ch ) && ch->top_level < 11 )
   {
      send_to_char( "Due to cheating, players under level 11 are not allowed to sell ships.\n\r", ch );
      return;
   }
   //sprintf(buf,"%s: removeship %s",ch->name,argument);
   //log_string(buf);
   if( !IS_SET( ch->in_room->room_flags2, ROOM_SHIPYARD ) && !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
   {
      send_to_char( "You must be in a shipyard.\n\r", ch );
      return;
   }
   if( !IS_SET( ch->in_room->room_flags2, ROOM_BLACKMARKET ) )
   {
      value = ( get_ship_value( ship ) / 2 );
      act( AT_WHITE, "You sell your ship to the shipyard for spare parts.", ch, NULL, NULL, TO_CHAR );
      act( AT_WHITE, "$n sells $s ship to the shipyard for spare parts.", ch, NULL, NULL, TO_ROOM );
      sprintf( buf, "You get %d credits for the spare parts.\n\r", value );
      send_to_char( buf, ch );
      ch->gold += value;
      extract_ship( ship );
      ship_to_room( ship, 46 );
      destroy_ship( ship, ch, "salvaged" );
      send_to_char( "Ship removed.\r\n", ch );
   }
   else
   {
      if( !ship->clanowner || ship->clanowner == NULL || ship->clanowner[0] == '\0' )
      {
         send_to_char( "&R&CA smuggler says:&R&W I don't want that! Gimme an organization-owned ship.\n\r", ch );
         return;
      }
      value = ( get_ship_value( ship ) / 2 );
      send_to_char( "You sell your ship to the Black Market.\n\r", ch );
      act( AT_WHITE, "$n sells his ship to the Black Market.\n\r", ch, NULL, NULL, TO_ROOM );
      sprintf( buf, "You get %d credits for selling your ship.\n\r", value );
      send_to_char( buf, ch );
      add_market_ship( ship );
      ch->gold += value;
      extract_ship( ship );
      ship_to_room( ship, 46 );
      destroy_ship( ship, ch, "sold to black market" );
   }
   return;
}

void do_clansalvage( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   CLAN_DATA *clan;
   int value;
   char buf[MAX_STRING_LENGTH];

   ship = ship_in_room( ch->in_room, argument );
   if( !ship )
   {
      send_to_char( "No such ship.\n\r", ch );
      return;
   }
   if( !ch->pcdata->clan )
   {
      send_to_char( "&RYou aren't a member of any organizations!\n\r", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata->bestowments
         && is_name( "clanbuyship", ch->pcdata->bestowments ) ) || nifty_is_name( ch->name, clan->leader ) )
      ;
   else
   {
      send_to_char( "&RYour organization hasn't seen fit to bestow you with ship handling.\n\r", ch );
      return;
   }

   if( str_cmp( ch->pcdata->clan->name, ship->owner ) )
   {
      send_to_char( "Your clan does not own this ship!\n\r", ch );
      return;
   }

   if( !IS_SET( ch->in_room->room_flags2, ROOM_SHIPYARD ) )
   {
      send_to_char( "You must be in a shipyard.\n\r", ch );
      return;
   }
   value = ( get_ship_value( ship ) / 2 );
   act( AT_WHITE, "You sell your clans ship to the shipyard for spare parts.", ch, NULL, NULL, TO_CHAR );
   act( AT_WHITE, "$n sells $s ship to the shipyard for spare parts.", ch, NULL, NULL, TO_ROOM );
   sprintf( buf, "Your clan is reimbursed %d credits for the parts.\n\r", value );
   send_to_char( buf, ch );
   clan->funds += value;
   destroy_ship( ship, ch, "salvaged" );
   return;
}

void do_endsimulator( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char buf[MAX_INPUT_LENGTH];

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be in the cockpit of a simulator.\n\r", ch );
      return;
   }
   if( !IS_SET( ship->flags, SHIP_SIMULATOR ) )
   {
      send_to_char( "You must be in the cockpit of a simulator.\n\r", ch );
      return;
   }
   sprintf( buf, "%s suddenly disappears from your radar.\n\r", ship->name );
   echo_to_system( AT_YELLOW, ship, buf, NULL );
   ship->shipstate = SHIP_READY;
   extract_ship( ship );
   ship_to_room( ship, ship->sim_vnum );
   if( !ship->in_room )
      ship_to_room( ship, 45 );
   ship->shipyard = ship->sim_vnum;
   ship->location = ship->shipyard;
   ship->lastdoc = ship->shipyard;
   ship->shipstate = SHIP_DOCKED;
   if( ship->starsystem )
      ship_from_starsystem( ship, ship->starsystem );
   save_ship( ship );
   send_to_char( "You press a button and the simulation ends.\n\r", ch );

}

void do_freeship( CHAR_DATA * ch, char *argument )
{
/*	SHIP_DATA *ship;
	char buf[MAX_INPUT_LENGTH];
    send_to_char ("&z+-----------------------------------------------------------------------------+\n\r", ch);
    send_to_char ("&z| &cShip                      &z| &YPrice      &z| &RLocation &z                          |\n\r", ch);
    send_to_char ("&z+---------------------------|------------|------------------------------------+\n\r", ch);
    for ( ship = first_ship; ship; ship = ship->next )
    {
        if ( strlen(ship->owner) < 2 && ship->location != 12 && ship->location != 45 && (ship->location < 100 || ship->location > 100) && ship->location != 136 && ship->location != 11499 )
	{
            if  ( ship->in_room )
            {
                sprintf (buf, "&z| %s%-25.25s &z| &Y%-10ld&z | &R%-34.33s &z|\n\r", ship->type == SHIP_REPUBLIC ? "&r": ship->type == SHIP_IMPERIAL ? "&g":  "&B", ship->name, get_ship_value(ship), ship->in_room->name);
                send_to_char (buf, ch);
            }
            else
            {
                sprintf (buf, "&z| %s%-25.25s &z| &Y%-10ld&z | &RSpace                              &z|\n\r",  ship->type == SHIP_REPUBLIC ? "&r": ship->type == SHIP_IMPERIAL ? "&g": "&B", ship->name, get_ship_value(ship));
                send_to_char (buf, ch);
            }
        }
    }
    send_to_char ("&z+-----------------------------------------------------------------------------+\n\r", ch);
  return;*/
}

void do_giveship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   CHAR_DATA *victim;

   char logbuf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Incorrect syntax, see 'HELP GIVESHIP'.\n\r", ch );
      return;
   }

   ship = ship_in_room( ch->in_room, arg1 );
   if( !ship )
   {
      send_to_char( "That ship isn't here.\n\r", ch );
      return;
   }
   if( str_cmp( ship->owner, ch->name ) )
   {
      send_to_char( "That isn't your ship.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg2 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Giving your ship to an NPC, eh? I don't think so.\n\r", ch );
      return;
   }
   if( !IS_NPC( ch ) && ch->top_level < 11 )
   {
      send_to_char( "Due to cheating, players under level 11 are not allowed to move ships.\n\r", ch );
      return;
   }

   STRFREE( ship->owner );
   ship->owner = STRALLOC( victim->name );
   ch_printf( ch, "&R&GYou give %s to %s.\n\r", ship->name, victim->name );
   ch_printf( victim, "&R&G%s gives %s to you.\n\r", ch->name, ship->name );
   sprintf( logbuf, "%s gave the ship %s to %s.\n\r", ch->name, ship->name, victim->name );
   log_string( logbuf );
   return;
}

void do_clangiveship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   CLAN_DATA *clan;

   char buf[MAX_STRING_LENGTH];
   char logbuf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Give what ship to what clan?\n\r", ch );
      return;
   }

   ship = ship_in_room( ch->in_room, arg1 );
   if( !ship )
   {
      send_to_char( "That ship isn't here.\n\r", ch );
      return;
   }
   if( str_cmp( ship->owner, ch->name ) )
   {
      send_to_char( "That isn't your ship.\n\r", ch );
      return;
   }

   if( ( clan = get_clan( argument ) ) == NULL )
   {
      send_to_char( "No such clan. HELP CLANGIVESHIP for more info.\n\r", ch );
      return;
   }

   if( !IS_NPC( ch ) && ch->top_level < 11 )
   {
      send_to_char( "Due to cheating, players under level 11 are not allowed to move credits.\n\r", ch );
      return;
   }

   STRFREE( ship->owner );
   ship->owner = STRALLOC( clan->name );
   ch_printf( ch, "&R&GYou give %s to %s.\n\r", ship->name, clan->name );
   sprintf( buf, "&R&G%s is now owned by %s.\n\r", ship->name, clan->name );
   act( AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM );
   sprintf( logbuf, "%s gave the ship %s to %s.\n\r", ch->name, ship->name, clan->name );
   log_string( logbuf );

   if( ship->class <= SHIP_SPACE_STATION )
      clan->spacecraft++;
   else
      clan->vehicles++;

   return;
}

void do_request( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   SHIP_DATA *target;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   bool open;

   open = FALSE;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax:  request <ship> <open/close>.\n\r", ch );
      return;
   }
   if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be at the pilot controls of a ship to do that.\n\r", ch );
      return;
   }
   if( ( target = get_ship_here( arg1, ship->starsystem ) ) == NULL )
   {
      send_to_char( "That ship doesn't seem to be here.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "open" ) )
   {
      if( target->bayopen == TRUE )
      {
         send_to_char( "The bay doors are already open.\n\r", ch );
         return;
      }

      if( !str_cmp( ch->name, target->owner ) )
         open = TRUE;
      if( ch->pcdata->clan )
      {
         if( !str_cmp( ch->pcdata->clan->name, target->owner ) )
            open = TRUE;
      }
      if( !str_cmp( target->owner, ship->owner ) )
         open = TRUE;

      sprintf( buf, "&GYou request clearance to land on %s.\n\r", target->name );
      send_to_char( buf, ch );
      sprintf( buf, "&G%s requesting landing clearance.\n\r", ship->name );
      echo_to_cockpit( AT_BLOOD, target, buf );

      if( open == TRUE )
      {
         sprintf( buf, "\n\r&R&G%s hails: &WConfirmed %s, you're clear to land.\n\r", target->name, ship->name );
         echo_to_cockpit( AT_WHITE, ship, buf );
         sprintf( buf, "&YThe bay doors on %s slide open.\n\r", target->name );
         echo_to_system( AT_YELLOW, target, buf, NULL );
         echo_to_cockpit( AT_WHITE, target, "&G[&gShip Computer&G]&w Opening bay doors." );

         if( ship->hanger1 != 0 )
            echo_to_room( AT_WHITE, get_room_index( ship->hanger1 ), "&R&YThe bay doors slowly slide open." );
         if( ship->hanger2 != 0 )
            echo_to_room( AT_WHITE, get_room_index( ship->hanger2 ), "&R&YThe bay doors slowly slide open." );
         if( ship->hanger3 != 0 )
            echo_to_room( AT_WHITE, get_room_index( ship->hanger3 ), "&R&YThe bay doors slowly slide open." );
         if( ship->hanger4 != 0 )
            echo_to_room( AT_WHITE, get_room_index( ship->hanger4 ), "&R&YThe bay doors slowly slide open." );

         target->bayopen = TRUE;
         return;
      }
      else
      {
         sprintf( buf, "\n\r&R&G%s hails: &WNegative %s, resume previous flight pattern.\n\r", target->name, ship->name );
         echo_to_cockpit( AT_WHITE, ship, buf );
         sprintf( buf, "&R%s is denied landing clearance.\n\r", ship->name );
         echo_to_cockpit( AT_WHITE, target, buf );
         return;
      }
   }
   else if( !str_cmp( arg2, "close" ) )
   {
      if( target->bayopen == FALSE )
      {
         send_to_char( "The bay doors are already closed.\n\r", ch );
         return;
      }

      sprintf( buf, "&R&G%s hails: &WAcknowledged, %s.", target->name, ship->name );
      echo_to_cockpit( AT_WHITE, ship, buf );
      echo_to_cockpit( AT_WHITE, target, "&G[&gShip Computer&G]&W Closing bay doors." );
      sprintf( buf, "The bay doors on %s slowly slide closed.", target->name );
      echo_to_system( AT_YELLOW, target, buf, NULL );
      if( ship->hanger1 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger1 ), "&R&YThe bay doors slowly slide closed." );
      if( ship->hanger2 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger2 ), "&R&YThe bay doors slowly slide closed." );
      if( ship->hanger3 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger3 ), "&R&YThe bay doors slowly slide closed." );
      if( ship->hanger4 != 0 )
         echo_to_room( AT_WHITE, get_room_index( ship->hanger4 ), "&R&YThe bay doors slowly slide closed." );
      target->bayopen = FALSE;

      return;
   }
   else
   {
      send_to_char( "Syntax: request <ship> <open/close>.\n\r", ch );
      return;
   }
}

void do_makesimulator( CHAR_DATA * ch, char *argument )
{

}

void do_makemobship( CHAR_DATA * ch, char *argument )
{

}

void do_prototypes( CHAR_DATA * ch, char *argument )
{

}

//Written by Ackbar, updated by Tranin, Reupdated by Tawnos
void do_sabotage( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance;
   SHIP_DATA *ship;
   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
         {
            send_to_char( "&RYou must be in the engine room of a ship to do that!\n\r", ch );
            return;
         }

         if( str_cmp( argument, "hull" ) && str_cmp( argument, "drive" )
             && str_cmp( argument, "launcher" ) && str_cmp( argument, "laser" ) )
         {
            send_to_char( "&RYou need to specify something to sabotage!\n\r", ch );
            send_to_char( "&rTry: drive, launcher, laser\n\r", ch );
            return;
         }

         if( IS_SET( ship->flags, SHIP_SIMULATOR ) )
         {
            send_to_char( "&GYou pull at some wires.. and end up with space invaders! *Whee!*\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_sabotage] );
         if( number_percent(  ) < schance )

         {
            send_to_char( "&GYou begin yanking at wires.\n\r", ch );
            act( AT_PLAIN, "$n starts ripping into the ships wires.", ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 5, do_sabotage, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou fail to locate the proper cords to sever!\n\r", ch );
         learn_from_failure( ch, gsn_sabotage );
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
         if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
            return;
         send_to_char( "&RYou are distracted and fail to finish your work.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
   {
      return;
   }

   if( !str_cmp( arg, "drive" ) )
   {
      ship->shipstate = SHIP_DISABLED;
      send_to_char( "&GShips drive successfully disabled.\n\r", ch );
   }

   if( !str_cmp( arg, "launcher" ) )
   {
      ship->missilestate = MISSILE_DAMAGED;
      ship->torpedostate = MISSILE_DAMAGED;
      ship->rocketstate = MISSILE_DAMAGED;
      send_to_char( "&GMissile launcher successfully disabled.\n\r", ch );
   }

   if( !str_cmp( arg, "laser" ) )
   {
      ship->primaryState = LASER_DAMAGED;
      send_to_char( "&GMain laser successfully disabled.\n\r", ch );
   }

   act( AT_PLAIN, "$n finishes his rage outburst on the wires.", ch, NULL, argument, TO_ROOM );

   learn_from_success( ch, gsn_sabotage );
}

void do_makeshipbomb( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   int level, schance, strength, weight;
   bool checktool, checkdrink, checkbatt, checkchem1, checkchem2, checkchem3, checkcirc1, checkcirc2;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *pObjIndex;
   int vnum;
   int circNum = 0, chemNum = 0;
   strcpy( arg, argument );

   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' )
         {
            send_to_char( "&RUsage: Makeshipbomb <name>\n\r&w", ch );
            return;
         }

         checktool = checkdrink = checkbatt = checkchem1 = checkchem2 = checkchem3 = checkcirc1 = checkcirc2 = FALSE;

         if( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
         {
            send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {

            if( obj->item_type == ITEM_TOOLKIT )
            {
               checktool = TRUE;
               continue;
            }
            if( obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
            {
               checkdrink = TRUE;
               continue;
            }
            if( obj->item_type == ITEM_BATTERY )
            {
               checkbatt = TRUE;
               continue;
            }
            if( obj->item_type == ITEM_CIRCUIT )
            {
               circNum += obj->count;
               continue;
            }
            if( obj->item_type == ITEM_CHEMICAL )
            {
               chemNum += obj->count;
               continue;
            }
         }

         if( !checktool )
         {
            send_to_char( "&RYou need toolkit.\n\r", ch );
            return;
         }

         if( !checkdrink )
         {
            send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\n\r", ch );
            return;
         }

         if( !checkbatt )
         {
            send_to_char( "&RYou need a battery to spark the explosion.\n\r", ch );
            return;
         }

         if( circNum == 0 )
         {
            send_to_char( "&RYou need a circuit board for the receiver.\n\r", ch );
            return;
         }

         if( circNum == 1 )
         {
            send_to_char( "&RYou need another circuit board for the transmitter.\n\r", ch );
            return;
         }

         if( chemNum == 0 )
         {
            send_to_char( "&RYou need 1 to 3 chemicals to cause an explosion!\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeshipbomb] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin the long process of making a shipbomb.\n\r", ch );
            act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
                 NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 25, do_makeshipbomb, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou can't figure out how to fit the parts together.\n\r", ch );
         learn_from_failure( ch, gsn_makeshipbomb );
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

   level = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeshipbomb] );
   vnum = 68;

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
   checkchem1 = FALSE;
   checkchem2 = FALSE;
   checkchem3 = FALSE;
   checkcirc1 = FALSE;
   checkcirc2 = FALSE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
      {
         checktool = TRUE;
         continue;
      }
      if( obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE && obj->value[1] == 0 )
      {
         checkdrink = TRUE;
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         continue;
      }
      if( obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
      {
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         checkbatt = TRUE;;
         continue;
      }
      if( obj->item_type == ITEM_CHEMICAL )
      {
         if( obj->count > 3 )
         {
            obj->count -= 3;
            chemNum += 3;
         }
         else if( obj->count == 1 )
         {
            separate_obj( obj );
            obj_from_char( obj );
            extract_obj( obj );
            chemNum += 1;
         }
         else if( obj->count == 3 )
         {
            obj_from_char( obj );
            extract_obj( obj );
            chemNum += 3;
         }
         else
         {
            obj_from_char( obj );
            extract_obj( obj );
            chemNum += 2;
         }

         strength = URANGE( chemNum * 10, obj->value[0] * 3, level * 15 );
         weight = obj->weight;
         continue;
      }

      if( obj->item_type == ITEM_CIRCUIT )
      {
         if( obj->count > 2 )
         {
            obj->count -= 2;
            circNum += 2;
         }
         else if( obj->count == 2 )
         {
            obj_from_char( obj );
            extract_obj( obj );
            circNum += 2;
         }
         else
         {
            obj_from_char( obj );
            extract_obj( obj );
            circNum += 1;
         }
         continue;
      }

   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_makeshipbomb] );

   if( number_percent(  ) > schance * 2 || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( chemNum < 1 )
       || ( circNum < 2 ) )
   {
      sprintf( buf, "%d %d", chemNum, circNum );
      log_string( buf );

      send_to_char
         ( "&RJust as you are about to finish your work,\n\ryour newly created bomb pops, and begins smoking...\n\ryou quickly realize you nearly died as it fizzled!\n\r",
           ch );
      learn_from_failure( ch, gsn_makeshipbomb );
      return;
   }

   obj = create_object( pObjIndex, level );

   obj->item_type = ITEM_SHIPBOMB;
   SET_BIT( obj->wear_flags, ITEM_TAKE );
   obj->level = level;
   obj->weight = weight;
   STRFREE( obj->name );
   strcpy( buf, arg );
   strcat( buf, " bomb" );
   obj->name = STRALLOC( buf );
   strcpy( buf, arg );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->description );
   strcat( buf, " was set here." );
   obj->description = STRALLOC( buf );
   obj->value[0] = strength / 2;
   obj->value[1] = strength;
   obj->cost = obj->value[1] * 25;

   obj = obj_to_char( obj, ch );

   send_to_char( "&GYou finish your work and hold up your newly created shipbomb.&w\n\r", ch );
   act( AT_PLAIN, "$n finishes making $s new shipbomb.", ch, NULL, argument, TO_ROOM );

   {
      long xpgain;

      xpgain =
         UMIN( obj->cost * 50,
               ( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
                 exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
      gain_exp( ch, xpgain, ENGINEERING_ABILITY );
      ch_printf( ch, "You gain %d engineering experience.", xpgain );
   }
   learn_from_success( ch, gsn_makeshipbomb );
}


void do_shiprepair( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int schance, change;
   SHIP_DATA *ship;

   strcpy( arg, argument );

   if( ( ch->pcdata->learned[gsn_shiprepair] ) <= 0 )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         if( str_cmp( argument, "turret" ) )
         {
            if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
            {
               send_to_char( "&RYou must be in the engine room of a ship to do that!\n\r", ch );
               return;
            }
         }
         else
         {
            if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
            {
               send_to_char( "&RYou must be in the turret you wish to repair.\n\r", ch );
               return;
            }
            else
            {

               if( ch->in_room->vnum == ship->turret1 )
               {
                  if( ship->statet1 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret2 )
               {
                  if( ship->statet2 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret3 )
               {
                  if( ship->statet3 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret4 )
               {
                  if( ship->statet4 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret5 )
               {
                  if( ship->statet5 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret6 )
               {
                  if( ship->statet6 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret7 )
               {
                  if( ship->statet7 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret8 )
               {
                  if( ship->statet8 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret9 )
               {
                  if( ship->statet9 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else if( ch->in_room->vnum == ship->turret10 )
               {
                  if( ship->statet10 == LASER_READY )
                  {
                     send_to_char( "&GThis turret is operational.\n\r", ch );
                     return;
                  }
               }
               else
               {
                  send_to_char( "&RThis isn't a turret.\n\r", ch );
                  return;
               }
            }
         }

         if( str_cmp( argument, "hull" ) && str_cmp( argument, "drive" ) &&
             str_cmp( argument, "launcher" ) && str_cmp( argument, "laser" ) && str_cmp( argument, "turret" ) )
         {
            send_to_char( "&RYou need to specify something to repair:\n\r", ch );
            send_to_char( "&rTry: hull, drive, launcher, laser, turret.\n\r", ch );
            return;
         }
         if( !str_cmp( arg, "drive" ) )
         {
            if( ship->shipstate == SHIP_HYPERSPACE )
            {
               send_to_char( "You cannot repair a hyperdrive unit in hyperspace.\n\r", ch );
               return;
            }
         }
         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_shiprepair] );
         if( number_percent(  ) < schance )
         {
            send_to_char( "&GYou begin your repairs.\n\r", ch );
            act( AT_PLAIN, "$n begins repairing the ships $T.", ch, NULL, argument, TO_ROOM );
            if( !str_cmp( arg, "hull" ) )
               add_timer( ch, TIMER_DO_FUN, 15, do_shiprepair, 1 );
            else
               add_timer( ch, TIMER_DO_FUN, 5, do_shiprepair, 1 );
            ch->dest_buf = str_dup( arg );
            return;
         }
         send_to_char( "&RYou fail to locate the source of the problem.\n\r", ch );
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
         send_to_char( "&RYou are distracted and fail to finish your repairs.\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL && ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
   {
      return;
   }

   if( !str_cmp( arg, "hull" ) )
   {
      change = URANGE( 0,
                       number_range( ( int )( ch->pcdata->learned[gsn_shiprepair] / 2 ),
                                     ( int )( ch->pcdata->learned[gsn_shiprepair] ) ), ( ship->maxhull - ship->hull ) );
      ship->hull += change;
      ch_printf( ch, "&GRepair complete.. Hull strength inreased by %d points.\n\r", change );
   }

   if( !str_cmp( arg, "drive" ) )
   {
      if( ship->location == ship->lastdoc )
         ship->shipstate = SHIP_DOCKED;
      else
         ship->shipstate = SHIP_READY;
      send_to_char( "&GShips drive repaired.\n\r", ch );
   }

   if( !str_cmp( arg, "launcher" ) )
   {
      ship->missilestate = MISSILE_READY;
      send_to_char( "&GMissile launcher repaired.\n\r", ch );
   }

   if( !str_cmp( arg, "laser" ) )
   {
      ship->primaryState = LASER_READY;
      send_to_char( "&GMain laser repaired.\n\r", ch );
   }
   if( !str_cmp( arg, "turret" ) )
   {

      if( ch->in_room->vnum == ship->turret1 )
         ship->statet1 = LASER_READY;
      if( ch->in_room->vnum == ship->turret2 )
         ship->statet2 = LASER_READY;
      if( ch->in_room->vnum == ship->turret3 )
         ship->statet3 = LASER_READY;
      if( ch->in_room->vnum == ship->turret4 )
         ship->statet4 = LASER_READY;
      if( ch->in_room->vnum == ship->turret5 )
         ship->statet5 = LASER_READY;
      if( ch->in_room->vnum == ship->turret6 )
         ship->statet6 = LASER_READY;
      if( ch->in_room->vnum == ship->turret7 )
         ship->statet7 = LASER_READY;
      if( ch->in_room->vnum == ship->turret8 )
         ship->statet8 = LASER_READY;
      if( ch->in_room->vnum == ship->turret9 )
         ship->statet9 = LASER_READY;
      if( ch->in_room->vnum == ship->turret10 )
         ship->statet10 = LASER_READY;

      send_to_char( "&GTurret operational.\n\r", ch );
   }
   act( AT_PLAIN, "$n finishes the repairs.", ch, NULL, argument, TO_ROOM );

   learn_from_success( ch, gsn_shiprepair );

}

void do_transship( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int arg3;
   SHIP_DATA *ship;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   ship = get_ship( arg1 );
   if( !ship )
   {
      send_to_char( "No such ship.\n\r", ch );
      return;
   }

   arg3 = atoi( arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
   {
      send_to_char( "Usage: transship <ship> <vnum>\n\r", ch );
      return;
   }

   ship->shipstate = SHIP_READY;
   ship->location = arg3;

   if( ship->class != SHIP_SPACE_STATION && ship->type != MOB_SHIP )
   {
      extract_ship( ship );
      ship_to_room( ship, ship->location );

      ship->lastdoc = ship->location;
      ship->shipstate = SHIP_DOCKED;
   }

   if( ship->starsystem )
      ship_from_starsystem( ship, ship->starsystem );

   save_ship( ship );
   send_to_char( "Ship Transfered.\n\r", ch );
}

void do_transshipss( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   SHIP_DATA *ship;
   SPACE_DATA *starsystem;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   ship = get_ship( arg1 );
   if( !ship )
   {
      send_to_char( "No such ship.\n\r", ch );
      return;
   }

   if( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
   {
      send_to_char( "Usage: transshipss <ship> <starsystem>\n\r", ch );
      return;
   }

   ship->shipstate = SHIP_READY;

   starsystem = starsystem_from_name( arg2 );
   if( !starsystem )
   {
      send_to_char( "No such starsystem.\n\r", ch );
      return;

   }
   extract_ship( ship );
   ship_to_starsystem( ship, starsystem );
   ship->location = 0;
   ship->hx = 1;
   ship->hy = 1;
   ship->hz = 1;
   ship->vx = 1111;
   ship->vy = -1111;
   ship->vz = 1111;
   resetship( ship );
   save_ship( ship );
   ship->inship = NULL;
   send_to_char( "Ship transferred.\n\r", ch );
}

void do_removeship( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
   char buf[MAX_STRING_LENGTH];

   if( argument[0] == '\0' )
   {
      send_to_char( "Remove which ship?\n\r", ch );
      return;
   }

   ship = get_ship( argument );
   if( !ship )
   {
      send_to_char( "No such ship.\n\r", ch );
      return;
   }
   sprintf( buf, "%s: removeship %s", ch->name, argument );
   log_string( buf );
   destroy_ship( ship, ch, "removed" );
   send_to_char( "Ship removed.\r\n", ch );
   return;
}



SHIP_DATA *get_ship2( char *name )
{
   SHIP_DATA *ship;
//    char buf[MAX_STRING_LENGTH];
   for( ship = first_ship; ship; ship = ship->next )
   {
//       sprintf(buf, "'%s' '%s'", name, ship->name);
//       bug(buf, 0);
      if( ship->name )
         if( !strcmp( name, ship->name ) )
            return ship;
   }
   return NULL;
}

void do_shiplock( CHAR_DATA * ch, char *argument )
{
   int locknum;
   char buf[MAX_INPUT_LENGTH];
   SHIP_DATA *ship;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be in the cockpit of a ship to do that.\n\r", ch );
      return;
   }

   if( !check_pilot( ch, ship ) )
   {
      send_to_char( "&RHey, thats not your ship! Try renting a public one.\n\r", ch );
      return;
   }

   locknum = atoi( argument );
   if( locknum < 1000 || locknum > 9999 )
   {
      send_to_char( "Number out of range.\n\r", ch );
      return;
   }

   ship->password = locknum;
   sprintf( buf, "%s lock number set to %d.\n\r", ship->name, ship->password );
   send_to_char( buf, ch );
   save_ship( ship );
}

char *ship_bit_name( int vector )
{
   static char buf[512];

   buf[0] = '\0';
   if( vector & SHIP_NOHIJACK )
      strcat( buf, " nohijack" );
   if( vector & SHIP_SHIELD_BOOST )
      strcat( buf, " shield_boost" );
   if( vector & SHIP_TORP_BOOST )
      strcat( buf, " torp_boost" );
   if( vector & SHIP_CHAFF_BOOST )
      strcat( buf, " chaff_boost" );
   if( vector & SHIP_HULL_BOOST )
      strcat( buf, " hull_boost" );
   if( vector & SHIP_LASER_BOOST )
      strcat( buf, " laser_boost" );
   if( vector & SHIP_MISSILE_BOOST )
      strcat( buf, " missile_boost" );
   if( vector & SHIP_ROCKET_BOOST )
      strcat( buf, " rocket_boost" );
   if( vector & SHIP_SIMULATOR )
      strcat( buf, " simulator" );
   if( vector & SHIP_NODESTROY )
      strcat( buf, " nodestroy" );
   if( vector & SHIP_NOSLICER )
      strcat( buf, " noslicer" );
   if( vector & XSHIP_ION_LASERS )
      strcat( buf, " ion_lasers" );
   if( vector & XSHIP_ION_DRIVE )
      strcat( buf, " ion_drive" );
   if( vector & XSHIP_ION_ION )
      strcat( buf, " ion_ion" );
   if( vector & XSHIP_ION_TURRET1 )
      strcat( buf, " ion_turret1" );
   if( vector & XSHIP_ION_TURRET2 )
      strcat( buf, " ion_turret2" );
   if( vector & XSHIP_ION_TURRET3 )
      strcat( buf, " ion_turret3" );
   if( vector & XSHIP_ION_TURRET4 )
      strcat( buf, " ion_turret4" );
   if( vector & XSHIP_ION_TURRET5 )
      strcat( buf, " ion_turret5" );
   if( vector & XSHIP_ION_TURRET6 )
      strcat( buf, " ion_turret6" );
   if( vector & XSHIP_ION_TURRET7 )
      strcat( buf, " ion_turret7" );
   if( vector & XSHIP_ION_TURRET8 )
      strcat( buf, " ion_turret8" );
   if( vector & XSHIP_ION_TURRET9 )
      strcat( buf, " ion_turret9" );
   if( vector & XSHIP_ION_TURRET10 )
      strcat( buf, " ion_turret10" );
   if( vector & SHIP_RESPAWN )
      strcat( buf, " respawn" );
   if( vector & XSHIP_ION_MISSILES )
      strcat( buf, " ion_missiles" );
   if( vector & XSHIP_ION_HYPER )
      strcat( buf, " ion_hyper" );
   if( vector & SHIP_CLOAK )
      strcat( buf, " cloak" );
   return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

void do_cut( CHAR_DATA * ch, char *argument )
{
   SHIP_DATA *ship;
//  char buf[MAX_STRING_LENGTH];
   /*
    * This function will be used to repair damage caused by an ion cannon. Only this
    * damage caused by it will be repaired. It checks by a flag assigned to the ship
    * by the use of ion cannons.
    */


   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be in a ship to cut power.\n\r", ch );
      return;
   }

   if( ship->shipstate != SHIP_READY && ship->shipstate != SHIP_DISABLED )
   {
      send_to_char( "Please wait until you have finished your current manuever.\n\r", ch );
      return;
   }
   switch ( ch->substate )
   {
      default:
         /*
          * if(ship->cloaked == TRUE)
          * {
          * send_to_char("&BYour cloak dissipates with the ship's power.\n\r", ch);
          * sprintf(buf,"&R&B%s blips onto your radar at %.0f, %.0f, %.0f.\n\r", ship->name, ship->vx, ship->vy, ship->vz);
          * echo_to_system(AT_BLUE, ship, buf, NULL);
          * ship->cloaked = FALSE;
          * }
          */
         send_to_char( "You flip the power switch and wait for the ship systems to reboot.\n\r", ch );
         add_timer( ch, TIMER_DO_FUN, 20, do_cut, 1 );
         return;

      case 1:
         break;

      case SUB_TIMER_DO_ABORT:
         ch->substate = SUB_NONE;
         send_to_char( "&RYou flip the power back on but find your systems are still damaged.\n\r", ch );
         return;
   }

   send_to_char( "The ship's systems power up restoring ion damaged systems.\n\r", ch );

   if( IS_SET( ship->flags, XSHIP_ION_DRIVE ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WDrive Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_DRIVE );
      ship->shipstate = SHIP_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_HYPER ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WHyper Drive Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_HYPER );
      ship->shipstate = LASER_READY;

   }

   if( IS_SET( ship->flags, XSHIP_ION_ION ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WSecondary weapon systems functional." );
      REMOVE_BIT( ship->flags, XSHIP_ION_ION );
      ship->secondaryState = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_LASERS ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WPrimary weapon systems functional." );
      REMOVE_BIT( ship->flags, XSHIP_ION_LASERS );
      ship->primaryState = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_MISSILES ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WMissiles Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_MISSILES );
      ship->missilestate = MISSILE_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET1 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret1 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET1 );
      ship->turret1 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET2 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret2 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET2 );
      ship->turret2 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET3 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret3 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET3 );
      ship->turret3 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET4 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret4 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET4 );
      ship->turret4 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET5 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret5 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET5 );
      ship->turret5 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET6 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret6 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET6 );
      ship->turret6 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET7 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret7 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET7 );
      ship->turret7 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET8 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret8 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET8 );
      ship->turret8 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET9 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret9 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET9 );
      ship->turret9 = LASER_READY;
   }

   if( IS_SET( ship->flags, XSHIP_ION_TURRET10 ) )
   {
      echo_to_cockpit( AT_GREEN, ship, "&G[&WALERT&G] &WTurret10 Functional" );
      REMOVE_BIT( ship->flags, XSHIP_ION_TURRET10 );
      ship->turret10 = LASER_READY;
   }
}
