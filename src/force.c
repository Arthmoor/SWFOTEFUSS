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

int check_force_skill( CHAR_DATA * ch, char *command, char *argument )
{
   FORCE_SKILL *fskill;
   bool SKILL_FOUND = FALSE;
   DO_FUN *fun;

   for( fskill = first_force_skill; fskill; fskill = fskill->next )
   {
      if( !str_cmp( command, fskill->name ) )
      {
         SKILL_FOUND = TRUE;
         break;
      }
   }
   if( !SKILL_FOUND )
      return 0;
   fun = get_force_skill_function( fskill->code );
   if( fun == skill_notfound )
      return 0;
   ( *fskill->do_fun ) ( ch, argument );
   return 1;
}


void force_send_to_room( CHAR_DATA * ch, CHAR_DATA * victim, char *msg )
{
   DESCRIPTOR_DATA *i;
   CHAR_DATA *dch;
   for( i = first_descriptor; i; i = i->next )
   {
      if( i->connected || !i->character )
         continue;
      dch = i->original ? i->original : i->character;
      if( ch->in_room != dch->in_room )
         continue;
      if( ( !victim || dch != victim ) && ( !ch || dch != ch ) )
         send_to_char( msg, dch );
   }
}

CHAR_DATA *force_get_victim( CHAR_DATA * ch, char *argument, int loc )
{
   CHAR_DATA *victim;
   char target[MAX_STRING_LENGTH];
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "You don't see anyone like that here.\r\n", ch );
      return NULL;
   }
   argument = one_argument( argument, target );
   victim = get_char_world( ch, target );
   if( !victim || ( loc == FORCE_INROOM && victim->in_room != ch->in_room ) )
   {
      send_to_char( "You don't see anyone like that here.\r\n", ch );
      return NULL;
   }
   return victim;
}

char *force_get_possessive( CHAR_DATA * ch )
{
   if( ch->sex == SEX_MALE )
      return "his";
   else if( ch->sex == SEX_FEMALE )
      return "her";
   return "its";
}

char *force_get_objective( CHAR_DATA * ch )
{
   if( ch->sex == SEX_MALE )
      return "him";
   else if( ch->sex == SEX_FEMALE )
      return "her";
   return "it";
}

char *force_get_pronoun( CHAR_DATA * ch )
{
   if( ch->sex == SEX_MALE )
      return "he";
   else if( ch->sex == SEX_FEMALE )
      return "she";
   return "it";
}

char *force_get_level( CHAR_DATA * ch )
{
   switch ( ch->force_level_status )
   {
      case FORCE_APPRENTICE:
         if( ch->force_type == FORCE_JEDI )
            return "Jedi Apprentice";
         else if( ch->force_type == FORCE_SITH )
            return "Sith Apprentice";
         else
            return "Force Apprentice";
      case FORCE_KNIGHT:
         if( ch->force_type == FORCE_JEDI )
            return "Jedi Knight";
         else if( ch->force_type == FORCE_SITH )
            return "Sith Lord";
         else
            return "Force Knight";
      case FORCE_MASTER:
         if( ch->force_type == FORCE_JEDI )
            return "Jedi Master";
         else if( ch->force_type == FORCE_SITH )
            return "Sith Master";
         else
            return "Force Master";
      default:
         return "";
   }
}

