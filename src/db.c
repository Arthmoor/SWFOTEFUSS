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
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <dlfcn.h>
#include "mud.h"
#include "mssp.h"

void init_supermob( void );

/*
 * Globals.
 */
WIZENT *first_wiz;
WIZENT *last_wiz;

time_t last_restore_all_time = 0;

HELP_DATA *first_help;
HELP_DATA *last_help;

SHOP_DATA *first_shop;
SHOP_DATA *last_shop;

REPAIR_DATA *first_repair;
REPAIR_DATA *last_repair;

TELEPORT_DATA *first_teleport;
TELEPORT_DATA *last_teleport;

extern BMARKET_DATA *first_market_ship;
extern BMARKET_DATA *last_market_ship;

OBJ_DATA *extracted_obj_queue;
EXTRACT_CHAR_DATA *extracted_char_queue;

CHAR_DATA *first_char;
CHAR_DATA *last_char;
char *help_greeting;
char log_buf[MAX_STRING_LENGTH];

OBJ_DATA *first_object;
OBJ_DATA *last_object;
TIME_INFO_DATA time_info;
WEATHER_DATA weather_info;

int cur_qobjs;
int cur_qchars;
int nummobsloaded;
int numobjsloaded;
int physicalobjects;
time_t mud_start_time;

AUCTION_DATA *auction;  /* auctions */
OBJ_DATA *supermob_obj;

/* criminals */
short gsn_torture;
short gsn_deception;
short gsn_disguise;
short gsn_beg;
short gsn_pickshiplock;
short gsn_hijack;

short gsn_plantbug;
short gsn_showbugs;
short gsn_silent;
short gsn_retreat;

/* Technicians */
short gsn_makemodule;
short gsn_installmodule;
short gsn_showmodules;
short gsn_removemodule;
short gsn_removebug;
short gsn_shipmaintenance;
short gsn_scanbugs;

/* slicers */
short gsn_spy;
short gsn_makecommsystem;
short gsn_commsystem;
short gsn_codecrack;
short gsn_slicebank;
short gsn_inquire;
short gsn_checkprints;
short gsn_makedatapad;
short gsn_disable;
short gsn_assignpilot;

/* soldiers and officers */
short gsn_battle_command;
short gsn_reinforcements;
short gsn_postguard;
short gsn_mine;
short gsn_first_aid;
short gsn_snipe;
short gsn_throw;

short gsn_addpatrol;
short gsn_eliteguard;
short gsn_specialforces;
short gsn_jail;
short gsn_smalltalk;
short gsn_propeganda;
short gsn_bribe;
short gsn_seduce;
short gsn_masspropeganda;
short gsn_gather_intelligence;

/* pilots and smugglers */
short gsn_smallspace;
short gsn_mediumspace;
short gsn_largespace;
short gsn_weaponsystems;
short gsn_navigation;
short gsn_shipsystems;
short gsn_tractorbeams;
short gsn_shipdesign;
short gsn_spacecombat;
short gsn_spacecombat2;
short gsn_spacecombat3;
short gsn_bomb;

short gsn_truesight;
/* player building skills */
short gsn_makelightsaber;
short gsn_makeduallightsaber;
short gsn_spice_refining;
short gsn_makeblade;
short gsn_makepike;
short gsn_sabotage;
short gsn_makeblaster;
short gsn_makelight;
short gsn_makecomlink;
short gsn_makegrenade;
short gsn_makeshipbomb;
short gsn_makelandmine;
short gsn_makearmor;
short gsn_makeshield;
short gsn_makecontainer;
short gsn_makemissile;
short gsn_gemcutting;
short gsn_makejewelry;
short gsn_repair;
short gsn_shiprepair;
short gsn_makebeacon;
short gsn_makebug;
short gsn_plantbeacon;
short gsn_showbeacons;
short gsn_checkbeacons;
short gsn_nullifybeacons;
short gsn_makebinders;
short gsn_makeempgrenade;
short gsn_makegoggles;
short gsn_barrelroll;
short gsn_juke;

/* weaponry */
short gsn_blasters;
short gsn_bowcasters;
short gsn_force_pikes;
short gsn_lightsabers;
short gsn_vibro_blades;
short gsn_flexible_arms;
short gsn_talonous_arms;
short gsn_bludgeons;
short gsn_launchers;

/* Hunter */
short gsn_ambush;
short gsn_bind;
short gsn_gag;

/* thief */
short gsn_detrap;
short gsn_bargain;
short gsn_backstab;
short gsn_dualstab;
short gsn_circle;
short gsn_dodge;
short gsn_concealment;
short gsn_hide;
short gsn_peek;
short gsn_pick_lock;
short gsn_sneak;
short gsn_steal;
short gsn_gouge;
short gsn_poison_weapon;

/* thief & warrior */
short gsn_disarm;
short gsn_enhanced_damage;
short gsn_kick;
short gsn_parry;
short gsn_reflect;
short gsn_rescue;
short gsn_second_attack;
short gsn_third_attack;
short gsn_dual_wield;
short gsn_punch;
short gsn_bash;
short gsn_stun;
short gsn_bashdoor;
short gsn_grip;
short gsn_berserk;
short gsn_hitall;

/* other   */
short gsn_aid;
short gsn_track;
short gsn_search;
short gsn_dig;
short gsn_mount;
short gsn_scribe;
short gsn_brew;
short gsn_climb;
short gsn_scan;
short gsn_slice;

/* spells */
short gsn_aqua_breath;
short gsn_blindness;
short gsn_charm_person;
short gsn_curse;
short gsn_invis;
short gsn_mass_invis;
short gsn_poison;
short gsn_sleep;
short gsn_fireball;
short gsn_lightning_bolt;
short gsn_possess;

/* languages */
short gsn_basic;
short gsn_wookiee;
short gsn_twilek;
short gsn_rodian;
short gsn_hutt;
short gsn_mon_calamari;
short gsn_noghri;
short gsn_ewok;
short gsn_ithorian;
short gsn_gotal;
short gsn_devaronian;
short gsn_binary;
short gsn_firrerreo;
short gsn_gamorrean;
short gsn_togorian;
short gsn_shistavanen;
short gsn_jawa;
short gsn_kubaz;
short gsn_verpine;
short gsn_defel;
short gsn_trandoshan;
short gsn_hapan;
short gsn_quarren;
short gsn_sullustan;
short gsn_falleen;
short gsn_barabel;
short gsn_yevethan;
short gsn_gand;
short gsn_coynite;
short gsn_duinuogwuin;

/* for searching */
short gsn_first_spell;
short gsn_first_skill;
short gsn_first_weapon;
short gsn_first_tongue;
short gsn_top_sn;

/*
 * Locals.
 */
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

AREA_DATA *first_area;
AREA_DATA *last_area;
AREA_DATA *first_area_name;   /*Used for alphanum. sort */
AREA_DATA *last_area_name;
AREA_DATA *first_build;
AREA_DATA *last_build;
AREA_DATA *first_asort;
AREA_DATA *last_asort;
AREA_DATA *first_bsort;
AREA_DATA *last_bsort;

SYSTEM_DATA sysdata;

int top_affect;
int top_area;
int top_ed;
int top_exit;
int top_help;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_shop;
int top_repair;
int top_vroom;

/*
 * Semi-locals.
 */
bool fBootDb;
bool DONT_UPPER;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];
char bname[MAX_STRING_LENGTH];


/*
 * Local booting procedures.
 */
void init_mm( void );
void load_specfuns( void );
void boot_log( const char *str, ... );
AREA_DATA *load_area( FILE * fp, int aversion );
void load_author( AREA_DATA * tarea, FILE * fp );
void load_economy( AREA_DATA * tarea, FILE * fp );
void load_resetmsg( AREA_DATA * tarea, FILE * fp );   /* Rennard */
void load_flags( AREA_DATA * tarea, FILE * fp );
void load_helps( FILE * fp );
void load_mobiles( AREA_DATA * tarea, FILE * fp );
void load_objects( AREA_DATA * tarea, FILE * fp );
void load_resets( AREA_DATA * tarea, FILE * fp );
void load_rooms( AREA_DATA * tarea, FILE * fp );
void load_shops( FILE * fp );
void load_repairs( FILE * fp );
void load_specials( FILE * fp );
void load_ranges( AREA_DATA * tarea, FILE * fp );
void load_buildlist( void );
bool load_systemdata( SYSTEM_DATA * sys );
void load_banlist( void );
void initialize_economy( void );

void fix_exits( void );

/*
 * External booting function
 */
void load_corpses( void );
void renumber_put_resets( ROOM_INDEX_DATA * room );
void wipe_resets( ROOM_INDEX_DATA * room );
void mprog_read_programs( FILE * fp, MOB_INDEX_DATA * pMobIndex );
void oprog_read_programs( FILE * fp, OBJ_INDEX_DATA * pObjIndex );
void rprog_read_programs( FILE * fp, ROOM_INDEX_DATA * pRoomIndex );

