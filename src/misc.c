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
#include "sha256.h"

extern int top_exit;

void do_buyhome( CHAR_DATA * ch, const char *argument )
{
   ROOM_INDEX_DATA *room;
   AREA_DATA *pArea;

   if( !ch->in_room )
      return;

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   if( ch->plr_home != NULL )
   {
      send_to_char( "&RYou already have a home!\r\n&w", ch );
      return;
   }

   room = ch->in_room;

   for( pArea = first_bsort; pArea; pArea = pArea->next_sort )
   {
      if( room->area == pArea )
      {
         send_to_char( "&RThis area isn't installed yet!\r\n&w", ch );
         return;
      }
   }

   if( !IS_SET( room->room_flags, ROOM_EMPTY_HOME ) )
   {
      send_to_char( "&RThis room isn't for sale!\r\n&w", ch );
      return;
   }

   if( ch->gold < 25000 )
   {
      send_to_char( "&RThis room costs 25000 credits you don't have enough!\r\n&w", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Set the room name.  A very brief single line room description.\r\n", ch );
      send_to_char( "Usage: Buyhome <Room Name>\r\n", ch );
      return;
   }

   STRFREE( room->name );
   room->name = STRALLOC( argument );

   ch->gold -= 25000;

   REMOVE_BIT( room->room_flags, ROOM_EMPTY_HOME );
   SET_BIT( room->room_flags, ROOM_PLR_HOME );

   fold_area( room->area, room->area->filename, FALSE );

   ch->plr_home = room;
   do_save( ch, "" );

   send_to_char( "&GYou buy your home for 25000 credits.\r\n", ch );
}

void do_sellhome( CHAR_DATA * ch, const char *argument )
{
   ROOM_INDEX_DATA *room = ch->plr_home;
   AREA_DATA *pArea;
   char hbuf[MSL];

   if( !ch->in_room )
      return;

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   room = ch->in_room;

   if( ch->plr_home != room )
   {
      send_to_char( "&RYou don't own this place!\r\n&w", ch );
      return;
   }

   for( pArea = first_bsort; pArea; pArea = pArea->next_sort )
   {
      if( room->area == pArea )
      {
         send_to_char( "&RThis area isn't installed yet!\r\n&w", ch );
         return;
      }
   }

   if( !IS_SET( room->room_flags, ROOM_PLR_HOME ) )
   {
      send_to_char( "&RThis room isn't owned!\r\n&w", ch );
      return;
   }

   STRFREE( room->name );
   room->name = STRALLOC( "An Empty Apartment" );
   ch->gold += 10000;
   REMOVE_BIT( room->room_flags, ROOM_PLR_HOME );
   SET_BIT( room->room_flags, ROOM_EMPTY_HOME );

   fold_area( room->area, room->area->filename, FALSE );

   ch->plr_home = NULL;
   do_save( ch, "" );
   snprintf( hbuf, MSL, "%s%c/%s.home", PLAYER_DIR, LOWER(ch->name[0]), ch->name );
   if( remove( hbuf ) == -1 )
       bug( "%s: There was an error removing %s's home file", __func__, ch->name );

   send_to_char( "&GYou sell your home for 10000 credits.\r\n", ch );
}

void do_clone( CHAR_DATA * ch, const char *argument )
{
   long credits, bank;
   long played;
   int frclevel, frc;
   int salary;
   char clanname[MAX_STRING_LENGTH];
   char bestowments[MAX_STRING_LENGTH];
   int flags;
   ROOM_INDEX_DATA *home;
   bool secondroom = FALSE;

   if( IS_NPC( ch ) )
   {
      ch_printf( ch, "Yeah right!\r\n" );
      return;
   }

   if( IS_IMMORTAL( ch ) )
   {
      send_to_char( "You wish!\r\n", ch );
      return;
   }

   if( ch->in_room->vnum != 10001 && ch->in_room->vnum != 1233 )
   {
      ch_printf( ch, "You can't do that here!\r\n" );
      return;
   }

   if( IS_DROID( ch ) )
   {
      ch_printf( ch, "Droids don't clone, they &W&RBackup&w&W!\r\n" );
      return;
   }

   if( ch->in_room->vnum == 1233 )
      secondroom = TRUE;

   if( ch->rppoints < 4 )
   {
      ch_printf( ch, "Cloning costs 4 RP Points. You don't have enough.\r\nHelp rppoints for more information." );
      return;
   }
   else
   {
      ch->rppoints -= 4;

      ch_printf( ch, "You are escorted into a small room.\r\n\r\n" );
   }

   if( !secondroom )
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 10000 ) );
   }
   else
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 1234 ) );
   }

   flags = ch->act;
   frc = ch->perm_frc;
   frclevel = ch->skill_level[FORCE_ABILITY];
   REMOVE_BIT( ch->act, PLR_KILLER );
   credits = ch->gold;
   ch->gold = 0;
   played = ch->played;
   ch->played = ch->played / 2;
   salary = ch->pcdata->salary;
   bank = ch->pcdata->bank;
   ch->pcdata->bank = 0;
   ch->pcdata->salary = 0;
   home = ch->plr_home;
   if( ch->perm_frc > 0 )
   {
      if( ch->skill_level[FORCE_ABILITY] > ( ch->perm_frc - 2 ) * 5 )
         ch->skill_level[FORCE_ABILITY] = ( ch->perm_frc - 2 ) * 5;
      ch->perm_frc = UMAX( 0, ch->perm_frc - 2 );
   }

   ch->plr_home = NULL;

   if( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
   {
      strlcpy( clanname, ch->pcdata->clan_name, MAX_STRING_LENGTH );
      STRFREE( ch->pcdata->clan_name );
      ch->pcdata->clan_name = STRALLOC( "" );
      if( ch->pcdata->bestowments )
      {
         strlcpy( bestowments, ch->pcdata->bestowments, MAX_STRING_LENGTH );
         DISPOSE( ch->pcdata->bestowments );
         ch->pcdata->bestowments = strdup( "" );
      }
      save_clone( ch );
      STRFREE( ch->pcdata->clan_name );
      ch->pcdata->clan_name = STRALLOC( clanname );
      if( bestowments[0] != '\0' )
      {
         DISPOSE( ch->pcdata->bestowments );
         ch->pcdata->bestowments = strdup( bestowments );
      }
   }
   else
      save_clone( ch );
   ch->pcdata->salary = salary;
   ch->plr_home = home;
   ch->played = played;
   ch->gold = credits;
   ch->pcdata->bank = bank;
   ch->act = flags;
   ch->perm_frc = frc;
   ch->skill_level[FORCE_ABILITY] = frclevel;

   if( !secondroom )
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 10002 ) );
   }
   else
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 1234 ) );
   }
   do_look( ch, "" );

   ch_printf( ch, "\r\n&WA small tissue sample is taken from your arm.\r\n" );
   ch_printf( ch, "&ROuch!\r\n\r\n" );
   ch_printf( ch, "&WYou have been succesfully cloned.\r\n" );

   ch->hit--;
}

void do_backup( CHAR_DATA * ch, const char *argument )
{
   long credits, bank;
   long played;
   int frclevel, frc;
   char clanname[MAX_STRING_LENGTH];
   char bestowments[MAX_STRING_LENGTH];
   int flags;
   ROOM_INDEX_DATA *home;
   bool secondroom = FALSE;

   if( IS_NPC( ch ) )
   {
      ch_printf( ch, "Yeah right!\r\n" );
      return;
   }

   if( IS_IMMORTAL( ch ) )
   {
      send_to_char( "You wish!\r\n", ch );
      return;
   }

   if( ch->in_room->vnum != 10001 && ch->in_room->vnum != 1233 )
   {
      ch_printf( ch, "You can't do that here!\r\n" );
      return;
   }

   if( !IS_DROID( ch ) )
   {
      ch_printf( ch, "Only droids need to backup their software!\r\n" );
      return;
   }

   if( ch->in_room->vnum == 1233 )
      secondroom = TRUE;

   if( ch->rppoints < 4 )
   {
      ch_printf( ch, "Backing up costs 4 RP Points. You don't have enough.\r\nHelp rppoints for more information." );
      return;
   }
   else
   {
      ch->rppoints -= 4;
      ch_printf( ch, "You are escorted into a small room.\r\n\r\n" );
   }

   if( !secondroom )
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 10000 ) );
   }
   else
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 1234 ) );
   }

   flags = ch->act;
   frc = ch->perm_frc;
   frclevel = ch->skill_level[FORCE_ABILITY];
   REMOVE_BIT( ch->act, PLR_KILLER );
   credits = ch->gold;
   ch->gold = 0;
   played = ch->played;
   ch->played = ch->played / 2;
   bank = ch->pcdata->bank;
   ch->pcdata->bank = 0;
   home = ch->plr_home;
   if( ch->perm_frc > 0 )
   {
      if( ch->skill_level[FORCE_ABILITY] > ( ch->perm_frc - 2 ) * 5 )
         ch->skill_level[FORCE_ABILITY] = ( ch->perm_frc - 2 ) * 5;
      ch->perm_frc = UMAX( 0, ch->perm_frc - 2 );
   }

   ch->plr_home = NULL;

   if( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
   {
      strlcpy( clanname, ch->pcdata->clan_name, MAX_STRING_LENGTH );
      STRFREE( ch->pcdata->clan_name );
      ch->pcdata->clan_name = STRALLOC( "" );
      strlcpy( bestowments, ch->pcdata->bestowments, MAX_STRING_LENGTH );
      DISPOSE( ch->pcdata->bestowments );
      ch->pcdata->bestowments = strdup( "" );
      save_clone( ch );
      STRFREE( ch->pcdata->clan_name );
      ch->pcdata->clan_name = STRALLOC( clanname );
      DISPOSE( ch->pcdata->bestowments );
      ch->pcdata->bestowments = strdup( clanname );
   }
   else
      save_clone( ch );
   ch->plr_home = home;
   ch->played = played;
   ch->gold = credits;
   ch->pcdata->bank = bank;
   ch->act = flags;
   ch->perm_frc = frc;
   ch->skill_level[FORCE_ABILITY] = frclevel;

   if( !secondroom )
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 10002 ) );
   }
   else
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( 1234 ) );
   }
   do_look( ch, "" );

   ch_printf( ch, "\r\n&WYou stick your head interface into a wall socket.\r\n" );
   ch_printf( ch, "&RA slight whirr in your brain feels a bit uncomfortable...\r\n\r\n" );
   ch_printf( ch, "&WYour data has been moved to a disaster recovery facility.\r\n" );

}

void do_arm( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      ch_printf( ch, "You have no idea how to do that.\r\n" );
      return;
   }

   obj = get_eq_char( ch, WEAR_HOLD );

   if( !obj || obj->item_type != ITEM_GRENADE )
   {
      ch_printf( ch, "You don't seem to be holding a grenade!\r\n" );
      return;
   }

   obj->timer = 1;
   if( obj->armed_by != NULL )
      STRFREE( obj->armed_by );
   obj->armed_by = STRALLOC( ch->name );

   ch_printf( ch, "You arm %s.\r\n", obj->short_descr );
   act( AT_PLAIN, "$n arms $p.", ch, obj, NULL, TO_ROOM );

}

