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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"
#include "mccp.h"
#include "mssp.h"
#include "sha256.h"

/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

#define MAX_NEST	100

const unsigned char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const unsigned char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const unsigned char go_ahead_str[] = { IAC, GA, '\0' };

void save_sysdata( SYSTEM_DATA sys );

/*  from act_info?  */
void show_condition( CHAR_DATA * ch, CHAR_DATA * victim );
void generate_com_freq( CHAR_DATA * ch );

// planets.c
void write_planet_list( void );

/*
 * Global variables.
 */
DESCRIPTOR_DATA *first_descriptor;  /* First descriptor     */
DESCRIPTOR_DATA *last_descriptor;   /* Last descriptor      */
DESCRIPTOR_DATA *d_next;   /* Next descriptor in loop */
int num_descriptors;
bool mud_down; /* Shutdown       */
bool wizlock;  /* Game is wizlocked    */
time_t boot_time;
HOUR_MIN_SEC set_boot_time_struct;
HOUR_MIN_SEC *set_boot_time;
struct tm *new_boot_time;
struct tm new_boot_struct;
char str_boot_time[MAX_INPUT_LENGTH];
char lastplayercmd[MAX_INPUT_LENGTH * 2];
time_t current_time; /* Time of this pulse      */
int port;   /* Port number to be used       */
int control;   /* Controlling descriptor  */
int newdesc;   /* New descriptor    */
fd_set in_set; /* Set of desc's for reading  */
fd_set out_set;   /* Set of desc's for writing  */
fd_set exc_set;   /* Set of desc's with errors  */
int maxdesc;

/*
 * OS-dependent local functions.
 */
void game_loop( void );
int init_socket( int mudport );
void new_descriptor( int new_desc );
bool read_from_descriptor( DESCRIPTOR_DATA * d );

/*
 * Other local functions (OS-independent).
 */
short check_reconnect( DESCRIPTOR_DATA * d, char *name, bool fConn );
short check_playing( DESCRIPTOR_DATA * d, char *name, bool kick );
bool check_multi( DESCRIPTOR_DATA * d, char *name );
int main( int argc, char **argv );
void nanny( DESCRIPTOR_DATA * d, char *argument );
bool flush_buffer( DESCRIPTOR_DATA * d, bool fPrompt );
void read_from_buffer( DESCRIPTOR_DATA * d );
void stop_idling( CHAR_DATA * ch );
void free_desc( DESCRIPTOR_DATA * d );
void display_prompt( DESCRIPTOR_DATA * d );
void set_pager_input( DESCRIPTOR_DATA * d, char *argument );
bool pager_output( DESCRIPTOR_DATA * d );
void mail_count( CHAR_DATA * ch );

int main( int argc, char **argv )
{
   struct timeval now_time;
   bool fCopyOver = FALSE;
#ifdef IMC
   int imcsocket = -1;
#endif

   /*
    * Memory debugging if needed.
    */
#if defined(MALLOC_DEBUG)
   malloc_debug( 2 );
#endif
   DONT_UPPER = FALSE;
   num_descriptors = 0;
   first_descriptor = NULL;
   last_descriptor = NULL;
   sysdata.NO_NAME_RESOLVING = TRUE;
   sysdata.WAIT_FOR_AUTH = TRUE;

   /*
    * Init time.
    */
   gettimeofday( &now_time, NULL );
   current_time = ( time_t ) now_time.tv_sec;
/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
   boot_time = time( 0 );  /*  <-- I think this is what you wanted */
   mudstrlcpy( str_boot_time, ctime( &current_time ), MAX_INPUT_LENGTH );

   /*
    * Init boot time.
    */
   set_boot_time = &set_boot_time_struct;
   /*
    * set_boot_time->hour   = 6;
    * set_boot_time->min    = 0;
    * set_boot_time->sec    = 0;
    */
   set_boot_time->manual = 0;

   new_boot_time = update_time( localtime( &current_time ) );
   /*
    * Copies *new_boot_time to new_boot_struct, and then points
    * new_boot_time to new_boot_struct again. -- Alty 
    */
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;
   new_boot_time->tm_mday += 1;
   if( new_boot_time->tm_hour > 12 )
      new_boot_time->tm_mday += 1;
   new_boot_time->tm_sec = 0;
   new_boot_time->tm_min = 0;
   new_boot_time->tm_hour = 6;

   /*
    * Update new_boot_time (due to day increment) 
    */
   new_boot_time = update_time( new_boot_time );
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;

   /*
    * Set reboot time string for do_time 
    */
   get_reboot_string(  );

   init_pfile_scan_time(  );  /* Pfile autocleanup initializer - Samson 5-8-99 */

   /*
    * Get the port number.
    */
   port = 1545;
   if( argc > 1 )
   {
      if( !is_number( argv[1] ) )
      {
         fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
         exit( 1 );
      }
      else if( ( port = atoi( argv[1] ) ) <= 1024 )
      {
         fprintf( stderr, "Port number must be above 1024.\n" );
         exit( 1 );
      }

      if( argv[2] && argv[2][0] )
      {
         fCopyOver = TRUE;
         control = atoi( argv[3] );
#ifdef IMC
         imcsocket = atoi( argv[4] );
#endif
      }
      else
         fCopyOver = FALSE;
   }

   /*
    * Run the game.
    */
   log_string( "Booting Database" );
   boot_db( fCopyOver );
   log_string( "Initializing socket" );

   if( !fCopyOver )  /* We have already the port if copyover'ed */
   {
      control = init_socket( port );
   }

#ifdef IMC
   /*
    * Initialize and connect to IMC2 
    */
   imc_startup( FALSE, imcsocket, fCopyOver );
#endif

   if( fCopyOver )
   {
      log_string( "Initiating hotboot recovery." );
      hotboot_recover(  );
   }

   log_printf( "SWFotE ready on port %d.", port );

   game_loop(  );
   close( control );
#ifdef IMC
   imc_shutdown( FALSE );
#endif
   /*
    * That's all, folks.
    */
   log_string( "Normal termination of game." );
   exit( 0 );
   return 0;
}

void init_descriptor( DESCRIPTOR_DATA * dnew, int desc )
{
   dnew->next = NULL;
   dnew->descriptor = desc;
   dnew->connected = CON_GET_NAME;
   dnew->outsize = 2000;
   dnew->idle = 0;
   dnew->lines = 0;
   dnew->scrlen = 24;
   dnew->newstate = 0;
   dnew->prevcolor = 0x07;
   dnew->can_compress = FALSE;
   dnew->ifd = -1; /* Descriptor pipes, used for DNS resolution and such */
   dnew->ipid = -1;
   CREATE( dnew->mccp, MCCP, 1 );
   CREATE( dnew->outbuf, char, dnew->outsize );
}

int init_socket( int mudport )
{
   char hostname[64];
   struct sockaddr_in sa;
   int x = 1;
   int fd;

   gethostname( hostname, sizeof( hostname ) );

   if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
   {
      perror( "Init_socket: socket" );
      exit( 1 );
   }

   if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( void * )&x, sizeof( x ) ) < 0 )
   {
      perror( "Init_socket: SO_REUSEADDR" );
      close( fd );
      exit( 1 );
   }

#if defined(SO_DONTLINGER) && !defined(SYSV)
   {
      struct linger ld;

      ld.l_onoff = 1;
      ld.l_linger = 1000;

      if( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER, ( void * )&ld, sizeof( ld ) ) < 0 )
      {
         perror( "Init_socket: SO_DONTLINGER" );
         close( fd );
         exit( 1 );
      }
   }
#endif

   gethostbyname( hostname );
   getservbyname( "service", "mud" );
   memset( &sa, '\0', sizeof( sa ) );
   sa.sin_family = AF_INET;   /* hp->h_addrtype; */
   sa.sin_port = htons( mudport );

   if( bind( fd, ( struct sockaddr * )&sa, sizeof( sa ) ) == -1 )
   {
      perror( "Init_socket: bind" );
      close( fd );
      exit( 1 );
   }

   if( listen( fd, 50 ) < 0 )
   {
      perror( "Init_socket: listen" );
      close( fd );
      exit( 1 );
   }

   return fd;
}

/*
static void SegVio()
{
  CHAR_DATA *ch;

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    log_printf( "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ', ch->name, ch->in_room->vnum );
  }
  exit(0);
}
*/

/*
 * LAG alarm!							-Thoric
 */
static void caught_alarm( int signum )
{
   char buf[MAX_STRING_LENGTH];

   bug( "%s: ALARM CLOCK!", __func__ );

   mudstrlcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\r\n", MAX_STRING_LENGTH );
   echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
   if( newdesc )
   {
      FD_CLR( newdesc, &in_set );
      FD_CLR( newdesc, &out_set );
      log_string( "clearing newdesc" );
   }
   game_loop(  );
   close( control );

   log_string( "Normal termination of game." );
   exit( 0 );
}

bool check_bad_desc( int desc )
{
   if( FD_ISSET( desc, &exc_set ) )
   {
      FD_CLR( desc, &in_set );
      FD_CLR( desc, &out_set );
      log_string( "Bad FD caught and disposed." );
      return TRUE;
   }
   return FALSE;
}