void shutdown_mud( const char *reason )
{
   FILE *fp;

   if( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
   {
      fprintf( fp, "%s\n", reason );
      FCLOSE( fp );
   }
}

/*
 * Big mama top level function.
 */
void boot_db( bool fCopyOver )
{
   short wear, x;

   show_hash( 32 );
   unlink( BOOTLOG_FILE );
   boot_log( "---------------------[ Boot Log ]--------------------" );

   log_string( "Initializing libdl support..." );
   /*
    * Open up a handle to the executable's symbol table for later use
    * when working with commands
    */
   sysdata.dlHandle = dlopen( NULL, RTLD_LAZY );
   if( !sysdata.dlHandle )
   {
      log_string( "dl: Error opening local system executable as handle, please check compile flags." );
      shutdown_mud( "libdl failure" );
      exit( 1 );
   }

   log_string( "Loading commands..." );
   load_commands(  );

   mud_start_time = current_time;

   log_string( "Loading spec_funs..." );
   load_specfuns(  );

   log_string( "Loading sysdata configuration..." );

   /*
    * default values 
    */
   sysdata.read_all_mail = LEVEL_DEMI;
   sysdata.read_mail_free = LEVEL_IMMORTAL;
   sysdata.write_mail_free = LEVEL_IMMORTAL;
   sysdata.take_others_mail = LEVEL_DEMI;
   sysdata.muse_level = LEVEL_DEMI;
   sysdata.think_level = LEVEL_HIGOD;
   sysdata.build_level = LEVEL_DEMI;
   sysdata.log_level = LEVEL_LOG;
   sysdata.level_modify_proto = LEVEL_LESSER;
   sysdata.level_override_private = LEVEL_GREATER;
   sysdata.level_mset_player = LEVEL_LESSER;
   sysdata.stun_plr_vs_plr = 15;
   sysdata.stun_regular = 15;
   sysdata.dam_plr_vs_plr = 100;
   sysdata.dam_plr_vs_mob = 100;
   sysdata.dam_mob_vs_plr = 100;
   sysdata.dam_mob_vs_mob = 100;
   sysdata.level_getobjnotake = LEVEL_GREATER;
   sysdata.save_frequency = 20;  /* minutes */
   sysdata.save_flags = SV_DEATH | SV_PASSCHG | SV_AUTO | SV_PUT | SV_DROP | SV_GIVE | SV_AUCTION | SV_ZAPDROP | SV_IDLE;
   if( !load_systemdata( &sysdata ) )
   {
      log_string( "Not found.  Creating new configuration." );
      sysdata.alltimemax = 0;
   }

   log_string( "Loading socials" );
   load_socials(  );

   log_string( "Loading skill table" );
   load_skill_table(  );
   sort_skill_table(  );

   gsn_first_spell = 0;
   gsn_first_skill = 0;
   gsn_first_weapon = 0;
   gsn_first_tongue = 0;
   gsn_top_sn = top_sn;

   for( x = 0; x < top_sn; x++ )
      if( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
         gsn_first_spell = x;
      else if( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
         gsn_first_skill = x;
      else if( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
         gsn_first_weapon = x;
      else if( !gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE )
         gsn_first_tongue = x;

   log_string( "Loading herb table" );
   load_herb_table(  );

   log_string( "Making wizlist" );
   make_wizlist(  );

   log_string( "Loading MSSP Data..." );
   load_mssp_data( );

   log_string( "Loading force" );
   load_force_skills(  );
   load_force_help(  );

   load_ship_prototypes(  );

   fBootDb = TRUE;

   nummobsloaded = 0;
   numobjsloaded = 0;
   physicalobjects = 0;
   sysdata.maxplayers = 0;
   first_object = NULL;
   last_object = NULL;
   first_char = NULL;
   last_char = NULL;
   first_area = NULL;
   last_area = NULL;
   first_area_name = NULL;
   last_area_name = NULL;
   first_build = NULL;
   last_area = NULL;
   first_shop = NULL;
   last_shop = NULL;
   first_repair = NULL;
   last_repair = NULL;
   first_teleport = NULL;
   last_teleport = NULL;
   first_asort = NULL;
   last_asort = NULL;
   extracted_obj_queue = NULL;
   extracted_char_queue = NULL;
   cur_qobjs = 0;
   cur_qchars = 0;
   cur_char = NULL;
   cur_obj = 0;
   cur_obj_serial = 0;
   cur_char_died = FALSE;
   cur_obj_extracted = FALSE;
   cur_room = NULL;
   quitting_char = NULL;
   loading_char = NULL;
   saving_char = NULL;
   CREATE( auction, AUCTION_DATA, 1 );
   auction->item = NULL;
   for( wear = 0; wear < MAX_WEAR; wear++ )
      for( x = 0; x < MAX_LAYERS; x++ )
         save_equipment[wear][x] = NULL;

   /*
    * Init random number generator.
    */
   log_string( "Initializing random number generator" );
   init_mm(  );

   /*
    * Set time and weather.
    */
   {
      long lhour, lday, lmonth;

      log_string( "Setting time and weather" );

      lhour = ( current_time - 650336715 ) / ( PULSE_TICK / PULSE_PER_SECOND );
      time_info.hour = lhour % 24;
      lday = lhour / 24;
      time_info.day = lday % 35;
      lmonth = lday / 35;
      time_info.month = lmonth % 17;
      time_info.year = lmonth / 17;

      if( time_info.hour < 5 )
         weather_info.sunlight = SUN_DARK;
      else if( time_info.hour < 6 )
         weather_info.sunlight = SUN_RISE;
      else if( time_info.hour < 19 )
         weather_info.sunlight = SUN_LIGHT;
      else if( time_info.hour < 20 )
         weather_info.sunlight = SUN_SET;
      else
         weather_info.sunlight = SUN_DARK;

      weather_info.change = 0;
      weather_info.mmhg = 960;
      if( time_info.month >= 7 && time_info.month <= 12 )
         weather_info.mmhg += number_range( 1, 50 );
      else
         weather_info.mmhg += number_range( 1, 80 );

      if( weather_info.mmhg <= 980 )
         weather_info.sky = SKY_LIGHTNING;
      else if( weather_info.mmhg <= 1000 )
         weather_info.sky = SKY_RAINING;
      else if( weather_info.mmhg <= 1020 )
         weather_info.sky = SKY_CLOUDY;
      else
         weather_info.sky = SKY_CLOUDLESS;

   }

   log_string( "Loading DNS cache..." ); /* Samson 1-30-02 */
   load_dns(  );

   /*
    * Assign gsn's for skills which need them.
    */
   {
      log_string( "Assigning gsn's" );
      ASSIGN_GSN( gsn_eliteguard, "elite_guard" );
      ASSIGN_GSN( gsn_addpatrol, "add_patrol" );
      ASSIGN_GSN( gsn_gather_intelligence, "gather_intelligence" );
      ASSIGN_GSN( gsn_specialforces, "special_forces" );
      ASSIGN_GSN( gsn_jail, "jail" );
      ASSIGN_GSN( gsn_smalltalk, "smalltalk" );
      ASSIGN_GSN( gsn_propeganda, "propeganda" );
      ASSIGN_GSN( gsn_possess, "possess" );
      ASSIGN_GSN( gsn_bribe, "bribe" );
      ASSIGN_GSN( gsn_seduce, "seduce" );
      ASSIGN_GSN( gsn_masspropeganda, "mass_propeganda" );
      ASSIGN_GSN( gsn_beg, "beg" );
      ASSIGN_GSN( gsn_hijack, "hijack" );
      ASSIGN_GSN( gsn_makejewelry, "makejewelry" );
      ASSIGN_GSN( gsn_makeblade, "makeblade" );
      ASSIGN_GSN( gsn_sabotage, "sabotage" );
      ASSIGN_GSN( gsn_makeblaster, "makeblaster" );
      ASSIGN_GSN( gsn_makelight, "makeflashlight" );
      ASSIGN_GSN( gsn_makecomlink, "makecomlink" );
      ASSIGN_GSN( gsn_makegrenade, "makegrenade" );
      ASSIGN_GSN( gsn_makeshipbomb, "makeshipbomb" );
      ASSIGN_GSN( gsn_makelandmine, "makelandmine" );
      ASSIGN_GSN( gsn_makearmor, "makearmor" );
      ASSIGN_GSN( gsn_makepike, "makepike" );
      ASSIGN_GSN( gsn_makeshield, "makeshield" );
      ASSIGN_GSN( gsn_makecontainer, "makecontainer" );
      ASSIGN_GSN( gsn_makemissile, "makemissile" );
      ASSIGN_GSN( gsn_gemcutting, "gemcutting" );
      ASSIGN_GSN( gsn_battle_command, "battle_command" );
      ASSIGN_GSN( gsn_reinforcements, "reinforcements" );
      ASSIGN_GSN( gsn_postguard, "post guard" );
      ASSIGN_GSN( gsn_smallspace, "small spacecraft" );
      ASSIGN_GSN( gsn_mediumspace, "medium spacecraft" );
      ASSIGN_GSN( gsn_largespace, "large spacecraft" );
      ASSIGN_GSN( gsn_torture, "torture" );
      ASSIGN_GSN( gsn_throw, "throw" );
      ASSIGN_GSN( gsn_snipe, "snipe" );
      ASSIGN_GSN( gsn_deception, "deception" );
      ASSIGN_GSN( gsn_disguise, "disguise" );
      ASSIGN_GSN( gsn_mine, "mine" );
      ASSIGN_GSN( gsn_plantbug, "plantbug" );
      ASSIGN_GSN( gsn_showbugs, "showbug" );
      ASSIGN_GSN( gsn_truesight, "true sight" );
      ASSIGN_GSN( gsn_makemodule, "makemodule" );
      ASSIGN_GSN( gsn_installmodule, "installmodule" );
      ASSIGN_GSN( gsn_showmodules, "showmodules" );
      ASSIGN_GSN( gsn_removemodule, "removemodule" );
      ASSIGN_GSN( gsn_removebug, "removebug" );
      ASSIGN_GSN( gsn_spy, "spy" );
      ASSIGN_GSN( gsn_makecommsystem, "makecommsystem" );
      ASSIGN_GSN( gsn_commsystem, "commsystem" );
      ASSIGN_GSN( gsn_codecrack, "codecrack" );
      ASSIGN_GSN( gsn_slicebank, "slicebank" );
      ASSIGN_GSN( gsn_inquire, "inquire" );
      ASSIGN_GSN( gsn_checkprints, "checkprints" );
      ASSIGN_GSN( gsn_makedatapad, "makedatapad" );
      ASSIGN_GSN( gsn_disable, "disable" );
      ASSIGN_GSN( gsn_assignpilot, "assignpilot" );
      ASSIGN_GSN( gsn_first_aid, "first aid" );
      ASSIGN_GSN( gsn_makelightsaber, "makelightsaber" );
      ASSIGN_GSN( gsn_makeduallightsaber, "makeduallightsaber" );
      ASSIGN_GSN( gsn_spice_refining, "spice refining" );
      ASSIGN_GSN( gsn_spacecombat, "space combat 1" );
      ASSIGN_GSN( gsn_spacecombat2, "space combat 2" );
      ASSIGN_GSN( gsn_spacecombat3, "space combat 3" );
      ASSIGN_GSN( gsn_weaponsystems, "weapon systems" );
      ASSIGN_GSN( gsn_bomb, "bomb" );
      ASSIGN_GSN( gsn_navigation, "navigation" );
      ASSIGN_GSN( gsn_shipsystems, "ship systems" );
      ASSIGN_GSN( gsn_tractorbeams, "tractor beams" );
      ASSIGN_GSN( gsn_shipmaintenance, "shipmaintenance" );
      ASSIGN_GSN( gsn_scanbugs, "checkbugs" );
      ASSIGN_GSN( gsn_shipdesign, "ship design" );
      ASSIGN_GSN( gsn_blasters, "blasters" );
      ASSIGN_GSN( gsn_bowcasters, "bowcasters" );
      ASSIGN_GSN( gsn_force_pikes, "force pikes" );
      ASSIGN_GSN( gsn_lightsabers, "lightsabers" );
      ASSIGN_GSN( gsn_vibro_blades, "vibro-blades" );
      ASSIGN_GSN( gsn_flexible_arms, "flexible arms" );
      ASSIGN_GSN( gsn_talonous_arms, "talonous arms" );
      ASSIGN_GSN( gsn_bludgeons, "bludgeons" );
      ASSIGN_GSN( gsn_detrap, "detrap" );
      ASSIGN_GSN( gsn_bargain, "bargain" );
      ASSIGN_GSN( gsn_backstab, "backstab" );
      ASSIGN_GSN( gsn_dualstab, "dualstab" );
      ASSIGN_GSN( gsn_ambush, "ambush" );
      ASSIGN_GSN( gsn_bind, "bind" );
      ASSIGN_GSN( gsn_gag, "gag" );
      ASSIGN_GSN( gsn_circle, "circle" );
      ASSIGN_GSN( gsn_dodge, "dodge" );
      ASSIGN_GSN( gsn_concealment, "concealment" );
      ASSIGN_GSN( gsn_hide, "hide" );
      ASSIGN_GSN( gsn_peek, "peek" );
      ASSIGN_GSN( gsn_pick_lock, "pick lock" );
      ASSIGN_GSN( gsn_pickshiplock, "pick ship lock" );
      ASSIGN_GSN( gsn_retreat, "retreat" );
      ASSIGN_GSN( gsn_silent, "silent" );
      ASSIGN_GSN( gsn_sneak, "sneak" );
      ASSIGN_GSN( gsn_steal, "steal" );
      ASSIGN_GSN( gsn_gouge, "gouge" );
      ASSIGN_GSN( gsn_poison_weapon, "poison weapon" );
      ASSIGN_GSN( gsn_disarm, "disarm" );
      ASSIGN_GSN( gsn_enhanced_damage, "enhanced damage" );
      ASSIGN_GSN( gsn_kick, "kick" );
      ASSIGN_GSN( gsn_parry, "parry" );
      ASSIGN_GSN( gsn_reflect, "reflect" );
      ASSIGN_GSN( gsn_rescue, "rescue" );
      ASSIGN_GSN( gsn_repair, "repair" );
      ASSIGN_GSN( gsn_shiprepair, "shiprepair" );
      ASSIGN_GSN( gsn_second_attack, "second attack" );
      ASSIGN_GSN( gsn_third_attack, "third attack" );
      ASSIGN_GSN( gsn_dual_wield, "dual wield" );
      ASSIGN_GSN( gsn_punch, "punch" );
      ASSIGN_GSN( gsn_bash, "bash" );
      ASSIGN_GSN( gsn_stun, "stun" );
      ASSIGN_GSN( gsn_bashdoor, "doorbash" );
      ASSIGN_GSN( gsn_grip, "grip" );
      ASSIGN_GSN( gsn_berserk, "berserk" );
      ASSIGN_GSN( gsn_hitall, "hitall" );
      ASSIGN_GSN( gsn_aid, "aid" );
      ASSIGN_GSN( gsn_track, "track" );
      ASSIGN_GSN( gsn_search, "search" );
      ASSIGN_GSN( gsn_dig, "dig" );
      ASSIGN_GSN( gsn_mount, "mount" );
      ASSIGN_GSN( gsn_scribe, "scribe" );
      ASSIGN_GSN( gsn_brew, "brew" );
      ASSIGN_GSN( gsn_climb, "climb" );
      ASSIGN_GSN( gsn_scan, "scan" );
      ASSIGN_GSN( gsn_slice, "slice" );
      ASSIGN_GSN( gsn_fireball, "fireball" );
      ASSIGN_GSN( gsn_lightning_bolt, "force bolt" );
      ASSIGN_GSN( gsn_aqua_breath, "aqua breath" );
      ASSIGN_GSN( gsn_blindness, "blindness" );
      ASSIGN_GSN( gsn_charm_person, "affect mind" );
      ASSIGN_GSN( gsn_invis, "mask" );
      ASSIGN_GSN( gsn_mass_invis, "group masking" );
      ASSIGN_GSN( gsn_poison, "poison" );
      ASSIGN_GSN( gsn_sleep, "sleep" );
      ASSIGN_GSN( gsn_basic, "basic" );
      ASSIGN_GSN( gsn_wookiee, "wookiee" );
      ASSIGN_GSN( gsn_twilek, "twilek" );
      ASSIGN_GSN( gsn_rodian, "rodian" );
      ASSIGN_GSN( gsn_hutt, "hutt" );
      ASSIGN_GSN( gsn_mon_calamari, "mon calamari" );
      ASSIGN_GSN( gsn_noghri, "noghri" );
      ASSIGN_GSN( gsn_ewok, "ewok" );
      ASSIGN_GSN( gsn_ithorian, "ithorian" );
      ASSIGN_GSN( gsn_gotal, "gotal" );
      ASSIGN_GSN( gsn_devaronian, "devaronian" );
      ASSIGN_GSN( gsn_binary, "binary" );
      ASSIGN_GSN( gsn_firrerreo, "firrerreo" );
      ASSIGN_GSN( gsn_gamorrean, "gamorrean" );
      ASSIGN_GSN( gsn_togorian, "togorian" );
      ASSIGN_GSN( gsn_shistavanen, "shistavanen" );
      ASSIGN_GSN( gsn_jawa, "jawa" );
      ASSIGN_GSN( gsn_kubaz, "kubaz" );
      ASSIGN_GSN( gsn_verpine, "verpine" );
      ASSIGN_GSN( gsn_defel, "defel" );
      ASSIGN_GSN( gsn_trandoshan, "trandoshan" );
      ASSIGN_GSN( gsn_hapan, "hapan" );
      ASSIGN_GSN( gsn_quarren, "quarren" );
      ASSIGN_GSN( gsn_sullustan, "sullustan" );
      ASSIGN_GSN( gsn_falleen, "falleen" );
      ASSIGN_GSN( gsn_barabel, "barabel" );
      ASSIGN_GSN( gsn_yevethan, "yevethan" );
      ASSIGN_GSN( gsn_gand, "gand" );
      ASSIGN_GSN( gsn_coynite, "coynite" );
      ASSIGN_GSN( gsn_duinuogwuin, "duinuogwuin" );
      ASSIGN_GSN( gsn_makebeacon, "makebeacon" );
      ASSIGN_GSN( gsn_makebug, "makebug" );
      ASSIGN_GSN( gsn_plantbeacon, "plantbeacon" );
      ASSIGN_GSN( gsn_showbeacons, "showbeacons" );
      ASSIGN_GSN( gsn_checkbeacons, "checkbeacons" );
      ASSIGN_GSN( gsn_nullifybeacons, "nullifybeacons" );
      ASSIGN_GSN( gsn_makebinders, "makebinders" );
      ASSIGN_GSN( gsn_launchers, "launchers" );
      ASSIGN_GSN( gsn_makemissile, "makemissile" );
      ASSIGN_GSN( gsn_makegoggles, "makegoggles" );
      ASSIGN_GSN( gsn_barrelroll, "barrel roll" );
      ASSIGN_GSN( gsn_juke, "juke" );

   }

   /*
    * Read in all the area files.
    */
   {
      FILE *fpList;

      log_string( "Reading in area files..." );
      if( !( fpList = fopen( AREA_LIST, "r" ) ) )
      {
         perror( AREA_LIST );
         shutdown_mud( "Unable to open area list" );
         exit( 1 );
      }

      for( ;; )
      {
         if( feof( fpList ) )
         {
            bug( "%s: EOF encountered reading area list - no $ found at end of file.", __func__ );
            break;
         }
         strlcpy( strArea, fread_word( fpList ), MAX_INPUT_LENGTH );
         if( strArea[0] == '$' )
            break;

         load_area_file( NULL, strArea );
      }
      FCLOSE( fpList );
   }

   /*
    *   initialize supermob.
    *    must be done before reset_area!
    *
    */
   init_supermob(  );


   /*
    * Fix up exits.
    * Declare db booting over.
    * Reset all areas once.
    * Load up the notes file.
    */
   {
      log_string( "Fixing exits" );
      fix_exits(  );
      fBootDb = FALSE;
      log_string( "Initializing economy" );
      initialize_economy(  );
      log_string( "Loading buildlist" );
      load_buildlist(  );
      log_string( "Loading boards" );
      load_boards(  );
      log_string( "Loading clans" );
      load_clans(  );
      log_string( "Loading senate" );
      load_senate(  );
      log_string( "Loading bans" );
      load_banlist(  );
      log_string( "Loading corpses" );
      load_corpses(  );
      log_string( "Loading space" );
      load_space(  );
//        log_string( "Loading ship prototypes" );
//        load_prototypes( );        
      log_string( "Loading ships" );
      load_ships(  );
//        log_string( "Place ships" );
//        placeships( );
      log_string( "Loading bounties" );
      load_bounties(  );
      log_string( "Loading governments" );
      load_planets(  );
      if( fCopyOver )
      {
         log_string( "Loading world state..." );
         load_world( supermob );
      }
      log_string( "Resetting areas" );
      area_update(  );

      //log_string( "Loading black market ships");
      //load_market_list();

      log_string( "Making new blackmarket list." );
      make_random_marketlist(  );

      MOBtrigger = TRUE;
   }
   return;
}

/*
 * Load an 'area' header line.
 */
AREA_DATA *load_area( FILE * fp, int aversion )
{
   AREA_DATA *pArea;

   CREATE( pArea, AREA_DATA, 1 );
   pArea->version = aversion;
   pArea->first_room = pArea->last_room = NULL;
   pArea->name = fread_string_nohash( fp );
   pArea->author = STRALLOC( "unknown" );
   pArea->filename = strdup( strArea );
   pArea->age = 15;
   pArea->nplayer = 0;
   pArea->low_r_vnum = 0;
   pArea->low_o_vnum = 0;
   pArea->low_m_vnum = 0;
   pArea->hi_r_vnum = 0;
   pArea->hi_o_vnum = 0;
   pArea->hi_m_vnum = 0;
   pArea->low_soft_range = 0;
   pArea->hi_soft_range = MAX_LEVEL;
   pArea->low_hard_range = 0;
   pArea->hi_hard_range = MAX_LEVEL;

   LINK( pArea, first_area, last_area, next, prev );
   top_area++;
   return pArea;
}

/* Load the version number of the area file if none exists, then it
 * is set to version 0 when #AREA is read in which is why we check for
 * the #AREA here.  --Shaddai
 */
void load_version( AREA_DATA * tarea, FILE * fp )
{
   tarea->version = fread_number( fp );
}

/*
 * Load an author section. Scryn 2/1/96
 */
void load_author( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   if( tarea->author )
      STRFREE( tarea->author );
   tarea->author = fread_string( fp );
   return;
}

/*
 * Load an economy section. Thoric
 */
void load_economy( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   tarea->high_economy = fread_number( fp );
   tarea->low_economy = fread_number( fp );
   return;
}

/* Reset Message Load, Rennard */
void load_resetmsg( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   if( tarea->resetmsg )
      DISPOSE( tarea->resetmsg );
   tarea->resetmsg = fread_string_nohash( fp );
   return;
}

/*
 * Load area flags. Narn, Mar/96 
 */
void load_flags( AREA_DATA * tarea, FILE * fp )
{
   char *ln;
   int x1, x2;

   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }
   ln = fread_line( fp );
   x1 = x2 = 0;
   sscanf( ln, "%d %d", &x1, &x2 );
   tarea->flags = x1;
   tarea->reset_frequency = x2;
   if( x2 )
      tarea->age = x2;
   return;
}

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help( HELP_DATA * pHelp )
{
   HELP_DATA *tHelp;
   int match;

   for( tHelp = first_help; tHelp; tHelp = tHelp->next )
      if( pHelp->level == tHelp->level && strcmp( pHelp->keyword, tHelp->keyword ) == 0 )
      {
         bug( "%s: duplicate: %s.  Deleting.", __func__, pHelp->keyword );
         STRFREE( pHelp->text );
         STRFREE( pHelp->keyword );
         DISPOSE( pHelp );
         return;
      }
      else
         if( ( match = strcmp( pHelp->keyword[0] == '\'' ? pHelp->keyword + 1 : pHelp->keyword,
                               tHelp->keyword[0] == '\'' ? tHelp->keyword + 1 : tHelp->keyword ) ) < 0
             || ( match == 0 && pHelp->level > tHelp->level ) )
      {
         if( !tHelp->prev )
            first_help = pHelp;
         else
            tHelp->prev->next = pHelp;
         pHelp->prev = tHelp->prev;
         pHelp->next = tHelp;
         tHelp->prev = pHelp;
         break;
      }

   if( !tHelp )
      LINK( pHelp, first_help, last_help, next, prev );

   top_help++;
}

/*
 * Load a help section.
 */
void load_helps( FILE * fp )
{
   HELP_DATA *pHelp;

   for( ;; )
   {
      CREATE( pHelp, HELP_DATA, 1 );
      pHelp->level = fread_number( fp );
      pHelp->keyword = fread_string( fp );
      if( pHelp->keyword[0] == '$' )
      {
         STRFREE( pHelp->keyword );
         DISPOSE( pHelp );
         break;
      }
      pHelp->text = fread_string( fp );
      if( pHelp->keyword[0] == '\0' )
      {
         STRFREE( pHelp->text );
         STRFREE( pHelp->keyword );
         DISPOSE( pHelp );
         continue;
      }

      if( !str_cmp( pHelp->keyword, "greeting" ) )
         help_greeting = pHelp->text;
      add_help( pHelp );
   }
   return;
}


/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA * ch )
{
   LINK( ch, first_char, last_char, next, prev );
}

/*
 * Load a mob section.
 */
void load_mobiles( AREA_DATA * tarea, FILE * fp )
{
   MOB_INDEX_DATA *pMobIndex;
   char *ln;
   int x1, x2, x3, x4, x5, x6, x7, x8;

   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   for( ;; )
   {
      int vnum, iHash;
      char letter;
      bool oldmob;
      bool tmpBootDb;

      letter = fread_letter( fp );
      if( letter != '#' )
      {
         bug( "%s: # not found.", __func__ );
         if( fBootDb )
         {
            shutdown_mud( "# not found" );
            exit( 1 );
         }
         else
            return;
      }

      vnum = fread_number( fp );
      if( vnum == 0 )
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if( get_mob_index( vnum ) )
      {
         if( tmpBootDb )
         {
            bug( "%s: vnum %d duplicated.", __func__, vnum );
            shutdown_mud( "duplicate vnum" );
            exit( 1 );
         }
         else
         {
            pMobIndex = get_mob_index( vnum );
            log_printf_plus( LOG_BUILD, sysdata.log_level, "Cleaning mobile: %d", vnum );
            clean_mob( pMobIndex );
            oldmob = TRUE;
         }
      }
      else
      {
         oldmob = FALSE;
         CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
      }
      fBootDb = tmpBootDb;

      pMobIndex->vnum = vnum;
      if( fBootDb )
      {
         if( !tarea->low_m_vnum )
            tarea->low_m_vnum = vnum;
         if( vnum > tarea->hi_m_vnum )
            tarea->hi_m_vnum = vnum;
      }
      pMobIndex->player_name = fread_string( fp );
      pMobIndex->short_descr = fread_string( fp );
      pMobIndex->long_descr = fread_string( fp );
      pMobIndex->description = fread_string( fp );

      pMobIndex->long_descr[0] = UPPER( pMobIndex->long_descr[0] );
      pMobIndex->description[0] = UPPER( pMobIndex->description[0] );

      pMobIndex->act = fread_number( fp ) | ACT_IS_NPC;
      pMobIndex->affected_by = fread_number( fp );
      pMobIndex->pShop = NULL;
      pMobIndex->rShop = NULL;
      pMobIndex->alignment = fread_number( fp );
      letter = fread_letter( fp );
      pMobIndex->level = fread_number( fp );

      pMobIndex->mobthac0 = fread_number( fp );
      pMobIndex->ac = fread_number( fp );
      pMobIndex->hitnodice = fread_number( fp );
      /*
       * 'd'      
       */ fread_letter( fp );
      pMobIndex->hitsizedice = fread_number( fp );
      /*
       * '+'      
       */ fread_letter( fp );
      pMobIndex->hitplus = fread_number( fp );
      pMobIndex->damnodice = fread_number( fp );
      /*
       * 'd'      
       */ fread_letter( fp );
      pMobIndex->damsizedice = fread_number( fp );
      /*
       * '+'      
       */ fread_letter( fp );
      pMobIndex->damplus = fread_number( fp );
      pMobIndex->gold = fread_number( fp );
      pMobIndex->exp = fread_number( fp );
      pMobIndex->position = fread_number( fp );
      pMobIndex->defposition = fread_number( fp );

      /*
       * Back to meaningful values.
       */
      pMobIndex->sex = fread_number( fp );

      if( letter != 'S' && letter != 'C' && letter != 'Z' )
      {
         bug( "%s: vnum %d: letter '%c' not Z, S or C.", __func__, vnum, letter );
         shutdown_mud( "bad mob data" );
         exit( 1 );
      }
      if( letter == 'C' || letter == 'Z' )   /* Realms complex mob    -Thoric  */
      {
         pMobIndex->perm_str = fread_number( fp );
         pMobIndex->perm_int = fread_number( fp );
         pMobIndex->perm_wis = fread_number( fp );
         pMobIndex->perm_dex = fread_number( fp );
         pMobIndex->perm_con = fread_number( fp );
         pMobIndex->perm_cha = fread_number( fp );
         pMobIndex->perm_lck = fread_number( fp );
         pMobIndex->saving_poison_death = fread_number( fp );
         pMobIndex->saving_wand = fread_number( fp );
         pMobIndex->saving_para_petri = fread_number( fp );
         pMobIndex->saving_breath = fread_number( fp );
         pMobIndex->saving_spell_staff = fread_number( fp );
         ln = fread_line( fp );
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
         sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
         pMobIndex->race = x1;
         pMobIndex->height = x3;
         pMobIndex->weight = x4;
         pMobIndex->speaks = x5;
         pMobIndex->speaking = x6;
         pMobIndex->numattacks = x7;
         if( !pMobIndex->speaks )
            pMobIndex->speaks = race_table[pMobIndex->race].language | LANG_BASIC;
         if( !pMobIndex->speaking )
            pMobIndex->speaking = race_table[pMobIndex->race].language;

         ln = fread_line( fp );
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
         sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
         pMobIndex->hitroll = x1;
         pMobIndex->damroll = x2;
         pMobIndex->xflags = x3;
         pMobIndex->resistant = x4;
         pMobIndex->immune = x5;
         pMobIndex->susceptible = x6;
         pMobIndex->attacks = x7;
         pMobIndex->defenses = x8;
      }
      else
      {
         pMobIndex->perm_str = 10;
         pMobIndex->perm_dex = 10;
         pMobIndex->perm_int = 10;
         pMobIndex->perm_wis = 10;
         pMobIndex->perm_cha = 10;
         pMobIndex->perm_con = 10;
         pMobIndex->perm_lck = 10;
         pMobIndex->race = 0;
         pMobIndex->xflags = 0;
         pMobIndex->resistant = 0;
         pMobIndex->immune = 0;
         pMobIndex->susceptible = 0;
         pMobIndex->numattacks = 0;
         pMobIndex->attacks = 0;
         pMobIndex->defenses = 0;
      }
      if( letter == 'Z' )  /*  STar Wars Reality Complex Mob  */
      {
         ln = fread_line( fp );
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
         sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
         pMobIndex->vip_flags = x1;
      }

      letter = fread_letter( fp );
      if( letter == '>' )
      {
         ungetc( letter, fp );
         mprog_read_programs( fp, pMobIndex );
      }
      else
         ungetc( letter, fp );

      if( !oldmob )
      {
         iHash = vnum % MAX_KEY_HASH;
         pMobIndex->next = mob_index_hash[iHash];
         mob_index_hash[iHash] = pMobIndex;
         top_mob_index++;
      }
   }

   return;
}

/*
 * Load an obj section.
 */
void load_objects( AREA_DATA * tarea, FILE * fp )
{
   OBJ_INDEX_DATA *pObjIndex;
   char letter;
   char *ln;
   int x1, x2, x3, x4, x5, x6;

   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   for( ;; )
   {
      int vnum, iHash;
      bool tmpBootDb;
      bool oldobj;

      letter = fread_letter( fp );
      if( letter != '#' )
      {
         bug( "%s: # not found.", __func__ );
         if( fBootDb )
         {
            shutdown_mud( "# not found" );
            exit( 1 );
         }
         else
            return;
      }

      vnum = fread_number( fp );
      if( vnum == 0 )
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if( get_obj_index( vnum ) )
      {
         if( tmpBootDb )
         {
            bug( "%s: vnum %d duplicated.", __func__, vnum );
            shutdown_mud( "duplicate vnum" );
            exit( 1 );
         }
         else
         {
            pObjIndex = get_obj_index( vnum );
            log_printf_plus( LOG_BUILD, sysdata.log_level, "Cleaning object: %d", vnum );
            clean_obj( pObjIndex );
            oldobj = TRUE;
         }
      }
      else
      {
         oldobj = FALSE;
         CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
      }
      fBootDb = tmpBootDb;

      pObjIndex->vnum = vnum;
      if( fBootDb )
      {
         if( !tarea->low_o_vnum )
            tarea->low_o_vnum = vnum;
         if( vnum > tarea->hi_o_vnum )
            tarea->hi_o_vnum = vnum;
      }
      pObjIndex->name = fread_string( fp );
      pObjIndex->short_descr = fread_string( fp );
      pObjIndex->description = fread_string( fp );
      pObjIndex->action_desc = fread_string( fp );

      /*
       * Commented out by Narn, Apr/96 to allow item short descs like 
       * Bonecrusher and Oblivion 
       */
      /*
       * pObjIndex->short_descr[0]  = LOWER(pObjIndex->short_descr[0]);
       */
      pObjIndex->description[0] = UPPER( pObjIndex->description[0] );

      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = 0;
      sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );
      pObjIndex->item_type = x1;
      pObjIndex->extra_flags = x2;
      pObjIndex->wear_flags = x3;
      pObjIndex->layers = x4;

      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = x5 = x6 = 0;
      sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
      pObjIndex->value[0] = x1;
      pObjIndex->value[1] = x2;
      pObjIndex->value[2] = x3;
      pObjIndex->value[3] = x4;
      pObjIndex->value[4] = x5;
      pObjIndex->value[5] = x6;
      pObjIndex->weight = fread_number( fp );
      pObjIndex->weight = UMAX( 1, pObjIndex->weight );
      pObjIndex->cost = fread_number( fp );
      pObjIndex->rent = fread_number( fp );  /* unused */

      for( ;; )
      {
         letter = fread_letter( fp );

         if( letter == 'A' )
         {
            AFFECT_DATA *paf;

            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = fread_number( fp );
            if( paf->location == APPLY_WEAPONSPELL
                || paf->location == APPLY_WEARSPELL || paf->location == APPLY_REMOVESPELL || paf->location == APPLY_STRIPSN )
               paf->modifier = slot_lookup( fread_number( fp ) );
            else
               paf->modifier = fread_number( fp );
            paf->bitvector = 0;
            LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
            top_affect++;
         }

         else if( letter == 'E' )
         {
            EXTRA_DESCR_DATA *ed;

            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword = fread_string( fp );
            ed->description = fread_string( fp );
            LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
            top_ed++;
         }

         else if( letter == '>' )
         {
            ungetc( letter, fp );
            oprog_read_programs( fp, pObjIndex );
         }

         else
         {
            ungetc( letter, fp );
            break;
         }
      }

      /*
       * Translate spell "slot numbers" to internal "skill numbers."
       */
      switch ( pObjIndex->item_type )
      {
         case ITEM_PILL:
         case ITEM_POTION:
            pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
            pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;

         case ITEM_DEVICE:
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;
         case ITEM_SALVE:
            pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
            pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
            break;
      }

      if( !oldobj )
      {
         iHash = vnum % MAX_KEY_HASH;
         pObjIndex->next = obj_index_hash[iHash];
         obj_index_hash[iHash] = pObjIndex;
         top_obj_index++;
      }
   }

   return;
}

/*
 * Load a reset section.
 */
void load_resets( AREA_DATA * tarea, FILE * fp )
{
   ROOM_INDEX_DATA *pRoomIndex = NULL;
   ROOM_INDEX_DATA *roomlist;
   bool not01 = FALSE;
   int count = 0;

   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   if( !tarea->first_room )
   {
      bug( "%s: No #ROOMS section found. Cannot load resets.", __func__ );
      if( fBootDb )
      {
         shutdown_mud( "No #ROOMS" );
         exit( 1 );
      }
      else
         return;
   }

   for( ;; )
   {
      EXIT_DATA *pexit;
      char letter;
      int extra, arg1, arg2, arg3;

      if( ( letter = fread_letter( fp ) ) == 'S' )
         break;

      if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }

      extra = fread_number( fp );
      if( letter == 'M' || letter == 'O' )
         extra = 0;
      arg1 = fread_number( fp );
      arg2 = fread_number( fp );
      arg3 = ( letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
      fread_to_eol( fp );
      ++count;

      /*
       * Validate parameters.
       * We're calling the index functions for the side effect.
       */
      switch ( letter )
      {
         default:
            bug( "%s: bad command '%c'.", __func__, letter );
            if( fBootDb )
               boot_log( "%s: %s (%d) bad command '%c'.", __func__, tarea->filename, count, letter );
            return;

         case 'M':
            if( get_mob_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) 'M': mobile %d doesn't exist.", __func__, tarea->filename, count, arg1 );

            if( ( pRoomIndex = get_room_index( arg3 ) ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) 'M': room %d doesn't exist.", __func__, tarea->filename, count, arg3 );
            else
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            break;

         case 'O':
            if( get_obj_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __func__, tarea->filename, count, letter, arg1 );

            if( ( pRoomIndex = get_room_index( arg3 ) ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': room %d doesn't exist.", __func__, tarea->filename, count, letter, arg3 );
            else
            {
               if( !pRoomIndex )
                  bug( "%s: Unable to add room reset - room not found.", __func__ );
               else
                  add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            }
            break;

         case 'P':
            if( get_obj_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __func__, tarea->filename, count, letter, arg1 );
            if( arg3 > 0 )
            {
               if( get_obj_index( arg3 ) == NULL && fBootDb )
                  boot_log( "%s: %s (%d) 'P': destination object %d doesn't exist.", __func__, tarea->filename, count,
                            arg3 );
               if( extra > 1 )
                  not01 = TRUE;
            }
            if( !pRoomIndex )
               bug( "%s: Unable to add room reset - room not found.", __func__ );
            else
            {
               if( arg3 == 0 )
                  arg3 = OBJ_VNUM_MONEY_ONE; // This may look stupid, but for some reason it works.
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            }
            break;

         case 'G':
         case 'E':
            if( get_obj_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __func__, tarea->filename, count, letter, arg1 );
            if( !pRoomIndex )
               bug( "%s: Unable to add room reset - room not found.", __func__ );
            else
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            break;

         case 'T':
            if( IS_SET( extra, TRAP_OBJ ) )
               bug( "%s: Unable to add legacy object trap reset. Must be converted manually.", __func__ );
            else
            {
               if( !( pRoomIndex = get_room_index( arg3 ) ) )
                  bug( "%s: Unable to add trap reset - room not found.", __func__ );
               else
                  add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            }
            break;

         case 'H':
            bug( "%s: Unable to convert legacy hide reset. Must be converted manually.", __func__ );
            break;

         case 'D':
            if( !( pRoomIndex = get_room_index( arg1 ) ) )
            {
               bug( "%s: 'D': room %d doesn't exist.", __func__, arg1 );
               bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'D': room %d doesn't exist.", __func__, tarea->filename, count, arg1 );
               break;
            }

            if( arg2 < 0 || arg2 > MAX_DIR + 1
                || !( pexit = get_exit( pRoomIndex, arg2 ) ) || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
            {
               bug( "%s: 'D': exit %d not door.", __func__, arg2 );
               bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'D': exit %d not door.", __func__, tarea->filename, count, arg2 );
            }

            if( arg3 < 0 || arg3 > 2 )
            {
               bug( "%s: 'D': bad 'locks': %d.", __func__, arg3 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'D': bad 'locks': %d.", __func__, tarea->filename, count, arg3 );
            }
            add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            break;

         case 'R':
            if( !( pRoomIndex = get_room_index( arg1 ) ) && fBootDb )
               boot_log( "%s: %s (%d) 'R': room %d doesn't exist.", __func__, tarea->filename, count, arg1 );
            else
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            if( arg2 < 0 || arg2 > 10 )
            {
               bug( "%s: 'R': bad exit %d.", __func__, arg2 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'R': bad exit %d.", __func__, tarea->filename, count, arg2 );
               break;
            }
            break;
      }
   }
   if( !not01 )
   {
      for( roomlist = tarea->first_room; roomlist; roomlist = roomlist->next_aroom )
         renumber_put_resets( roomlist );
   }
   return;
}

void load_room_reset( ROOM_INDEX_DATA * room, FILE * fp )
{
   EXIT_DATA *pexit;
   char letter;
   int extra, arg1, arg2, arg3;
   bool not01 = FALSE;
   int count = 0;

   letter = fread_letter( fp );
   extra = fread_number( fp );
   if( letter == 'M' || letter == 'O' )
      extra = 0;
   arg1 = fread_number( fp );
   arg2 = fread_number( fp );
   arg3 = ( letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
   fread_to_eol( fp );
   ++count;

   /*
    * Validate parameters.
    * We're calling the index functions for the side effect.
    */
   switch ( letter )
   {
      default:
         bug( "%s: bad command '%c'.", __func__, letter );
         if( fBootDb )
            boot_log( "%s: %s (%d) bad command '%c'.", __func__, room->area->filename, count, letter );
         return;

      case 'M':
         if( get_mob_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) 'M': mobile %d doesn't exist.", __func__, room->area->filename, count, arg1 );
         break;

      case 'O':
         if( get_obj_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __func__, room->area->filename, count, letter,
                      arg1 );
         break;

      case 'P':
         if( get_obj_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __func__, room->area->filename, count, letter,
                      arg1 );

         if( arg3 <= 0 )
            arg3 = OBJ_VNUM_MONEY_ONE; // This may look stupid, but for some reason it works.
         if( get_obj_index( arg3 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) 'P': destination object %d doesn't exist.", __func__, room->area->filename, count,
                      arg3 );
         if( extra > 1 )
            not01 = TRUE;
         break;

      case 'G':
      case 'E':
         if( get_obj_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __func__, room->area->filename, count, letter,
                      arg1 );
         break;

      case 'T':
      case 'H':
         break;

      case 'D':
         if( arg2 < 0 || arg2 > MAX_DIR + 1
             || !( pexit = get_exit( room, arg2 ) ) || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
         {
            bug( "%s: 'D': exit %d not door.", __func__, arg2 );
            bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
            if( fBootDb )
               boot_log( "%s: %s (%d) 'D': exit %d not door.", __func__, room->area->filename, count, arg2 );
         }

         if( arg3 < 0 || arg3 > 2 )
         {
            bug( "%s: 'D': bad 'locks': %d.", __func__, arg3 );
            if( fBootDb )
               boot_log( "%s: %s (%d) 'D': bad 'locks': %d.", __func__, room->area->filename, count, arg3 );
         }
         break;

      case 'R':
         if( arg2 < 0 || arg2 > 10 )
         {
            bug( "%s: 'R': bad exit %d.", __func__, arg2 );
            if( fBootDb )
               boot_log( "%s: %s (%d) 'R': bad exit %d.", __func__, room->area->filename, count, arg2 );
            break;
         }
         break;
   }
   add_reset( room, letter, extra, arg1, arg2, arg3 );

   if( !not01 )
      renumber_put_resets( room );
   return;
}

/*
 * Load a room section.
 */
void load_rooms( AREA_DATA * tarea, FILE * fp )
{
   ROOM_INDEX_DATA *pRoomIndex;
   char *ln;

   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      shutdown_mud( "No #AREA" );
      exit( 1 );
   }

   tarea->first_room = tarea->last_room = NULL;

   for( ;; )
   {
      int vnum;
      char letter;
      int door;
      int iHash;
      bool tmpBootDb;
      bool oldroom;
      int x1, x2, x3, x4, x5, x6, x7;

      letter = fread_letter( fp );
      if( letter != '#' )
      {
         bug( "%s: # not found.", __func__ );
         if( fBootDb )
         {
            shutdown_mud( "# not found" );
            exit( 1 );
         }
         else
            return;
      }

      vnum = fread_number( fp );
      if( vnum == 0 )
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if( get_room_index( vnum ) != NULL )
      {
         if( tmpBootDb )
         {
            bug( "%s: vnum %d duplicated.", __func__, vnum );
            shutdown_mud( "duplicate vnum" );
            exit( 1 );
         }
         else
         {
            pRoomIndex = get_room_index( vnum );
            log_printf_plus( LOG_BUILD, sysdata.log_level, "Cleaning room: %d", vnum );
            clean_room( pRoomIndex );
            oldroom = TRUE;
         }
      }
      else
      {
         oldroom = FALSE;
         CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
         pRoomIndex->first_person = NULL;
         pRoomIndex->last_person = NULL;
         pRoomIndex->first_content = NULL;
         pRoomIndex->last_content = NULL;
      }

      fBootDb = tmpBootDb;
      pRoomIndex->area = tarea;
      pRoomIndex->vnum = vnum;
      pRoomIndex->first_extradesc = NULL;
      pRoomIndex->last_extradesc = NULL;

      if( fBootDb )
      {
         if( !tarea->low_r_vnum )
            tarea->low_r_vnum = vnum;
         if( vnum > tarea->hi_r_vnum )
            tarea->hi_r_vnum = vnum;
      }
      pRoomIndex->name = fread_string( fp );
      pRoomIndex->description = fread_string( fp );

      /*
       * Area number         fread_number( fp ); 
       */
      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = x5 = x6 = 0;
      sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );

      pRoomIndex->room_flags = x2;
      pRoomIndex->sector_type = x3;
      pRoomIndex->room_flags2 = x4;
      pRoomIndex->tele_delay = x5;
      pRoomIndex->tele_vnum = x6;
      pRoomIndex->tunnel = x7;
      if( pRoomIndex->sector_type < 0 || pRoomIndex->sector_type >= SECT_MAX )
      {
         bug( "%s: vnum %d has bad sector_type %d.", __func__, vnum, pRoomIndex->sector_type );
         pRoomIndex->sector_type = 1;
      }
      pRoomIndex->light = 0;
      pRoomIndex->first_exit = NULL;
      pRoomIndex->last_exit = NULL;

      for( ;; )
      {
         letter = fread_letter( fp );

         if( letter == 'S' )
            break;

         if( letter == 'D' )
         {
            EXIT_DATA *pexit;
            int locks;

            door = fread_number( fp );
            if( door < 0 || door > 10 )
            {
               bug( "%s: vnum %d has bad door number %d.", __func__, vnum, door );
               if( fBootDb )
                  exit( 1 );
            }
            else
            {
               pexit = make_exit( pRoomIndex, NULL, door );
               pexit->description = fread_string( fp );
               pexit->keyword = fread_string( fp );
               pexit->exit_info = 0;
               ln = fread_line( fp );
               x1 = x2 = x3 = x4 = 0;
               sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

               locks = x1;
               pexit->key = x2;
               pexit->vnum = x3;
               pexit->vdir = door;
               pexit->distance = x4;

               switch ( locks )
               {
                  case 1:
                     pexit->exit_info = EX_ISDOOR;
                     break;
                  case 2:
                     pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
                     break;
                  default:
                     pexit->exit_info = locks;
               }
            }
         }
         else if( letter == 'E' )
         {
            EXTRA_DESCR_DATA *ed;

            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword = fread_string( fp );
            ed->description = fread_string( fp );
            LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc, next, prev );
            top_ed++;
         }
         else if( letter == 'R' )
            load_room_reset( pRoomIndex, fp );
         else if( letter == '>' )
         {
            ungetc( letter, fp );
            rprog_read_programs( fp, pRoomIndex );
         }
         else
         {
            bug( "%s: vnum %d has flag '%c' not 'DES'.", __func__, vnum, letter );
            shutdown_mud( "Room flag not DES" );
            exit( 1 );
         }
      }

      if( !oldroom )
      {
         iHash = vnum % MAX_KEY_HASH;
         pRoomIndex->next = room_index_hash[iHash];
         room_index_hash[iHash] = pRoomIndex;
         LINK( pRoomIndex, tarea->first_room, tarea->last_room, next_aroom, prev_aroom );
         top_room++;
      }
   }
   return;
}

/*
 * Load a shop section.
 */
void load_shops( FILE * fp )
{
   SHOP_DATA *pShop;

   for( ;; )
   {
      MOB_INDEX_DATA *pMobIndex;
      int iTrade;

      CREATE( pShop, SHOP_DATA, 1 );
      pShop->keeper = fread_number( fp );
      if( pShop->keeper == 0 )
      {
         DISPOSE( pShop );
         break;
      }
      for( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
         pShop->buy_type[iTrade] = fread_number( fp );
      pShop->profit_buy = fread_number( fp );
      pShop->profit_sell = fread_number( fp );
      pShop->profit_buy = URANGE( pShop->profit_sell + 5, pShop->profit_buy, 1000 );
      pShop->profit_sell = URANGE( 0, pShop->profit_sell, pShop->profit_buy - 5 );
      pShop->open_hour = fread_number( fp );
      pShop->close_hour = fread_number( fp );
      fread_to_eol( fp );
      pMobIndex = get_mob_index( pShop->keeper );
      pMobIndex->pShop = pShop;

      if( !first_shop )
         first_shop = pShop;
      else
         last_shop->next = pShop;
      pShop->next = NULL;
      pShop->prev = last_shop;
      last_shop = pShop;
      top_shop++;
   }
   return;
}

/*
 * Load a repair shop section.					-Thoric
 */
void load_repairs( FILE * fp )
{
   REPAIR_DATA *rShop;

   for( ;; )
   {
      MOB_INDEX_DATA *pMobIndex;
      int iFix;

      CREATE( rShop, REPAIR_DATA, 1 );
      rShop->keeper = fread_number( fp );
      if( rShop->keeper == 0 )
      {
         DISPOSE( rShop );
         break;
      }
      for( iFix = 0; iFix < MAX_FIX; iFix++ )
         rShop->fix_type[iFix] = fread_number( fp );
      rShop->profit_fix = fread_number( fp );
      rShop->shop_type = fread_number( fp );
      rShop->open_hour = fread_number( fp );
      rShop->close_hour = fread_number( fp );
      fread_to_eol( fp );
      pMobIndex = get_mob_index( rShop->keeper );
      pMobIndex->rShop = rShop;

      if( !first_repair )
         first_repair = rShop;
      else
         last_repair->next = rShop;
      rShop->next = NULL;
      rShop->prev = last_repair;
      last_repair = rShop;
      top_repair++;
   }
   return;
}

/*
 * Load spec proc declarations.
 */
void load_specials( FILE * fp )
{
   for( ;; )
   {
      MOB_INDEX_DATA *pMobIndex;
      char letter;

      switch ( letter = fread_letter( fp ) )
      {
         default:
            bug( "%s: letter '%c' not *MS.", __func__, letter );
            exit( 1 );

         case 'S':
            return;

         case '*':
            break;

         case 'M':
         {
            char *temp;
            pMobIndex = get_mob_index( fread_number( fp ) );
            temp = fread_word( fp );
            if( !pMobIndex )
            {
               bug( "%s: 'M': Invalid mob vnum!", __func__ );
               break;
            }

            if( !pMobIndex->spec_fun )
            {
               pMobIndex->spec_fun = spec_lookup( temp );
               if( pMobIndex->spec_fun == NULL )
               {
                  bug( "%s: 'M': vnum %d.", __func__, pMobIndex->vnum );
                  pMobIndex->spec_funname = NULL;
               }
               else
                  pMobIndex->spec_funname = STRALLOC( temp );
            }
            else if( !pMobIndex->spec_2 )
            {
               pMobIndex->spec_2 = spec_lookup( temp );
               if( pMobIndex->spec_2 == NULL )
               {
                  bug( "%s: 'M': vnum %d.", __func__, pMobIndex->vnum );
                  pMobIndex->spec_funname2 = NULL;
               }
               else
                  pMobIndex->spec_funname2 = STRALLOC( temp );
            }
         }
            break;
      }
      fread_to_eol( fp );
   }
}

/*
 * Load soft / hard area ranges.
 */
void load_ranges( AREA_DATA * tarea, FILE * fp )
{
   int x1, x2, x3, x4;
   char *ln;

   if( !tarea )
   {
      bug( "%s: no #AREA seen yet.", __func__ );
      shutdown_mud( "No #AREA" );
      exit( 1 );
   }

   for( ;; )
   {
      ln = fread_line( fp );

      if( ln[0] == '$' )
         break;

      x1 = x2 = x3 = x4 = 0;
      sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

      tarea->low_soft_range = x1;
      tarea->hi_soft_range = x2;
      tarea->low_hard_range = x3;
      tarea->hi_hard_range = x4;
   }
   return;

}

/*
 * Go through all areas, and set up initial economy based on mob
 * levels and gold
 */
void initialize_economy( void )
{
   AREA_DATA *tarea;
   MOB_INDEX_DATA *mob;
   int idx, gold, rng;

   for( tarea = first_area; tarea; tarea = tarea->next )
   {
      /*
       * skip area if they already got some gold 
       */
      if( tarea->high_economy > 0 || tarea->low_economy > 10000 )
         continue;
      rng = tarea->hi_soft_range - tarea->low_soft_range;
      if( rng )
         rng /= 2;
      else
         rng = 25;
      gold = rng * rng * 10000;
      boost_economy( tarea, gold );
      for( idx = tarea->low_m_vnum; idx < tarea->hi_m_vnum; idx++ )
         if( ( mob = get_mob_index( idx ) ) != NULL )
            boost_economy( tarea, mob->gold * 10 );
   }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
   ROOM_INDEX_DATA *pRoomIndex;
   EXIT_DATA *pexit, *pexit_next, *rev_exit;
   int iHash;

   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
      for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      {
         bool fexit;

         fexit = FALSE;
         for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
         {
            pexit_next = pexit->next;
            pexit->rvnum = pRoomIndex->vnum;
            if( pexit->vnum <= 0 || ( pexit->to_room = get_room_index( pexit->vnum ) ) == NULL )
            {
               if( fBootDb )
                  boot_log( "%s: room %d, exit %s leads to bad vnum (%d)", __func__,
                            pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum );

               bug( "%s: Deleting %s exit in room %d", __func__, dir_name[pexit->vdir], pRoomIndex->vnum );
               extract_exit( pRoomIndex, pexit );
            }
            else
               fexit = TRUE;
         }
         if( !fexit )
            SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
      }
   }

   /*
    * Set all the rexit pointers   -Thoric 
    */
   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
      for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      {
         for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
         {
            if( pexit->to_room && !pexit->rexit )
            {
               rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
               if( rev_exit )
               {
                  pexit->rexit = rev_exit;
                  rev_exit->rexit = pexit;
               }
            }
         }
      }
   }
   return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
EXIT_DATA *get_exit_number( ROOM_INDEX_DATA * room, int xit )
{
   EXIT_DATA *pexit;
   int count;

   count = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
      if( ++count == xit )
         return pexit;
   return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA ** xit1, EXIT_DATA ** xit2 )
{
   int d1, d2;

   d1 = ( *xit1 )->vdir;
   d2 = ( *xit2 )->vdir;

   if( d1 < d2 )
      return -1;
   if( d1 > d2 )
      return 1;
   return 0;
}

void sort_exits( ROOM_INDEX_DATA * room )
{
   EXIT_DATA *pexit; /* *texit *//* Unused */
   EXIT_DATA *exits[MAX_REXITS];
   int x, nexits;

   nexits = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
   {
      exits[nexits++] = pexit;
      if( nexits > MAX_REXITS )
      {
         bug( "%s: more than %d exits in room... fatal", __func__, nexits );
         return;
      }
   }
   qsort( &exits[0], nexits, sizeof( EXIT_DATA * ), ( int ( * )( const void *, const void * ) )exit_comp );
   for( x = 0; x < nexits; x++ )
   {
      if( x > 0 )
         exits[x]->prev = exits[x - 1];
      else
      {
         exits[x]->prev = NULL;
         room->first_exit = exits[x];
      }
      if( x >= ( nexits - 1 ) )
      {
         exits[x]->next = NULL;
         room->last_exit = exits[x];
      }
      else
         exits[x]->next = exits[x + 1];
   }
}

void randomize_exits( ROOM_INDEX_DATA * room, short maxdir )
{
   EXIT_DATA *pexit;
   int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
   int vdirs[MAX_REXITS];

   nexits = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
      vdirs[nexits++] = pexit->vdir;

   for( d0 = 0; d0 < nexits; d0++ )
   {
      if( vdirs[d0] > maxdir )
         continue;
      count = 0;
      while( vdirs[( d1 = number_range( d0, nexits - 1 ) )] > maxdir || ++count > 5 );
      if( vdirs[d1] > maxdir )
         continue;
      door = vdirs[d0];
      vdirs[d0] = vdirs[d1];
      vdirs[d1] = door;
   }
   count = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
      pexit->vdir = vdirs[count++];

   sort_exits( room );
}

/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
   AREA_DATA *pArea;

   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      CHAR_DATA *pch;
      int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

      if( ( reset_age == -1 && pArea->age == -1 ) || ++pArea->age < ( reset_age - 1 ) )
         continue;

      /*
       * Check for PC's.
       */
      if( pArea->nplayer > 0 && pArea->age == ( reset_age - 1 ) )
      {
         char buf[MAX_STRING_LENGTH];

         /*
          * Rennard 
          */
         if( pArea->resetmsg )
            snprintf( buf, MAX_STRING_LENGTH, "%s\r\n", pArea->resetmsg );
         else
            strlcpy( buf, "You hear some squeaking sounds...\r\n", MAX_STRING_LENGTH );
         for( pch = first_char; pch; pch = pch->next )
         {
            if( !IS_NPC( pch ) && IS_AWAKE( pch ) && pch->in_room && pch->in_room->area == pArea )
            {
               set_char_color( AT_RESET, pch );
               send_to_char( buf, pch );
            }
         }
      }

      /*
       * Check age and reset.
       * Note: Mud Academy resets every 3 minutes (not 15).
       */
      if( pArea->nplayer == 0 || pArea->age >= reset_age )
      {
         ROOM_INDEX_DATA *pRoomIndex;

         reset_area( pArea );
         if( reset_age == -1 )
            pArea->age = -1;
         else
            pArea->age = number_range( 0, reset_age / 5 );
         pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
         if( pRoomIndex != NULL && pArea == pRoomIndex->area && pArea->reset_frequency == 0 )
            pArea->age = 15 - 3;
      }
   }
   return;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA * pMobIndex )
{
   CHAR_DATA *mob;

   if( !pMobIndex )
   {
      bug( "%s: NULL pMobIndex.", __func__ );
      exit( 1 );
   }

   CREATE( mob, CHAR_DATA, 1 );
   clear_char( mob );
   mob->pIndexData = pMobIndex;

   mob->editor = NULL;
   mob->name = QUICKLINK( pMobIndex->player_name );
   mob->short_descr = QUICKLINK( pMobIndex->short_descr );
   mob->long_descr = QUICKLINK( pMobIndex->long_descr );
   mob->description = QUICKLINK( pMobIndex->description );
   mob->spec_fun = pMobIndex->spec_fun;
   mob->spec_2 = pMobIndex->spec_2;
   if( pMobIndex->spec_funname )
      mob->spec_funname = QUICKLINK( pMobIndex->spec_funname );
   if( pMobIndex->spec_funname2 )
      mob->spec_funname2 = QUICKLINK( pMobIndex->spec_funname2 );
   mob->mpscriptpos = 0;
   mob->top_level = number_fuzzy( pMobIndex->level );
   {
      int ability;
      for( ability = 0; ability < MAX_ABILITY; ability++ )
         mob->skill_level[ability] = mob->top_level;
   }
   mob->act = pMobIndex->act;
   mob->home_vnum = -1;
   mob->resetvnum = -1;
   mob->resetnum = -1;
   mob->affected_by = pMobIndex->affected_by;
   mob->alignment = pMobIndex->alignment;
   mob->sex = pMobIndex->sex;
   mob->main_ability = 0;
   mob->mob_clan = STRALLOC( "" );
   mob->was_sentinel = NULL;
   mob->plr_home = NULL;
   mob->guard_data = NULL;

   if( pMobIndex->ac )
      mob->armor = pMobIndex->ac;
   else
      mob->armor = ( short )100 - mob->top_level * 2.5;

   if( !pMobIndex->hitnodice )
      mob->max_hit = mob->top_level * 10 + number_range( mob->top_level, mob->top_level * 10 );
   else
      mob->max_hit = pMobIndex->hitnodice * number_range( 1, pMobIndex->hitsizedice ) + pMobIndex->hitplus;
   mob->hit = mob->max_hit;
   /*
    * lets put things back the way they used to be! -Thoric 
    */
   mob->gold = pMobIndex->gold;
   mob->position = pMobIndex->position;
   mob->defposition = pMobIndex->defposition;
   mob->barenumdie = pMobIndex->damnodice;
   mob->baresizedie = pMobIndex->damsizedice;
   mob->mobthac0 = pMobIndex->mobthac0;
   mob->hitplus = pMobIndex->hitplus;
   mob->damplus = pMobIndex->damplus;
   mob->mana = 0;
   mob->max_mana = 0;
   mob->perm_str = pMobIndex->perm_str;
   mob->perm_dex = pMobIndex->perm_dex;
   mob->perm_wis = pMobIndex->perm_wis;
   mob->perm_int = pMobIndex->perm_int;
   mob->perm_con = pMobIndex->perm_con;
   mob->perm_cha = pMobIndex->perm_cha;
   mob->perm_lck = pMobIndex->perm_lck;
   mob->hitroll = pMobIndex->hitroll;
   mob->damroll = pMobIndex->damroll;
   mob->race = pMobIndex->race;
   mob->xflags = pMobIndex->xflags;
   mob->saving_poison_death = pMobIndex->saving_poison_death;
   mob->saving_wand = pMobIndex->saving_wand;
   mob->saving_para_petri = pMobIndex->saving_para_petri;
   mob->saving_breath = pMobIndex->saving_breath;
   mob->saving_spell_staff = pMobIndex->saving_spell_staff;
   mob->height = pMobIndex->height;
   mob->weight = pMobIndex->weight;
   mob->resistant = pMobIndex->resistant;
   mob->immune = pMobIndex->immune;
   mob->susceptible = pMobIndex->susceptible;
   mob->attacks = pMobIndex->attacks;
   mob->defenses = pMobIndex->defenses;
   mob->numattacks = pMobIndex->numattacks;
   mob->speaks = pMobIndex->speaks;
   mob->speaking = pMobIndex->speaking;
   mob->vip_flags = pMobIndex->vip_flags;

   /*
    * Insert in list.
    */
   add_char( mob );
   pMobIndex->count++;
   nummobsloaded++;
   return mob;
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA * pObjIndex, int level )
{
   OBJ_DATA *obj;

   if( !pObjIndex )
   {
      bug( "%s: NULL pObjIndex.", __func__ );
      exit( 1 );
   }

   CREATE( obj, OBJ_DATA, 1 );

   obj->pIndexData = pObjIndex;
   obj->in_room = NULL;
   obj->level = level;
   obj->wear_loc = -1;
   obj->count = 1;
   cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
   obj->serial = obj->pIndexData->serial = cur_obj_serial;

   obj->armed_by = STRALLOC( "" );
   obj->name = QUICKLINK( pObjIndex->name );
   obj->short_descr = QUICKLINK( pObjIndex->short_descr );
   obj->description = QUICKLINK( pObjIndex->description );
   obj->action_desc = QUICKLINK( pObjIndex->action_desc );
   obj->item_type = pObjIndex->item_type;
   obj->extra_flags = pObjIndex->extra_flags;
   obj->wear_flags = pObjIndex->wear_flags;
   obj->value[0] = pObjIndex->value[0];
   obj->value[1] = pObjIndex->value[1];
   obj->value[2] = pObjIndex->value[2];
   obj->value[3] = pObjIndex->value[3];
   obj->value[4] = pObjIndex->value[4];
   obj->value[5] = pObjIndex->value[5];
   obj->weight = pObjIndex->weight;
   obj->cost = pObjIndex->cost;

   /*
    * Mess with object properties.
    */
   switch ( obj->item_type )
   {
      default:
         bug( "%s: vnum %d bad type.", __func__, pObjIndex->vnum );
         bug( "------------------------>     %d", obj->item_type );
         break;

      case ITEM_GOVERNMENT:
      case ITEM_SPACECRAFT:
      case ITEM_RAWSPICE:
      case ITEM_LENS:
      case ITEM_CRYSTAL:
      case ITEM_DURAPLAST:
      case ITEM_DURASTEEL:
      case ITEM_SUPERCONDUCTOR:
      case ITEM_COMLINK:
      case ITEM_MEDPAC:
      case ITEM_FABRIC:
      case ITEM_RARE_METAL:
      case ITEM_MAGNET:
      case ITEM_THREAD:
      case ITEM_CHEMICAL:
      case ITEM_SPICE:
      case ITEM_SMUT:
      case ITEM_OVEN:
      case ITEM_MIRROR:
      case ITEM_CIRCUIT:
      case ITEM_TOOLKIT:
      case ITEM_LIGHT:
      case ITEM_TREASURE:
      case ITEM_FURNITURE:
      case ITEM_TRASH:
      case ITEM_CONTAINER:
      case ITEM_DRINK_CON:
      case ITEM_KEY:
      case ITEM_BINDERS:
      case ITEM_GOGGLES:
         break;
      case ITEM_FOOD:
         /*
          * optional food condition (rotting food)    -Thoric
          * value1 is the max condition of the food
          * value4 is the optional initial condition
          */
         if( obj->value[4] )
            obj->timer = obj->value[4];
         else
            obj->timer = obj->value[1];
         break;

      case ITEM_DROID_CORPSE:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
      case ITEM_FOUNTAIN:
      case ITEM_SCRAPS:
      case ITEM_GRENADE:
      case ITEM_EMP_GRENADE:
      case ITEM_SHIPBOMB:
      case ITEM_LANDMINE:
      case ITEM_FIRE:
      case ITEM_BOOK:
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_BUTTON:
      case ITEM_DIAL:
      case ITEM_TRAP:
      case ITEM_MAP:
      case ITEM_PAPER:
      case ITEM_PEN:
      case ITEM_LOCKPICK:
      case ITEM_FUEL:
      case ITEM_MISSILE:
      case ITEM_COMMSYSTEM:
      case ITEM_MODULE:
      case ITEM_DATAPAD:
      case ITEM_BEACON:
      case ITEM_BUG:
      case ITEM_RLAUNCHER:
      case ITEM_GLAUNCHER:
      case ITEM_SHOVEL:
         break;

      case ITEM_SALVE:
         obj->value[3] = number_fuzzy( obj->value[3] );
         break;

      case ITEM_DEVICE:
         obj->value[0] = number_fuzzy( obj->value[0] );
         obj->value[1] = number_fuzzy( obj->value[1] );
         obj->value[2] = obj->value[1];
         break;

      case ITEM_BATTERY:
         if( obj->value[0] <= 0 )
            obj->value[0] = number_fuzzy( 95 );
         break;


      case ITEM_BOLT:
         if( obj->value[0] <= 0 )
            obj->value[0] = number_fuzzy( 95 );
         break;

      case ITEM_AMMO:
         if( obj->value[0] <= 0 )
            obj->value[0] = number_fuzzy( 495 );
         break;

      case ITEM_WEAPON:
/*
	if ( obj->value[1] && obj->value[2] )
	   obj->value[2] *= obj->value[1];
	else
	{
	   obj->value[1] = number_fuzzy( number_fuzzy( 1 + level/20 ) );
	   obj->value[2] = number_fuzzy( number_fuzzy( 10 + level/10 ) ); 
	}
*/
         if( obj->value[1] > obj->value[2] )
            obj->value[1] = obj->value[2] / 3;
         if( obj->value[0] == 0 )
            obj->value[0] = INIT_WEAPON_CONDITION;
         switch ( obj->value[3] )
         {
            case WEAPON_BLASTER:
            case WEAPON_LIGHTSABER:
            case WEAPON_DUAL_LIGHTSABER:
            case WEAPON_VIBRO_BLADE:
            case WEAPON_FORCE_PIKE:
            case WEAPON_BOWCASTER:
               if( obj->value[5] <= 0 )
                  obj->value[5] = number_fuzzy( 1000 );
         }
         obj->value[4] = obj->value[5];
         break;

      case ITEM_ARMOR:
         if( obj->value[0] == 0 )
            obj->value[0] = obj->value[1];
         obj->timer = obj->value[3];
         break;

      case ITEM_POTION:
      case ITEM_PILL:
         obj->value[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
         break;

      case ITEM_MONEY:
         obj->value[0] = obj->cost;
         break;
   }

   LINK( obj, first_object, last_object, next, prev );
   ++pObjIndex->count;
   ++numobjsloaded;
   ++physicalobjects;

   return obj;
}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA * ch )
{
   ch->editor = NULL;
   ch->hunting = NULL;
   ch->fearing = NULL;
   ch->hating = NULL;
   ch->name = NULL;
   ch->short_descr = NULL;
   ch->long_descr = NULL;
   ch->description = NULL;
   ch->next = NULL;
   ch->prev = NULL;
   ch->first_carrying = NULL;
   ch->last_carrying = NULL;
   ch->next_in_room = NULL;
   ch->prev_in_room = NULL;
   ch->fighting = NULL;
   ch->switched = NULL;
   ch->first_affect = NULL;
   ch->last_affect = NULL;
   ch->prev_cmd = NULL; /* maps */
   ch->last_cmd = NULL;
   ch->dest_buf = NULL;
   ch->dest_buf_2 = NULL;
   ch->spare_ptr = NULL;
   ch->mount = NULL;
   ch->affected_by = 0;
   ch->logon = current_time;
   ch->armor = 100;
   ch->position = POS_STANDING;
   ch->hit = 500;
   ch->max_hit = 500;
   ch->mana = 1000;
   ch->max_mana = 0;
   ch->move = 1000;
   ch->max_move = 1000;
   ch->height = 72;
   ch->weight = 180;
   ch->xflags = 0;
   ch->race = 0;
   ch->speaking = LANG_BASIC;
   ch->speaks = LANG_BASIC;
   ch->barenumdie = 1;
   ch->baresizedie = 4;
   ch->substate = 0;
   ch->tempnum = 0;
   ch->perm_str = 10;
   ch->perm_dex = 10;
   ch->perm_int = 10;
   ch->perm_wis = 10;
   ch->perm_cha = 10;
   ch->perm_con = 10;
   ch->perm_lck = 10;
   ch->mod_str = 0;
   ch->mod_dex = 0;
   ch->mod_int = 0;
   ch->mod_wis = 0;
   ch->mod_cha = 0;
   ch->mod_con = 0;
   ch->mod_lck = 0;
   ch->plr_home = NULL;
   return;
}

/*
 * Free a character.
 */
void free_char( CHAR_DATA * ch )
{
   OBJ_DATA *obj;
   BUG_DATA *bugs;
   CONTRACT_DATA *contract;
   FELLOW_DATA *fellow;
   AFFECT_DATA *paf;
   TIMER *timer;
   MPROG_ACT_LIST *mpact, *mpact_next;
   NOTE_DATA *comments, *comments_next;

   if( !ch )
   {
      bug( "%s: null ch!", __func__ );
      return;
   }

   if( ch->desc )
      bug( "%s: char still has descriptor.", __func__ );

   while( ( obj = ch->last_carrying ) != NULL )
      extract_obj( obj );

   while( ( paf = ch->last_affect ) != NULL )
      affect_remove( ch, paf );

   while( ( timer = ch->first_timer ) != NULL )
      extract_timer( ch, timer );

   while( ( bugs = ch->first_bug ) != NULL )
   {
      UNLINK( bugs, ch->first_bug, ch->last_bug, next_in_bug, prev_in_bug );
      STRFREE( bugs->name );
      DISPOSE( bugs );
   }

   while( ( contract = ch->first_contract ) != NULL )
   {
      UNLINK( contract, ch->first_contract, ch->last_contract, next_in_contract, prev_in_contract );
      DISPOSE( contract->target );
      DISPOSE( contract );
   }

   while( ( fellow = ch->first_fellow ) != NULL )
   {
      UNLINK( fellow, ch->first_fellow, ch->last_fellow, next, prev );
      STRFREE( fellow->knownas );
      STRFREE( fellow->victim );
      DISPOSE( fellow );
   }

   STRFREE( ch->name );
   STRFREE( ch->short_descr );
   STRFREE( ch->long_descr );
   STRFREE( ch->description );
   STRFREE( ch->spec_funname );
   STRFREE( ch->spec_funname2 );
   if( ch->editor )
      stop_editing( ch );

   stop_hunting( ch );
   stop_hating( ch );
   stop_fearing( ch );
   free_fight( ch );

   if( ch->pnote )
      free_note( ch->pnote );

   if( ch->pcdata )
   {
      STRFREE( ch->pcdata->clan_name );
      DISPOSE( ch->pcdata->pwd );   /* no hash */
      DISPOSE( ch->pcdata->email ); /* no hash */
      DISPOSE( ch->pcdata->bamfin );   /* no hash */
      DISPOSE( ch->pcdata->bamfout );  /* no hash */
      STRFREE( ch->pcdata->title );
      STRFREE( ch->pcdata->disguise );
      STRFREE( ch->pcdata->bio );
      DISPOSE( ch->pcdata->bestowments ); /* no hash */
      DISPOSE( ch->pcdata->homepage ); /* no hash */
      DISPOSE( ch->pcdata->screenname );  /* no hash */
      DISPOSE( ch->pcdata->image ); /* no hash */
      STRFREE( ch->pcdata->authed_by );
      STRFREE( ch->pcdata->prompt );
      if( ch->pcdata->subprompt )
         STRFREE( ch->pcdata->subprompt );

      STRFREE( ch->pcdata->store_title );
      STRFREE( ch->pcdata->tell_snoop );
      STRFREE( ch->pcdata->last_name );
      STRFREE( ch->pcdata->fiance );
      STRFREE( ch->pcdata->propose );
      STRFREE( ch->pcdata->proposed );
      STRFREE( ch->pcdata->spouse );
      DISPOSE( ch->pcdata->shipname );
      STRFREE( ch->pcdata->helled_by );

      DISPOSE( ch->pcdata );
   }

   STRFREE( ch->owner );
   STRFREE( ch->force_master );
   STRFREE( ch->force_temp_master );
   STRFREE( ch->force_disguise );
   STRFREE( ch->comfreq );
   DISPOSE( ch->rank );

   for( mpact = ch->mpact; mpact; mpact = mpact_next )
   {
      mpact_next = mpact->next;
      DISPOSE( mpact->buf );
      DISPOSE( mpact );
   }

   for( comments = ch->comments; comments; comments = comments_next )
   {
      comments_next = comments->next;
      STRFREE( comments->text );
      STRFREE( comments->to_list );
      STRFREE( comments->subject );
      STRFREE( comments->sender );
      STRFREE( comments->date );
      DISPOSE( comments );
   }
   DISPOSE( ch );
}

/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA * ed )
{
   for( ; ed; ed = ed->next )
      if( is_name( name, ed->keyword ) )
         return ed->description;

   return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
   MOB_INDEX_DATA *pMobIndex;

   if( vnum < 0 )
      vnum = 0;

   for( pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex; pMobIndex = pMobIndex->next )
      if( pMobIndex->vnum == vnum )
         return pMobIndex;

   if( fBootDb )
      bug( "%s: bad vnum %d.", __func__, vnum );

   return NULL;
}

/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
   OBJ_INDEX_DATA *pObjIndex;

   if( vnum < 0 )
      vnum = 0;

   for( pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex; pObjIndex = pObjIndex->next )
      if( pObjIndex->vnum == vnum )
         return pObjIndex;

   if( fBootDb )
      bug( "%s: bad vnum %d.", __func__, vnum );

   return NULL;
}

/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
   ROOM_INDEX_DATA *pRoomIndex;

   if( vnum < 0 )
      vnum = 0;

   for( pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      if( pRoomIndex->vnum == vnum )
         return pRoomIndex;

   if( fBootDb )
      bug( "%s: bad vnum %d.", __func__, vnum );

   return NULL;
}

/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */

/*
 * Read a letter from a file.
 */
char fread_letter( FILE * fp )
{
   char c;

   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         return '\0';
      }
      c = getc( fp );
   }
   while( isspace( c ) );

   return c;
}