void do_ammo( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *wield;
   OBJ_DATA *obj;
   bool checkammo = FALSE;
   int charge = 0;

   obj = NULL;
   wield = get_eq_char( ch, WEAR_WIELD );
   if( wield )
   {
      obj = get_eq_char( ch, WEAR_DUAL_WIELD );
      if( !obj )
         obj = get_eq_char( ch, WEAR_HOLD );
   }
   else
   {
      wield = get_eq_char( ch, WEAR_HOLD );
      obj = NULL;
   }

   if( !wield || wield->item_type != ITEM_WEAPON )
   {
      send_to_char( "&RYou don't seem to be holding a weapon.\r\n&w", ch );
      return;
   }

   if( wield->value[3] == WEAPON_BLASTER )
   {

      if( obj && obj->item_type != ITEM_AMMO )
      {
         send_to_char( "&RYour hands are too full to reload your blaster.\r\n&w", ch );
         return;
      }

      if( obj )
      {
         if( obj->value[0] > wield->value[5] )
         {
            send_to_char( "That cartridge is too big for your blaster.", ch );
            return;
         }
         unequip_char( ch, obj );
         checkammo = TRUE;
         charge = obj->value[0];
         separate_obj( obj );
         extract_obj( obj );
      }
      else
      {
         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_AMMO )
            {
               if( obj->value[0] > wield->value[5] )
               {
                  send_to_char( "That cartridge is too big for your blaster.", ch );
                  continue;
               }
               checkammo = TRUE;
               charge = obj->value[0];
               separate_obj( obj );
               extract_obj( obj );
               break;
            }
         }
      }

      if( !checkammo )
      {
         send_to_char( "&RYou don't seem to have any ammo to reload your blaster with.\r\n&w", ch );
         return;
      }

      ch_printf( ch,
                 "You replace your ammunition cartridge.\r\nYour blaster is charged with %d shots at high power to %d shots on low.\r\n",
                 charge / 5, charge );
      act( AT_PLAIN, "$n replaces the ammunition cell in $p.", ch, wield, NULL, TO_ROOM );

   }
   else if( wield->value[3] == WEAPON_BOWCASTER )
   {

      if( obj && obj->item_type != ITEM_BOLT )
      {
         send_to_char( "&RYour hands are too full to reload your bowcaster.\r\n&w", ch );
         return;
      }

      if( obj )
      {
         if( obj->value[0] > wield->value[5] )
         {
            send_to_char( "That cartridge is too big for your bowcaster.", ch );
            return;
         }
         unequip_char( ch, obj );
         checkammo = TRUE;
         charge = obj->value[0];
         separate_obj( obj );
         extract_obj( obj );
      }
      else
      {
         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_BOLT )
            {
               if( obj->value[0] > wield->value[5] )
               {
                  send_to_char( "That cartridge is too big for your bowcaster.", ch );
                  continue;
               }
               checkammo = TRUE;
               charge = obj->value[0];
               separate_obj( obj );
               extract_obj( obj );
               break;
            }
         }
      }

      if( !checkammo )
      {
         send_to_char( "&RYou don't seem to have any quarrels to reload your bowcaster with.\r\n&w", ch );
         return;
      }

      ch_printf( ch, "You replace your quarrel pack.\r\nYour bowcaster is charged with %d energy bolts.\r\n", charge );
      act( AT_PLAIN, "$n replaces the quarrels in $p.", ch, wield, NULL, TO_ROOM );

   }
   else
   {

      if( obj && obj->item_type != ITEM_BATTERY )
      {
         send_to_char( "&RYour hands are too full to replace the power cell.\r\n&w", ch );
         return;
      }

      if( obj )
      {
         unequip_char( ch, obj );
         checkammo = TRUE;
         charge = obj->value[0];
         separate_obj( obj );
         extract_obj( obj );
      }
      else
      {
         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_BATTERY )
            {
               checkammo = TRUE;
               charge = obj->value[0];
               separate_obj( obj );
               extract_obj( obj );
               break;
            }
         }
      }

      if( !checkammo )
      {
         send_to_char( "&RYou don't seem to have a power cell.\r\n&w", ch );
         return;
      }

      if( wield->value[3] == WEAPON_LIGHTSABER )
      {
         ch_printf( ch, "You replace your power cell.\r\nYour lightsaber is charged to %d/%d units.\r\n", charge, charge );
         act( AT_PLAIN, "$n replaces the power cell in $p.", ch, wield, NULL, TO_ROOM );
         act( AT_PLAIN, "$p ignites with a bright glow.", ch, wield, NULL, TO_ROOM );
      }
      else if( wield->value[3] == WEAPON_VIBRO_BLADE )
      {
         ch_printf( ch, "You replace your power cell.\r\nYour vibro-blade is charged to %d/%d units.\r\n", charge, charge );
         act( AT_PLAIN, "$n replaces the power cell in $p.", ch, wield, NULL, TO_ROOM );
      }
      else if( wield->value[3] == WEAPON_FORCE_PIKE )
      {
         ch_printf( ch, "You replace your power cell.\r\nYour force-pike is charged to %d/%d units.\r\n", charge, charge );
         act( AT_PLAIN, "$n replaces the power cell in $p.", ch, wield, NULL, TO_ROOM );
      }
      else
      {
         ch_printf( ch, "You feel very foolish.\r\n" );
         act( AT_PLAIN, "$n tries to jam a power cell into $p.", ch, wield, NULL, TO_ROOM );
      }
   }

   wield->value[4] = charge;

}

void do_setblaster( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *wield;
   OBJ_DATA *wield2;

   wield = get_eq_char( ch, WEAR_WIELD );
   if( wield && !( wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_BLASTER ) )
      wield = NULL;
   wield2 = get_eq_char( ch, WEAR_DUAL_WIELD );
   if( wield2 && !( wield2->item_type == ITEM_WEAPON && wield2->value[3] == WEAPON_BLASTER ) )
      wield2 = NULL;

   if( !wield && !wield2 )
   {
      send_to_char( "&RYou don't seem to be wielding a blaster.\r\n&w", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "&RUsage: setblaster <full|high|normal|half|low|stun>\r\n&w", ch );
      return;
   }

   if( wield )
      act( AT_PLAIN, "$n adjusts the settings on $p.", ch, wield, NULL, TO_ROOM );

   if( wield2 )
      act( AT_PLAIN, "$n adjusts the settings on $p.", ch, wield2, NULL, TO_ROOM );

   if( !str_cmp( argument, "full" ) )
   {
      if( wield )
      {
         wield->blaster_setting = BLASTER_FULL;
         send_to_char( "&YWielded blaster set to FULL Power\r\n&w", ch );
      }
      if( wield2 )
      {
         wield2->blaster_setting = BLASTER_FULL;
         send_to_char( "&YDual wielded blaster set to FULL Power\r\n&w", ch );
      }
      return;
   }
   if( !str_cmp( argument, "high" ) )
   {
      if( wield )
      {
         wield->blaster_setting = BLASTER_HIGH;
         send_to_char( "&YWielded blaster set to HIGH Power\r\n&w", ch );
      }
      if( wield2 )
      {
         wield2->blaster_setting = BLASTER_HIGH;
         send_to_char( "&YDual wielded blaster set to HIGH Power\r\n&w", ch );
      }
      return;
   }
   if( !str_cmp( argument, "normal" ) )
   {
      if( wield )
      {
         wield->blaster_setting = BLASTER_NORMAL;
         send_to_char( "&YWielded blaster set to NORMAL Power\r\n&w", ch );
      }
      if( wield2 )
      {
         wield2->blaster_setting = BLASTER_NORMAL;
         send_to_char( "&YDual wielded blaster set to NORMAL Power\r\n&w", ch );
      }
      return;
   }
   if( !str_cmp( argument, "half" ) )
   {
      if( wield )
      {
         wield->blaster_setting = BLASTER_HALF;
         send_to_char( "&YWielded blaster set to HALF Power\r\n&w", ch );
      }
      if( wield2 )
      {
         wield2->blaster_setting = BLASTER_HALF;
         send_to_char( "&YDual wielded blaster set to HALF Power\r\n&w", ch );
      }
      return;
   }
   if( !str_cmp( argument, "low" ) )
   {
      if( wield )
      {
         wield->blaster_setting = BLASTER_LOW;
         send_to_char( "&YWielded blaster set to LOW Power\r\n&w", ch );
      }
      if( wield2 )
      {
         wield2->blaster_setting = BLASTER_LOW;
         send_to_char( "&YDual wielded blaster set to LOW Power\r\n&w", ch );
      }
      return;
   }
   if( !str_cmp( argument, "stun" ) )
   {
      if( wield )
      {
         wield->blaster_setting = BLASTER_STUN;
         send_to_char( "&YWielded blaster set to STUN\r\n&w", ch );
      }
      if( wield2 )
      {
         wield2->blaster_setting = BLASTER_STUN;
         send_to_char( "&YDual wielded blaster set to STUN\r\n&w", ch );
      }
      return;
   }
   else
      do_setblaster( ch, "" );

}

void do_use( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char argd[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *device;
   OBJ_DATA *obj;
   ch_ret retcode;

   argument = one_argument( argument, argd );
   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "on" ) )
      argument = one_argument( argument, arg );

   if( argd[0] == '\0' )
   {
      send_to_char( "Use what?\r\n", ch );
      return;
   }

   if( ( device = get_eq_char( ch, WEAR_HOLD ) ) == NULL || !nifty_is_name( argd, device->name ) )
   {
      do_takedrug( ch, argd );
      return;
   }

   if( device->item_type == ITEM_SPICE )
   {
      do_takedrug( ch, argd );
      return;
   }

   if( device->item_type != ITEM_DEVICE )
   {
      send_to_char( "You can't figure out what it is your supposed to do with it.\r\n", ch );
      return;
   }

   if( device->value[2] <= 0 )
   {
      send_to_char( "It has no more charge left.", ch );
      return;
   }

   obj = NULL;
   if( arg[0] == '\0' )
   {
      if( ch->fighting )
      {
         victim = who_fighting( ch );
      }
      else
      {
         send_to_char( "Use on whom or what?\r\n", ch );
         return;
      }
   }
   else
   {
      if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
      {
         send_to_char( "You can't find your target.\r\n", ch );
         return;
      }
   }

   WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

   if( device->value[2] > 0 )
   {
      device->value[2]--;
      if( victim )
      {
         if( !oprog_use_trigger( ch, device, victim, NULL, NULL ) )
         {
            act( AT_MAGIC, "$n uses $p on $N.", ch, device, victim, TO_ROOM );
            act( AT_MAGIC, "You use $p on $N.", ch, device, victim, TO_CHAR );
         }
      }
      else
      {
         if( !oprog_use_trigger( ch, device, NULL, obj, NULL ) )
         {
            act( AT_MAGIC, "$n uses $p on $P.", ch, device, obj, TO_ROOM );
            act( AT_MAGIC, "You use $p on $P.", ch, device, obj, TO_CHAR );
         }
      }

      retcode = obj_cast_spell( device->value[3], device->value[0], ch, victim, obj );
      if( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
      {
         bug( "%s: char died", __func__ );
         return;
      }
   }
   return;
}

