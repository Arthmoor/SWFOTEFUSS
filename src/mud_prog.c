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
#include <ctype.h>
#include "mud.h"

bool MOBtrigger;

void uphold_supermob( int *curr_serial, int serial, ROOM_INDEX_DATA **supermob_room, OBJ_DATA *true_supermob_obj )
{
   if( *curr_serial != serial )
   {
      char buf[128];

      if( supermob->in_room != *supermob_room )
      {
         char_from_room( supermob );
         char_to_room( supermob, *supermob_room );
      }

      if( true_supermob_obj && true_supermob_obj != supermob_obj )
      {
          supermob_obj = true_supermob_obj;
          STRFREE( supermob->short_descr );
          STRFREE( supermob->description );
          supermob->short_descr = QUICKLINK( supermob_obj->short_descr );
          snprintf( buf, 128, "Object #%d", supermob_obj->pIndexData->vnum );
          supermob->description = STRALLOC( buf );
      }
      else
      {
         if( !true_supermob_obj )
            supermob_obj = NULL;
         if( supermob->short_descr )
            STRFREE( supermob->short_descr );
         if( supermob->description )
            STRFREE( supermob->description );
         supermob->short_descr = QUICKLINK( (*supermob_room)->name );
         snprintf( buf, 128, "Room #%d", (*supermob_room)->vnum );
         supermob->description = STRALLOC( buf );
      }
      *curr_serial = serial;
   }
   else
      *supermob_room = supermob->in_room;
}

/* Defines by Narn for new mudprog parsing, used as 
   return values from mprog_do_command. */
#define COMMANDOK    1
#define IFTRUE       2
#define IFFALSE      3
#define ORTRUE       4
#define ORFALSE      5
#define FOUNDELSE    6
#define FOUNDENDIF   7
#define IFIGNORED    8
#define ORIGNORED    9

/* Ifstate defines, used to create and access ifstate array
   in mprog_driver. */

/* Moved these to mud.h as I needed two of them for mpsleep -rkb */
int mprog_do_command( char *cmnd, CHAR_DATA * mob, CHAR_DATA * actor,
                      OBJ_DATA * obj, void *vo, CHAR_DATA * rndm, bool ignore, bool ignore_ors );

/*
 *  Mudprogram additions
 */
CHAR_DATA *supermob;
struct act_prog_data *room_act_list;
struct act_prog_data *obj_act_list;
struct act_prog_data *mob_act_list;


/* 
 * Global variables to handle sleeping mud progs. 
 */
MPSLEEP_DATA *first_mpsleep = NULL;
MPSLEEP_DATA *last_mpsleep = NULL;
MPSLEEP_DATA *current_mpsleep = NULL;


/*
 * Local function prototypes
 */

char *mprog_next_command( char *clist );
bool mprog_seval( const char *lhs, char *opr, char *rhs, CHAR_DATA * mob );
bool mprog_veval( int lhs, char *opr, int rhs, CHAR_DATA * mob );
int mprog_do_ifcheck( char *ifcheck, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, CHAR_DATA * rndm );
void mprog_translate( char ch, char *t, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, CHAR_DATA * rndm );
void mprog_driver( char *com_list, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, bool single_step );

bool mprog_keyword_check( const char *argu, const char *argl );


void oprog_wordlist_check( const char *arg, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type, OBJ_DATA * iobj );
void set_supermob( OBJ_DATA * obj );
bool oprog_percent_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type );
void rprog_percent_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type );
void rprog_wordlist_check( const char *arg, CHAR_DATA * mob, CHAR_DATA * actor,
                           OBJ_DATA * obj, void *vo, int type, ROOM_INDEX_DATA * room );

void init_supermob( void )
{
   ROOM_INDEX_DATA *office;

   supermob = create_mobile( get_mob_index( MOB_VNUM_SUPERMOB ) );
   office = get_room_index( ROOM_VNUM_POLY );
   char_to_room( supermob, office );
}

/* Used to get sequential lines of a multi line string (separated by "\r\n")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char *mprog_next_command( char *clist )
{
   char *pointer = clist;

   while( *pointer != '\n' && *pointer != '\0' )
      pointer++;
   if( *pointer == '\n' )
      *pointer++ = '\0';
   if( *pointer == '\r' )
      *pointer++ = '\0';

   return ( pointer );

}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval( const char *lhs, char *opr, char *rhs, CHAR_DATA * mob )
{

   if( !str_cmp( opr, "==" ) )
      return ( bool ) ( !str_cmp( lhs, rhs ) );
   if( !str_cmp( opr, "!=" ) )
      return ( bool ) ( str_cmp( lhs, rhs ) );
   if( !str_cmp( opr, "/" ) )
      return ( bool ) ( !str_infix( rhs, lhs ) );
   if( !str_cmp( opr, "!/" ) )
      return ( bool ) ( str_infix( rhs, lhs ) );

   snprintf( log_buf, MAX_STRING_LENGTH, "Improper MOBprog operator '%s'", opr );
   progbug( log_buf, mob );
   return 0;
}

bool mprog_veval( int lhs, char *opr, int rhs, CHAR_DATA * mob )
{

   if( !str_cmp( opr, "==" ) )
      return ( lhs == rhs );
   if( !str_cmp( opr, "!=" ) )
      return ( lhs != rhs );
   if( !str_cmp( opr, ">" ) )
      return ( lhs > rhs );
   if( !str_cmp( opr, "<" ) )
      return ( lhs < rhs );
   if( !str_cmp( opr, "<=" ) )
      return ( lhs <= rhs );
   if( !str_cmp( opr, ">=" ) )
      return ( lhs >= rhs );
   if( !str_cmp( opr, "&" ) )
      return ( lhs & rhs );
   if( !str_cmp( opr, "|" ) )
      return ( lhs | rhs );

   snprintf( log_buf, MAX_STRING_LENGTH, "Improper MOBprog operator '%s'", opr );
   progbug( log_buf, mob );

   return 0;
}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifcheck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return BERR otherwise return boolean 1,0
 * Redone by Altrag.. kill all that big copy-code that performs the
 * same action on each variable..
 */