/*
 * Read a float number from a file. Turn the result into a float value.
 */
float fread_float( FILE * fp )
{
   float number;
   bool sign, decimal;
   char c;
   double place = 0;

   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.", __func__ );
         if( fBootDb )
         {
            shutdown_mud( "Corrupt file somewhere." );
            exit( 1 );
         }
         return 0;
      }
      c = getc( fp );
   }
   while( isspace( c ) );

   number = 0;

   sign = FALSE;
   decimal = FALSE;

   if( c == '+' )
      c = getc( fp );
   else if( c == '-' )
   {
      sign = TRUE;
      c = getc( fp );
   }

   if( !isdigit( c ) )
   {
      bug( "%s: bad format. (%c)", __func__, c );
      if( fBootDb )
         exit( 1 );
      return 0;
   }

   while( 1 )
   {
      if( c == '.' || isdigit( c ) )
      {
         if( c == '.' )
         {
            decimal = TRUE;
            c = getc( fp );
         }

         if( feof( fp ) )
         {
            bug( "%s: EOF encountered on read.", __func__ );
            if( fBootDb )
               exit( 1 );
            return number;
         }
         if( !decimal )
            number = number * 10 + c - '0';
         else
         {
            place++;
            number += pow( 10, ( -1 * place ) ) * ( c - '0' );
         }
         c = getc( fp );
      }
      else
         break;
   }

   if( sign )
      number = 0 - number;

   if( c == '|' )
      number += fread_float( fp );
   else if( c != ' ' )
      ungetc( c, fp );

   return number;
}