void do_takedrug( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;
   AFFECT_DATA af;
   int drug;
   int sn = 0;

   if( argument[0] == '\0' || !str_cmp( argument, "" ) )
   {
      send_to_char( "Use what?\r\n", ch );
      return;
   }

   if( ( obj = find_obj( ch, argument, TRUE ) ) == NULL )
      return;

   if( obj->item_type == ITEM_DEVICE )
   {
      send_to_char( "Try holding it first.\r\n", ch );
      return;
   }

   if( obj->item_type != ITEM_SPICE )
   {
      act( AT_ACTION, "$n looks at $p and scratches $s head.", ch, obj, NULL, TO_ROOM );
      act( AT_ACTION, "You can't quite figure out what to do with $p.", ch, obj, NULL, TO_CHAR );
      return;
   }

   separate_obj( obj );
   if( obj->in_obj )
   {
      act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
      act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
   }

   if( ch->fighting && number_percent(  ) > ( get_curr_dex( ch ) * 2 + 48 ) )
   {
      act( AT_MAGIC, "$n accidentally drops $p rendering it useless.", ch, obj, NULL, TO_ROOM );
      act( AT_MAGIC, "Oops... $p gets knocked from your hands rendering it completely useless!", ch, obj, NULL, TO_CHAR );
   }
   else
   {
      if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
      {
         act( AT_ACTION, "$n takes $p.", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You take $p.", ch, obj, NULL, TO_CHAR );
      }

      if( IS_NPC( ch ) )
      {
         extract_obj( obj );
         return;
      }

      drug = obj->value[0];

      WAIT_STATE( ch, PULSE_PER_SECOND / 4 );

      gain_condition( ch, COND_THIRST, 1 );

      ch->pcdata->drug_level[drug] = UMIN( ch->pcdata->drug_level[drug] + obj->value[1], 255 );
      if( ch->pcdata->drug_level[drug] >= 255 || ch->pcdata->drug_level[drug] > ( ch->pcdata->addiction[drug] + 100 ) )
      {
         act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
         act( AT_POISON, "You feel sick. You may have taken too much.", ch, NULL, NULL, TO_CHAR );
         ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
         af.type = gsn_poison;
         af.location = APPLY_INT;
         af.modifier = -5;
         af.duration = ch->pcdata->drug_level[drug];
         af.bitvector = AFF_POISON;
         affect_to_char( ch, &af );
         ch->hit = 1;
      }

      switch ( drug )
      {
         default:
         case SPICE_GLITTERSTIM:

            sn = skill_lookup( "true sight" );
            if( sn < MAX_SKILL && !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
            {
               af.type = sn;
               af.location = APPLY_AC;
               af.modifier = -10;
               af.duration = URANGE( 1, ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug], obj->value[1] );
               af.bitvector = AFF_TRUESIGHT;
               affect_to_char( ch, &af );
            }
            break;

         case SPICE_CARSANUM:

            sn = skill_lookup( "sanctuary" );
            if( sn < MAX_SKILL && !IS_AFFECTED( ch, AFF_SANCTUARY ) )
            {
               af.type = sn;
               af.location = APPLY_NONE;
               af.modifier = 0;
               af.duration = URANGE( 1, ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug], obj->value[1] );
               af.bitvector = AFF_SANCTUARY;
               affect_to_char( ch, &af );
            }
            break;

         case SPICE_RYLL:

            af.type = -1;
            af.location = APPLY_DEX;
            af.modifier = 1;
            af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
            af.bitvector = AFF_NONE;
            affect_to_char( ch, &af );

            af.type = -1;
            af.location = APPLY_HITROLL;
            af.modifier = 1;
            af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
            af.bitvector = AFF_NONE;
            affect_to_char( ch, &af );

            break;

         case SPICE_ANDRIS:

            af.type = -1;
            af.location = APPLY_HIT;
            af.modifier = 10;
            af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
            af.bitvector = AFF_NONE;
            affect_to_char( ch, &af );

            af.type = sn;
            af.location = APPLY_CON;
            af.modifier = 1;
            af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
            af.bitvector = AFF_NONE;
            affect_to_char( ch, &af );

            break;

      }

   }
   if( cur_obj == obj->serial )
      global_objcode = rOBJ_EATEN;
   extract_obj( obj );
   return;
}

void jedi_bonus( CHAR_DATA * ch )
{
   if( number_range( 1, 100 ) == 1 )
   {
      ch->max_mana++;
      send_to_char( "&YYou are wise in your use of the force.\r\n", ch );
      send_to_char( "You feel a little stronger in your wisdom.&w\r\n", ch );
   }
}