char *force_parse_string( CHAR_DATA * ch, CHAR_DATA * victim, char *srcmsg )
{
   static char msg[MAX_STRING_LENGTH];

   strncpy( msg, srcmsg, MAX_STRING_LENGTH );

   if( victim )
   {
      strncpy( msg, strrep( msg, "$vfl", force_get_level( victim ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$vp", force_get_possessive( victim ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$vo", force_get_objective( victim ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$vn", force_get_pronoun( victim ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$v", PERS( victim, ch ) ), MAX_STRING_LENGTH );
   }
   if( ch )
   {
      strncpy( msg, strrep( msg, "$nfl", force_get_level( ch ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$np", force_get_possessive( ch ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$no", force_get_objective( ch ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$nn", force_get_pronoun( ch ) ), MAX_STRING_LENGTH );
      strncpy( msg, strrep( msg, "$n", ch->name ), MAX_STRING_LENGTH );

      if( ch->force_type == FORCE_JEDI )
      {
         strncpy( msg, strrep( msg, "$FC", "&C" ), MAX_STRING_LENGTH );
         strncpy( msg, strrep( msg, "$fc", "&B" ), MAX_STRING_LENGTH );
      }
      else if( ch->force_type == FORCE_SITH )
      {
         strncpy( msg, strrep( msg, "$FC", "&R" ), MAX_STRING_LENGTH );
         strncpy( msg, strrep( msg, "$fc", "&r" ), MAX_STRING_LENGTH );
      }
      else
      {
         strncpy( msg, strrep( msg, "$FC", "&W" ), MAX_STRING_LENGTH );
         strncpy( msg, strrep( msg, "$fc", "&w" ), MAX_STRING_LENGTH );
      }
   }
   strncpy( msg, strrep( msg, "$RN$$RN$", "\r\n\r\n" ), MAX_STRING_LENGTH );
   strncpy( msg, strrep( msg, "$RN$", "\r\n" ), MAX_STRING_LENGTH );
   return msg;
}

void force_learn_from_failure( CHAR_DATA * ch, FORCE_SKILL * fskill )
{
   int total = 0;
   int amt;
   int MOD;
   if( fskill->mastertrain && ch->force_level_status != FORCE_MASTER )
   {
      MOD = 2;
      if( ch->fighting && IS_NPC( ch->fighting->who ) )
         if( ch->force_master && ch->force_master[0] != '\0' )
            if( ch->fighting->who->name && ch->fighting->who->name[0] != '\0' )
               if( !strcmp( ch->fighting->who->name, ch->force_master ) )
                  MOD = 5;
   }
   else
      MOD = 5;
   if( number_range( 1, 100 ) < 90 )
      return;
   if( fskill->control )
      total += ch->force_control;
   if( fskill->sense )
      total += ch->force_sense;
   if( fskill->control )
      total += ch->force_alter;
   amt = number_range( 1, total ) * MOD / 300;
   if( amt < 1 )
      amt = 1;
   ch->force_skill[fskill->index] += amt;
   if( ch->force_skill[fskill->index] > 100 )
      ch->force_skill[fskill->index] = 100;
}

void force_learn_from_success( CHAR_DATA * ch, FORCE_SKILL * fskill )
{
   int total = 0;
   int amt;
   int MOD;
   if( fskill->mastertrain && ch->force_level_status != FORCE_MASTER )
   {
      MOD = 5;
      if( ch->fighting && IS_NPC( ch->fighting->who ) )
         if( ch->force_master && ch->force_master[0] != '\0' )
            if( ch->fighting->who->name && ch->fighting->who->name[0] != '\0' )
               if( !strcmp( ch->fighting->who->name, ch->force_master ) )
                  MOD = 10;
   }
   else
      MOD = 10;
   if( number_range( 1, 100 ) < 0 )
      return;
   if( fskill->control )
      total += ch->force_control;
   if( fskill->sense )
      total += ch->force_sense;
   if( fskill->control )
      total += ch->force_alter;
   amt = number_range( 1, total ) * MOD / 300;
   if( amt < 1 )
      amt = 1;
   ch->force_skill[fskill->index] += amt;
   if( ch->force_skill[fskill->index] > 100 )
      ch->force_skill[fskill->index] = 100;
}

FORCE_SKILL *force_test_skill_use( char *fskill_name, CHAR_DATA * ch, int skill_type )
{
   FORCE_SKILL *fskill;
   bool SKILL_FOUND = FALSE;
   for( fskill = first_force_skill; fskill; fskill = fskill->next )
   {
      if( !str_cmp( fskill_name, fskill->name ) )
      {
         SKILL_FOUND = TRUE;
         break;
      }
   }
   if( !SKILL_FOUND )
   {
      send_to_char( "ERROR: skill not found.\r\n", ch );
      return NULL;
   }
   if( ch->force_identified != 1 )
   {
      send_to_char( "Huh?\r\n", ch );
      return NULL;
   }
   if( ch->force_level_status < fskill->status && !ch->force_converted )
   {
      send_to_char( "Huh?\r\n", ch );
      return NULL;
   }
   if( ch->force_converted && fskill->type == ch->force_type && ch->force_level_status < fskill->status )
   {
      send_to_char( "Huh?\r\n", ch );
      return NULL;
   }
   if( ch->force_skill[fskill->index] == 0 && fskill->notskill == 0 )
   {
      send_to_char( "Huh?\r\n", ch );
      return NULL;
   }
   if( fskill->disabled == 1 )
   {
      send_to_char( "That force skill is disabled.\r\n", ch );
      return NULL;
   }
   if( skill_type != FORCE_NORESTRICT )
   {
      if( skill_type == FORCE_COMBAT && ch->fighting == NULL && str_cmp( fskill->name, "slash" ) )
      {
         send_to_char( "You must be fighting to use this skill!\r\n", ch );
         return NULL;
      }
      if( skill_type == FORCE_NONCOMBAT && ch->fighting != NULL )
      {
         send_to_char( "You cannot use this skill while fighting.\r\n", ch );
         return NULL;
      }
   }
   if( ch->substate == 1 || ch->substate == SUB_TIMER_DO_ABORT )
      return fskill;
   if( fskill->wait_state > 0 && ch->wait_state != 0 )
   {
      send_to_char( "You have not recovered yet.\r\n", ch );
      return NULL;
   }
   if( fskill->cost < 0 )
      fskill->cost = 0;
   if( fskill->cost != 0 && ch->mana < fskill->cost )
   {
      send_to_char( "You do not feel strong enough in the force.\r\n", ch );
      return NULL;
   }
   if( fskill->cost != 0 )
   {
      ch->mana -= fskill->cost;
   }
   if( fskill->wait_state )
      ch->wait_state = fskill->wait_state;
   if( ch->force_type == FORCE_JEDI && fskill->type == FORCE_SITH )
   {
      send_to_char( "&RYou feel the hatred grow within you.&G&w\r\n", ch );
      force_send_to_room( ch, NULL, force_parse_string( ch, NULL, "&R$n's eyes glow red.&G&w\r\n" ) );
      ch->force_align -= number_range( 1, 5 );
   }
   if( ch->force_type == FORCE_SITH && fskill->type == FORCE_JEDI )
   {
      send_to_char( "&BYou grimace in pain as you embrace the light side of the force.&G&w\r\n", ch );
      force_send_to_room( ch, NULL, force_parse_string( ch, NULL, "&B$n grimaces in pain.&G&w\r\n" ) );
      ch->force_align += number_range( 1, 5 );
   }
   if( ch->force_align > MAX_FORCE_ALIGN )
      ch->force_align = MAX_FORCE_ALIGN;
   if( ch->force_align < MIN_FORCE_ALIGN )
      ch->force_align = MIN_FORCE_ALIGN;
   return fskill;
}

void update_force(  )
{
   DESCRIPTOR_DATA *i;
   FORCE_SKILL *fskill;
   CHAR_DATA *ch;
   int change;
   for( i = first_descriptor; i; i = i->next )
   {
      if( i->connected || !i->character )
         continue;
      ch = i->original ? i->original : i->character;
      if( IS_NPC( ch ) )
         continue;
      if( ch->force_identified != 1 || !ch->force_level_status )
         continue;
      if( ch->force_disguise_count > 0 )
         ch->force_disguise_count--;
      if( ch->force_type == FORCE_JEDI && ch->alignment < 0 )
      {
         change = number_range( -10, 5 );
         if( change < 0 )
            change = 0;
         else
            send_to_char( "&RYou feel the hatred grow within you.&G&w\r\n", ch );
         ch->force_align -= change;
      }
      else if( ch->force_type == FORCE_JEDI )
      {
         change = number_range( -30, 5 );
         if( change < 0 )
            change = 0;
         ch->force_align += change;
      }
      if( ch->force_type == FORCE_SITH && ch->alignment > 0 )
      {
         change = number_range( -30, 5 );
         if( change < 0 )
            change = 0;
         else
            send_to_char( "&BYou grimace in pain as the light side of the force grows within you.&G&w\r\n", ch );
         ch->force_align += change;
      }
      else if( ch->force_type == FORCE_SITH )
      {
         change = number_range( -10, 5 );
         if( change < 0 )
            change = 0;
         ch->force_align -= change;
      }
      if( ch->force_align > MAX_FORCE_ALIGN )
         ch->force_align = MAX_FORCE_ALIGN;
      if( ch->force_align < MIN_FORCE_ALIGN )
         ch->force_align = MIN_FORCE_ALIGN;
      for( fskill = first_force_skill; fskill; fskill = fskill->next )
      {
         if( fskill->type != FORCE_GENERAL && fskill->type != ch->force_type && ch->force_skill[fskill->index] > 0 )
         {
            change = number_range( 0, 1 );
            if( change != 0 )
            {
               //ch_printf(ch,"You feel your ability decrease in: %s.\r\n",capitalize(fskill->name));
               ch->force_skill[fskill->index] -= change;
               if( ch->force_skill[fskill->index] < 0 )
                  ch->force_skill[fskill->index] = 0;
            }
         }
         if( fskill->type == FORCE_JEDI && ch->force_type == FORCE_JEDI && ch->force_align < 0
             && ch->force_skill[fskill->index] > 0 )
         {
            change = number_range( 0, 1 );
            if( change == 1 )
            {
               //ch_printf(ch,"You feel your ability decrease in: %s.\r\n",capitalize(fskill->name));
               ch->force_skill[fskill->index] -= change;
               if( ch->force_skill[fskill->index] < 0 )
                  ch->force_skill[fskill->index] = 0;
            }
         }
         if( fskill->type == FORCE_SITH && ch->force_type == FORCE_SITH && ch->force_align > 0
             && ch->force_skill[fskill->index] > 0 )
         {
            change = number_range( 0, 1 );
            if( change == 1 )
            {
               //ch_printf(ch,"You feel your ability decrease in: %s.\r\n",capitalize(fskill->name));
               ch->force_skill[fskill->index] -= change;
               if( ch->force_skill[fskill->index] < 0 )
                  ch->force_skill[fskill->index] = 0;
            }
         }
      }
   }
}

int force_promote_ready( CHAR_DATA * ch )
{
   FORCE_SKILL *fskill;
   int count = 0;
   int total = 0;
   for( fskill = first_force_skill; fskill; fskill = fskill->next )
   {
      if( fskill->status == ch->force_level_status )
      {
         count++;
         total += ch->force_skill[fskill->index];
      }
   }
   if( total / count < 50 )
      return 0;
   return 1;
}
void draw_force_line_rev( CHAR_DATA * ch, int length )
{
   int x;

   for( x = 0; x < length; x++ )
   {
      if( ch->force_type == FORCE_JEDI )
      {
         if( x == 0 )
            send_to_char( "&b-", ch );
         else
            send_to_char( "&B=", ch );
      }
      else if( ch->force_type == FORCE_SITH )

      {
         if( x == 0 )
            send_to_char( "&r-", ch );
         else
            send_to_char( "&R=", ch );
      }
      else
      {
         if( x == 0 )
            send_to_char( "&g-", ch );
         else
            send_to_char( "&G=", ch );
      }
   }
   send_to_char( "&z)[::&wo&z::::)", ch );
   send_to_char( "&G&w", ch );
}
void draw_force_line( CHAR_DATA * ch, int length )
{
   int x;
   send_to_char( "&z(::::&wo&z::](", ch );
   for( x = 0; x < length; x++ )
   {
      if( ch->force_type == FORCE_JEDI )
      {
         if( x == length - 1 )
            send_to_char( "&b-", ch );
         else
            send_to_char( "&B=", ch );
      }
      else if( ch->force_type == FORCE_SITH )

      {
         if( x == length - 1 )
            send_to_char( "&r-", ch );
         else
            send_to_char( "&R=", ch );
      }
      else
      {
         if( x == length - 1 )
            send_to_char( "&g-", ch );
         else
            send_to_char( "&G=", ch );
      }
   }
   send_to_char( "&G&w", ch );
}

FORCE_SKILL *get_force_skill( char *argument )
{
   FORCE_SKILL *fskill;
   for( fskill = first_force_skill; fskill; fskill = fskill->next )
      if( nifty_is_name_prefix( argument, fskill->name ) )
         break;
   return fskill;
}

void do_fstat( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   char arg1[MAX_STRING_LENGTH];
   int x;
   argument = one_argument( argument, arg1 );
   fskill = get_force_skill( arg1 );
   if( !fskill )
   {
      send_to_char( "No such skill.\r\n", ch );
      send_to_char( "USAGE: fstat <skill> [mesg]\r\n", ch );
      return;
   }
   ch_printf( ch, "&G&WSkill Name: &G&w%-20s\r\n", fskill->name );
   draw_force_line( ch, 70 );
   send_to_char( "\r\n", ch );
   ch_printf( ch, "&G&WLevel: &G&w%-9d &G&WType: &G&w%-10s &G&WIndex: &G&w%-9d &G&WCost: &G&w%-10d\r\n", fskill->status,
              fskill->type == FORCE_JEDI ? "Jedi" : fskill->type == FORCE_SITH ? "Sith" : "General", fskill->index,
              fskill->cost );
   ch_printf( ch, "&G&WControl: &G&w%-7s &G&WAlter: &G&w%-9s &G&WSense: &G&w%-9s &G&WWaitState: &G&w%-5d\r\n",
              fskill->control ? "Yes" : "No", fskill->alter ? "Yes" : "No", fskill->sense ? "Yes" : "No",
              fskill->wait_state );
   ch_printf( ch, "&G&WDisabled: &G&w%-6s &G&WNotskill: &G&w%-6s &G&WMastertrain: &G&w%-6s\r\n\r\n",
              fskill->disabled ? "Yes" : "No", fskill->notskill ? "Yes" : "No", fskill->mastertrain ? "Yes" : "No",
              fskill->wait_state );
   if( !argument || argument[0] == '\0' )
      return;
   if( strcmp( argument, "mesg" ) )
      return;
   for( x = 0; x < 5; x++ )
   {
      if( fskill->room_effect[x][0] != '\0' || fskill->room_effect[x][0] != '\0' || fskill->room_effect[x][0] != '\0' )
         ch_printf( ch, "Effect Messages [%d]\r\n", x );
      if( fskill->room_effect[x][0] != '\0' )
         ch_printf( ch, "&G&W[&GR&G&W] %s\r\n", force_parse_string( NULL, NULL, fskill->room_effect[x] ) );
      if( fskill->victim_effect[x][0] != '\0' )
         ch_printf( ch, "&G&W[&GV&G&W] %s\r\n", force_parse_string( NULL, NULL, fskill->victim_effect[x] ) );
      if( fskill->ch_effect[x][0] != '\0' )
         ch_printf( ch, "&G&W[&GC&G&W] %s\r\n", force_parse_string( NULL, NULL, fskill->ch_effect[x] ) );
   }
   return;
}

void do_fset( CHAR_DATA * ch, char *argument )
{
   FORCE_SKILL *fskill;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   if( !ch->desc )
   {
      bug( "do_fset: no descriptor", 0 );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   fskill = get_force_skill( arg1 );
   if( fskill && arg2[0] != '\0' )
   {
      if( !strcmp( arg2, "type" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "You have to specify a type: general/jedi/sith\r\n", ch );
            return;
         }
         if( !strcmp( argument, "jedi" ) )
         {
            fskill->type = FORCE_JEDI;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "sith" ) )
         {
            fskill->type = FORCE_SITH;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "general" ) )
         {
            fskill->type = FORCE_GENERAL;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else
         {
            send_to_char( "You have to specify a type: general/jedi/sith\r\n", ch );
            save_forceskill( fskill );
            return;
         }
      }
      else if( !strcmp( arg2, "control" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
         if( !strcmp( argument, "yes" ) || !strcmp( argument, "Yes" ) || !strcmp( argument, "y" )
             || !strcmp( argument, "Y" ) )
         {
            fskill->control = 1;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "no" ) || !strcmp( argument, "No" ) || !strcmp( argument, "n" )
                  || !strcmp( argument, "N" ) )
         {
            fskill->control = 0;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
      }
      else if( !strcmp( arg2, "sense" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
         if( !strcmp( argument, "yes" ) || !strcmp( argument, "Yes" ) || !strcmp( argument, "y" )
             || !strcmp( argument, "Y" ) )
         {
            fskill->sense = 1;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "no" ) || !strcmp( argument, "No" ) || !strcmp( argument, "n" )
                  || !strcmp( argument, "N" ) )
         {
            fskill->sense = 0;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
      }
      else if( !strcmp( arg2, "alter" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
         if( !strcmp( argument, "yes" ) || !strcmp( argument, "Yes" ) || !strcmp( argument, "y" )
             || !strcmp( argument, "Y" ) )
         {
            fskill->alter = 1;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "no" ) || !strcmp( argument, "No" ) || !strcmp( argument, "n" )
                  || !strcmp( argument, "N" ) )
         {
            fskill->alter = 0;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
      }
      else if( !strcmp( arg2, "disabled" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
         if( !strcmp( argument, "yes" ) || !strcmp( argument, "Yes" ) || !strcmp( argument, "y" )
             || !strcmp( argument, "Y" ) )
         {
            fskill->disabled = 1;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "no" ) || !strcmp( argument, "No" ) || !strcmp( argument, "n" )
                  || !strcmp( argument, "N" ) )
         {
            fskill->disabled = 0;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
      }
      else if( !strcmp( arg2, "notskill" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
         if( !strcmp( argument, "yes" ) || !strcmp( argument, "Yes" ) || !strcmp( argument, "y" )
             || !strcmp( argument, "Y" ) )
         {
            fskill->notskill = 1;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "no" ) || !strcmp( argument, "No" ) || !strcmp( argument, "n" )
                  || !strcmp( argument, "N" ) )
         {
            fskill->notskill = 0;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
      }
      else if( !strcmp( arg2, "mastertrain" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
         if( !strcmp( argument, "yes" ) || !strcmp( argument, "Yes" ) || !strcmp( argument, "y" )
             || !strcmp( argument, "Y" ) )
         {
            fskill->mastertrain = 1;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else if( !strcmp( argument, "no" ) || !strcmp( argument, "No" ) || !strcmp( argument, "n" )
                  || !strcmp( argument, "N" ) )
         {
            fskill->mastertrain = 0;
            send_to_char( "Set.\r\n", ch );
            save_forceskill( fskill );
            return;
         }
         else
         {
            send_to_char( "You must specify yes or no.\r\n", ch );
            return;
         }
      }
      else if( !strcmp( arg2, "cost" ) )
      {
         int level;
         level = atoi( argument );
         if( level < 0 )
         {
            send_to_char( "Valid costs are greater than -1.\r\n", ch );
            return;
         }
         fskill->cost = level;
         send_to_char( "Set.\r\n", ch );
         save_forceskill( fskill );
         return;
      }
      else if( !strcmp( arg2, "index" ) )
      {
         int level;
         level = atoi( argument );
         if( level < 0 || level > MAX_FORCE_SKILL - 1 )
         {
            ch_printf( ch, "Valid indices are between 0 and %d", MAX_FORCE_SKILL - 1 );
            return;
         }
         fskill->index = level;
         send_to_char( "Set.\r\n", ch );
         save_forceskill( fskill );
         return;
      }
      else if( !strcmp( arg2, "waitstate" ) )
      {
         int level;
         level = atoi( argument );
         if( level < 1 || level > 3 )
         {
            send_to_char( "Valid waitstates are greater than -1.\r\n", ch );
            return;
         }
         fskill->wait_state = level;
         send_to_char( "Set.\r\n", ch );
         save_forceskill( fskill );
         return;
      }
      else if( !strcmp( arg2, "level" ) )
      {
         int level;
         level = atoi( argument );
         if( level < 1 || level > 3 )
         {
            send_to_char( "Valid levels are 1 thru 3.\r\n", ch );
            return;
         }
         fskill->status = level;
         send_to_char( "Set.\r\n", ch );
         save_forceskill( fskill );
         return;
      }
      else if( !strcmp( arg2, "remove" ) )
      {
         UNLINK( fskill, first_force_skill, last_force_skill, next, prev );
         write_all_forceskills(  );
         send_to_char( "Done.\r\n", ch );
         return;
      }
      else if( !strcmp( arg2, "ch0" ) )
      {
         ch->substate = SUB_FORCE_CH0;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->ch_effect[0] );
         set_editor_desc( ch, fskill->ch_effect[0] );
         return;
      }
      else if( !strcmp( arg2, "ch1" ) )
      {
         ch->substate = SUB_FORCE_CH1;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->ch_effect[1] );
         set_editor_desc( ch, fskill->ch_effect[1] );
         return;
      }
      else if( !strcmp( arg2, "ch2" ) )
      {
         ch->substate = SUB_FORCE_CH2;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->ch_effect[2] );
         set_editor_desc( ch, fskill->ch_effect[2] );
         return;
      }
      else if( !strcmp( arg2, "ch3" ) )
      {
         ch->substate = SUB_FORCE_CH3;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->ch_effect[3] );
         set_editor_desc( ch, fskill->ch_effect[3] );
         return;
      }
      else if( !strcmp( arg2, "ch4" ) )
      {
         ch->substate = SUB_FORCE_CH4;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->ch_effect[4] );
         set_editor_desc( ch, fskill->ch_effect[4] );
         return;
      }
      else if( !strcmp( arg2, "room0" ) )
      {
         ch->substate = SUB_FORCE_ROOM0;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->room_effect[0] );
         set_editor_desc( ch, fskill->room_effect[0] );
         return;
      }
      else if( !strcmp( arg2, "room1" ) )
      {
         ch->substate = SUB_FORCE_ROOM1;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->room_effect[1] );
         set_editor_desc( ch, fskill->room_effect[1] );
         return;
      }
      else if( !strcmp( arg2, "room2" ) )
      {
         ch->substate = SUB_FORCE_ROOM2;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->room_effect[2] );
         set_editor_desc( ch, fskill->room_effect[2] );
         return;
      }
      else if( !strcmp( arg2, "room3" ) )
      {
         ch->substate = SUB_FORCE_ROOM3;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->room_effect[3] );
         set_editor_desc( ch, fskill->room_effect[3] );
         return;
      }
      else if( !strcmp( arg2, "room4" ) )
      {
         ch->substate = SUB_FORCE_ROOM4;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->room_effect[4] );
         set_editor_desc( ch, fskill->room_effect[4] );
         return;
      }
      else if( !strcmp( arg2, "victim0" ) )
      {
         ch->substate = SUB_FORCE_VICTIM0;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->victim_effect[0] );
         set_editor_desc( ch, fskill->victim_effect[0] );
         return;
      }
      else if( !strcmp( arg2, "victim1" ) )
      {
         ch->substate = SUB_FORCE_VICTIM1;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->victim_effect[1] );
         set_editor_desc( ch, fskill->victim_effect[1] );
         return;
      }
      else if( !strcmp( arg2, "victim2" ) )
      {
         ch->substate = SUB_FORCE_VICTIM2;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->victim_effect[2] );
         set_editor_desc( ch, fskill->victim_effect[2] );
         return;
      }
      else if( !strcmp( arg2, "victim3" ) )
      {
         ch->substate = SUB_FORCE_VICTIM3;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->victim_effect[3] );
         set_editor_desc( ch, fskill->victim_effect[3] );
         return;
      }
      else if( !strcmp( arg2, "victim4" ) )
      {
         ch->substate = SUB_FORCE_VICTIM4;
         ch->dest_buf = ch;
         ch->spare_ptr = fskill;
         start_editing( ch, fskill->victim_effect[4] );
         set_editor_desc( ch, fskill->victim_effect[4] );
         return;
      }
   }
   if( !strcmp( arg2, "create" ) )
   {
      char code[MAX_STRING_LENGTH];
      DO_FUN *fun;
      CREATE( fskill, FORCE_SKILL, 1 );
      fskill->name = STRALLOC( arg1 );
      sprintf( code, "fskill_%s", fskill->name );
      fskill->code = STRALLOC( code );
      fun = get_force_skill_function( fskill->code );
      fskill->do_fun = fun;
      fskill->ch_effect[0] = STRALLOC( "\0" );
      fskill->ch_effect[1] = STRALLOC( "\0" );
      fskill->ch_effect[2] = STRALLOC( "\0" );
      fskill->ch_effect[3] = STRALLOC( "\0" );
      fskill->ch_effect[4] = STRALLOC( "\0" );
      fskill->room_effect[0] = STRALLOC( "\0" );
      fskill->room_effect[1] = STRALLOC( "\0" );
      fskill->room_effect[2] = STRALLOC( "\0" );
      fskill->room_effect[3] = STRALLOC( "\0" );
      fskill->room_effect[4] = STRALLOC( "\0" );
      fskill->victim_effect[0] = STRALLOC( "\0" );
      fskill->victim_effect[1] = STRALLOC( "\0" );
      fskill->victim_effect[2] = STRALLOC( "\0" );
      fskill->victim_effect[3] = STRALLOC( "\0" );
      fskill->victim_effect[4] = STRALLOC( "\0" );
      fskill->index = MAX_FORCE_SKILL - 1;
      LINK( fskill, first_force_skill, last_force_skill, next, prev );
      send_to_char( "Created.\r\n", ch );
      write_all_forceskills(  );
      return;
   }
   switch ( ch->substate )
   {
      default:
         bug( "do_fset: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\n\r", ch );
         return;

      case SUB_NONE:
         send_to_char( "USAGE: fset <skill> <field> [<value>]\r\n\r\n", ch );
         send_to_char( "PUT VALID FIELDS HERE\r\n", ch );
         return;
      case SUB_FORCE_CH0:
         fskill = ch->spare_ptr;
         STRFREE( fskill->ch_effect[0] );
         fskill->ch_effect[0] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_CH1:
         fskill = ch->spare_ptr;
         STRFREE( fskill->ch_effect[1] );
         fskill->ch_effect[1] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_CH2:
         fskill = ch->spare_ptr;
         STRFREE( fskill->ch_effect[2] );
         fskill->ch_effect[2] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_CH3:
         fskill = ch->spare_ptr;
         STRFREE( fskill->ch_effect[3] );
         fskill->ch_effect[3] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_CH4:
         fskill = ch->spare_ptr;
         STRFREE( fskill->ch_effect[4] );
         fskill->ch_effect[4] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_ROOM0:
         fskill = ch->spare_ptr;
         STRFREE( fskill->room_effect[0] );
         fskill->room_effect[0] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_ROOM1:
         fskill = ch->spare_ptr;
         STRFREE( fskill->room_effect[1] );
         fskill->room_effect[1] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_ROOM2:
         fskill = ch->spare_ptr;
         STRFREE( fskill->room_effect[2] );
         fskill->room_effect[2] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_ROOM3:
         fskill = ch->spare_ptr;
         STRFREE( fskill->room_effect[3] );
         fskill->room_effect[3] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_ROOM4:
         fskill = ch->spare_ptr;
         STRFREE( fskill->room_effect[4] );
         fskill->room_effect[4] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_VICTIM0:
         fskill = ch->spare_ptr;
         STRFREE( fskill->victim_effect[0] );
         fskill->victim_effect[0] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_VICTIM1:
         fskill = ch->spare_ptr;
         STRFREE( fskill->victim_effect[1] );
         fskill->victim_effect[1] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_VICTIM2:
         fskill = ch->spare_ptr;
         STRFREE( fskill->victim_effect[2] );
         fskill->victim_effect[2] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_VICTIM3:
         fskill = ch->spare_ptr;
         STRFREE( fskill->victim_effect[3] );
         fskill->victim_effect[3] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;

      case SUB_FORCE_VICTIM4:
         fskill = ch->spare_ptr;
         STRFREE( fskill->victim_effect[4] );
         fskill->victim_effect[4] = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forceskill( fskill );
         return;
   }
}