/*
 * Read a number from a file.
 */
int fread_number( FILE * fp )
{
   int number;
   bool sign;
   char c;

   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         return 0;
      }
      c = getc( fp );
   }
   while( isspace( c ) );

   number = 0;

   sign = FALSE;
   if( c == '+' )
   {
      c = getc( fp );
   }
   else if( c == '-' )
   {
      sign = TRUE;
      c = getc( fp );
   }

   if( !isdigit( c ) )
   {
      bug( "%s: bad format. (%c)", __func__, c );
      if( fBootDb )
         exit( 1 );
      return 0;
   }

   while( isdigit( c ) )
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         return number;
      }
      number = number * 10 + c - '0';
      c = getc( fp );
   }

   if( sign )
      number = 0 - number;

   if( c == '|' )
      number += fread_number( fp );
   else if( c != ' ' )
      ungetc( c, fp );

   return number;
}

bool is_valid_filename( CHAR_DATA * ch, const char *direct, const char *filename )
{
   char newfilename[256];
   struct stat fst;

   /*
    * Length restrictions 
    */
   if( !filename || filename[0] == '\0' || strlen( filename ) < 3 )
   {
      if( !filename || !str_cmp( filename, "" ) )
         send_to_char( "Empty filename is not valid.\r\n", ch );
      else
         ch_printf( ch, "%s: Filename is too short.\r\n", filename );
      return FALSE;
   }

   /*
    * Illegal characters 
    */
   if( strstr( filename, ".." ) || strstr( filename, "/" ) || strstr( filename, "\\" ) )
   {
      send_to_char( "A filename may not contain a '..', '/', or '\\' in it.\r\n", ch );
      return FALSE;
   }

   /*
    * If that filename is already being used lets not allow it now to be on the safe side 
    */
   snprintf( newfilename, sizeof( newfilename ), "%s%s", direct, filename );
   if( stat( newfilename, &fst ) != -1 )
   {
      ch_printf( ch, "%s is already an existing filename.\r\n", newfilename );
      return FALSE;
   }

   /*
    * If we got here assume its valid 
    */
   return TRUE;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *plast;
   char c;
   int ln;

   plast = buf;
   buf[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         return STRALLOC( "" );
      }
      c = getc( fp );
   }
   while( isspace( c ) );

   if( ( *plast++ = c ) == '~' )
      return STRALLOC( "" );

   for( ;; )
   {
      if( ln >= ( MAX_STRING_LENGTH - 1 ) )
      {
         bug( "%s: string too long", __func__ );
         *plast = '\0';
         return STRALLOC( buf );
      }
      switch ( *plast = getc( fp ) )
      {
         default:
            plast++;
            ln++;
            break;

         case EOF:
            bug( "%s: EOF", __func__ );
            if( fBootDb )
               exit( 1 );
            *plast = '\0';
            return STRALLOC( buf );
            break;

         case '\n':
            plast++;
            ln++;
            *plast++ = '\r';
            ln++;
            break;

         case '\r':
            break;

         case '~':
            *plast = '\0';
            return STRALLOC( buf );
      }
   }
}

/*
 * Read a string from file fp using strdup (ie: no string hashing)
 */
char *fread_string_nohash( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *plast;
   char c;
   int ln;

   plast = buf;
   buf[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         return strdup( "" );
      }
      c = getc( fp );
   }
   while( isspace( c ) );

   if( ( *plast++ = c ) == '~' )
      return strdup( "" );

   for( ;; )
   {
      if( ln >= ( MAX_STRING_LENGTH - 1 ) )
      {
         bug( "%s: string too long", __func__ );
         *plast = '\0';
         return strdup( buf );
      }
      switch ( *plast = getc( fp ) )
      {
         default:
            plast++;
            ln++;
            break;

         case EOF:
            bug( "%s: EOF", __func__ );
            if( fBootDb )
               exit( 1 );
            *plast = '\0';
            return strdup( buf );
            break;

         case '\n':
            plast++;
            ln++;
            *plast++ = '\r';
            ln++;
            break;

         case '\r':
            break;

         case '~':
            *plast = '\0';
            return strdup( buf );
      }
   }
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE * fp )
{
   char c;

   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         return;
      }
      c = getc( fp );
   }
   while( c != '\n' && c != '\r' );

   do
   {
      c = getc( fp );
   }
   while( c == '\n' || c == '\r' );

   ungetc( c, fp );
   return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE * fp )
{
   static char line[MAX_STRING_LENGTH];
   char *pline;
   char c;
   int ln;

   pline = line;
   line[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         strlcpy( line, "", MAX_STRING_LENGTH );
         return line;
      }
      c = getc( fp );
   }
   while( isspace( c ) );

   ungetc( c, fp );
   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         *pline = '\0';
         return line;
      }
      c = getc( fp );
      *pline++ = c;
      ln++;
      if( ln >= ( MAX_STRING_LENGTH - 1 ) )
      {
         bug( "%s: line too long", __func__ );
         break;
      }
   }
   while( c != '\n' && c != '\r' );

   do
   {
      c = getc( fp );
   }
   while( c == '\n' || c == '\r' );

   ungetc( c, fp );
   *pline = '\0';
   return line;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE * fp )
{
   static char word[MAX_INPUT_LENGTH];
   char *pword;
   char cEnd;

   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         word[0] = '\0';
         return word;
      }
      cEnd = getc( fp );
   }
   while( isspace( cEnd ) );

   if( cEnd == '\'' || cEnd == '"' )
   {
      pword = word;
   }
   else
   {
      word[0] = cEnd;
      pword = word + 1;
      cEnd = ' ';
   }

   for( ; pword < word + MAX_INPUT_LENGTH; pword++ )
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.\r\n", __func__ );
         if( fBootDb )
            exit( 1 );
         word[0] = '\0';
         return word;
      }
      *pword = getc( fp );
      if( cEnd == ' ' ? isspace( *pword ) : *pword == cEnd )
      {
         if( cEnd == ' ' )
            ungetc( *pword, fp );
         *pword = '\0';
         return word;
      }
   }

   bug( "%s: word too long", __func__ );
   exit( 1 );
   return NULL;
}

void do_memory( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int hash;

   argument = one_argument( argument, arg );
   ch_printf( ch, "Affects %5d    Areas   %5d\r\n", top_affect, top_area );
   ch_printf( ch, "ExtDes  %5d    Exits   %5d\r\n", top_ed, top_exit );
   ch_printf( ch, "Helps   %5d    Resets  %5d\r\n", top_help, top_reset );
   ch_printf( ch, "IdxMobs %5d    Mobs    %5d\r\n", top_mob_index, nummobsloaded );
   ch_printf( ch, "IdxObjs %5d    Objs    %5d (%d)\r\n", top_obj_index, numobjsloaded, physicalobjects );
   ch_printf( ch, "Rooms   %5d    VRooms  %5d\r\n", top_room, top_vroom );
   ch_printf( ch, "Shops   %5d    RepShps %5d\r\n", top_shop, top_repair );
   ch_printf( ch, "CurOq's %5d    CurCq's %5d\r\n", cur_qobjs, cur_qchars );
   ch_printf( ch, "Players %5d    Maxplrs %5d\r\n", num_descriptors, sysdata.maxplayers );
   ch_printf( ch, "MaxEver %5d    Topsn   %5d (%d)\r\n", sysdata.alltimemax, top_sn, MAX_SKILL );
   ch_printf( ch, "MaxEver time recorded at:   %s\r\n", sysdata.time_of_max );
   if( !str_cmp( arg, "check" ) )
   {
#ifdef HASHSTR
      send_to_char( check_hash( argument ), ch );
#else
      send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
      return;
   }
   if( !str_cmp( arg, "showhigh" ) )
   {
#ifdef HASHSTR
      show_high_hash( atoi( argument ) );
#else
      send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
      return;
   }
   if( argument[0] != '\0' )
      hash = atoi( argument );
   else
      hash = -1;
   if( !str_cmp( arg, "hash" ) )
   {
#ifdef HASHSTR
      ch_printf( ch, "Hash statistics:\r\n%s", hash_stats(  ) );
      if( hash != -1 )
         hash_dump( hash );
#else
      send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
   }
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
   switch ( number_bits( 2 ) )
   {
      case 0:
         number -= 1;
         break;
      case 3:
         number += 1;
         break;
   }

   return UMAX( 1, number );
}

/*
 * Generate a random number.
 */

int number_range( int from, int to )
{
   if( ( to - from ) < 1 )
      return from;
   return ( ( number_mm(  ) % ( to - from + 1 ) ) + from );
}



/*
 * Generate a percentile roll.
 * number_mm() % 100 only does 0-99, changed to do 1-100 -Shaddai
 */
int number_percent( void )
{
   return ( number_mm(  ) % 100 ) + 1;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
   int door;

   while( ( door = number_mm(  ) & ( 16 - 1 ) ) > 9 )
      ;

   return door;
/*    return number_mm() & 10; */
}



int number_bits( int width )
{
   return number_mm(  ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];

void init_mm(  )
{
   int *piState;
   int iState;

   piState = &rgiState[2];

   piState[-2] = 55 - 55;
   piState[-1] = 55 - 24;

   piState[0] = ( ( int )current_time ) & ( ( 1 << 30 ) - 1 );
   piState[1] = 1;
   for( iState = 2; iState < 55; iState++ )
   {
      piState[iState] = ( piState[iState - 1] + piState[iState - 2] ) & ( ( 1 << 30 ) - 1 );
   }
   return;
}



int number_mm( void )
{
   int *piState;
   int iState1;
   int iState2;
   int iRand;

   piState = &rgiState[2];
   iState1 = piState[-2];
   iState2 = piState[-1];
   iRand = ( piState[iState1] + piState[iState2] ) & ( ( 1 << 30 ) - 1 );
   piState[iState1] = iRand;
   if( ++iState1 == 55 )
      iState1 = 0;
   if( ++iState2 == 55 )
      iState2 = 0;
   piState[-2] = iState1;
   piState[-1] = iState2;
   return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
   int idice;
   int sum;

   switch ( size )
   {
      case 0:
         return 0;
      case 1:
         return number;
   }

   for( idice = 0, sum = 0; idice < number; idice++ )
      sum += number_range( 1, size );

   return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
   return value_00 + level * ( value_32 - value_00 ) / 32;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
   for( ; *str != '\0'; str++ )
      if( *str == '~' )
         *str = '-';

   return;
}

const char *smash_tilde( const char *str )
{
   static char buf[MAX_STRING_LENGTH];
   snprintf( buf, MSL, "%s", str );
   smash_tilde( buf );
   return buf;
}

char *smash_tilde_copy( const char *str )
{
   char *result = strdup( str );
   smash_tilde( result );
   return result;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
   for( ; *str != '\0'; str++ )
      if( *str == '~' )
         *str = HIDDEN_TILDE;

   return;
}

char *show_tilde( const char *str )
{
   static char buf[MAX_STRING_LENGTH];
   char *bufptr;

   bufptr = buf;
   for( ; *str != '\0'; str++, bufptr++ )
   {
      if( *str == HIDDEN_TILDE )
         *bufptr = '~';
      else
         *bufptr = *str;
   }
   *bufptr = '\0';

   return buf;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
   if( !astr )
   {
      bug( "%s: null astr.", __func__ );
      if( bstr )
         return TRUE;
   }

   if( !bstr )
   {
      bug( "%s: null bstr.", __func__ );
      if( astr )
         return TRUE;
   }

   for( ; *astr || *bstr; astr++, bstr++ )
   {
      if( LOWER( *astr ) != LOWER( *bstr ) )
         return TRUE;
   }

   return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
   if( !astr )
   {
      bug( "%s: null astr.", __func__ );
      return TRUE;
   }

   if( !bstr )
   {
      bug( "%s: null bstr.", __func__ );
      return TRUE;
   }

   for( ; *astr; astr++, bstr++ )
   {
      if( LOWER( *astr ) != LOWER( *bstr ) )
         return TRUE;
   }

   return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
   int sstr1;
   int sstr2;
   int ichar;
   char c0;

   if( ( c0 = LOWER( astr[0] ) ) == '\0' )
      return FALSE;

   sstr1 = strlen( astr );
   sstr2 = strlen( bstr );

   for( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
      if( c0 == LOWER( bstr[ichar] ) && !str_prefix( astr, bstr + ichar ) )
         return FALSE;

   return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
   int sstr1;
   int sstr2;

   sstr1 = strlen( astr );
   sstr2 = strlen( bstr );
   if( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
      return FALSE;
   else
      return TRUE;
}

/*
 * Returns an initial-capped string.
 * Rewritten by FearItself@AvP
 */
char *capitalize( const char *str )
{
   static char buf[MAX_STRING_LENGTH];
   char *dest = buf;
   enum
   { Normal, Color } state = Normal;
   bool bFirst = TRUE;
   char c;

   while( ( c = *str++ ) )
   {
      if( state == Normal )
      {
         if( c == '&' || c == '^' || c == '}' )
         {
            state = Color;
         }
         else if( isalpha( c ) )
         {
            c = bFirst ? toupper( c ) : tolower( c );
            bFirst = FALSE;
         }
      }
      else
      {
         state = Normal;
      }
      *dest++ = c;
   }
   *dest = c;

   return buf;
}

/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
   static char strlow[MAX_STRING_LENGTH];
   int i;

   for( i = 0; str[i] != '\0'; i++ )
      strlow[i] = LOWER( str[i] );
   strlow[i] = '\0';
   return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
   static char strup[MAX_STRING_LENGTH];
   int i;

   for( i = 0; str[i] != '\0'; i++ )
      strup[i] = UPPER( str[i] );
   strup[i] = '\0';
   return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel			-Thoric
 */
bool isavowel( char letter )
{
   char c;

   c = tolower( letter );
   if( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
      return TRUE;
   else
      return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
const char *aoran( const char *str )
{
   static char temp[MAX_STRING_LENGTH];

   if( !str )
   {
      bug( "%s: NULL str", __func__ );
      return "";
   }

   if( isavowel( str[0] ) || ( strlen( str ) > 1 && tolower( str[0] ) == 'y' && !isavowel( str[1] ) ) )
      strlcpy( temp, "an ", MAX_STRING_LENGTH );
   else
      strlcpy( temp, "a ", MAX_STRING_LENGTH );
   strlcat( temp, str, MAX_STRING_LENGTH );
   return temp;
}

/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA * ch, const char *file, const char *str )
{
   FILE *fp;

   if( IS_NPC( ch ) || str[0] == '\0' )
      return;

   if( ( fp = fopen( file, "a" ) ) == NULL )
   {
      send_to_char( "Could not open the file!\r\n", ch );
   }
   else
   {
      fprintf( fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
      FCLOSE( fp );
   }
   return;
}

/*
 * Append a string to a file.
 */
void append_to_file( const char *file, const char *str )
{
   FILE *fp;

   if( ( fp = fopen( file, "a" ) ) == NULL )
   {
   }
   else
   {
      fprintf( fp, "%s\n", str );
      FCLOSE( fp );
   }

   return;
}

/*
 * Prepend a string to a file.
 */
void prepend_to_file( const char *file, const char *str )
{
   char temp[MAX_STRING_LENGTH];
   char tempname[256];

   snprintf( tempname, 256, "%s.temp", file );
   rename( file, tempname );
   append_to_file( file, str );
   snprintf( temp, MAX_STRING_LENGTH, "cat %s >> %s", tempname, file );
   system( temp );
   remove( tempname );
}

/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
   char buf[MAX_STRING_LENGTH];
   FILE *fp;
   struct stat fst;

   strlcpy( buf, "[*****] BUG: ", MAX_STRING_LENGTH );
   {
      va_list param;

      va_start( param, str );
      vsnprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), str, param );
      va_end( param );
   }
   log_string( buf );

   if( fpArea != NULL )
   {
      int iLine;
      int iChar;

      if( fpArea == stdin )
      {
         iLine = 0;
      }
      else
      {
         iChar = ftell( fpArea );
         fseek( fpArea, 0, 0 );
         for( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
         {
            int letter;

            while( ( letter = getc( fpArea ) ) && letter != EOF && letter != '\n' )
               ;
         }
         fseek( fpArea, iChar, 0 );
      }

      log_printf_plus( LOG_COMM, sysdata.log_level, "[*****] FILE: %s LINE: %d", strArea, iLine );

      if( stat( SHUTDOWN_FILE, &fst ) != -1 )   /* file exists */
      {
         if( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
         {
            fprintf( fp, "%s\n", buf );
            fprintf( fp, "[*****] FILE: %s LINE: %d\n", strArea, iLine );
            FCLOSE( fp );
         }
      }
   }
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log( const char *str, ... )
{
   char buf[MAX_STRING_LENGTH];
   FILE *fp;
   va_list param;

   strlcpy( buf, "[*****] BOOT: ", MAX_STRING_LENGTH );
   va_start( param, str );
   vsprintf( buf + strlen( buf ), str, param );
   va_end( param );
   log_string( buf );

   if( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
   {
      fprintf( fp, "%s\n", buf );
      FCLOSE( fp );
   }
   return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA * ch, const char *filename )
{
   FILE *fp;
   char buf[MAX_STRING_LENGTH];
   int c;
   int num = 0;

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      while( !feof( fp ) )
      {
         while( ( buf[num] = fgetc( fp ) ) != EOF
                && buf[num] != '\n' && buf[num] != '\r' && num < ( MAX_STRING_LENGTH - 2 ) )
            num++;
         c = fgetc( fp );
         if( ( c != '\n' && c != '\r' ) || c == buf[num] )
            ungetc( c, fp );
         buf[num++] = '\n';
         buf[num++] = '\r';
         buf[num] = '\0';
         send_to_pager( buf, ch );
         num = 0;
      }
      FCLOSE( fp );
   }
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg( CHAR_DATA * ch, const char *argument )
{
   set_pager_color( AT_LOG, ch );
   show_file( ch, BOOTLOG_FILE );
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, short log_type, short level )
{
   char *strtime;
   int offset;

   strtime = ctime( &current_time );
   strtime[strlen( strtime ) - 1] = '\0';
   fprintf( stderr, "%s :: %s\n", strtime, str );
   if( strncmp( str, "Log ", 4 ) == 0 )
      offset = 4;
   else
      offset = 0;
   switch ( log_type )
   {
      default:
         to_channel( str + offset, CHANNEL_LOG, "Log", level );
         break;
      case LOG_BUILD:
         to_channel( str + offset, CHANNEL_BUILD, "Build", level );
         break;
      case LOG_COMM:
         to_channel( str + offset, CHANNEL_COMM, "Comm", level );
         break;
      case LOG_ALL:
         break;
   }
}

/*
 * wizlist builder!						-Thoric
 */
void towizfile( const char *line, bool Border )
{
   int filler, xx, ofiller;
   char outline[MAX_STRING_LENGTH-30];
   char outline2[MAX_STRING_LENGTH];
   FILE *wfp;
   FILE *wwwfp;
   bool SNIP = FALSE;

   outline[0] = '\0';
   outline2[0] = '\0';
   if( line && line[0] != '\0' )
   {
      filler = ( 78 - strlen( line ) );
      if( filler < 1 )
         filler = 1;
      ofiller = filler;
      if( Border && filler % 2 == 0 )
         SNIP = TRUE;
      filler /= 2;
      if( SNIP )
         filler--;
      if( Border )
         filler -= 10;
      for( xx = 0; xx < filler; xx++ )
         strlcat( outline, " ", MAX_STRING_LENGTH-30 );
      strlcat( outline, line, MAX_STRING_LENGTH-30 );
   }
   if( Border )
   {
      if( ( filler * 2 ) != ofiller )
         filler++;
      if( SNIP )
         filler++;
      snprintf( outline2, MAX_STRING_LENGTH, "         &C| |&R%s&C", outline );
      for( xx = 0; xx < filler; xx++ )
         strlcat( outline2, " ", MAX_STRING_LENGTH );
      strlcat( outline2, "&C| |", MAX_STRING_LENGTH );
   }
   else
      strlcpy( outline2, outline, MAX_STRING_LENGTH );
   strlcat( outline2, "\r\n", MAX_STRING_LENGTH );
   wfp = fopen( WIZLIST_FILE, "a" );
   if( wfp )
   {
      fputs( outline2, wfp );
      FCLOSE( wfp );
   }
   wwwfp = fopen( WEBWIZLIST_FILE, "a" );
   if( wwwfp )
   {
      fputs( htmlcolor( outline2 ), wwwfp );
      FCLOSE( wwwfp );
   }
}

void add_to_wizlist( char *name, int level )
{
   WIZENT *wiz, *tmp;

#ifdef DEBUG
   log_string( "Adding to wizlist..." );
#endif

   CREATE( wiz, WIZENT, 1 );
   wiz->name = strdup( name );
   wiz->level = level;

   if( !first_wiz )
   {
      wiz->last = NULL;
      wiz->next = NULL;
      first_wiz = wiz;
      last_wiz = wiz;
      return;
   }

   /*
    * insert sort, of sorts 
    */
   for( tmp = first_wiz; tmp; tmp = tmp->next )
      if( level > tmp->level )
      {
         if( !tmp->last )
            first_wiz = wiz;
         else
            tmp->last->next = wiz;
         wiz->last = tmp->last;
         wiz->next = tmp;
         tmp->last = wiz;
         return;
      }

   wiz->last = last_wiz;
   wiz->next = NULL;
   last_wiz->next = wiz;
   last_wiz = wiz;
   return;
}

/*
 * Wizlist builder						-Thoric
 */
void make_wizlist(  )
{
   DIR *dp;
   struct dirent *dentry;
   FILE *gfp;
   const char *word;
   int ilevel, iflags;
   WIZENT *wiz, *wiznext;
   char godfile[266];
   char buf[MAX_STRING_LENGTH];

   first_wiz = NULL;
   last_wiz = NULL;

   dp = opendir( GOD_DIR );

   ilevel = 0;
   dentry = readdir( dp );
   while( dentry )
   {
      if( dentry->d_name[0] != '.' )
      {
         snprintf( godfile, 266, "%s%s", GOD_DIR, dentry->d_name );
         gfp = fopen( godfile, "r" );
         if( gfp )
         {
            word = feof( gfp ) ? "End" : fread_word( gfp );
            ilevel = fread_number( gfp );
            fread_to_eol( gfp );
            word = feof( gfp ) ? "End" : fread_word( gfp );
            if( !str_cmp( word, "Pcflags" ) )
               iflags = fread_number( gfp );
            else
               iflags = 0;
            FCLOSE( gfp );
            if( IS_SET( iflags, PCFLAG_RETIRED ) )
               ilevel = MAX_LEVEL - 5;
            if( IS_SET( iflags, PCFLAG_GUEST ) )
               ilevel = MAX_LEVEL - 10;
            add_to_wizlist( dentry->d_name, ilevel );
         }
      }
      dentry = readdir( dp );
   }
   closedir( dp );

   buf[0] = '\0';
   unlink( WIZLIST_FILE );
   unlink( WEBWIZLIST_FILE );
   towizfile( " ", 0 );
   towizfile( "&B ________   ___   ____                  ", 0 );
   towizfile( "&B/ __   __| / _ \\ |  _ \\                ", 0 );
   towizfile( " &C           _____ &B> \\ | |   |  _  ||    / &C____________________________ ", 0 );
   towizfile( " &C          / ___ &B<__/ |_|   |_| |_||_|\\__> &C__________________________ \\ ", 0 );
   towizfile( " &C       / /                                                        \\ \\ ", 0 );
   towizfile( " &C        | |              &YFall of the Empire Immortals:&C               | | ", 0 );
   ilevel = MAX_LEVEL + 1;
   for( wiz = first_wiz; wiz; wiz = wiz->next )
   {
      if( wiz->level > MAX_LEVEL - 11 )
      {
         if( wiz->level < ilevel )
         {
            if( buf[0] )
            {
               towizfile( buf, 1 );
               buf[0] = '\0';
            }
            towizfile( " &C        | |&B                                                          &C| &C|", 0 );
            ilevel = wiz->level;
            switch ( ilevel )
            {

               case LEVEL_IMPLEMENTOR:
                  towizfile( " &C        | &C|                        &W&GThe Admin &C                        | &C|&C", 0 );
                  break;
               case LEVEL_SUB_IMPLEM:
                  towizfile( " &C        | &C|                        &W&GHead Imms    &C                     | &C| ", 0 );
                  break;
               case LEVEL_ASCENDANT:
                  towizfile( " &C        | &C|                     &W&GChief Builders  &C                     | &C| ", 0 );
                  break;
               case LEVEL_GREATER:
                  towizfile( " &C        | &C|                     &W&GThe Architects  &C                     | &C| ", 0 );
                  break;
               case LEVEL_LESSER:
                  towizfile( " &C        | &C|                     &W&GThe Apprentices  &C                    | &C| ", 0 );
                  break;
               case LEVEL_RETIRED:
                  towizfile( " &C        | &C|                       &W&GThe Retired   &C                     | &C| ", 0 );
                  break;
               default:
                  towizfile( " &C        | &C|                      &W&GThe Guest Imms   &C                   | &C| ", 0 );
                  break;
            }
         }
         if( strlen( buf ) + strlen( wiz->name ) > 76 )
         {
            towizfile( buf, 1 );
            buf[0] = '\0';
         }
         strlcat( buf, " ", MAX_STRING_LENGTH );
         strlcat( buf, wiz->name, MAX_STRING_LENGTH );
         if( strlen( buf ) > 70 )
         {
            towizfile( buf, 1 );
            buf[0] = '\0';
         }
      }
   }

   if( buf[0] )
   {
      towizfile( buf, 1 );
   }
   for( wiz = first_wiz; wiz; wiz = wiznext )
   {
      wiznext = wiz->next;
      DISPOSE( wiz->name );
      DISPOSE( wiz );
   }
   first_wiz = NULL;
   last_wiz = NULL;
   towizfile( "&C         | |                                                          | &C|", 0 );
   towizfile( "&C          \\ \\__________________________ &B _    _  ___   ____   ___&C ___/ /", 0 );
   towizfile( "&C           \\___________________________ &B| |  | |/ _ \\ |  _ \\ / __>&C ___/", 0 );
   towizfile( "                                      &B| |/\\| || _  ||    / > \\&W     LS", 0 );
   towizfile( "                              &B\\_/\\_/ |_||_||_|\\_\\|__/", 0 );
}

void do_makewizlist( CHAR_DATA * ch, const char *argument )
{
   make_wizlist(  );
}

/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type( char *name )
{
   if( !str_cmp( name, "in_file_prog" ) )
      return IN_FILE_PROG;
   if( !str_cmp( name, "act_prog" ) )
      return ACT_PROG;
   if( !str_cmp( name, "speech_prog" ) )
      return SPEECH_PROG;
   if( !str_cmp( name, "rand_prog" ) )
      return RAND_PROG;
   if( !str_cmp( name, "fight_prog" ) )
      return FIGHT_PROG;
   if( !str_cmp( name, "hitprcnt_prog" ) )
      return HITPRCNT_PROG;
   if( !str_cmp( name, "death_prog" ) )
      return DEATH_PROG;
   if( !str_cmp( name, "entry_prog" ) )
      return ENTRY_PROG;
   if( !str_cmp( name, "greet_prog" ) )
      return GREET_PROG;
   if( !str_cmp( name, "all_greet_prog" ) )
      return ALL_GREET_PROG;
   if( !str_cmp( name, "give_prog" ) )
      return GIVE_PROG;
   if( !str_cmp( name, "bribe_prog" ) )
      return BRIBE_PROG;
   if( !str_cmp( name, "time_prog" ) )
      return TIME_PROG;
   if( !str_cmp( name, "hour_prog" ) )
      return HOUR_PROG;
   if( !str_cmp( name, "wear_prog" ) )
      return WEAR_PROG;
   if( !str_cmp( name, "remove_prog" ) )
      return REMOVE_PROG;
   if( !str_cmp( name, "sac_prog" ) )
      return SAC_PROG;
   if( !str_cmp( name, "look_prog" ) )
      return LOOK_PROG;
   if( !str_cmp( name, "exa_prog" ) )
      return EXA_PROG;
   if( !str_cmp( name, "custom_prog" ) )
      return CUSTOM_PROG;
   if( !str_cmp( name, "get_prog" ) )
      return GET_PROG;
   if( !str_cmp( name, "drop_prog" ) )
      return DROP_PROG;
   if( !str_cmp( name, "damage_prog" ) )
      return DAMAGE_PROG;
   if( !str_cmp( name, "repair_prog" ) )
      return REPAIR_PROG;
   if( !str_cmp( name, "greet_prog" ) )
      return GREET_PROG;
   if( !str_cmp( name, "randiw_prog" ) )
      return RANDIW_PROG;
   if( !str_cmp( name, "speechiw_prog" ) )
      return SPEECHIW_PROG;
   if( !str_cmp( name, "pull_prog" ) )
      return PULL_PROG;
   if( !str_cmp( name, "push_prog" ) )
      return PUSH_PROG;
   if( !str_cmp( name, "sleep_prog" ) )
      return SLEEP_PROG;
   if( !str_cmp( name, "rest_prog" ) )
      return REST_PROG;
   if( !str_cmp( name, "rfight_prog" ) )
      return FIGHT_PROG;
   if( !str_cmp( name, "enter_prog" ) )
      return ENTRY_PROG;
   if( !str_cmp( name, "leave_prog" ) )
      return LEAVE_PROG;
   if( !str_cmp( name, "rdeath_prog" ) )
      return DEATH_PROG;
   if( !str_cmp( name, "script_prog" ) )
      return SCRIPT_PROG;
   if( !str_cmp( name, "use_prog" ) )
      return USE_PROG;
   return ( ERROR_PROG );
}

void mobprog_file_read( MOB_INDEX_DATA * mob, char *f )
{
   MPROG_DATA *mprg = NULL;
   char MUDProgfile[256];
   FILE *progfile;
   char letter;

   snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

   if( !( progfile = fopen( MUDProgfile, "r" ) ) )
   {
      bug( "%s: couldn't open mudprog file", __func__ );
      return;
   }

   for( ;; )
   {
      letter = fread_letter( progfile );

      if( letter == '|' )
         break;

      if( letter != '>' )
      {
         bug( "%s: MUDPROG char", __func__ );
         break;
      }

      CREATE( mprg, MPROG_DATA, 1 );
      mprg->type = mprog_name_to_type( fread_word( progfile ) );
      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "%s: mudprog file type error", __func__ );
            DISPOSE( mprg );
            continue;

         case IN_FILE_PROG:
            bug( "%s: Nested file programs are not allowed.", __func__ );
            DISPOSE( mprg );
            continue;

         default:
            mprg->arglist = fread_string( progfile );
            mprg->comlist = fread_string( progfile );
            mprg->fileprog = TRUE;
            SET_BIT( mob->progtypes, mprg->type );
            mprg->next = mob->mudprogs;
            mob->mudprogs = mprg;
            break;
      }
   }
   FCLOSE( progfile );
   return;
}

/* This procedure is responsible for reading any in_file MUDprograms.
 */
void mprog_read_programs( FILE * fp, MOB_INDEX_DATA * mob )
{
   MPROG_DATA *mprg;
   char letter;
   char *word;

   for( ;; )
   {
      letter = fread_letter( fp );

      if( letter == '|' )
         return;

      if( letter != '>' )
      {
         bug( "%s: vnum %d MUDPROG char", __func__, mob->vnum );
         exit( 1 );
      }
      CREATE( mprg, MPROG_DATA, 1 );
      mprg->next = mob->mudprogs;
      mob->mudprogs = mprg;

      word = fread_word( fp );
      mprg->type = mprog_name_to_type( word );

      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "%s: vnum %d MUDPROG type.", __func__, mob->vnum );
            exit( 1 );

         case IN_FILE_PROG:
            mprg->arglist = fread_string( fp );
            mprg->fileprog = FALSE;
            mobprog_file_read( mob, mprg->arglist );
            break;

         default:
            SET_BIT( mob->progtypes, mprg->type );
            mprg->fileprog = FALSE;
            mprg->arglist = fread_string( fp );
            mprg->comlist = fread_string( fp );
            break;
      }
   }
   return;
}

/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
void objprog_file_read( OBJ_INDEX_DATA * obj, char *f )
{
   MPROG_DATA *mprg = NULL;
   char MUDProgfile[256];
   FILE *progfile;
   char letter;

   snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

   if( !( progfile = fopen( MUDProgfile, "r" ) ) )
   {
      bug( "%s: couldn't open mudprog file", __func__ );
      return;
   }

   for( ;; )
   {
      letter = fread_letter( progfile );

      if( letter == '|' )
         break;

      if( letter != '>' )
      {
         bug( "%s: MUDPROG char", __func__ );
         break;
      }

      CREATE( mprg, MPROG_DATA, 1 );
      mprg->type = mprog_name_to_type( fread_word( progfile ) );
      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "%s: mudprog file type error", __func__ );
            DISPOSE( mprg );
            continue;

         case IN_FILE_PROG:
            bug( "%s: Nested file programs are not allowed.", __func__ );
            DISPOSE( mprg );
            continue;

         default:
            mprg->arglist = fread_string( progfile );
            mprg->comlist = fread_string( progfile );
            mprg->fileprog = TRUE;
            SET_BIT( obj->progtypes, mprg->type );
            mprg->next = obj->mudprogs;
            obj->mudprogs = mprg;
            break;
      }
   }
   FCLOSE( progfile );
   return;
}