void sith_penalty( CHAR_DATA * ch )
{
   if( number_range( 1, 100 ) == 1 )
   {
      ch->max_mana++;
      if( ch->max_hit > 100 )
         ch->max_hit--;
      ch->hit--;
      send_to_char( "&zYour body grows weaker as your strength in the dark side grows.&w\r\n", ch );
   }
}

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
void do_fill( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   OBJ_DATA *source;
   short dest_item, src_item1, src_item2, src_item3, src_item4;
   int diff;
   bool all = FALSE;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   /*
    * munch optional words 
    */
   if( ( !str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ) ) && argument[0] != '\0' )
      argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Fill what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that item.\r\n", ch );
      return;
   }
   else
      dest_item = obj->item_type;

   src_item1 = src_item2 = src_item3 = src_item4 = -1;
   switch ( dest_item )
   {
      default:
         act( AT_ACTION, "$n tries to fill $p... (Don't ask me how)", ch, obj, NULL, TO_ROOM );
         send_to_char( "You cannot fill that.\r\n", ch );
         return;
         /*
          * place all fillable item types here 
          */
      case ITEM_DRINK_CON:
         src_item1 = ITEM_FOUNTAIN;
         src_item2 = ITEM_BLOOD;
         break;
      case ITEM_HERB_CON:
         src_item1 = ITEM_HERB;
         src_item2 = ITEM_HERB_CON;
         break;
      case ITEM_PIPE:
         src_item1 = ITEM_HERB;
         src_item2 = ITEM_HERB_CON;
         break;
      case ITEM_CONTAINER:
         src_item1 = ITEM_CONTAINER;
         src_item2 = ITEM_CORPSE_NPC;
         src_item3 = ITEM_CORPSE_PC;
         src_item4 = ITEM_CORPSE_NPC;
         break;
   }

   if( dest_item == ITEM_CONTAINER )
   {
      if( IS_SET( obj->value[1], CONT_CLOSED ) )
      {
         act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
         return;
      }
      if( get_obj_weight( obj ) / obj->count >= obj->value[0] )
      {
         send_to_char( "It's already full as it can be.\r\n", ch );
         return;
      }
   }
   else
   {
      diff = obj->value[0] - obj->value[1];
      if( diff < 1 || obj->value[1] >= obj->value[0] )
      {
         send_to_char( "It's already full as it can be.\r\n", ch );
         return;
      }
   }

   if( dest_item == ITEM_PIPE && IS_SET( obj->value[3], PIPE_FULLOFASH ) )
   {
      send_to_char( "It's full of ashes, and needs to be emptied first.\r\n", ch );
      return;
   }

   if( arg2[0] != '\0' )
   {
      if( dest_item == ITEM_CONTAINER && ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) ) )
      {
         all = TRUE;
         source = NULL;
      }
      else
         /*
          * This used to let you fill a pipe from an object on the ground.  Seems
          * to me you should be holding whatever you want to fill a pipe with.
          * It's nitpicking, but I needed to change it to get a mobprog to work
          * right.  Check out Lord Fitzgibbon if you're curious.  -Narn 
          */
      if( dest_item == ITEM_PIPE )
      {
         if( ( source = get_obj_carry( ch, arg2 ) ) == NULL )
         {
            send_to_char( "You don't have that item.\r\n", ch );
            return;
         }
         if( source->item_type != src_item1 && source->item_type != src_item2
             && source->item_type != src_item3 && source->item_type != src_item4 )
         {
            act( AT_PLAIN, "You cannot fill $p with $P!", ch, obj, source, TO_CHAR );
            return;
         }
      }
      else
      {
         if( ( source = get_obj_here( ch, arg2 ) ) == NULL )
         {
            send_to_char( "You cannot find that item.\r\n", ch );
            return;
         }
      }
   }
   else
      source = NULL;

   if( !source && dest_item == ITEM_PIPE )
   {
      send_to_char( "Fill it with what?\r\n", ch );
      return;
   }

   if( !source )
   {
      bool found = FALSE;
      OBJ_DATA *src_next;

      found = FALSE;
      separate_obj( obj );
      for( source = ch->in_room->first_content; source; source = src_next )
      {
         src_next = source->next_content;
         if( dest_item == ITEM_CONTAINER )
         {
            if( !CAN_WEAR( source, ITEM_TAKE )
                || ( IS_OBJ_STAT( source, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
                || ch->carry_weight + get_obj_weight( source ) > can_carry_w( ch )
                || ( get_obj_weight( source ) + get_obj_weight( obj ) / obj->count ) > obj->value[0] )
               continue;
            if( all && arg2[3] == '.' && !nifty_is_name( &arg2[4], source->name ) )
               continue;
            obj_from_room( source );
            if( source->item_type == ITEM_MONEY )
            {
               ch->gold += source->value[0];
               extract_obj( source );
            }
            else
               obj_to_obj( source, obj );
            found = TRUE;
         }
         else
            if( source->item_type == src_item1
                || source->item_type == src_item2 || source->item_type == src_item3 || source->item_type == src_item4 )
         {
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         switch ( src_item1 )
         {
            default:
               send_to_char( "There is nothing appropriate here!\r\n", ch );
               return;
            case ITEM_FOUNTAIN:
               send_to_char( "There is no fountain or pool here!\r\n", ch );
               return;
            case ITEM_BLOOD:
               send_to_char( "There is no blood pool here!\r\n", ch );
               return;
            case ITEM_HERB_CON:
               send_to_char( "There are no herbs here!\r\n", ch );
               return;
            case ITEM_HERB:
               send_to_char( "You cannot find any smoking herbs.\r\n", ch );
               return;
         }
      }
      if( dest_item == ITEM_CONTAINER )
      {
         act( AT_ACTION, "You fill $p.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n fills $p.", ch, obj, NULL, TO_ROOM );
         return;
      }
   }

   if( dest_item == ITEM_CONTAINER )
   {
      OBJ_DATA *otmp, *otmp_next;
      char name[MAX_INPUT_LENGTH];
      CHAR_DATA *gch;
      char *pd;
      bool found = FALSE;

      if( source == obj )
      {
         send_to_char( "You can't fill something with itself!\r\n", ch );
         return;
      }

      switch ( source->item_type )
      {
         default:   /* put something in container */
            if( !source->in_room /* disallow inventory items */
                || !CAN_WEAR( source, ITEM_TAKE )
                || ( IS_OBJ_STAT( source, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
                || ch->carry_weight + get_obj_weight( source ) > can_carry_w( ch )
                || ( get_obj_weight( source ) + get_obj_weight( obj ) / obj->count ) > obj->value[0] )
            {
               send_to_char( "You can't do that.\r\n", ch );
               return;
            }
            separate_obj( obj );
            act( AT_ACTION, "You take $P and put it inside $p.", ch, obj, source, TO_CHAR );
            act( AT_ACTION, "$n takes $P and puts it inside $p.", ch, obj, source, TO_ROOM );
            obj_from_room( source );
            obj_to_obj( source, obj );
            break;
         case ITEM_MONEY:
            send_to_char( "You can't do that... yet.\r\n", ch );
            break;
         case ITEM_CORPSE_PC:
            if( IS_NPC( ch ) )
            {
               send_to_char( "You can't do that.\r\n", ch );
               return;
            }

            pd = source->short_descr;
            pd = one_argument( pd, name );
            pd = one_argument( pd, name );
            pd = one_argument( pd, name );
            pd = one_argument( pd, name );

            if( str_cmp( name, ch->name ) && !IS_IMMORTAL( ch ) )
            {
               bool fGroup;

               fGroup = FALSE;
               for( gch = first_char; gch; gch = gch->next )
               {
                  if( !IS_NPC( gch ) && is_same_group( ch, gch ) && !str_cmp( name, gch->name ) )
                  {
                     fGroup = TRUE;
                     break;
                  }
               }
               if( !fGroup )
               {
                  send_to_char( "That's someone else's corpse.\r\n", ch );
                  return;
               }
            }

         case ITEM_CONTAINER:
            if( source->item_type == ITEM_CONTAINER   /* don't remove */
                && IS_SET( source->value[1], CONT_CLOSED ) )
            {
               act( AT_PLAIN, "The $d is closed.", ch, NULL, source->name, TO_CHAR );
               return;
            }
         case ITEM_DROID_CORPSE:
         case ITEM_CORPSE_NPC:
            if( ( otmp = source->first_content ) == NULL )
            {
               send_to_char( "It's empty.\r\n", ch );
               return;
            }
            separate_obj( obj );
            for( ; otmp; otmp = otmp_next )
            {
               otmp_next = otmp->next_content;

               if( !CAN_WEAR( otmp, ITEM_TAKE )
                   || ( IS_OBJ_STAT( otmp, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
                   || ch->carry_number + otmp->count > can_carry_n( ch )
                   || ch->carry_weight + get_obj_weight( otmp ) > can_carry_w( ch )
                   || ( get_obj_weight( source ) + get_obj_weight( obj ) / obj->count ) > obj->value[0] )
                  continue;
               obj_from_obj( otmp );
               obj_to_obj( otmp, obj );
               found = TRUE;
            }
            if( found )
            {
               act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
               act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
            }
            else
               send_to_char( "There is nothing appropriate in there.\r\n", ch );
            break;
      }
      return;
   }

   if( source->value[1] < 1 )
   {
      send_to_char( "There's none left!\r\n", ch );
      return;
   }
   if( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
      separate_obj( source );
   separate_obj( obj );

   switch ( source->item_type )
   {
      default:
         bug( "%s: got bad item type: %d", __func__, source->item_type );
         send_to_char( "Something went wrong...\r\n", ch );
         return;
      case ITEM_FOUNTAIN:
         if( obj->value[1] != 0 && obj->value[2] != 0 )
         {
            send_to_char( "There is already another liquid in it.\r\n", ch );
            return;
         }
         obj->value[2] = 0;
         obj->value[1] = obj->value[0];
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         return;
      case ITEM_BLOOD:
         if( obj->value[1] != 0 && obj->value[2] != 13 )
         {
            send_to_char( "There is already another liquid in it.\r\n", ch );
            return;
         }
         obj->value[2] = 13;
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         if( ( source->value[1] -= diff ) < 1 )
         {
            extract_obj( source );
            make_bloodstain( ch );
         }
         return;
      case ITEM_HERB:
         if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
         {
            send_to_char( "There is already another type of herb in it.\r\n", ch );
            return;
         }
         obj->value[2] = source->value[2];
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         act( AT_ACTION, "You fill $p with $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p with $P.", ch, obj, source, TO_ROOM );
         if( ( source->value[1] -= diff ) < 1 )
            extract_obj( source );
         return;
      case ITEM_HERB_CON:
         if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
         {
            send_to_char( "There is already another type of herb in it.\r\n", ch );
            return;
         }
         obj->value[2] = source->value[2];
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         source->value[1] -= diff;
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         return;
      case ITEM_DRINK_CON:
         if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
         {
            send_to_char( "There is already another liquid in it.\r\n", ch );
            return;
         }
         obj->value[2] = source->value[2];
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         source->value[1] -= diff;
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         return;
   }
}

void do_drink( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int amount;
   int liquid;

   argument = one_argument( argument, arg );
   /*
    * munch optional words 
    */
   if( !str_cmp( arg, "from" ) && argument[0] != '\0' )
      argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
         if( ( obj->item_type == ITEM_FOUNTAIN ) || ( obj->item_type == ITEM_BLOOD ) )
            break;

      if( !obj )
      {
         send_to_char( "Drink what?\r\n", ch );
         return;
      }
   }
   else
   {
      if( ( obj = get_obj_here( ch, arg ) ) == NULL )
      {
         send_to_char( "You can't find it.\r\n", ch );
         return;
      }
   }

   if( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
      separate_obj( obj );

   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 40 )
   {
      send_to_char( "You fail to reach your mouth.  *Hic*\r\n", ch );
      return;
   }

   switch ( obj->item_type )
   {
      default:
         if( obj->carried_by == ch )
         {
            act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
            act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
         }
         else
         {
            act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
            act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
         }
         break;

      case ITEM_POTION:
         if( obj->carried_by == ch )
            do_quaff( ch, obj->name );
         else
            send_to_char( "You're not carrying that.\r\n", ch );
         break;

      case ITEM_FOUNTAIN:
         if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
         {
            act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You take a long thirst quenching drink.\r\n", ch );
         }

         if( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_THIRST] = 40;
         break;

      case ITEM_DRINK_CON:
         if( obj->value[1] <= 0 )
         {
            send_to_char( "It is already empty.\r\n", ch );
            return;
         }

         if( ( liquid = obj->value[2] ) >= LIQ_MAX )
         {
            bug( "%s: bad liquid number %d.", __func__, liquid );
            liquid = obj->value[2] = 0;
         }

         if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
         {
            act( AT_ACTION, "$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM );
            act( AT_ACTION, "You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR );
         }

         amount = 1; /* UMIN(amount, obj->value[1]); */
         /*
          * what was this? concentrated drinks?  concentrated water
          * too I suppose... sheesh! 
          */

         gain_condition( ch, COND_DRUNK, amount * liq_table[liquid].liq_affect[COND_DRUNK] );
         gain_condition( ch, COND_FULL, amount * liq_table[liquid].liq_affect[COND_FULL] );
         gain_condition( ch, COND_THIRST, amount * liq_table[liquid].liq_affect[COND_THIRST] );

         if( !IS_NPC( ch ) )
         {
            if( ch->pcdata->condition[COND_DRUNK] > 24 )
               send_to_char( "You feel quite sloshed.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] > 18 )
               send_to_char( "You feel very drunk.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] > 12 )
               send_to_char( "You feel drunk.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] > 8 )
               send_to_char( "You feel a little drunk.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] > 5 )
               send_to_char( "You feel light headed.\r\n", ch );

            if( ch->pcdata->condition[COND_FULL] > 40 )
               send_to_char( "You are full.\r\n", ch );

            if( ch->pcdata->condition[COND_THIRST] > 40 )
               send_to_char( "You feel bloated.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] > 36 )
               send_to_char( "Your stomach is sloshing around.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] > 30 )
               send_to_char( "You do not feel thirsty.\r\n", ch );
         }

         if( obj->value[3] )
         {
            /*
             * The drink was poisoned! 
             */
            AFFECT_DATA af;

            act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
            act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
            af.type = gsn_poison;
            af.duration = 3 * obj->value[3];
            af.location = APPLY_NONE;
            af.modifier = 0;
            af.bitvector = AFF_POISON;
            affect_join( ch, &af );
         }

         obj->value[1] -= amount;
         break;
   }
   WAIT_STATE( ch, PULSE_PER_SECOND );
   return;
}

void do_eat( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;
   ch_ret retcode;
   int foodcond;

   if( argument[0] == '\0' )
   {
      send_to_char( "Eat what?\r\n", ch );
      return;
   }

   if( IS_NPC( ch ) || ch->pcdata->condition[COND_FULL] > 5 )
      if( ms_find_obj( ch ) )
         return;

   if( ( obj = find_obj( ch, argument, TRUE ) ) == NULL )
      return;

   if( !IS_IMMORTAL( ch ) )
   {
      if( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
      {
         act( AT_ACTION, "$n starts to nibble on $p... ($e must really be hungry)", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You try to nibble on $p...", ch, obj, NULL, TO_CHAR );
         return;
      }

      if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
      {
         send_to_char( "You are too full to eat more.\r\n", ch );
         return;
      }
   }

   /*
    * required due to object grouping 
    */
   separate_obj( obj );

   WAIT_STATE( ch, PULSE_PER_SECOND / 2 );

   if( obj->in_obj )
   {
      act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
      act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
   }
   if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
   {
      if( !obj->action_desc || obj->action_desc[0] == '\0' )
      {
         act( AT_ACTION, "$n eats $p.", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR );
      }
      else
         actiondesc( ch, obj, NULL );
   }

   switch ( obj->item_type )
   {

      case ITEM_FOOD:
         if( obj->timer > 0 && obj->value[1] > 0 )
            foodcond = ( obj->timer * 10 ) / obj->value[1];
         else
            foodcond = 10;

         if( !IS_NPC( ch ) )
         {
            int condition;

            condition = ch->pcdata->condition[COND_FULL];
            gain_condition( ch, COND_FULL, ( obj->value[0] * foodcond ) / 10 );
            if( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
               send_to_char( "You are no longer hungry.\r\n", ch );
            else if( ch->pcdata->condition[COND_FULL] > 40 )
               send_to_char( "You are full.\r\n", ch );
         }

         if( obj->value[3] != 0 || ( foodcond < 4 && number_range( 0, foodcond + 1 ) == 0 ) )
         {
            /*
             * The food was poisoned! 
             */
            AFFECT_DATA af;

            if( obj->value[3] != 0 )
            {
               act( AT_POISON, "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
               act( AT_POISON, "You choke and gag.", ch, NULL, NULL, TO_CHAR );
               ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
            }
            else
            {
               act( AT_POISON, "$n gags on $p.", ch, obj, NULL, TO_ROOM );
               act( AT_POISON, "You gag on $p.", ch, obj, NULL, TO_CHAR );
               ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
            }

            af.type = gsn_poison;
            af.duration = 2 * obj->value[0] * ( obj->value[3] > 0 ? obj->value[3] : 1 );
            af.location = APPLY_NONE;
            af.modifier = 0;
            af.bitvector = AFF_POISON;
            affect_join( ch, &af );
         }
         break;

      case ITEM_PILL:
         /*
          * allow pills to fill you, if so desired 
          */
         if( !IS_NPC( ch ) && obj->value[4] )
         {
            int condition;

            condition = ch->pcdata->condition[COND_FULL];
            gain_condition( ch, COND_FULL, obj->value[4] );
            if( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
               send_to_char( "You are no longer hungry.\r\n", ch );
            else if( ch->pcdata->condition[COND_FULL] > 40 )
               send_to_char( "You are full.\r\n", ch );
         }
         retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
         if( retcode == rNONE )
            retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
         if( retcode == rNONE )
            retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
         break;
   }

   if( obj->serial == cur_obj )
      global_objcode = rOBJ_EATEN;
   extract_obj( obj );
   return;
}

void do_quaff( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;
   ch_ret retcode;

   if( argument[0] == '\0' || !str_cmp( argument, "" ) )
   {
      send_to_char( "Quaff what?\r\n", ch );
      return;
   }

   if( ( obj = find_obj( ch, argument, TRUE ) ) == NULL )
      return;

   if( obj->item_type != ITEM_POTION )
   {
      if( obj->item_type == ITEM_DRINK_CON )
         do_drink( ch, obj->name );
      else
      {
         act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
      }
      return;
   }

   /*
    * Fullness checking               -Thoric
    */
   if( !IS_NPC( ch ) && ( ch->pcdata->condition[COND_FULL] >= 48 || ch->pcdata->condition[COND_THIRST] >= 48 ) )
   {
      send_to_char( "Your stomach cannot contain any more.\r\n", ch );
      return;
   }

   separate_obj( obj );
   if( obj->in_obj )
   {
      act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
      act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
   }

   /*
    * If fighting, chance of dropping potion         -Thoric
    */
   if( ch->fighting && number_percent(  ) > ( get_curr_dex( ch ) * 2 + 48 ) )
   {
      act( AT_MAGIC, "$n accidentally drops $p and it smashes into a thousand fragments.", ch, obj, NULL, TO_ROOM );
      act( AT_MAGIC, "Oops... $p gets knocked from your hands and smashes into pieces!", ch, obj, NULL, TO_CHAR );
   }
   else
   {
      if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
      {
         act( AT_ACTION, "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You quaff $p.", ch, obj, NULL, TO_CHAR );
      }

      WAIT_STATE( ch, PULSE_PER_SECOND / 4 );

      gain_condition( ch, COND_THIRST, 1 );
      retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
      if( retcode == rNONE )
         retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
      if( retcode == rNONE )
         retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
   }
   if( cur_obj == obj->serial )
      global_objcode = rOBJ_QUAFFED;
   extract_obj( obj );
   return;
}


void do_recite( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *scroll;
   OBJ_DATA *obj;
   ch_ret retcode;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Activate what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that item.\r\n", ch );
      return;
   }

   if( scroll->item_type != ITEM_SCROLL )
   {
      act( AT_ACTION, "$n attempts to activate $p ... the silly fool.", ch, scroll, NULL, TO_ROOM );
      act( AT_ACTION, "You try to activate $p. (Now what?)", ch, scroll, NULL, TO_CHAR );
      return;
   }

   if( IS_NPC( ch ) && ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) )
   {
      send_to_char( "As a mob, this dialect is foreign to you.\r\n", ch );
      return;
   }

   if( ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) && ( ch->top_level + 10 < scroll->value[0] ) )
   {
      send_to_char( "This item is too complex for you to understand.\r\n", ch );
      return;
   }

   obj = NULL;
   if( arg2[0] == '\0' )
      victim = ch;
   else
   {
      if( ( victim = get_char_room( ch, arg2 ) ) == NULL && ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You can't find it.\r\n", ch );
         return;
      }
   }

   separate_obj( scroll );
   act( AT_MAGIC, "$n activate $p.", ch, scroll, NULL, TO_ROOM );
   act( AT_MAGIC, "You activate $p.", ch, scroll, NULL, TO_CHAR );


   WAIT_STATE( ch, PULSE_PER_SECOND / 2 );

   retcode = obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
   if( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
   if( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );

   if( scroll->serial == cur_obj )
      global_objcode = rOBJ_USED;
   extract_obj( scroll );
   return;
}

/*
 * Function to handle the state changing of a triggerobject (lever)  -Thoric
 */
void pullorpush( CHAR_DATA * ch, OBJ_DATA * obj, bool pull )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *rch;
   bool isup;
   ROOM_INDEX_DATA *room, *to_room = NULL;
   EXIT_DATA *pexit, *pexit_rev;
   int edir;
   const char *txt;

   if( IS_SET( obj->value[0], TRIG_UP ) )
      isup = TRUE;
   else
      isup = FALSE;
   switch ( obj->item_type )
   {
      default:
         ch_printf( ch, "You can't %s that!\r\n", pull ? "pull" : "push" );
         return;
         break;
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
         if( ( !pull && isup ) || ( pull && !isup ) )
         {
            ch_printf( ch, "It is already %s.\r\n", isup ? "up" : "down" );
            return;
         }
      case ITEM_BUTTON:
         if( ( !pull && isup ) || ( pull & !isup ) )
         {
            ch_printf( ch, "It is already %s.\r\n", isup ? "in" : "out" );
            return;
         }
         break;
   }
   if( ( pull ) && IS_SET( obj->pIndexData->progtypes, PULL_PROG ) )
   {
      if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
         REMOVE_BIT( obj->value[0], TRIG_UP );
      oprog_pull_trigger( ch, obj );
      return;
   }
   if( ( !pull ) && IS_SET( obj->pIndexData->progtypes, PUSH_PROG ) )
   {
      if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
         SET_BIT( obj->value[0], TRIG_UP );
      oprog_push_trigger( ch, obj );
      return;
   }

   if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "$n %s $p.", pull ? "pulls" : "pushes" );
      act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
      snprintf( buf, MAX_STRING_LENGTH, "You %s $p.", pull ? "pull" : "push" );
      act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
   }

   if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
   {
      if( pull )
         REMOVE_BIT( obj->value[0], TRIG_UP );
      else
         SET_BIT( obj->value[0], TRIG_UP );
   }

   if( IS_SET( obj->value[0], TRIG_TELEPORT )
       || IS_SET( obj->value[0], TRIG_TELEPORTALL ) || IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
   {
      int flags;

      if( ( room = get_room_index( obj->value[1] ) ) == NULL )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }
      flags = 0;
      if( IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) )
         SET_BIT( flags, TELE_SHOWDESC );
      if( IS_SET( obj->value[0], TRIG_TELEPORTALL ) )
         SET_BIT( flags, TELE_TRANSALL );
      if( IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
         SET_BIT( flags, TELE_TRANSALLPLUS );

      teleport( ch, obj->value[1], flags );
      return;
   }

   if( IS_SET( obj->value[0], TRIG_RAND4 ) || IS_SET( obj->value[0], TRIG_RAND6 ) )
   {
      int maxd;

      if( ( room = get_room_index( obj->value[1] ) ) == NULL )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }

      if( IS_SET( obj->value[0], TRIG_RAND4 ) )
         maxd = 3;
      else
         maxd = 5;

      randomize_exits( room, maxd );
      for( rch = room->first_person; rch; rch = rch->next_in_room )
      {
         send_to_char( "You hear a loud rumbling sound.\r\n", rch );
         send_to_char( "Something seems different...\r\n", rch );
      }
   }

   /*
    * Death support added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_DEATH ) )
   {
      /*
       * Should we really send a message to the room? 
       */
      act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
      act( AT_DEAD, "Oopsie... you're dead!\r\n", ch, NULL, NULL, TO_CHAR );
      snprintf( buf, MAX_STRING_LENGTH, "%s hit a DEATH TRIGGER in room %d!", ch->name, ch->in_room->vnum );
      log_string( buf );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );

      /*
       * Personaly I fiqured if we wanted it to be a full DT we could just have it send them into a DT. 
       */
      set_cur_char( ch );
      raw_kill( ch, ch );

      /*
       * If you want it to be more like a room deathtrap use this instead 
       */
/*
      if( is_npc( ch ) )
         extract_char( ch, true );
      else
         extract_char( ch, false );
*/
      return;
   }

   /*
    * Object loading added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_OLOAD ) )
   {
      OBJ_INDEX_DATA *pObjIndex;
      OBJ_DATA *tobj;

      /*
       * value[1] for the obj vnum 
       */
      if( !( pObjIndex = get_obj_index( obj->value[1] ) ) )
      {
         bug( "%s: obj points to invalid object vnum %d", __func__, obj->value[1] );
         return;
      }
      /*
       * Set room to NULL before the check 
       */
      room = NULL;
      /*
       * value[2] for the room vnum to put the object in if there is one, 0 for giving it to char or current room 
       */
      if( obj->value[2] > 0 && !( room = get_room_index( obj->value[2] ) ) )
      {
         bug( "%s: obj points to invalid room vnum %d", __func__, obj->value[2] );
         return;
      }
      /*
       * Uses value[3] for level 
       */
      if( !( tobj = create_object( pObjIndex, URANGE( 0, obj->value[3], MAX_LEVEL ) ) ) )
      {
         bug( "%s: obj couldnt create_obj vnum %d at level %d", __func__, obj->value[1], obj->value[3] );
         return;
      }
      if( room )
         obj_to_room( tobj, room );
      else
      {
         if( CAN_WEAR( obj, ITEM_TAKE ) )
            obj_to_char( tobj, ch );
         else
            obj_to_room( tobj, ch->in_room );
      }
      return;
   }

   /*
    * Mob loading added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_MLOAD ) )
   {
      MOB_INDEX_DATA *pMobIndex;
      CHAR_DATA *mob;

      /*
       * value[1] for the obj vnum 
       */
      if( !( pMobIndex = get_mob_index( obj->value[1] ) ) )
      {
         bug( "%s: obj points to invalid mob vnum %d", __func__, obj->value[1] );
         return;
      }
      /*
       * Set room to current room before the check 
       */
      room = ch->in_room;
      /*
       * value[2] for the room vnum to put the object in if there is one, 0 for giving it to char or current room 
       */
      if( obj->value[2] > 0 && !( room = get_room_index( obj->value[2] ) ) )
      {
         bug( "%s: obj points to invalid room vnum %d", __func__, obj->value[2] );
         return;
      }
      if( !( mob = create_mobile( pMobIndex ) ) )
      {
         bug( "%s: obj couldnt create_mobile vnum %d", __func__, obj->value[1] );
         return;
      }
      char_to_room( mob, room );
      return;
   }

   /*
    * Spell casting support added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_CAST ) )
   {
      if( obj->value[1] <= 0 || !IS_VALID_SN( obj->value[1] ) )
      {
         bug( "%s: obj points to invalid sn [%d]", __func__, obj->value[1] );
         return;
      }
      obj_cast_spell( obj->value[1], URANGE( 1, ( obj->value[2] > 0 ) ? obj->value[2] : ch->top_level, MAX_LEVEL ), ch, ch,
                      NULL );
      return;
   }

   /*
    * Container support added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_CONTAINER ) )
   {
      OBJ_DATA *container = NULL;

      room = get_room_index( obj->value[1] );
      if( !room )
         room = obj->in_room;
      if( !room )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }

      for( container = ch->in_room->first_content; container; container = container->next_content )
      {
         if( container->pIndexData->vnum == obj->value[2] )
            break;
      }
      if( !container )
      {
         bug( "%s: obj points to a container [%d] thats not where it should be?", __func__, obj->value[2] );
         return;
      }
      if( container->item_type != ITEM_CONTAINER )
      {
         bug( "%s: obj points to object [%d], but it isn't a container.", __func__, obj->value[2] );
         return;
      }
      /*
       * Could toss in some messages. Limit how it is handled etc... I'll leave that to each one to do 
       */
      /*
       * Started to use TRIG_OPEN, TRIG_CLOSE, TRIG_LOCK, and TRIG_UNLOCK like TRIG_DOOR does. 
       */
      /*
       * It limits it alot, but it wouldn't allow for an EATKEY change 
       */
      if( IS_SET( obj->value[3], CONT_CLOSEABLE ) )
         TOGGLE_BIT( container->value[1], CONT_CLOSEABLE );
      if( IS_SET( obj->value[3], CONT_PICKPROOF ) )
         TOGGLE_BIT( container->value[1], CONT_PICKPROOF );
      if( IS_SET( obj->value[3], CONT_CLOSED ) )
         TOGGLE_BIT( container->value[1], CONT_CLOSED );
      if( IS_SET( obj->value[3], CONT_LOCKED ) )
         TOGGLE_BIT( container->value[1], CONT_LOCKED );
      return;
   }

   if( IS_SET( obj->value[0], TRIG_DOOR ) )
   {
      room = get_room_index( obj->value[1] );
      if( !room )
         room = obj->in_room;
      if( !room )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_D_NORTH ) )
      {
         edir = DIR_NORTH;
         txt = "to the north";
      }
      else if( IS_SET( obj->value[0], TRIG_D_SOUTH ) )
      {
         edir = DIR_SOUTH;
         txt = "to the south";
      }
      else if( IS_SET( obj->value[0], TRIG_D_EAST ) )
      {
         edir = DIR_EAST;
         txt = "to the east";
      }
      else if( IS_SET( obj->value[0], TRIG_D_WEST ) )
      {
         edir = DIR_WEST;
         txt = "to the west";
      }
      else if( IS_SET( obj->value[0], TRIG_D_UP ) )
      {
         edir = DIR_UP;
         txt = "from above";
      }
      else if( IS_SET( obj->value[0], TRIG_D_DOWN ) )
      {
         edir = DIR_DOWN;
         txt = "from below";
      }
      else
      {
         bug( "%s: door: no direction flag set.", __func__ );
         return;
      }
      pexit = get_exit( room, edir );
      if( !pexit )
      {
         if( !IS_SET( obj->value[0], TRIG_PASSAGE ) )
         {
            bug( "%s: obj points to non-exit %d", __func__, obj->value[1] );
            return;
         }
         to_room = get_room_index( obj->value[2] );
         if( !to_room )
         {
            bug( "%s: dest points to invalid room %d", __func__, obj->value[2] );
            return;
         }
         pexit = make_exit( room, to_room, edir );
         pexit->keyword = STRALLOC( "" );
         pexit->description = STRALLOC( "" );
         pexit->key = -1;
         pexit->exit_info = 0;
         top_exit++;
         act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR );
         act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_UNLOCK ) && IS_SET( pexit->exit_info, EX_LOCKED ) )
      {
         REMOVE_BIT( pexit->exit_info, EX_LOCKED );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_LOCK ) && !IS_SET( pexit->exit_info, EX_LOCKED ) )
      {
         SET_BIT( pexit->exit_info, EX_LOCKED );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
            SET_BIT( pexit_rev->exit_info, EX_LOCKED );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_OPEN ) && IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         REMOVE_BIT( pexit->exit_info, EX_CLOSED );
         for( rch = room->first_person; rch; rch = rch->next_in_room )
            act( AT_ACTION, "The $d opens.", rch, NULL, pexit->keyword, TO_CHAR );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
         {
            REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            for( rch = to_room->first_person; rch; rch = rch->next_in_room )
               act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
         }
         check_room_for_traps( ch, trap_door[edir] );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_CLOSE ) && !IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         SET_BIT( pexit->exit_info, EX_CLOSED );
         for( rch = room->first_person; rch; rch = rch->next_in_room )
            act( AT_ACTION, "The $d closes.", rch, NULL, pexit->keyword, TO_CHAR );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
         {
            SET_BIT( pexit_rev->exit_info, EX_CLOSED );
            for( rch = to_room->first_person; rch; rch = rch->next_in_room )
               act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
         }
         check_room_for_traps( ch, trap_door[edir] );
         return;
      }
   }
}

void do_pull( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Pull what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
      return;
   }

   pullorpush( ch, obj, TRUE );
}