void do_fhstat( CHAR_DATA * ch, char *argument )
{
   FORCE_HELP *fhelp, *ghelp, *shelp, *jhelp;
   char type[MAX_STRING_LENGTH];
   char fname[MAX_STRING_LENGTH];
   char temp[MAX_STRING_LENGTH];
   bool found_general = FALSE;
   bool found_jedi = FALSE;
   bool found_sith = FALSE;
   bool match = FALSE;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "USAGE: fhstat <help file> [jedi/sith/general]\r\n", ch );
      return;
   }
   argument = one_argument( argument, fname );
   strcpy( type, argument );
   for( fhelp = first_force_help; fhelp; fhelp = fhelp->next )
   {
      if( nifty_is_name_prefix( fname, fhelp->name ) )
      {
         if( fhelp->type == FORCE_JEDI )
         {
            found_jedi = TRUE;
            jhelp = fhelp;
         }
         else if( fhelp->type == FORCE_SITH )
         {
            found_sith = TRUE;
            shelp = fhelp;
         }
         else
         {
            found_general = TRUE;
            ghelp = fhelp;
         }
         match = TRUE;
      }
   }
   if( !match )
   {
      send_to_char( "No such help file found.\r\n", ch );
      return;
   }
   if( found_jedi + found_sith + found_general > 1 && strcmp( type, "jedi" ) && strcmp( type, "general" )
       && strcmp( type, "sith" ) )
   {
      send_to_char( "More than one help file was found, you must specify jedi/sith/general.\r\n", ch );
      return;
   }
   if( !strcmp( type, "jedi" ) && !found_jedi )
   {
      send_to_char( "No such help file found.\r\n", ch );
      return;
   }
   if( !strcmp( type, "sith" ) && !found_sith )
   {
      send_to_char( "No such help file found.\r\n", ch );
      return;
   }
   if( !strcmp( type, "general" ) && !found_general )
   {
      send_to_char( "No such help file found.\r\n", ch );
      return;
   }
   if( found_jedi && !strcmp( type, "jedi" ) )
      fhelp = jhelp;
   else if( found_sith && !strcmp( type, "sith" ) )
      fhelp = shelp;
   else
      fhelp = ghelp;
   ch_printf( ch, "Name: %-20.20s Level: %-3d Type: %-10s Skill Number: %-3d\r\n",
              fhelp->name, fhelp->status,
              fhelp->type == FORCE_GENERAL ? "General" : fhelp->type == FORCE_JEDI ? "Jedi" : "Sith", fhelp->skill );
   draw_force_line( ch, 79 );
   send_to_char( "\r\n", ch );
   strcpy( temp, fhelp->desc );
   strrep( temp, "$RN$", "\r\n" );
   strrep( temp, "$RN$", "\r\n" );
   send_to_char( temp, ch );
   return;
}