/* This procedure is responsible for reading any in_file OBJprograms.
 */
void oprog_read_programs( FILE * fp, OBJ_INDEX_DATA * obj )
{
   MPROG_DATA *mprg;
   char letter;
   char *word;

   for( ;; )
   {
      letter = fread_letter( fp );

      if( letter == '|' )
         return;

      if( letter != '>' )
      {
         bug( "%s: vnum %d MUDPROG char", __func__, obj->vnum );
         exit( 1 );
      }
      CREATE( mprg, MPROG_DATA, 1 );
      mprg->next = obj->mudprogs;
      obj->mudprogs = mprg;

      word = fread_word( fp );
      mprg->type = mprog_name_to_type( word );

      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "%s: vnum %d MUDPROG type.", __func__, obj->vnum );
            exit( 1 );

         case IN_FILE_PROG:
            mprg->arglist = fread_string( fp );
            mprg->fileprog = FALSE;
            objprog_file_read( obj, mprg->arglist );
            break;

         default:
            SET_BIT( obj->progtypes, mprg->type );
            mprg->fileprog = FALSE;
            mprg->arglist = fread_string( fp );
            mprg->comlist = fread_string( fp );
            break;
      }
   }
   return;
}

/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
void roomprog_file_read( ROOM_INDEX_DATA * room, char *f )
{
   MPROG_DATA *mprg = NULL;
   char MUDProgfile[256];
   FILE *progfile;
   char letter;

   snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

   if( !( progfile = fopen( MUDProgfile, "r" ) ) )
   {
      bug( "%s: couldn't open mudprog file", __func__ );
      return;
   }

   for( ;; )
   {
      letter = fread_letter( progfile );

      if( letter == '|' )
         break;

      if( letter != '>' )
      {
         bug( "%s: MUDPROG char", __func__ );
         break;
      }

      CREATE( mprg, MPROG_DATA, 1 );
      mprg->type = mprog_name_to_type( fread_word( progfile ) );
      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "%s: mudprog file type error", __func__ );
            DISPOSE( mprg );
            continue;

         case IN_FILE_PROG:
            bug( "%s: Nested file programs are not allowed.", __func__ );
            DISPOSE( mprg );
            continue;

         default:
            mprg->arglist = fread_string( progfile );
            mprg->comlist = fread_string( progfile );
            mprg->fileprog = TRUE;
            SET_BIT( room->progtypes, mprg->type );
            mprg->next = room->mudprogs;
            room->mudprogs = mprg;
            break;
      }
   }
   FCLOSE( progfile );
   return;
}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */
void rprog_read_programs( FILE * fp, ROOM_INDEX_DATA * room )
{
   MPROG_DATA *mprg;
   char letter;
   char *word;

   for( ;; )
   {
      letter = fread_letter( fp );

      if( letter == '|' )
         return;

      if( letter != '>' )
      {
         bug( "%s: vnum %d MUDPROG char", __func__, room->vnum );
         exit( 1 );
      }
      CREATE( mprg, MPROG_DATA, 1 );
      mprg->next = room->mudprogs;
      room->mudprogs = mprg;

      word = fread_word( fp );
      mprg->type = mprog_name_to_type( word );

      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "%s: vnum %d MUDPROG type.", __func__, room->vnum );
            exit( 1 );

         case IN_FILE_PROG:
            mprg->arglist = fread_string( fp );
            mprg->fileprog = FALSE;
            roomprog_file_read( room, mprg->arglist );
            break;

         default:
            SET_BIT( room->progtypes, mprg->type );
            mprg->fileprog = FALSE;
            mprg->arglist = fread_string( fp );
            mprg->comlist = fread_string( fp );
            break;
      }
   }
   return;
}

/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
  Narn, May/96
  Don't ask me why they return bool.. :).. oh well.. -- Alty
  Don't ask me either, so I changed it to void. - Samson
*/
void delete_room( ROOM_INDEX_DATA * room )
{
   int hash;
   ROOM_INDEX_DATA *prev, *limbo = get_room_index( ROOM_VNUM_LIMBO );
   OBJ_DATA *o;
   CHAR_DATA *ch;
   EXTRA_DESCR_DATA *ed;
   EXIT_DATA *ex;
   MPROG_ACT_LIST *mpact;
   MPROG_DATA *mp;

   UNLINK( room, room->area->first_room, room->area->last_room, next_aroom, prev_aroom );

   while( ( ch = room->first_person ) != NULL )
   {
      if( !IS_NPC( ch ) )
      {
         char_from_room( ch );
         char_to_room( ch, limbo );
      }
      else
         extract_char( ch, TRUE );
   }

   for( ch = first_char; ch; ch = ch->next )
   {
      if( ch->was_in_room == room )
         ch->was_in_room = ch->in_room;
      if( ch->substate == SUB_ROOM_DESC && ch->dest_buf == room )
      {
         send_to_char( "The room is no more.\r\n", ch );
         stop_editing( ch );
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
      }
      else if( ch->substate == SUB_ROOM_EXTRA && ch->dest_buf )
      {
         for( ed = room->first_extradesc; ed; ed = ed->next )
         {
            if( ed == ch->dest_buf )
            {
               send_to_char( "The room is no more.\r\n", ch );
               stop_editing( ch );
               ch->substate = SUB_NONE;
               ch->dest_buf = NULL;
               break;
            }
         }
      }
   }

   while( ( o = room->first_content ) != NULL )
      extract_obj( o );

   wipe_resets( room );

   while( ( ed = room->first_extradesc ) != NULL )
   {
      room->first_extradesc = ed->next;
      STRFREE( ed->keyword );
      STRFREE( ed->description );
      DISPOSE( ed );
      --top_ed;
   }

   while( ( ex = room->first_exit ) != NULL )
      extract_exit( room, ex );

   while( ( mpact = room->mpact ) != NULL )
   {
      room->mpact = mpact->next;
      DISPOSE( mpact->buf );
      DISPOSE( mpact );
   }

   while( ( mp = room->mudprogs ) != NULL )
   {
      room->mudprogs = mp->next;
      STRFREE( mp->arglist );
      STRFREE( mp->comlist );
      DISPOSE( mp );
   }

   STRFREE( room->name );
   STRFREE( room->description );

   hash = room->vnum % MAX_KEY_HASH;
   if( room == room_index_hash[hash] )
      room_index_hash[hash] = room->next;
   else
   {
      for( prev = room_index_hash[hash]; prev; prev = prev->next )
         if( prev->next == room )
            break;
      if( prev )
         prev->next = room->next;
      else
         bug( "%s: room %d not in hash bucket %d.", __func__, room->vnum, hash );
   }
   DISPOSE( room );
   --top_room;
   return;
}

/* See comment on delete_room. */
void delete_obj( OBJ_INDEX_DATA * obj )
{
   int hash;
   OBJ_INDEX_DATA *prev;
   OBJ_DATA *o, *o_next;
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *af;
   MPROG_DATA *mp;
   CHAR_DATA *ch;

   /*
    * Remove references to object index 
    */
   for( o = first_object; o; o = o_next )
   {
      o_next = o->next;
      if( o->pIndexData == obj )
         extract_obj( o );
   }

   for( ch = first_char; ch; ch = ch->next )
   {
      if( ch->substate == SUB_OBJ_EXTRA && ch->dest_buf )
      {
         for( ed = obj->first_extradesc; ed; ed = ed->next )
         {
            if( ed == ch->dest_buf )
            {
               send_to_char( "You suddenly forget which object you were editing!\r\n", ch );
               stop_editing( ch );
               ch->substate = SUB_NONE;
               break;
            }
         }
      }
      else if( ch->substate == SUB_MPROG_EDIT && ch->dest_buf )
      {
         for( mp = obj->mudprogs; mp; mp = mp->next )
         {
            if( mp == ch->dest_buf )
            {
               send_to_char( "You suddenly forget which object you were working on.\r\n", ch );
               stop_editing( ch );
               ch->dest_buf = NULL;
               ch->substate = SUB_NONE;
               break;
            }
         }
      }
   }

   while( ( ed = obj->first_extradesc ) != NULL )
   {
      obj->first_extradesc = ed->next;
      STRFREE( ed->keyword );
      STRFREE( ed->description );
      DISPOSE( ed );
      --top_ed;
   }

   while( ( af = obj->first_affect ) != NULL )
   {
      obj->first_affect = af->next;
      DISPOSE( af );
      --top_affect;
   }

   while( ( mp = obj->mudprogs ) != NULL )
   {
      obj->mudprogs = mp->next;
      STRFREE( mp->arglist );
      STRFREE( mp->comlist );
      DISPOSE( mp );
   }

   STRFREE( obj->name );
   STRFREE( obj->short_descr );
   STRFREE( obj->description );
   STRFREE( obj->action_desc );

   hash = obj->vnum % MAX_KEY_HASH;
   if( obj == obj_index_hash[hash] )
      obj_index_hash[hash] = obj->next;
   else
   {
      for( prev = obj_index_hash[hash]; prev; prev = prev->next )
         if( prev->next == obj )
            break;
      if( prev )
         prev->next = obj->next;
      else
         bug( "%s: object %d not in hash bucket %d.", __func__, obj->vnum, hash );
   }
   DISPOSE( obj );
   --top_obj_index;
   return;
}

