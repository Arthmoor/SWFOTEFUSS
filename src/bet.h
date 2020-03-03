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

#include <ctype.h>


/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.
*/

int advatoi( char *s )
{
   int number = 0;   /* number to be returned */
   int multiplier = 0;  /* multiplier used to get the extra digits right */

   /*
    * as long as the current character is a digit add to current number
    */
   while( isdigit( s[0] ) )
      number = ( number * 10 ) + ( *s++ - '0' );

   switch ( UPPER( s[0] ) )
   {
      case 'K':
         number *= ( multiplier = 1000 );
         ++s;
         break;
      case 'M':
         number *= ( multiplier = 1000000 );
         ++s;
         break;
      case '\0':
         break;
      default:
         return 0;   /* not k nor m nor NULL - return 0! */
   }

   /*
    * if any digits follow k/m, add those too 
    */
   while( isdigit( s[0] ) && ( multiplier > 1 ) )
   {
      /*
       * the further we get to right, the less the digit 'worth' 
       */
      multiplier /= 10;
      number = number + ( ( *s++ - '0' ) * multiplier );
   }

   /*
    * return 0 if non-digit character was found, other than NULL 
    */
   if( s[0] != '\0' && !isdigit( s[0] ) )
      return 0;

   /*
    * anything left is likely extra digits (ie: 14k4443  -> 3 is extra) 
    */

   return number;
}


/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/
int parsebet( const int currentbet, char *s )
{
   /*
    * check to make sure it's not blank 
    */
   if( s[0] != '\0' )
   {
      /*
       * if first char is a digit, use advatoi 
       */
      if( isdigit( s[0] ) )
         return ( advatoi( s ) );
      if( s[0] == '+' ) /* add percent (default 25%) */
      {
         if( s[1] == '\0' )
            return ( currentbet * 125 ) / 100;
         return ( currentbet * ( 100 + atoi( s + 1 ) ) ) / 100;
      }
      if( s[0] == '*' || s[0] == 'x' ) /* multiply (default is by 2) */
      {
         if( s[1] == '\0' )
            return ( currentbet * 2 );
         return ( currentbet * atoi( s + 1 ) );
      }
   }
   return 0;
}