void do_push( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Push what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
      return;
   }

   pullorpush( ch, obj, FALSE );
}

/* pipe commands (light, tamp, smoke) by Thoric */
void do_tamp( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *pipe;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Tamp what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( pipe = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }
   if( pipe->item_type != ITEM_PIPE )
   {
      send_to_char( "You can't tamp that.\r\n", ch );
      return;
   }
   if( !IS_SET( pipe->value[3], PIPE_TAMPED ) )
   {
      act( AT_ACTION, "You gently tamp $p.", ch, pipe, NULL, TO_CHAR );
      act( AT_ACTION, "$n gently tamps $p.", ch, pipe, NULL, TO_ROOM );
      SET_BIT( pipe->value[3], PIPE_TAMPED );
      return;
   }
   send_to_char( "It doesn't need tamping.\r\n", ch );
}

void do_smoke( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *pipe;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Smoke what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( pipe = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }
   if( pipe->item_type != ITEM_PIPE )
   {
      act( AT_ACTION, "You try to smoke $p... but it doesn't seem to work.", ch, pipe, NULL, TO_CHAR );
      act( AT_ACTION, "$n tries to smoke $p... (I wonder what $e's been putting his $s pipe?)", ch, pipe, NULL, TO_ROOM );
      return;
   }
   if( !IS_SET( pipe->value[3], PIPE_LIT ) )
   {
      act( AT_ACTION, "You try to smoke $p, but it's not lit.", ch, pipe, NULL, TO_CHAR );
      act( AT_ACTION, "$n tries to smoke $p, but it's not lit.", ch, pipe, NULL, TO_ROOM );
      return;
   }
   if( pipe->value[1] > 0 )
   {
      if( !oprog_use_trigger( ch, pipe, NULL, NULL, NULL ) )
      {
         act( AT_ACTION, "You draw thoughtfully from $p.", ch, pipe, NULL, TO_CHAR );
         act( AT_ACTION, "$n draws thoughtfully from $p.", ch, pipe, NULL, TO_ROOM );
      }

      if( IS_VALID_HERB( pipe->value[2] ) && pipe->value[2] < top_herb )
      {
         int sn = pipe->value[2] + TYPE_HERB;
         SKILLTYPE *skill = get_skilltype( sn );

         WAIT_STATE( ch, skill->beats );
         if( skill->spell_fun )
            obj_cast_spell( sn, UMIN( skill->min_level, ch->top_level ), ch, ch, NULL );
         if( obj_extracted( pipe ) )
            return;
      }
      else
         bug( "%s: bad herb type %d", __func__, pipe->value[2] );

      SET_BIT( pipe->value[3], PIPE_HOT );
      if( --pipe->value[1] < 1 )
      {
         REMOVE_BIT( pipe->value[3], PIPE_LIT );
         SET_BIT( pipe->value[3], PIPE_DIRTY );
         SET_BIT( pipe->value[3], PIPE_FULLOFASH );
      }
   }
}