int mprog_do_ifcheck( char *ifcheck, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, CHAR_DATA * rndm )
{
   char cvar[MAX_INPUT_LENGTH];
   char chck[MAX_INPUT_LENGTH];
   char opr[MAX_INPUT_LENGTH];
   char rval[MAX_STRING_LENGTH];
   char *point = ifcheck;
   char *pchck = chck;
   CHAR_DATA *chkchar = NULL;
   OBJ_DATA *chkobj = NULL;
   int lhsvl, rhsvl;

   if( !*point )
   {
      progbug( "Null ifcheck", mob );
      return BERR;
   }
   while( *point == ' ' )
      point++;
   while( *point != '(' )
      if( *point == '\0' )
      {
         progbug( "Ifcheck syntax error", mob );
         return BERR;
      }
      else if( *point == ' ' )
         point++;
      else
         *pchck++ = *point++;
   *pchck = '\0';
   point++;
   pchck = cvar;
   while( *point != ')' )
      if( *point == '\0' )
      {
         progbug( "Ifcheck syntax error", mob );
         return BERR;
      }
      else if( *point == ' ' )
         point++;
      else
         *pchck++ = *point++;
   point++;
   *pchck = '\0';

   while( *point == ' ' )
      point++;
   if( !*point )
   {
      opr[0] = '\0';
      rval[0] = '\0';
   }
   else
   {
      pchck = opr;
      while( *point != ' ' && !isalnum( *point ) )
         if( *point == '\0' )
         {
            progbug( "Ifcheck operator without value", mob );
            return BERR;
         }
         else
            *pchck++ = *point++;
      *pchck = '\0';

      while( *point == ' ' )
         point++;
      pchck = rval;
      while( *point != '\0' && *point != '\0' )
         *pchck++ = *point++;
      *pchck = '\0';
   }

   /*
    * chck contains check, cvar is the variable in the (), opr is the
    * * operator if there is one, and rval is the value if there was an
    * * operator.
    */
   if( cvar[0] == '$' )
   {
      switch ( cvar[1] )
      {
         case 'i':
            chkchar = mob;
            break;
         case 'n':
            chkchar = actor;
            break;
         case 't':
            chkchar = ( CHAR_DATA * ) vo;
            break;
         case 'r':
            chkchar = rndm;
            break;
         case 'o':
            chkobj = obj;
            break;
         case 'p':
            chkobj = ( OBJ_DATA * ) vo;
            break;
         default:
            snprintf( rval, MAX_STRING_LENGTH, "Bad argument '%c' to '%s'", cvar[0], chck );
            progbug( rval, mob );
            return BERR;
      }
      if( !chkchar && !chkobj )
         return BERR;
   }
   if( !str_cmp( chck, "rand" ) )
   {
      return ( number_percent(  ) <= atoi( cvar ) );
   }
   if( !str_cmp( chck, "economy" ) )
   {
      int idx = atoi( cvar );
      ROOM_INDEX_DATA *room;

      if( !idx )
      {
         if( !mob->in_room )
         {
            progbug( "'economy' ifcheck: mob in NULL room with no room vnum " "argument", mob );
            return BERR;
         }
         room = mob->in_room;
      }
      else
         room = get_room_index( idx );
      if( !room )
      {
         progbug( "Bad room vnum passed to 'economy'", mob );
         return BERR;
      }
      return mprog_veval( ( ( room->area->high_economy > 0 ) ? 1000000000 : 0 )
                          + room->area->low_economy, opr, atoi( rval ), mob );
   }
   if( !str_cmp( chck, "mobinroom" ) )
   {
      int vnum = atoi( cvar );
      CHAR_DATA *oMob;

      if( vnum < 1 || vnum > 32767 )
      {
         progbug( "Bad vnum to 'mobinroom'", mob );
         return BERR;
      }
      lhsvl = 0;
      for( oMob = mob->in_room->first_person; oMob; oMob = oMob->next_in_room )
         if( IS_NPC( oMob ) && oMob->pIndexData->vnum == vnum )
            lhsvl++;
      rhsvl = atoi( rval );
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "timeskilled" ) )
   {
      MOB_INDEX_DATA *pMob;

      if( chkchar )
         pMob = chkchar->pIndexData;
      else if( !( pMob = get_mob_index( atoi( cvar ) ) ) )
      {
         progbug( "TimesKilled ifcheck: bad vnum", mob );
         return BERR;
      }
      return mprog_veval( pMob->killed, opr, atoi( rval ), mob );
   }
   if( !str_cmp( chck, "ovnumhere" ) )
   {
      OBJ_DATA *pObj;
      int vnum = atoi( cvar );

      if( vnum < 1 || vnum > 32767 )
      {
         progbug( "OvnumHere: bad vnum", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->pIndexData->vnum == vnum )
            lhsvl += pObj->count;
      for( pObj = mob->in_room->first_content; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->pIndexData->vnum == vnum )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "otypehere" ) )
   {
      OBJ_DATA *pObj;
      int type;

      if( is_number( cvar ) )
         type = atoi( cvar );
      else
         type = get_otype( cvar );
      if( type < 0 || type > MAX_ITEM_TYPE )
      {
         progbug( "OtypeHere: bad type", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->item_type == type )
            lhsvl += pObj->count;
      for( pObj = mob->in_room->first_content; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->item_type == type )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "ovnumroom" ) )
   {
      OBJ_DATA *pObj;
      int vnum = atoi( cvar );

      if( vnum < 1 || vnum > 32767 )
      {
         progbug( "OvnumRoom: bad vnum", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->in_room->first_content; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->pIndexData->vnum == vnum )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "otyperoom" ) )
   {
      OBJ_DATA *pObj;
      int type;

      if( is_number( cvar ) )
         type = atoi( cvar );
      else
         type = get_otype( cvar );
      if( type < 0 || type > MAX_ITEM_TYPE )
      {
         progbug( "OtypeRoom: bad type", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->in_room->first_content; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->item_type == type )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH);
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "ovnumcarry" ) )
   {
      OBJ_DATA *pObj;
      int vnum = atoi( cvar );

      if( vnum < 1 || vnum > 32767 )
      {
         progbug( "OvnumCarry: bad vnum", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->pIndexData->vnum == vnum )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "otypecarry" ) )
   {
      OBJ_DATA *pObj;
      int type;

      if( is_number( cvar ) )
         type = atoi( cvar );
      else
         type = get_otype( cvar );
      if( type < 0 || type > MAX_ITEM_TYPE )
      {
         progbug( "OtypeCarry: bad type", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( can_see_obj( mob, pObj ) && pObj->item_type == type )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "ovnumwear" ) )
   {
      OBJ_DATA *pObj;
      int vnum = atoi( cvar );

      if( vnum < 1 || vnum > 32767 )
      {
         progbug( "OvnumWear: bad vnum", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( pObj->wear_loc != WEAR_NONE && can_see_obj( mob, pObj ) && pObj->pIndexData->vnum == vnum )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "otypewear" ) )
   {
      OBJ_DATA *pObj;
      int type;

      if( is_number( cvar ) )
         type = atoi( cvar );
      else
         type = get_otype( cvar );
      if( type < 0 || type > MAX_ITEM_TYPE )
      {
         progbug( "OtypeWear: bad type", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( pObj->wear_loc != WEAR_NONE && can_see_obj( mob, pObj ) && pObj->item_type == type )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "ovnuminv" ) )
   {
      OBJ_DATA *pObj;
      int vnum = atoi( cvar );

      if( vnum < 1 || vnum > 32767 )
      {
         progbug( "OvnumInv: bad vnum", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( pObj->wear_loc == WEAR_NONE && can_see_obj( mob, pObj ) && pObj->pIndexData->vnum == vnum )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( !str_cmp( chck, "otypeinv" ) )
   {
      OBJ_DATA *pObj;
      int type;

      if( is_number( cvar ) )
         type = atoi( cvar );
      else
         type = get_otype( cvar );
      if( type < 0 || type > MAX_ITEM_TYPE )
      {
         progbug( "OtypeInv: bad type", mob );
         return BERR;
      }
      lhsvl = 0;
      for( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
         if( pObj->wear_loc == WEAR_NONE && can_see_obj( mob, pObj ) && pObj->item_type == type )
            lhsvl += pObj->count;
      rhsvl = is_number( rval ) ? atoi( rval ) : -1;
      if( rhsvl < 1 )
         rhsvl = 1;
      if( !*opr )
         strlcpy( opr, "==", MAX_INPUT_LENGTH );
      return mprog_veval( lhsvl, opr, rhsvl, mob );
   }
   if( chkchar )
   {
      if( !str_cmp( chck, "ismobinvis" ) )
      {
         return ( IS_NPC( chkchar ) && IS_SET( chkchar->act, ACT_MOBINVIS ) );
      }
      if( !str_cmp( chck, "mobinvislevel" ) )
      {
         return ( IS_NPC( chkchar ) ? mprog_veval( chkchar->mobinvis, opr, atoi( rval ), mob ) : FALSE );
      }
      if( !str_cmp( chck, "ispc" ) )
      {
         return IS_NPC( chkchar ) ? FALSE : TRUE;
      }
      if( !str_cmp( chck, "isnpc" ) )
      {
         return IS_NPC( chkchar ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "ismounted" ) )
      {
         return ( chkchar->position == POS_MOUNTED );
      }
      if( !str_cmp( chck, "isgood" ) )
      {
         return IS_GOOD( chkchar ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "isneutral" ) )
      {
         return IS_NEUTRAL( chkchar ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "isevil" ) )
      {
         return IS_EVIL( chkchar ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "isfight" ) )
      {
         return who_fighting( chkchar ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "isimmort" ) )
      {
         return ( get_trust( chkchar ) >= LEVEL_IMMORTAL );
      }
      if( !str_cmp( chck, "ischarmed" ) )
      {
         return IS_AFFECTED( chkchar, AFF_CHARM ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "isfollow" ) )
      {
         return ( chkchar->master != NULL && chkchar->master->in_room == chkchar->in_room );
      }
      if( !str_cmp( chck, "isaffected" ) )
      {
         int value = get_aflag( rval );

         if( value < 0 || value > 31 )
         {
            progbug( "Unknown affect being checked", mob );
            return BERR;
         }
         return IS_AFFECTED( chkchar, 1 << value ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "hitprcnt" ) )
      {
         return mprog_veval( chkchar->hit / chkchar->max_hit, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "inroom" ) )
      {
         return mprog_veval( chkchar->in_room->vnum, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "wasinroom" ) )
      {
         return mprog_veval( chkchar->was_in_room->vnum, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "norecall" ) )
      {
         return IS_SET( chkchar->in_room->room_flags, ROOM_NO_RECALL ) ? TRUE : FALSE;
      }
      if( !str_cmp( chck, "sex" ) )
      {
         return mprog_veval( chkchar->sex, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "position" ) )
      {
         return mprog_veval( chkchar->position, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "doingquest" ) )
      {
         return IS_NPC( actor ) ? FALSE : mprog_veval( chkchar->pcdata->quest_number, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "ishelled" ) )
      {
         return IS_NPC( actor ) ? FALSE : mprog_veval( chkchar->pcdata->release_date, opr, atoi( rval ), mob );
      }

      if( !str_cmp( chck, "level" ) )
      {
         return mprog_veval( get_trust( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "goldamt" ) )
      {
         return mprog_veval( chkchar->gold, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "race" ) )
      {
         if( IS_NPC( chkchar ) )
            return mprog_seval( npc_race[chkchar->race], opr, rval, mob );
         return mprog_seval( ( char * )race_table[chkchar->race].race_name, opr, rval, mob );
      }
      if( !str_cmp( chck, "clan" ) )
      {
         if( IS_NPC( chkchar ) || !chkchar->pcdata->clan )
            return FALSE;
         return mprog_seval( chkchar->pcdata->clan->name, opr, rval, mob );
      }
      if( !str_cmp( chck, "council" ) || !str_cmp( chck, "senator" ) )
      {
         SENATE_DATA *senator;

         if( IS_NPC( chkchar ) )
            return FALSE;
         if( IS_IMMORTAL( chkchar ) )
            return TRUE;
         for( senator = first_senator; senator; senator = senator->next )
            if( !str_cmp( chkchar->name, senator->name ) )
               return TRUE;
         return FALSE;
      }
      if( !str_cmp( chck, "clantype" ) )
      {
         if( IS_NPC( chkchar ) || !chkchar->pcdata->clan )
            return FALSE;
         return mprog_veval( chkchar->pcdata->clan->clan_type, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "str" ) )
      {
         return mprog_veval( get_curr_str( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "frc" ) )
      {
         return mprog_veval( get_curr_frc( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "wis" ) )
      {
         return mprog_veval( get_curr_wis( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "int" ) )
      {
         return mprog_veval( get_curr_int( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "dex" ) )
      {
         return mprog_veval( get_curr_dex( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "con" ) )
      {
         return mprog_veval( get_curr_con( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "cha" ) )
      {
         return mprog_veval( get_curr_cha( chkchar ), opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "lck" ) )
      {
         return mprog_veval( get_curr_lck( chkchar ), opr, atoi( rval ), mob );
      }
   }
   if( chkobj )
   {
      if( !str_cmp( chck, "objtype" ) )
      {
         return mprog_veval( chkobj->item_type, opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "objval0" ) )
      {
         return mprog_veval( chkobj->value[0], opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "objval1" ) )
      {
         return mprog_veval( chkobj->value[1], opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "objval2" ) )
      {
         return mprog_veval( chkobj->value[2], opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "objval3" ) )
      {
         return mprog_veval( chkobj->value[3], opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "objval4" ) )
      {
         return mprog_veval( chkobj->value[4], opr, atoi( rval ), mob );
      }
      if( !str_cmp( chck, "objval5" ) )
      {
         return mprog_veval( chkobj->value[5], opr, atoi( rval ), mob );
      }
   }
   /*
    * The following checks depend on the fact that cval[1] can only contain
    * one character, and that NULL checks were made previously. 
    */
   if( !str_cmp( chck, "number" ) )
   {
      if( chkchar )
      {
         if( !IS_NPC( chkchar ) )
            return FALSE;
         lhsvl = ( chkchar == mob ) ? chkchar->gold : chkchar->pIndexData->vnum;
         return mprog_veval( lhsvl, opr, atoi( rval ), mob );
      }
      return mprog_veval( chkobj->pIndexData->vnum, opr, atoi( rval ), mob );
   }
   if( !str_cmp( chck, "name" ) )
   {
      if( chkchar )
         return mprog_seval( chkchar->name, opr, rval, mob );
      return mprog_seval( chkobj->name, opr, rval, mob );
   }

   /*
    * Ok... all the ifchecks are done, so if we didnt find ours then something
    * * odd happened.  So report the bug and abort the MUDprogram (return error)
    */
   progbug( "Unknown ifcheck", mob );
   return BERR;
}

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object 
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */

/*
 *  There's no reason to make the mud crash when a variable's
 *  fubared.  I added some ifs.  I'm willing to trade some 
 *  performance for stability. -Haus
 *
 *  Narn's fubar ***ANNIHILATES*** you!  Hmm, could we add that
 *  as a weapon type? -Narn
 *
 *  Added char_died and obj_extracted checks	-Thoric
 */
void mprog_translate( char ch, char *t, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, CHAR_DATA * rndm )
{
   static const char *he_she[] = { "it", "he", "she" };
   static const char *him_her[] = { "it", "him", "her" };
   static const char *his_her[] = { "its", "his", "her" };
   CHAR_DATA *vict = ( CHAR_DATA * ) vo;
   OBJ_DATA *v_obj = ( OBJ_DATA * ) vo;

   *t = '\0';
   switch ( ch )
   {
      case 'i':
         if( mob && !char_died( mob ) )
         {
            if( mob->name )
               one_argument( mob->name, t );
         }
         else
            strcpy( t, "someone" );
         break;

      case 'I':
         if( mob && !char_died( mob ) )
         {
            if( mob->short_descr )
            {
               strcpy( t, mob->short_descr );
            }
            else
            {
               strcpy( t, "someone" );
            }
         }
         else
            strcpy( t, "someone" );
         break;

      case 'n':
         if( actor && !char_died( actor ) )
         {
            if( can_see( mob, actor ) )
               one_argument( actor->name, t );
            else
               strcpy( t, "someone" );
            if( !IS_NPC( actor ) )
               *t = UPPER( *t );
         }
         else
            strcpy( t, "someone" );
         break;

      case 'N':
         if( actor && !char_died( actor ) )
         {
            if( can_see( mob, actor ) )
               if( IS_NPC( actor ) )
                  strcpy( t, actor->short_descr );
               else
               {
                  strcpy( t, actor->name );
                  strcat( t, actor->pcdata->title );
               }
            else
               strcpy( t, "someone" );
         }
         else
            strcpy( t, "someone" );
         break;

      case 't':
         if( vict && !char_died( vict ) )
         {
            one_argument( vict->name, t );
            if( !IS_NPC( vict ) )
               *t = UPPER( *t );
         }
         else
            strcpy( t, "someone" );

         break;

      case 'T':
         if( vict && !char_died( vict ) )
         {
            if( can_see( mob, vict ) )
               if( IS_NPC( vict ) )
                  strcpy( t, vict->short_descr );
               else
               {
                  strcpy( t, vict->name );
                  strcat( t, " " );
                  strcat( t, vict->pcdata->title );
               }
            else
               strcpy( t, "someone" );
         }
         else
            strcpy( t, "someone" );
         break;

      case 'r':
         if( rndm && !char_died( rndm ) )
         {
            one_argument( rndm->name, t );
            if( !IS_NPC( rndm ) )
            {
               *t = UPPER( *t );
            }
         }
         else
            strcpy( t, "someone" );
         break;

      case 'R':
         if( rndm && !char_died( rndm ) )
         {
            if( can_see( mob, rndm ) )
               if( IS_NPC( rndm ) )
                  strcpy( t, rndm->short_descr );
               else
               {
                  strcpy( t, rndm->name );
                  strcat( t, " " );
                  strcat( t, rndm->pcdata->title );
               }
            else
               strcpy( t, "someone" );
         }
         else
            strcpy( t, "someone" );
         break;

      case 'e':
         if( actor && !char_died( actor ) )
         {
            can_see( mob, actor ) ? strcpy( t, he_she[actor->sex] ) : strcpy( t, "someone" );
         }
         else
            strcpy( t, "it" );
         break;

      case 'm':
         if( actor && !char_died( actor ) )
         {
            can_see( mob, actor ) ? strcpy( t, him_her[actor->sex] ) : strcpy( t, "someone" );
         }
         else
            strcpy( t, "it" );
         break;

      case 's':
         if( actor && !char_died( actor ) )
         {
            can_see( mob, actor ) ? strcpy( t, his_her[actor->sex] ) : strcpy( t, "someone's" );
         }
         else
            strcpy( t, "its'" );
         break;

      case 'E':
         if( vict && !char_died( vict ) )
         {
            can_see( mob, vict ) ? strcpy( t, he_she[vict->sex] ) : strcpy( t, "someone" );
         }
         else
            strcpy( t, "it" );
         break;

      case 'M':
         if( vict && !char_died( vict ) )
         {
            can_see( mob, vict ) ? strcpy( t, him_her[vict->sex] ) : strcpy( t, "someone" );
         }
         else
            strcpy( t, "it" );
         break;

      case 'S':
         if( vict && !char_died( vict ) )
         {
            can_see( mob, vict ) ? strcpy( t, his_her[vict->sex] ) : strcpy( t, "someone's" );
         }
         else
            strcpy( t, "its'" );
         break;

      case 'j':
         if( mob && !char_died( mob ) )
         {
            strcpy( t, he_she[mob->sex] );
         }
         else
         {
            strcpy( t, "it" );
         }
         break;

      case 'k':
         if( mob && !char_died( mob ) )
         {
            strcpy( t, him_her[mob->sex] );
         }
         else
         {
            strcpy( t, "it" );
         }
         break;

      case 'l':
         if( mob && !char_died( mob ) )
         {
            strcpy( t, his_her[mob->sex] );
         }
         else
         {
            strcpy( t, "it" );
         }
         break;

      case 'J':
         if( rndm && !char_died( rndm ) )
         {
            can_see( mob, rndm ) ? strcpy( t, he_she[rndm->sex] ) : strcpy( t, "someone" );
         }
         else
            strcpy( t, "it" );
         break;

      case 'K':
         if( rndm && !char_died( rndm ) )
         {
            can_see( mob, rndm ) ? strcpy( t, him_her[rndm->sex] ) : strcpy( t, "someone's" );
         }
         else
            strcpy( t, "its'" );
         break;

      case 'L':
         if( rndm && !char_died( rndm ) )
         {
            can_see( mob, rndm ) ? strcpy( t, his_her[rndm->sex] ) : strcpy( t, "someone" );
         }
         else
            strcpy( t, "its" );
         break;

      case 'o':
         if( obj && !obj_extracted( obj ) )
         {
            can_see_obj( mob, obj ) ? one_argument( obj->name, t ) : strcpy( t, "something" );
         }
         else
            strcpy( t, "something" );
         break;

      case 'O':
         if( obj && !obj_extracted( obj ) )
         {
            can_see_obj( mob, obj ) ? strcpy( t, obj->short_descr ) : strcpy( t, "something" );
         }
         else
            strcpy( t, "something" );
         break;

      case 'p':
         if( v_obj && !obj_extracted( v_obj ) )
         {
            can_see_obj( mob, v_obj ) ? one_argument( v_obj->name, t ) : strcpy( t, "something" );
         }
         else
            strcpy( t, "something" );
         break;

      case 'P':
         if( v_obj && !obj_extracted( v_obj ) )
         {
            can_see_obj( mob, v_obj ) ? strcpy( t, v_obj->short_descr ) : strcpy( t, "something" );
         }
         else
            strcpy( t, "something" );
         break;

      case 'q':
         if( actor && !char_died( actor ) )
         {
            one_argument( actor->name, t );
            if( !IS_NPC( actor ) )
            {
               strcpy( t, PERS( actor, mob ) );
               *t = UPPER( *t );
            }
         }
         else
            strcpy( t, "someone" );
         break;

      case 'Q':
         if( actor && !char_died( actor ) )
         {
            if( can_see( mob, actor ) )
               if( IS_NPC( actor ) )
                  strcpy( t, actor->short_descr );
               else
               {
                  strcpy( t, PERS( actor, mob ) );
               }
            else
               strcpy( t, "someone" );
         }
         else
            strcpy( t, "someone" );
         break;


      case 'a':
         if( obj && !obj_extracted( obj ) )
         {
            strcpy( t, aoran( obj->name ) );
/*
          switch ( *( obj->name ) )
	  {
	    case 'a': case 'e': case 'i':
            case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
*/
         }
         else
            strcpy( t, "a" );
         break;

      case 'A':
         if( v_obj && !obj_extracted( v_obj ) )
         {
            strcpy( t, aoran( v_obj->name ) );
         }
         else
            strcpy( t, "a" );
         break;

      case '$':
         strcpy( t, "$" );
         break;

      default:
         progbug( "Bad $var", mob );
         break;
   }
   return;
}

/*  The main focus of the MOBprograms.  This routine is called 
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 *
 *  This function rewritten by Narn for Realms of Despair, Dec/95.
 *
 */
void mprog_driver( char *com_list, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, bool single_step )
{
   char tmpcmndlst[MAX_STRING_LENGTH];
   char *command_list;
   char *cmnd;
   CHAR_DATA *rndm = NULL;
   CHAR_DATA *vch = NULL;
   int count = 0;
   int count2 = 0;
   int ignorelevel = 0;
   int iflevel, result;
   bool ifstate[MAX_IFS][DO_ELSE + 1];
   static int prog_nest;
   MPSLEEP_DATA *mpsleep = NULL;
   char arg[MAX_INPUT_LENGTH];
   static int serial;
   int curr_serial;
   ROOM_INDEX_DATA *supermob_room;
   OBJ_DATA *true_supermob_obj;
   bool rprog_oprog = ( mob == supermob );

   if( rprog_oprog )
   {
      serial++;
      supermob_room = mob->in_room;
      true_supermob_obj = supermob_obj;
   }
   else
      true_supermob_obj = NULL, supermob_room = NULL;
   curr_serial = serial;

   if( IS_AFFECTED( mob, AFF_CHARM ) )
      return;

   /*
    * Next couple of checks stop program looping. -- Altrag 
    */
   if( mob == actor )
   {
      progbug( "triggering oneself.", mob );
      return;
   }

   if( ++prog_nest > MAX_PROG_NEST )
   {
      progbug( "max_prog_nest exceeded.", mob );
      --prog_nest;
      return;
   }

   /*
    * Make sure all ifstate bools are set to FALSE 
    */
   for( iflevel = 0; iflevel < MAX_IFS; iflevel++ )
   {
      for( count = 0; count < DO_ELSE; count++ )
      {
         ifstate[iflevel][count] = FALSE;
      }
   }

   iflevel = 0;

   /*
    * get a random visible player who is in the room with the mob.
    *
    *  If there isn't a random player in the room, rndm stays NULL.
    *  If you do a $r, $R, $j, or $k with rndm = NULL, you'll crash
    *  in mprog_translate.
    *
    *  Adding appropriate error checking in mprog_translate.
    *    -Haus
    *
    * This used to ignore players MAX_LEVEL - 3 and higher (standard
    * Merc has 4 immlevels).  Thought about changing it to ignore all
    * imms, but decided to just take it out.  If the mob can see you, 
    * you may be chosen as the random player. -Narn
    *
    */

   count = 0;
   for( vch = mob->in_room->first_person; vch; vch = vch->next_in_room )
      if( !IS_NPC( vch ) )
      {
         if( number_range( 0, count ) == 0 )
            rndm = vch;
         count++;
      }

   strlcpy( tmpcmndlst, com_list, MAX_STRING_LENGTH );
   command_list = tmpcmndlst;

   /*
    * mpsleep - Restore the environment -rkb 
    */
   if( current_mpsleep )
   {
      ignorelevel = current_mpsleep->ignorelevel;
      iflevel = current_mpsleep->iflevel;

      if( single_step )
         mob->mpscriptpos = 0;

      for( count = 0; count < MAX_IFS; count++ )
      {
         for( count2 = 0; count2 <= DO_ELSE; ++count2 )
            ifstate[count][count2] = current_mpsleep->ifstate[count][count2];
      }
      current_mpsleep = NULL;
   }

   if( single_step )
   {
      if( mob->mpscriptpos > strlen( tmpcmndlst ) )
         mob->mpscriptpos = 0;
      else
         command_list += mob->mpscriptpos;
      if( *command_list == '\0' )
      {
         command_list = tmpcmndlst;
         mob->mpscriptpos = 0;
      }
   }

   /*
    * From here on down, the function is all mine.  The original code
    * did not support nested ifs, so it had to be redone.  The max 
    * logiclevel (MAX_IFS) is defined at the beginning of this file, 
    * use it to increase/decrease max allowed nesting.  -Narn 
    */

   while( TRUE )
   {
      /*
       * With these two lines, cmnd becomes the current line from the prog,
       * and command_list becomes everything after that line. 
       */
      cmnd = command_list;
      command_list = mprog_next_command( command_list );

      /*
       * Are we at the end? 
       */
      if( cmnd[0] == '\0' )
      {
         if( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] )
         {
            progbug( "Missing endif", mob );
         }
         --prog_nest;
         return;
      }

      /*
       * mpsleep - Check if we should sleep -rkb 
       */
      if( !str_prefix( "mpsleep", cmnd ) )
      {
         CREATE( mpsleep, MPSLEEP_DATA, 1 );

         /*
          * State variables 
          */
         mpsleep->ignorelevel = ignorelevel;
         mpsleep->iflevel = iflevel;
         for( count = 0; count < MAX_IFS; count++ )
         {
            for( count2 = 0; count2 <= DO_ELSE; ++count2 )
               mpsleep->ifstate[count][count2] = ifstate[count][count2];
         }

         /*
          * Driver arguments 
          */
         mpsleep->com_list = STRALLOC( command_list );
         mpsleep->mob = mob;
         mpsleep->actor = actor;
         mpsleep->obj = obj;
         mpsleep->vo = vo;
         mpsleep->single_step = single_step;

         /*
          * Time to sleep 
          */
         cmnd = one_argument( cmnd, arg );
         cmnd = one_argument( cmnd, arg );
         if( arg[0] == '\0' )
            mpsleep->timer = 4;
         else
            mpsleep->timer = atoi( arg );

         if( mpsleep->timer < 1 )
         {
            progbug( "mpsleep - bad arg, using default", mob );
            mpsleep->timer = 4;
         }

         /*
          * Save type of prog, room, object or mob 
          */
         if( mpsleep->mob->pIndexData->vnum == 3 )
         {
            if( !str_prefix( "Room", mpsleep->mob->description ) )
            {
               mpsleep->type = MP_ROOM;
               mpsleep->room = mpsleep->mob->in_room;
            }
            else if( !str_prefix( "Object", mpsleep->mob->description ) )
               mpsleep->type = MP_OBJ;
         }
         else
            mpsleep->type = MP_MOB;

         LINK( mpsleep, first_mpsleep, last_mpsleep, next, prev );

         --prog_nest;
         return;
      }

      /*
       * Evaluate/execute the command, check what happened. 
       */
      result = mprog_do_command( cmnd, mob, actor, obj, vo, rndm,
                                 ( ifstate[iflevel][IN_IF] && !ifstate[iflevel][DO_IF] )
                                 || ( ifstate[iflevel][IN_ELSE] && !ifstate[iflevel][DO_ELSE] ), ( ignorelevel > 0 ) );

      if( rprog_oprog )
         uphold_supermob( &curr_serial, serial, &supermob_room, true_supermob_obj );

      /*
       * Script prog support  -Thoric 
       */
      if( single_step )
      {
         mob->mpscriptpos = command_list - tmpcmndlst;
         --prog_nest;
         return;
      }

      /*
       * This is the complicated part.  Act on the returned value from
       * mprog_do_command according to the current logic state. 
       */
      switch ( result )
      {
         case COMMANDOK:
#ifdef DEBUG
            log_string( "COMMANDOK" );
#endif
            /*
             * Ok, this one's a no-brainer. 
             */
            continue;
            break;

         case IFTRUE:
#ifdef DEBUG
            log_string( "IFTRUE" );
#endif
            /*
             * An if was evaluated and found true.  Note that we are in an
             * if section and that we want to execute it. 
             */
            iflevel++;
            if( iflevel == MAX_IFS )
            {
               progbug( "Maximum nested ifs exceeded", mob );
               --prog_nest;
               return;
            }

            ifstate[iflevel][IN_IF] = TRUE;
            ifstate[iflevel][DO_IF] = TRUE;
            break;

         case IFFALSE:
#ifdef DEBUG
            log_string( "IFFALSE" );
#endif
            /*
             * An if was evaluated and found false.  Note that we are in an
             * if section and that we don't want to execute it unless we find
             * an or that evaluates to true. 
             */
            iflevel++;
            if( iflevel == MAX_IFS )
            {
               progbug( "Maximum nested ifs exceeded", mob );
               --prog_nest;
               return;
            }
            ifstate[iflevel][IN_IF] = TRUE;
            ifstate[iflevel][DO_IF] = FALSE;
            break;

         case ORTRUE:
#ifdef DEBUG
            log_string( "ORTRUE" );
#endif
            /*
             * An or was evaluated and found true.  We should already be in an
             * if section, so note that we want to execute it. 
             */
            if( !ifstate[iflevel][IN_IF] )
            {
               progbug( "Unmatched or", mob );
               --prog_nest;
               return;
            }
            ifstate[iflevel][DO_IF] = TRUE;
            break;

         case ORFALSE:
#ifdef DEBUG
            log_string( "ORFALSE" );
#endif
            /*
             * An or was evaluated and found false.  We should already be in an
             * if section, and we don't need to do much.  If the if was true or
             * there were/will be other ors that evaluate(d) to true, they'll set
             * do_if to true. 
             */
            if( !ifstate[iflevel][IN_IF] )
            {
               progbug( "Unmatched or", mob );
               --prog_nest;
               return;
            }
            continue;
            break;

         case FOUNDELSE:
#ifdef DEBUG
            log_string( "FOUNDELSE" );
#endif
            /*
             * Found an else.  Make sure we're in an if section, bug out if not.
             * If this else is not one that we wish to ignore, note that we're now 
             * in an else section, and look at whether or not we executed the if 
             * section to decide whether to execute the else section.  Ca marche 
             * bien. 
             */
            if( ignorelevel > 0 )
               continue;

            if( ifstate[iflevel][IN_ELSE] )
            {
               progbug( "Found else in an else section", mob );
               --prog_nest;
               return;
            }
            if( !ifstate[iflevel][IN_IF] )
            {
               progbug( "Unmatched else", mob );
               --prog_nest;
               return;
            }

            ifstate[iflevel][IN_ELSE] = TRUE;
            ifstate[iflevel][DO_ELSE] = !ifstate[iflevel][DO_IF];
            ifstate[iflevel][IN_IF] = FALSE;
            ifstate[iflevel][DO_IF] = FALSE;

            break;

         case FOUNDENDIF:
#ifdef DEBUG
            log_string( "FOUNDENDIF" );
#endif
            /*
             * Hmm, let's see... FOUNDENDIF must mean that we found an endif.
             * So let's make sure we were expecting one, return if not.  If this
             * endif matches the if or else that we're executing, note that we are 
             * now no longer executing an if.  If not, keep track of what we're 
             * ignoring. 
             */
            if( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
               progbug( "Unmatched endif", mob );
               --prog_nest;
               return;
            }

            if( ignorelevel > 0 )
            {
               ignorelevel--;
               continue;
            }

            ifstate[iflevel][IN_IF] = FALSE;
            ifstate[iflevel][DO_IF] = FALSE;
            ifstate[iflevel][IN_ELSE] = FALSE;
            ifstate[iflevel][DO_ELSE] = FALSE;

            iflevel--;
            break;

         case IFIGNORED:
#ifdef DEBUG
            log_string( "IFIGNORED" );
#endif
            if( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
               progbug( "Parse error, ignoring if while not in if or else", mob );
               --prog_nest;
               return;
            }
            ignorelevel++;
            break;

         case ORIGNORED:
#ifdef DEBUG
            log_string( "ORIGNORED" );
#endif
            if( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
               progbug( "Unmatched or", mob );
               --prog_nest;
               return;
            }
            if( ignorelevel == 0 )
            {
               progbug( "Parse error, mistakenly ignoring or", mob );
               --prog_nest;
               return;
            }

            break;

         case BERR:
#ifdef DEBUG
            log_string( "BERR" );
#endif
            --prog_nest;
            return;
            break;
      }
   }
   --prog_nest;
   return;
}

/* This function replaces mprog_process_cmnd.  It is called from 
 * mprog_driver, once for each line in a mud prog.  This function
 * checks what the line is, executes if/or checks and calls interpret
 * to perform the the commands.  Written by Narn, Dec 95.
 */
int mprog_do_command( char *cmnd, CHAR_DATA * mob, CHAR_DATA * actor,
                      OBJ_DATA * obj, void *vo, CHAR_DATA * rndm, bool ignore, bool ignore_ors )
{
   char firstword[MAX_INPUT_LENGTH];
   char *ifcheck;
   char buf[MAX_INPUT_LENGTH];
   char tmp[MAX_INPUT_LENGTH];
   char *point, *str, *i;
   int validif;

   /*
    * Isolate the first word of the line, it gives us a clue what
    * we want to do. 
    */
   ifcheck = one_argument( cmnd, firstword );

   if( !str_cmp( firstword, "if" ) )
   {
      /*
       * Ok, we found an if.  According to the boolean 'ignore', either
       * ignore the ifcheck and report that back to mprog_driver or do
       * the ifcheck and report whether it was successful. 
       */
      if( ignore )
         return IFIGNORED;
      else
         validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

      if( validif == 1 )
         return IFTRUE;

      if( validif == 0 )
         return IFFALSE;

      return BERR;
   }

   if( !str_cmp( firstword, "or" ) )
   {
      /*
       * Same behavior as with ifs, but use the boolean 'ignore_ors' to
       * decide which way to go. 
       */
      if( ignore_ors )
         return ORIGNORED;
      else
         validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

      if( validif == 1 )
         return ORTRUE;

      if( validif == 0 )
         return ORFALSE;

      return BERR;
   }

   /*
    * For else and endif, just report back what we found.  Mprog_driver
    * keeps track of logiclevels. 
    */
   if( !str_cmp( firstword, "else" ) )
   {
      return FOUNDELSE;
   }

   if( !str_cmp( firstword, "endif" ) )
   {
      return FOUNDENDIF;
   }

   /*
    * Ok, didn't find an if, an or, an else or an endif.  
    * If the command is in an if or else section that is not to be 
    * performed, the boolean 'ignore' is set to true and we just 
    * return.  If not, we try to execute the command. 
    */

   if( ignore )
      return COMMANDOK;

   /*
    * If the command is 'break', that's all folks. 
    */
   if( !str_cmp( firstword, "break" ) )
      return BERR;

   point = buf;
   str = cmnd;

   /*
    * This chunk of code taken from mprog_process_cmnd. 
    */
   while( *str != '\0' )
   {
      if( *str != '$' )
      {
         *point++ = *str++;
         continue;
      }
      str++;
      mprog_translate( *str, tmp, mob, actor, obj, vo, rndm );
      i = tmp;
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;
   }
   *point = '\0';

   interpret( mob, buf );

   /*
    * If the mob is mentally unstable and does things like fireball
    * itself, let's make sure it's still alive. 
    */
   if( char_died( mob ) )
   {
      return BERR;
   }

   return COMMANDOK;
}

/***************************************************************************
 * Global function code and brief comments.
 */

/* See if there's any mud programs waiting to be continued -rkb */
void mpsleep_update(  )
{
   MPSLEEP_DATA *mpsleep;
   MPSLEEP_DATA *tmpMpsleep;
   bool delete_it;

   mpsleep = first_mpsleep;
   while( mpsleep )
   {
      delete_it = FALSE;

      if( mpsleep->mob )
         delete_it = char_died( mpsleep->mob );

      if( mpsleep->actor && !delete_it )
         delete_it = char_died( mpsleep->actor );

      if( mpsleep->obj && !delete_it )
         delete_it = obj_extracted( mpsleep->obj );

      if( delete_it )
      {

         tmpMpsleep = mpsleep;
         mpsleep = mpsleep->next;
         STRFREE( tmpMpsleep->com_list );
         UNLINK( tmpMpsleep, first_mpsleep, last_mpsleep, next, prev );
         DISPOSE( tmpMpsleep );

         continue;
      }

      mpsleep = mpsleep->next;
   }

   mpsleep = first_mpsleep;
   while( mpsleep )  /* Find progs to continue */
   {
      if( --mpsleep->timer <= 0 )
      {
         current_mpsleep = mpsleep;

         if( mpsleep->type == MP_ROOM )
            rset_supermob( mpsleep->room );
         else if( mpsleep->type == MP_OBJ )
            set_supermob( mpsleep->obj );

         mprog_driver( mpsleep->com_list, mpsleep->mob, mpsleep->actor, mpsleep->obj, mpsleep->vo, mpsleep->single_step );

         release_supermob(  );

         tmpMpsleep = mpsleep;
         mpsleep = mpsleep->next;
         STRFREE( tmpMpsleep->com_list );
         UNLINK( tmpMpsleep, first_mpsleep, last_mpsleep, next, prev );
         DISPOSE( tmpMpsleep );

         continue;
      }

      mpsleep = mpsleep->next;
   }
}

bool mprog_keyword_check( const char *argu, const char *argl )
{
   char word[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   unsigned int i;
   char *arg, *arglist;
   char *start, *end;

   strlcpy( arg1, strlower( argu ), MAX_INPUT_LENGTH );
   arg = arg1;
   strlcpy( arg2, strlower( argl ), MAX_INPUT_LENGTH );
   arglist = arg2;

   for( i = 0; i < strlen( arglist ); i++ )
      arglist[i] = LOWER( arglist[i] );
   for( i = 0; i < strlen( arg ); i++ )
      arg[i] = LOWER( arg[i] );
   if( ( arglist[0] == 'p' ) && ( arglist[1] == ' ' ) )
   {
      arglist += 2;
      while( ( start = strstr( arg, arglist ) ) )
         if( ( start == arg || *( start - 1 ) == ' ' )
             && ( *( end = start + strlen( arglist ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
            return TRUE;
         else
            arg = start + 1;
   }
   else
   {
      arglist = one_argument( arglist, word );
      for( ; word[0] != '\0'; arglist = one_argument( arglist, word ) )
         while( ( start = strstr( arg, word ) ) )
            if( ( start == arg || *( start - 1 ) == ' ' )
                && ( *( end = start + strlen( word ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
               return TRUE;
            else
               arg = start + 1;
   }
/*    bug( "don't match" ); */
   return FALSE;
}

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( const char *arg, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type )
{
   char temp1[MAX_STRING_LENGTH];
   char temp2[MAX_INPUT_LENGTH];
   char word[MAX_INPUT_LENGTH];
   MPROG_DATA *mprg;
   char *list;
   char *start;
   char *dupl;
   char *end;
   unsigned int i;

   for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
      if( mprg->type & type )
      {
         strlcpy( temp1, mprg->arglist, MAX_STRING_LENGTH );
         list = temp1;
         for( i = 0; i < strlen( list ); i++ )
            list[i] = LOWER( list[i] );
         strlcpy( temp2, arg, MAX_INPUT_LENGTH );
         dupl = temp2;
         for( i = 0; i < strlen( dupl ); i++ )
            dupl[i] = LOWER( dupl[i] );
         if( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
         {
            list += 2;
            while( ( start = strstr( dupl, list ) ) )
               if( ( start == dupl || *( start - 1 ) == ' ' )
                   && ( *( end = start + strlen( list ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
               {
                  mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                  break;
               }
               else
                  dupl = start + 1;
         }
         else
         {
            list = one_argument( list, word );
            for( ; word[0] != '\0'; list = one_argument( list, word ) )
               while( ( start = strstr( dupl, word ) ) )
                  if( ( start == dupl || *( start - 1 ) == ' ' )
                      && ( *( end = start + strlen( word ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
                  {
                     mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                     break;
                  }
                  else
                     dupl = start + 1;
         }
      }
   return;
}

void mprog_percent_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type )
{
   MPROG_DATA *mprg;

   for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
      if( ( mprg->type & type ) && ( number_percent(  ) <= atoi( mprg->arglist ) ) )
      {
         mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
         if( type != GREET_PROG && type != ALL_GREET_PROG )
            break;
      }

   return;

}

void mprog_time_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type )
{
   MPROG_DATA *mprg;
   bool trigger_time;

   for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
   {
      trigger_time = ( time_info.hour == atoi( mprg->arglist ) );

      if( !trigger_time )
      {
         if( mprg->triggered )
            mprg->triggered = FALSE;
         continue;
      }

      if( ( mprg->type & type ) && ( ( !mprg->triggered ) || ( mprg->type == HOUR_PROG ) ) )
      {
         mprg->triggered = TRUE;
         mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
      }
   }
   return;
}


void mob_act_add( CHAR_DATA * mob )
{
   struct act_prog_data *runner, *tmp_mal;

   for( runner = mob_act_list; runner; runner = runner->next )
      if( runner->vo == mob )
         return;

   CREATE( runner, struct act_prog_data, 1 );
   runner->vo = mob;
   runner->next = NULL;
   /*
    * The head of the list is being changed in
    * aggr_update, So append to the end of the list 
    * instead. -Druid
    */
   if( mob_act_list )
   {
      tmp_mal = mob_act_list;

      while( tmp_mal->next != NULL )
         tmp_mal = tmp_mal->next;

      /*
       * put at the end 
       */
      tmp_mal->next = runner;
   }
   else
      mob_act_list = runner;
}


/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger( char *buf, CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj, void *vo )
{
   MPROG_ACT_LIST *tmp_act, *tmp_mal;
   MPROG_DATA *mprg;
   bool found = FALSE;

   if( IS_NPC( mob ) && IS_SET( mob->pIndexData->progtypes, ACT_PROG ) )
   {
      /*
       * Don't let a mob trigger itself, nor one instance of a mob
       * trigger another instance. 
       */
      if( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
         return;

      /*
       * make sure this is a matching trigger 
       */
      for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
         if( mprg->type & ACT_PROG && mprog_keyword_check( buf, mprg->arglist ) )
         {
            found = TRUE;
            break;
         }
      if( !found )
         return;

      CREATE( tmp_act, MPROG_ACT_LIST, 1 );
/* 
 * Losing the head of the list -Druid
 */
      if( mob->mpactnum > 0 )
      {
         tmp_mal = mob->mpact;

         while( tmp_mal->next != NULL )
            tmp_mal = tmp_mal->next;

         /*
          * Put at the end 
          */
         tmp_mal->next = tmp_act;
      }
      else
         mob->mpact = tmp_act;

      tmp_act->next = NULL;
      tmp_act->buf = strdup( buf );
      tmp_act->ch = ch;
      tmp_act->obj = obj;
      tmp_act->vo = vo;
      mob->mpactnum++;
      mob_act_add( mob );
   }
   return;
}

void mprog_bribe_trigger( CHAR_DATA * mob, CHAR_DATA * ch, int amount )
{
   char buf[MAX_STRING_LENGTH];
   MPROG_DATA *mprg;
   OBJ_DATA *obj;

   if( IS_NPC( mob ) && ( mob->pIndexData->progtypes & BRIBE_PROG ) )
   {
      /*
       * Don't let a mob trigger itself, nor one instance of a mob
       * trigger another instance. 
       */
      if( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
         return;

      obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
      snprintf( buf, MAX_STRING_LENGTH, obj->short_descr, amount );
      STRFREE( obj->short_descr );
      obj->short_descr = STRALLOC( buf );
      obj->value[0] = amount;
      obj = obj_to_char( obj, mob );
      mob->gold -= amount;

      for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
         if( ( mprg->type & BRIBE_PROG ) && ( amount >= atoi( mprg->arglist ) ) )
         {
            mprog_driver( mprg->comlist, mob, ch, obj, NULL, FALSE );
            break;
         }
   }
   return;
}

void mprog_death_trigger( CHAR_DATA * killer, CHAR_DATA * mob )
{
   if( IS_NPC( mob ) && killer != mob && ( mob->pIndexData->progtypes & DEATH_PROG ) )
   {
      mprog_percent_check( mob, killer, NULL, NULL, DEATH_PROG );
   }
   death_cry( mob );
   return;
}

void mprog_entry_trigger( CHAR_DATA * mob )
{

   if( IS_NPC( mob ) && ( mob->pIndexData->progtypes & ENTRY_PROG ) )
      mprog_percent_check( mob, NULL, NULL, NULL, ENTRY_PROG );

   return;

}

void mprog_fight_trigger( CHAR_DATA * mob, CHAR_DATA * ch )
{

   if( IS_NPC( mob ) && ( mob->pIndexData->progtypes & FIGHT_PROG ) )
      mprog_percent_check( mob, ch, NULL, NULL, FIGHT_PROG );

   return;

}

void mprog_give_trigger( CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj )
{

   char buf[MAX_INPUT_LENGTH];
   MPROG_DATA *mprg;

   if( IS_NPC( mob ) && ( mob->pIndexData->progtypes & GIVE_PROG ) )
   {
      /*
       * Don't let a mob trigger itself, nor one instance of a mob
       * trigger another instance. 
       */
      if( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
         return;

      for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
      {
         one_argument( mprg->arglist, buf );

         if( ( mprg->type & GIVE_PROG ) && ( ( !str_cmp( obj->name, mprg->arglist ) ) || ( !str_cmp( "all", buf ) ) ) )
         {

            mprog_driver( mprg->comlist, mob, ch, obj, NULL, FALSE );
            break;
         }
      }
   }
   return;
}

void mprog_greet_trigger( CHAR_DATA * ch )
{
   CHAR_DATA *vmob, *vmob_next;

#ifdef DEBUG
   log_printf( "mprog_greet_trigger -> %s", ch->name );
#endif

   for( vmob = ch->in_room->first_person; vmob; vmob = vmob_next )
   {
      vmob_next = vmob->next_in_room;
      if( !IS_NPC( vmob ) || vmob->fighting || !IS_AWAKE( vmob ) )
         continue;

      /*
       * Don't let a mob trigger itself, nor one instance of a mob
       * trigger another instance. 
       */
      if( IS_NPC( ch ) && ch->pIndexData == vmob->pIndexData )
         continue;

      if( vmob->pIndexData->progtypes & GREET_PROG )
         mprog_percent_check( vmob, ch, NULL, NULL, GREET_PROG );
      else if( vmob->pIndexData->progtypes & ALL_GREET_PROG )
         mprog_percent_check( vmob, ch, NULL, NULL, ALL_GREET_PROG );
   }
   return;
}

void mprog_hitprcnt_trigger( CHAR_DATA * mob, CHAR_DATA * ch )
{

   MPROG_DATA *mprg;

   if( IS_NPC( mob ) && ( mob->pIndexData->progtypes & HITPRCNT_PROG ) )
      for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
         if( ( mprg->type & HITPRCNT_PROG ) && ( ( 100 * mob->hit / mob->max_hit ) < atoi( mprg->arglist ) ) )
         {
            mprog_driver( mprg->comlist, mob, ch, NULL, NULL, FALSE );
            break;
         }

   return;

}

void mprog_random_trigger( CHAR_DATA * mob )
{
   if( mob->pIndexData->progtypes & RAND_PROG )
      mprog_percent_check( mob, NULL, NULL, NULL, RAND_PROG );

   return;
}

void mprog_time_trigger( CHAR_DATA * mob )
{
   if( mob->pIndexData->progtypes & TIME_PROG )
      mprog_time_check( mob, NULL, NULL, NULL, TIME_PROG );
   return;
}

void mprog_hour_trigger( CHAR_DATA * mob )
{
   if( mob->pIndexData->progtypes & HOUR_PROG )
      mprog_time_check( mob, NULL, NULL, NULL, HOUR_PROG );
   return;
}

void mprog_speech_trigger( const char *txt, CHAR_DATA * actor )
{

   CHAR_DATA *vmob;

   for( vmob = actor->in_room->first_person; vmob; vmob = vmob->next_in_room )
   {
      if( IS_NPC( vmob ) && ( vmob->pIndexData->progtypes & SPEECH_PROG ) )
      {
         if( IS_NPC( actor ) && actor->pIndexData == vmob->pIndexData )
            continue;
         mprog_wordlist_check( txt, vmob, actor, NULL, NULL, SPEECH_PROG );
      }
   }
   return;

}

void mprog_script_trigger( CHAR_DATA * mob )
{
   MPROG_DATA *mprg;

   if( mob->pIndexData->progtypes & SCRIPT_PROG )
      for( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
         if( ( mprg->type & SCRIPT_PROG ) )
         {
            if( mprg->arglist[0] == '\0' || mob->mpscriptpos != 0 || atoi( mprg->arglist ) == time_info.hour )
               mprog_driver( mprg->comlist, mob, NULL, NULL, NULL, TRUE );
         }
   return;
}

void oprog_script_trigger( OBJ_DATA * obj )
{
   MPROG_DATA *mprg;

   if( obj->pIndexData->progtypes & SCRIPT_PROG )
      for( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
         if( ( mprg->type & SCRIPT_PROG ) )
         {
            if( mprg->arglist[0] == '\0' || obj->mpscriptpos != 0 || atoi( mprg->arglist ) == time_info.hour )
            {
               set_supermob( obj );
               mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, TRUE );
               obj->mpscriptpos = supermob->mpscriptpos;
               release_supermob(  );
            }
         }
   return;
}

void rprog_script_trigger( ROOM_INDEX_DATA * room )
{
   MPROG_DATA *mprg;

   if( room->progtypes & SCRIPT_PROG )
      for( mprg = room->mudprogs; mprg; mprg = mprg->next )
         if( ( mprg->type & SCRIPT_PROG ) )
         {
            if( mprg->arglist[0] == '\0' || room->mpscriptpos != 0 || atoi( mprg->arglist ) == time_info.hour )
            {
               rset_supermob( room );
               mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, TRUE );
               room->mpscriptpos = supermob->mpscriptpos;
               release_supermob(  );
            }
         }
   return;
}

/*
 *  Mudprogram additions begin here
 */
void set_supermob( OBJ_DATA * obj )
{
   ROOM_INDEX_DATA *room;
   OBJ_DATA *in_obj;
   char buf[200];

   if( !supermob )
      supermob = create_mobile( get_mob_index( MOB_VNUM_SUPERMOB ) );

   if( !obj )
      return;

   supermob_obj = obj;

   for( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
      ;

   if( in_obj->carried_by )
      room = in_obj->carried_by->in_room;
   else
      room = obj->in_room;

   if( !room )
      return;

   if( supermob->short_descr )
      STRFREE( supermob->short_descr );

   supermob->short_descr = QUICKLINK( obj->short_descr );
   supermob->mpscriptpos = obj->mpscriptpos;

   /*
    * Added by Jenny to allow bug messages to show the vnum
    * of the object, and not just supermob's vnum 
    */
   snprintf( buf, 200, "Object #%d", obj->pIndexData->vnum );
   STRFREE( supermob->description );
   supermob->description = STRALLOC( buf );

   if( room != NULL )
   {
      char_from_room( supermob );
      char_to_room( supermob, room );
   }
}

void release_supermob(  )
{
   supermob_obj = NULL;
   char_from_room( supermob );
   char_to_room( supermob, get_room_index( ROOM_VNUM_POLY ) );
}

bool oprog_percent_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type )
{
   MPROG_DATA *mprg;
   bool executed = FALSE;

   for( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
      if( ( mprg->type & type ) && ( number_percent(  ) <= atoi( mprg->arglist ) ) )
      {
         executed = TRUE;
         mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
         if( type != GREET_PROG )
            break;
      }

   return executed;

}

/*
 * Triggers follow
 */

int oprog_custom_trigger( const char *command, const char *argument, CHAR_DATA * ch )
{
   ROOM_INDEX_DATA *room;
   MPROG_DATA *mprg;
   char pcom[MAX_STRING_LENGTH];
   char parg[MAX_STRING_LENGTH];
   char *temp;
   bool found = FALSE;
   OBJ_DATA *vobj;
   if( ( room = ch->in_room ) == NULL )
      return 0;
   for( vobj = room->first_content; vobj; vobj = vobj->next_content )
   {
      if( vobj != NULL && vobj->pIndexData != NULL )
      {
         for( mprg = vobj->pIndexData->mudprogs; mprg; mprg = mprg->next )
         {
            if( mprg->type & CUSTOM_PROG )
            {
               temp = strdup( mprg->arglist );
               temp = one_argument( temp, pcom );
               temp = one_argument( temp, parg );
               if( !strcmp( pcom, command ) )
               {
                  if( parg[0] == '\0' )
                  {
                     found = TRUE;
                     break;
                  }
                  else if( !strcmp( parg, argument ) )
                  {
                     found = TRUE;
                     break;
                  }
               }
            }
         }
      }
      if( found )
         break;
   }
   if( !found )
   {
      for( vobj = ch->first_carrying; vobj; vobj = vobj->next_content )
      {
         if( vobj != NULL && vobj->pIndexData != NULL )
         {
            for( mprg = vobj->pIndexData->mudprogs; mprg; mprg = mprg->next )
            {
               if( mprg->type & CUSTOM_PROG )
               {
                  temp = strdup( mprg->arglist );
                  temp = one_argument( temp, pcom );
                  temp = one_argument( temp, parg );
                  if( !strcmp( pcom, command ) )
                  {
                     if( parg[0] == '\0' )
                     {
                        found = TRUE;
                        break;
                     }
                     else if( !strcmp( parg, argument ) )
                     {
                        found = TRUE;
                        break;
                     }
                  }
               }
            }
         }
         if( found )
            break;
      }
   }
   if( !found )
      return 0;
   set_supermob( vobj );
   mprog_driver( mprg->comlist, supermob, ch, vobj, NULL, FALSE );
   set_supermob( vobj );
   return 1;
}

int mprog_custom_trigger( const char *command, const char *argument, CHAR_DATA * ch )
{
   ROOM_INDEX_DATA *room;
   MPROG_DATA *mprg;
   char pcom[MAX_STRING_LENGTH];
   char parg[MAX_STRING_LENGTH];
   char *temp;
   bool found = FALSE;
   CHAR_DATA *vmob;
   if( ( room = ch->in_room ) == NULL )
      return 0;
   for( vmob = room->first_person; vmob; vmob = vmob->next_in_room )
   {
      if( vmob != NULL && vmob->pIndexData != NULL )
         for( mprg = vmob->pIndexData->mudprogs; mprg; mprg = mprg->next )
         {
            if( mprg->type & CUSTOM_PROG )
            {
               temp = strdup( mprg->arglist );
               temp = one_argument( temp, pcom );
               temp = one_argument( temp, parg );
               if( !strcmp( pcom, command ) )
               {
                  if( parg[0] == '\0' )
                  {
                     found = TRUE;
                     break;
                  }
                  else if( !strcmp( parg, argument ) )
                  {
                     found = TRUE;
                     break;
                  }
               }
            }
         }
      if( found )
         break;
   }
   if( !found )
      return 0;
   mprog_driver( mprg->comlist, vmob, ch, NULL, NULL, FALSE );
   return 1;
}

int rprog_custom_trigger( const char *command, const char *argument, CHAR_DATA * ch )
{
   ROOM_INDEX_DATA *room;
   MPROG_DATA *mprg;
   char pcom[MAX_STRING_LENGTH];
   char parg[MAX_STRING_LENGTH];
   char *temp;
   bool found = FALSE;
   if( ( room = ch->in_room ) == NULL )
      return 0;
   for( mprg = room->mudprogs; mprg; mprg = mprg->next )
   {
      if( mprg->type & CUSTOM_PROG )
      {
         temp = strdup( mprg->arglist );
         temp = one_argument( temp, pcom );
         temp = one_argument( temp, parg );
         if( !strcmp( pcom, command ) )
         {
            if( parg[0] == '\0' )
            {
               found = TRUE;
               break;
            }
            else if( !strcmp( parg, argument ) )
            {
               found = TRUE;
               break;
            }
         }
      }
   }
   if( !found )
      return 0;
   rset_supermob( room );
   mprog_driver( mprg->comlist, supermob, ch, NULL, NULL, FALSE );
   release_supermob(  );
   return 1;
}


/*
 *  Hold on this
 *
void oprog_act_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   set_supermob( obj );
   if ( obj->pIndexData->progtypes & ACT_PROG ) 
     oprog_percent_check( supermob, ch, obj, NULL, ACT_PROG );

 release_supermob();
 return;
}
 *
 *
 */

void oprog_greet_trigger( CHAR_DATA * ch )
{
   OBJ_DATA *vobj;

   for( vobj = ch->in_room->first_content; vobj; vobj = vobj->next_content )
      if( vobj->pIndexData->progtypes & GREET_PROG )
      {
         set_supermob( vobj );   /* not very efficient to do here */
         oprog_percent_check( supermob, ch, vobj, NULL, GREET_PROG );
         release_supermob(  );
      }

   return;
}

void oprog_speech_trigger( const char *txt, CHAR_DATA * ch )
{
   OBJ_DATA *vobj;

   /*
    * supermob is set and released in oprog_wordlist_check 
    */
   for( vobj = ch->in_room->first_content; vobj; vobj = vobj->next_content )
      if( vobj->pIndexData->progtypes & SPEECH_PROG )
      {
         oprog_wordlist_check( txt, supermob, ch, vobj, NULL, SPEECH_PROG, vobj );
      }

   return;
}

/*
 * Called at top of obj_update
 * make sure to put an if(!obj) continue
 * after it
 */
void oprog_random_trigger( OBJ_DATA * obj )
{
   if( !obj || !obj->pIndexData )
      return;

   if( obj->pIndexData->progtypes & RAND_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, NULL, obj, NULL, RAND_PROG );
      release_supermob(  );
   }
   return;
}

/*
 * in wear_obj, between each successful equip_char 
 * the subsequent return
 */
void oprog_wear_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & WEAR_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, WEAR_PROG );
      release_supermob(  );
   }
   return;
}

bool oprog_use_trigger( CHAR_DATA * ch, OBJ_DATA * obj, CHAR_DATA * vict, OBJ_DATA * targ, void *vo )
{
   bool executed = FALSE;

   if( obj->pIndexData->progtypes & USE_PROG )
   {
      set_supermob( obj );
      if( obj->item_type == ITEM_STAFF )
      {
         if( vict )
            executed = oprog_percent_check( supermob, ch, obj, vict, USE_PROG );
         else
            executed = oprog_percent_check( supermob, ch, obj, targ, USE_PROG );
      }
      else
      {
         executed = oprog_percent_check( supermob, ch, obj, NULL, USE_PROG );
      }
      release_supermob(  );
   }
   return executed;
}

/*
 * call in remove_obj, right after unequip_char   
 * do a if(!ch) return right after, and return TRUE (?)
 * if !ch
 */
void oprog_remove_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & REMOVE_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, REMOVE_PROG );
      release_supermob(  );
   }
   return;
}


/*
 * call in do_sac, right before extract_obj
 */
void oprog_sac_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & SAC_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, SAC_PROG );
      release_supermob(  );
   }
   return;
}

/*
 * call in do_get, right before check_for_trap
 * do a if(!ch) return right after
 */
void oprog_get_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & GET_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, GET_PROG );
      release_supermob(  );
   }
   return;
}

/*
 * called in damage_obj in act_obj.c
 */
void oprog_damage_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & DAMAGE_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, DAMAGE_PROG );
      release_supermob(  );
   }
   return;
}

/*
 * called in do_repair in shops.c
 */
void oprog_repair_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{

   if( obj->pIndexData->progtypes & REPAIR_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, REPAIR_PROG );
      release_supermob(  );
   }
   return;
}

/*
 * call twice in do_drop, right after the act( AT_ACTION,...)
 * do a if(!ch) return right after
 */
void oprog_drop_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & DROP_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, DROP_PROG );
      release_supermob(  );
   }
   return;
}

/*
 * call towards end of do_examine, right before check_for_trap
 */
void oprog_examine_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & EXA_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, EXA_PROG );
      release_supermob(  );
   }
   return;
}


/*
 * call in fight.c, group_gain, after (?) the obj_to_room
 */
/* Commented, replaced with custom triggers.
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & ZAP_PROG ) 
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, ZAP_PROG );
      release_supermob();
   }
   return;
}
*/

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_pull_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & PULL_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, PULL_PROG );
      release_supermob(  );
   }
   return;
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_push_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( obj->pIndexData->progtypes & PUSH_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, PUSH_PROG );
      release_supermob(  );
   }
   return;
}

void obj_act_add( OBJ_DATA * obj );
void oprog_act_trigger( char *buf, OBJ_DATA * mobj, CHAR_DATA * ch, OBJ_DATA * obj, void *vo )
{
   if( mobj->pIndexData->progtypes & ACT_PROG )
   {
      MPROG_ACT_LIST *tmp_act, *tmp_mal;

      CREATE( tmp_act, MPROG_ACT_LIST, 1 );
/* 
 * Losing the head of the list -Druid
 */
      if( mobj->mpactnum > 0 )
      {
         tmp_mal = mobj->mpact;

         while( tmp_mal->next != NULL )
            tmp_mal = tmp_mal->next;

         /*
          * Put at the end 
          */
         tmp_mal->next = tmp_act;
      }
      else
         mobj->mpact = tmp_act;

      tmp_act->next = NULL;
      tmp_act->buf = strdup( buf );
      tmp_act->ch = ch;
      tmp_act->obj = obj;
      tmp_act->vo = vo;
      mobj->mpactnum++;
      obj_act_add( mobj );
   }
   return;
}

void oprog_wordlist_check( const char *arg, CHAR_DATA * mob, CHAR_DATA * actor,
                           OBJ_DATA * obj, void *vo, int type, OBJ_DATA * iobj )
{
   char temp1[MAX_STRING_LENGTH];
   char temp2[MAX_INPUT_LENGTH];
   char word[MAX_INPUT_LENGTH];
   MPROG_DATA *mprg;
   char *list;
   char *start;
   char *dupl;
   char *end;
   unsigned int i;

   for( mprg = iobj->pIndexData->mudprogs; mprg; mprg = mprg->next )
      if( mprg->type & type )
      {
         strlcpy( temp1, mprg->arglist, MAX_STRING_LENGTH );
         list = temp1;
         for( i = 0; i < strlen( list ); i++ )
            list[i] = LOWER( list[i] );
         strlcpy( temp2, arg, MAX_INPUT_LENGTH );
         dupl = temp2;
         for( i = 0; i < strlen( dupl ); i++ )
            dupl[i] = LOWER( dupl[i] );
         if( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
         {
            list += 2;
            while( ( start = strstr( dupl, list ) ) )
               if( ( start == dupl || *( start - 1 ) == ' ' )
                   && ( *( end = start + strlen( list ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
               {
                  set_supermob( iobj );
                  mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                  release_supermob(  );
                  break;
               }
               else
                  dupl = start + 1;
         }
         else
         {
            list = one_argument( list, word );
            for( ; word[0] != '\0'; list = one_argument( list, word ) )
               while( ( start = strstr( dupl, word ) ) )
                  if( ( start == dupl || *( start - 1 ) == ' ' )
                      && ( *( end = start + strlen( word ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
                  {
                     set_supermob( iobj );
                     mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                     release_supermob(  );
                     break;
                  }
                  else
                     dupl = start + 1;
         }
      }
   return;
}

/*
 *  room_prog support starts here
 *
 *
 */
void rset_supermob( ROOM_INDEX_DATA * room )
{
   char buf[200];

   if( room )
   {
      STRFREE( supermob->short_descr );
      supermob->short_descr = QUICKLINK( room->name );
      STRFREE( supermob->name );
      supermob->name = QUICKLINK( room->name );

      supermob->mpscriptpos = room->mpscriptpos;

      /*
       * Added by Jenny to allow bug messages to show the vnum
       * of the room, and not just supermob's vnum 
       */
      snprintf( buf, 200, "Room #%d", room->vnum );
      STRFREE( supermob->description );
      supermob->description = STRALLOC( buf );

      char_from_room( supermob );
      char_to_room( supermob, room );

   }
}

void rprog_percent_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type )
{
   MPROG_DATA *mprg;

   if( !mob->in_room )
      return;

   for( mprg = mob->in_room->mudprogs; mprg; mprg = mprg->next )
      if( ( mprg->type & type ) && ( number_percent(  ) <= atoi( mprg->arglist ) ) )
      {
         mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
         if( type != ENTER_PROG )
            break;
      }

   return;
}

/*
 * Triggers follow
 */


/*
 *  Hold on this
 * Unhold. -- Alty
 */
void room_act_add( ROOM_INDEX_DATA * room );
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA * room, CHAR_DATA * ch, OBJ_DATA * obj, void *vo )
{
   if( room->progtypes & ACT_PROG )
   {
      MPROG_ACT_LIST *tmp_act, *tmp_mal;

      CREATE( tmp_act, MPROG_ACT_LIST, 1 );
/* 
 * Losing the head of the list -Druid
 */
      if( room->mpactnum > 0 )
      {
         tmp_mal = room->mpact;

         while( tmp_mal->next != NULL )
            tmp_mal = tmp_mal->next;

         /*
          * Put at the end 
          */
         tmp_mal->next = tmp_act;
      }
      else
         room->mpact = tmp_act;

      tmp_act->next = NULL;
      tmp_act->buf = strdup( buf );
      tmp_act->ch = ch;
      tmp_act->obj = obj;
      tmp_act->vo = vo;
      room->mpactnum++;
      room_act_add( room );
   }
   return;
}

/*
 *
 */


void rprog_leave_trigger( CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & LEAVE_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_percent_check( supermob, ch, NULL, NULL, LEAVE_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_enter_trigger( CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & ENTER_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_percent_check( supermob, ch, NULL, NULL, ENTER_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_sleep_trigger( CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & SLEEP_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_percent_check( supermob, ch, NULL, NULL, SLEEP_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_rest_trigger( CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & REST_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_percent_check( supermob, ch, NULL, NULL, REST_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_rfight_trigger( CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & RFIGHT_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_percent_check( supermob, ch, NULL, NULL, RFIGHT_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_death_trigger( CHAR_DATA * killer, CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & RDEATH_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_percent_check( supermob, ch, NULL, NULL, RDEATH_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_speech_trigger( const char *txt, CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & SPEECH_PROG )
   {
      /*
       * supermob is set and released in rprog_wordlist_check 
       */
      rprog_wordlist_check( txt, supermob, ch, NULL, NULL, SPEECH_PROG, ch->in_room );
   }
   return;
}

void rprog_random_trigger( CHAR_DATA * ch )
{

   if( ch->in_room->progtypes & RAND_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_percent_check( supermob, ch, NULL, NULL, RAND_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_wordlist_check( const char *arg, CHAR_DATA * mob, CHAR_DATA * actor,
                           OBJ_DATA * obj, void *vo, int type, ROOM_INDEX_DATA * room )
{
   char temp1[MAX_STRING_LENGTH];
   char temp2[MAX_INPUT_LENGTH];
   char word[MAX_INPUT_LENGTH];
   MPROG_DATA *mprg;
   char *list;
   char *start;
   char *dupl;
   char *end;
   unsigned int i;

   if( actor && !char_died( actor ) && actor->in_room )
      room = actor->in_room;

   for( mprg = room->mudprogs; mprg; mprg = mprg->next )
      if( mprg->type & type )
      {
         strlcpy( temp1, mprg->arglist, MAX_STRING_LENGTH );
         list = temp1;
         for( i = 0; i < strlen( list ); i++ )
            list[i] = LOWER( list[i] );
         strlcpy( temp2, arg, MAX_INPUT_LENGTH );
         dupl = temp2;
         for( i = 0; i < strlen( dupl ); i++ )
            dupl[i] = LOWER( dupl[i] );
         if( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
         {
            list += 2;
            while( ( start = strstr( dupl, list ) ) )
               if( ( start == dupl || *( start - 1 ) == ' ' )
                   && ( *( end = start + strlen( list ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
               {
                  rset_supermob( room );
                  mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                  release_supermob(  );
                  break;
               }
               else
                  dupl = start + 1;
         }
         else
         {
            list = one_argument( list, word );
            for( ; word[0] != '\0'; list = one_argument( list, word ) )
               while( ( start = strstr( dupl, word ) ) )
                  if( ( start == dupl || *( start - 1 ) == ' ' )
                      && ( *( end = start + strlen( word ) ) == ' ' || *end == '\n' || *end == '\r' || *end == '\0' ) )
                  {
                     rset_supermob( room );
                     mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                     release_supermob(  );
                     break;
                  }
                  else
                     dupl = start + 1;
         }
      }
   return;
}

void rprog_time_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * obj, void *vo, int type )
{
   ROOM_INDEX_DATA *room = ( ROOM_INDEX_DATA * ) vo;
   MPROG_DATA *mprg;
   bool trigger_time;

   for( mprg = room->mudprogs; mprg; mprg = mprg->next )
   {
      trigger_time = ( time_info.hour == atoi( mprg->arglist ) );

      if( !trigger_time )
      {
         if( mprg->triggered )
            mprg->triggered = FALSE;
         continue;
      }

      if( ( mprg->type & type ) && ( ( !mprg->triggered ) || ( mprg->type & HOUR_PROG ) ) )
      {
         mprg->triggered = TRUE;
         mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
      }
   }
   return;
}

void rprog_time_trigger( CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & TIME_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_time_check( supermob, NULL, NULL, ch->in_room, TIME_PROG );
      release_supermob(  );
   }
   return;
}

void rprog_hour_trigger( CHAR_DATA * ch )
{
   if( ch->in_room->progtypes & HOUR_PROG )
   {
      rset_supermob( ch->in_room );
      rprog_time_check( supermob, NULL, NULL, ch->in_room, HOUR_PROG );
      release_supermob(  );
   }
   return;
}

/* Written by Jenny, Nov 29/95 */
void progbug( const char *str, CHAR_DATA * mob )
{
   /*
    * Check if we're dealing with supermob, which means the bug occurred
    * in a room or obj prog. 
    */
   if( mob->pIndexData->vnum == MOB_VNUM_SUPERMOB )
   {
      /*
       * It's supermob.  In set_supermob and rset_supermob, the description
       * was set to indicate the object or room, so we just need to show
       * the description in the bug message. 
       */
      bug( "%s, %s.", str, mob->description == NULL ? "(unknown)" : mob->description );
   }
   else
      bug( "%s, Mob #%d.", str, mob->pIndexData->vnum );

   return;
}

/* Room act prog updates.  Use a separate list cuz we dont really wanna go
   thru 5-10000 rooms every pulse.. can we say lag? -- Alty */

void room_act_add( ROOM_INDEX_DATA * room )
{
   struct act_prog_data *runner, *tmp_ral;

   for( runner = room_act_list; runner; runner = runner->next )
      if( runner->vo == room )
         return;
   CREATE( runner, struct act_prog_data, 1 );
   runner->vo = room;
   runner->next = NULL;
   /*
    * The head of the list is being changed in
    * room_act_update, So append to the end of the list 
    * instead. -Druid
    */
   if( room_act_list )
   {
      tmp_ral = room_act_list;

      while( tmp_ral->next != NULL )
         tmp_ral = tmp_ral->next;

      /*
       * put at the end 
       */
      tmp_ral->next = runner;
   }
   else
      room_act_list = runner;
}


void room_act_update( void )
{
   struct act_prog_data *runner;
   MPROG_ACT_LIST *mpact;

   while( ( runner = room_act_list ) != NULL )
   {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *)runner->vo;

      while( ( mpact = room->mpact ) != NULL )
      {
         if( mpact->ch->in_room == room )
            rprog_wordlist_check( mpact->buf, supermob, mpact->ch, mpact->obj, mpact->vo, ACT_PROG, room );
         room->mpact = mpact->next;
         DISPOSE( mpact->buf );
         DISPOSE( mpact );
      }
      room->mpact = NULL;
      room->mpactnum = 0;
      room_act_list = runner->next;
      DISPOSE( runner );
   }
   return;
}

void obj_act_add( OBJ_DATA * obj )
{
   struct act_prog_data *runner, *tmp_oal;

   for( runner = obj_act_list; runner; runner = runner->next )
      if( runner->vo == obj )
         return;
   CREATE( runner, struct act_prog_data, 1 );
   runner->vo = obj;
   runner->next = NULL;
   /*
    * The head of the list is being changed in
    * obj_act_update, So append to the end of the list 
    * instead. -Druid
    */
   if( obj_act_list )
   {
      tmp_oal = obj_act_list;

      while( tmp_oal->next != NULL )
         tmp_oal = tmp_oal->next;

      /*
       * put at the end 
       */
      tmp_oal->next = runner;
   }
   else
      obj_act_list = runner;
}

void obj_act_update( void )
{
   struct act_prog_data *runner;
   MPROG_ACT_LIST *mpact;

   while( ( runner = obj_act_list ) != NULL )
   {
      OBJ_DATA *obj = (OBJ_DATA *)runner->vo;

      while( ( mpact = obj->mpact ) != NULL )
      {
         oprog_wordlist_check( mpact->buf, supermob, mpact->ch, mpact->obj, mpact->vo, ACT_PROG, obj );
         obj->mpact = mpact->next;
         DISPOSE( mpact->buf );
         DISPOSE( mpact );
      }
      obj->mpact = NULL;
      obj->mpactnum = 0;
      obj_act_list = runner->next;
      DISPOSE( runner );
   }
   return;
}