/* See comment on delete_room. */
void delete_mob( MOB_INDEX_DATA * mob )
{
   int hash;
   MOB_INDEX_DATA *prev;
   CHAR_DATA *ch, *ch_next;
   MPROG_DATA *mp;

   for( ch = first_char; ch; ch = ch_next )
   {
      ch_next = ch->next;

      if( ch->pIndexData == mob )
         extract_char( ch, TRUE );
      else if( ch->substate == SUB_MPROG_EDIT && ch->dest_buf )
      {
         for( mp = mob->mudprogs; mp; mp = mp->next )
         {
            if( mp == ch->dest_buf )
            {
               send_to_char( "Your victim has departed.\r\n", ch );
               stop_editing( ch );
               ch->dest_buf = NULL;
               ch->substate = SUB_NONE;
               break;
            }
         }
      }
   }

   while( ( mp = mob->mudprogs ) != NULL )
   {
      mob->mudprogs = mp->next;
      STRFREE( mp->arglist );
      STRFREE( mp->comlist );
      DISPOSE( mp );
   }

   if( mob->pShop )
   {
      UNLINK( mob->pShop, first_shop, last_shop, next, prev );
      DISPOSE( mob->pShop );
      --top_shop;
   }

   if( mob->rShop )
   {
      UNLINK( mob->rShop, first_repair, last_repair, next, prev );
      DISPOSE( mob->rShop );
      --top_repair;
   }

   STRFREE( mob->player_name );
   STRFREE( mob->short_descr );
   STRFREE( mob->long_descr );
   STRFREE( mob->description );
   STRFREE( mob->spec_funname );
   STRFREE( mob->spec_funname2 );

   hash = mob->vnum % MAX_KEY_HASH;
   if( mob == mob_index_hash[hash] )
      mob_index_hash[hash] = mob->next;
   else
   {
      for( prev = mob_index_hash[hash]; prev; prev = prev->next )
         if( prev->next == mob )
            break;
      if( prev )
         prev->next = mob->next;
      else
         bug( "%s: mobile %d not in hash bucket %d.", __func__, mob->vnum, hash );
   }
   DISPOSE( mob );
   --top_mob_index;
   return;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( int vnum, AREA_DATA * area )
{
   ROOM_INDEX_DATA *pRoomIndex;
   int iHash;

   CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
   pRoomIndex->first_person = NULL;
   pRoomIndex->last_person = NULL;
   pRoomIndex->first_content = NULL;
   pRoomIndex->last_content = NULL;
   pRoomIndex->first_reset = pRoomIndex->last_reset = NULL;
   pRoomIndex->first_extradesc = NULL;
   pRoomIndex->last_extradesc = NULL;
   pRoomIndex->first_ship = NULL;
   pRoomIndex->last_ship = NULL;
   pRoomIndex->area = area;
   pRoomIndex->vnum = vnum;
   pRoomIndex->name = STRALLOC( "Floating in a void" );
   pRoomIndex->description = STRALLOC( "" );
   pRoomIndex->room_flags = ROOM_PROTOTYPE;
   pRoomIndex->sector_type = 1;
   pRoomIndex->light = 0;
   pRoomIndex->first_exit = NULL;
   pRoomIndex->last_exit = NULL;
   LINK( pRoomIndex, area->first_room, area->last_room, next_aroom, prev_aroom );

   iHash = vnum % MAX_KEY_HASH;
   pRoomIndex->next = room_index_hash[iHash];
   room_index_hash[iHash] = pRoomIndex;
   top_room++;

   return pRoomIndex;
}



/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, const char *name )
{
   OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
   char buf[MAX_STRING_LENGTH];
   int iHash;

   if( cvnum > 0 )
      cObjIndex = get_obj_index( cvnum );
   else
      cObjIndex = NULL;
   CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
   pObjIndex->vnum = vnum;
   pObjIndex->name = STRALLOC( name );
   pObjIndex->first_affect = NULL;
   pObjIndex->last_affect = NULL;
   pObjIndex->first_extradesc = NULL;
   pObjIndex->last_extradesc = NULL;
   if( !cObjIndex )
   {
      snprintf( buf, MAX_STRING_LENGTH, "A %s", name );
      pObjIndex->short_descr = STRALLOC( buf );
      snprintf( buf, MAX_STRING_LENGTH, "A %s is here.", name );
      pObjIndex->description = STRALLOC( buf );
      pObjIndex->action_desc = STRALLOC( "" );
      pObjIndex->short_descr[0] = LOWER( pObjIndex->short_descr[0] );
      pObjIndex->description[0] = UPPER( pObjIndex->description[0] );
      pObjIndex->item_type = ITEM_TRASH;
      pObjIndex->extra_flags = ITEM_PROTOTYPE;
      pObjIndex->wear_flags = 0;
      pObjIndex->value[0] = 0;
      pObjIndex->value[1] = 0;
      pObjIndex->value[2] = 0;
      pObjIndex->value[3] = 0;
      pObjIndex->weight = 1;
      pObjIndex->cost = 0;
   }
   else
   {
      EXTRA_DESCR_DATA *ed, *ced;
      AFFECT_DATA *paf, *cpaf;

      pObjIndex->short_descr = QUICKLINK( cObjIndex->short_descr );
      pObjIndex->description = QUICKLINK( cObjIndex->description );
      pObjIndex->action_desc = QUICKLINK( cObjIndex->action_desc );
      pObjIndex->item_type = cObjIndex->item_type;
      pObjIndex->extra_flags = cObjIndex->extra_flags | ITEM_PROTOTYPE;
      pObjIndex->wear_flags = cObjIndex->wear_flags;
      pObjIndex->value[0] = cObjIndex->value[0];
      pObjIndex->value[1] = cObjIndex->value[1];
      pObjIndex->value[2] = cObjIndex->value[2];
      pObjIndex->value[3] = cObjIndex->value[3];
      pObjIndex->weight = cObjIndex->weight;
      pObjIndex->cost = cObjIndex->cost;
      for( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
      {
         CREATE( ed, EXTRA_DESCR_DATA, 1 );
         ed->keyword = QUICKLINK( ced->keyword );
         ed->description = QUICKLINK( ced->description );
         LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
         top_ed++;
      }
      for( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = cpaf->type;
         paf->duration = cpaf->duration;
         paf->location = cpaf->location;
         paf->modifier = cpaf->modifier;
         paf->bitvector = cpaf->bitvector;
         LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
         top_affect++;
      }
   }
   pObjIndex->count = 0;
   iHash = vnum % MAX_KEY_HASH;
   pObjIndex->next = obj_index_hash[iHash];
   obj_index_hash[iHash] = pObjIndex;
   top_obj_index++;

   return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( int vnum, int cvnum, const char *name )
{
   MOB_INDEX_DATA *pMobIndex, *cMobIndex;
   char buf[MAX_STRING_LENGTH];
   int iHash;

   if( cvnum > 0 )
      cMobIndex = get_mob_index( cvnum );
   else
      cMobIndex = NULL;
   CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
   pMobIndex->vnum = vnum;
   pMobIndex->count = 0;
   pMobIndex->killed = 0;
   pMobIndex->player_name = STRALLOC( name );
   if( !cMobIndex )
   {
      snprintf( buf, MAX_STRING_LENGTH, "A newly created %s", name );
      pMobIndex->short_descr = STRALLOC( buf );
      snprintf( buf, MAX_STRING_LENGTH, "Some god abandoned a newly created %s here.\r\n", name );
      pMobIndex->long_descr = STRALLOC( buf );
      pMobIndex->description = STRALLOC( "" );
      pMobIndex->short_descr[0] = LOWER( pMobIndex->short_descr[0] );
      pMobIndex->long_descr[0] = UPPER( pMobIndex->long_descr[0] );
      pMobIndex->description[0] = UPPER( pMobIndex->description[0] );
      pMobIndex->act = ACT_IS_NPC | ACT_PROTOTYPE;
      pMobIndex->affected_by = 0;
      pMobIndex->pShop = NULL;
      pMobIndex->rShop = NULL;
      pMobIndex->spec_fun = NULL;
      pMobIndex->spec_2 = NULL;
      pMobIndex->mudprogs = NULL;
      pMobIndex->progtypes = 0;
      pMobIndex->alignment = 0;
      pMobIndex->level = 1;
      pMobIndex->mobthac0 = 0;
      pMobIndex->ac = 0;
      pMobIndex->hitnodice = 0;
      pMobIndex->hitsizedice = 0;
      pMobIndex->hitplus = 0;
      pMobIndex->damnodice = 0;
      pMobIndex->damsizedice = 0;
      pMobIndex->damplus = 0;
      pMobIndex->gold = 0;
      pMobIndex->exp = 0;
      pMobIndex->position = POS_STANDING;
      pMobIndex->defposition = POS_STANDING;
      pMobIndex->sex = 0;
      pMobIndex->perm_str = 10;
      pMobIndex->perm_dex = 10;
      pMobIndex->perm_int = 10;
      pMobIndex->perm_wis = 10;
      pMobIndex->perm_cha = 10;
      pMobIndex->perm_con = 10;
      pMobIndex->perm_lck = 10;
      pMobIndex->race = 0;
      pMobIndex->xflags = 0;
      pMobIndex->resistant = 0;
      pMobIndex->immune = 0;
      pMobIndex->susceptible = 0;
      pMobIndex->numattacks = 1;
      pMobIndex->attacks = 0;
      pMobIndex->defenses = 0;
   }
   else
   {
      pMobIndex->short_descr = QUICKLINK( cMobIndex->short_descr );
      pMobIndex->long_descr = QUICKLINK( cMobIndex->long_descr );
      pMobIndex->description = QUICKLINK( cMobIndex->description );
      pMobIndex->act = cMobIndex->act | ACT_PROTOTYPE;
      pMobIndex->affected_by = cMobIndex->affected_by;
      pMobIndex->pShop = NULL;
      pMobIndex->rShop = NULL;
      pMobIndex->spec_fun = cMobIndex->spec_fun;
      pMobIndex->spec_2 = cMobIndex->spec_2;
      pMobIndex->mudprogs = NULL;
      pMobIndex->progtypes = 0;
      pMobIndex->alignment = cMobIndex->alignment;
      pMobIndex->level = cMobIndex->level;
      pMobIndex->mobthac0 = cMobIndex->mobthac0;
      pMobIndex->ac = cMobIndex->ac;
      pMobIndex->hitnodice = cMobIndex->hitnodice;
      pMobIndex->hitsizedice = cMobIndex->hitsizedice;
      pMobIndex->hitplus = cMobIndex->hitplus;
      pMobIndex->damnodice = cMobIndex->damnodice;
      pMobIndex->damsizedice = cMobIndex->damsizedice;
      pMobIndex->damplus = cMobIndex->damplus;
      pMobIndex->gold = cMobIndex->gold;
      pMobIndex->exp = cMobIndex->exp;
      pMobIndex->position = cMobIndex->position;
      pMobIndex->defposition = cMobIndex->defposition;
      pMobIndex->sex = cMobIndex->sex;
      pMobIndex->perm_str = cMobIndex->perm_str;
      pMobIndex->perm_dex = cMobIndex->perm_dex;
      pMobIndex->perm_int = cMobIndex->perm_int;
      pMobIndex->perm_wis = cMobIndex->perm_wis;
      pMobIndex->perm_cha = cMobIndex->perm_cha;
      pMobIndex->perm_con = cMobIndex->perm_con;
      pMobIndex->perm_lck = cMobIndex->perm_lck;
      pMobIndex->race = cMobIndex->race;
      pMobIndex->xflags = cMobIndex->xflags;
      pMobIndex->resistant = cMobIndex->resistant;
      pMobIndex->immune = cMobIndex->immune;
      pMobIndex->susceptible = cMobIndex->susceptible;
      pMobIndex->numattacks = cMobIndex->numattacks;
      pMobIndex->attacks = cMobIndex->attacks;
      pMobIndex->defenses = cMobIndex->defenses;
   }
   iHash = vnum % MAX_KEY_HASH;
   pMobIndex->next = mob_index_hash[iHash];
   mob_index_hash[iHash] = pMobIndex;
   top_mob_index++;

   return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA * pRoomIndex, ROOM_INDEX_DATA * to_room, short door )
{
   EXIT_DATA *pexit, *texit;
   bool broke;

   CREATE( pexit, EXIT_DATA, 1 );
   pexit->vdir = door;
   pexit->rvnum = pRoomIndex->vnum;
   pexit->to_room = to_room;
   pexit->distance = 1;
   if( to_room )
   {
      pexit->vnum = to_room->vnum;
      texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );
      if( texit ) /* assign reverse exit pointers */
      {
         texit->rexit = pexit;
         pexit->rexit = texit;
      }
   }
   broke = FALSE;
   for( texit = pRoomIndex->first_exit; texit; texit = texit->next )
      if( door < texit->vdir )
      {
         broke = TRUE;
         break;
      }
   if( !pRoomIndex->first_exit )
      pRoomIndex->first_exit = pexit;
   else
   {
      /*
       * keep exits in incremental order - insert exit into list 
       */
      if( broke && texit )
      {
         if( !texit->prev )
            pRoomIndex->first_exit = pexit;
         else
            texit->prev->next = pexit;
         pexit->prev = texit->prev;
         pexit->next = texit;
         texit->prev = pexit;
         top_exit++;
         return pexit;
      }
      pRoomIndex->last_exit->next = pexit;
   }
   pexit->next = NULL;
   pexit->prev = pRoomIndex->last_exit;
   pRoomIndex->last_exit = pexit;
   top_exit++;
   return pexit;
}

void fix_area_exits( AREA_DATA * tarea )
{
   ROOM_INDEX_DATA *pRoomIndex;
   EXIT_DATA *pexit, *rev_exit;
   int rnum;
   bool fexit;

   for( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
   {
      if( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
         continue;

      fexit = FALSE;
      for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
      {
         fexit = TRUE;
         pexit->rvnum = pRoomIndex->vnum;
         if( pexit->vnum <= 0 )
            pexit->to_room = NULL;
         else
            pexit->to_room = get_room_index( pexit->vnum );
      }
      if( !fexit )
         SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
   }


   for( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
   {
      if( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
         continue;

      for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
      {
         if( pexit->to_room && !pexit->rexit )
         {
            rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
            if( rev_exit )
            {
               pexit->rexit = rev_exit;
               rev_exit->rexit = pexit;
            }
         }
      }
   }
}

void process_sorting( AREA_DATA * tarea )
{
   if( fBootDb )
   {
      sort_area_by_name( tarea );   /* 4/27/97 */
      sort_area( tarea, false );
   }
   fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d Mobs: %5d - %d\n",
            tarea->filename,
            tarea->low_r_vnum, tarea->hi_r_vnum, tarea->low_o_vnum, tarea->hi_o_vnum, tarea->low_m_vnum, tarea->hi_m_vnum );
   if( !tarea->author )
      tarea->author = STRALLOC( "" );
   SET_BIT( tarea->status, AREA_LOADED );
}

EXTRA_DESCR_DATA *fread_fuss_exdesc( FILE * fp )
{
   EXTRA_DESCR_DATA *ed;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   CREATE( ed, EXTRA_DESCR_DATA, 1 );

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDEXDESC" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDEXDESC";
      }

      switch ( word[0] )
      {
         default:
            log_printf( "%s: no match: %s", __FUNCTION__, word );
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#ENDEXDESC" ) )
            {
               if( !ed->keyword )
               {
                  bug( "%s: Missing ExDesc keyword. Returning NULL.", __func__ );
                  STRFREE( ed->description );
                  DISPOSE( ed );
                  return NULL;
               }

               if( !ed->description )
                  ed->description = STRALLOC( "" );

               return ed;
            }
            break;

         case 'E':
            KEY( "ExDescKey", ed->keyword, fread_string( fp ) );
            KEY( "ExDesc", ed->description, fread_string( fp ) );
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }

   // Reach this point, you fell through somehow. The data is no longer valid.
   bug( "%s: Reached fallout point! ExtraDesc data invalid.", __func__ );
   DISPOSE( ed );
   return NULL;
}

AFFECT_DATA *fread_fuss_affect( FILE * fp, const char *word )
{
   AFFECT_DATA *paf;
   int pafmod;

   CREATE( paf, AFFECT_DATA, 1 );
   if( !strcmp( word, "Affect" ) )
   {
      paf->type = fread_number( fp );
   }
   else
   {
      int sn;

      sn = skill_lookup( fread_word( fp ) );
      if( sn < 0 )
         bug( "%s: unknown skill.", __func__ );
      else
         paf->type = sn;
   }
   paf->duration = fread_number( fp );
   pafmod = fread_number( fp );
   paf->location = fread_number( fp );
   paf->bitvector = fread_number( fp );

   if( paf->location == APPLY_WEAPONSPELL
       || paf->location == APPLY_WEARSPELL
       || paf->location == APPLY_STRIPSN || paf->location == APPLY_REMOVESPELL )
      paf->modifier = slot_lookup( pafmod );
   else
      paf->modifier = pafmod;

   ++top_affect;
   return paf;
}

void fread_fuss_exit( FILE * fp, ROOM_INDEX_DATA * pRoomIndex )
{
   EXIT_DATA *pexit = NULL;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDEXIT" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDEXIT";
      }

      switch ( word[0] )
      {
         default:
            log_printf( "%s: no match: %s", __FUNCTION__, word );
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#ENDEXIT" ) )
            {
               if( !pexit->description )
                  pexit->description = STRALLOC( "" );
               if( !pexit->keyword )
                  pexit->keyword = STRALLOC( "" );

               return;
            }
            break;

         case 'D':
            KEY( "Desc", pexit->description, fread_string( fp ) );
            KEY( "Distance", pexit->distance, fread_number( fp ) );
            if( !str_cmp( word, "Direction" ) )
            {
               int door = get_dir( fread_flagstring( fp ) );

               if( door < 0 || door > DIR_SOMEWHERE )
               {
                  bug( "%s: vnum %d has bad door number %d.", __func__, pRoomIndex->vnum, door );
                  if( fBootDb )
                     return;
               }
               pexit = make_exit( pRoomIndex, NULL, door );
            }
            break;

         case 'F':
            if( !str_cmp( word, "Flags" ) )
            {
               const char *exitflags = NULL;
               char flag[MAX_INPUT_LENGTH];
               int value;

               exitflags = fread_flagstring( fp );

               while( exitflags[0] != '\0' )
               {
                  exitflags = one_argument( exitflags, flag );
                  value = get_exflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown exitflag: %s", __func__, flag );
                  else
                     SET_BIT( pexit->exit_info, 1 << value );
               }
               break;
            }
            break;

         case 'K':
            KEY( "Key", pexit->key, fread_number( fp ) );
            KEY( "Keywords", pexit->keyword, fread_string( fp ) );
            break;

         case 'T':
            KEY( "ToRoom", pexit->vnum, fread_number( fp ) );
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }

   // Reach this point, you fell through somehow. The data is no longer valid.
   bug( "%s: Reached fallout point! Exit data invalid.", __func__ );
   if( pexit )
      extract_exit( pRoomIndex, pexit );
   return;
}

void rprog_file_read( ROOM_INDEX_DATA * prog_target, const char *f )
{
   MPROG_DATA *mprg = NULL;
   char MUDProgfile[256];
   FILE *progfile;
   char letter;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

   if( !( progfile = fopen( MUDProgfile, "r" ) ) )
   {
      bug( "%s: couldn't open mudprog file", __func__ );
      return;
   }

   for( ;; )
   {
      letter = fread_letter( progfile );

      if( letter != '#' )
      {
         bug( "%s: MUDPROG char", __func__ );
         break;
      }

      const char *word = ( feof( progfile ) ? "ENDFILE" : fread_word( progfile ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __func__ );
         word = "ENDFILE";
      }

      if( !str_cmp( word, "ENDFILE" ) )
         break;

      if( !str_cmp( word, "MUDPROG" ) )
      {
         CREATE( mprg, MPROG_DATA, 1 );

         for( ;; )
         {
            word = ( feof( progfile ) ? "#ENDPROG" : fread_word( progfile ) );

            if( word[0] == '\0' )
            {
               log_printf( "%s: EOF encountered reading file!", __func__ );
               word = "#ENDPROG";
            }

            if( !str_cmp( word, "#ENDPROG" ) )
            {
               mprg->next = prog_target->mudprogs;
               prog_target->mudprogs = mprg;
               break;
            }

            switch ( word[0] )
            {
               default:
                  log_printf( "%s: no match: %s", __func__, word );
                  fread_to_eol( progfile );
                  break;

               case 'A':
                  if( !str_cmp( word, "Arglist" ) )
                  {
                     mprg->arglist = fread_string( progfile );
                     mprg->fileprog = true;

                     switch ( mprg->type )
                     {
                        case IN_FILE_PROG:
                           bug( "%s: Nested file programs are not allowed.", __func__ );
                           DISPOSE( mprg );
                           break;

                        default:
                           break;
                     }
                     break;
                  }
                  break;

               case 'C':
                  KEY( "Comlist", mprg->comlist, fread_string( progfile ) );
                  break;

               case 'P':
                  if( !str_cmp( word, "Progtype" ) )
                  {
                     mprg->type = mprog_name_to_type( fread_flagstring( progfile ) );
                     break;
                  }
                  break;
            }
            if( !fMatch )
            {
               fread_to_eol( progfile );
               continue;
            }
         }
      }
   }
   FCLOSE( progfile );
   return;
}

void fread_fuss_roomprog( FILE * fp, MPROG_DATA * mprg, ROOM_INDEX_DATA * prog_target )
{
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDPROG" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDPROG";
      }

      if( !str_cmp( word, "#ENDPROG" ) )
         return;

      switch ( word[0] )
      {
         default:
            log_printf( "%s: no match: %s", __FUNCTION__, word );
            fread_to_eol( fp );
            break;

         case 'A':
            if( !str_cmp( word, "Arglist" ) )
            {
               mprg->arglist = fread_string( fp );
               mprg->fileprog = false;

               switch ( mprg->type )
               {
                  case IN_FILE_PROG:
                     rprog_file_read( prog_target, mprg->arglist );
                     break;
                  default:
                     break;
               }
               break;
            }
            break;

         case 'C':
            KEY( "Comlist", mprg->comlist, fread_string( fp ) );
            break;

         case 'P':
            if( !str_cmp( word, "Progtype" ) )
            {
               mprg->type = mprog_name_to_type( fread_flagstring( fp ) );
               SET_BIT( prog_target->progtypes, mprg->type );
               break;
            }
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }
}

void fread_fuss_room( FILE * fp, AREA_DATA * tarea )
{
   ROOM_INDEX_DATA *pRoomIndex = NULL;
   bool oldroom = false;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDROOM" : fread_word( fp ) );
      char flag[MAX_INPUT_LENGTH];
      int value = 0;


      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __func__ );
         word = "#ENDROOM";
      }

      switch ( word[0] )
      {
         default:
            bug( "%s: no match: %s", __func__, word );
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#ENDROOM" ) )
            {
               if( !pRoomIndex->description )
                  pRoomIndex->description = STRALLOC( "" );

               if( !oldroom )
               {
                  int iHash = pRoomIndex->vnum % MAX_KEY_HASH;
                  pRoomIndex->next = room_index_hash[iHash];
                  room_index_hash[iHash] = pRoomIndex;
                  LINK( pRoomIndex, tarea->first_room, tarea->last_room, next_aroom, prev_aroom );
                  ++top_room;
               }
               return;
            }

            if( !str_cmp( word, "#EXIT" ) )
            {
               fread_fuss_exit( fp, pRoomIndex );
               break;
            }

            if( !str_cmp( word, "#EXDESC" ) )
            {
               EXTRA_DESCR_DATA *ed = fread_fuss_exdesc( fp );

               if( ed )
                  LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc, next, prev );
               break;
            }

            if( !str_cmp( word, "#MUDPROG" ) )
            {
               MPROG_DATA *mprg;

               CREATE( mprg, MPROG_DATA, 1 );
               fread_fuss_roomprog( fp, mprg, pRoomIndex );
               mprg->next = pRoomIndex->mudprogs;
               pRoomIndex->mudprogs = mprg;
               break;
            }
            break;

         /*case 'A':
            if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
            {
               AFFECT_DATA *af = fread_fuss_affect( fp, word );

               if( af )
                  LINK( af, pRoomIndex->first_permaffect, pRoomIndex->last_permaffect, next, prev );
               break;
            }
            break; */

         case 'D':
            KEY( "Desc", pRoomIndex->description, fread_string( fp ) );
            break;

         case 'F':
            if( !str_cmp( word, "Flags" ) )
            {
               const char *rflags = fread_flagstring( fp );

               while( rflags[0] != '\0' )
               {
                  rflags = one_argument( rflags, flag );
                  value = get_rflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown room flag: %s", __func__, flag );
                  else
                     SET_BIT( pRoomIndex->room_flags, 1 << value );
               }
               break;
            }
            if( !str_cmp( word, "Flags2" ) )
            {
               const char *rflags2 = fread_flagstring( fp );

               while( rflags2[0] != '\0' )
               {
                  rflags2 = one_argument( rflags2, flag );
                  value = get_rflag2( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown room flag2: %s", __func__, flag );
                  else
                     SET_BIT( pRoomIndex->room_flags2, 1 << value );
               }
               break;
            }
            break;

         case 'N':
            KEY( "Name", pRoomIndex->name, fread_string( fp ) );
            break;

         case 'R':
            if( !str_cmp( word, "Reset" ) )
            {
               load_room_reset( pRoomIndex, fp );
               break;
            }
            break;

         case 'S':
            if( !str_cmp( word, "Sector" ) )
            {
               int sector = get_secflag( fread_flagstring( fp ) );

               if( sector < 0 || sector >= SECT_MAX )
               {
                  bug( "%s: Room #%d has bad sector type.", __func__, pRoomIndex->vnum );
                  sector = 1;
               }

               pRoomIndex->sector_type = sector;
               break;
            }

            if( !str_cmp( word, "Stats" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3;

               x1 = x2 = x3 = 0;
               sscanf( ln, "%d %d %d", &x1, &x2, &x3 );

               pRoomIndex->tele_delay = x1;
               pRoomIndex->tele_vnum = x2;
               pRoomIndex->tunnel = x3;

               break;
            }
            break;

         case 'V':
            if( !str_cmp( word, "Vnum" ) )
            {
               bool tmpBootDb = fBootDb;
               fBootDb = false;

               int vnum = fread_number( fp );

               if( get_room_index( vnum ) )
               {
                  if( tmpBootDb )
                  {
                     fBootDb = tmpBootDb;
                     bug( "%s: vnum %d duplicated.", __func__, vnum );

                     // Try to recover, read to end of duplicated room and then bail out
                     for( ;; )
                     {
                        word = feof( fp ) ? "#ENDROOM" : fread_word( fp );

                        if( !str_cmp( word, "#ENDROOM" ) )
                           return;
                     }
                  }
                  else
                  {
                     pRoomIndex = get_room_index( vnum );
                     log_printf_plus( LOG_BUILD, sysdata.build_level, "Cleaning room: %d", vnum );
                     clean_room( pRoomIndex );
                     oldroom = true;
                  }
               }
               else
               {
                  CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
                  oldroom = false;
               }
               pRoomIndex->vnum = vnum;
               pRoomIndex->area = tarea;
               fBootDb = tmpBootDb;

               if( fBootDb )
               {
                  if( !tarea->low_r_vnum )
                     tarea->low_r_vnum = vnum;
                  if( vnum > tarea->hi_r_vnum )
                     tarea->hi_r_vnum = vnum;
               }
               break;
            }
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }
}

void oprog_file_read( OBJ_INDEX_DATA * prog_target, const char *f )
{
   MPROG_DATA *mprg = NULL;
   char MUDProgfile[256];
   FILE *progfile;
   char letter;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

   if( !( progfile = fopen( MUDProgfile, "r" ) ) )
   {
      bug( "%s: couldn't open mudprog file", __func__ );
      return;
   }

   for( ;; )
   {
      letter = fread_letter( progfile );

      if( letter != '#' )
      {
         bug( "%s: MUDPROG char", __func__ );
         break;
      }

      const char *word = ( feof( progfile ) ? "ENDFILE" : fread_word( progfile ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __func__ );
         word = "ENDFILE";
      }

      if( !str_cmp( word, "ENDFILE" ) )
         break;

      if( !str_cmp( word, "MUDPROG" ) )
      {
         CREATE( mprg, MPROG_DATA, 1 );

         for( ;; )
         {
            word = ( feof( progfile ) ? "#ENDPROG" : fread_word( progfile ) );

            if( word[0] == '\0' )
            {
               log_printf( "%s: EOF encountered reading file!", __func__ );
               word = "#ENDPROG";
            }

            if( !str_cmp( word, "#ENDPROG" ) )
            {
               mprg->next = prog_target->mudprogs;
               prog_target->mudprogs = mprg;
               break;
            }

            switch ( word[0] )
            {
               default:
                  log_printf( "%s: no match: %s", __func__, word );
                  fread_to_eol( progfile );
                  break;

               case 'A':
                  if( !str_cmp( word, "Arglist" ) )
                  {
                     mprg->arglist = fread_string( progfile );
                     mprg->fileprog = true;

                     switch ( mprg->type )
                     {
                        case IN_FILE_PROG:
                           bug( "%s: Nested file programs are not allowed.", __func__ );
                           DISPOSE( mprg );
                           break;

                        default:
                           break;
                     }
                     break;
                  }
                  break;

               case 'C':
                  KEY( "Comlist", mprg->comlist, fread_string( progfile ) );
                  break;

               case 'P':
                  if( !str_cmp( word, "Progtype" ) )
                  {
                     mprg->type = mprog_name_to_type( fread_flagstring( progfile ) );
                     break;
                  }
                  break;
            }
            if( !fMatch )
            {
               fread_to_eol( progfile );
               continue;
            }
         }
      }
   }
   FCLOSE( progfile );
   return;
}

void fread_fuss_objprog( FILE * fp, MPROG_DATA * mprg, OBJ_INDEX_DATA * prog_target )
{
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDPROG" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDPROG";
      }

      if( !str_cmp( word, "#ENDPROG" ) )
         return;

      switch ( word[0] )
      {
         default:
            log_printf( "%s: no match: %s", __FUNCTION__, word );
            fread_to_eol( fp );
            break;

         case 'A':
            if( !str_cmp( word, "Arglist" ) )
            {
               mprg->arglist = fread_string( fp );
               mprg->fileprog = false;

               switch ( mprg->type )
               {
                  case IN_FILE_PROG:
                     oprog_file_read( prog_target, mprg->arglist );
                     break;
                  default:
                     break;
               }
               break;
            }
            break;

         case 'C':
            KEY( "Comlist", mprg->comlist, fread_string( fp ) );
            break;

         case 'P':
            if( !str_cmp( word, "Progtype" ) )
            {
               mprg->type = mprog_name_to_type( fread_flagstring( fp ) );
               SET_BIT( prog_target->progtypes, mprg->type );
               break;
            }
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }
}