void accept_new( int ctrl )
{
   static struct timeval null_time;
   DESCRIPTOR_DATA *d;
   /*
    * int maxdesc; Moved up for use with id.c as extern 
    */

#if defined(MALLOC_DEBUG)
   if( malloc_verify(  ) != 1 )
      abort(  );
#endif

   /*
    * Poll all active descriptors.
    */
   FD_ZERO( &in_set );
   FD_ZERO( &out_set );
   FD_ZERO( &exc_set );
   FD_SET( ctrl, &in_set );
   maxdesc = ctrl;
   newdesc = 0;
   for( d = first_descriptor; d; d = d->next )
   {
      maxdesc = UMAX( maxdesc, d->descriptor );
      FD_SET( d->descriptor, &in_set );
      FD_SET( d->descriptor, &out_set );
      FD_SET( d->descriptor, &exc_set );
      if( d->ifd != -1 && d->ipid != -1 )
      {
         maxdesc = UMAX( maxdesc, d->ifd );
         FD_SET( d->ifd, &in_set );
      }
      if( d == last_descriptor )
         break;
   }

   if( select( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
   {
      perror( "accept_new: select: poll" );
      exit( 1 );
   }

   if( FD_ISSET( ctrl, &exc_set ) )
   {
      bug( "%s: Exception raise on controlling descriptor %d", __func__, ctrl );
      FD_CLR( ctrl, &in_set );
      FD_CLR( ctrl, &out_set );
   }
   else if( FD_ISSET( ctrl, &in_set ) )
   {
      newdesc = ctrl;
      new_descriptor( newdesc );
   }
}

void game_loop( void )
{
   struct timeval last_time;
   char cmdline[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
/*  time_t	last_check = 0;  */

   signal( SIGPIPE, SIG_IGN );
   signal( SIGALRM, caught_alarm );
   /*
    * signal( SIGSEGV, SegVio ); 
    */
   gettimeofday( &last_time, NULL );
   current_time = ( time_t ) last_time.tv_sec;

   /*
    * Main loop 
    */
   while( !mud_down )
   {
      accept_new( control );
      /*
       * Kick out descriptors with raised exceptions
       * or have been idle, then check for input.
       */
      for( d = first_descriptor; d; d = d_next )
      {
         if( d == d->next )
         {
            bug( "%s: loop found & fixed", __func__ );
            d->next = NULL;
         }
         d_next = d->next;

         d->idle++;  /* make it so a descriptor can idle out */
         if( FD_ISSET( d->descriptor, &exc_set ) )
         {
            FD_CLR( d->descriptor, &in_set );
            FD_CLR( d->descriptor, &out_set );
            if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
               save_char_obj( d->character );
            d->outtop = 0;
            close_socket( d, TRUE );
            continue;
         }
         else if( ( !d->character && d->idle > 360 )  /* 2 mins */
                  || ( d->connected != CON_PLAYING && d->idle > 1200 )  /* 5 mins */
                  || d->idle > 28800 ) /* 2 hrs  */
         {
            write_to_descriptor( d, "Idle timeout... disconnecting.\r\n", 0 );
            d->outtop = 0;
            close_socket( d, TRUE );
            continue;
         }
         else
         {
            d->fcommand = FALSE;

            if( FD_ISSET( d->descriptor, &in_set ) )
            {
               d->idle = 0;
               if( d->character )
                  d->character->timer = 0;
               if( !read_from_descriptor( d ) )
               {
                  FD_CLR( d->descriptor, &out_set );
                  if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                     save_char_obj( d->character );
                  d->outtop = 0;
                  close_socket( d, FALSE );
                  continue;
               }
            }

            /* check for input from the dns */
            if( ( d->connected == CON_PLAYING || d->character != NULL ) && d->ifd != -1 && FD_ISSET( d->ifd, &in_set ) )
               process_dns( d );

            if( d->character && d->character->wait > 0 )
            {
               --d->character->wait;
               continue;
            }

            read_from_buffer( d );
            if( d->incomm[0] != '\0' )
            {
               d->fcommand = TRUE;
               stop_idling( d->character );

               mudstrlcpy( cmdline, d->incomm, MAX_INPUT_LENGTH );
               d->incomm[0] = '\0';

               if( d->character )
                  set_cur_char( d->character );

               if( d->pagepoint )
                  set_pager_input( d, cmdline );
               else
                  switch ( d->connected )
                  {
                     default:
                        nanny( d, cmdline );
                        break;
                     case CON_PLAYING:
                        interpret( d->character, cmdline );
                        break;
                     case CON_EDITING:
                        edit_buffer( d->character, cmdline );
                        break;
                  }
            }
         }
         if( d == last_descriptor )
            break;
      }

#ifdef IMC
      imc_loop(  );
#endif

      /*
       * Autonomous game motion.
       */
      update_handler(  );

      /*
       * Output.
       */
      for( d = first_descriptor; d; d = d_next )
      {
         d_next = d->next;

         if( ( d->fcommand || d->outtop > 0 ) && FD_ISSET( d->descriptor, &out_set ) )
         {
            if( d->pagepoint )
            {
               if( !pager_output( d ) )
               {
                  if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                     save_char_obj( d->character );
                  d->outtop = 0;
                  close_socket( d, FALSE );
               }
            }
            else if( !flush_buffer( d, TRUE ) )
            {
               if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                  save_char_obj( d->character );
               d->outtop = 0;
               close_socket( d, FALSE );
            }
         }
         if( d == last_descriptor )
            break;
      }

      /*
       * Synchronize to a clock.
       * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
       * Careful here of signed versus unsigned arithmetic.
       */
      {
         struct timeval now_time;
         long secDelta;
         long usecDelta;

         gettimeofday( &now_time, NULL );
         usecDelta = ( ( int )last_time.tv_usec ) - ( ( int )now_time.tv_usec ) + 1000000 / PULSE_PER_SECOND;
         secDelta = ( ( int )last_time.tv_sec ) - ( ( int )now_time.tv_sec );
         while( usecDelta < 0 )
         {
            usecDelta += 1000000;
            secDelta -= 1;
         }

         while( usecDelta >= 1000000 )
         {
            usecDelta -= 1000000;
            secDelta += 1;
         }

         if( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
         {
            struct timeval stall_time;

            stall_time.tv_usec = usecDelta;
            stall_time.tv_sec = secDelta;
            if( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
            {
               perror( "game_loop: select: stall" );
               exit( 1 );
            }
         }
      }

      gettimeofday( &last_time, NULL );
      current_time = ( time_t ) last_time.tv_sec;

      /*
       * Check every 5 seconds...  (don't need it right now)
       * if ( last_check+5 < current_time )
       * {
       * CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
       * DESCRIPTOR_DATA);
       * last_check = current_time;
       * }
       */
   }
   return;
}

void new_descriptor( int new_desc )
{
   char buf[MAX_STRING_LENGTH];
   DESCRIPTOR_DATA *dnew;
   BAN_DATA *pban;
   struct sockaddr_in sock;
   int desc;
   socklen_t size;

   set_alarm( 20 );
   size = sizeof( sock );
   if( check_bad_desc( new_desc ) )
   {
      set_alarm( 0 );
      return;
   }

   set_alarm( 20 );
   if( ( desc = accept( new_desc, ( struct sockaddr * )&sock, &size ) ) < 0 )
   {
      perror( "New_descriptor: accept" );
      set_alarm( 0 );
      return;
   }

   if( check_bad_desc( new_desc ) )
   {
      set_alarm( 0 );
      return;
   }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

   set_alarm( 20 );
   if( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
   {
      perror( "New_descriptor: fcntl: FNDELAY" );
      set_alarm( 0 );
      return;
   }
   if( check_bad_desc( new_desc ) )
      return;

   CREATE( dnew, DESCRIPTOR_DATA, 1 );
   init_descriptor( dnew, desc );
   dnew->port = ntohs( sock.sin_port );
   mudstrlcpy( log_buf, inet_ntoa( sock.sin_addr ), MAX_STRING_LENGTH );
   dnew->host = STRALLOC( log_buf );
   if( !sysdata.NO_NAME_RESOLVING )
   {
      mudstrlcpy( buf, in_dns_cache( log_buf ), MAX_STRING_LENGTH );

      if( buf[0] == '\0' )
         resolve_dns( dnew, sock.sin_addr.s_addr );
      else
      {
         STRFREE( dnew->host );
         dnew->host = STRALLOC( buf );
      }
   }

   for( pban = first_ban; pban; pban = pban->next )
   {
      if( ( !str_prefix( pban->name, dnew->host ) || !str_suffix( pban->name, dnew->host ) ) && pban->level >= LEVEL_SUPREME )
      {
         write_to_descriptor( dnew, "Your site has been banned from this Mud.\r\n", 0 );
         free_desc( dnew );
         set_alarm( 0 );
         return;
      }
   }

   /*
    * Init descriptor data.
    */
   if( !last_descriptor && first_descriptor )
   {
      DESCRIPTOR_DATA *d;

      bug( "%s: last_desc is NULL, but first_desc is not! ...fixing", __func__ );
      for( d = first_descriptor; d; d = d->next )
         if( !d->next )
            last_descriptor = d;
   }

   LINK( dnew, first_descriptor, last_descriptor, next, prev );

   /*
    * MCCP Compression 
    */
   write_to_buffer( dnew, (const char *)will_compress2_str, 0 );

   /*
    * Send the greeting. Forces new color function - Tawnos
    */
   {
      extern char *help_greeting;
      if( help_greeting[0] == '.' )
         send_to_desc_color( help_greeting + 1, dnew );
      else
         send_to_desc_color( help_greeting, dnew );
   }

   if( ++num_descriptors > sysdata.maxplayers )
      sysdata.maxplayers = num_descriptors;
   if( sysdata.maxplayers > sysdata.alltimemax )
   {
      if( sysdata.time_of_max )
         DISPOSE( sysdata.time_of_max );
      snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
      sysdata.time_of_max = str_dup( buf );
      sysdata.alltimemax = sysdata.maxplayers;
      snprintf( log_buf, MAX_STRING_LENGTH, "Broke all-time maximum player record: %d", sysdata.alltimemax );
      log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
      to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
      save_sysdata( sysdata );
   }
   set_alarm( 0 );
   return;
}

void log_printf_plus( short log_type, short level, const char *fmt, ... )
{
   char buf[MSL * 2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, MSL * 2, fmt, args );
   va_end( args );

   log_string_plus( buf, log_type, level );
}

void log_printf( const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   log_string( buf );
}

void free_desc( DESCRIPTOR_DATA * d )
{
   compressEnd( d );
   close( d->descriptor );
   if( d->host )
      STRFREE( d->host );
   if( d->outbuf )
      DISPOSE( d->outbuf );
   if( d->pagebuf )
      DISPOSE( d->pagebuf );
   DISPOSE( d->mccp );
   DISPOSE( d );
   --num_descriptors;
   return;
}

void close_socket( DESCRIPTOR_DATA * dclose, bool force )
{
   CHAR_DATA *ch;
   DESCRIPTOR_DATA *d;
   bool DoNotUnlink = FALSE;

   if( dclose->ipid != -1 )
   {
      int status;

      kill( dclose->ipid, SIGKILL );
      waitpid( dclose->ipid, &status, 0 );
   }
   if( dclose->ifd != -1 )
      close( dclose->ifd );

   /*
    * flush outbuf 
    */
   if( !force && dclose->outtop > 0 )
      flush_buffer( dclose, FALSE );

   /*
    * say bye to whoever's snooping this descriptor 
    */
   if( dclose->snoop_by )
      write_to_buffer( dclose->snoop_by, "Your victim has left the game.\r\n", 0 );

   /*
    * stop snooping everyone else 
    */
   for( d = first_descriptor; d; d = d->next )
      if( d->snoop_by == dclose )
         d->snoop_by = NULL;

   /*
    * Check for switched people who go link-dead. -- Altrag 
    */
   if( dclose->original )
   {
      if( ( ch = dclose->character ) != NULL )
         do_return( ch, "" );
      else
      {
         bug( "%s: dclose->original without character %s", __func__,
              ( dclose->original->name ? dclose->original->name : "unknown" ) );
         dclose->character = dclose->original;
         dclose->original = NULL;
      }
   }

   ch = dclose->character;

   /*
    * sanity check :( 
    */
   if( !dclose->prev && dclose != first_descriptor )
   {
      DESCRIPTOR_DATA *dp, *dn;
      bug( "%s: %s desc:%p != first_desc:%p and desc->prev = NULL!", __func__,
           ch ? ch->name : d->host, dclose, first_descriptor );
      dp = NULL;
      for( d = first_descriptor; d; d = dn )
      {
         dn = d->next;
         if( d == dclose )
         {
            bug( "%s: %s desc:%p found, prev should be:%p, fixing.", __func__, ch ? ch->name : d->host, dclose, dp );
            dclose->prev = dp;
            break;
         }
         dp = d;
      }
      if( !dclose->prev )
      {
         bug( "%s: %s desc:%p could not be found!.", __func__, ch ? ch->name : dclose->host, dclose );
         DoNotUnlink = TRUE;
      }
   }
   if( !dclose->next && dclose != last_descriptor )
   {
      DESCRIPTOR_DATA *dp, *dn;
      bug( "%s: %s desc:%p != last_desc:%p and desc->next = NULL!", __func__,
           ch ? ch->name : d->host, dclose, last_descriptor );
      dn = NULL;
      for( d = last_descriptor; d; d = dp )
      {
         dp = d->prev;
         if( d == dclose )
         {
            bug( "%s: %s desc:%p found, next should be:%p, fixing.", __func__, ch ? ch->name : d->host, dclose, dn );
            dclose->next = dn;
            break;
         }
         dn = d;
      }
      if( !dclose->next )
      {
         bug( "%s: %s desc:%p could not be found!.", __func__, ch ? ch->name : dclose->host, dclose );
         DoNotUnlink = TRUE;
      }
   }

   if( dclose->character )
   {
      log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->top_level ), "Closing link to %s.", ch->name );

      if( dclose->connected == CON_PLAYING || dclose->connected == CON_EDITING )
      {
         act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
         ch->desc = NULL;
      }
      else
      {
         /*
          * clear descriptor pointer to get rid of bug message in log 
          */
         dclose->character->desc = NULL;
         free_char( dclose->character );
      }
   }

   if( !DoNotUnlink )
   {
      /*
       * make sure loop doesn't get messed up 
       */
      if( d_next == dclose )
         d_next = d_next->next;
      UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
   }
   compressEnd( dclose );

   if( dclose->descriptor == maxdesc )
      --maxdesc;

   free_desc( dclose );
   return;
}

bool read_from_descriptor( DESCRIPTOR_DATA * d )
{
   unsigned int iStart;

   /*
    * Hold horses if pending command already. 
    */
   if( d->incomm[0] != '\0' )
      return TRUE;

   /*
    * Check for overflow. 
    */
   iStart = strlen( d->inbuf );
   if( iStart >= sizeof( d->inbuf ) - 10 )
   {
      log_printf( "%s input overflow!", d->host );
      write_to_descriptor( d, "\r\n*** PUT A LID ON IT!!! ***\r\n", 0 );
      return FALSE;
   }

   for( ;; )
   {
      int nRead;

      nRead = read( d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart );
      if( nRead > 0 )
      {
         iStart += nRead;
         if( d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r' )
            break;
      }
      else if( nRead == 0 )
      {
         log_string_plus( "EOF encountered on read.", LOG_COMM, sysdata.log_level );
         return FALSE;
      }
      else if( errno == EWOULDBLOCK )
         break;
      else
      {
         perror( "Read_from_descriptor" );
         return FALSE;
      }
   }

   d->inbuf[iStart] = '\0';
   return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA * d )
{
   int i, j, k, iac = 0;

   /*
    * Hold horses if pending command already.
    */
   if( d->incomm[0] != '\0' )
      return;

   /*
    * Look for at least one new line.
    */
   for( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i < MAX_INBUF_SIZE; i++ )
   {
      if( d->inbuf[i] == '\0' )
         return;
   }

   /*
    * Canonical input processing.
    */
   for( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
   {
      if( k >= 254 )
      {
         write_to_descriptor( d, "Line too long.\n\r", 0 );

         /*
          * skip the rest of the line 
          */
         d->inbuf[i] = '\n';
         d->inbuf[i + 1] = '\0';
         break;
      }

      if( d->inbuf[i] == ( signed char )IAC )
         iac = 1;
      else if( iac == 1
               && ( d->inbuf[i] == ( signed char )DO || d->inbuf[i] == ( signed char )DONT
                    || d->inbuf[i] == ( signed char )WILL ) )
         iac = 2;
      else if( iac == 2 )
      {
         iac = 0;
         if( d->inbuf[i] == ( signed char )TELOPT_COMPRESS2 )
         {
            if( d->inbuf[i - 1] == ( signed char )DO )
               compressStart( d );
            else if( d->inbuf[i - 1] == ( signed char )DONT )
               compressEnd( d );
         }
      }
      else if( d->inbuf[i] == '\b' && k > 0 )
         --k;
      else if( isascii( d->inbuf[i] ) && isprint( d->inbuf[i] ) )
         d->incomm[k++] = d->inbuf[i];
   }

   /*
    * Finish off the line.
    */
   if( k == 0 )
      d->incomm[k++] = ' ';
   d->incomm[k] = '\0';

   /*
    * Deal with bozos with #repeat 1000 ...
    */
   if( k > 1 || d->incomm[0] == '!' )
   {
      if( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
      {
         d->repeat = 0;
      }
      else
      {
         if( ++d->repeat >= 20 )
         {
            write_to_descriptor( d, "\r\n*** PUT A LID ON IT!!! ***\r\n", 0 );
         }
      }
   }

   /*
    * Do '!' substitution.
    */
   if( d->incomm[0] == '!' )
      mudstrlcpy( d->incomm, d->inlast, MAX_INPUT_LENGTH );
   else
      mudstrlcpy( d->inlast, d->incomm, MAX_INPUT_LENGTH );

   /*
    * Shift the input buffer.
    */
   while( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
      i++;
   for( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != '\0'; j++ )
      ;
   return;
}

/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA * d, bool fPrompt )
{
   char buf[MAX_INPUT_LENGTH];
   CHAR_DATA *ch;

   ch = d->original ? d->original : d->character;
   if( ch && ch->fighting && ch->fighting->who )
      show_condition( ch, ch->fighting->who );

   /*
    * If buffer has more than 4K inside, spit out .5K at a time   -Thoric
    */
   if( !mud_down && d->outtop > 4096 )
   {
      memcpy( buf, d->outbuf, 512 );
      memmove( d->outbuf, d->outbuf + 512, d->outtop - 512 );
      d->outtop -= 512;
      if( d->snoop_by )
      {
         char snoopbuf[MAX_INPUT_LENGTH];

         buf[512] = '\0';
         if( d->character && d->character->name )
         {
            if( d->original && d->original->name )
               snprintf( snoopbuf, MAX_INPUT_LENGTH, "%s (%s)", d->character->name, d->original->name );
            else
               snprintf( snoopbuf, MAX_INPUT_LENGTH, "%s", d->character->name );
            write_to_buffer( d->snoop_by, snoopbuf, 0 );
         }
         write_to_buffer( d->snoop_by, "% ", 2 );
         write_to_buffer( d->snoop_by, buf, 0 );
      }
      if( !write_to_descriptor( d, buf, 512 ) )
      {
         d->outtop = 0;
         return FALSE;
      }
      return TRUE;
   }

   /*
    * Bust a prompt.
    */
   if( fPrompt && !mud_down && d->connected == CON_PLAYING )
   {
      ch = d->original ? d->original : d->character;
      if( IS_SET( ch->act, PLR_BLANK ) )
         write_to_buffer( d, "\r\n", 2 );

      if( IS_SET( ch->act, PLR_PROMPT ) )
         display_prompt( d );
      if( IS_SET( ch->act, PLR_TELNET_GA ) )
         write_to_buffer( d, (const char *)go_ahead_str, 0 );
   }

   /*
    * Short-circuit if nothing to write.
    */
   if( d->outtop == 0 )
      return TRUE;

   /*
    * Snoop-o-rama.
    */
   if( d->snoop_by )
   {
      /*
       * without check, 'force mortal quit' while snooped caused crash, -h 
       */
      if( d->character && d->character->name )
      {
         /*
          * Show original snooped names. -- Altrag 
          */
         if( d->original && d->original->name )
            snprintf( buf, MAX_INPUT_LENGTH, "%s (%s)", d->character->name, d->original->name );
         else
            snprintf( buf, MAX_INPUT_LENGTH, "%s", d->character->name );
         write_to_buffer( d->snoop_by, buf, 0 );
      }
      write_to_buffer( d->snoop_by, "% ", 2 );
      write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
   }

   /*
    * OS-dependent output.
    */
   if( !write_to_descriptor( d, d->outbuf, d->outtop ) )
   {
      d->outtop = 0;
      return FALSE;
   }
   else
   {
      d->outtop = 0;
      return TRUE;
   }
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA * d, const char *txt, size_t length )
{
   if( !d )
   {
      bug( "%s: NULL descriptor", __func__ );
      return;
   }

   /*
    * Normally a bug... but can happen if loadup is used.
    */
   if( !d->outbuf )
      return;

   /*
    * Find length in case caller didn't.
    */
   if( length <= 0 )
      length = strlen( txt );

/* Uncomment if debugging or something
    if ( length != strlen(txt) )
    {
	bug( "%s: length(%d) != strlen(txt)!", __func__, length );
	length = strlen(txt);
    }
*/

   /*
    * Initial \r\n if needed.
    */
   if( d->outtop == 0 && !d->fcommand )
   {
      d->outbuf[0] = '\n';
      d->outbuf[1] = '\r';
      d->outtop = 2;
   }

   /*
    * Expand the buffer as needed.
    */
   while( d->outtop + length >= d->outsize )
   {
      if( d->outsize > 32000 )
      {
         /*
          * empty buffer 
          */
         d->outtop = 0;
         bug( "%s: Buffer overflow. Closing (%s).", __func__, d->character ? d->character->name : "???" );
         close_socket( d, TRUE );
         return;
      }
      d->outsize *= 2;
      RECREATE( d->outbuf, char, d->outsize );
   }

   /*
    * Copy.
    */
   strncpy( d->outbuf + d->outtop, txt, length );
   d->outtop += length;
   d->outbuf[d->outtop] = '\0';
   return;
}

/*
 * This is the MCCP version. Use write_to_descriptor_old to send non-compressed
 * text.
 * Updated to run with the block checks by Orion... if it doesn't work, blame
 * him.;P -Orion
 */
bool write_to_descriptor( DESCRIPTOR_DATA * d, const char *txt, int length )
{
   int iStart = 0;
   int nWrite = 0;
   int nBlock;
   int iErr;
   int len;

   if( length <= 0 )
      length = strlen( txt );

   if( d && d->mccp->out_compress )
   {
      d->mccp->out_compress->next_in = ( unsigned char * )txt;
      d->mccp->out_compress->avail_in = length;

      while( d->mccp->out_compress->avail_in )
      {
         d->mccp->out_compress->avail_out =
            COMPRESS_BUF_SIZE - ( d->mccp->out_compress->next_out - d->mccp->out_compress_buf );

         if( d->mccp->out_compress->avail_out )
         {
            int status = deflate( d->mccp->out_compress, Z_SYNC_FLUSH );

            if( status != Z_OK )
               return FALSE;
         }

         len = d->mccp->out_compress->next_out - d->mccp->out_compress_buf;
         if( len > 0 )
         {
            for( iStart = 0; iStart < len; iStart += nWrite )
            {
               nBlock = UMIN( len - iStart, 4096 );
               nWrite = send( d->descriptor, d->mccp->out_compress_buf + iStart, nBlock, 0 );
               if( nWrite == -1 )
               {
                  iErr = errno;
                  if( iErr == EWOULDBLOCK )
                  {
                     /*
                      * This is a SPAMMY little bug error. I would suggest
                      * not using it, but I've included it in case. -Orion
                      *
                      perror( "Write_to_descriptor: Send is blocking" );
                      */
                     nWrite = 0;
                     continue;
                  }
                  else
                  {
                     perror( "Write_to_descriptor" );
                     return FALSE;
                  }
               }

               if( !nWrite )
                  break;
            }

            if( !iStart )
               break;

            if( iStart < len )
               memmove( d->mccp->out_compress_buf, d->mccp->out_compress_buf + iStart, len - iStart );

            d->mccp->out_compress->next_out = d->mccp->out_compress_buf + len - iStart;
         }
      }
      return TRUE;
   }

   for( iStart = 0; iStart < length; iStart += nWrite )
   {
      nBlock = UMIN( length - iStart, 4096 );
      nWrite = send( d->descriptor, txt + iStart, nBlock, 0 );
      if( nWrite == -1 )
      {
         iErr = errno;
         if( iErr == EWOULDBLOCK )
         {
            /*
             * This is a SPAMMY little bug error. I would suggest
             * not using it, but I've included it in case. -Orion
             *
             perror( "Write_to_descriptor: Send is blocking" );
             */
            nWrite = 0;
            continue;
         }
         else
         {
            perror( "Write_to_descriptor" );
            return FALSE;
         }
      }
   }
   return TRUE;
}

void descriptor_printf( DESCRIPTOR_DATA * d, const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH * 2];

    va_list args;

    va_start( args, fmt );
    vsprintf( buf, fmt, args );
    va_end( args );

    write_to_descriptor( d, buf, strlen( buf ) );
}

/*
 *
 * Added block checking to prevent random booting of the descriptor. Thanks go
 * out to Rustry for his suggestions. -Orion
 */
bool write_to_descriptor_old( int desc, const char *txt, int length )
{
   int iStart = 0;
   int nWrite = 0;
   int nBlock = 0;
   int iErr = 0;

   if( length <= 0 )
      length = strlen( txt );

   for( iStart = 0; iStart < length; iStart += nWrite )
   {
      nBlock = UMIN( length - iStart, 4096 );
      nWrite = send( desc, txt + iStart, nBlock, 0 );

      if( nWrite == -1 )
      {
         iErr = errno;
         if( iErr == EWOULDBLOCK )
         {
            /*
             * This is a SPAMMY little bug error. I would suggest
             * not using it, but I've included it in case. -Orion
             *
             perror( "Write_to_descriptor: Send is blocking" );
             */
            nWrite = 0;
            continue;
         }
         else
         {
            perror( "Write_to_descriptor" );
            return FALSE;
         }
      }
   }
   return TRUE;
}

void show_title( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch;

   ch = d->character;

   if( !IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
   {
      if( IS_SET( ch->act, PLR_ANSI ) )
         send_ansi_title( ch );
      else
         send_ascii_title( ch );
   }
   else
   {
      write_to_buffer( d, "Press enter...\r\n", 0 );
   }
   d->connected = CON_PRESS_ENTER;
}

void nanny_get_name( DESCRIPTOR_DATA *d, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   BAN_DATA *pban;
   bool fOld;
   short chk;

   if( argument[0] == '\0' )
   {
      close_socket( d, FALSE );
      return;
   }

   argument[0] = UPPER( argument[0] );

   if( !str_cmp( argument, "MSSP-REQUEST" ) )
   {
      send_mssp_data( d );
      //Uncomment below if you want to know when an MSSP request occurs
      //log_printf( "IP: %s requested MSSP data!", d->host );
      close_socket( d, FALSE );
      return;
   }

   if( !check_parse_name( argument ) )
   {
      write_to_buffer( d, "Illegal name, try another.\r\nName: ", 0 );
      return;
   }

   if( !str_cmp( argument, "New" ) )
   {
      if( d->newstate == 0 )
      {
         /*
          * New player 
          * Don't allow new players if DENY_NEW_PLAYERS is true 
          */
         if( sysdata.DENY_NEW_PLAYERS == TRUE )
         {
            write_to_buffer( d, "The mud is currently preparing for a reboot.\r\n", 0 );
            write_to_buffer( d, "New players are not accepted during this time.\r\n", 0 );
            write_to_buffer( d, "Please try again in a few minutes.\r\n", 0 );
            close_socket( d, FALSE );
         }
         write_to_buffer( d, "\r\nChoosing a name is one of the most important parts of this game...\r\n"
            "Make sure to pick a name appropriate to the character you are going\r\n"
            "to role play, and be sure that it suits our theme.\r\n"
            "If the name you select is not acceptable, you will be asked to choose\r\n"
            "another one.\r\n\r\nPlease choose a name for your character: ", 0 );
         d->newstate++;
         d->connected = CON_GET_NAME;
         return;
      }
      else
      {
         send_to_desc_color( "Illegal name, try another.\r\nName: ", d );
         return;
      }
   }

   if( check_playing( d, argument, FALSE ) == BERR )
   {
      write_to_buffer( d, "Name: ", 0 );
      return;
   }

   fOld = load_char_obj( d, argument, TRUE, FALSE );
   if( !d->character )
   {
      log_printf( "Bad player file %s@%s.", argument, d->host );

      write_to_buffer( d, "Your playerfile is corrupt...Please notify the admins.\r\n", 0 );
      close_socket( d, FALSE );
      return;
   }
   ch = d->character;

   for( pban = first_ban; pban; pban = pban->next )
   {
      if( ( !str_prefix( pban->name, d->host )
         || !str_suffix( pban->name, d->host ) ) && pban->level >= ch->top_level )
      {
         write_to_buffer( d, "Your site has been banned from this Mud.\r\n", 0 );
         close_socket( d, FALSE );
         return;
      }
   }
   if( IS_SET( ch->act, PLR_DENY ) )
   {
      log_printf_plus( LOG_COMM, sysdata.log_level, "Denying access to %s@%s.", argument, d->host );

      if( d->newstate != 0 )
      {
         write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
         d->connected = CON_GET_NAME;
         return;
      }
      write_to_buffer( d, "You are denied access.\r\n", 0 );
      close_socket( d, FALSE );
      return;
   }

   chk = check_reconnect( d, argument, FALSE );
   if( chk == BERR )
      return;

   if( chk )
   {
      fOld = TRUE;
   }
   else
   {
      if( wizlock && !IS_IMMORTAL( ch ) )
      {
         write_to_buffer( d, "The game is wizlocked.  Only immortals can connect now.\r\n", 0 );
         write_to_buffer( d, "Please try back later.\r\n", 0 );
         close_socket( d, FALSE );
         return;
      }
   }

   if( fOld )
   {
      if( d->newstate != 0 )
      {
         write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
         d->connected = CON_GET_NAME;
         return;
      }
      /*
       * Old player 
       */
      write_to_buffer( d, "Password: ", 0 );
      write_to_buffer( d, (const char *)echo_off_str, 0 );
      d->connected = CON_GET_OLD_PASSWORD;
      return;
   }
   else
   {
      SET_BIT( ch->act, PLR_ANSI );
      send_to_desc_color( "\r\n&zI don't recognize your name, you must be new here.\r\n\r\n", d );
      snprintf( buf, MAX_STRING_LENGTH, "Did I get that right, &W%s &z(&WY&z/&WN&z)? &w", argument );
      send_to_desc_color( buf, d );
      d->connected = CON_CONFIRM_NEW_NAME;
      return;
   }
}

void nanny_get_old_password( DESCRIPTOR_DATA * d, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   short chk;

   write_to_buffer( d, "\r\n", 2 );

   if( strcmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
   {
      write_to_buffer( d, "Wrong password.\r\n", 0 );
      /*
       * clear descriptor pointer to get rid of bug message in log 
       */
      d->character->desc = NULL;
      log_printf( "%s@%s: Invalid password.", ch->name, d->host );
      close_socket( d, FALSE );
      return;
   }

   write_to_buffer( d, (const char *)echo_on_str, 0 );

   if( check_playing( d, ch->name, TRUE ) )
      return;

   chk = check_reconnect( d, ch->name, TRUE );
   if( chk == BERR )
   {
      if( d->character && d->character->desc )
         d->character->desc = NULL;
      close_socket( d, FALSE );
      return;
   }
   if( chk == TRUE )
      return;

   if( check_multi( d, ch->name ) )
   {
      close_socket( d, FALSE );
      return;
   }

   mudstrlcpy( buf, ch->name, MAX_STRING_LENGTH );
   d->character->desc = NULL;
   free_char( d->character );
   load_char_obj( d, buf, FALSE, FALSE );
   ch = d->character;
   log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->top_level ), "%s (%s) has connected.", ch->name, d->host );
   show_title( d );
   if( ch->pcdata->area )
      do_loadarea( ch, "" );

   if( ch->max_mana != ( ch->force_control + ch->force_sense + ch->force_alter ) * 3 * ch->force_level_status )
      ch->max_mana = ( ch->force_control + ch->force_sense + ch->force_alter ) * 3 * ch->force_level_status;
   ch->mana = ch->max_mana;
   if( ch->force_identified == 1 && ch->force_level_status == FORCE_MASTER )
   {
      int ft;
      FORCE_SKILL *skill;

      if( ch->force_skill[FORCE_SKILL_PARRY] < 50 )
         ch->force_skill[FORCE_SKILL_PARRY] = 50;
      ft = ch->force_type;
      for( skill = first_force_skill; skill; skill = skill->next )
         if( ( skill->type == ft || skill->type == FORCE_GENERAL ) && ch->force_skill[skill->index] < 50
            && ( strcmp( skill->name, "master" ) && strcmp( skill->name, "student" )
               && strcmp( skill->name, "promote" ) && strcmp( skill->name, "instruct" ) ) )
            ch->force_skill[skill->index] = 50;
   }
}

void nanny_confirm_new_name( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;

   switch ( *argument )
   {
      case 'y':
      case 'Y':
         snprintf( buf, MAX_STRING_LENGTH, "\r\n&zMake sure to use a password that won't be easily guessed by someone else."
                        "\r\nPick a good password for %s: &w%s", ch->name, (const char *)echo_off_str );
         send_to_desc_color( buf, d );
         d->connected = CON_GET_NEW_PASSWORD;
         break;

      case 'n':
      case 'N':
         send_to_desc_color( "&zOk, what is it, then? &w", d );
         /*
          * clear descriptor pointer to get rid of bug message in log 
          */
         d->character->desc = NULL;
         free_char( d->character );
         d->character = NULL;
         d->connected = CON_GET_NAME;
         break;

      default:
         send_to_desc_color( "&zPlease type &WYes&z or &WNo&z. &w", d );
         break;
   }
}

void nanny_get_new_password( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch = d->character;
   char *pwdnew;

   write_to_buffer( d, "\r\n", 2 );

   if( strlen( argument ) < 5 )
   {
      send_to_desc_color( "&zPassword must be at least five characters long.\r\nPassword: &w", d );
      return;
   }

   if( argument[0] == '!' )
   {
      write_to_buffer( d, "Password cannot begin with the '!' character.\r\nPassword: ", 0 );
      return;
   }

   pwdnew = sha256_crypt( argument );

   DISPOSE( ch->pcdata->pwd );
   ch->pcdata->pwd = str_dup( pwdnew );
   send_to_desc_color( "\r\n&zPlease retype the password to confirm: &w", d );
   d->connected = CON_CONFIRM_NEW_PASSWORD;
}

void nanny_confirm_new_password( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch = d->character;

   write_to_buffer( d, "\r\n", 2 );
   
   if( strcmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
   {
      send_to_desc_color( "&zPasswords don't match.\r\nRetype password: &w", d );
      d->connected = CON_GET_NEW_PASSWORD;
      return;
   }

   write_to_buffer( d, (const char *)echo_on_str, 0 );
   send_to_desc_color( "\r\n&zWhat is your sex (&WM&z/&WF&z/&WN&z)? &w", d );
   d->connected = CON_GET_NEW_SEX;
}

void nanny_get_new_sex( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iRace, col = 0;

   switch ( argument[0] )
   {
      case 'm':
      case 'M':
         ch->sex = SEX_MALE;
         break;
      case 'f':
      case 'F':
         ch->sex = SEX_FEMALE;
         break;
      case 'n':
      case 'N':
         ch->sex = SEX_NEUTRAL;
         break;
      default:
         send_to_desc_color( "&zThat's not a sex.\r\nWhat is your sex?&w ", d );
         return;
   }
   send_to_desc_color( "\r\n&zYou may choose from the following races, or type &Whelp [race]&z to learn more.\r\n", d );
   send_to_desc_color( "Keep in mind that you must ROLEPLAY the race you select. If you choose a\r\n", d );
   send_to_desc_color( "droid, you must RP your character accordingly. If you choose noghri, you\r\n", d );
   send_to_desc_color( "must roleplay your character as the race is expected. This applies to all\r\n", d );
   send_to_desc_color( "races, and if you are unsure about how to RP a race, refer to the helpfile.\r\n", d );
   send_to_desc_color( "If you are still unsure, do not pick that race.\r\n\r\n", d );
   buf[0] = '\0';
   col = 0;
   for( iRace = 0; iRace < MAX_RACE; iRace++ )
   {
      if( race_table[iRace].race_name && race_table[iRace].race_name[0] != '\0' )
      {
         if( iRace >= 0 )
         {
            snprintf( buf2, MAX_STRING_LENGTH, "&R[&z%-15.15s&R]&w  ", race_table[iRace].race_name );
            mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
            if( ++col % 4 == 0 )
            {
               mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               send_to_desc_color( buf, d );
               buf[0] = '\0';
            }
         }
      }
   }
   if( col % 4 != 0 )
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   mudstrlcat( buf, "&z:&w ", MAX_STRING_LENGTH );
   send_to_desc_color( buf, d );
   d->connected = CON_GET_NEW_RACE;
}

void nanny_get_new_race( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iRace, iClass, col = 0;

   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "help" ) )
   {
      do_help( ch, argument );
      send_to_desc_color( "&zPlease choose a race:&w ", d );
      return;
   }

   for( iRace = 0; iRace < MAX_RACE; iRace++ )
   {
      if( toupper( arg[0] ) == toupper( race_table[iRace].race_name[0] )
         && !str_prefix( arg, race_table[iRace].race_name ) )
      {
         ch->race = iRace;
         break;
      }
   }

   if( iRace == MAX_RACE || !race_table[iRace].race_name || race_table[iRace].race_name[0] == '\0' )
   {
      send_to_desc_color( "&zThat's not a race.\r\nWhat is your race?&w ", d );
      return;
   }

   send_to_desc_color( "\r\n&zPlease choose a main ability from the following classes:&w\r\n", d );
   buf[0] = '\0';
   col = 0;
   for( iClass = 0; iClass < MAX_ABILITY; iClass++ )
   {
      if( ability_name[iClass] && ability_name[iClass][0] != '\0' && str_cmp( ability_name[iClass], "force" ) )
      {
         snprintf( buf2, MAX_STRING_LENGTH, "&R[&z%-15.15s&R]&w  ", ability_name[iClass] );
         mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
         if( ++col % 4 == 0 )
         {
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_desc_color( buf, d );
            buf[0] = '\0';
         }
      }
   }
   if( col % 4 != 0 )
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   mudstrlcat( buf, "&z:&w ", MAX_STRING_LENGTH );

   send_to_desc_color( buf, d );
   d->connected = CON_GET_NEW_CLASS;
}

void nanny_get_new_primary( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iClass, col = 0;

   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "help" ) )
   {
      do_help( ch, argument );
      send_to_desc_color( "&zPlease choose an ability class:&w ", 0 );
      return;
   }
   
   for( iClass = 0; iClass < MAX_ABILITY; iClass++ )
   {
      if( toupper( arg[0] ) == toupper( ability_name[iClass][0] )
         && !str_prefix( arg, ability_name[iClass] ) && str_prefix( arg, "force" ) )
      {
         ch->main_ability = iClass;
         break;
      }
   }

   if( iClass == MAX_ABILITY || !ability_name[iClass] || ability_name[iClass][0] == '\0' )
   {
      send_to_desc_color( "&zThat's not a skill class.\r\nWhat is it going to be? &w", d );
      return;
   }
   send_to_desc_color( "\r\n&zPlease choose a secondary ability from the following classes:&w\r\n", d );
   buf[0] = '\0';
   col = 0;
   for( iClass = 0; iClass < MAX_ABILITY; iClass++ )
   {
      if( ability_name[iClass] && ability_name[iClass][0] != '\0' && str_cmp( ability_name[iClass], "force" )
         && ch->main_ability != iClass )
      {
         snprintf( buf2, MAX_STRING_LENGTH, "&R[&z%-15.15s&R]&w  ", ability_name[iClass] );
         mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
         if( ++col % 4 == 0 )
         {
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_desc_color( buf, d );
            buf[0] = '\0';
         }
      }
   }
   if( col % 4 != 0 )
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   mudstrlcat( buf, "&z:&w ", MAX_STRING_LENGTH );

   send_to_desc_color( buf, d );
   d->connected = CON_GET_NEW_SECOND;
}

void nanny_get_new_secondary( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iClass;

   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "help" ) )
   {
      do_help( ch, argument );
      send_to_desc_color( "&zPlease choose an ability class:&w ", d );
      return;
   }

   for( iClass = 0; iClass < MAX_ABILITY; iClass++ )
   {
      if( toupper( arg[0] ) == toupper( ability_name[iClass][0] )
         && !str_prefix( arg, ability_name[iClass] ) && str_prefix( arg, "force" ) && ch->main_ability != iClass )
      {
         ch->secondary_ability = iClass;
         break;
      }
   }

   if( iClass == MAX_ABILITY || !ability_name[iClass] || ability_name[iClass][0] == '\0' )
   {
      send_to_desc_color( "&zThat's not a skill class.\r\nWhat is it going to be?&w ", d );
      return;
   }

   send_to_desc_color( "\r\n&zRolling stats...\r\n", d );
   ch->perm_str = number_range( 1, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
   ch->perm_int = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
   ch->perm_wis = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
   ch->perm_dex = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
   ch->perm_con = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
   ch->perm_cha = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
   ch->perm_str += race_table[ch->race].str_plus;
   ch->perm_int += race_table[ch->race].int_plus;
   ch->perm_wis += race_table[ch->race].wis_plus;
   ch->perm_dex += race_table[ch->race].dex_plus;
   ch->perm_con += race_table[ch->race].con_plus;
   ch->perm_cha += race_table[ch->race].cha_plus;

   snprintf( buf, MAX_STRING_LENGTH, "\r\n&zSTR: &R%d  &zINT: &R%d  &zWIS: &R%d  &zDEX: &R%d  &zCON: &R%d  &zCHA: &R%d\r\n",
      ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha );

   send_to_desc_color( buf, d );
   send_to_desc_color( "\r\n&zAre these stats OK?&w ", d );
   d->connected = CON_ROLL_STATS;
}

void nanny_roll_stats( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iClass, iDroid, col = 0;

   switch ( argument[0] )
   {
      case 'y':
      case 'Y':
         break;
      case 'n':
      case 'N':
         ch->perm_str = number_range( 1, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
         ch->perm_int = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
         ch->perm_wis = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
         ch->perm_dex = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
         ch->perm_con = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );
         ch->perm_cha = number_range( 3, 6 ) + number_range( 1, 6 ) + number_range( 1, 6 );

         ch->perm_str += race_table[ch->race].str_plus;
         ch->perm_int += race_table[ch->race].int_plus;
         ch->perm_wis += race_table[ch->race].wis_plus;
         ch->perm_dex += race_table[ch->race].dex_plus;
         ch->perm_con += race_table[ch->race].con_plus;
         ch->perm_cha += race_table[ch->race].cha_plus;

         snprintf( buf, MAX_STRING_LENGTH, "\r\n&zSTR: &R%d  &zINT: &R%d  &zWIS: &R%d  &zDEX: &R%d  &zCON: &R%d  &zCHA: &R%d\r\n",
            ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha );

         send_to_desc_color( buf, d );
         send_to_desc_color( "\r\n&zOK?&w ", d );
         return;
      default:
         send_to_desc_color( "&zInvalid selection.\r\n&WYES&z or &WNO&z? ", d );
         return;
   }
   
   if( !IS_DROID( ch ) )
   {
      send_to_desc_color( "\r\n&zPlease choose a height from the following list:&w\r\n", d );
      buf[0] = '\0';
      col = 0;
      for( iClass = 0; iClass < 4; iClass++ )
      {
         if( height_name[iClass] && height_name[iClass][0] != '\0' )
         {
            snprintf( buf3, MAX_STRING_LENGTH, "%s", height_name[iClass] );
            buf3[0] = UPPER( buf3[0] );
            snprintf( buf2, MAX_STRING_LENGTH, "&R[&z%-15.15s&R]&w  ", buf3 );
            mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
            if( ++col % 4 == 0 )
            {
               mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               send_to_desc_color( buf, d );
               buf[0] = '\0';
            }
         }
      }
   }
   else
   {
      send_to_desc_color( "\r\n&zPlease choose a droid description from the following list:&w\r\n", d );
      buf[0] = '\0';
      col = 0;
      for( iDroid = 0; iDroid < 8; iDroid++ )
      {
         if( droid_name[iDroid] && droid_name[iDroid][0] != '\0' )
         {
            snprintf( buf3, MAX_STRING_LENGTH, "%s", droid_name[iDroid] );
            buf3[0] = UPPER( buf3[0] );
            snprintf( buf2, MAX_STRING_LENGTH, "&R[&z%-15.15s&R]&w  ", buf3 );
            mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
            if( ++col % 4 == 0 )
            {
               mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               send_to_desc_color( buf, d );
               buf[0] = '\0';
            }
         }
      }
   }
   if( col % 4 != 0 )
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   mudstrlcat( buf, "&z:&w ", MAX_STRING_LENGTH );

   send_to_desc_color( buf, d );
   if( !IS_DROID( ch ) )
      d->connected = CON_GET_HEIGHT;
   else
      d->connected = CON_GET_DROID;
}

void nanny_get_height( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iClass, col = 0;

   argument = one_argument( argument, arg );
   for( iClass = 0; iClass < 4; iClass++ )
   {
      if( toupper( arg[0] ) == toupper( height_name[iClass][0] ) && !str_prefix( arg, height_name[iClass] ) )
      {
         ch->pheight = iClass;
         break;
      }
   }

   if( iClass == 4 || !height_name[iClass] || height_name[iClass][0] == '\0' )
   {
      send_to_desc_color( "&zThat's not a height.\r\nWhat is it going to be?&w ", d );
      return;
   }

   send_to_desc_color( "\r\n&zPlease choose a build from the following list:&w\r\n", d );
   buf[0] = '\0';
   col = 0;
   for( iClass = 0; iClass < 6; iClass++ )
   {
      if( build_name[iClass] && build_name[iClass][0] != '\0' )
      {
         snprintf( buf3, MAX_STRING_LENGTH, "%s", build_name[iClass] );
         buf3[0] = UPPER( buf3[0] );
         snprintf( buf2, MAX_STRING_LENGTH, "&R[&z%-15.15s&R]&w  ", buf3 );
         mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
         if( ++col % 4 == 0 )
         {
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_desc_color( buf, d );
            buf[0] = '\0';
         }
   
      }
   }
   if( col % 4 != 0 )
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   mudstrlcat( buf, "&z:&w ", MAX_STRING_LENGTH );
   send_to_desc_color( buf, d );
   d->connected = CON_GET_BUILD;
}

void nanny_get_build( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iClass;

   argument = one_argument( argument, arg );
   for( iClass = 0; iClass < 6; iClass++ )
   {
      if( toupper( arg[0] ) == toupper( build_name[iClass][0] ) && !str_prefix( arg, build_name[iClass] ) )
      {
         ch->build = iClass;
         SET_BIT( ch->act, PLR_ANSI );
         snprintf( log_buf, MAX_STRING_LENGTH, "%s@%s new %s.", ch->name, d->host, race_table[ch->race].race_name );
         log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
         to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
         send_to_desc_color( "\r\n&R&zWelcome to &RFall of the Empire&z. Press enter to continue.&w\r\n\r\n", d );
         {
            int ability;
   
            for( ability = 0; ability < MAX_ABILITY; ability++ )
            {
               ch->skill_level[ability] = 0;
               ch->bonus[ability] = 0;
            }
         }
         ch->top_level = 0;
         ch->position = POS_STANDING;
         // Threw in commfreq here, hope it doesn't fuck with anything
         snprintf( buf, MAX_STRING_LENGTH, "%d%d%d.%d%d%d",
            number_range( 0, 9 ), number_range( 0, 9 ),
            number_range( 0, 9 ), number_range( 0, 9 ), number_range( 0, 9 ), number_range( 0, 9 ) );

         if( ch->comfreq )
            STRFREE( ch->comfreq );
         ch->comfreq = STRALLOC( buf );
         d->connected = CON_PRESS_ENTER;
         break;
      }
   }

   if( iClass == 6 || !build_name[iClass] || build_name[iClass][0] == '\0' )
   {
      send_to_desc_color( "&zThat's not a build.\r\nWhat is it going to be?&w ", d );
      return;
   }
}

void nanny_get_droid( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;
   int iDroid;

   if( IS_DROID( ch ) )
   {
      argument = one_argument( argument, arg );
      for( iDroid = 0; iDroid < 8; iDroid++ )
      {
         if( toupper( arg[0] ) == toupper( droid_name[iDroid][0] ) && !str_prefix( arg, droid_name[iDroid] ) )
         {
            ch->build = iDroid;
            SET_BIT( ch->act, PLR_ANSI );
            snprintf( log_buf, MAX_STRING_LENGTH, "%s@%s new %s.", ch->name, d->host, race_table[ch->race].race_name );
            log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
            send_to_desc_color( "\r\n&R&zWelcome to &RFall of the Empire&z. Press enter to continue.&w\r\n\r\n", d );
            {
               int ability;

               for( ability = 0; ability < MAX_ABILITY; ability++ )
               {
                  ch->skill_level[ability] = 0;
                  ch->bonus[ability] = 0;
               }
            }
            ch->top_level = 0;
            ch->position = POS_STANDING;
            // Threw in commfreq here, hope it doesn't fuck with anything
            snprintf( buf, MAX_STRING_LENGTH, "%d%d%d.%d%d%d",
                  number_range( 0, 9 ),
                  number_range( 0, 9 ),
                  number_range( 0, 9 ), number_range( 0, 9 ), number_range( 0, 9 ), number_range( 0, 9 ) );

            if( ch->comfreq )
               STRFREE( ch->comfreq );
            ch->comfreq = STRALLOC( buf );
            d->connected = CON_PRESS_ENTER;
            break;
         }
      }

      if( iDroid == 8 || !droid_name[iDroid] || droid_name[iDroid][0] == '\0' )
      {
         send_to_desc_color( "&zThat's not a droid description.\r\nWhat is it going to be?&w ", d );
         return;
      }
   }
}

void nanny_press_enter( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch = d->character;

   if( ch->top_level >= 0 )
   {
      send_to_pager( "\r\n&WMessage of the Day&w\r\n", ch );
      do_help( ch, "motd" );
   }
/* Uh, do we use this?
   if( IS_IMMORTAL(ch) )
   {
      send_to_pager( "&WImmortal Message of the Day&w\r\n", ch );
      do_help( ch, "imotd" );
   }
*/
   send_to_pager( "\r\n&WPress [ENTER] &w", ch );
   d->connected = CON_READ_MOTD;
}

void nanny_read_motd( DESCRIPTOR_DATA * d, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch = d->character;

   write_to_buffer( d, "\r\n\r\n", 0 );
   add_char( ch );

   log_printf_plus( LOG_NORMAL, get_trust( ch ), " %s has entered the game.", ch->name  );
   d->connected = CON_PLAYING;

   if( ch->top_level == 0 )
   {
      OBJ_DATA *obj;
      int iLang;

      ch->pcdata->clan = NULL;
      ch->pcdata->learned[gsn_smallspace] = 25;
      ch->pcdata->learned[gsn_shipsystems] = 25;
      ch->pcdata->learned[gsn_navigation] = 25;
      ch->pcdata->learned[gsn_scan] = 25;

      ch->perm_lck = number_range( 6, 18 );
      ch->perm_frc = number_range( -1000, 20 );
      ch->affected_by = race_table[ch->race].affected;
      ch->perm_lck += race_table[ch->race].lck_plus;
      ch->perm_frc += race_table[ch->race].frc_plus;

      if( ch->main_ability == FORCE_ABILITY )
         ch->perm_frc = URANGE( 0, ch->perm_frc, 20 );
      else
         ch->perm_frc = URANGE( 0, ch->perm_frc, 20 );

      if( ch->main_ability == HUNTING_ABILITY || ch->main_ability == ASSASSIN_ABILITY || IS_DROID( ch ) )
         ch->perm_frc = 0;

      for( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
         if( lang_array[iLang] == race_table[ch->race].language )
            break;
      if( lang_array[iLang] == LANG_UNKNOWN )
      {
         if( IS_DROID( ch ) && ( iLang = skill_lookup( "binary" ) ) >= 0 )
         {
            ch->pcdata->learned[iLang] = 100;
            ch->speaking = LANG_BINARY;
            SET_BIT( ch->speaks, LANG_BINARY );
         }
         else
            bug( "%s: invalid racial language.", __func__ );
      }
      else
      {
         if( ( iLang = skill_lookup( lang_names[iLang] ) ) < 0 )
            bug( "%s: cannot find racial language.", __func__ );
         else
         {
            ch->pcdata->learned[iLang] = 100;
            ch->speaking = race_table[ch->race].language;
            if( ch->race == RACE_QUARREN && ( iLang = skill_lookup( "quarren" ) ) >= 0 )
            {
               ch->pcdata->learned[iLang] = 100;
               SET_BIT( ch->speaks, LANG_QUARREN );
            }
            if( ch->race == RACE_MON_CALAMARI && ( iLang = skill_lookup( "basic" ) ) >= 0 )
               ch->pcdata->learned[iLang] = 100;
         }
      }

      reset_colors( ch );
      name_stamp_stats( ch );

      {
         int ability;

         for( ability = 0; ability < MAX_ABILITY; ability++ )
         {
            ch->skill_level[ability] = 1;
            ch->experience[ability] = 0;
            ch->bonus[ability] = 0;
         }
      }
      ch->top_level = 1;
      ch->hit = ch->max_hit;
      ch->hit += race_table[ch->race].hit;
      ch->move = ch->max_move;
      if( ch->perm_frc > 0 )
         ch->max_mana = 100 + 100 * ch->perm_frc;
      else
         ch->max_mana = 0;
      ch->mana = ch->max_mana;
      snprintf( buf, MAX_STRING_LENGTH, "%s the %s", ch->name, race_table[ch->race].race_name );
      set_title( ch, buf );

      /*
       * Newbies get some cash! - Tawnos 
       */
      ch->gold = 10000;

      /*
       * Added by Narn.  Start new characters with autoexit and autgold
       * already turned on.  Very few people don't use those. 
       */
      SET_BIT( ch->act, PLR_AUTOGOLD );
      SET_BIT( ch->act, PLR_AUTOEXIT );

      /*
       * New players don't have to earn some eq 
       */
      if( ch->race != RACE_DEFEL )
      {
         obj = create_object( get_obj_index( 121 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_HANDS );
         obj = create_object( get_obj_index( 122 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_BODY );
         obj = create_object( get_obj_index( 123 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_HEAD );
         obj = create_object( get_obj_index( 124 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_FEET );
         obj = create_object( get_obj_index( 125 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_WAIST );
         obj = create_object( get_obj_index( 126 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_ARMS );
         obj = create_object( get_obj_index( 127 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_LEGS );
         obj = create_object( get_obj_index( 128 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_SHIELD );
         obj = create_object( get_obj_index( 129 ), 0 );
         obj_to_char( obj, ch );
         equip_char( ch, obj, WEAR_ABOUT );
      }
      obj = create_object( get_obj_index( 130 ), 0 );
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_WIELD );
      obj = create_object( get_obj_index( 131 ), 0 );
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_EARS );
      obj = create_object( get_obj_index( 132 ), 0 );
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_BACK );

      if( !sysdata.WAIT_FOR_AUTH )
      {
         char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
         ch->pcdata->auth_state = 3;
      }
      else
      {
         char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
         ch->pcdata->auth_state = 1;
         SET_BIT( ch->pcdata->flags, PCFLAG_UNAUTHED );
      }
   }
   else if( !IS_IMMORTAL( ch ) && ch->pcdata->release_date > current_time )
   {
      char_to_room( ch, get_room_index( 6 ) );
   }
   else if( ch->in_room && !IS_IMMORTAL( ch ) && !IS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) && ch->in_room != get_room_index( 6 ) )
   {
      char_to_room( ch, ch->in_room );
   }
   else if( ch->in_room && !IS_IMMORTAL( ch ) && IS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) && ch->in_room != get_room_index( 6 ) )
   {
      /*
       * SHIP_DATA *ship;
       * 
       * for ( ship = first_ship; ship; ship = ship->next )
       * if ( ch->in_room->vnum >= ship->firstroom && ch->in_room->vnum <= ship->lastroom )
       * if ( ship->sclass != SHIP_PLATFORM || ship->starsystem ) 
       */
      char_to_room( ch, ch->in_room );
   }
   else
   {
      char_to_room( ch, get_room_index( wherehome( ch ) ) );
   }

   if( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
      remove_timer( ch, TIMER_SHOVEDRAG );

   if( get_timer( ch, TIMER_PKILLED ) > 0 )
      remove_timer( ch, TIMER_PKILLED );

   {
      int ability;

      for( ability = 0; ability < MAX_ABILITY; ability++ )
         if( ch->skill_level[ability] > max_level( ch, ability ) )
            ch->skill_level[ability] = max_level( ch, ability );
   }

   //act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   mail_count( ch );
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA * d, char *argument )
{
   while( isspace( *argument ) )
      argument++;

   switch ( d->connected )
   {
      default:
         bug( "%s: bad d->connected %d.", __func__, d->connected );
         close_socket( d, TRUE );
         return;

      case CON_GET_NAME:
         nanny_get_name( d, argument );
         break;

      case CON_GET_OLD_PASSWORD:
         nanny_get_old_password( d, argument );
         break;

      case CON_CONFIRM_NEW_NAME:
         nanny_confirm_new_name( d, argument );
         break;

      case CON_GET_NEW_PASSWORD:
         nanny_get_new_password( d, argument );
         break;

      case CON_CONFIRM_NEW_PASSWORD:
         nanny_confirm_new_password( d, argument );
         break;

      case CON_GET_NEW_SEX:
         nanny_get_new_sex( d, argument );
         break;

      case CON_GET_NEW_RACE:
         nanny_get_new_race( d, argument );
         break;

      case CON_GET_NEW_CLASS:
         nanny_get_new_primary( d, argument );
         break;

      case CON_GET_NEW_SECOND:
         nanny_get_new_secondary( d, argument );
         break;
         
      case CON_ROLL_STATS:
         nanny_roll_stats( d, argument );
         break;

      case CON_GET_HEIGHT:
         nanny_get_height( d, argument );
         break;

      case CON_GET_BUILD:
         nanny_get_build( d, argument );
         break;         

      case CON_GET_DROID:
         nanny_get_droid( d, argument );
         break;

      case CON_PRESS_ENTER:
         nanny_press_enter( d, argument );
         break;

      case CON_READ_MOTD:
         nanny_read_motd( d, argument );
         break;
   }
   return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( const char *name )
{
   /*
    * Reserved words.
    */
   if( is_name
       ( name,
         "all auto someone immortal self god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit public phines" ) )
      return FALSE;

   /*
    * Length restrictions.
    */
   if( strlen( name ) < 3 )
      return FALSE;

   if( strlen( name ) > 12 )
      return FALSE;

   /*
    * Alphanumerics only.
    * Lock out IllIll twits.
    */
   {
      const char *pc;
      bool fIll;

      fIll = TRUE;
      for( pc = name; *pc != '\0'; pc++ )
      {
         if( !isalpha( *pc ) )
            return FALSE;
         if( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
            fIll = FALSE;
      }

      if( fIll )
         return FALSE;
   }

   /*
    * Code that followed here used to prevent players from naming
    * themselves after mobs... this caused much havoc when new areas
    * would go in...
    */

   return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
short check_reconnect( DESCRIPTOR_DATA * d, char *name, bool fConn )
{
   CHAR_DATA *ch;

   for( ch = first_char; ch; ch = ch->next )
   {
      if( !IS_NPC( ch ) && ( !fConn || !ch->desc ) && ch->name && !str_cmp( name, ch->name ) )
      {
         if( fConn && ch->switched )
         {
            write_to_buffer( d, "Already playing.\r\nName: ", 0 );
            d->connected = CON_GET_NAME;
            if( d->character )
            {
               /*
                * clear descriptor pointer to get rid of bug message in log 
                */
               d->character->desc = NULL;
               free_char( d->character );
               d->character = NULL;
            }
            return BERR;
         }
         if( fConn == FALSE )
         {
            DISPOSE( d->character->pcdata->pwd );
            d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
         }
         else
         {
            /*
             * clear descriptor pointer to get rid of bug message in log 
             */
            d->character->desc = NULL;
            free_char( d->character );
            d->character = ch;
            ch->desc = d;
            ch->timer = 0;
            send_to_char( "Reconnecting.\r\n", ch );
            act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
            log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->top_level ), "%s (%s) reconnected.", ch->name, d->host );
            d->connected = CON_PLAYING;
         }
         return TRUE;
      }
   }

   return FALSE;
}

/*
 * Check if already playing.
 */
bool check_multi( DESCRIPTOR_DATA * d, char *name )
{
   DESCRIPTOR_DATA *dold;

   for( dold = first_descriptor; dold; dold = dold->next )
   {
      if( dold != d
          && ( dold->character || dold->original )
          && str_cmp( name, dold->original
                      ? dold->original->name : dold->character->name ) && !str_cmp( dold->host, d->host ) )
      {
         const char *ok = "194.234.177";
         const char *ok2 = "209.183.133.229";
         int iloop;

         if( get_trust( d->character ) >= LEVEL_SUPREME
             || get_trust( dold->original ? dold->original : dold->character ) >= LEVEL_SUPREME )
            return FALSE;
         for( iloop = 0; iloop < 11; iloop++ )
         {
            if( ok[iloop] != d->host[iloop] )
               break;
         }
         if( iloop >= 10 )
            return FALSE;
         for( iloop = 0; iloop < 11; iloop++ )
         {
            if( ok2[iloop] != d->host[iloop] )
               break;
         }
         if( iloop >= 10 )
            return FALSE;
         write_to_buffer( d, "Sorry multi-playing is not allowed ... have you other character quit first.\r\n", 0 );
         log_printf_plus( LOG_COMM, sysdata.log_level, "%s attempting to multiplay with %s.",
                  dold->original ? dold->original->name : dold->character->name, d->character->name );
         d->character = NULL;
         free_char( d->character );
         return TRUE;
      }
   }

   return FALSE;

}

short check_playing( DESCRIPTOR_DATA * d, char *name, bool kick )
{
   CHAR_DATA *ch;

   DESCRIPTOR_DATA *dold;
   int cstate;

   for( dold = first_descriptor; dold; dold = dold->next )
   {
      if( dold != d
          && ( dold->character || dold->original )
          && !str_cmp( name, dold->original ? dold->original->name : dold->character->name ) )
      {
         cstate = dold->connected;
         ch = dold->original ? dold->original : dold->character;
         if( !ch->name || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
         {
            write_to_buffer( d, "Already connected - try again.\r\n", 0 );
            log_printf_plus( LOG_COMM, sysdata.log_level, "%s already connected.", ch->name );
            return BERR;
         }
         if( !kick )
            return TRUE;
         write_to_buffer( d, "Already playing... Kicking off old connection.\r\n", 0 );
         write_to_buffer( dold, "Kicking off old connection... bye!\r\n", 0 );
         close_socket( dold, FALSE );
         /*
          * clear descriptor pointer to get rid of bug message in log 
          */
         d->character->desc = NULL;
         free_char( d->character );
         d->character = ch;
         ch->desc = d;
         ch->timer = 0;
         if( ch->switched )
            do_return( ch->switched, "" );
         ch->switched = NULL;
         send_to_char( "Reconnecting.\r\n", ch );
         act( AT_ACTION, "$n has reconnected, kicking off old link.", ch, NULL, NULL, TO_ROOM );
         log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->top_level ), "%s@%s reconnected, kicking off old link.", ch->name, d->host );
         d->connected = cstate;
         return TRUE;
      }
   }

   return FALSE;
}

void stop_idling( CHAR_DATA * ch )
{
   if( !ch
       || !ch->desc
       || ch->desc->connected != CON_PLAYING || !ch->was_in_room || ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
      return;

   ch->timer = 0;
   char_from_room( ch );
   char_to_room( ch, ch->was_in_room );
   ch->was_in_room = NULL;
   act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
   return;
}

/*
 * Write to one char. Commented out in favour of colour
 * Update: Need a use for this, no color, removes &'s from string
 * Used for infrared viewing, bright red on players/mobiles
 * (sets color before sending text, act_info.c send_char_to_char_0)
 */
void send_to_char_noand( const char *txt, CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];

   if( !ch )
   {
      bug( "%s: NULL *ch", __func__ );
      return;
   }
   if( txt && ch->desc )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s", txt );
      snprintf( buf, MAX_STRING_LENGTH, "%s", remand( buf ) );
      write_to_buffer( ch->desc, buf, strlen( buf ) );
   }
   return;
}

char *obj_short( OBJ_DATA * obj )
{
   static char buf[MAX_STRING_LENGTH];

   if( obj->count > 1 )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s (%d)", obj->short_descr, obj->count );
      return buf;
   }
   return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
/* Changed so it shows PERS instead of ch->name... sometimes -- Tawnos */
#define NAME(ch)	(IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string( const char *format, CHAR_DATA * to, CHAR_DATA * ch, const void *arg1, const void *arg2 )
{
   static const char *const he_she[] = { "it", "he", "she" };
   static const char *const him_her[] = { "it", "him", "her" };
   static const char *const his_her[] = { "its", "his", "her" };
   static char buf[MAX_STRING_LENGTH];
   char fname[MAX_INPUT_LENGTH];
   char *point = buf;
   char temp[MSL];
   const char *str = format;
   const char *i;
   bool should_upper = false;
   CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
   OBJ_DATA *obj1 = ( OBJ_DATA * ) arg1;
   OBJ_DATA *obj2 = ( OBJ_DATA * ) arg2;

   if( str[0] == '$' )
      DONT_UPPER = false;

   while( *str != '\0' )
   {
      if( *str == '.' || *str == '?' || *str == '!' )
         should_upper = true;
      else if( should_upper == true && !isspace( *str ) && *str != '$' )
         should_upper = false;

      if( *str != '$' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      if( !arg2 && *str >= 'A' && *str <= 'Z' )
      {
         bug( "%s: missing arg2 for code %c:", __func__, *str );
         bug( "%s", format );
         i = " <@@@> ";
      }
      else
      {
         switch ( *str )
         {
            default:
               bug( "%s: bad code %c.", __func__, *str );
               i = " <@@@> ";
               break;

            case 'd':
               if( !arg2 || ( ( char * )arg2 )[0] == '\0' )
                  i = "door";
               else
               {
                  one_argument( ( char * )arg2, fname );
                  i = fname;
               }
               break;

            case 'e':
               if( ch->sex > 2 || ch->sex < 0 )
               {
                  bug( "%s: player %s has sex set at %d!", __func__, ch->name, ch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, ch ) ? "It" : capitalize( he_she[URANGE( 0, ch->sex, 2 )] ) :
                   !can_see( to, ch ) ? "it" : he_she[URANGE( 0, ch->sex, 2 )];
               break;

            case 'E':
               if( vch->sex > 2 || vch->sex < 0 )
               {
                  bug( "%s: player %s has sex set at %d!", __func__, vch->name, vch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, vch ) ? "It" : capitalize( he_she[URANGE( 0, vch->sex, 2 )] ) :
                   !can_see( to, vch ) ? "it" : he_she[URANGE( 0, vch->sex, 2 )];
               break;

            case 'm':
               if( ch->sex > 2 || ch->sex < 0 )
               {
                  bug( "%s: player %s has sex set at %d!", __func__, ch->name, ch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, ch ) ? "It" : capitalize( him_her[URANGE( 0, ch->sex, 2 )] ) :
                   !can_see( to, ch ) ? "it" : him_her[URANGE( 0, ch->sex, 2 )];
               break;

            case 'M':
               if( vch->sex > 2 || vch->sex < 0 )
               {
                  bug( "%s: player %s has sex set at %d!", __func__, vch->name, vch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, vch ) ? "It" : capitalize( him_her[URANGE( 0, vch->sex, 2 )] ) :
                   !can_see( to, vch ) ? "it" : him_her[URANGE( 0, vch->sex, 2 )];
               break;

            case 'n':
               if( !can_see( to, ch ) )
                  i = "Someone";
               else
               {
                  snprintf( temp, sizeof( temp ), "%s", ( to ? PERS( ch, to ) : NAME( ch ) ) );
                  i = temp;
               }
               break;

            case 'N':
               if( !can_see( to, vch ) )
                  i = "Someone";
               else
               {
                  snprintf( temp, sizeof( temp ), "%s", ( to ? PERS( vch, to ) : NAME( vch ) ) );
                  i = temp;
               }
               break;

            case 'p':
               if( !obj1 )
               {
                  bug( "%s: $p used with NULL obj1!", __func__ );
                  i = "something";
               }
               else
                  i = should_upper ? ( ( !to || can_see_obj( to, obj1 ) ) ? capitalize( obj_short( obj1 ) ) : "Something" )
                                     :( ( !to || can_see_obj( to, obj1 ) ) ? obj_short( obj1 ) : "something" );
               break;

            case 'P':
               if( !obj2 )
               {
                  bug( "%s: $P used with NULL obj2!", __func__ );
                  i = "something";
               }
               else
                  i = should_upper ? (  !to || can_see_obj( to, obj2 ) ? capitalize( obj_short( obj2 ) ) : "Something" )
                                     :(  !to || can_see_obj( to, obj2 ) ? obj_short( obj2 ) : "something" );
               break;

            case 'q':
               i = ( to == ch ) ? "" : "s";
               break;
            case 'Q':
               i = ( to == ch ) ? "your" : his_her[URANGE( 0, ch->sex, 2 )];
               break;

            case 's':
               if( ch->sex > 2 || ch->sex < 0 )
               {
                  bug( "%s: player %s has sex set at %d!", __func__, ch->name, ch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, ch ) ? "It" : capitalize( his_her[URANGE( 0, ch->sex, 2 )] ) :
                   !can_see( to, ch ) ? "it" : his_her[URANGE( 0, ch->sex, 2 )];
               break;

            case 'S':
               if( vch->sex > 2 || vch->sex < 0 )
               {
                  bug( "%s: player %s has sex set at %d!", __func__, vch->name, vch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, vch ) ? "It" : capitalize( his_her[URANGE( 0, vch->sex, 2 )] ) :
                   !can_see( to, vch ) ? "it" : his_her[URANGE( 0, vch->sex, 2 )];
               break;

            case 't':
               i = ( char * )arg1;
               break;

            case 'T':
               i = ( char * )arg2;
               break;
         }
      }
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;
   }
   mudstrlcpy( point, "\r\n", MSL );
   if( !DONT_UPPER )
   {
      bool bUppercase = true;     //Always uppercase first letter 
      char *astr = buf; 
      for( char c = *astr; c; c= *++astr ) 
      { 
          if( c == '&' ) 
          { 
               //Color Code 
               c = *++astr;     //Read Color Code 
               if( c == '[' ) 
               { 
                    //Extended color code, skip until ']' 
                    do { c = *++astr; } while ( c && c != ']' ); 
               } 
      
               if( !c )     
                   break; 
          } 
          else if( bUppercase && isalpha( c ) ) 
          { 
               *astr = toupper(c); 
               bUppercase = false; 
          } 
     }
   }
   return buf;
}

#undef NAME

void act( short AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type )
{
   char *txt;
   CHAR_DATA *to;
   CHAR_DATA *vch = ( CHAR_DATA * ) arg2;

   /*
    * Discard null and zero-length messages.
    */
   if( !format || format[0] == '\0' )
      return;

   if( !ch )
   {
      bug( "%s: null ch. (%s)", __func__, format );
      return;
   }

   if( !ch->in_room )
      to = NULL;
   else if( type == TO_CHAR )
      to = ch;
   else if( type == TO_MUD )
      to = first_char;
   else
      to = ch->in_room->first_person;

   /*
    * ACT_SECRETIVE handling
    */
   if( IS_NPC( ch ) && IS_SET( ch->act, ACT_SECRETIVE ) && type != TO_CHAR )
      return;

   if( type == TO_VICT )
   {
      if( !vch )
      {
         bug( "%s: null vch with TO_VICT.", __func__ );
         bug( "%s (%s)", ch->name, format );
         return;
      }
      if( !vch->in_room )
      {
         bug( "%s: vch in NULL room!", __func__ );
         bug( "%s -> %s (%s)", ch->name, vch->name, format );
         return;
      }
      to = vch;
/*	to = vch->in_room->first_person;*/
   }

   if( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
   {
      OBJ_DATA *to_obj;

      txt = act_string( format, NULL, ch, arg1, arg2 );

      if( to && IS_SET( to->in_room->progtypes, ACT_PROG ) )
         rprog_act_trigger( txt, to->in_room, ch, ( OBJ_DATA * ) arg1, ( void * )arg2 );
      for( to_obj = to->in_room->first_content; to_obj; to_obj = to_obj->next_content )
         if( IS_SET( to_obj->pIndexData->progtypes, ACT_PROG ) )
            oprog_act_trigger( txt, to_obj, ch, ( OBJ_DATA * ) arg1, ( void * )arg2 );
   }

   /*
    * Anyone feel like telling me the point of looping through the whole
    * room when we're only sending to one char anyways..? -- Alty 
    */
   for( ; to; to = ( type == TO_MUD ) ? to->next : ( type == TO_CHAR || type == TO_VICT ) ? NULL : to->next_in_room )

   {
      if( ( !to->desc && ( IS_NPC( to ) && !IS_SET( to->pIndexData->progtypes, ACT_PROG ) ) ) || !IS_AWAKE( to ) )
         continue;
      if( type == TO_MUD && ( to == ch || to == vch ) )
         continue;
      if( type == TO_CHAR && to != ch )
         continue;
      if( type == TO_VICT && ( to != vch || to == ch ) )
         continue;
      if( type == TO_ROOM && to == ch )
         continue;
      if( type == TO_NOTVICT && ( to == ch || to == vch ) )
         continue;

      txt = act_string( format, to, ch, arg1, arg2 );
      if( to->desc )
      {
         set_char_color( AType, to );
         send_to_char( txt, to );
      }
      if( MOBtrigger )
      {
         /*
          * Note: use original string, not string with ANSI. -- Alty 
          */
         mprog_act_trigger( txt, to, ch, ( OBJ_DATA * ) arg1, ( void * )arg2 );
      }
   }
   MOBtrigger = TRUE;
   return;
}

void do_name( CHAR_DATA * ch, const char *argument )
{
   char fname[256];
   struct stat fst;
   CHAR_DATA *tmp;
   char arg[MIL];

   if( !NOT_AUTHED( ch ) || ch->pcdata->auth_state != 2 )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   snprintf( arg, MIL, "%s", argument );
   arg[0] = UPPER( arg[0] );

   if( !check_parse_name( arg ) )
   {
      send_to_char( "Illegal name, try another.\r\n", ch );
      return;
   }

   if( !str_cmp( ch->name, arg ) )
   {
      send_to_char( "That's already your name!\r\n", ch );
      return;
   }

   for( tmp = first_char; tmp; tmp = tmp->next )
   {
      if( !str_cmp( arg, tmp->name ) )
         break;
   }

   if( tmp )
   {
      send_to_char( "That name is already taken.  Please choose another.\r\n", ch );
      return;
   }

   snprintf( fname, 256, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), capitalize( arg ) );
   if( stat( fname, &fst ) != -1 )
   {
      send_to_char( "That name is already taken.  Please choose another.\r\n", ch );
      return;
   }

   STRFREE( ch->name );
   ch->name = STRALLOC( arg );
   send_to_char( "Your name has been changed.  Please apply again.\r\n", ch );
   ch->pcdata->auth_state = 1;
   return;
}

char *hit_prompt( CHAR_DATA * ch )
{
   CHAR_DATA *victim;
   int percent;
   static char pbuf[MAX_STRING_LENGTH];

   if( ( victim = who_fighting( ch ) ) != NULL )
   {
      if( victim->max_hit > 0 )
         percent = ( 100 * victim->hit ) / victim->max_hit;
      else
         percent = -1;
      if( percent >= 100 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m++\e[0;33m++\e[1;33m++\e[1;32m++", MAX_STRING_LENGTH );
      else if( percent >= 90 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m++\e[0;33m++\e[1;33m++\e[1;32m+\e[1;30m+", MAX_STRING_LENGTH );
      else if( percent >= 80 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m++\e[0;33m++\e[1;33m++\e[1;30m++", MAX_STRING_LENGTH );
      else if( percent >= 70 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m++\e[0;33m++\e[1;33m+\e[1;30m+++", MAX_STRING_LENGTH );
      else if( percent >= 60 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m++\e[0;33m++\e[1;30m++++", MAX_STRING_LENGTH );
      else if( percent >= 50 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m++\e[0;33m+\e[1;30m+++++", MAX_STRING_LENGTH );
      else if( percent >= 40 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m++\e[1;30m++++++", MAX_STRING_LENGTH );
      else if( percent >= 30 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;31m+\e[1;30m+++++++", MAX_STRING_LENGTH );
      else if( percent >= 20 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m++\e[1;30m++++++++", MAX_STRING_LENGTH );
      else if( percent >= 10 )
         mudstrlcpy( pbuf, "\e[1;30m\e[0;31m+\e[1;30m+++++++++", MAX_STRING_LENGTH );
      else
         mudstrlcpy( pbuf, "\e[1;30m++++++++++", MAX_STRING_LENGTH );
   }
   return pbuf;
}

char *default_prompt( CHAR_DATA * ch )
{
   static char buf[MAX_STRING_LENGTH];

   mudstrlcpy( buf, "", MAX_STRING_LENGTH );
   if( ch->skill_level[FORCE_ABILITY] > 1 || get_trust( ch ) >= LEVEL_IMMORTAL )
      mudstrlcat( buf, "&pForce:&P%m/&p%M  &pAlign:&P%a\r\n", MAX_STRING_LENGTH );
//  mudstrlcat(buf, "&BHealth:&C%h&B/%H  &BMovement:&C%v&B/%V  &w%e", MAX_STRING_LENGTH );
//  mudstrlcat(buf, "&C >&w", MAX_STRING_LENGTH, );

   mudstrlcat( buf, "&G[&zHp:&w%h&G/&w%H&G] &G[&zMv:&w%v&G/&w%V&G] &G(&zAlign:&w%a&G) &w", MAX_STRING_LENGTH );
   if( ch->position == POS_FIGHTING )
      mudstrlcat( buf, "&W&G[%e&W&G] &w", MAX_STRING_LENGTH );
   return buf;
}

int getcolor( char clr )
{
   static const char colors[17] = "xrgObpcwzRGYBPCW";
   int r;

   for( r = 0; r < 16; r++ )
      if( clr == colors[r] )
         return r;
   return -1;
}

void display_prompt( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch = d->character;
   CHAR_DATA *och = ( d->original ? d->original : d->character );
   bool ansi = ( !IS_NPC( och ) && IS_SET( och->act, PLR_ANSI ) );
   const char *prompt;
   char buf[MAX_STRING_LENGTH];
   char *pbuf = buf;
   unsigned int pstat;

   if( !ch )
   {
      bug( "%s: NULL ch", __func__ );
      return;
   }

   if( !IS_NPC( ch ) && ch->substate != SUB_NONE && ch->pcdata->subprompt && ch->pcdata->subprompt[0] != '\0' )
      prompt = ch->pcdata->subprompt;
   else if( IS_NPC( ch ) || !ch->pcdata->prompt || !*ch->pcdata->prompt )
      prompt = default_prompt( ch );
   else
      prompt = ch->pcdata->prompt;

   if( ansi )
   {
      mudstrlcpy( pbuf, ANSI_RESET, MAX_STRING_LENGTH );
      d->prevcolor = 0x08;
      pbuf += 4;
   }

   for( ; *prompt; prompt++ )
   {
      /*
       * '&' = foreground color/intensity bit
       * '^' = background color/blink bit
       * '%' = prompt commands
       * Note: foreground changes will revert background to 0 (black)
       */
      if( *prompt != '%' )
      {
         *( pbuf++ ) = *prompt;
         continue;
      }
      ++prompt;
      if( !*prompt )
         break;
      if( *prompt == *( prompt - 1 ) )
      {
         *( pbuf++ ) = *prompt;
         continue;
      }
      switch ( *( prompt - 1 ) )
      {
         default:
            bug( "%s: bad command char '%c'.", __func__, *( prompt - 1 ) );
            break;
         case '%':
            *pbuf = '\0';
            pstat = 0x80000000;
            switch ( *prompt )
            {
               case '%':
                  *pbuf++ = '%';
                  *pbuf = '\0';
                  break;
               case 'a':
                  if( ch->top_level >= 10 )
                     pstat = ch->alignment;
                  else if( IS_GOOD( ch ) )
                     mudstrlcpy( pbuf, "good", MAX_STRING_LENGTH );
                  else if( IS_EVIL( ch ) )
                     mudstrlcpy( pbuf, "evil", MAX_STRING_LENGTH );
                  else
                     mudstrlcpy( pbuf, "neutral", MAX_STRING_LENGTH );
                  break;
               case 'e':
                  if( ch->position == POS_FIGHTING )
                     mudstrlcpy( pbuf, hit_prompt( ch ), MAX_STRING_LENGTH );
                  break;
               case 'h':
                  pstat = ch->hit;
                  break;
               case 'H':
                  pstat = ch->max_hit;
                  break;
               case 'm':
                  if( IS_IMMORTAL( ch ) || ch->skill_level[FORCE_ABILITY] > 1 )
                     pstat = ch->mana;
                  else
                     pstat = 0;
                  break;
               case 'M':
                  if( IS_IMMORTAL( ch ) || ch->skill_level[FORCE_ABILITY] > 1 )
                     pstat = ch->max_mana;
                  else
                     pstat = 0;
                  break;
               case 'n':
                  mudstrlcpy( pbuf, "\r\n", MAX_STRING_LENGTH );
                  break;
               case 'u':
                  pstat = num_descriptors;
                  break;
               case 'U':
                  pstat = sysdata.maxplayers;
                  break;
               case 'v':
                  pstat = ch->move;
                  break;
               case 'V':
                  pstat = ch->max_move;
                  break;
               case 'g':
                  pstat = ch->gold;
                  break;
               case 'r':
                  if( IS_IMMORTAL( och ) )
                     pstat = ch->in_room->vnum;
                  break;
               case 'R':
                  if( IS_SET( och->act, PLR_ROOMVNUM ) )
                     snprintf( pbuf, MAX_STRING_LENGTH, "<#%d> ", ch->in_room->vnum );
                  break;
               case 'i':
                  if( ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_WIZINVIS ) ) ||
                      ( IS_NPC( ch ) && IS_SET( ch->act, ACT_MOBINVIS ) ) )
                     snprintf( pbuf, MAX_STRING_LENGTH, "(Invis %d) ", ( IS_NPC( ch ) ? ch->mobinvis : ch->pcdata->wizinvis ) );
                  else if( IS_AFFECTED( ch, AFF_INVISIBLE ) )
                     mudstrlcpy( pbuf, "(Invis) ", MAX_STRING_LENGTH );
                  break;
               case 'I':
                  pstat = ( IS_NPC( ch ) ? ( IS_SET( ch->act, ACT_MOBINVIS ) ? ch->mobinvis : 0 )
                            : ( IS_SET( ch->act, PLR_WIZINVIS ) ? ch->pcdata->wizinvis : 0 ) );
                  break;
            }
            if( pstat != 0x80000000 )
               snprintf( pbuf, MAX_STRING_LENGTH, "%d", pstat );
            pbuf += strlen( pbuf );
            break;
      }
   }
   *pbuf = '\0';
   send_to_char( buf, ch );
   return;
}

void set_pager_input( DESCRIPTOR_DATA * d, char *argument )
{
   while( isspace( *argument ) )
      argument++;
   d->pagecmd = *argument;
   return;
}

bool pager_output( DESCRIPTOR_DATA * d )
{
   char *last;
   CHAR_DATA *ch;
   int pclines;
   int lines;
   bool ret;

   if( !d || !d->pagepoint || d->pagecmd == -1 )
      return TRUE;
   ch = d->original ? d->original : d->character;
   pclines = UMAX( ch->pcdata->pagerlen, 5 ) - 1;
   switch ( LOWER( d->pagecmd ) )
   {
      default:
         lines = 0;
         break;
      case 'b':
         lines = -1 - ( pclines * 2 );
         break;
      case 'r':
         lines = -1 - pclines;
         break;
      case 'q':
         d->pagetop = 0;
         d->pagepoint = NULL;
         flush_buffer( d, TRUE );
         DISPOSE( d->pagebuf );
         d->pagesize = MAX_STRING_LENGTH;
         return TRUE;
   }
   while( lines < 0 && d->pagepoint >= d->pagebuf )
      if( *( --d->pagepoint ) == '\n' )
         ++lines;
   if( *d->pagepoint == '\r' && *( ++d->pagepoint ) == '\n' )
      ++d->pagepoint;
   if( d->pagepoint < d->pagebuf )
      d->pagepoint = d->pagebuf;
   for( lines = 0, last = d->pagepoint; lines < pclines; ++last )
      if( !*last )
         break;
      else if( *last == '\n' )
         ++lines;
   if( *last == '\r' )
      ++last;
   if( last != d->pagepoint )
   {
      if( !write_to_descriptor( d, d->pagepoint, ( last - d->pagepoint ) ) )
         return FALSE;
      d->pagepoint = last;
   }
   while( isspace( *last ) )
      ++last;
   if( !*last )
   {
      d->pagetop = 0;
      d->pagepoint = NULL;
      flush_buffer( d, TRUE );
      DISPOSE( d->pagebuf );
      d->pagesize = MAX_STRING_LENGTH;
      return TRUE;
   }
   d->pagecmd = -1;
   if( IS_SET( ch->act, PLR_ANSI ) )
      if( write_to_descriptor( d, ANSI_LBLUE, 0 ) == FALSE )
         return FALSE;
   if( ( ret = write_to_descriptor( d, "(C)ontinue, (R)efresh, (B)ack, (Q)uit: [C] ", 0 ) ) == FALSE )
      return FALSE;
   if( IS_SET( ch->act, PLR_ANSI ) )
   {
      char buf[32];

      snprintf( buf, 32, "%s", color_str( d->pagecolor, ch ) );
      ret = write_to_descriptor( d, buf, 0 );
   }
   return ret;
}

void do_idealog( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[256];
   char buf2[256];

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_pager
         ( "         &B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&w&WIDEA LOG&r=-&B=-&r=-&B=-&r=-&B=-&r=-&B=-&r=-&B=-&r=-&B=-&r=-&w&W\r\n",
           ch );
      show_file( ch, IDEA_FILE );
      return;
   }

   if( !str_cmp( arg, "clear" ) )
   {
      snprintf( buf, 256, "%sideas.txt", SYSTEM_DIR );
      snprintf( buf2, 256, "%sideas.bak", SYSTEM_DIR );
      rename( buf, buf2 );
      send_to_char( "Idealog removed, saved as ideas.bak\r\n", ch );
      return;
   }
   else
   {
      send_to_char
         ( "         &B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&B-=&r-=&WIDEA LOG&r=-&B=-&r=-&B=-&r=-&B=-&r=-&B=-&r=-&B=-&r=-&B=-&r=-\r\n",
           ch );
      show_file( ch, IDEA_FILE );
   }
   return;

}

void do_giveslug( CHAR_DATA * ch, const char *argument )
{
/*   SHIP_PROTOTYPE * prototype;
   CHAR_DATA	  * victim;

   if(argument[0] == '\0')
   {
	send_to_char("Give who a slug?\r\n", ch);
	return;
   }

   if ( (victim = get_char_world(ch, argument)) == NULL)
   {
	send_to_char("They aren't here.\r\n", ch);
	return;
   }

   if (IS_NPC(victim))
   {
	send_to_char("Only players can have slugs given to them.\r\n", ch);
	return;
   }

   prototype = get_ship_prototype( "Slug" );
            
            if ( prototype )
            {
              SHIP_DATA *ship;
              char shipname[MAX_STRING_LENGTH];
              ship = make_ship( prototype );
              ship_to_room( ship, 178 );
              ship->location = 178;
              ship->lastdoc = 178;
              snprintf( shipname, MAX_STRING_LENGTH, "%s's %s %s" , victim->name , prototype->name , ship->filename );
              STRFREE( ship->owner );
              ship->owner = STRALLOC( victim->name );
              STRFREE( ship->name );
              ship->name = STRALLOC( shipname );
	      ship->password = number_range(1111,9999);
	      ship->shipyard = 1050;
              save_ship( ship );
              write_ship_list();
            }
 
    send_to_char("&w&GA slug has been created for you at the newbie docking bay.\r\n", victim);
    send_to_char("Done.\r\n", ch);
*/
}

FELLOW_DATA *knowsof( CHAR_DATA * ch, CHAR_DATA * victim )
{
   FELLOW_DATA *fellow;

   for( fellow = ch->first_fellow; fellow; fellow = fellow->next )
   {
      if( fellow->victim == victim->name )
         return fellow;
   }

   return NULL;
}

const char *PERS( CHAR_DATA * ch, CHAR_DATA * looker )
{
   static char buf[MAX_STRING_LENGTH];
   char race[MAX_STRING_LENGTH-30];
   FELLOW_DATA *fellow;

   if( can_see( looker, ch ) )
   {
      if( IS_NPC( ch ) )
         return ch->short_descr;
      else
      {
         if( IS_IMMORTAL( looker ) || ch == looker )
            return ch->name;
         else if( ch->pcdata->disguise && ch->pcdata->disguise[0] != '\0' )
            return ch->pcdata->disguise;
         else if( ( fellow = knowsof( looker, ch ) ) != NULL )
            return fellow->knownas;
         else
         {
            if( IS_IMMORTAL( ch ) )
               return ch->name;
            snprintf( race, MAX_STRING_LENGTH-30, "%s", npc_race[ch->race] );
            race[0] = tolower( race[0] );
            if( !IS_DROID( ch ) )
               snprintf( buf, MAX_STRING_LENGTH, "%s %s %s of %s height", aoran( build_name[ch->build] ), race,
                        ch->sex == 1 ? "male" : ch->sex == 2 ? "female" : "neutral", height_name[ch->pheight] );
            else
               snprintf( buf, MAX_STRING_LENGTH, "%s %s", aoran( droid_name[ch->build] ), race );
            return buf;
         }
      }
   }
   else
   {
      if( IS_IMMORTAL( ch ) )
         return "Immortal";
      else
         return "someone";
   }
}