void do_light( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *pipe;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Light what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( pipe = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }
   if( pipe->item_type != ITEM_PIPE )
   {
      send_to_char( "You can't light that.\r\n", ch );
      return;
   }
   if( !IS_SET( pipe->value[3], PIPE_LIT ) )
   {
      if( pipe->value[1] < 1 )
      {
         act( AT_ACTION, "You try to light $p, but it's empty.", ch, pipe, NULL, TO_CHAR );
         act( AT_ACTION, "$n tries to light $p, but it's empty.", ch, pipe, NULL, TO_ROOM );
         return;
      }
      act( AT_ACTION, "You carefully light $p.", ch, pipe, NULL, TO_CHAR );
      act( AT_ACTION, "$n carefully lights $p.", ch, pipe, NULL, TO_ROOM );
      SET_BIT( pipe->value[3], PIPE_LIT );
      return;
   }
   send_to_char( "It's already lit.\r\n", ch );
}

void do_empty( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
      argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Empty what?\r\n", ch );
      return;
   }
   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }
   if( obj->count > 1 )
      separate_obj( obj );

   switch ( obj->item_type )
   {
      default:
         act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
         return;
      case ITEM_PIPE:
         act( AT_ACTION, "You gently tap $p and empty it out.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n gently taps $p and empties it out.", ch, obj, NULL, TO_ROOM );
         REMOVE_BIT( obj->value[3], PIPE_FULLOFASH );
         REMOVE_BIT( obj->value[3], PIPE_LIT );
         obj->value[1] = 0;
         return;
      case ITEM_DRINK_CON:
         if( obj->value[1] < 1 )
         {
            send_to_char( "It's already empty.\r\n", ch );
            return;
         }
         act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
         obj->value[1] = 0;
         return;
      case ITEM_CONTAINER:
         if( IS_SET( obj->value[1], CONT_CLOSED ) )
         {
            act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
            return;
         }
         if( !obj->first_content )
         {
            send_to_char( "It's already empty.\r\n", ch );
            return;
         }
         if( arg2[0] == '\0' )
         {
            if( IS_SET( ch->in_room->room_flags, ROOM_NODROP ) || ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_LITTERBUG ) ) )
            {
               set_char_color( AT_MAGIC, ch );
               send_to_char( "A magical force stops you!\r\n", ch );
               set_char_color( AT_TELL, ch );
               send_to_char( "Someone tells you, 'No littering here!'\r\n", ch );
               return;
            }
            if( IS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) )
            {
               send_to_char( "You can't seem to do that here...\r\n", ch );
               return;
            }
            if( empty_obj( obj, NULL, ch->in_room ) )
            {
               act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
               act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
               if( IS_SET( sysdata.save_flags, SV_DROP ) )
                  save_char_obj( ch );
            }
            else
               send_to_char( "Hmmm... didn't work.\r\n", ch );
         }
         else
         {
            OBJ_DATA *dest = get_obj_here( ch, arg2 );

            if( !dest )
            {
               send_to_char( "You can't find it.\r\n", ch );
               return;
            }
            if( dest == obj )
            {
               send_to_char( "You can't empty something into itself!\r\n", ch );
               return;
            }
            if( dest->item_type != ITEM_CONTAINER )
            {
               send_to_char( "That's not a container!\r\n", ch );
               return;
            }
            if( IS_SET( dest->value[1], CONT_CLOSED ) )
            {
               act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
               return;
            }
            separate_obj( dest );
            if( empty_obj( obj, dest, NULL ) )
            {
               act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
               act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
               if( !dest->carried_by && IS_SET( sysdata.save_flags, SV_PUT ) )
                  save_char_obj( ch );
            }
            else
               act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
         }
         return;
   }
}

/*
 * Apply a salve/ointment					-Thoric
 */