void do_fhset( CHAR_DATA * ch, char *argument )
{
   FORCE_HELP *fhelp;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   char arg3[MAX_STRING_LENGTH];
   char arg4[MAX_STRING_LENGTH];
   int type = -1;
   if( !ch->desc )
   {
      bug( "do_fset: no descriptor", 0 );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   argument = one_argument( argument, arg4 );
   fhelp = get_force_help( arg1, arg2 );
   if( !strcmp( arg3, "create" ) )
   {
      if( strcmp( arg2, "jedi" ) )
         type = FORCE_JEDI;
      if( strcmp( arg2, "sith" ) )
         type = FORCE_SITH;
      if( strcmp( arg2, "GENERAL" ) )
         type = FORCE_GENERAL;
      if( type == -1 )
      {
         send_to_char( "You must specify a valid type for the help file to create it.\r\n", ch );
         return;
      }
      CREATE( fhelp, FORCE_HELP, 1 );
      fhelp->name = STRALLOC( arg1 );
      fhelp->desc = STRALLOC( "\0" );
      fhelp->type = type;
      LINK( fhelp, first_force_help, last_force_help, next, prev );
      send_to_char( "Created.\r\n", ch );
      write_all_forcehelps(  );
      return;
   }
   if( fhelp && arg3[0] != '\0' )
   {
      if( !strcmp( arg3, "desc" ) )
      {
         ch->substate = SUB_FORCE_HELP;
         ch->dest_buf = ch;
         ch->spare_ptr = fhelp;
         if( !fhelp->desc || fhelp->desc[0] == '\0' )
            fhelp->desc = STRALLOC( "" );
         start_editing( ch, fhelp->desc );
         set_editor_desc( ch, fhelp->desc );
         return;
      }
      else if( !strcmp( arg3, "level" ) )
      {
         type = atoi( arg4 );
         if( type > 3 || type < 1 )
         {
            send_to_char( "The valid level range is 1 to 3.\r\n", ch );
            return;
         }
         fhelp->status = type;
         send_to_char( "Set.\r\n", ch );
         save_forcehelp( fhelp );
         return;
      }
      else if( !strcmp( arg3, "skill" ) )
      {
         type = atoi( arg4 );
         if( type < -1 )
         {
            send_to_char( "The valid skill numbers are above -1.\r\n", ch );
            return;
         }
         fhelp->skill = type;
         send_to_char( "Set.\r\n", ch );
         save_forcehelp( fhelp );
         return;
      }
      else if( !strcmp( arg3, "remove" ) )
      {
         UNLINK( fhelp, first_force_help, last_force_help, next, prev );
         write_all_forcehelps(  );
         send_to_char( "Done.\r\n", ch );
         return;
      }
   }
   switch ( ch->substate )
   {
      default:
         bug( "do_fhset: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\n\r", ch );
         return;

      case SUB_NONE:
         send_to_char( "USAGE: fhset <helpfile> <jedi/sith/general> <field> [value]\r\n\r\n", ch );
         send_to_char( "PUT VALID FIELDS HERE\r\n", ch );
         return;
      case SUB_FORCE_HELP:
         fhelp = ch->spare_ptr;
         STRFREE( fhelp->desc );
         fhelp->desc = STRALLOC( copy_buffer( ch ) );
         stop_editing( ch );
         save_forcehelp( fhelp );
         return;
   }
}

FORCE_HELP *get_force_help( char *fname, char *type )
{
   FORCE_HELP *fhelp, *ghelp, *shelp, *jhelp;
   bool found_general = FALSE;
   bool found_jedi = FALSE;
   bool found_sith = FALSE;
   bool match = FALSE;
   if( !fname || fname == '\0' )
      return NULL;
   if( !type || type == '\0' )
      return NULL;
   for( fhelp = first_force_help; fhelp; fhelp = fhelp->next )
   {
      if( nifty_is_name_prefix( fname, fhelp->name ) )
      {
         if( fhelp->type == FORCE_JEDI )
         {
            found_jedi = TRUE;
            jhelp = fhelp;
         }
         else if( fhelp->type == FORCE_SITH )
         {
            found_sith = TRUE;
            shelp = fhelp;
         }
         else
         {
            found_general = TRUE;
            ghelp = fhelp;
         }
         match = TRUE;
      }
   }
   if( !match )
      return NULL;
   if( found_jedi + found_sith + found_general > 1 && strcmp( type, "jedi" ) && strcmp( type, "general" )
       && strcmp( type, "sith" ) )
      return NULL;;
   if( !strcmp( type, "jedi" ) && !found_jedi )
      return NULL;
   if( !strcmp( type, "sith" ) && !found_sith )
      return NULL;
   if( !strcmp( type, "general" ) && !found_general )
      return NULL;
   if( found_jedi && !strcmp( type, "jedi" ) )
      fhelp = jhelp;
   else if( found_sith && !strcmp( type, "sith" ) )
      fhelp = shelp;
   else
      fhelp = ghelp;
   return fhelp;
}