void fread_fuss_object( FILE * fp, AREA_DATA * tarea )
{
   OBJ_INDEX_DATA *pObjIndex = NULL;
   bool oldobj = false;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDOBJECT" : fread_word( fp ) );
      char flag[MAX_INPUT_LENGTH];
      int value = 0;

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDOBJECT";
      }

      switch ( word[0] )
      {
         default:
            bug( "%s: no match: %s", __func__, word );
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#ENDOBJECT" ) )
            {
               if( !pObjIndex->description )
                  pObjIndex->description = STRALLOC( "" );
               if( !pObjIndex->action_desc )
                  pObjIndex->action_desc = STRALLOC( "" );

               if( !oldobj )
               {
                  int iHash = pObjIndex->vnum % MAX_KEY_HASH;
                  pObjIndex->next = obj_index_hash[iHash];
                  obj_index_hash[iHash] = pObjIndex;
                  ++top_obj_index;
               }
               return;
            }

            if( !str_cmp( word, "#EXDESC" ) )
            {
               EXTRA_DESCR_DATA *ed = fread_fuss_exdesc( fp );
               if( ed )
                  LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
               break;
            }

            if( !str_cmp( word, "#MUDPROG" ) )
            {
               MPROG_DATA *mprg;

               CREATE( mprg, MPROG_DATA, 1 );
               fread_fuss_objprog( fp, mprg, pObjIndex );
               mprg->next = pObjIndex->mudprogs;
               pObjIndex->mudprogs = mprg;
               break;
            }
            break;

         case 'A':
            KEY( "Action", pObjIndex->action_desc, fread_string( fp ) );

            if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
            {
               AFFECT_DATA *af = fread_fuss_affect( fp, word );

               if( af )
                  LINK( af, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
               break;
            }
            break;

         case 'F':
            if( !str_cmp( word, "Flags" ) )
            {
               const char *eflags = fread_flagstring( fp );

               while( eflags[0] != '\0' )
               {
                  eflags = one_argument( eflags, flag );
                  value = get_oflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown object extraflag: %s", __func__, flag );
                  else
                     SET_BIT( pObjIndex->extra_flags, 1 << value );
               }
               break;
            }
            break;

         case 'K':
            KEY( "Keywords", pObjIndex->name, fread_string( fp ) );
            break;

         case 'L':
            KEY( "Long", pObjIndex->description, fread_string( fp ) );
            break;

         case 'S':
            KEY( "Short", pObjIndex->short_descr, fread_string( fp ) );
            if( !str_cmp( word, "Spells" ) )
            {
               switch ( pObjIndex->item_type )
               {
                  default:
                     break;

                  case ITEM_PILL:
                  case ITEM_POTION:
                  case ITEM_SCROLL:
                     pObjIndex->value[1] = skill_lookup( fread_word( fp ) );
                     pObjIndex->value[2] = skill_lookup( fread_word( fp ) );
                     pObjIndex->value[3] = skill_lookup( fread_word( fp ) );
                     break;

                  case ITEM_STAFF:
                  case ITEM_WAND:
                     pObjIndex->value[3] = skill_lookup( fread_word( fp ) );
                     break;

                  case ITEM_SALVE:
                     pObjIndex->value[4] = skill_lookup( fread_word( fp ) );
                     pObjIndex->value[5] = skill_lookup( fread_word( fp ) );
                     break;
               }
               break;
            }

            if( !str_cmp( word, "Stats" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3, x4, x5;

               x1 = x2 = x3 = x4 = x5 = 0;
               sscanf( ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5 );

               pObjIndex->weight = x1;
               pObjIndex->cost = x2;
               pObjIndex->rent = x3;
               pObjIndex->level = x4;
               pObjIndex->layers = x5;

               break;
            }
            break;

         case 'T':
            if( !str_cmp( word, "Type" ) )
            {
               value = get_otype( fread_flagstring( fp ) );

               if( value < 0 )
               {
                  bug( "%s: vnum %d: Object has invalid type! Defaulting to trash.", __func__, pObjIndex->vnum );
                  value = get_otype( "trash" );
               }
               pObjIndex->item_type = value;
               break;
            }
            break;

         case 'V':
            if( !str_cmp( word, "Values" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3, x4, x5, x6;
               x1 = x2 = x3 = x4 = x5 = x6 = 0;

               sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );

               pObjIndex->value[0] = x1;
               pObjIndex->value[1] = x2;
               pObjIndex->value[2] = x3;
               pObjIndex->value[3] = x4;
               pObjIndex->value[4] = x5;
               pObjIndex->value[5] = x6;

               break;
            }

            if( !str_cmp( word, "Vnum" ) )
            {
               bool tmpBootDb = fBootDb;
               fBootDb = false;

               int vnum = fread_number( fp );

               if( get_obj_index( vnum ) )
               {
                  if( tmpBootDb )
                  {
                     fBootDb = tmpBootDb;
                     bug( "%s: vnum %d duplicated.", __func__, vnum );

                     // Try to recover, read to end of duplicated object and then bail out
                     for( ;; )
                     {
                        word = feof( fp ) ? "#ENDOBJECT" : fread_word( fp );

                        if( !str_cmp( word, "#ENDOBJECT" ) )
                           return;
                     }
                  }
                  else
                  {
                     pObjIndex = get_obj_index( vnum );
                     log_printf_plus( LOG_BUILD, sysdata.build_level, "Cleaning object: %d", vnum );
                     clean_obj( pObjIndex );
                     oldobj = true;
                  }
               }
               else
               {
                  CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
                  oldobj = false;
               }
               pObjIndex->vnum = vnum;
               fBootDb = tmpBootDb;

               if( fBootDb )
               {
                  if( !tarea->low_o_vnum )
                     tarea->low_o_vnum = vnum;
                  if( vnum > tarea->hi_o_vnum )
                     tarea->hi_o_vnum = vnum;
               }
               break;
            }
            break;

         case 'W':
            if( !str_cmp( word, "WFlags" ) )
            {
               const char *wflags = fread_flagstring( fp );

               while( wflags[0] != '\0' )
               {
                  wflags = one_argument( wflags, flag );
                  value = get_wflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown wear flag: %s", __func__, flag );
                  else
                     SET_BIT( pObjIndex->wear_flags, 1 << value );
               }
               break;
            }
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }
}

void mprog_file_read( MOB_INDEX_DATA * prog_target, const char *f )
{
   MPROG_DATA *mprg = NULL;
   char MUDProgfile[256];
   FILE *progfile;
   char letter;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

   if( !( progfile = fopen( MUDProgfile, "r" ) ) )
   {
      bug( "%s: couldn't open mudprog file", __func__ );
      return;
   }

   for( ;; )
   {
      letter = fread_letter( progfile );

      if( letter != '#' )
      {
         bug( "%s: MUDPROG char", __func__ );
         break;
      }

      const char *word = ( feof( progfile ) ? "ENDFILE" : fread_word( progfile ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __func__ );
         word = "ENDFILE";
      }

      if( !str_cmp( word, "ENDFILE" ) )
         break;

      if( !str_cmp( word, "MUDPROG" ) )
      {
         CREATE( mprg, MPROG_DATA, 1 );

         for( ;; )
         {
            word = ( feof( progfile ) ? "#ENDPROG" : fread_word( progfile ) );

            if( word[0] == '\0' )
            {
               log_printf( "%s: EOF encountered reading file!", __func__ );
               word = "#ENDPROG";
            }

            if( !str_cmp( word, "#ENDPROG" ) )
            {
               mprg->next = prog_target->mudprogs;
               prog_target->mudprogs = mprg;
               break;
            }

            switch ( word[0] )
            {
               default:
                  log_printf( "%s: no match: %s", __func__, word );
                  fread_to_eol( progfile );
                  break;

               case 'A':
                  if( !str_cmp( word, "Arglist" ) )
                  {
                     mprg->arglist = fread_string( progfile );
                     mprg->fileprog = true;

                     switch ( mprg->type )
                     {
                        case IN_FILE_PROG:
                           bug( "%s: Nested file programs are not allowed.", __func__ );
                           DISPOSE( mprg );
                           break;

                        default:
                           break;
                     }
                     break;
                  }
                  break;

               case 'C':
                  KEY( "Comlist", mprg->comlist, fread_string( progfile ) );
                  break;

               case 'P':
                  if( !str_cmp( word, "Progtype" ) )
                  {
                     mprg->type = mprog_name_to_type( fread_flagstring( progfile ) );
                     break;
                  }
                  break;
            }
            if( !fMatch )
            {
               fread_to_eol( progfile );
               continue;
            }
         }
      }
   }
   FCLOSE( progfile );
   return;
}

void fread_fuss_mobprog( FILE * fp, MPROG_DATA * mprg, MOB_INDEX_DATA * prog_target )
{
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDPROG" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDPROG";
      }

      if( !str_cmp( word, "#ENDPROG" ) )
         return;

      switch ( word[0] )
      {
         default:
            log_printf( "%s: no match: %s", __FUNCTION__, word );
            fread_to_eol( fp );
            break;

         case 'A':
            if( !str_cmp( word, "Arglist" ) )
            {
               mprg->arglist = fread_string( fp );
               mprg->fileprog = false;

               switch ( mprg->type )
               {
                  case IN_FILE_PROG:
                     mprog_file_read( prog_target, mprg->arglist );
                     break;
                  default:
                     break;
               }
               break;
            }
            break;

         case 'C':
            KEY( "Comlist", mprg->comlist, fread_string( fp ) );
            break;

         case 'P':
            if( !str_cmp( word, "Progtype" ) )
            {
               mprg->type = mprog_name_to_type( fread_flagstring( fp ) );
               SET_BIT( prog_target->progtypes, mprg->type );
               break;
            }
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }
}

void fread_fuss_mobile( FILE * fp, AREA_DATA * tarea )
{
   MOB_INDEX_DATA *pMobIndex = NULL;
   bool oldmob = false;
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDMOBILE" : fread_word( fp ) );
      char flag[MAX_INPUT_LENGTH];
      int value = 0;

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDMOBILE";
      }

      switch ( word[0] )
      {
         default:
            log_printf( "%s: no match: %s", __FUNCTION__, word );
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#MUDPROG" ) )
            {
               MPROG_DATA *mprg;
               CREATE( mprg, MPROG_DATA, 1 );
               fread_fuss_mobprog( fp, mprg, pMobIndex );
               mprg->next = pMobIndex->mudprogs;
               pMobIndex->mudprogs = mprg;
               break;
            }

            if( !str_cmp( word, "#ENDMOBILE" ) )
            {
               if( !pMobIndex->long_descr )
                  pMobIndex->long_descr = STRALLOC( "" );
               if( !pMobIndex->description )
                  pMobIndex->description = STRALLOC( "" );

               if( !oldmob )
               {
                  int iHash = pMobIndex->vnum % MAX_KEY_HASH;
                  pMobIndex->next = mob_index_hash[iHash];
                  mob_index_hash[iHash] = pMobIndex;
                  ++top_mob_index;
               }
               return;
            }
            break;

         case 'A':
            if( !str_cmp( word, "Actflags" ) )
            {
               const char *actflags = NULL;

               actflags = fread_flagstring( fp );

               while( actflags[0] != '\0' )
               {
                  actflags = one_argument( actflags, flag );
                  value = get_actflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown actflag: %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->act, 1 << value );
               }
               break;
            }

            if( !str_cmp( word, "Affected" ) )
            {
               const char *affectflags = NULL;

               affectflags = fread_flagstring( fp );

               while( affectflags[0] != '\0' )
               {
                  affectflags = one_argument( affectflags, flag );
                  value = get_aflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown affectflag: %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->affected_by, 1 << value );
               }
               break;
            }

            if( !str_cmp( word, "Attacks" ) )
            {
               const char *attacks = fread_flagstring( fp );

               while( attacks[0] != '\0' )
               {
                  attacks = one_argument( attacks, flag );
                  value = get_attackflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown attackflag: %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->attacks, 1 << value );
               }
               break;
            }

            if( !str_cmp( word, "Attribs" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3, x4, x5, x6, x7, x8;

               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
               sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );

               pMobIndex->perm_str = x1;
               pMobIndex->perm_int = x2;
               pMobIndex->perm_wis = x3;
               pMobIndex->perm_dex = x4;
               pMobIndex->perm_con = x5;
               pMobIndex->perm_cha = x6;
               pMobIndex->perm_lck = x7;
               pMobIndex->perm_frc = x8;

               break;
            }
            break;

         case 'B':
            if( !str_cmp( word, "Bodyparts" ) )
            {
               const char *bodyparts = fread_flagstring( fp );

               while( bodyparts[0] != '\0' )
               {
                  bodyparts = one_argument( bodyparts, flag );
                  value = get_partflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown bodypart: %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->xflags, 1 << value );
               }
               break;
            }
            break;

         case 'D':
            if( !str_cmp( word, "Defenses" ) )
            {
               const char *defenses = fread_flagstring( fp );

               while( defenses[0] != '\0' )
               {
                  defenses = one_argument( defenses, flag );
                  value = get_defenseflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown defenseflag: %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->defenses, 1 << value );
               }
               break;
            }

            if( !str_cmp( word, "DefPos" ) )
            {
               short position = get_npc_position( fread_flagstring( fp ) );

               if( position < 0 || position > POS_DRAG )
               {
                  bug( "%s: vnum %d: Mobile in invalid default position! Defaulting to standing.", __func__,
                       pMobIndex->vnum );
                  position = POS_STANDING;
               }
               pMobIndex->defposition = position;
               break;
            }

            KEY( "Desc", pMobIndex->description, fread_string( fp ) );
            break;

         case 'G':
            if( !str_cmp( word, "Gender" ) )
            {
               short sex = get_npc_sex( fread_flagstring( fp ) );

               if( sex < 0 || sex > SEX_FEMALE )
               {
                  bug( "%s: vnum %d: Mobile has invalid sex! Defaulting to neuter.", __func__, pMobIndex->vnum );
                  sex = SEX_NEUTRAL;
               }
               pMobIndex->sex = sex;
               break;
            }
            break;

         case 'I':
            if( !str_cmp( word, "Immune" ) )
            {
               const char *immune = fread_flagstring( fp );

               while( immune[0] != '\0' )
               {
                  immune = one_argument( immune, flag );
                  value = get_risflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown RIS flag (I): %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->immune, 1 << value );
               }
               break;
            }
            break;

         case 'K':
            KEY( "Keywords", pMobIndex->player_name, fread_string( fp ) );
            break;

         case 'L':
            KEY( "Long", pMobIndex->long_descr, fread_string( fp ) );
            break;

         case 'P':
            if( !str_cmp( word, "Position" ) )
            {
               short position = get_npc_position( fread_flagstring( fp ) );

               if( position < 0 || position > POS_DRAG )
               {
                  bug( "%s: vnum %d: Mobile in invalid position! Defaulting to standing.", __func__, pMobIndex->vnum );
                  position = POS_STANDING;
               }
               pMobIndex->position = position;
               break;
            }
            break;

         case 'R':
            if( !str_cmp( word, "Race" ) )
            {
               short race = get_npc_race( fread_flagstring( fp ) );

               if( race < 0 || race >= MAX_NPC_RACE )
               {
                  bug( "%s: vnum %d: Mob has invalid race! Defaulting to human.", __func__, pMobIndex->vnum );
                  race = get_npc_race( "human" );
               }

               pMobIndex->race = race;
               break;
            }

            if( !str_cmp( word, "RepairData" ) )
            {
               int iFix;
               REPAIR_DATA *rShop;

               CREATE( rShop, REPAIR_DATA, 1 );
               rShop->keeper = pMobIndex->vnum;
               for( iFix = 0; iFix < MAX_FIX; ++iFix )
                  rShop->fix_type[iFix] = fread_number( fp );
               rShop->profit_fix = fread_number( fp );
               rShop->shop_type = fread_number( fp );
               rShop->open_hour = fread_number( fp );
               rShop->close_hour = fread_number( fp );

               pMobIndex->rShop = rShop;
               LINK( rShop, first_repair, last_repair, next, prev );
               ++top_repair;

               break;
            }

            if( !str_cmp( word, "Resist" ) )
            {
               const char *resist = fread_flagstring( fp );

               while( resist[0] != '\0' )
               {
                  resist = one_argument( resist, flag );
                  value = get_risflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown RIS flag (R): %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->resistant, 1 << value );
               }
               break;
            }
            break;

         case 'S':
            if( !str_cmp( word, "Saves" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3, x4, x5;

               x1 = x2 = x3 = x4 = x5 = 0;
               sscanf( ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5 );

               pMobIndex->saving_poison_death = x1;
               pMobIndex->saving_wand = x2;
               pMobIndex->saving_para_petri = x3;
               pMobIndex->saving_breath = x4;
               pMobIndex->saving_spell_staff = x5;

               break;
            }

            KEY( "Short", pMobIndex->short_descr, fread_string( fp ) );

            if( !str_cmp( word, "ShopData" ) )
            {
               int iTrade;
               SHOP_DATA *pShop;

               CREATE( pShop, SHOP_DATA, 1 );
               pShop->keeper = pMobIndex->vnum;
               for( iTrade = 0; iTrade < MAX_TRADE; ++iTrade )
                  pShop->buy_type[iTrade] = fread_number( fp );
               pShop->profit_buy = fread_number( fp );
               pShop->profit_sell = fread_number( fp );
               pShop->profit_buy = URANGE( pShop->profit_sell + 5, pShop->profit_buy, 1000 );
               pShop->profit_sell = URANGE( 0, pShop->profit_sell, pShop->profit_buy - 5 );
               pShop->open_hour = fread_number( fp );
               pShop->close_hour = fread_number( fp );

               pMobIndex->pShop = pShop;
               LINK( pShop, first_shop, last_shop, next, prev );
               ++top_shop;

               break;
            }

            if( !str_cmp( word, "Speaks" ) )
            {
               const char *speaks = fread_flagstring( fp );

               while( speaks[0] != '\0' )
               {
                  speaks = one_argument( speaks, flag );
                  value = get_langnum_save( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown speaks language: %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->speaks, 1 << value );
               }

               if( !pMobIndex->speaks )
                  pMobIndex->speaks = LANG_BASIC;
               break;
            }

            if( !str_cmp( word, "Speaking" ) )
            {
               const char *speaking = fread_flagstring( fp );

               while( speaking[0] != '\0' )
               {
                  speaking = one_argument( speaking, flag );
                  value = get_langnum_save( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown speaking language: %s", __func__, flag );
          	      else
                     SET_BIT( pMobIndex->speaking, 1 << value );
               }

               if( !pMobIndex->speaking )
                  pMobIndex->speaking = LANG_BASIC;
               break;
            }

            if( !str_cmp( word, "Specfun" ) )
            {
               const char *temp = fread_flagstring( fp );
               if( !pMobIndex )
               {
                  bug( "%s: Specfun: Invalid mob vnum!", __func__ );
                  break;
               }
               if( !( pMobIndex->spec_fun = spec_lookup( temp ) ) )
               {
                  bug( "%s: Specfun: vnum %d, no spec_fun called %s.", __func__, pMobIndex->vnum, temp );
                  pMobIndex->spec_funname = NULL;
               }
               else
                  pMobIndex->spec_funname = STRALLOC( temp );
               break;
            }

            if( !str_cmp( word, "Specfun2" ) )
            {
               const char *temp = fread_flagstring( fp );
               if( !pMobIndex )
               {
                  bug( "%s: Specfun: Invalid mob vnum!", __func__ );
                  break;
               }
               if( !( pMobIndex->spec_2 = spec_lookup( temp ) ) )
               {
                  bug( "%s: Specfun: vnum %d, no spec_fun called %s.", __func__, pMobIndex->vnum, temp );
                  pMobIndex->spec_funname2 = NULL;
               }
               else
                  pMobIndex->spec_funname2 = STRALLOC( temp );
               break;
            }
            
            if( !str_cmp( word, "Stats1" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3, x4, x5, x6;

               x1 = x2 = x3 = x4 = x5 = x6 = 0;
               sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );

               pMobIndex->alignment = x1;
               pMobIndex->level = x2;
               pMobIndex->mobthac0 = x3;
               pMobIndex->ac = x4;
               pMobIndex->gold = x5;
               pMobIndex->exp = x6;

               break;
            }

            if( !str_cmp( word, "Stats2" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3;
               x1 = x2 = x3 = 0;
               sscanf( ln, "%d %d %d", &x1, &x2, &x3 );

               pMobIndex->hitnodice = x1;
               pMobIndex->hitsizedice = x2;
               pMobIndex->hitplus = x3;

               break;
            }

            if( !str_cmp( word, "Stats3" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3;
               x1 = x2 = x3 = 0;
               sscanf( ln, "%d %d %d", &x1, &x2, &x3 );

               pMobIndex->damnodice = x1;
               pMobIndex->damsizedice = x2;
               pMobIndex->damplus = x3;

               break;
            }

            if( !str_cmp( word, "Stats4" ) )
            {
               char *ln = fread_line( fp );
               int x1, x2, x3, x4, x5;

               x1 = x2 = x3 = x4 = x5 = 0;
               sscanf( ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5 );

               pMobIndex->height = x1;
               pMobIndex->weight = x2;
               pMobIndex->numattacks = x3;
               pMobIndex->hitroll = x4;
               pMobIndex->damroll = x5;

               break;
            }

            if( !str_cmp( word, "Suscept" ) )
            {
               const char *suscep = fread_flagstring( fp );

               while( suscep[0] != '\0' )
               {
                  suscep = one_argument( suscep, flag );
                  value = get_risflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown RIS flag (S): %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->susceptible, 1 << value );
               }
               break;
            }
            break;

         case 'V':
            if( !str_cmp( word, "VIPFlags" ) )
            {
               const char *vip = fread_flagstring( fp );

               while( vip[0] != '\0' )
               {
                  vip = one_argument( vip, flag );
                  if( !str_cmp( flag, "yavin" ) || !str_cmp( flag, "iv" ) )
                     value = get_vip_flag( "yavin_iv" );
                  else if( !str_cmp( flag, "mon" ) || !str_cmp( flag, "calamari" ) )
                     value = get_vip_flag( "mon_calamari" );
                  else if( !str_cmp( flag, "ord" ) || !str_cmp( flag, "mantell" ) )
                     value = get_vip_flag( "ord_mantell" );
                  else if( !str_cmp( flag, "nal" ) || !str_cmp( flag, "hutta" ) )
                     value = get_vip_flag( "nal_hutta" );
                  else
                     value = get_vip_flag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown VIP flag: %s", __func__, flag );
                  else
                     SET_BIT( pMobIndex->vip_flags, 1 << value );
               }
               break;
            }

            if( !str_cmp( word, "Vnum" ) )
            {
               bool tmpBootDb = fBootDb;
               fBootDb = false;

               int vnum = fread_number( fp );

               if( get_mob_index( vnum ) )
               {
                  if( tmpBootDb )
                  {
                     fBootDb = tmpBootDb;
                     bug( "%s: vnum %d duplicated.", __func__, vnum );

                     // Try to recover, read to end of duplicated mobile and then bail out
                     for( ;; )
                     {
                        word = feof( fp ) ? "#ENDMOBILE" : fread_word( fp );

                        if( !str_cmp( word, "#ENDMOBILE" ) )
                           return;
                     }
                  }
                  else
                  {
                     pMobIndex = get_mob_index( vnum );
                     log_printf_plus( LOG_BUILD, sysdata.build_level, "Cleaning mobile: %d", vnum );
                     clean_mob( pMobIndex );
                     oldmob = true;
                  }
               }
               else
               {
                  CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
                  oldmob = false;
               }
               pMobIndex->vnum = vnum;
               fBootDb = tmpBootDb;

               if( fBootDb )
               {
                  if( !tarea->low_m_vnum )
                     tarea->low_m_vnum = vnum;
                  if( vnum > tarea->hi_m_vnum )
                     tarea->hi_m_vnum = vnum;
               }
               break;
            }
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }
}

void fread_fuss_areadata( FILE * fp, AREA_DATA * tarea )
{
   bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

   for( ;; )
   {
      const char *word = ( feof( fp ) ? "#ENDAREADATA" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
         word = "#ENDAREADATA";
      }

      switch ( word[0] )
      {
         default:
            log_printf( "%s: no match: %s", __FUNCTION__, word );
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#ENDAREADATA" ) )
            {
               tarea->age = tarea->reset_frequency;
               return;
            }
            break;

         case 'A':
            KEY( "Author", tarea->author, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "Economy" ) )
            {
               tarea->high_economy = fread_number( fp );
               tarea->low_economy = fread_number( fp );
               break;
            }
            break;

         case 'F':
            if( !str_cmp( word, "Flags" ) )
            {
               const char *areaflags = NULL;
               char flag[MAX_INPUT_LENGTH];
               int value;

               areaflags = fread_flagstring( fp );

               while( areaflags[0] != '\0' )
               {
                  areaflags = one_argument( areaflags, flag );
                  value = get_areaflag( flag );
                  if( value < 0 || value > 31 )
                     bug( "%s: Unknown area flag: %s", __func__, flag );
                  else
                     SET_BIT( tarea->flags, 1 << value );
               }
               break;
            }
            break;

         case 'N':
            KEY( "Name", tarea->name, fread_string_nohash( fp ) );
            break;

         case 'R':
            if( !str_cmp( word, "Ranges" ) )
            {
               int x1, x2, x3, x4;
               char *ln;

               ln = fread_line( fp );

               x1 = x2 = x3 = x4 = 0;
               sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

               tarea->low_soft_range = x1;
               tarea->hi_soft_range = x2;
               tarea->low_hard_range = x3;
               tarea->hi_hard_range = x4;

               break;
            }
            KEY( "ResetMsg", tarea->resetmsg, fread_string_nohash( fp ) );
            KEY( "ResetFreq", tarea->reset_frequency, fread_number( fp ) );
            break;

         case 'V':
            KEY( "Version", tarea->version, fread_number( fp ) );
            break;
      }
      if( !fMatch )
      {
         fread_to_eol( fp );
         continue;
      }
   }
}

/*
 * Load an 'area' header line.
 */
AREA_DATA *create_area( void )
{
   AREA_DATA *pArea;

   CREATE( pArea, AREA_DATA, 1 );
   pArea->first_room = pArea->last_room = NULL;
   pArea->name = NULL;
   pArea->author = NULL;
   pArea->filename = strdup( strArea );
   pArea->age = 15;
   pArea->reset_frequency = 15;
   pArea->nplayer = 0;
   pArea->low_r_vnum = 0;
   pArea->low_o_vnum = 0;
   pArea->low_m_vnum = 0;
   pArea->hi_r_vnum = 0;
   pArea->hi_o_vnum = 0;
   pArea->hi_m_vnum = 0;
   pArea->low_soft_range = 0;
   pArea->hi_soft_range = MAX_LEVEL;
   pArea->low_hard_range = 0;
   pArea->hi_hard_range = MAX_LEVEL;
   pArea->version = 1;

   LINK( pArea, first_area, last_area, next, prev );
   ++top_area;
   return pArea;
}

AREA_DATA *fread_fuss_area( AREA_DATA * tarea, FILE * fp )
{
   for( ;; )
   {
      char letter;
      const char *word;

      letter = fread_letter( fp );
      if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }

      if( letter != '#' )
      {
         bug( "%s: # not found. Invalid format.", __func__ );
         if( fBootDb )
            exit( 1 );
         break;
      }

      word = ( feof( fp ) ? "ENDAREA" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         bug( "%s: EOF encountered reading file!", __func__ );
         word = "ENDAREA";
      }

      if( !str_cmp( word, "AREADATA" ) )
      {
         if( !tarea )
            tarea = create_area(  );
         fread_fuss_areadata( fp, tarea );
      }
      else if( !str_cmp( word, "MOBILE" ) )
         fread_fuss_mobile( fp, tarea );
      else if( !str_cmp( word, "OBJECT" ) )
         fread_fuss_object( fp, tarea );
      else if( !str_cmp( word, "ROOM" ) )
         fread_fuss_room( fp, tarea );
      else if( !str_cmp( word, "ENDAREA" ) )
         break;
      else
      {
         bug( "%s: Bad section header: %s", __func__, word );
         fread_to_eol( fp );
      }
   }
   return tarea;
}