void do_apply( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *obj;
   ch_ret retcode;

   if( argument[0] == '\0' )
   {
      send_to_char( "Apply what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
   {
      send_to_char( "You do not have that.\r\n", ch );
      return;
   }

   if( obj->item_type != ITEM_SALVE )
   {
      act( AT_ACTION, "$n starts to rub $p on $mself...", ch, obj, NULL, TO_ROOM );
      act( AT_ACTION, "You try to rub $p on yourself...", ch, obj, NULL, TO_CHAR );
      return;
   }

   separate_obj( obj );

   --obj->value[1];
   if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
   {
      if( !obj->action_desc || obj->action_desc[0] == '\0' )
      {
         act( AT_ACTION, "$n rubs $p onto $s body.", ch, obj, NULL, TO_ROOM );
         if( obj->value[1] <= 0 )
            act( AT_ACTION, "You apply the last of $p onto your body.", ch, obj, NULL, TO_CHAR );
         else
            act( AT_ACTION, "You apply $p onto your body.", ch, obj, NULL, TO_CHAR );
      }
      else
         actiondesc( ch, obj, NULL );
   }

   WAIT_STATE( ch, obj->value[2] );
   retcode = obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
   if( retcode == rNONE )
      retcode = obj_cast_spell( obj->value[5], obj->value[0], ch, ch, NULL );

   if( !obj_extracted( obj ) && obj->value[1] <= 0 )
      extract_obj( obj );

   return;
}

void actiondesc( CHAR_DATA * ch, OBJ_DATA * obj, void *vo )
{
   char charbuf[MAX_STRING_LENGTH];
   char roombuf[MAX_STRING_LENGTH];
   char *srcptr = obj->action_desc;
   char *charptr = charbuf;
   char *roomptr = roombuf;
   const char *ichar;
   const char *iroom;

   while( *srcptr != '\0' )
   {
      if( *srcptr == '$' )
      {
         srcptr++;
         switch ( *srcptr )
         {
            case 'e':
               ichar = "you";
               iroom = "$e";
               break;

            case 'm':
               ichar = "you";
               iroom = "$m";
               break;

            case 'n':
               ichar = "you";
               iroom = "$n";
               break;

            case 's':
               ichar = "your";
               iroom = "$s";
               break;

               /*
                * case 'q':
                * iroom = "s";
                * break;
                */

            default:
               srcptr--;
               *charptr++ = *srcptr;
               *roomptr++ = *srcptr;
               break;
         }
      }
      else if( *srcptr == '%' && *++srcptr == 's' )
      {
         ichar = "You";
         iroom = "$n";
      }
      else
      {
         *charptr++ = *srcptr;
         *roomptr++ = *srcptr;
         srcptr++;
         continue;
      }

      while( ( *charptr = *ichar ) != '\0' )
      {
         charptr++;
         ichar++;
      }

      while( ( *roomptr = *iroom ) != '\0' )
      {
         roomptr++;
         iroom++;
      }
      srcptr++;
   }

   *charptr = '\0';
   *roomptr = '\0';

   switch ( obj->item_type )
   {
      case ITEM_BLOOD:
      case ITEM_FOUNTAIN:
         act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
         act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
         return;

      case ITEM_DRINK_CON:
         act( AT_ACTION, charbuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_CHAR );
         act( AT_ACTION, roombuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_ROOM );
         return;

      case ITEM_PIPE:
         return;

      case ITEM_ARMOR:
      case ITEM_WEAPON:
      case ITEM_LIGHT:
         act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
         act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
         return;

      case ITEM_FOOD:
      case ITEM_PILL:
         act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
         act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
         return;

      default:
         return;
   }
   return;
}

void do_hail( CHAR_DATA * ch, const char *argument )
{
   int vnum;
   ROOM_INDEX_DATA *room = NULL;

   if( !ch->in_room )
      return;

   if( ch->position < POS_FIGHTING )
   {
      send_to_char( "You might want to stop fighting first!\r\n", ch );
      return;
   }

   if( ch->position < POS_STANDING )
   {
      send_to_char( "You might want to stand up first!\r\n", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
   {
      send_to_char( "You'll have to go outside to do that!\r\n", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) )
   {
      send_to_char( "You can't do that on spacecraft!\r\n", ch );
      return;
   }

   if( ch->gold < ( ch->top_level - 9 ) )
   {
      send_to_char( "You don't have enough credits!\r\n", ch );
      return;
   }

   if( !IS_NPC( ch ) && IS_SET( ch->pcdata->act2, ACT_BOUND ) )
   {
      send_to_char( "You attempt to wave your hand for a cab, but in vain!\r\n", ch );
      return;
   }

   vnum = ch->in_room->vnum;

   for( vnum = ch->in_room->area->low_r_vnum; vnum <= ch->in_room->area->hi_r_vnum; vnum++ )
   {
      room = get_room_index( vnum );

      if( room != NULL )
      {
         if( IS_SET( room->room_flags, ROOM_HOTEL ) && !IS_SET( room->room_flags, ROOM_PLR_HOME ) )
            break;
         else
            room = NULL;
      }
   }

   if( room == NULL )
   {
      send_to_char( "There doesn't seem to be any taxis nearby!\r\n", ch );
      return;
   }

   ch->gold -= UMAX( ch->top_level - 9, 0 );

   act( AT_ACTION, "$n hails a speederbike, and drives off to seek shelter.", ch, NULL, NULL, TO_ROOM );

   char_from_room( ch );
   char_to_room( ch, room );

   send_to_char( "A speederbike picks you up and drives you to a safe location.\r\nYou pay the driver 20 credits.\r\n\n\n",
                 ch );
   act( AT_ACTION, "$n $T", ch, NULL, "arrives on a speederbike, gets off and pays the driver before it leaves.", TO_ROOM );

   do_look( ch, "auto" );

}

void do_train( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *mob;
   bool tfound = FALSE;
   bool successful = FALSE;

   if( IS_NPC( ch ) )
      return;

   strlcpy( arg, argument, MAX_INPUT_LENGTH );

   /*
    * switch( ch->substate )
    * { 
    * default:
    */

   if( arg[0] == '\0' )
   {
      send_to_char( "Train what?\r\n", ch );
      send_to_char( "\r\nChoices: strength, intelligence, wisdom, dexterity, constitution or charisma\r\n", ch );
      return;
   }

   if( !IS_AWAKE( ch ) )
   {
      send_to_char( "In your dreams, or what?\r\n", ch );
      return;
   }

   for( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
      if( IS_NPC( mob ) && IS_SET( mob->act, ACT_TRAIN ) )
      {
         tfound = TRUE;
         break;
      }

   if( ( !mob ) || ( !tfound ) )
   {
      send_to_char( "You can't do that here.\r\n", ch );
      return;
   }

   if( str_cmp( arg, "str" ) && str_cmp( arg, "strength" )
       && str_cmp( arg, "dex" ) && str_cmp( arg, "dexterity" )
       && str_cmp( arg, "con" ) && str_cmp( arg, "constitution" )
       && str_cmp( arg, "cha" ) && str_cmp( arg, "charisma" )
       && str_cmp( arg, "wis" ) && str_cmp( arg, "wisdom" ) && str_cmp( arg, "int" ) && str_cmp( arg, "intelligence" ) )
   {
      do_train( ch, "" );
      return;
   }

   if( !str_cmp( arg, "str" ) || !str_cmp( arg, "strength" ) )
   {
      if( mob->perm_str <= ch->perm_str || ch->perm_str >= 20 + race_table[ch->race].str_plus || ch->perm_str >= 25 )
      {
         act( AT_TELL, "$n tells you 'I cannot help you... you are already stronger than I.'", mob, NULL, ch, TO_VICT );
         return;
      }
      send_to_char( "&GYou begin your weight training.\r\n", ch );
   }
   if( !str_cmp( arg, "dex" ) || !str_cmp( arg, "dexterity" ) )
   {
      if( mob->perm_dex <= ch->perm_dex || ch->perm_dex >= 20 + race_table[ch->race].dex_plus || ch->perm_dex >= 25 )
      {
         act( AT_TELL, "$n tells you 'I cannot help you... you are already more dextrous than I.'", mob, NULL, ch, TO_VICT );
         return;
      }
      send_to_char( "&GYou begin to work at some challenging tests of coordination.\r\n", ch );
   }
   if( !str_cmp( arg, "int" ) || !str_cmp( arg, "intelligence" ) )
   {
      if( mob->perm_int <= ch->perm_int || ch->perm_int >= 20 + race_table[ch->race].int_plus || ch->perm_int >= 25 )
      {
         act( AT_TELL, "$n tells you 'I cannot help you... you are already more educated than I.'", mob, NULL, ch, TO_VICT );
         return;
      }
      send_to_char( "&GYou begin your studies.\r\n", ch );
   }
   if( !str_cmp( arg, "wis" ) || !str_cmp( arg, "wisdom" ) )
   {
      if( mob->perm_wis <= ch->perm_wis || ch->perm_wis >= 20 + race_table[ch->race].wis_plus || ch->perm_wis >= 25 )
      {
         act( AT_TELL, "$n tells you 'I cannot help you... you are already far wiser than I.'", mob, NULL, ch, TO_VICT );
         return;
      }
      send_to_char( "&GYou begin contemplating several ancient texts in an effort to gain wisdom.\r\n", ch );
   }
   if( !str_cmp( arg, "con" ) || !str_cmp( arg, "constitution" ) )
   {
      if( mob->perm_con <= ch->perm_con || ch->perm_con >= 20 + race_table[ch->race].con_plus || ch->perm_con >= 25 )
      {
         act( AT_TELL, "$n tells you 'I cannot help you... you are already healthier than I.'", mob, NULL, ch, TO_VICT );
         return;
      }
      send_to_char( "&GYou begin your endurance training.\r\n", ch );
   }
   if( !str_cmp( arg, "cha" ) || !str_cmp( arg, "charisma" ) )
   {
      if( mob->perm_cha <= ch->perm_cha || ch->perm_cha >= 20 + race_table[ch->race].cha_plus || ch->perm_cha >= 25 )
      {
         act( AT_TELL, "$n tells you 'I cannot help you... you already are more charming than I.'", mob, NULL, ch, TO_VICT );
         return;
      }
      send_to_char( "&GYou begin lessons in maners and ettiquite.\r\n", ch );
   }

   successful = TRUE;
   if( !str_cmp( arg, "str" ) || !str_cmp( arg, "strength" ) )
   {
      if( !successful )
      {
         send_to_char( "&RYou feel that you have wasted alot of energy for nothing...\r\n", ch );
         return;
      }
      send_to_char( "&GAfter much excercise you feel a little stronger.\r\n", ch );
      ch->perm_str++;
      return;
   }

   if( !str_cmp( arg, "dex" ) || !str_cmp( arg, "dexterity" ) )
   {
      if( !successful )
      {
         send_to_char( "&RAfter all that training you still feel like a clutz...\r\n", ch );
         return;
      }
      send_to_char( "&GAfter working hard at many challenging tasks you feel a bit more coordinated.\r\n", ch );
      ch->perm_dex++;
      return;
   }

   if( !str_cmp( arg, "int" ) || !str_cmp( arg, "intelligence" ) )
   {
      if( !successful )
      {
         send_to_char( "&RHitting the books leaves you only with sore eyes...\r\n", ch );
         return;
      }
      send_to_char( "&GAfter much studying you feel alot more knowledgeable.\r\n", ch );
      ch->perm_int++;
      return;
   }

   if( !str_cmp( arg, "wis" ) || !str_cmp( arg, "wisdom" ) )
   {
      if( !successful )
      {
         send_to_char( "&RStudying the ancient texts has left you more confused than wise...\r\n", ch );
         return;
      }
      send_to_char
         ( "&GAfter contemplating several seemingly meaningless events you suddenly \r\nreceive a flash of insight into the workings of the universe.\r\n",
           ch );
      ch->perm_wis++;
      return;
   }

   if( !str_cmp( arg, "con" ) || !str_cmp( arg, "constitution" ) )
   {
      if( !successful )
      {
         send_to_char
            ( "&RYou spend long a long arobics session ecersising very hard but finish \r\nfeeling only tired and out of breath....\r\n",
              ch );
         return;
      }
      send_to_char( "&GAfter a long tiring exercise session you feel much healthier than before.\r\n", ch );
      ch->perm_con++;
      return;
   }


   if( !str_cmp( arg, "cha" ) || !str_cmp( arg, "charisma" ) )
   {
      if( !successful )
      {
         send_to_char( "&RYou finish your self improvement session feeling a little depressed.\r\n", ch );
         return;
      }
      send_to_char
         ( "&GYou spend some time focusing on how to improve your personality and feel \r\nmuch better about yourself and the ways others see you.\r\n",
           ch );
      ch->perm_cha++;
      return;
   }

}

void do_suicide( CHAR_DATA * ch, const char *argument )
{
   OBJ_DATA *wield;

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char( "Yeah right!\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "&RIf you really want to delete this character type suicide and your password.\r\n", ch );
      return;
   }

   if( strcmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
   {
      send_to_char( "Sorry wrong password.\r\n", ch );
      log_printf( "%s attempting to commit suicide... WRONG PASSWORD!", ch->name );
      return;
   }

   wield = get_eq_char( ch, WEAR_WIELD );

   if( !wield )
   {
      act( AT_BLOOD, "You ram your thumbs into your eyesockets, blood spurts onto the floor.\r\n", ch, NULL, NULL, TO_CHAR );
      act( AT_BLOOD, "Blood spurts onto your shoes as $n rams his thumbs into $s eyesockets.", ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      switch ( wield->value[3] )
      {
         default:
            act( AT_BLOOD, "You ram your thumbs into your eyesockets, blood spurts onto the floor.\r\n", ch, NULL, NULL,
                 TO_CHAR );
            act( AT_BLOOD, "Blood spurts onto your shoes as $n rams his thumbs into $s eyesockets.", ch, NULL, NULL,
                 TO_ROOM );
            break;

         case WEAPON_BLASTER:
            act( AT_BLOOD,
                 "You calmly place the barrel of your blaster into your mouth, and a spurt of brains falls to the ground behind you.",
                 ch, NULL, NULL, TO_CHAR );
            act( AT_BLOOD, "A spurt of brains hits the ground as $n blows $s head off.", ch, NULL, NULL, TO_ROOM );
            break;

         case WEAPON_VIBRO_BLADE:
            act( AT_BLOOD, "With a sad determination and trembling hands you slit your own throat!", ch, NULL, NULL,
                 TO_CHAR );
            act( AT_BLOOD, "Cold shivers run down your spine as you watch $n slit $s own throat!", ch, NULL, NULL, TO_ROOM );
            break;

         case WEAPON_FORCE_PIKE:
            act( AT_BLOOD, "You wedge your force pike into the ground and slam your face into the blade.", ch, NULL, NULL,
                 TO_CHAR );
            act( AT_BLOOD,
                 "$n slams $s face into $s force pike, pieces of face fall to the ground as the pike slices through it like butter.",
                 ch, NULL, NULL, TO_ROOM );
            break;

         case WEAPON_LIGHTSABER:
         case WEAPON_DUAL_LIGHTSABER:
            act( AT_BLOOD,
                 "You calmly kneel, and hold your saber hilt to your sternum, and without a moments hesitation, turn it on.",
                 ch, NULL, NULL, TO_CHAR );
            act( AT_BLOOD, "$n calmly kneels and holds $s saber hilt to $s chest, and turns it on. $e falls over in agony.",
                 ch, NULL, NULL, TO_ROOM );
            break;

         case WEAPON_BOWCASTER:
            act( AT_BLOOD, "You pull back the lever, hold the bowcaster to your face and... death.", ch, NULL, NULL,
                 TO_CHAR );
            act( AT_BLOOD, "$n blows his head off with a bowcaster quarrel!", ch, NULL, NULL, TO_ROOM );
            break;

      }
   }

   log_printf( "%s has commited suicide.", ch->name );

   set_cur_char( ch );
   raw_kill( ch, ch );
}

void do_bank( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   long amount = 0;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( IS_NPC( ch ) || !ch->pcdata )
      return;

   if( !ch->in_room || !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
   {
      send_to_char( "You must be in a bank to do that!\r\n", ch );
      return;
   }

   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: BANK <deposit|withdraw|balance|transfer> [amount] (character, if transfer)\r\n", ch );
      return;
   }

   if( arg2[0] != '\0' )
      amount = atoi( arg2 );

   if( !str_prefix( arg1, "deposit" ) )
   {
      if( amount <= 0 )
      {
         send_to_char( "You may only deposit amounts greater than zero.\r\n", ch );
         do_bank( ch, "" );
         return;
      }

      if( ch->gold < amount )
      {
         send_to_char( "You don't have that many credits on you.\r\n", ch );
         return;
      }

      ch->gold -= amount;
      ch->pcdata->bank += amount;

      ch_printf( ch, "You deposit %ld credits into your account.\r\n", amount );
      return;
   }
   else if( !str_prefix( arg1, "withdraw" ) )
   {
      if( amount <= 0 )
      {
         send_to_char( "You may only withdraw amounts greater than zero.\r\n", ch );
         do_bank( ch, "" );
         return;
      }

      if( ch->pcdata->bank < amount )
      {
         send_to_char( "You don't have that many credits in your account.\r\n", ch );
         return;
      }

      ch->gold += amount;
      ch->pcdata->bank -= amount;

      ch_printf( ch, "You withdraw %ld credits from your account.\r\n", amount );
      return;

   }
   else if( !str_prefix( arg1, "balance" ) )
   {
      ch_printf( ch, "You have %ld credits in your account.\r\n", ch->pcdata->bank );
      return;
   }
   else if( !str_prefix( arg1, "transfer" ) )
   {
      victim = get_char_world( ch, arg3 );

      if( victim == NULL || IS_NPC( victim ) )
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }

      if( ch->top_level <= 10 )
      {
         send_to_char( "You must be level 11 or higher to use the bank transfer command.\r\n", ch );
         return;
      }

      if( ch->pcdata->bank < amount )
      {
         send_to_char( "You don't have that many credits.\r\n", ch );
         return;
      }

      if( amount <= 0 )
      {
         send_to_char( "You may only transfer amounts greater than zero.\r\n", ch );
         return;
      }
      if( victim->pcdata->bank > 0 && victim->pcdata->bank + amount < 0 )
      {
         send_to_char( "Their account cannot handle that much money!\r\n", ch );
         return;
      }
      ch_printf( ch, "&W&GYou transfer %ld credits to %s's account.\r\n", amount, arg3 );
      log_printf( "%s transfers %ld credits to %s", ch->name, amount, victim->name );
      ch->pcdata->bank -= amount;
      victim->pcdata->bank += amount;
      ch_printf( ch, "Successful.\r\n" );
      ch_printf( victim, "&W&G%s has deposited %ld credits into your account.\r\n", ch->name, amount );
      return;
   }
   else
   {
      do_bank( ch, "" );
      return;
   }
}

void do_showsocial( CHAR_DATA * ch, const char *argument )
{
   SOCIALTYPE *social;

   if( argument[0] == '\0' || !argument )
   {
      send_to_char( "&RSyntax: showsocial <social>&W\r\n", ch );
      return;
   }

   social = find_social( argument, FALSE );

   if( !social )
   {
      send_to_char( "No such social.\r\n", ch );
      return;
   }

   ch_printf( ch, "&GSocial&B:&W %s\r\n\r\n&GChar, No Arg&B:&W %s\r\n", social->name, social->char_no_arg );
   ch_printf( ch, "&GOthers, No Arg&B:&W %s\r\n&GChar, Vict found&B:&W %s\r\n&GOthers, Vict Found&B:&W %s\r\n",
              social->others_no_arg ? social->others_no_arg : "(not set)",
              social->char_found ? social->char_found : "(not set)",
              social->others_found ? social->others_found : "(not set)" );
   ch_printf( ch, "&GVict, Found&B:&W %s\r\n&GChar, Self &B:&W %s\r\n&GOthers, Self &B:&W %s\r\n",
              social->vict_found ? social->vict_found : "(not set)", social->char_auto ? social->char_auto : "(not set)",
              social->others_auto ? social->others_auto : "(not set)" );
   return;
}

void do_tune( CHAR_DATA * ch, const char *argument )
{
   SHIP_DATA *ship;
   int num;

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be in the cockpit of a ship to tune it's channel.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: Tune <channel>\r\n", ch );
      return;
   }

   num = atoi( argument );
   if( num > 100 || num < 0 )
   {
      send_to_char( "The accepted channel range runs 1-100 or 0 for public.\r\n", ch );
      return;
   }
   act( AT_WHITE, "You make some adjustments on the ships comm system.", ch, NULL, NULL, TO_CHAR );
   act( AT_WHITE, "$n makes some adjustments on the ships comm system.", ch, NULL, NULL, TO_ROOM );
   ch_printf( ch, "Ships channel tuned to %s%d.\r\n", num == 0 ? "the public channel " : "", num );
   ship->channel = num;
   save_ship( ship );
}

void do_whisper( CHAR_DATA * ch, const char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   strlcpy( arg2, argument, MAX_INPUT_LENGTH );
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They are not in here.\r\n", ch );
      return;
   }
   ch_printf( ch, "&B[&Cwhisper&B] &Cto %s: &W%s\r\n", PERS( victim, ch ), arg2 );
   ch_printf( victim, "&B[&Cwhisper&B] &C%s whispers: &W%s\r\n", PERS( ch, victim ), arg2 );
}

#define MAX_DROP_TROOPS 20

//do_droptroops fixed by KeB 10/24/06
void do_droptroops( CHAR_DATA * ch, const char *argument )
{
   int num, vnum, i;
   SHIP_DATA *ship;
   CHAR_DATA *mob;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_DATA *blaster;
   OBJ_INDEX_DATA *pObjIndex;
   ROOM_INDEX_DATA *room, *rtest;
   char tmpbuf[MAX_STRING_LENGTH];

   if( ch->pcdata->clan == NULL )
   {
      send_to_char( "You must be in a clan to drop troops.\r\n", ch );
      return;
   }

   if( ch->pcdata->clan->troops < 1 )
   {
      send_to_char( "Your clan has no ground assault troops.\r\n", ch );
      return;
   }

   num = atoi( argument );
   if( num > ch->pcdata->clan->troops )
   {
      send_to_char( "Your clan doesn't have that many ground assault troops.\r\n", ch );
      return;
   }

   if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
   {
      send_to_char( "You must be in the cockpit of the dropship to do this.\r\n", ch );
      return;
   }

   vnum = ship->location;
   if( ( room = get_room_index( vnum ) ) == NULL )
   {
      send_to_char( "This ship is not in a room.\r\n", ch );
      return;
   }

   if( num > MAX_DROP_TROOPS )
   {
      ch_printf( ch, "You can only drop %d troops at once!\r\n", MAX_DROP_TROOPS );
      return;
   }
   if( num > ch->pcdata->clan->troops )
      num = ch->pcdata->clan->troops;

   if( ( pMobIndex = get_mob_index( 82 ) ) == NULL )
      return;

   if( room->area == NULL )
      return;

   for( i = 1; i <= num; i++ )
   {
      /*
       * Can be any room but the hi_room and low_room 
       */
      vnum = number_range( room->area->low_r_vnum + 1, room->area->hi_r_vnum - 1 );

      if( ( rtest = get_room_index( vnum ) ) == NULL )
         continue;

      /*
       * Now lets make sure the room isn't safe and isn't a player home 
       */
      if( IS_SET( rtest->room_flags, ROOM_SAFE ) || IS_SET( rtest->room_flags, ROOM_PLR_HOME ) )
      {
         --i;
         continue;
      }
      mob = create_mobile( pMobIndex );
      char_to_room( mob, rtest );
      if( ch->pcdata && ch->pcdata->clan )
         snprintf( tmpbuf, MAX_STRING_LENGTH, "(%s) %s", ch->pcdata->clan->name, mob->long_descr );
      STRFREE( mob->long_descr );
      mob->mob_clan = QUICKLINK( ch->pcdata->clan->name );
      mob->long_descr = STRALLOC( tmpbuf );
      if( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
      {
         blaster = create_object( pObjIndex, mob->top_level );
         obj_to_char( blaster, mob );
         equip_char( mob, blaster, WEAR_WIELD );
      }
   }
   snprintf( tmpbuf, MAX_STRING_LENGTH,
            "&RYelling and the thunder of feet is heard from the troop hold as %d troops exit the ship and fan out.\r\n",
            num );
   ch->pcdata->clan->troops -= num;
   echo_to_cockpit( AT_RED, ship, tmpbuf );
}

void do_hale( CHAR_DATA * ch, const char *argument )
{
   SHIP_DATA *ship;
   SHIP_DATA *ship2 = ship_from_cockpit( ch->in_room->vnum );
   char buf[MAX_STRING_LENGTH];
   char buf1[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   strlcpy( arg2, argument, MAX_INPUT_LENGTH );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: Hail <ship> <message>\r\n", ch );
      return;
   }

   ship = get_ship( arg1 );
   send_to_char( "\r\n", ch );
   if( !ship )
   {
      send_to_char( "No Such Ship!\r\n", ch );
      return;
   }
   if( !ship2 )
   {
      send_to_char( "But your not in a cockpit!\r\n", ch );
      return;
   }

   if( IS_SET( ship->flags, SHIP_SIMULATOR ) && !IS_SET( ship2->flags, SHIP_SIMULATOR ) )
   {
      send_to_char( "No Such Ship!\r\n", ch );
      return;
   }

   if( !IS_SET( ship->flags, SHIP_SIMULATOR ) && IS_SET( ship2->flags, SHIP_SIMULATOR ) )
   {
      send_to_char( "No Such Ship!\r\n", ch );
      return;
   }

   snprintf( buf, MAX_STRING_LENGTH, "&B[&CShip Hail&B] &C%s&B: &w%s", ship2->name, arg2 );
   ch_printf( ch, "&B[&CShip Hail&B] &CYou send&B: &w%s", arg2 );

   snprintf( buf1, MAX_STRING_LENGTH, "&g%s adjusts some settings on the comm system and says:\r\n&C'&w%s&C'", ch->name, arg2 );
   act( AT_GREEN, buf1, ch, NULL, NULL, TO_NOTVICT );
   echo_to_cockpit( AT_BLUE, ship, buf );
   return;
}

void do_rpconvert( CHAR_DATA * ch, const char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      send_to_char( "&w&WSpend your RP points on what? The various options are:\r\n", ch );
      send_to_char( "&G+-----------------------------------------------------+\r\n", ch );
      send_to_char( "&G| &W1&G | &WBonus Level to any Class  &G| &W             10 RPP &G|&W\r\n", ch );
      send_to_char( "&G| &W2&G | &WClone                     &G| &W              4 RPP &G|&W\r\n", ch );
      send_to_char( "&G| &W3&G | &W5% to any skill(max 100%) &G| &W              1 RPP &G|&W\r\n", ch );
      send_to_char( "&G+-----------------------------------------------------+\r\n\r\n", ch );

      send_to_char( "&WFor more information on any bonus, type 'help rpconvert'\r\n", ch );
      send_to_char( "&WTo buy your bonus, type 'rpconvert <number of bonus> [extra arguments]'\r\n", ch );
      send_to_char( "&WException: For cloning, type 'clone' at the cloning center or 'backup' if you're droid.\r\n", ch );
      return;
   }

   if( atoi( argument ) == 1 )
   {
      char arg1[MAX_STRING_LENGTH];
      int iClass;
      argument = one_argument( argument, arg1 );

      if( argument[0] == '\0' || !argument )
      {
         send_to_char( "&RSyntax: rpconvert 1 <class>\r\n", ch );
         return;
      }

      if( ch->rppoints < 10 )
      {
         send_to_char( "&RThis bonus costs 10 RP points. You don't have enough.\r\n", ch );
         return;
      }

      for( iClass = 0; iClass < MAX_ABILITY; iClass++ )
      {
         if( UPPER( argument[0] ) == UPPER( ability_name[iClass][0] )
         && !str_prefix( argument, ability_name[iClass] ) && str_prefix( argument, "force" ) )
         {
            if( max_level( ch, iClass ) >= 30 )
            {
               send_to_char( "&RYou are already at the max possible for that.\r\n", ch );
               return;
            }
            ch->bonus[iClass] += 1;
            ch_printf( ch, "&GYour ability in %s has been increased by 1!\r\n", ability_name[iClass] );
            log_printf( "%s increased class %s with rpconvert.", ch->name, ability_name[iClass] );
            ch->rppoints = ch->rppoints - 10;
            break;
         }
      }

      if( iClass == MAX_ABILITY )
      {
         send_to_char( "No such class.\r\n", ch );
         return;
      }

      return;
   }

   if( atoi( argument ) == 2 )
   {
      send_to_char( "&RJust type 'clone' or 'backup' at a cloning facility!\r\n", ch );
      return;
   }

   if( atoi( argument ) == 3 )
   {
      char arg1[MAX_STRING_LENGTH];
      int sn;
      argument = one_argument( argument, arg1 );

      if( argument[0] == '\0' || !argument )
      {
         send_to_char( "&RSyntax: rpconvert 2 <skill>\r\n", ch );
         return;
      }

      if( ch->rppoints < 1 )
      {
         send_to_char( "&RThis bonus costs 1 RP point. You don't have enough.\r\n", ch );
         return;
      }

      sn = skill_lookup( argument );

      if( !sn || sn < 1 )
      {
         send_to_char( "&RNo such skill.\r\n", ch );
         return;
      }

      if( ch->pcdata->learned[sn] < 1 )
      {
         send_to_char( "&RYou must have at least practiced the skill to increase its percentage!\r\n", ch );
         return;
      }

      if( ch->pcdata->learned[sn] >= 100 )
      {
         send_to_char( "&RYou already have this skill adepted!\r\n", ch );
         return;
      }

      ch->pcdata->learned[sn] = UMIN( ch->pcdata->learned[sn] + 5, 100 );
      ch->rppoints -= 1;

      ch_printf( ch, "&GYou have spent 1 RPP to increase %s by 5%%!\r\n", skill_table[sn]->name );
      log_printf( "%s increased %s by 5%% with rpconvert.\r\n", ch->name, skill_table[sn]->name );

      return;
   }

   do_rpconvert( ch, "" );
}