void load_area_file( AREA_DATA * tarea, const char *filename )
{
   char *word;
   int aversion = 0;

   if( !( fpArea = fopen( filename, "r" ) ) )
   {
      perror( filename );
      bug( "%s: error loading file (can't open) %s", __func__, filename );
      return;
   }

   if( fread_letter( fpArea ) != '#' )
   {
      if( fBootDb )
      {
         bug( "%s: No # found at start of area file.", __func__ );
         exit( 1 );
      }
      else
      {
         bug( "%s: No # found at start of area file.", __func__ );
         FCLOSE( fpArea );
         return;
      }
   }

   word = fread_word( fpArea );

   // New FUSS area format support -- Samson 7/5/07
   if( !str_cmp( word, "FUSSAREA" ) )
   {
      tarea = fread_fuss_area( tarea, fpArea );
      FCLOSE( fpArea );

      if( tarea )
         process_sorting( tarea );
      return;
   }

   // Drop through to the old format processor
   if( !str_cmp( word, "AREA" ) )
   {
      if( fBootDb )
         tarea = load_area( fpArea, 0 );
      else
      {
         DISPOSE( tarea->name );
         tarea->name = fread_string_nohash( fpArea );
      }
   }
   // Only seen at this stage for help.are
   else if( !str_cmp( word, "HELPS" ) )
      load_helps( fpArea );
   // Only seen at this stage for SmaugWiz areas
   else if( !str_cmp( word, "VERSION" ) )
      aversion = fread_number( fpArea );

   for( ;; )
   {
      if( fread_letter( fpArea ) != '#' )
      {
         bug( "%s: # not found", __func__ );
         exit( 1 );
      }

      word = fread_word( fpArea );

      if( word[0] == '$' )
         break;
      // Only seen at this stage for SmaugWiz areas. The format had better be right or there'll be trouble here!
      else if( !str_cmp( word, "AREA" ) )
         tarea = load_area( fpArea, aversion );
      else if( !str_cmp( word, "AUTHOR" ) )
         load_author( tarea, fpArea );
      else if( !str_cmp( word, "FLAGS" ) )
         load_flags( tarea, fpArea );
      else if( !str_cmp( word, "RANGES" ) )
         load_ranges( tarea, fpArea );
      else if( !str_cmp( word, "ECONOMY" ) )
         load_economy( tarea, fpArea );
      else if( !str_cmp( word, "RESETMSG" ) )
         load_resetmsg( tarea, fpArea );
      /*
       * Rennard 
       */
      else if( !str_cmp( word, "HELPS" ) )
         load_helps( fpArea );
      else if( !str_cmp( word, "MOBILES" ) )
         load_mobiles( tarea, fpArea );
      else if( !str_cmp( word, "OBJECTS" ) )
         load_objects( tarea, fpArea );
      else if( !str_cmp( word, "RESETS" ) )
         load_resets( tarea, fpArea );
      else if( !str_cmp( word, "ROOMS" ) )
         load_rooms( tarea, fpArea );
      else if( !str_cmp( word, "SHOPS" ) )
         load_shops( fpArea );
      else if( !str_cmp( word, "REPAIRS" ) )
         load_repairs( fpArea );
      else if( !str_cmp( word, "SPECIALS" ) )
         load_specials( fpArea );
      else if( !str_cmp( word, "VERSION" ) )
         load_version( tarea, fpArea );
      else
      {
         bug( "%s: bad section name: %s", __func__, word );
         if( fBootDb )
            exit( 1 );
         else
         {
            FCLOSE( fpArea );
            return;
         }
      }
   }
   FCLOSE( fpArea );

   if( tarea )
      process_sorting( tarea );
   else
      fprintf( stderr, "(%s)\n", filename );
}

/* Build list of in_progress areas.  Do not load areas.
 * define AREA_READ if you want it to build area names rather than reading
 * them out of the area files. -- Altrag */
void load_buildlist( void )
{
   DIR *dp;
   struct dirent *dentry;
   FILE *fp;
   char godfile[270];
   char buildfile[280];
   char buf[MAX_STRING_LENGTH];
   AREA_DATA *pArea;
   char line[81];
   char word[81];
   int low, hi;
   int mlow, mhi, olow, ohi, rlow, rhi;
   bool badfile = FALSE;
   char temp;

   dp = opendir( GOD_DIR );
   dentry = readdir( dp );
   while( dentry )
   {
      if( dentry->d_name[0] != '.' )
      {
         snprintf( godfile, 270, "%s%s", GOD_DIR, dentry->d_name );
         if( !( fp = fopen( godfile, "r" ) ) )
         {
            bug( "%s: invalid file", __func__ );
            dentry = readdir( dp );
            continue;
         }
         log_string( godfile );
         badfile = FALSE;
         rlow = rhi = olow = ohi = mlow = mhi = 0;
         while( !feof( fp ) && !ferror( fp ) )
         {
            low = 0;
            hi = 0;
            word[0] = 0;
            line[0] = 0;
            if( ( temp = fgetc( fp ) ) != EOF )
               ungetc( temp, fp );
            else
               break;

            fgets( line, 80, fp );
            sscanf( line, "%s %d %d", word, &low, &hi );
            if( !strcmp( word, "Level" ) )
            {
               if( low < LEVEL_AVATAR )
               {
                  snprintf( buf, MAX_STRING_LENGTH, "%s: God file with level %d < %d", dentry->d_name, low, LEVEL_AVATAR );
                  badfile = TRUE;
               }
            }
            if( !strcmp( word, "RoomRange" ) )
               rlow = low, rhi = hi;
            else if( !strcmp( word, "MobRange" ) )
               mlow = low, mhi = hi;
            else if( !strcmp( word, "ObjRange" ) )
               olow = low, ohi = hi;
         }
         FCLOSE( fp );
         if( rlow && rhi && !badfile )
         {
            snprintf( buildfile, 280, "%s%s.are", BUILD_DIR, dentry->d_name );
            if( !( fp = fopen( buildfile, "r" ) ) )
            {
               bug( "%s: cannot open area file for read", __func__ );
               dentry = readdir( dp );
               continue;
            }
#if !defined(READ_AREA) /* Dont always want to read stuff.. dunno.. shrug */

            strlcpy( word, fread_word( fp ), 81 );
            if( word[0] != '#' || strcmp( &word[1], "AREA" ) )
            {
               bug( "%s: %s.are: no #AREA found.", __func__, dentry->d_name );
               FCLOSE( fp );
               dentry = readdir( dp );
               continue;
            }
#endif
            CREATE( pArea, AREA_DATA, 1 );
            snprintf( buf, MAX_STRING_LENGTH, "%s.are", dentry->d_name );
            pArea->author = STRALLOC( dentry->d_name );
            pArea->filename = strdup( buf );
#if !defined(READ_AREA)
            pArea->name = fread_string_nohash( fp );
#else
            snprintf( buf, MAX_STRING_LENGTH, "{PROTO} %s's area in progress", dentry->d_name );
            pArea->name = strdup( buf );
#endif
            FCLOSE( fp );
            pArea->low_r_vnum = rlow;
            pArea->hi_r_vnum = rhi;
            pArea->low_m_vnum = mlow;
            pArea->hi_m_vnum = mhi;
            pArea->low_o_vnum = olow;
            pArea->hi_o_vnum = ohi;
            pArea->low_soft_range = -1;
            pArea->hi_soft_range = -1;
            pArea->low_hard_range = -1;
            pArea->hi_hard_range = -1;
            pArea->first_room = pArea->last_room = NULL;
            SET_BIT( pArea->flags, AFLAG_PROTOTYPE );
            LINK( pArea, first_build, last_build, next, prev );
            fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d "
                     "Mobs: %5d - %-5d\n",
                     pArea->filename,
                     pArea->low_r_vnum, pArea->hi_r_vnum,
                     pArea->low_o_vnum, pArea->hi_o_vnum, pArea->low_m_vnum, pArea->hi_m_vnum );
            sort_area( pArea, TRUE );
         }
      }
      dentry = readdir( dp );
   }
   closedir( dp );
}

/*
 * Sort areas by name alphanumercially
 *      - 4/27/97, Fireblade
 */
void sort_area_by_name( AREA_DATA * pArea )
{
   AREA_DATA *temp_area;

   if( !pArea )
   {
      bug( "%s: NULL pArea", __func__ );
      return;
   }
   for( temp_area = first_area_name; temp_area; temp_area = temp_area->next_sort_name )
   {
      if( strcmp( pArea->name, temp_area->name ) < 0 )
      {
         INSERT( pArea, temp_area, first_area_name, next_sort_name, prev_sort_name );
         break;
      }
   }
   if( !temp_area )
   {
      LINK( pArea, first_area_name, last_area_name, next_sort_name, prev_sort_name );
   }
   return;
}

/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA * pArea, bool proto )
{
   AREA_DATA *area = NULL;
   AREA_DATA *first_sort, *last_sort;
   bool found;

   if( !pArea )
   {
      bug( "%s: NULL pArea", __func__ );
      return;
   }

   if( proto )
   {
      first_sort = first_bsort;
      last_sort = last_bsort;
   }
   else
   {
      first_sort = first_asort;
      last_sort = last_asort;
   }

   found = false;
   pArea->next_sort = NULL;
   pArea->prev_sort = NULL;

   if( !first_sort )
   {
      pArea->prev_sort = NULL;
      pArea->next_sort = NULL;
      first_sort = pArea;
      last_sort = pArea;
      found = true;
   }
   else
      for( area = first_sort; area; area = area->next_sort )
         if( pArea->low_r_vnum < area->low_r_vnum )
         {
            if( !area->prev_sort )
               first_sort = pArea;
            else
               area->prev_sort->next_sort = pArea;
            pArea->prev_sort = area->prev_sort;
            pArea->next_sort = area;
            area->prev_sort = pArea;
            found = true;
            break;
         }

   if( !found )
   {
      pArea->prev_sort = last_sort;
      pArea->next_sort = NULL;
      last_sort->next_sort = pArea;
      last_sort = pArea;
   }

   if( proto )
   {
      first_bsort = first_sort;
      last_bsort = last_sort;
   }
   else
   {
      first_asort = first_sort;
      last_asort = last_sort;
   }
}

/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA * ch, int low, int high, bool proto, bool shownl, const char *loadst, const char *notloadst )
{
   AREA_DATA *pArea, *first_sort;
   int count, loaded;

   count = 0;
   loaded = 0;
   set_pager_color( AT_PLAIN, ch );
   if( proto )
      first_sort = first_bsort;
   else
      first_sort = first_asort;
   for( pArea = first_sort; pArea; pArea = pArea->next_sort )
   {
      if( IS_SET( pArea->status, AREA_DELETED ) )
         continue;
      if( pArea->low_r_vnum < low )
         continue;
      if( pArea->hi_r_vnum > high )
         break;
      if( IS_SET( pArea->status, AREA_LOADED ) )
         loaded++;
      else if( !shownl )
         continue;
      pager_printf( ch, "&w&W%-15s&B| &GRooms: &W%5d &G- &W%-5d"
                    " &GObjs: &W%5d &G- &W%-5d &GMobs: &W%5d &G- &W%-5d%s\r\n",
                    ( pArea->filename ? pArea->filename : "(invalid)" ),
                    pArea->low_r_vnum, pArea->hi_r_vnum,
                    pArea->low_o_vnum, pArea->hi_o_vnum,
                    pArea->low_m_vnum, pArea->hi_m_vnum, IS_SET( pArea->status, AREA_LOADED ) ? loadst : notloadst );
      count++;
   }
   pager_printf( ch, "&GAreas listed: &W%d  &GLoaded: &W%d\r\n", count, loaded );
   return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */

void do_vnums( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int low, high;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   low = 0;
   high = 32766;
   if( arg1[0] != '\0' )
   {
      low = atoi( arg1 );
      if( arg2[0] != '\0' )
         high = atoi( arg2 );
   }
   show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int low, high;

   do_vnums( ch, argument );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   low = 0;
   high = 32766;

   if( arg1[0] != '\0' )
   {
      low = atoi( arg1 );
      if( arg2[0] != '\0' )
         high = atoi( arg2 );
   }

   show_vnums( ch, low, high, FALSE, TRUE, "", " X" );

}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_newzones( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int low, high;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   low = 0;
   high = 32766;
   if( arg1[0] != '\0' )
   {
      low = atoi( arg1 );
      if( arg2[0] != '\0' )
         high = atoi( arg2 );
   }
   show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}

/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%ssysdata.dat", SYSTEM_DIR );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: fopen", __func__ );
   }
   else
   {
      fprintf( fp, "#SYSTEM\n" );
      fprintf( fp, "Highplayers    %d\n", sys.alltimemax );
      fprintf( fp, "Highplayertime %s~\n", sys.time_of_max );
      fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING );
      fprintf( fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH );
      fprintf( fp, "Readallmail    %d\n", sys.read_all_mail );
      fprintf( fp, "Readmailfree   %d\n", sys.read_mail_free );
      fprintf( fp, "Writemailfree  %d\n", sys.write_mail_free );
      fprintf( fp, "Takeothersmail %d\n", sys.take_others_mail );
      fprintf( fp, "Muse           %d\n", sys.muse_level );
      fprintf( fp, "Think          %d\n", sys.think_level );
      fprintf( fp, "Build          %d\n", sys.build_level );
      fprintf( fp, "Log            %d\n", sys.log_level );
      fprintf( fp, "Protoflag      %d\n", sys.level_modify_proto );
      fprintf( fp, "Overridepriv   %d\n", sys.level_override_private );
      fprintf( fp, "Msetplayer     %d\n", sys.level_mset_player );
      fprintf( fp, "Stunplrvsplr   %d\n", sys.stun_plr_vs_plr );
      fprintf( fp, "Stunregular    %d\n", sys.stun_regular );
      fprintf( fp, "Damplrvsplr    %d\n", sys.dam_plr_vs_plr );
      fprintf( fp, "Damplrvsmob    %d\n", sys.dam_plr_vs_mob );
      fprintf( fp, "Dammobvsplr    %d\n", sys.dam_mob_vs_plr );
      fprintf( fp, "Dammobvsmob    %d\n", sys.dam_mob_vs_mob );
      fprintf( fp, "Forcepc        %d\n", sys.level_forcepc );
      fprintf( fp, "Guildoverseer  %s~\n", sys.guild_overseer );
      fprintf( fp, "Guildadvisor   %s~\n", sys.guild_advisor );
      fprintf( fp, "Saveflags      %d\n", sys.save_flags );
      fprintf( fp, "Savefreq       %d\n", sys.save_frequency );
      fprintf( fp, "Newbie_purge	%d\n", sys.newbie_purge );
      fprintf( fp, "Regular_purge	%d\n", sys.regular_purge );
      fprintf( fp, "Autopurge		%d\n", sys.CLEANPFILES );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      FCLOSE( fp );
   }
   return;
}


void fread_sysdata( SYSTEM_DATA * sys, FILE * fp )
{
   const char *word;
   bool fMatch;

   sys->time_of_max = NULL;
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
            KEY( "Autopurge", sys->CLEANPFILES, fread_number( fp ) );
            break;
         case 'B':
            KEY( "Build", sys->build_level, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Damplrvsplr", sys->dam_plr_vs_plr, fread_number( fp ) );
            KEY( "Damplrvsmob", sys->dam_plr_vs_mob, fread_number( fp ) );
            KEY( "Dammobvsplr", sys->dam_mob_vs_plr, fread_number( fp ) );
            KEY( "Dammobvsmob", sys->dam_mob_vs_mob, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !sys->time_of_max )
                  sys->time_of_max = strdup( "(not recorded)" );
               return;
            }
            break;

         case 'F':
            KEY( "Forcepc", sys->level_forcepc, fread_number( fp ) );
            break;

         case 'G':
            KEY( "Guildoverseer", sys->guild_overseer, fread_string_nohash( fp ) );
            KEY( "Guildadvisor", sys->guild_advisor, fread_string_nohash( fp ) );
            break;

         case 'H':
            KEY( "Highplayers", sys->alltimemax, fread_number( fp ) );
            KEY( "Highplayertime", sys->time_of_max, fread_string_nohash( fp ) );
            break;

         case 'L':
            KEY( "Log", sys->log_level, fread_number( fp ) );
            break;

         case 'M':
            KEY( "Msetplayer", sys->level_mset_player, fread_number( fp ) );
            KEY( "Muse", sys->muse_level, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Nameresolving", sys->NO_NAME_RESOLVING, fread_number( fp ) );
            KEY( "Newbie_purge", sys->newbie_purge, fread_number( fp ) );
            break;

         case 'O':
            KEY( "Overridepriv", sys->level_override_private, fread_number( fp ) );
            break;

         case 'P':
            KEY( "Protoflag", sys->level_modify_proto, fread_number( fp ) );
            break;

         case 'R':
            KEY( "Readallmail", sys->read_all_mail, fread_number( fp ) );
            KEY( "Readmailfree", sys->read_mail_free, fread_number( fp ) );
            KEY( "Regular_purge", sys->regular_purge, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Stunplrvsplr", sys->stun_plr_vs_plr, fread_number( fp ) );
            KEY( "Stunregular", sys->stun_regular, fread_number( fp ) );
            KEY( "Saveflags", sys->save_flags, fread_number( fp ) );
            KEY( "Savefreq", sys->save_frequency, fread_number( fp ) );
            break;

         case 'T':
            KEY( "Takeothersmail", sys->take_others_mail, fread_number( fp ) );
            KEY( "Think", sys->think_level, fread_number( fp ) );
            break;


         case 'W':
            KEY( "Waitforauth", sys->WAIT_FOR_AUTH, fread_number( fp ) );
            KEY( "Writemailfree", sys->write_mail_free, fread_number( fp ) );
            break;
      }


      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
      }
   }
}

/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA * sys )
{
   char filename[256];
   FILE *fp;
   bool found;

   found = FALSE;
   snprintf( filename, 256, "%ssysdata.dat", SYSTEM_DIR );

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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SYSTEM" ) )
         {
            fread_sysdata( sys, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            break;
         }
      }
      FCLOSE( fp );
   }

   if( !sysdata.guild_overseer )
      sysdata.guild_overseer = strdup( "" );
   if( !sysdata.guild_advisor )
      sysdata.guild_advisor = strdup( "" );
   return found;
}

void load_banlist( void )
{
   BAN_DATA *pban;
   FILE *fp;
   int number;
   char letter;

   if( !( fp = fopen( SYSTEM_DIR BAN_LIST, "r" ) ) )
      return;

   for( ;; )
   {
      if( feof( fp ) )
      {
         bug( "%s: no -1 found.", __func__ );
         FCLOSE( fp );
         return;
      }
      number = fread_number( fp );
      if( number == -1 )
      {
         FCLOSE( fp );
         return;
      }
      CREATE( pban, BAN_DATA, 1 );
      pban->level = number;
      pban->name = fread_string_nohash( fp );
      if( ( letter = fread_letter( fp ) ) == '~' )
         pban->ban_time = fread_string_nohash( fp );
      else
      {
         ungetc( letter, fp );
         pban->ban_time = strdup( "(unrecorded)" );
      }
      LINK( pban, first_ban, last_ban, next, prev );
   }
}

/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   AREA_DATA *pArea;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   bool room, mob, obj, all, area_conflict;
   int low_range, high_range;

   room = FALSE;
   mob = FALSE;
   obj = FALSE;
   all = FALSE;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Please specify room, mob, object, or all as your first argument.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "room" ) )
      room = TRUE;

   else if( !str_cmp( arg1, "mob" ) )
      mob = TRUE;

   else if( !str_cmp( arg1, "object" ) )
      obj = TRUE;

   else if( !str_cmp( arg1, "all" ) )
      all = TRUE;
   else
   {
      send_to_char( "Please specify room, mob, or object as your first argument.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Please specify the low end of the range to be searched.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Please specify the high end of the range to be searched.\r\n", ch );
      return;
   }

   low_range = atoi( arg2 );
   high_range = atoi( argument );

   if( low_range < 1 || low_range > 32767 )
   {
      send_to_char( "Invalid argument for bottom of range.\r\n", ch );
      return;
   }

   if( high_range < 1 || high_range > 32767 )
   {
      send_to_char( "Invalid argument for top of range.\r\n", ch );
      return;
   }

   if( high_range < low_range )
   {
      send_to_char( "Bottom of range must be below top of range.\r\n", ch );
      return;
   }

   if( all )
   {
      snprintf( buf, MAX_STRING_LENGTH, "room %d %d", low_range, high_range );
      do_check_vnums( ch, buf );
      snprintf( buf, MAX_STRING_LENGTH, "mob %d %d", low_range, high_range );
      do_check_vnums( ch, buf );
      snprintf( buf, MAX_STRING_LENGTH, "object %d %d", low_range, high_range );
      do_check_vnums( ch, buf );
      return;
   }
   set_char_color( AT_PLAIN, ch );

   for( pArea = first_asort; pArea; pArea = pArea->next_sort )
   {
      area_conflict = FALSE;
      if( IS_SET( pArea->status, AREA_DELETED ) )
         continue;
      else if( room )
      {
         if( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_r_vnum ) && ( low_range <= pArea->hi_r_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_r_vnum ) && ( high_range >= pArea->low_r_vnum ) )
            area_conflict = TRUE;
      }

      if( mob )
      {
         if( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
            area_conflict = TRUE;
         if( ( low_range >= pArea->low_m_vnum ) && ( low_range <= pArea->hi_m_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_m_vnum ) && ( high_range >= pArea->low_m_vnum ) )
            area_conflict = TRUE;
      }

      if( obj )
      {
         if( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_o_vnum ) && ( low_range <= pArea->hi_o_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_o_vnum ) && ( high_range >= pArea->low_o_vnum ) )
            area_conflict = TRUE;
      }

      if( area_conflict )
      {
         snprintf( buf, MAX_STRING_LENGTH, "Conflict:%-15s| ", ( pArea->filename ? pArea->filename : "(invalid)" ) );
         if( room )
            snprintf( buf2, MAX_STRING_LENGTH, "Rooms: %5d - %-5d\r\n", pArea->low_r_vnum, pArea->hi_r_vnum );
         if( mob )
            snprintf( buf2, MAX_STRING_LENGTH, "Mobs: %5d - %-5d\r\n", pArea->low_m_vnum, pArea->hi_m_vnum );
         if( obj )
            snprintf( buf2, MAX_STRING_LENGTH, "Objects: %5d - %-5d\r\n", pArea->low_o_vnum, pArea->hi_o_vnum );

         ch_printf( ch, "%s%s", buf, buf2 );
      }
   }
   for( pArea = first_bsort; pArea; pArea = pArea->next_sort )
   {
      area_conflict = FALSE;
      if( IS_SET( pArea->status, AREA_DELETED ) )
         continue;
      else if( room )
      {
         if( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_r_vnum ) && ( low_range <= pArea->hi_r_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_r_vnum ) && ( high_range >= pArea->low_r_vnum ) )
            area_conflict = TRUE;
      }

      if( mob )
      {
         if( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
            area_conflict = TRUE;
         if( ( low_range >= pArea->low_m_vnum ) && ( low_range <= pArea->hi_m_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_m_vnum ) && ( high_range >= pArea->low_m_vnum ) )
            area_conflict = TRUE;
      }

      if( obj )
      {
         if( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_o_vnum ) && ( low_range <= pArea->hi_o_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_o_vnum ) && ( high_range >= pArea->low_o_vnum ) )
            area_conflict = TRUE;
      }

      if( area_conflict )
      {
         snprintf( buf, MAX_STRING_LENGTH, "Conflict:%-15s| ", ( pArea->filename ? pArea->filename : "(invalid)" ) );
         if( room )
            snprintf( buf2, MAX_STRING_LENGTH, "Rooms: %5d - %-5d\r\n", pArea->low_r_vnum, pArea->hi_r_vnum );
         if( mob )
            snprintf( buf2, MAX_STRING_LENGTH, "Mobs: %5d - %-5d\r\n", pArea->low_m_vnum, pArea->hi_m_vnum );
         if( obj )
            snprintf( buf2, MAX_STRING_LENGTH, "Objects: %5d - %-5d\r\n", pArea->low_o_vnum, pArea->hi_o_vnum );

         ch_printf( ch, "%s%s", buf, buf2 );
      }
   }

/*
    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\r\n",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }

    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\r\n",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }
*/
   return;
}

/* little proggy to center text with whitespace in between. */
const char *centertext( const char *text, size_t size )
{
   static char buf[MAX_STRING_LENGTH];
   int i;
   int filler = ( size - strlen( remand( text ) ) ) / 2 - 1;

   strlcpy( buf, " ", MAX_STRING_LENGTH );

   if( size == 0 || size < strlen( remand( text ) ) )
      return "(null)";

   if( size == strlen( remand( text ) ) )
      return text;

   for( i = 0; i < filler; i++ )
      strlcat( buf, " ", MAX_STRING_LENGTH );

   strlcat( buf, text, MAX_STRING_LENGTH );

   for( i = 0; i < filler; i++ )
      strlcat( buf, " ", MAX_STRING_LENGTH );

   return buf;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
   return;
}

/*
 * Read a string of text based flags from file fp. Ending in ~
 */
char *fread_flagstring( FILE * fp )
{
   static char buf[MSL];
   char *plast;
   char c;
   int ln;

   plast = buf;
   buf[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if( feof( fp ) )
      {
         bug( "%s: EOF encountered on read.", __func__ );
         if( fBootDb )
         {
            shutdown_mud( "Corrupt file somewhere." );
            exit( 1 );
         }
         snprintf( buf, MSL, "%s", "" );
         return buf;
      }
      c = getc( fp );
   }
   while( isspace( c ) );

   if( ( *plast++ = c ) == '~' )
   {
      snprintf( buf, MSL, "%s", "" );
      return buf;
   }

   for( ;; )
   {
      if( ln >= ( MSL - 1 ) )
      {
         bug( "%s: string too long", __func__ );
         *plast = '\0';
         return ( buf );
      }
      switch ( *plast = getc( fp ) )
      {
         default:
            plast++;
            ln++;
            break;

         case EOF:
            bug( "%s: EOF", __func__ );
            if( fBootDb )
               exit( 1 );
            *plast = '\0';
            return ( buf );

         case '\n':
            ++plast;
            ++ln;
            *plast++ = '\r';
            ++ln;
            break;

         case '\r':
            break;

         case '~':
            *plast = '\0';
            return ( buf );
      }
   }
}

// The following 2 functions are taken from FreeBSD under the following license terms:

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 *
 * Renamed so it can play itself system independent.
 * Samson 10-12-03
 */
size_t strlcpy( char * __restrict dst, const char * __restrict src, size_t dsize )
{
   const char *osrc = src;
   size_t nleft = dsize;

   /* Copy as many bytes as will fit. */
   if( nleft != 0 )
   {
      while( --nleft != 0 )
      {
         if( ( *dst++ = *src++ ) == '\0' )
            break;
      }
   }

   /* Not enough room in dst, add NUL and traverse rest of src. */
   if( nleft == 0 ) 
   {
      if( dsize != 0 )
         *dst = '\0'; /* NUL-terminate dst */
      while( *src++ )
         ;
   }

   return( src - osrc - 1 ); /* count does not include NUL */
}

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 *
 * Renamed so it can play itself system independent.
 * Samson 10-12-03
 */
size_t strlcat( char * __restrict dst, const char * __restrict src, size_t dsize )
{
   const char *odst = dst;
   const char *osrc = src;
   size_t n = dsize;
   size_t dlen;

   /* Find the end of dst and adjust bytes left but don't go past end. */
   while( n-- != 0 && *dst != '\0' )
      dst++;

   dlen = dst - odst;
   n = dsize - dlen;

   if( n-- == 0 )
      return( dlen + strlen(src) );

   while( *src != '\0' )
   {
      if(n != 0 )
      {
         *dst++ = *src;
         n--;
      }
      src++;
   }
   *dst = '\0';

   return( dlen + (src - osrc) ); /* count does not include NUL */
}
