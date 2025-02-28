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

#include <stdlib.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <math.h>
#ifdef __cplusplus
#include <typeinfo>
#endif

using namespace std;

#define CODENAME "SWFotEFUSS"
#define CODEVERSION "1.5.3"

// Backward compatibility for snippets and such.
#define mudstrlcpy strlcpy
#define mudstrlcat strlcat
#define str_dup strdup

typedef int ch_ret;
typedef int obj_ret;

#ifdef __cplusplus
#define DECLARE_DO_FUN( fun )    extern "C" { DO_FUN    fun; } DO_FUN fun##_mangled
#define DECLARE_SPEC_FUN( fun )  extern "C" { SPEC_FUN  fun; } SPEC_FUN fun##_mangled
#define DECLARE_SPELL_FUN( fun ) extern "C" { SPELL_FUN fun; } SPELL_FUN fun##_mangled
#else
#define DECLARE_DO_FUN( fun )     DO_FUN    fun; DO_FUN fun##_mangled
#define DECLARE_SPEC_FUN( fun )   SPEC_FUN  fun; SPEC_FUN fun##_mangled
#define DECLARE_SPELL_FUN( fun )  SPELL_FUN fun; SPELL_FUN fun##_mangled
#endif

/* Stuff from newarena.c */
void show_jack_pot( void );
void do_game( void );
int num_in_arena( void );
void find_game_winner( void );
void do_end_game( void );
void start_game( void );
void silent_end( void );
void write_fame_list( void );
void load_hall_of_fame( void );

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types. [2025 here - this hasn't been an issue for 20 years now]
 */
const bool TRUE = true;
const bool FALSE = false;
const short BERR = 255;

#define KEY( literal, field, value )   \
if ( !str_cmp( word, (literal) ) )     \
{                                      \
   (field) = (value);                  \
   fMatch = TRUE;                      \
   break;                              \
}

/* Macro taken from DOTD codebase. Fcloses a file, then nulls its pointer for safety. */
#define FCLOSE(fp)  fclose((fp)); (fp)=NULL;

/*
 * Structure types.
 */
typedef struct affect_data AFFECT_DATA;
typedef struct area_data AREA_DATA;
typedef struct auction_data AUCTION_DATA; /* auction data */
typedef struct ban_data BAN_DATA;
typedef struct extracted_char_data EXTRACT_CHAR_DATA;
typedef struct bug_data BUG_DATA;
typedef struct contract_data CONTRACT_DATA;
typedef struct fellow_data FELLOW_DATA;
typedef struct char_data CHAR_DATA;
typedef struct hunt_hate_fear HHF_DATA;
typedef struct fighting_data FIGHT_DATA;
typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct exit_data EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data HELP_DATA;
typedef struct mob_index_data MOB_INDEX_DATA;
typedef struct note_data NOTE_DATA;
typedef struct board_data BOARD_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct pc_data PC_DATA;
typedef struct reset_data RESET_DATA;
typedef struct room_index_data ROOM_INDEX_DATA;
typedef struct shop_data SHOP_DATA;
typedef struct repairshop_data REPAIR_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct hour_min_sec HOUR_MIN_SEC;
typedef struct weather_data WEATHER_DATA;
typedef struct bounty_data BOUNTY_DATA;
typedef struct blackmarket_data BMARKET_DATA;
typedef struct cargo_data CARGO_DATA;
typedef struct planet_data PLANET_DATA;
typedef struct storeroom STOREROOM;
typedef struct guard_data GUARD_DATA;
typedef struct space_data SPACE_DATA;
typedef struct clan_data CLAN_DATA;
typedef struct senate_data SENATE_DATA;
typedef struct ship_data SHIP_DATA;
typedef struct module_data MODULE_DATA;
typedef struct hanger_data HANGER_DATA;
typedef struct turret_data TURRET_DATA;
typedef struct ship_prototype_data SHIP_PROTOTYPE;
typedef struct missile_data MISSILE_DATA;
typedef struct mob_prog_data MPROG_DATA;
typedef struct mob_prog_act_list MPROG_ACT_LIST;
typedef struct mpsleep_data MPSLEEP_DATA;
typedef struct editor_data EDITOR_DATA;
typedef struct teleport_data TELEPORT_DATA;
typedef struct timer_data TIMER;
typedef struct system_data SYSTEM_DATA;
typedef struct smaug_affect SMAUG_AFF;
typedef struct who_data WHO_DATA;
typedef struct skill_type SKILLTYPE;
typedef struct social_type SOCIALTYPE;
typedef struct cmd_type CMDTYPE;
typedef struct killed_data KILLED_DATA;
typedef struct wizent WIZENT;
typedef struct member_data MEMBER_DATA;   /* Individual member data */
typedef struct member_list MEMBER_LIST;   /* List of members in clan */
typedef struct membersort_data MS_DATA;   /* List for sorted roster list */
typedef struct specfun_list SPEC_LIST;

/*
 * Function types.
 */
typedef void DO_FUN( CHAR_DATA * ch, const char *argument );
typedef bool SPEC_FUN( CHAR_DATA * ch );
typedef ch_ret SPELL_FUN( int sn, int level, CHAR_DATA * ch, void *vo );

#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096 /* buf */
#define MAX_INPUT_LENGTH	 1024 /* arg */
#define MAX_INBUF_SIZE		 1024

#define MSL			MAX_STRING_LENGTH
#define MIL			MAX_INPUT_LENGTH

#define MAX_MOB_COUNT		10


#define HASHSTR   /* use string hashing */

#define	MAX_LAYERS		 8 /* maximum clothing layers */
#define MAX_NEST	       100  /* maximum container nesting */

#define MAX_KILLTRACK		20 /* track mob vnums killed */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH	       500000
#define MIN_EXP_WORTH		   25

#define MAX_REXITS		   20 /* Maximum exits allowed in 1 room */
#define MAX_SKILL		  282
#define MAX_ABILITY		   10
#define MAX_RL_ABILITY		    8
#define MAX_RACE		   41
#define MAX_PLANET_BASE_VALUE     100000
#define MAX_NPC_RACE		   91
#define MAX_LEVEL		   36
#define MAX_CLAN		   50
#define MAX_PLANET		  100
#define MAX_SHIP                 1000
#define MAX_SHIP_ROOMS             25
#define MAX_BOUNTY                255
#define MAX_GOV                   255

#define	MAX_HERB		   20

#define LEVEL_HERO		   (MAX_LEVEL - 4)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 4)
#define LEVEL_SUPREME		   MAX_LEVEL
#define LEVEL_INFINITE		   (MAX_LEVEL - 4)
#define LEVEL_ETERNAL		   (MAX_LEVEL - 4)
#define LEVEL_IMPLEMENTOR	   (MAX_LEVEL)
#define LEVEL_SUB_IMPLEM	   (MAX_LEVEL - 1)
#define LEVEL_ASCENDANT		   (MAX_LEVEL - 2)
#define LEVEL_GREATER		   (MAX_LEVEL - 3)
#define LEVEL_LESSER		   (MAX_LEVEL - 4)
#define LEVEL_RETIRED		   (MAX_LEVEL - 5)
#define LEVEL_GOD		   (MAX_LEVEL - 4)
#define LEVEL_TRUEIMM		   (MAX_LEVEL - 4)
#define LEVEL_DEMI		   (MAX_LEVEL - 4)
#define LEVEL_SAVIOR		   (MAX_LEVEL - 4)
#define LEVEL_CREATOR		   (MAX_LEVEL - 4)
#define LEVEL_ACOLYTE		   (MAX_LEVEL - 4)
#define LEVEL_NEOPHYTE		   (MAX_LEVEL - 4)
#define LEVEL_AVATAR		   (MAX_LEVEL - 5)

#include "dns.h"
#include "pfiles.h"
#include "color.h"
#include "hotboot.h"

#define LEVEL_LOG		    LEVEL_LESSER
#define LEVEL_HIGOD		    LEVEL_GOD

#define OBJ_VNUM_DEED		67 /* vnum of deed */

#define PULSE_PER_SECOND	    4
#define PULSE_MINUTE              ( 60 * PULSE_PER_SECOND)
#define PULSE_VIOLENCE		  (  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  (  4 * PULSE_PER_SECOND)
#define PULSE_TICK		  ( 70 * PULSE_PER_SECOND)
#define PULSE_AREA		  ( 60 * PULSE_PER_SECOND)
#define PULSE_AUCTION             ( 10 * PULSE_PER_SECOND)
#define PULSE_SPACE               ( 10 * PULSE_PER_SECOND)
#define PULSE_TAXES               ( 60 * PULSE_MINUTE)
#define PULSE_ARENA               (30 * PULSE_PER_SECOND)
#define PULSE_FORCE               PULSE_MINUTE

/* 
 * Stuff for area versions --Shaddai
 */
#define HAS_SPELL_INDEX     -1

/*
Old Smaug area version identifiers:

Version 1: Stock 1.4a areas.
Version 2: Skipped - Probably won't ever see these, but originated from Smaug 1.8.
Version 3: Stock 1.8 areas.
*/

// This value has been reset due to the new KEY/Value based area format.
// It will not conflict with the above former area file versions.
#define AREA_VERSION_WRITE 1

/*
 * Command logging types.
 */
typedef enum
{
   LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM, LOG_ALL
} log_types;

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
   rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
   rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
   rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
   rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE, rCHAR_AND_OBJ_EXTRACTED = 128,
   rERROR = 255
} ret_types;

/* Begin new force defines */
typedef enum
{
   FORCE_INROOM, FORCE_ANYWHERE
} force_locations;

typedef enum
{
   FORCE_SKILL_REFRESH, FORCE_SKILL_FINFO, FORCE_SKILL_STUDENT, FORCE_SKILL_MASTER, FORCE_SKILL_IDENTIFY,
   FORCE_SKILL_PROMOTE, FORCE_SKILL_INSTRUCT, FORCE_SKILL_HEAL, FORCE_SKILL_PROTECT, FORCE_SKILL_SHIELD,
   FORCE_SKILL_WHIRLWIND, FORCE_SKILL_STRIKE, FORCE_SKILL_SQUEEZE, FORCE_SKILL_FORCE_LIGHTNING,
   FORCE_SKILL_DISGUISE, FORCE_SKILL_MAKELIGHTSABER, FORCE_SKILL_PARRY, FORCE_SKILL_FINISH,
   FORCE_SKILL_FHELP, FORCE_SKILL_DUALLIGHTSABER, FORCE_SKILL_REFLECT, FORCE_SKILL_CONVERT,
   FORCE_SKILL_MAKEDUALSABER, FORCE_SKILL_AWARENESS
} force_skills_type;

typedef enum
{
   FORCE_NONCOMBAT, FORCE_COMBAT, FORCE_NORESTRICT
} force_skill_types;

typedef enum
{
   FORCE_NONE, FORCE_APPRENTICE, FORCE_KNIGHT, FORCE_MASTER
} force_level_type;

typedef enum
{
   FORCE_GENERAL, FORCE_JEDI, FORCE_SITH
} force_skills_class;

#define MAX_FORCE_SKILL 24

typedef struct force_skills_struct FORCE_SKILL;

struct force_skills_struct
{
   int type;
   int index;
   char *name;
   char *room_effect[5];
   char *victim_effect[5];
   char *ch_effect[5];
   int cost;
   int control;
   int alter;
   int sense;
   char *code;
   int status;
   int wait_state;
   int disabled;
   int notskill;
   int mastertrain;
   DO_FUN *do_fun;
   FORCE_SKILL *next;
   FORCE_SKILL *prev;
};

extern FORCE_SKILL *first_force_skill;
extern FORCE_SKILL *last_force_skill;

#define MAX_FORCE_ALIGN 100
#define MIN_FORCE_ALIGN -100

typedef struct force_help_struct FORCE_HELP;

struct force_help_struct
{
   char *name;
   int status;
   int type;
   char *desc;
   int skill;
   FORCE_HELP *next;
   FORCE_HELP *prev;
};

extern FORCE_HELP *first_force_help;
extern FORCE_HELP *last_force_help;

/* End force defines */

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2

/* short cut crash bug fix provided by gfinello@mail.karmanet.it*/
typedef enum
{
   relMSET_ON, relOSET_ON
} relation_type;

typedef struct rel_data REL_DATA;

struct rel_data
{
   void *Actor;
   void *Subject;
   REL_DATA *next;
   REL_DATA *prev;
   relation_type Type;
};

/* defines for new do_who */
#define WT_MORTAL 0
#define WT_IMM    2
#define WT_AVATAR 1
#define WT_NEWBIE 3

/*
 * do_who output structure -- Narn
 */
struct who_data
{
   WHO_DATA *prev;
   WHO_DATA *next;
   char *text;
   int type;
};

/*
 * Site ban structure.
 */
struct ban_data
{
   BAN_DATA *next;
   BAN_DATA *prev;
   char *name;
   int level;
   char *ban_time;
};


/*
 * Time and weather stuff.
 */
typedef enum
{
   SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
   SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

struct time_info_data
{
   int hour;
   int day;
   int month;
   int year;
};

struct hour_min_sec
{
   int hour;
   int min;
   int sec;
   int manual;
};

struct weather_data
{
   int mmhg;
   int change;
   int sky;
   int sunlight;
};

/*
 * Structure used to build wizlist
 */
struct wizent
{
   WIZENT *next;
   WIZENT *last;
   char *name;
   short level;
};

/*
 * Connected state for a channel.
 */
typedef enum
{
   CON_GET_NAME = -99, CON_GET_OLD_PASSWORD,
   CON_CONFIRM_NEW_NAME, CON_GET_NEW_PASSWORD, CON_CONFIRM_NEW_PASSWORD,
   CON_GET_NEW_SEX, CON_READ_MOTD,
   CON_GET_NEW_RACE, CON_GET_EMULATION,
   CON_GET_WANT_RIPANSI, CON_TITLE, CON_PRESS_ENTER,
   CON_WAIT_1, CON_WAIT_2, CON_WAIT_3,
   CON_ACCEPTED, CON_GET_PKILL, CON_READ_IMOTD,
   CON_GET_NEW_EMAIL, CON_GET_MSP, CON_GET_NEW_CLASS,
   CON_GET_NEW_SECOND, CON_ROLL_STATS, CON_STATS_OK,
   CON_GET_PUEBLO, CON_GET_HEIGHT, CON_GET_BUILD,
   CON_GET_DROID,
   CON_COPYOVER_RECOVER, CON_PLAYING = 0, CON_EDITING
} connection_types;

/*
 * Character substates
 */
typedef enum
{
   SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_OBJ_SHORT, SUB_OBJ_LONG,
   SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC, SUB_ROOM_EXTRA,
   SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, SUB_HELP_EDIT,
   SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD, SUB_RESTRICTED,
   SUB_DEITYDESC, SUB_SHIPDESC, SUB_FORCE_CH0, SUB_FORCE_CH1, SUB_FORCE_CH2,
   SUB_FORCE_CH3, SUB_FORCE_CH4, SUB_FORCE_ROOM0, SUB_FORCE_ROOM1, SUB_FORCE_ROOM2,
   SUB_FORCE_ROOM3, SUB_FORCE_ROOM4, SUB_FORCE_VICTIM0, SUB_FORCE_VICTIM1, SUB_FORCE_VICTIM2,
   SUB_FORCE_VICTIM3, SUB_FORCE_VICTIM4, SUB_FORCE_HELP,

   /*
    * timer types ONLY below this point 
    */
   SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
   DESCRIPTOR_DATA *next;
   DESCRIPTOR_DATA *prev;
   DESCRIPTOR_DATA *snoop_by;
   CHAR_DATA *character;
   CHAR_DATA *original;
   struct mccp_data *mccp; /* Mud Client Compression Protocol */
   bool can_compress;
   char *host;
   char *hostip;
   int port;
   int descriptor;
   short connected;
   short idle;
   short lines;
   short scrlen;
   bool fcommand;
   char inbuf[MAX_INBUF_SIZE];
   char incomm[MAX_INPUT_LENGTH];
   char inlast[MAX_INPUT_LENGTH];
   int repeat;
   char *outbuf;
   unsigned long outsize;
   int outtop;
   char *pagebuf;
   unsigned long pagesize;
   int pagetop;
   char *pagepoint;
   char pagecmd;
   char pagecolor;
   int newstate;
   unsigned char prevcolor;
   int ifd;
   pid_t ipid;
};

/*
 * Attribute bonus structures.
 */
struct str_app_type
{
   short tohit;
   short todam;
   short carry;
   short wield;
};

struct int_app_type
{
   short learn;
};

struct wis_app_type
{
   short practice;
};

struct dex_app_type
{
   short defensive;
};

struct con_app_type
{
   short hitp;
   short shock;
};

struct cha_app_type
{
   short charm;
};

struct lck_app_type
{
   short luck;
};

struct frc_app_type
{
   short force;
};

/* ability classes */

#define ABILITY_NONE		-1
#define COMBAT_ABILITY 		0
#define PILOTING_ABILITY	1
#define ENGINEERING_ABILITY	2
#define HUNTING_ABILITY		3
#define SMUGGLING_ABILITY	4
/*#define DIPLOMACY_ABILITY	5
#define LEADERSHIP_ABILITY	6*/
/*
 * Gonna replace the diplomacy and leadership abilities and make them POLITICIANs 
*/
#define POLITICIAN_ABILITY	5
#define FORCE_ABILITY		6
#define SLICER_ABILITY		7
#define	ASSASSIN_ABILITY	8
#define	TECHNICIAN_ABILITY	9


/* the races */
#define RACE_HUMAN	        0
#define RACE_WOOKIEE		1
#define RACE_TWI_LEK		2
#define RACE_RODIAN		3
#define RACE_HUTT		4
#define RACE_MON_CALAMARI	5
#define RACE_NOGHRI		6
#define RACE_GAMORREAN		7
#define RACE_JAWA		8
#define RACE_ADARIAN            9
#define RACE_EWOK              10
#define RACE_VERPINE           11
#define RACE_DEFEL             12
#define RACE_TRANDOSHAN        13
#define RACE_HAPAN	       14
#define RACE_QUARREN           15
#define RACE_SHISTAVANEN       16
#define RACE_FALLEEN           17
#define RACE_ITHORIAN          18
#define RACE_DEVARONIAN        19
#define RACE_GOTAL             20
#define RACE_DROID             21
#define RACE_FIRRERREO         22
#define RACE_BARABEL           23
#define RACE_BOTHAN            24
#define RACE_TOGORIAN          25
#define RACE_DUG               26
#define RACE_KUBAZ             27
#define RACE_SELONIAN          28
#define RACE_GRAN              29
#define RACE_YEVETHA           30
#define RACE_GAND              31
#define RACE_DUROS             32
#define RACE_COYNITE           33
#define RACE_SULLUSTAN         34
#define RACE_PROTOCAL_DROID    35
#define RACE_ASSASSIN_DROID    36
#define RACE_GLADIATOR_DROID   37
#define RACE_ASTROMECH_DROID   38
#define RACE_INTERROGATION_DROID 39


/*
 * Languages -- Altrag
 */
#define LANG_BASIC      BV00  /* Human base language */
#define LANG_WOOKIEE     BV01
#define LANG_TWI_LEK     BV02
#define LANG_RODIAN      BV03
#define LANG_HUTT        BV04
#define LANG_MON_CALAMARI   BV05
#define LANG_NOGHRI      BV06
#define LANG_EWOK        BV07
#define LANG_ITHORIAN    BV08
#define LANG_GOTAL       BV09
#define LANG_DEVARONIAN  BV10
#define LANG_BINARY      BV11
#define LANG_FIRRERREO   BV12
#define LANG_CLAN        BV13
#define LANG_GAMORREAN   BV14
#define LANG_TOGORIAN    BV15
#define LANG_SHISTAVANEN BV16
#define LANG_JAWA        BV17
#define LANG_KUBAZ	 BV18
#define LANG_ADARIAN	 BV19
#define LANG_VERPINE	 BV20
#define LANG_DEFEL       BV21
#define LANG_TRANDOSHAN  BV22
#define LANG_HAPAN	 BV23
#define LANG_QUARREN     BV24
#define LANG_SULLUSTAN   BV25
#define LANG_FALLEEN     BV26
#define LANG_BARABEL     BV27
#define LANG_YEVETHAN    BV28
#define LANG_GAND        BV29
#define LANG_DUROS       BV30
#define LANG_COYNITE     BV31
#define LANG_UNKNOWN        0 /* Anything that doesnt fit a category */
#define VALID_LANGS    ( LANG_BASIC | LANG_WOOKIEE | LANG_TWI_LEK | LANG_RODIAN  \
		       | LANG_HUTT | LANG_MON_CALAMARI | LANG_NOGHRI | LANG_GAMORREAN \
		       | LANG_JAWA | LANG_ADARIAN | LANG_EWOK | LANG_VERPINE | LANG_DEFEL \
		       | LANG_TRANDOSHAN | LANG_HAPAN | LANG_QUARREN | LANG_SULLUSTAN | LANG_BINARY \
		       | LANG_FIRRERREO | LANG_CLAN | LANG_TOGORIAN | LANG_SHISTAVANEN \
		       | LANG_KUBAZ | LANG_YEVETHAN | LANG_GAND | LANG_DUROS | LANG_COYNITE \
		       | LANG_GOTAL | LANG_DEVARONIAN | LANG_FALLEEN | LANG_ITHORIAN | LANG_BARABEL )
/*  32 Languages */

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_MUD		    4

#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 30

/*
 * Help table types.
 */
struct help_data
{
   HELP_DATA *next;
   HELP_DATA *prev;
   short level;
   char *keyword;
   char *text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct shop_data
{
   SHOP_DATA *next;  /* Next shop in list    */
   SHOP_DATA *prev;  /* Previous shop in list   */
   int keeper; /* Vnum of shop keeper mob */
   short buy_type[MAX_TRADE]; /* Item types shop will buy   */
   short profit_buy; /* Cost multiplier for buying */
   short profit_sell;   /* Cost multiplier for selling   */
   short open_hour;  /* First opening hour      */
   short close_hour; /* First closing hour      */
};

#define MAX_FIX		3
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct repairshop_data
{
   REPAIR_DATA *next;   /* Next shop in list    */
   REPAIR_DATA *prev;   /* Previous shop in list   */
   int keeper; /* Vnum of shop keeper mob */
   short fix_type[MAX_FIX];   /* Item types shop will fix   */
   short profit_fix; /* Cost multiplier for fixing */
   short shop_type;  /* Repair shop type     */
   short open_hour;  /* First opening hour      */
   short close_hour; /* First closing hour      */
};


/* Mob program structures */

/* Mob program structures and defines */
/* Moved these defines here from mud_prog.c as I need them -rkb */
#define MAX_IFS 20   /* should always be generous */
#define IN_IF 0
#define IN_ELSE 1
#define DO_IF 2
#define DO_ELSE 3

#define MAX_PROG_NEST 20

struct act_prog_data
{
   struct act_prog_data *next;
   void *vo;
};

struct mob_prog_act_list
{
   MPROG_ACT_LIST *next;
   char *buf;
   CHAR_DATA *ch;
   OBJ_DATA *obj;
   void *vo;
};

struct mob_prog_data
{
   MPROG_DATA *next;
   int type;
   bool triggered;
   int resetdelay;
   char *arglist;
   char *comlist;
   bool fileprog;
};

/* Used to store sleeping mud progs. -rkb */
typedef enum
{ MP_MOB, MP_ROOM, MP_OBJ } mp_types;
struct mpsleep_data
{
   MPSLEEP_DATA *next;
   MPSLEEP_DATA *prev;

   int timer;  /* Pulses to sleep */
   mp_types type; /* Mob, Room or Obj prog */
   ROOM_INDEX_DATA *room;  /* Room when type is MP_ROOM */

   /*
    * mprog_driver state variables 
    */
   int ignorelevel;
   int iflevel;
   bool ifstate[MAX_IFS][DO_ELSE];

   /*
    * mprog_driver arguments 
    */
   char *com_list;
   CHAR_DATA *mob;
   CHAR_DATA *actor;
   OBJ_DATA *obj;
   void *vo;
   bool single_step;
};


extern bool MOBtrigger;

/* race dedicated stuff */
struct race_type
{
   const char *race_name;  /* Race name         */
   int affected;  /* Default affect bitvectors  */
   short str_plus;   /* Str bonus/penalty    */
   short dex_plus;   /* Dex      "        */
   short wis_plus;   /* Wis      "        */
   short int_plus;   /* Int      "        */
   short con_plus;   /* Con      "        */
   short cha_plus;   /* Cha      "        */
   short lck_plus;   /* Lck       "       */
   short frc_plus;   /* Frc       "       */
   short hit;
   short mana;
   short resist;
   short suscept;
   int class_restriction;  /* Flags for illegal classes  */
   int language;  /* Default racial language      */
};

typedef enum
{
   CLAN_PLAIN, CLAN_CRIME, CLAN_GUILD, CLAN_SUBCLAN, CLAN_CORPORATION
} clan_types;


typedef enum
{ PLAYER_SHIP, MOB_SHIP, SHIP_IMPERIAL, SHIP_REPUBLIC } ship_types;
typedef enum
{ SHIP_DOCKED, SHIP_READY, SHIP_BUSY, SHIP_BUSY_2, SHIP_BUSY_3, SHIP_REFUEL,
   SHIP_LAUNCH, SHIP_LAUNCH_2, SHIP_LAND, SHIP_LAND_2, SHIP_HYPERSPACE, SHIP_DISABLED, SHIP_FLYING
} ship_states;
typedef enum
{ MISSILE_READY, MISSILE_FIRED, MISSILE_RELOAD, MISSILE_RELOAD_2, MISSILE_DAMAGED } missile_states;

typedef enum
{
   LAND_VEHICLE, SHIP_FIGHTER, SHIP_BOMBER, SHIP_SHUTTLE, SHIP_FREIGHTER, SHIP_FRIGATE, SHIP_TT,
   SHIP_CORVETTE, SHIP_CRUISER, SHIP_DREADNAUGHT, SHIP_DESTROYER, SHIP_SPACE_STATION
} ship_classes;

typedef enum
{
   // 0         1            2           3          4            5
   B_NONE, SINGLE_LASER, DUAL_LASER, TRI_LASER, QUAD_LASER, AUTOBLASTER,

   //   6           7           8            9
   HEAVY_LASER, LIGHT_ION, REPEATING_ION, HEAVY_ION
} beam_types;


typedef enum
{ CONCUSSION_MISSILE, PROTON_TORPEDO, HEAVY_ROCKET, HEAVY_BOMB } missile_types;

#define LASER_DAMAGED    -1
#define LASER_READY       0

struct planet_data
{
   PLANET_DATA *next;
   PLANET_DATA *prev;
   PLANET_DATA *next_in_system;
   PLANET_DATA *prev_in_system;
   AREA_DATA *next_in_area;
   AREA_DATA *prev_in_area;
   GUARD_DATA *first_guard;
   AREA_DATA *first_area;
   AREA_DATA *last_area;
   GUARD_DATA *last_guard;
   SPACE_DATA *starsystem;
   AREA_DATA *area;
   char *name;
   char *filename;
   CLAN_DATA *governed_by;
   int population;
   float pop_support;
   int sector;
   int x, y, z;
   int size;
   bool flags;
   int base_value;
   int citysize;
   int wilderness;
   int wildlife;
   int farmland;
   int barracks;
   int controls;
};

struct cargo_data
{
   int cargo0;
   int cargo1;
   int cargo2;
   int cargo3;
   int cargo4;
   int cargo5;
   int cargo6;
   int cargo7;
   int cargo8;
   int cargo9;
   int orgcargo0;
   int orgcargo1;
   int orgcargo2;
   int orgcargo3;
   int orgcargo4;
   int orgcargo5;
   int orgcargo6;
   int orgcargo7;
   int orgcargo8;
   int orgcargo9;
   int price0;
   int price1;
   int price2;
   int price3;
   int price4;
   int price5;
   int price6;
   int price7;
   int price8;
   int price9;
   bool smug;
};

struct space_data
{
   SPACE_DATA *next;
   SPACE_DATA *prev;
   SHIP_DATA *first_ship;
   SHIP_DATA *last_ship;
   MISSILE_DATA *first_missile;
   MISSILE_DATA *last_missile;
   PLANET_DATA *first_planet;
   PLANET_DATA *last_planet;
   char *filename;
   char *name;
   char *star1;
   char *star2;
   char *planet1;
   char *planet2;
   char *planet3;
   char *location1a;
   char *location2a;
   char *location3a;
   char *location1b;
   char *location2b;
   char *location3b;
   char *location1c;
   char *location2c;
   char *location3c;
   int xpos;
   int ypos;
   int zpos;
   int s1x;
   int s1y;
   int s1z;
   int s2x;
   int s2y;
   int s2z;
   int doc1a;
   int doc2a;
   int doc3a;
   int doc1b;
   int doc2b;
   int doc3b;
   int doc1c;
   int doc2c;
   int doc3c;
   bool seca;
   bool secb;
   bool secc;
   int p1x;
   bool trainer;
   int p1y;
   int p1z;
   int p2x;
   int p2y;
   int p2z;
   int p3x;
   int p3y;
   int p3z;
   int gravitys1;
   int gravitys2;
   int gravityp1;
   int gravityp2;
   int gravityp3;
   int p1_low;
   int p1_high;
   int p2_low;
   int p2_high;
   int p3_low;
   int p3_high;
   int crash;
};

struct bounty_data
{
   BOUNTY_DATA *next;
   BOUNTY_DATA *prev;
   char *target;
   long int amount;
};

struct blackmarket_data
{
   BMARKET_DATA *next;
   BMARKET_DATA *prev;
   char *filename;
   int quantity;
};

struct guard_data
{
   GUARD_DATA *next;
   GUARD_DATA *prev;
   GUARD_DATA *next_on_planet;
   GUARD_DATA *prev_on_planet;
   CHAR_DATA *mob;
   ROOM_INDEX_DATA *reset_loc;
   PLANET_DATA *planet;
};

struct senate_data
{
   SENATE_DATA *next;
   SENATE_DATA *prev;
   char *name;
};

#define PLANET_NOCAPTURE  BV00

struct clan_data
{
   CLAN_DATA *next;  /* next clan in list       */
   CLAN_DATA *prev;  /* previous clan in list      */
   CLAN_DATA *next_subclan;
   CLAN_DATA *prev_subclan;
   CLAN_DATA *first_subclan;
   CLAN_DATA *last_subclan;
   CLAN_DATA *mainclan;
   char *acro;
   char *filename;   /* Clan filename        */
   char *shortname;  /* Clan shortname - used in member lists */
   char *name; /* Clan name            */
   char *description;   /* A brief description of the clan  */
   char *leader;  /* Head clan leader        */
   char *number1; /* First officer        */
   char *number2; /* Second officer       */
   int pkills; /* Number of pkills on behalf of clan  */
   int pdeaths;   /* Number of pkills against clan */
   int mkills; /* Number of mkills on behalf of clan  */
   int mdeaths;   /* Number of clan deaths due to mobs   */
   short clan_type;  /* See clan type defines      */
   char *atwar;   /* Clan name            */
   short members; /* Number of clan members     */
   int board;  /* Vnum of clan board         */
   int storeroom; /* Vnum of clan's store room     */
   int guard1; /* Vnum of clan guard type 1     */
   int guard2; /* Vnum of clan guard type 2     */
   int patrol1;   /* vnum of patrol */
   int patrol2;   /* vnum of patrol */
   int trooper1;  /* vnum of reinforcements */
   int trooper2;  /* vnum of elite troopers */
   long int funds;
   int spacecraft;
   int troops;
   int vehicles;
   int jail;
   char *tmpstr;
};

struct ship_prototype_data
{
   SHIP_PROTOTYPE *next;
   SHIP_PROTOTYPE *prev;
   char *filename;
   char *name;
   char *description;
   short sclass;
   short model;
   short hyperspeed;
   short realspeed;
   short maxbombs;
   short maxmissiles;
   short maxtorpedos;
   short maxrockets;
   int max_modules;  // This is used to set what the maximum number of upgrade modules is. If none is set 10 is.
   short lasers;
   CLAN_DATA *clan;  // This is used to limit certain prototypes to a specific clan.
   short tractorbeam;
   short manuever;
   int weight;
   int maxenergy;
   int maxshield;
   int maxhull;
   short maxchaff;
   short maxmods;
};

struct turret_data
{
   TURRET_DATA *next;
   TURRET_DATA *prev;
   ROOM_INDEX_DATA *room;
   SHIP_DATA *target;
   short laserstate;
};

struct hanger_data
{
   HANGER_DATA *next;
   HANGER_DATA *prev;
   ROOM_INDEX_DATA *room;
   bool bayopen;
   int type;
};

struct module_data
{
   MODULE_DATA *next;
   MODULE_DATA *prev;
   int affect; // What item is it going to affect.
   int ammount;   // How much is it going to affect it.
};

struct ship_data
{
   SHIP_DATA *next;
   SHIP_DATA *prev;
   SHIP_DATA *next_in_starsystem;
   SHIP_DATA *prev_in_starsystem;
   SHIP_DATA *next_in_room;
   SHIP_DATA *prev_in_room;
   ROOM_INDEX_DATA *in_room;
   SPACE_DATA *starsystem;
   SHIP_DATA *inship;
   char *filename;
   char *name;
   char *protoname;
   char *clanowner;
   char *home;
   char *description;
   char *owner;
   char *pilot;
   char *copilot;
   char *dest;
   char *pbeacon;
   short type;
   short sclass;
   short comm;
   int cost;
   short sensor;
   short astro_array;
   short hyperspeed;
   int hyperdistance;
   short realspeed;
   short currspeed;
   short shipstate;
   short hyperstate;

/* New ship shit by || && Tawnos for FotE */

   bool juking;
   bool rolling;
   short primaryState;  // (was statet0) Primary beam state   (Damaged/charging)
   short secondaryState;   // (was statet0i) Secondary beam state

   short primaryType;   // Primary weapon type, defined in beam_types
   short secondaryType; // Secondary weapon type, defined in beam_types

   bool primaryLinked;  // Linked fire, if !single can fire all available (up to 4 at once)
   bool secondaryLinked;   // Linked fire, if !single will fire all available
   bool warheadLinked;  // Linked fire, if !single will fire all available

   short primaryCount;  // (was lasers) Number of primaries
   short secondaryCount;   // (was ions) Number of secondaries

   short statet1; // Begin turbolaser turret states
   short statet2;
   short statet3;
   short statet4;
   short statet5;
   short statet6;
   short statet7;
   short statet8;
   short statet9;
   short statet10;   // End turbolaser turret states

   short missiletype;

   short missilestate;
   short torpedostate;
   short rocketstate;

   short bombs;
   short maxbombs;
   short alarm;
   short missiles;
   short maxmissiles;
   short torpedos;
   short maxtorpedos;
   short rockets;
   short maxrockets;
   short maxmod;
   short tractorbeam;
   short manuever;
   bool bayopen;
   bool hatchopen;
   bool autorecharge;
   bool autotrack;
   bool autospeed;
   float vx, vy, vz;
   float hx, hy, hz;
   float jx, jy, jz;
   int maxenergy;
   int energy;
   int shield;
   int maxshield;
   int hull;
   int maxhull;
   int cockpit;
   int turret1;
   int turret2;
   int turret3;
   int turret4;
   int turret5;
   int turret6;
   int turret7;
   int turret8;
   int turret9;
   int turret10;
   int location;
   int lastdoc;
   int shipyard;
   int entrance;
   int engineroom;
   int firstroom;
   int lastroom;
   int navseat;
   int pilotseat;
   int coseat;
   int gunseat;
   long collision;
   SHIP_DATA *target0;
   SHIP_DATA *target1;
   SHIP_DATA *target2;
   SHIP_DATA *target3;
   SHIP_DATA *target4;
   SHIP_DATA *target5;
   SHIP_DATA *target6;
   SHIP_DATA *target7;
   SHIP_DATA *target8;
   SHIP_DATA *target9;
   SHIP_DATA *target10;
   SPACE_DATA *currjump;
   short chaff;
   short maxchaff;
   short chaff_released;
   bool autopilot;
   int channel;
   int password;
   int flags;
   MODULE_DATA *first_module;
   MODULE_DATA *last_module;
   short maxmods;
   TURRET_DATA *first_turret;
   TURRET_DATA *last_turret;
   int hanger1;
   int hanger2;
   int hanger3;
   int hanger4;
   int exlocation;
   int sim_vnum;
   int max_modules;
   int baycode;
   int hibombstr;
   int lowbombstr;
   SHIP_DATA *tractored_by;
   SHIP_DATA *tractoring;

};

struct missile_data
{
   MISSILE_DATA *next;
   MISSILE_DATA *prev;
   MISSILE_DATA *next_in_starsystem;
   MISSILE_DATA *prev_in_starsystem;
   SPACE_DATA *starsystem;
   SHIP_DATA *target;
   SHIP_DATA *fired_from;
   char *fired_by;
   short missiletype;
   short age;
   int speed;
   int mx, my, mz;
};

struct tourney_data
{
   int open;
   int low_level;
   int hi_level;
};

/*
 * Data structure for notes.
 */
struct note_data
{
   NOTE_DATA *next;
   NOTE_DATA *prev;
   char *sender;
   char *date;
   char *to_list;
   char *subject;
   int voting;
   char *yesvotes;
   char *novotes;
   char *abstentions;
   char *text;
};

struct board_data
{
   BOARD_DATA *next; /* Next board in list         */
   BOARD_DATA *prev; /* Previous board in list     */
   NOTE_DATA *first_note;  /* First note on board        */
   NOTE_DATA *last_note;   /* Last note on board         */
   char *note_file;  /* Filename to save notes to     */
   char *read_group; /* Can restrict a board to a       */
   char *post_group; /* council, clan, guild etc        */
   char *extra_readers; /* Can give read rights to players */
   char *extra_removers;   /* Can give remove rights to players */
   int board_obj; /* Vnum of board object       */
   short num_posts;  /* Number of notes on this board   */
   short min_read_level;   /* Minimum level to read a note     */
   short min_post_level;   /* Minimum level to post a note    */
   short min_remove_level; /* Minimum level to remove a note  */
   short max_posts;  /* Maximum amount of notes allowed */
   int type;   /* Normal board or mail board? */
};


/*
 * An affect.
 */
struct affect_data
{
   AFFECT_DATA *next;
   AFFECT_DATA *prev;
   short type;
   int duration;
   short location;
   int modifier;
   int bitvector;
};


/*
 * A SMAUG spell
 */
struct smaug_affect
{
   SMAUG_AFF *next;
   SMAUG_AFF *prev;
   char *duration;
   short location;
   char *modifier;
   int bitvector;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_ANIMATED_CORPSE   5
#define MOB_VNUM_POLY_WOLF	   10

#define MOB_VNUM_STORMTROOPER	20
#define MOB_VNUM_IMP_GUARD	21
#define MOB_VNUM_NR_GUARD	22
#define MOB_VNUM_NR_TROOPER	23
#define MOB_VNUM_MERCINARY	24
#define MOB_VNUM_BOUNCER	25
#define MOB_VNUM_IMP_ELITE 	26
#define MOB_VNUM_IMP_PATROL 	27
#define MOB_VNUM_IMP_FORCES 	28
#define MOB_VNUM_NR_ELITE 	29
#define MOB_VNUM_NR_PATROL 	30
#define MOB_VNUM_NR_FORCES 	31
#define MOB_VNUM_MERC_ELITE 	32
#define MOB_VNUM_MERC_PATROL 	33
#define MOB_VNUM_MERC_FORCES 	34
#define MOB_VNUM_SHIP_GUARD	35
#define MOB_VNUM_SUPERMOB 3

/* Ship Flags */
#define SHIP_NOHIJACK           BV00
#define SHIP_SHIELD_BOOST	BV01
#define SHIP_TORP_BOOST		BV02
#define SHIP_CHAFF_BOOST	BV03
#define SHIP_HULL_BOOST		BV04
#define SHIP_LASER_BOOST	BV05
#define SHIP_MISSILE_BOOST	BV06
#define SHIP_ROCKET_BOOST	BV07
#define SHIP_SIMULATOR		BV08
#define SHIP_NODESTROY		BV09
#define SHIP_NOSLICER		BV10
#define XSHIP_ION_LASERS	BV11
#define XSHIP_ION_DRIVE		BV12
#define XSHIP_ION_ION		BV13
#define XSHIP_ION_TURRET1        BV14
#define XSHIP_ION_TURRET2        BV15
#define XSHIP_ION_TURRET3        BV16
#define XSHIP_ION_TURRET4        BV17
#define XSHIP_ION_TURRET5        BV18
#define XSHIP_ION_TURRET6        BV19
#define XSHIP_ION_TURRET7        BV20
#define XSHIP_ION_TURRET8        BV21
#define XSHIP_ION_TURRET9        BV22
#define XSHIP_ION_TURRET10       BV23
#define SHIP_RESPAWN            BV24
#define XSHIP_ION_HYPER		BV25
#define XSHIP_ION_MISSILES	BV26
#define SHIP_CLOAK		BV27

#define SHIP_DAMAGE_DRIVE         BV00
#define SHIP_DAMAGE_HYPERDRIVE    BV01
#define SHIP_DAMAGE_LASER         BV02
#define SHIP_DAMAGE_ION           BV03
#define SHIP_DAMAGE_TURRET1       BV04
#define SHIP_DAMAGE_TURRET2       BV05
#define SHIP_DAMAGE_TURRET3       BV06
#define SHIP_DAMAGE_TURRET4       BV07
#define SHIP_DAMAGE_TURRET5       BV08
#define SHIP_DAMAGE_TURRET6       BV09
#define SHIP_DAMAGE_TURRET7       BV10
#define SHIP_DAMAGE_TURRET8       BV11
#define SHIP_DAMAGE_TURRET9       BV12
#define SHIP_DAMAGE_TURRET10      BV13
#define SHIP_DAMAGE_SHIELD        BV14
#define SHIP_DAMAGE_PLASMASHIELD  BV15
#define SHIP_DAMAGE_LIFESUPPORT   BV16
#define SHIP_DAMAGE_MISSILE       BV17

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		 BV00 /* Auto set for mobs */
#define ACT_SENTINEL		 BV01 /* Stays in one room */
#define ACT_SCAVENGER		 BV02 /* Picks up objects  */
#define ACT_NOFLEE		 BV03 /* Mobs don't flee. -T  */
#define ACT_AGGRESSIVE		 BV05 /* Attacks PC's      */
#define ACT_STAY_AREA		 BV06 /* Won't leave area  */
#define ACT_WIMPY		 BV07 /* Flees when hurt   */
#define ACT_PET			 BV08 /* Auto set for pets */
#define ACT_TRAIN		 BV09 /* Can train PC's */
#define ACT_PRACTICE		 BV10 /* Can practice PC's */
#define ACT_IMMORTAL		 BV11 /* Cannot be killed  */
#define ACT_DEADLY		 BV12 /* Has a deadly poison  */
#define ACT_POLYSELF		 BV13
#define ACT_META_AGGR		 BV14 /* Extremely aggressive */
#define ACT_GUARDIAN		 BV15 /* Protects master   */
#define ACT_RUNNING		 BV16 /* Hunts quickly  */
#define ACT_NOWANDER		 BV17 /* Doesn't wander */
#define ACT_MOUNTABLE		 BV18 /* Can be mounted */
#define ACT_MOUNTED		 BV19 /* Is mounted     */
#define ACT_SCHOLAR              BV20  /* Can teach languages  */
#define ACT_SECRETIVE		 BV21 /* actions aren't seen  */
#define ACT_POLYMORPHED		 BV22 /* Mob is a ch    */
#define ACT_MOBINVIS		 BV23 /* Like wizinvis  */
#define ACT_NOASSIST		 BV24 /* Doesn't assist mobs  */
#define ACT_NOKILL               BV25  /* Mob can't die */
#define ACT_DROID                BV26  /* mob is a droid */
#define ACT_NOCORPSE             BV27
#define ACT_PUEBLO		 BV28 /* This is the pueblo flag */
#define ACT_PROTOTYPE		 BV30 /* A prototype mob   */

/* Act2 Flags */
#define	ACT_BOUND		 BV00 /* This is the bind flag */
#define ACT_JEDI		 BV01 /* This is a light jedi */
#define	ACT_SITH		 BV02 /* This is a dark jedi */
#define	ACT_GAGGED		 BV03 /* This is a gagged flag */
/* 21 acts */

/* bits for vip flags */

#define VIP_CORUSCANT           BV00
#define VIP_YAVIN_IV		BV01
#define VIP_TATOOINE            BV02
#define VIP_KASHYYYK          	BV03
#define VIP_MON_CALAMARI       	BV04
#define VIP_ENDOR		BV05
#define VIP_ORD_MANTELL         BV06
#define VIP_NAL_HUTTA           BV07
#define VIP_CORELLIA            BV08
#define VIP_BAKURA	        BV09

/* player wanted bits */

#define WANTED_CORUSCANT   	VIP_CORUSCANT
#define WANTED_YAVIN_IV		VIP_YAVIN_IV
#define WANTED_TATOOINE   	VIP_TATOOINE
#define WANTED_KASHYYYK   	VIP_KASHYYYK
#define WANTED_MON_CALAMARI   	VIP_MON_CALAMARI
#define WANTED_ENDOR		VIP_ENDOR
#define WANTED_ORD_MANTELL   	VIP_ORD_MANTELL
#define WANTED_NAL_HUTTA   	VIP_NAL_HUTTA
#define WANTED_CORELLIA   	VIP_CORELLIA
#define WANTED_BAKURA   	VIP_BAKURA

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_NONE                  0

#define AFF_BLIND		  BV00
#define AFF_INVISIBLE		  BV01
#define AFF_DETECT_EVIL		  BV02
#define AFF_DETECT_INVIS	  BV03
#define AFF_DETECT_MAGIC	  BV04
#define AFF_DETECT_HIDDEN	  BV05
#define AFF_WEAKEN		  BV06
#define AFF_SANCTUARY		  BV07
#define AFF_FAERIE_FIRE		  BV08
#define AFF_INFRARED		  BV09
#define AFF_CURSE		  BV10
#define AFF_COVER_TRAIL		  BV11
#define AFF_POISON		  BV12
#define AFF_PROTECT		  BV13
#define AFF_PARALYSIS		  BV14
#define AFF_SNEAK		  BV15
#define AFF_HIDE		  BV16
#define AFF_SLEEP		  BV17
#define AFF_CHARM		  BV18
#define AFF_FLYING		  BV19
#define AFF_PASS_DOOR		  BV20
#define AFF_FLOATING		  BV21
#define AFF_TRUESIGHT		  BV22
#define AFF_DETECTTRAPS		  BV23
#define AFF_SCRYING	          BV24
#define AFF_FIRESHIELD	          BV25
#define AFF_SHOCKSHIELD	          BV26
#define AFF_FASTHEAL              BV27
#define AFF_ICESHIELD  		  BV28
#define AFF_POSSESS		  BV29
#define AFF_BERSERK		  BV30
#define AFF_AQUA_BREATH		  BV31

/* 31 aff's (1 left.. :P) */
/* make that none - ugh - time for another field? :P */
/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_PLUS5		  BV18
#define RIS_PLUS6		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21
/* 21 RIS's*/

/* 
 * Attack types
 */
#define ATCK_BITE		  BV00
#define ATCK_CLAWS		  BV01
#define ATCK_TAIL		  BV02
#define ATCK_STING		  BV03
#define ATCK_PUNCH		  BV04
#define ATCK_KICK		  BV05
#define ATCK_TRIP		  BV06
#define ATCK_BACKSTAB		  BV10

/*
 * Defense types
 */
#define DFND_PARRY		  BV00
#define DFND_DODGE		  BV01
#define DFND_DISARM		  BV19
#define DFND_GRIP		  BV21

/*
 * Body parts
 */
#define PART_HEAD		  BV00
#define PART_ARMS		  BV01
#define PART_LEGS		  BV02
#define PART_HEART		  BV03
#define PART_BRAINS		  BV04
#define PART_GUTS		  BV05
#define PART_HANDS		  BV06
#define PART_FEET		  BV07
#define PART_FINGERS		  BV08
#define PART_EAR		  BV09
#define PART_EYE		  BV10
#define PART_LONG_TONGUE	  BV11
#define PART_EYESTALKS		  BV12
#define PART_TENTACLES		  BV13
#define PART_FINS		  BV14
#define PART_WINGS		  BV15
#define PART_TAIL		  BV16
#define PART_SCALES		  BV17
/* for combat */
#define PART_CLAWS		  BV18
#define PART_FANGS		  BV19
#define PART_HORNS		  BV20
#define PART_TUSKS		  BV21
#define PART_TAILATTACK		  BV22
#define PART_SHARPSCALES	  BV23
#define PART_BEAK		  BV24

#define PART_HAUNCH		  BV25
#define PART_HOOVES		  BV26
#define PART_PAWS		  BV27
#define PART_FORELEGS		  BV28
#define PART_FEATHERS		  BV29

/*
 * Autosave flags
 */
#define SV_DEATH		  BV00
#define SV_KILL			  BV01
#define SV_PASSCHG		  BV02
#define SV_DROP			  BV03
#define SV_PUT			  BV04
#define SV_GIVE			  BV05
#define SV_AUTO			  BV06
#define SV_ZAPDROP		  BV07
#define SV_AUCTION		  BV08
#define SV_GET			  BV09
#define SV_RECEIVE		  BV10
#define SV_IDLE			  BV11
#define SV_BACKUP		  BV12

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT		  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH		  BV08

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		  BV11
#define SF_EARTH		  BV12
#define SF_AIR			  BV13
#define SF_ASTRAL		  BV14
#define SF_AREA			  BV15   /* is an area spell      */
#define SF_DISTANT		  BV16   /* affects something far away  */
#define SF_REVERSE		  BV17
#define SF_SAVE_HALF_DAMAGE	  BV18   /* save for half damage     */
#define SF_SAVE_NEGATES		  BV19   /* save negates affect      */
#define SF_ACCUMULATIVE		  BV20   /* is accumulative    */
#define SF_RECASTABLE		  BV21   /* can be refreshed      */
#define SF_NOSCRIBE		  BV22   /* cannot be scribed     */
#define SF_NOBREW		  BV23   /* cannot be brewed      */
#define SF_GROUPSPELL		  BV24   /* only affects group members  */
#define SF_OBJECT		  BV25   /* directed at an object   */
#define SF_CHARACTER		  BV26   /* directed at a character  */
#define SF_SECRETSKILL		  BV27   /* hidden unless learned   */
#define SF_PKSENSITIVE		  BV28   /* much harder for plr vs. plr   */
#define SF_STOPONFAIL		  BV29   /* stops spell on first failure */

typedef enum
{ SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
   SS_BREATH, SS_SPELL_STAFF
} save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)

typedef enum
{ SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
   SD_POISON, SD_DRAIN
} spell_dam_types;

typedef enum
{ SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
   SA_DIVINATE, SA_OBSCURE, SA_CHANGE
} spell_act_types;

typedef enum
{ SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum
{ SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
   SC_LIFE, SC_DEATH, SC_ILLUSION
} spell_class_types;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum
{ SEX_NEUTRAL, SEX_MALE, SEX_FEMALE } sex_types;

typedef enum
{
   TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART, TRAP_TYPE_POISON_NEEDLE,
   TRAP_TYPE_POISON_DAGGER, TRAP_TYPE_POISON_ARROW, TRAP_TYPE_BLINDNESS_GAS,
   TRAP_TYPE_SLEEPING_GAS, TRAP_TYPE_FLAME, TRAP_TYPE_EXPLOSION,
   TRAP_TYPE_ACID_SPRAY, TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
   TRAP_TYPE_SEX_CHANGE
} trap_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3

#define COMMSYS_VNUM		     62
#define DATAPAD_VNUM		     63

#define	MODULE_VNUM		     73
#define SABER_VNUM    		 72

#define OBJ_VNUM_DROID_CORPSE        9
#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_SPILLED_GUTS	     16
#define OBJ_VNUM_BLOOD		     17
#define OBJ_VNUM_BLOODSTAIN	     18
#define OBJ_VNUM_SCRAPS		     19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_SLICE		     24
#define OBJ_VNUM_SHOPPING_BAG	     25

#define OBJ_VNUM_FIRE		     30
#define OBJ_VNUM_TRAP		     31
#define OBJ_VNUM_PORTAL		     32

#define OBJ_VNUM_BLACK_POWDER	     33
#define OBJ_VNUM_SCROLL_SCRIBING     34
#define OBJ_VNUM_FLASK_BREWING       35
#define OBJ_VNUM_NOTE		     36

/* Academy eq */
#define OBJ_VNUM_SCHOOL_MACE	  10315
#define OBJ_VNUM_SCHOOL_DAGGER	  10312
#define OBJ_VNUM_SCHOOL_SWORD	  10313
#define OBJ_VNUM_SCHOOL_VEST	  10308
#define OBJ_VNUM_SCHOOL_SHIELD	  10310
#define OBJ_VNUM_SCHOOL_BANNER    10311
#define OBJ_VNUM_SCHOOL_DIPLOMA   10321

#define OBJ_VNUM_BLASTECH_E11     50
#define OBJ_VNUM_SHIPBOMB     	  68

/* These are some defines for modules */
#define AFFECT_PRIMARY		1
#define AFFECT_SECONDARY        2
#define AFFECT_MISSILE		3
#define AFFECT_ROCKET		4
#define AFFECT_TORPEDO		5
#define AFFECT_HULL		6
#define AFFECT_SHIELD		7
#define AFFECT_SPEED		8
#define AFFECT_HYPER		9
#define AFFECT_ENERGY		10
#define AFFECT_MANUEVER		11
#define AFFECT_CHAFF		12
#define AFFECT_ALARM		13

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
   ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
   ITEM_FIREWEAPON, ITEM_MISSILE, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
   ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER,
   ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
   ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
   ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
   ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
   ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_RUNE, ITEM_RUNEPOUCH,
   ITEM_MATCH, ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER,
   ITEM_TINDER, ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
   ITEM_SHORT_BOW, ITEM_LONG_BOW, ITEM_CROSSBOW, ITEM_AMMO, ITEM_QUIVER,
   ITEM_SHOVEL, ITEM_SALVE, ITEM_RAWSPICE, ITEM_LENS, ITEM_CRYSTAL, ITEM_DURAPLAST,
   ITEM_BATTERY, ITEM_TOOLKIT, ITEM_DURASTEEL, ITEM_OVEN, ITEM_MIRROR,
   ITEM_CIRCUIT, ITEM_SUPERCONDUCTOR, ITEM_COMLINK, ITEM_MEDPAC, ITEM_FABRIC,
   ITEM_RARE_METAL, ITEM_MAGNET, ITEM_THREAD, ITEM_SPICE, ITEM_SMUT, ITEM_DEVICE, ITEM_SPACECRAFT,
   ITEM_GRENADE, ITEM_LANDMINE, ITEM_GOVERNMENT, ITEM_DROID_CORPSE, ITEM_BOLT, ITEM_CHEMICAL, ITEM_COMMSYSTEM,
   ITEM_DATAPAD, ITEM_MODULE, ITEM_BUG, ITEM_BEACON, ITEM_GLAUNCHER,
   ITEM_RLAUNCHER, ITEM_BINDERS, ITEM_GOGGLES, ITEM_SHIPBOMB, ITEM_EMP_GRENADE
} item_types;


#define MAX_ITEM_TYPE		     ITEM_EMP_GRENADE
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		BV00
#define ITEM_HUM		BV01
#define ITEM_DARK		BV02
#define ITEM_HUTT_SIZE		BV03
#define ITEM_CONTRABAND		BV04
#define ITEM_INVIS		BV05
#define ITEM_MAGIC		BV06
#define ITEM_NODROP		BV07
#define ITEM_BLESS		BV08
#define ITEM_ANTI_GOOD		BV09
#define ITEM_ANTI_EVIL		BV10
#define ITEM_ANTI_NEUTRAL	BV11
#define ITEM_NOREMOVE		BV12
#define ITEM_INVENTORY		BV13
#define ITEM_ANTI_SOLDIER	BV14
#define ITEM_ANTI_THIEF	        BV15
#define ITEM_ANTI_HUNTER	BV16
#define ITEM_ANTI_JEDI  	BV17
#define ITEM_SMALL_SIZE		BV18
#define ITEM_LARGE_SIZE		BV19
#define ITEM_DONATION		BV20
#define ITEM_CLANOBJECT		BV21
#define ITEM_ANTI_CITIZEN	BV22
#define ITEM_ANTI_SITH  	BV23
#define ITEM_ANTI_PILOT	        BV24
#define ITEM_HIDDEN		BV25
#define ITEM_POISONED		BV26
#define ITEM_COVERING		BV27
#define ITEM_DEATHROT		BV28
#define ITEM_BURRIED		BV29  /* item is underground */
#define ITEM_PROTOTYPE		BV30
#define ITEM_HUMAN_SIZE         BV31

/* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING		BV00
#define ITEM_BACKSTABBER  	BV01
#define ITEM_BANE		BV02
#define ITEM_LOYAL		BV03
#define ITEM_HASTE		BV04
#define ITEM_DRAIN		BV05
#define ITEM_LIGHTNING_BLADE  	BV06

/* Blaster settings - only saves on characters */
#define BLASTER_NORMAL          0
#define BLASTER_HALF		2
#define BLASTER_FULL            5
#define BLASTER_LOW		1
#define	BLASTER_STUN		3
#define BLASTER_HIGH            4

/* Weapon Types */

#define WEAPON_NONE     	0
#define WEAPON_VIBRO_AXE	1
#define WEAPON_VIBRO_BLADE	2
#define WEAPON_LIGHTSABER	3
#define WEAPON_WHIP		4
#define WEAPON_CLAW		5
#define WEAPON_BLASTER		6
#define WEAPON_BLUDGEON		8
#define WEAPON_BOWCASTER        9
#define WEAPON_FORCE_PIKE	11
#define WEAPON_DUAL_LIGHTSABER	12

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02

/* drug types */
#define SPICE_GLITTERSTIM        0
#define SPICE_CARSANUM           1
#define SPICE_RYLL               2
#define SPICE_ANDRIS             3

/* crystal types */
#define GEM_NON_ADEGEN          0
#define GEM_KATHRACITE		1
#define GEM_RELACITE		2
#define GEM_DANITE		3
#define GEM_MEPHITE		4
#define GEM_PONITE		5
#define GEM_ILLUM               6
#define GEM_CORUSCA             7

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		BV00
#define ITEM_WEAR_FINGER	BV01
#define ITEM_WEAR_NECK		BV02
#define ITEM_WEAR_BODY		BV03
#define ITEM_WEAR_HEAD		BV04
#define ITEM_WEAR_LEGS		BV05
#define ITEM_WEAR_FEET		BV06
#define ITEM_WEAR_HANDS		BV07
#define ITEM_WEAR_ARMS		BV08
#define ITEM_WEAR_SHIELD	BV09
#define ITEM_WEAR_ABOUT		BV10
#define ITEM_WEAR_WAIST		BV11
#define ITEM_WEAR_WRIST		BV12
#define ITEM_WIELD		BV13
#define ITEM_HOLD		BV14
#define ITEM_DUAL_WIELD		BV15
#define ITEM_WEAR_EARS		BV16
#define ITEM_WEAR_EYES		BV17
#define ITEM_MISSILE_WIELD	BV18
#define	ITEM_WEAR_BACK		BV19
#define	ITEM_WEAR_HOLSTER1	BV20
#define	ITEM_WEAR_HOLSTER2	BV21
#define	ITEM_WEAR_BOTHWRISTS	BV22


/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
   APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
   APPLY_SEX, APPLY_NULL, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
   APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
   APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
   APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
   APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
   APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
   APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP, APPLY_DODGE,
   APPLY_PEEK, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT, APPLY_DISARM,
   APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH, APPLY_CLIMB,
   APPLY_GRIP, APPLY_SCRIBE, APPLY_COVER_TRAIL, APPLY_WEARSPELL, APPLY_REMOVESPELL,
   APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE, APPLY_DIG,
   APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_POLY		      3
#define ROOM_VNUM_CHAT		  32144
#define ROOM_VNUM_TEMPLE	  32144
#define ROOM_VNUM_ALTAR		  32144
#define ROOM_VNUM_SCHOOL	    115
#define ROOM_AUTH_START		  10300
#define ROOM_START_HUMAN            211
#define ROOM_START_WOOKIEE        28600
#define ROOM_START_TWILEK         32148
#define ROOM_START_RODIAN         32148
#define ROOM_START_HUTT           32148
#define ROOM_START_MON_CALAMARIAN 21069
#define ROOM_START_NOGHRI          1015
#define ROOM_START_GAMORREAN      28100
#define ROOM_START_JAWA           31819
#define ROOM_START_ADARIAN        29000
#define ROOM_START_EWOK           32148
#define ROOM_START_VERPINE        32148
#define ROOM_START_DEFEL          32148
#define ROOM_START_TRANDOSHAN     32148
#define ROOM_START_HAPAN     32148
#define ROOM_START_DUINUOGWUIN    32148
#define ROOM_START_QUARREN        21069
#define ROOM_START_IMMORTAL         100
#define ROOM_LIMBO_SHIPYARD          45
#define ROOM_DEFAULT_CRASH        28025

#define ROOM_PLUOGUS_QUIT         905

#define ROOM_SHUTTLE_BUS           907 /* Sol */
#define ROOM_SHUTTLE_BUS_2         914 /* Monir */
#define ROOM_SHUTTLE_BUS_3		   921   /* Fau */
#define ROOM_SHUTTLE_BUS_4		   928   /* Taw */

#define ROOM_CORUSCANT_SHUTTLE     199
#define ROOM_SENATE_SHUTTLE      10197
#define ROOM_CORUSCANT_TURBOCAR    226

#define SHIP_AREA               "shipvnum.are"

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

#define ROOM_DARK		BV00
/* BV01 now reserved for track  BV01  and hunt */
#define ROOM_NO_MOB		BV02
#define ROOM_INDOORS		BV03
#define ROOM_CAN_LAND		BV04
#define ROOM_CAN_FLY		BV05
#define ROOM_NO_DRIVING 	BV06
#define ROOM_NO_MAGIC		BV07
#define ROOM_BANK		BV08
#define ROOM_PRIVATE		BV09
#define ROOM_SAFE		BV10
#define ROOM_SOLITARY		BV11
#define ROOM_PET_SHOP		BV12
#define ROOM_NO_RECALL		BV13
#define ROOM_DONATION		BV14
#define ROOM_NODROPALL		BV15
#define ROOM_SILENCE		BV16
#define ROOM_LOGSPEECH		BV17
#define ROOM_NODROP		BV18
#define ROOM_CLANSTOREROOM	BV19
#define ROOM_PLR_HOME		BV20
#define ROOM_EMPTY_HOME 	BV21
#define ROOM_TELEPORT		BV22
#define ROOM_HOTEL      	BV23
#define ROOM_NOFLOOR		BV24
#define ROOM_REFINERY           BV25
#define ROOM_FACTORY            BV26
#define ROOM_R_RECRUIT          BV27
#define ROOM_E_RECRUIT          BV28
#define ROOM_SPACECRAFT         BV29
#define ROOM_PROTOTYPE	     	BV30
#define ROOM_AUCTION            BV31

/* Second Set of Room Flags */
#define ROOM_EMPTY_SHOP		BV00
#define ROOM_PLR_SHOP		BV01
#define ROOM_SHIPYARD		BV02
#define ROOM_GARAGE		BV03
#define ROOM_BARRACKS		BV04
#define ROOM_CONTROL		BV05
#define ROOM_CLANLAND		BV06
#define ROOM_ARENA		BV07
#define ROOM_CLANJAIL		BV08
#define ROOM_BLACKMARKET	BV09
#define ROOM_HIDDENPAD		BV10

/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
   DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
   DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define MAX_DIR			DIR_SOUTHWEST  /* max for normal walking */
#define DIR_PORTAL		DIR_SOMEWHERE  /* portal direction    */


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_RES1                   BV09 /* are these res[1-4] important? */
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES2			  BV14
#define EX_RES3			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_RES4	  		  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define MAX_EXFLAG		  26


/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
   SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
   SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
   SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_SCRUB, SECT_ROCKY,
   SECT_SAVANNA, SECT_TUNDRA, SECT_GLACIAL, SECT_RAINFOREST, SECT_JUNGLE,
   SECT_SWAMP, SECT_WETLANDS, SECT_BRUSH, SECT_STEPPE, SECT_FARMLAND, SECT_VOLCANIC,
   SECT_MAX
} sector_types;

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
   WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
   WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
   WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
   WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EARS, WEAR_EYES,
   WEAR_MISSILE_WIELD, WEAR_BACK, WEAR_HOLSTER_L, WEAR_HOLSTER_R, WEAR_BOTH_WRISTS,
   MAX_WEAR
} wear_locations;

/* Board Types */
typedef enum
{ BOARD_NOTE, BOARD_MAIL } board_types;

/* Auth Flags */
#define FLAG_WRAUTH		      1
#define FLAG_AUTH		      2

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
   COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

/*
 * Positions.
 */
typedef enum
{
   POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_RESTING,
   POS_SITTING, POS_FIGHTING, POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG
} positions;

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      BV00  /* Don't EVER set.   */
#define PLR_EXEMPT		      BV01
#define PLR_SHOVEDRAG		      BV02
#define PLR_AUTOEXIT		      BV03
#define PLR_AUTOLOOT		      BV04
#define PLR_AUTOSAC                   BV05
#define PLR_BLANK		      BV06
#define PLR_OUTCAST 		      BV07
#define PLR_BRIEF		      BV08
#define PLR_COMBINE		      BV09
#define PLR_PROMPT		      BV10
#define PLR_TELNET_GA		      BV11

#define PLR_HOLYLIGHT		   BV12
#define PLR_WIZINVIS		   BV13
#define PLR_ROOMVNUM		   BV14

#define	PLR_SILENCE		   BV15
#define PLR_NO_EMOTE		   BV16
#define PLR_ATTACKER    	   BV17
#define PLR_NO_TELL		   BV18
#define PLR_LOG			   BV19
#define PLR_DENY		   BV20
#define PLR_FREEZE		   BV21
#define PLR_KILLER    	           BV22
#define PLR_WHOINVIS 	           BV23
#define PLR_LITTERBUG	           BV24
#define PLR_ANSI	           BV25
#define PLR_SOUND	           BV26
#define PLR_NICE	           BV27
#define PLR_FLEE	           BV28
#define PLR_AUTOGOLD               BV29
#define PLR_SLOG                   BV30
#define PLR_AFK                    BV31

/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
/*
#define PCFLAG_                    BV01     extra flag
*/
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_HASSLUG             BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_ROOM                BV11

typedef enum
{
   TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
   TIMER_APPLIED, TIMER_PKILLED
} timer_types;

struct timer_data
{
   TIMER *prev;
   TIMER *next;
   DO_FUN *do_fun;
   int value;
   short type;
   short count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_QUEST		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_ASK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_YELL		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
#define CHANNEL_104		   BV10
#define CHANNEL_CLAN		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_105		   BV13
#define CHANNEL_AVTALK		   BV14
#define CHANNEL_PRAY		   BV15
#define CHANNEL_COUNCIL 	   BV16
#define CHANNEL_GUILD              BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_ORDER              BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_OOC                BV23
#define CHANNEL_SHIP               BV24
#define CHANNEL_SYSTEM             BV25
#define CHANNEL_SPACE              BV26
#define CHANNEL_103		   BV27
#define CHANNEL_SPORTS		   BV28
#define CHANNEL_HOLONET		   BV31

#define CHANNEL_CLANTALK	   CHANNEL_CLAN

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_PROTOTYPE             BV01

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
   MOB_INDEX_DATA *next;
   MOB_INDEX_DATA *next_sort;
   SPEC_FUN *spec_fun;
   SPEC_FUN *spec_2;
   SHOP_DATA *pShop;
   REPAIR_DATA *rShop;
   MPROG_DATA *mudprogs;
   int progtypes;
   char *player_name;
   char *short_descr;
   char *long_descr;
   char *description;
   char *spec_funname;
   char *spec_funname2;
   int vnum;
   short count;
   short killed;
   short sex;
   short level;
   int act;
   int affected_by;
   short alignment;
   short mobthac0;   /* Unused */
   short ac;
   short hitnodice;
   short hitsizedice;
   short hitplus;
   short damnodice;
   short damsizedice;
   short damplus;
   short numattacks;
   int gold;
   int exp;
   int xflags;
   int resistant;
   int immune;
   int susceptible;
   int attacks;
   int defenses;
   int speaks;
   int speaking;
   short position;
   short defposition;
   short height;
   short weight;
   short race;
   short hitroll;
   short damroll;
   short perm_str;
   short perm_int;
   short perm_wis;
   short perm_dex;
   short perm_con;
   short perm_cha;
   short perm_lck;
   short perm_frc;
   short saving_poison_death;
   short saving_wand;
   short saving_para_petri;
   short saving_breath;
   short saving_spell_staff;
   int vip_flags;
};


struct hunt_hate_fear
{
   char *name;
   CHAR_DATA *who;
};

struct fighting_data
{
   CHAR_DATA *who;
   int xp;
   short align;
   short duration;
   short timeskilled;
};

struct extracted_char_data
{
   EXTRACT_CHAR_DATA *next;
   CHAR_DATA *ch;
   ROOM_INDEX_DATA *room;
   ch_ret retcode;
   bool extract;
};

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
struct char_data
{
   CHAR_DATA *next;
   CHAR_DATA *prev;
   CHAR_DATA *next_in_room;
   CHAR_DATA *prev_in_room;
   CHAR_DATA *master;
   CHAR_DATA *leader;
   FIGHT_DATA *fighting;
   CHAR_DATA *reply;
   char *owner;
   ROOM_INDEX_DATA *home;
   CHAR_DATA *switched;
   BUG_DATA *first_bug;
   BUG_DATA *last_bug;
   CONTRACT_DATA *first_contract;
   CONTRACT_DATA *last_contract;
   FELLOW_DATA *first_fellow;
   FELLOW_DATA *last_fellow;
   CHAR_DATA *mount;
   HHF_DATA *hunting;
   HHF_DATA *fearing;
   HHF_DATA *hating;
   SPEC_FUN *spec_fun;
   SPEC_FUN *spec_2;
   char *spec_funname;
   char *spec_funname2;
   MPROG_ACT_LIST *mpact;
   int mpactnum;
   unsigned short mpscriptpos;
   MOB_INDEX_DATA *pIndexData;
   DESCRIPTOR_DATA *desc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   NOTE_DATA *pnote;
   NOTE_DATA *comments;
   OBJ_DATA *first_carrying;
   OBJ_DATA *last_carrying;
   ROOM_INDEX_DATA *in_room;
   ROOM_INDEX_DATA *was_in_room;
   ROOM_INDEX_DATA *was_sentinel;
   ROOM_INDEX_DATA *plr_home;
   PC_DATA *pcdata;
   DO_FUN *last_cmd;
   DO_FUN *prev_cmd; /* mapping */
   CHAR_DATA *challenged;
   CHAR_DATA *betted_on;
   int bet_amt;
   void *dest_buf;
   void *dest_buf_2;
   void *spare_ptr;
   int tempnum;
   EDITOR_DATA *editor;
   TIMER *first_timer;
   TIMER *last_timer;
   char *name;
   char *short_descr;
   char *long_descr;
   char *description;
   short num_fighting;
   short substate;
   short sex;
   short race;
   short top_level;
   short skill_level[MAX_ABILITY];
   short bonus[MAX_ABILITY];
   short trust;
   int played;
   time_t logon;
   time_t save_time;
   short timer;
   short wait;
   short hit;
   short max_hit;
   int force_skill[MAX_FORCE_SKILL];
   short force_control;
   short force_sense;
   short force_alter;
   short force_chance;
   short force_identified;
   short force_level_status;
   short force_align;
   short force_converted;
   short force_type;
   char *force_master;
   char *force_temp_master;
   char *force_disguise;
   int force_disguise_count;
   int wait_state;
   short mana;
   short max_mana;
   short move;
   short max_move;
   short numattacks;
   int gold;
   long experience[MAX_ABILITY];
   int act;
   int affected_by;
   int carry_weight;
   int carry_number;
   int xflags;
   int resistant;
   int immune;
   int susceptible;
   int attacks;
   int defenses;
   int speaks;
   int speaking;
   short saving_poison_death;
   short saving_wand;
   short saving_para_petri;
   short saving_breath;
   short saving_spell_staff;
   short alignment;
   short barenumdie;
   short baresizedie;
   short mobthac0;
   short hitroll;
   short damroll;
   short hitplus;
   short damplus;
   short position;
   short defposition;
   short height;
   short weight;
   short armor;
   short wimpy;
   int deaf;
   short perm_str;
   short perm_int;
   short perm_wis;
   short perm_dex;
   short perm_con;
   short perm_cha;
   short perm_lck;
   short perm_frc;
   short mod_str;
   short mod_int;
   short mod_wis;
   short mod_dex;
   short mod_con;
   short mod_cha;
   short mod_lck;
   short mod_frc;
   short mental_state;  /* simplified */
   short emotional_state;  /* simplified */
   int retran;
   int regoto;
   short mobinvis;   /* Mobinvis level SB */
   int vip_flags;
   short backup_wait;   /* reinforcements */
   int backup_mob;   /* reinforcements */
   short was_stunned;
   char *mob_clan;   /* for spec_clan_guard.. set by postguard */
   GUARD_DATA *guard_data;
   short main_ability;
   short secondary_ability;
   short rppoints;
   char *comfreq;
   char *rank;
   int pheight, build;
   CHAR_DATA *aiming_at;
   short colors[MAX_COLORS];
   int home_vnum; /* hotboot tracker */
   int resetvnum;
   int resetnum;
};

struct killed_data
{
   int vnum;
   char count;
};

struct bug_data
{
   char *name;
   BUG_DATA *next_in_bug;
   BUG_DATA *prev_in_bug;
};

struct contract_data
{
   char *target;
   int amount;
   CONTRACT_DATA *next_in_contract;
   CONTRACT_DATA *prev_in_contract;
};

struct fellow_data
{

   char *victim;
   char *knownas;
   FELLOW_DATA *next;
   FELLOW_DATA *prev;
};

/*
 * Data which only PC's have.
 */
struct pc_data
{
   CLAN_DATA *clan;
   AREA_DATA *area;
   ROOM_INDEX_DATA *roomarena;
   char *homepage;
   char *screenname;
   char *image;
   char *clan_name;
   char *pwd;
   char *email;
   char *bamfin;
   char *bamfout;
   int lost_attacks;
   char *rank;
   int shipnum;
   char *shipname;
   char *title;
   char *disguise;
   char *bestowments;   /* Special bestowed commands     */
   int act2;
   int flags;  /* Whether the player is deadly and whatever else we add.      */
   int pkills; /* Number of pkills on behalf of clan */
   int pdeaths;   /* Number of times pkilled (legally)  */
   int mkills; /* Number of mobs killed         */
   int mdeaths;   /* Number of deaths due to mobs       */
   int illegal_pk;   /* Number of illegal pk's committed   */
   char *fiance;
   char *propose;
   char *proposed;
   char *spouse;
   int forcerank;
   char *last_name;
   long int outcast_time;  /* The time at which the char was outcast */
   long int restore_time;  /* The last time the char did a restore all */
   int r_range_lo;   /* room range */
   int r_range_hi;
   int m_range_lo;   /* mob range  */
   int m_range_hi;
   int o_range_lo;   /* obj range  */
   int o_range_hi;
   char *tell_snoop; /* Tell snoop */
   short wizinvis;   /* wizinvis level */
   short min_snoop;  /* minimum snoop level */
   short condition[MAX_CONDS];
   short learned[MAX_SKILL];
   KILLED_DATA killed[MAX_KILLTRACK];
   short quest_number;  /* current *QUEST BEING DONE* DON'T REMOVE! */
   short quest_curr; /* current number of quest points */
   int quest_accum;  /* quest points accumulated in players life */
   int auth_state;
   time_t release_date; /* Auto-helling.. Altrag */
   char *helled_by;
   char *bio;  /* Personal Bio */
   char *authed_by;  /* what crazy imm authed this name ;) */
   SKILLTYPE *special_skills[5]; /* personalized skills/spells */
   char *prompt;  /* User config prompts */
   char *subprompt;  /* Substate prompt */
   short pagerlen;   /* For pager (NOT menus) */
   bool openedtourney;
   short addiction[10];
   short drug_level[10];
   char *store_title;
   bool is_hacking;
   int wanted_flags;
   long bank;
   int salary;
   bool hotboot;  /* hotboot tracker */
};

/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		19

struct liq_type
{
   const char *liq_name;
   const char *liq_color;
   short liq_affect[4]; // CPPCheck detected an out of bounds usage when this was set to 3.
};

/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
   EXTRA_DESCR_DATA *next; /* Next in list                     */
   EXTRA_DESCR_DATA *prev; /* Previous in list                 */
   char *keyword; /* Keyword in look/examine          */
   char *description;   /* What to see                      */
};

/*
 * Prototype for an object.
 */
struct obj_index_data
{
   OBJ_INDEX_DATA *next;
   OBJ_INDEX_DATA *next_sort;
   EXTRA_DESCR_DATA *first_extradesc;
   EXTRA_DESCR_DATA *last_extradesc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   MPROG_DATA *mudprogs;   /* objprogs */
   int progtypes; /* objprogs */
   char *name;
   char *short_descr;
   char *description;
   char *action_desc;
   int vnum;
   short level;
   short item_type;
   int extra_flags;
   int magic_flags;  /*Need more bitvectors for spells - Scryn */
   int wear_flags;
   short count;
   short weight;
   int cost;
   int value[6];
   int serial;
   short layers;
   int rent;   /* Unused */
};

/*
 * One object.
 */
struct obj_data
{
   OBJ_DATA *next;
   OBJ_DATA *prev;
   OBJ_DATA *next_content;
   OBJ_DATA *prev_content;
   OBJ_DATA *first_content;
   OBJ_DATA *last_content;
   OBJ_DATA *in_obj;
   CHAR_DATA *carried_by;
   EXTRA_DESCR_DATA *first_extradesc;
   EXTRA_DESCR_DATA *last_extradesc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   OBJ_INDEX_DATA *pIndexData;
   ROOM_INDEX_DATA *in_room;
   char *armed_by;
   char *name;
   char *short_descr;
   char *description;
   char *action_desc;
   short item_type;
   short mpscriptpos;
   int extra_flags;
   int magic_flags;  /*Need more bitvectors for spells - Scryn */
   int wear_flags;
   int blaster_setting;
   MPROG_ACT_LIST *mpact;  /* mudprogs */
   int mpactnum;  /* mudprogs */
   short wear_loc;
   short weight;
   char *killer;  /* This serves one real purpose. When making a corpse we assign the killers name to it. */
   int cost;
   short level;
   short timer;
   int value[6];
   short count;   /* support for object grouping */
   int serial; /* serial number         */
   int room_vnum; /* hotboot tracker */
};

/*
 * Exit data.
 */
struct exit_data
{
   EXIT_DATA *prev;  /* previous exit in linked list  */
   EXIT_DATA *next;  /* next exit in linked list   */
   EXIT_DATA *rexit; /* Reverse exit pointer    */
   ROOM_INDEX_DATA *to_room;  /* Pointer to destination room   */
   char *keyword; /* Keywords for exit or door  */
   char *description;   /* Description of exit     */
   int vnum;   /* Vnum of room exit leads to */
   int rvnum;  /* Vnum of room in opposite dir  */
   int exit_info; /* door states & other flags  */
   int key; /* Key vnum       */
   short vdir; /* Physical "direction"    */
   short distance;   /* how far to the next room   */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data
{
   RESET_DATA *next;
   RESET_DATA *prev;
   RESET_DATA *first_reset;
   RESET_DATA *last_reset;
   RESET_DATA *next_reset;
   RESET_DATA *prev_reset;
   char command;
   int extra;
   int arg1;
   int arg2;
   int arg3;
   bool sreset;
};

/*
 * Area definition.
 */
struct area_data
{
   AREA_DATA *next;
   AREA_DATA *prev;
   AREA_DATA *next_sort;
   AREA_DATA *prev_sort;
   AREA_DATA *next_sort_name; /* Used for alphanum. sort */
   AREA_DATA *prev_sort_name; /* Ditto, Fireblade */
   ROOM_INDEX_DATA *first_room;
   ROOM_INDEX_DATA *last_room;
   PLANET_DATA *planet;
   AREA_DATA *next_on_planet;
   AREA_DATA *prev_on_planet;
   char *name;
   char *filename;
   int flags;
   short version;
   short status;  /* h, 8/11 */
   short age;
   short nplayer;
   short reset_frequency;
   int low_r_vnum;
   int hi_r_vnum;
   int low_o_vnum;
   int hi_o_vnum;
   int low_m_vnum;
   int hi_m_vnum;
   int low_soft_range;
   int hi_soft_range;
   int low_hard_range;
   int hi_hard_range;
   char *author;  /* Scryn */
   char *resetmsg;   /* Rennard */
   RESET_DATA *last_mob_reset;
   RESET_DATA *last_obj_reset;
   short max_players;
   int mkills;
   int mdeaths;
   int pkills;
   int pdeaths;
   int gold_looted;
   int illegal_pk;
   int high_economy;
   int low_economy;
};

/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct system_data
{
   int maxplayers;   /* Maximum players this boot   */
   int alltimemax;   /* Maximum players ever   */
   char *time_of_max;   /* Time of max ever */
   bool NO_NAME_RESOLVING; /* Hostnames are not resolved  */
   bool DENY_NEW_PLAYERS;  /* New players cannot connect  */
   bool WAIT_FOR_AUTH;  /* New players must be auth'ed */
   short newbie_purge;  /* Level to auto-purge newbies at - Samson 12-27-98 */
   short regular_purge; /* Level to purge normal players at - Samson 12-27-98 */
   bool CLEANPFILES; /* Should the mud clean up pfiles daily? - Samson 12-27-98 */
   short read_all_mail; /* Read all player mail(was 54) */
   short read_mail_free;   /* Read mail for free (was 51) */
   short write_mail_free;  /* Write mail for free(was 51) */
   short take_others_mail; /* Take others mail (was 54)   */
   short muse_level; /* Level of muse channel */
   short think_level;   /* Level of think channel LEVEL_HIGOD */
   short build_level;   /* Level of build channel LEVEL_BUILD */
   short log_level;  /* Level of log channel LEVEL LOG */
   short level_modify_proto;  /* Level to modify prototype stuff LEVEL_LESSER */
   short level_override_private; /* override private flag */
   short level_mset_player;   /* Level to mset a player */
   short stun_plr_vs_plr;  /* Stun mod player vs. player */
   short stun_regular;  /* Stun difficult */
   short dam_plr_vs_plr;   /* Damage mod player vs. player */
   short dam_plr_vs_mob;   /* Damage mod player vs. mobile */
   short dam_mob_vs_plr;   /* Damage mod mobile vs. player */
   short dam_mob_vs_mob;   /* Damage mod mobile vs. mobile */
   short level_getobjnotake;  /* Get objects without take flag */
   short level_forcepc; /* The level at which you can use force on players. */
   short max_sn;  /* Max skills */
   char *guild_overseer;   /* Pointer to char containing the name of the */
   char *guild_advisor; /* guild overseer and advisor. */
   int save_flags;   /* Toggles for saving conditions */
   short save_frequency;   /* How old to autosave someone */
   void *dlHandle;
};

/*
 * Room type.
 */
struct room_index_data
{
   ROOM_INDEX_DATA *next;
   ROOM_INDEX_DATA *next_sort;
   CHAR_DATA *first_person;
   CHAR_DATA *last_person;
   OBJ_DATA *first_content;
   OBJ_DATA *last_content;
   EXTRA_DESCR_DATA *first_extradesc;
   EXTRA_DESCR_DATA *last_extradesc;
   AREA_DATA *area;
   EXIT_DATA *first_exit;
   EXIT_DATA *last_exit;
   RESET_DATA *first_reset;
   RESET_DATA *last_reset;
   RESET_DATA *last_mob_reset;
   RESET_DATA *last_obj_reset;
   ROOM_INDEX_DATA *next_aroom;  /* Rooms within an area */
   ROOM_INDEX_DATA *prev_aroom;
   ROOM_INDEX_DATA *next_in_area;
   ROOM_INDEX_DATA *prev_in_area;
   ROOM_INDEX_DATA *next_in_ship;
   ROOM_INDEX_DATA *prev_in_ship;
   char *name;
   int exvnum;
   SHIP_DATA *first_ship;
   SHIP_DATA *last_ship;
   char *description;
   int vnum;
   int room_flags;
   int room_flags2;
   MPROG_ACT_LIST *mpact;  /* mudprogs */
   int mpactnum;  /* mudprogs */
   MPROG_DATA *mudprogs;   /* mudprogs */
   short mpscriptpos;
   int progtypes; /* mudprogs */
   short light;
   short sector_type;
   int tele_vnum;
   short tele_delay;
   short tunnel;  /* max people that will fit */
};

/*
 * Delayed teleport type.
 */
struct teleport_data
{
   TELEPORT_DATA *next;
   TELEPORT_DATA *prev;
   ROOM_INDEX_DATA *room;
   short timer;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_MISSILE			111
#define TYPE_HIT                     1000 /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000   /* allows for 1000 attack types  */
#define TYPE_PERSONAL		     3000   /* allows for 1000 herb types    */

/*
 *  Target types.
 */
typedef enum
{
   TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
   TAR_OBJ_INV
} target_types;

typedef enum
{
   SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
   SKILL_HERB
} skill_types;

struct timerset
{
   int num_uses;
   struct timeval total_time;
   struct timeval min_time;
   struct timeval max_time;
};

/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
   char *name; /* Name of skill     */
   SPELL_FUN *spell_fun;   /* Spell pointer (for spells) */
   char *spell_fun_name;   /* Spell function name - Trax */
   DO_FUN *skill_fun;   /* Skill pointer (for skills) */
   char *skill_fun_name;   /* Skill function name - Trax */
   short target;  /* Legal targets     */
   short minimum_position; /* Position for caster / user */
   short slot; /* Slot for #OBJECT loading   */
   short min_mana;   /* Minimum mana used    */
   short beats;   /* Rounds required to use skill  */
   char *noun_damage;   /* Damage message    */
   char *msg_off; /* Wear off message     */
   short guild;   /* Which guild the skill belongs to */
   short min_level;  /* Minimum level to be able to cast */
   short type; /* Spell/Skill/Weapon/Tongue  */
   int flags;  /* extra stuff       */
   char *hit_char;   /* Success message to caster  */
   char *hit_vict;   /* Success message to victim  */
   char *hit_room;   /* Success message to room */
   char *miss_char;  /* Failure message to caster  */
   char *miss_vict;  /* Failure message to victim  */
   char *miss_room;  /* Failure message to room */
   char *die_char;   /* Victim death msg to caster */
   char *die_vict;   /* Victim death msg to victim */
   char *die_room;   /* Victim death msg to room   */
   char *imm_char;   /* Victim immune msg to caster   */
   char *imm_vict;   /* Victim immune msg to victim   */
   char *imm_room;   /* Victim immune msg to room  */
   char *dice; /* Dice roll         */
   int value;  /* Misc value        */
   char saves; /* What saving spell applies  */
   char difficulty;  /* Difficulty of casting/learning */
   SMAUG_AFF *first_affect;  /* Spell affects, if any   */
   SMAUG_AFF *last_affect;
   char *components; /* Spell components, if any   */
   char *teachers;   /* Skill requires a special teacher */
   char participants;   /* # of required participants */
   struct timerset userec; /* Usage record         */
   int alignment; /* for jedi powers */
};


struct auction_data
{
   OBJ_DATA *item;   /* a pointer to the item */
   CHAR_DATA *seller;   /* a pointer to the seller - which may NOT quit */
   CHAR_DATA *buyer; /* a pointer to the buyer - which may NOT quit */
   int bet; /* last bet - or 0 if noone has bet anything */
   short going;   /* 1,2, sold */
   short pulse;   /* how many pulses (.25 sec) until another call-out ? */
   int starting;
};


/*
 * These are skill_lookup return values for basic skills and spells.
 */
extern short gsn_smallspace;
extern short gsn_mediumspace;
extern short gsn_largespace;
extern short gsn_weaponsystems;
extern short gsn_navigation;
extern short gsn_shipsystems;
extern short gsn_tractorbeams;
extern short gsn_spacecombat;
extern short gsn_spacecombat2;
extern short gsn_spacecombat3;
extern short gsn_bomb;

extern short gsn_shipdesign;

/* Technician skills */
extern short gsn_makemodule;
extern short gsn_installmodule;
extern short gsn_showmodules;
extern short gsn_shipmaintenance;
extern short gsn_scanbugs;
extern short gsn_removebug;
extern short gsn_removemodule;

/* These are bh skills */
extern short gsn_ambush;
extern short gsn_bind;
extern short gsn_gag;

extern short gsn_battle_command;
extern short gsn_reinforcements;
extern short gsn_postguard;

extern short gsn_addpatrol;
extern short gsn_eliteguard;
extern short gsn_specialforces;
extern short gsn_jail;
extern short gsn_smalltalk;
extern short gsn_propeganda;
extern short gsn_bribe;
extern short gsn_seduce;
extern short gsn_masspropeganda;
extern short gsn_gather_intelligence;

/* hunter assassin gsn ints */
extern short gsn_plantbug;
extern short gsn_showbugs;
extern short gsn_silent;
extern short gsn_retreat;

/* The gsn ints for the slicers */
extern short gsn_spy;
extern short gsn_makecommsystem;
extern short gsn_sabotage;
extern short gsn_commsystem;
extern short gsn_codecrack;
extern short gsn_slicebank;
extern short gsn_inquire;
extern short gsn_makedatapad;
extern short gsn_disable;
extern short gsn_assignpilot;
extern short gsn_checkprints;


extern short gsn_torture;
extern short gsn_snipe;
extern short gsn_throw;
extern short gsn_deception;
extern short gsn_disguise;
extern short gsn_mine;
extern short gsn_first_aid;

extern short gsn_beg;
extern short gsn_makeblade;
extern short gsn_makebug;
extern short gsn_makebeacon;
extern short gsn_makepike;
extern short gsn_makejewelry;
extern short gsn_makeblaster;
extern short gsn_makelight;
extern short gsn_makecomlink;
extern short gsn_makegrenade;
extern short gsn_makeshipbomb;
extern short gsn_makelandmine;
extern short gsn_makearmor;
extern short gsn_makeshield;
extern short gsn_makecontainer;
extern short gsn_gemcutting;
extern short gsn_makelightsaber;
extern short gsn_makeduallightsaber;
extern short gsn_repair;
extern short gsn_shiprepair;
extern short gsn_spice_refining;

extern short gsn_detrap;
extern short gsn_backstab;
extern short gsn_dualstab;
extern short gsn_bargain;
extern short gsn_circle;
extern short gsn_dodge;
extern short gsn_hide;
extern short gsn_concealment;
extern short gsn_peek;
extern short gsn_pick_lock;
extern short gsn_scan;
extern short gsn_sneak;
extern short gsn_steal;
extern short gsn_gouge;
extern short gsn_track;
extern short gsn_search;
extern short gsn_dig;
extern short gsn_mount;
extern short gsn_bashdoor;
extern short gsn_berserk;
extern short gsn_hitall;
extern short gsn_pickshiplock;
extern short gsn_hijack;

extern short gsn_disarm;
extern short gsn_enhanced_damage;
extern short gsn_kick;
extern short gsn_parry;
extern short gsn_rescue;
extern short gsn_second_attack;
extern short gsn_third_attack;
extern short gsn_dual_wield;
extern short gsn_reflect;


extern short gsn_aid;
extern short gsn_plantbeacon;
extern short gsn_showbeacons;
extern short gsn_checkbeacons;
extern short gsn_nullifybeacons;
extern short gsn_makebinders;
extern short gsn_launchers;
extern short gsn_makemissile;
extern short gsn_makegoggles;
extern short gsn_truesight;
extern short gsn_barrelroll;
extern short gsn_juke;

/* used to do specific lookups */
extern short gsn_first_spell;
extern short gsn_first_skill;
extern short gsn_first_weapon;
extern short gsn_first_tongue;
extern short gsn_top_sn;

/* spells */
extern short gsn_blindness;
extern short gsn_charm_person;
extern short gsn_aqua_breath;
extern short gsn_invis;
extern short gsn_mass_invis;
extern short gsn_poison;
extern short gsn_sleep;
extern short gsn_possess;
extern short gsn_fireball; /* for fireshield  */
extern short gsn_lightning_bolt; /* for shockshield */

/* newer attack skills */
extern short gsn_punch;
extern short gsn_bash;
extern short gsn_stun;

extern short gsn_poison_weapon;
extern short gsn_climb;

extern short gsn_blasters;
extern short gsn_force_pikes;
extern short gsn_bowcasters;
extern short gsn_lightsabers;
extern short gsn_vibro_blades;
extern short gsn_flexible_arms;
extern short gsn_talonous_arms;
extern short gsn_bludgeons;

extern short gsn_grip;

/* languages */
extern short gsn_basic;
extern short gsn_wookiee;
extern short gsn_twilek;
extern short gsn_rodian;
extern short gsn_hutt;
extern short gsn_mon_calamari;
extern short gsn_noghri;
extern short gsn_ewok;
extern short gsn_ithorian;
extern short gsn_gotal;
extern short gsn_devaronian;
extern short gsn_binary;
extern short gsn_firrerreo;
extern short gsn_gamorrean;
extern short gsn_togorian;
extern short gsn_shistavanen;
extern short gsn_jawa;
extern short gsn_kubaz;
extern short gsn_verpine;
extern short gsn_defel;
extern short gsn_trandoshan;
extern short gsn_hapan;
extern short gsn_quarren;
extern short gsn_sullustan;
extern short gsn_falleen;
extern short gsn_barabel;
extern short gsn_yevethan;
extern short gsn_gand;
extern short gsn_coynite;
extern short gsn_duinuogwuin;

// Utility macros.
int umin( int check, int ncheck );
int umax( int check, int ncheck );
int urange( int mincheck, int check, int maxcheck );

#define UMIN( a, b )      ( umin( (a), (b) ) )
#define UMAX( a, b )      ( umax( (a), (b) ) )
#define URANGE(a, b, c )  ( urange( (a), (b), (c) ) )
#define LOWER( c )        ( (c) >= 'A' && (c) <= 'Z' ? (c) + 'a' - 'A' : (c) )
#define UPPER( c )        ( (c) >= 'a' && (c) <= 'z' ? (c) + 'A' - 'a' : (c) )

#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))
#define CH(d)			((d)->original ? (d)->original : (d)->character)

/*
 * Memory allocation macros.
 */
#define CREATE(result, type, number)                                    \
do                                                                      \
{                                                                       \
   if (!((result) = (type *) calloc ((number), sizeof(type))))          \
   {                                                                    \
      perror("malloc failure");                                         \
      fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
      abort();                                                          \
   }                                                                    \
} while(0)

#define RECREATE(result,type,number)                                    \
do                                                                      \
{                                                                       \
   if(!((result) = (type *)realloc((result), sizeof(type) * (number)))) \
   {                                                                    \
      perror("realloc failure");                                        \
      fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__); \
      abort();                                                          \
   }                                                                    \
} while(0)

#if defined(__FreeBSD__)
#define DISPOSE(point)                      \
do                                          \
{                                           \
   if( (point) )                            \
   {                                        \
      free( (void*) (point) );              \
      (point) = NULL;                       \
   }                                        \
} while(0)
#else
#define DISPOSE(point)                         \
do                                             \
{                                              \
   if( (point) )                               \
   {                                           \
      if( typeid((point)) == typeid(char*) || typeid((point)) == typeid(const char*) ) \
      {                                        \
         if( in_hash_table( (char*)(point) ) ) \
         {                                     \
            log_printf( "&RDISPOSE called on STRALLOC pointer: %s, line %d\n", __FILE__, __LINE__ ); \
            log_string( "Attempting to correct." ); \
            if( str_free( (char*)(point) ) == -1 ) \
               log_printf( "&RSTRFREEing bad pointer: %s, line %d\n", __FILE__, __LINE__ ); \
         }                                     \
         else                                  \
            free( (void*) (point) );           \
      }                                        \
      else                                     \
         free( (void*) (point) );              \
      (point) = NULL;                          \
   }                                           \
   else                                          \
      (point) = NULL;                            \
} while(0)
#endif

#define STRALLOC(point)		str_alloc((point))
#define QUICKLINK(point)	quick_link((point))
#if defined(__FreeBSD__)
#define STRFREE(point)                          \
do                                              \
{                                               \
   if((point))                                  \
   {                                            \
      if( str_free((point)) == -1 )             \
         bug( "%s: &RSTRFREEing bad pointer: %s, line %d", __func__, __FILE__, __LINE__ ); \
      (point) = NULL;                           \
   }                                            \
} while(0)
#else
#define STRFREE(point)                           \
do                                               \
{                                                \
   if((point))                                   \
   {                                             \
      if( !in_hash_table( (point) ) )            \
      {                                          \
         log_printf( "&RSTRFREE called on strdup pointer: %s, line %d\n", __FILE__, __LINE__ ); \
         log_string( "Attempting to correct." ); \
         free( (void*) (point) );                \
      }                                          \
      else if( str_free((point)) == -1 )         \
         log_printf( "&RSTRFREEing bad pointer: %s, line %d\n", __FILE__, __LINE__ ); \
      (point) = NULL;                            \
   }                                             \
   else                                          \
      (point) = NULL;                            \
} while(0)
#endif

/* double-linked list handling macros -Thoric */
/* Updated by Scion 8/6/1999 */
#define LINK(link, first, last, next, prev) \
do                                          \
{                                           \
   if ( !(first) )                          \
   {                                        \
      (first) = (link);                     \
      (last) = (link);                      \
   }                                        \
   else                                     \
      (last)->next = (link);                \
   (link)->next = NULL;                     \
   if ((first) == (link))                   \
      (link)->prev = NULL;                  \
   else                                     \
      (link)->prev = (last);                \
   (last) = (link);                         \
} while(0)

#define INSERT(link, insert, first, next, prev) \
do                                              \
{                                               \
   (link)->prev = (insert)->prev;               \
   if ( !(insert)->prev )                       \
      (first) = (link);                         \
   else                                         \
      (insert)->prev->next = (link);            \
   (insert)->prev = (link);                     \
   (link)->next = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)   \
do                                              \
{                                               \
   if ( !(link)->prev )                         \
   {                                            \
      (first) = (link)->next;                   \
      if ((first))                              \
         (first)->prev = NULL;                  \
   }                                            \
   else                                         \
   {                                            \
      (link)->prev->next = (link)->next;        \
   }                                            \
   if ( !(link)->next )                         \
   {                                            \
      (last) = (link)->prev;                    \
      if((last))                                \
         (last)->next = NULL;                   \
   }                                            \
   else                                         \
   {                                            \
      (link)->next->prev = (link)->prev;        \
   }                                            \
} while(0)

#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "%s: CHECK_LINKS: last with NULL first!  %s.",		\
        __func__, __STRING(first) );				\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "%s: CHECK_LINKS: first with NULL last!  %s.",		\
        __func__, __STRING(first) );				\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "%s: CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            __func__, __STRING(first), ptr, pptr );		\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "%s: CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            __func__, __STRING(first), ptr, ptr );		\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "%s: CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            __func__, __STRING(first), ptr, pptr );		\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "%s: CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            __func__, __STRING(first), ptr, ptr );		\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)

#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
	fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\r\n", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		(get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_DROID(ch)		(ch->race == RACE_DROID || ch->race == RACE_PROTOCAL_DROID || ch->race == RACE_ASSASSIN_DROID  || ch->race == RACE_GLADIATOR_DROID || ch->race == RACE_ASTROMECH_DROID || ch->race == RACE_INTERROGATION_DROID)
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_HERO)
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || IS_SET((ch)->xflags, (part)))

#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)
#define GET_AC(ch)		( (ch)->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) \
				- ( (ch)->race == RACE_DEFEL ? (ch)->skill_level[COMBAT_ABILITY]*7+5 : (ch)->skill_level[COMBAT_ABILITY]/2 ) )
#define GET_HITROLL(ch)		((ch)->hitroll				    \
				    +str_app[get_curr_str(ch)].tohit	    \
				    +(2-(abs((ch)->mental_state)/10)))
#define GET_DAMROLL(ch)		((ch)->damroll                              \
				    +str_app[get_curr_str(ch)].todam	    \
				    +(((ch)->mental_state > 5		    \
				    &&(ch)->mental_state < 15) ? 1 : 0) )

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS) && !IS_SET(               \
				    (ch)->in_room->room_flags,              \
				    ROOM_SPACECRAFT) )

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    )

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))


#define EXIT(ch, door)		( get_exit( (ch)->in_room, door ) )

#define CAN_GO(ch, door)	(EXIT((ch),(door))			 \
				&& (EXIT((ch),(door))->to_room != NULL)  \
                          	&& !IS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name )

#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name )

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )
#define SPELL_DAMAGE(skill)	( ((skill)->flags     ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->flags >> 3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->flags >> 6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->flags >> 9) & 3 )
#define SET_SDAM(skill, val)	( (skill)->flags =  ((skill)->flags & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->flags =  ((skill)->flags & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->flags =  ((skill)->flags & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->flags =  ((skill)->flags & SPOW_MASK) + (((val) & 3) << 9) )

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_GUEST))

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE )
#define IS_COLD(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_COLD )
#define IS_ACID(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ACID )
#define IS_ELECTRICITY(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ELECTRICITY )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )

#define IS_DRAIN(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_DRAIN )

#define IS_POISON(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_POISON )


#define NOT_AUTHED(ch)		(!IS_NPC(ch) && ch->pcdata->auth_state <= 3  \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define HAS_SLUG(ch)		(!IS_NPC(ch) && IS_SET(ch->pcdata->flags, \
				PCFLAG_HASSLUG) )
#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->desc		     \
			      && ch->pcdata->auth_state == 1		     \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))

#define log_string( txt )	( log_string_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
   CMDTYPE *next;
   char *name;
   DO_FUN *do_fun;
   char *fun_name;
   short position;
   short level;
   short log;
   int flags;
   struct timerset userec;
};

/*
 * Structure for a social in the socials table.
 */
struct social_type
{
   SOCIALTYPE *next;
   char *name;
   char *char_no_arg;
   char *others_no_arg;
   char *char_found;
   char *others_found;
   char *vict_found;
   char *char_auto;
   char *others_auto;
};

struct specfun_list
{
   SPEC_LIST *next;
   SPEC_LIST *prev;
   char *name;
};

/*
 * Global constants.
 */
extern time_t last_restore_all_time;
extern time_t boot_time;   /* this should be moved down */
extern HOUR_MIN_SEC *set_boot_time;
extern struct tm *new_boot_time;
extern time_t new_boot_time_t;
extern bool mud_down;
extern FILE *fpArea;
extern char strArea[MAX_INPUT_LENGTH];

extern const struct str_app_type str_app[26];
extern const struct int_app_type int_app[26];
extern const struct wis_app_type wis_app[26];
extern const struct dex_app_type dex_app[30];
extern const struct con_app_type con_app[26];
extern const struct cha_app_type cha_app[26];
extern const struct lck_app_type lck_app[26];
extern const struct frc_app_type frc_app[26];
extern struct race_type race_table[MAX_RACE];
extern const struct liq_type liq_table[LIQ_MAX];
extern const char *const attack_table[13];
extern const char *const ability_name[MAX_ABILITY];
extern const char *const height_name[4];
extern const char *const build_name[6];
extern const char *const droid_name[8];

extern const char *const skill_tname[];
extern short const movement_loss[SECT_MAX];
extern const char *const dir_name[];
extern const char *const where_name[];
extern const short rev_dir[];
extern const int trap_door[];
extern const char *const r_flags[];
extern const char *const r_flags2[];
extern const char *const w_flags[];
extern const char *const o_flags[];
extern const char *const a_flags[];
extern const char *const o_types[];
extern const char *const a_types[];
extern const char *const cmd_flags[];
extern const char *const act_flags[];
extern const char *const planet_flags[];
extern const char *const mprog_flags[];
extern const char *const weapon_table[13];
extern const char *const spice_table[];
extern const char *const plr_flags[];
extern const char *const pc_flags[];
extern const char *const trap_flags[];
extern const char *const ris_flags[];
extern const char *const trig_flags[];
extern const char *const part_flags[];
extern const char *const npc_race[];
extern const char *const defense_flags[];
extern const char *const attack_flags[];
extern const char *const area_flags[];
extern const char *const wear_locs[];
extern int const lang_array[];
extern const char *const lang_names[];
extern const char *const lang_names_save[];
extern const char *sector_name[SECT_MAX];

extern bool bootup;
extern char namefreq[MAX_STRING_LENGTH];
extern char bname[MAX_STRING_LENGTH];

/*
 * Global variables.
 */
extern MPSLEEP_DATA *first_mpwait;  /* Storing sleeping mud progs */
extern MPSLEEP_DATA *last_mpwait;   /* - */
extern MPSLEEP_DATA *current_mpwait;   /* - */
extern int numobjsloaded;
extern int nummobsloaded;
extern int physicalobjects;
extern int num_descriptors;
extern struct system_data sysdata;
extern int top_sn;
extern int top_vroom;
extern int top_herb;

extern CMDTYPE *command_hash[126];

extern SKILLTYPE *skill_table[MAX_SKILL];
extern SOCIALTYPE *social_index[27];
extern CHAR_DATA *cur_char;
extern ROOM_INDEX_DATA *cur_room;
extern bool cur_char_died;
extern ch_ret global_retcode;
extern SKILLTYPE *herb_table[MAX_HERB];

extern int cur_obj;
extern int cur_obj_serial;
extern bool cur_obj_extracted;
extern obj_ret global_objcode;

extern HELP_DATA *first_help;
extern HELP_DATA *last_help;
extern SHOP_DATA *first_shop;
extern SHOP_DATA *last_shop;
extern REPAIR_DATA *first_repair;
extern REPAIR_DATA *last_repair;

extern BAN_DATA *first_ban;
extern BAN_DATA *last_ban;
extern CHAR_DATA *first_char;
extern CHAR_DATA *last_char;
extern DESCRIPTOR_DATA *first_descriptor;
extern DESCRIPTOR_DATA *last_descriptor;
extern BOARD_DATA *first_board;
extern BOARD_DATA *last_board;
extern OBJ_DATA *first_object;
extern OBJ_DATA *last_object;
extern CLAN_DATA *first_clan;
extern CLAN_DATA *last_clan;
extern GUARD_DATA *first_guard;
extern GUARD_DATA *last_guard;
extern SHIP_DATA *first_ship;
extern SHIP_DATA *last_ship;
extern SHIP_PROTOTYPE *first_ship_prototype;
extern SHIP_PROTOTYPE *last_ship_prototype;
extern SPACE_DATA *first_starsystem;
extern SPACE_DATA *last_starsystem;
extern PLANET_DATA *first_planet;
extern PLANET_DATA *last_planet;
extern SENATE_DATA *first_senator;
extern SENATE_DATA *last_senator;
extern BOUNTY_DATA *first_bounty;
extern BOUNTY_DATA *last_bounty;
extern BOUNTY_DATA *first_disintegration;
extern BOUNTY_DATA *last_disintegration;
extern AREA_DATA *first_area;
extern AREA_DATA *last_area;
extern AREA_DATA *first_build;
extern AREA_DATA *last_build;
extern AREA_DATA *first_asort;
extern AREA_DATA *last_asort;
extern AREA_DATA *first_bsort;
extern AREA_DATA *last_bsort;
extern AREA_DATA *first_area_name;  /*alphanum. sort */
extern AREA_DATA *last_area_name;   /* Fireblade */
extern SPEC_LIST *first_specfun;
extern SPEC_LIST *last_specfun;

extern TELEPORT_DATA *first_teleport;
extern TELEPORT_DATA *last_teleport;
extern OBJ_DATA *extracted_obj_queue;
extern EXTRACT_CHAR_DATA *extracted_char_queue;
extern OBJ_DATA *save_equipment[MAX_WEAR][MAX_LAYERS];
extern CHAR_DATA *quitting_char;
extern CHAR_DATA *loading_char;
extern CHAR_DATA *saving_char;
extern OBJ_DATA *all_obj;

extern time_t current_time;
extern bool fLogAll;
extern bool fLogPC;
extern char log_buf[];
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;

extern AUCTION_DATA *auction;
extern struct act_prog_data *mob_act_list;

extern BMARKET_DATA *first_market_ship;
extern BMARKET_DATA *last_market_ship;

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_buymobship );
DECLARE_DO_FUN( do_aaccept );
DECLARE_DO_FUN( do_ahall );
DECLARE_DO_FUN( do_arena );
DECLARE_DO_FUN( do_awho );
DECLARE_DO_FUN( do_bet );
DECLARE_DO_FUN( do_challenge );
DECLARE_DO_FUN( do_chaos );
DECLARE_DO_FUN( do_cut );
DECLARE_DO_FUN( do_adecline );
DECLARE_DO_FUN( do_setmssp );
DECLARE_DO_FUN( do_setplanet );
DECLARE_DO_FUN( do_setrank );
DECLARE_DO_FUN( do_setinfrared );
DECLARE_DO_FUN( do_makefree );
DECLARE_DO_FUN( do_makeplanet );
DECLARE_DO_FUN( do_makeprototypeship );
DECLARE_DO_FUN( do_planets );
DECLARE_DO_FUN( do_teach );
DECLARE_DO_FUN( do_gather_intelligence );
DECLARE_DO_FUN( do_generate_market );
DECLARE_DO_FUN( do_add_patrol );
DECLARE_DO_FUN( do_special_forces );
DECLARE_DO_FUN( do_jail );
DECLARE_DO_FUN( do_checkwar );
DECLARE_DO_FUN( do_elite_guard );
DECLARE_DO_FUN( do_smalltalk );
DECLARE_DO_FUN( do_propeganda );
DECLARE_DO_FUN( do_bribe );
DECLARE_DO_FUN( do_bind );
DECLARE_DO_FUN( do_seduce );
DECLARE_DO_FUN( do_mass_propeganda );
DECLARE_DO_FUN( do_copyship );
DECLARE_DO_FUN( do_sound );
DECLARE_DO_FUN( do_autopilot );
DECLARE_DO_FUN( do_allspeeders );
DECLARE_DO_FUN( do_speeders );
DECLARE_DO_FUN( do_suicide );
DECLARE_DO_FUN( do_gain );
DECLARE_DO_FUN( do_train );
DECLARE_DO_FUN( do_beg );
DECLARE_DO_FUN( do_bank );
DECLARE_DO_FUN( do_battle_command );
DECLARE_DO_FUN( do_hijack );
DECLARE_DO_FUN( do_pickshiplock );
DECLARE_DO_FUN( do_shipstat );
DECLARE_DO_FUN( do_shiptalk );
DECLARE_DO_FUN( do_clone );
DECLARE_DO_FUN( do_systemtalk );
DECLARE_DO_FUN( do_spacetalk );
DECLARE_DO_FUN( do_hail );
DECLARE_DO_FUN( do_allships );
DECLARE_DO_FUN( do_newclan );
DECLARE_DO_FUN( do_appoint );
DECLARE_DO_FUN( do_demote );
DECLARE_DO_FUN( do_empower );
DECLARE_DO_FUN( do_capture );
DECLARE_DO_FUN( do_arm );
DECLARE_DO_FUN( do_addchange );
DECLARE_DO_FUN( do_changes );
DECLARE_DO_FUN( do_chaff );
DECLARE_DO_FUN( do_clan_donate );
DECLARE_DO_FUN( do_clan_withdraw );
DECLARE_DO_FUN( do_fly );
DECLARE_DO_FUN( do_drive );
DECLARE_DO_FUN( do_bomb );
DECLARE_DO_FUN( do_setblaster );
DECLARE_DO_FUN( do_ammo );
DECLARE_DO_FUN( do_ambush );
DECLARE_DO_FUN( do_takedrug );
DECLARE_DO_FUN( do_use );
DECLARE_DO_FUN( do_link );
DECLARE_DO_FUN( do_unlink );
DECLARE_DO_FUN( do_load );
DECLARE_DO_FUN( do_unload );
DECLARE_DO_FUN( do_enlist );
DECLARE_DO_FUN( do_resign );
DECLARE_DO_FUN( do_retune );
DECLARE_DO_FUN( do_retreat );
DECLARE_DO_FUN( do_reward );
DECLARE_DO_FUN( do_pluogus );
DECLARE_DO_FUN( do_makemodule );
DECLARE_DO_FUN( do_showmodules );
DECLARE_DO_FUN( do_installmodule );
DECLARE_DO_FUN( do_removebug );
DECLARE_DO_FUN( do_removemodule );
DECLARE_DO_FUN( do_tractorbeam );
DECLARE_DO_FUN( do_makearmor );
DECLARE_DO_FUN( do_makejewelry );
DECLARE_DO_FUN( do_makegrenade );
DECLARE_DO_FUN( do_makeshipbomb );
DECLARE_DO_FUN( do_makelandmine );
DECLARE_DO_FUN( do_makelight );
DECLARE_DO_FUN( do_makecomlink );
DECLARE_DO_FUN( do_makeshield );
DECLARE_DO_FUN( do_makecontainer );
DECLARE_DO_FUN( do_makemissile );
DECLARE_DO_FUN( do_gemcutting );
DECLARE_DO_FUN( do_reinforcements );
DECLARE_DO_FUN( do_postguard );
DECLARE_DO_FUN( do_torture );
DECLARE_DO_FUN( do_snipe );
DECLARE_DO_FUN( do_throw );
DECLARE_DO_FUN( do_deception );
DECLARE_DO_FUN( do_disguise );
DECLARE_DO_FUN( do_mine );
DECLARE_DO_FUN( do_first_aid );
DECLARE_DO_FUN( do_make_master );
DECLARE_DO_FUN( do_makeblade );
DECLARE_DO_FUN( do_makebeacon );
DECLARE_DO_FUN( do_makebinders );
DECLARE_DO_FUN( do_makebug );
DECLARE_DO_FUN( do_makegoggles );
DECLARE_DO_FUN( do_spousetalk );
DECLARE_DO_FUN( do_makepike );
DECLARE_DO_FUN( do_makeblaster );
DECLARE_DO_FUN( do_makelightsaber );
DECLARE_DO_FUN( do_makeduallightsaber );
DECLARE_DO_FUN( do_makespice );
DECLARE_DO_FUN( do_closebay );
DECLARE_DO_FUN( do_openbay );
DECLARE_DO_FUN( do_autotrack );
DECLARE_DO_FUN( do_jumpvector );
DECLARE_DO_FUN( do_reload );
DECLARE_DO_FUN( do_radar );
DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_buyship );
DECLARE_DO_FUN( do_buytroops );
DECLARE_DO_FUN( do_buyhome );
DECLARE_DO_FUN( do_setforcer );
DECLARE_DO_FUN( do_clanbuyship );
DECLARE_DO_FUN( do_clangiveship );
DECLARE_DO_FUN( do_clansalvage );
DECLARE_DO_FUN( do_clanbuytroops );
DECLARE_DO_FUN( do_gatherclans );
DECLARE_DO_FUN( do_sellship );
DECLARE_DO_FUN( do_autorecharge );
DECLARE_DO_FUN( do_openhatch );
DECLARE_DO_FUN( do_closehatch );
DECLARE_DO_FUN( do_status );
DECLARE_DO_FUN( do_std );
DECLARE_DO_FUN( do_info );
DECLARE_DO_FUN( do_introduce );
DECLARE_DO_FUN( do_remember );
DECLARE_DO_FUN( do_describe );
DECLARE_DO_FUN( do_hyperspace );
DECLARE_DO_FUN( do_target );
DECLARE_DO_FUN( do_fire );
DECLARE_DO_FUN( do_calculate );
DECLARE_DO_FUN( do_recharge );
DECLARE_DO_FUN( do_shiprepair );
DECLARE_DO_FUN( do_shipmaintenance );
DECLARE_DO_FUN( do_shiplist );
DECLARE_DO_FUN( do_refuel );
DECLARE_DO_FUN( do_addpilot );
DECLARE_DO_FUN( do_rempilot );
DECLARE_DO_FUN( do_remclan );
DECLARE_DO_FUN( do_removeship );
DECLARE_DO_FUN( do_trajectory );
DECLARE_DO_FUN( do_accelerate );
DECLARE_DO_FUN( do_launch );
DECLARE_DO_FUN( do_land );
DECLARE_DO_FUN( do_leaveship );
DECLARE_DO_FUN( do_setstarsystem );
DECLARE_DO_FUN( do_makestarsystem );
DECLARE_DO_FUN( do_makesimulator );
DECLARE_DO_FUN( do_makemobship );
DECLARE_DO_FUN( do_starsystems );
DECLARE_DO_FUN( do_showstarsystem );
DECLARE_DO_FUN( skill_notfound );
DECLARE_DO_FUN( do_aassign );
DECLARE_DO_FUN( do_addbounty );
DECLARE_DO_FUN( do_contract );
DECLARE_DO_FUN( do_showcontracts );
DECLARE_DO_FUN( do_remcontract );
DECLARE_DO_FUN( do_vassign );
DECLARE_DO_FUN( do_advance );
DECLARE_DO_FUN( do_affected );
DECLARE_DO_FUN( do_afk );
DECLARE_DO_FUN( do_aid );
DECLARE_DO_FUN( do_rembounty );
DECLARE_DO_FUN( do_allow );
DECLARE_DO_FUN( do_ansi );
DECLARE_DO_FUN( do_answer );
DECLARE_DO_FUN( do_apply );
DECLARE_DO_FUN( do_appraise );
DECLARE_DO_FUN( do_areas );
DECLARE_DO_FUN( do_aset );
DECLARE_DO_FUN( do_ask );
DECLARE_DO_FUN( do_astat );
DECLARE_DO_FUN( do_at );
DECLARE_DO_FUN( do_auction );
DECLARE_DO_FUN( do_authorize );
DECLARE_DO_FUN( do_avtalk );
DECLARE_DO_FUN( do_backstab );
DECLARE_DO_FUN( do_backup );
DECLARE_DO_FUN( do_balzhur );
DECLARE_DO_FUN( do_bamfin );
DECLARE_DO_FUN( do_bamfout );
DECLARE_DO_FUN( do_ban );
DECLARE_DO_FUN( do_bash );
DECLARE_DO_FUN( do_bashdoor );
DECLARE_DO_FUN( do_beep );
DECLARE_DO_FUN( do_berserk );
DECLARE_DO_FUN( do_bestow );
DECLARE_DO_FUN( do_bestowarea );
DECLARE_DO_FUN( do_bio );
DECLARE_DO_FUN( do_bite );
DECLARE_DO_FUN( do_board );
DECLARE_DO_FUN( do_boards );
DECLARE_DO_FUN( do_bodybag );
DECLARE_DO_FUN( do_bounties );
DECLARE_DO_FUN( do_brandish );
DECLARE_DO_FUN( do_brew );
DECLARE_DO_FUN( do_bset );
DECLARE_DO_FUN( do_bstat );
DECLARE_DO_FUN( do_bug );
DECLARE_DO_FUN( do_bury );
DECLARE_DO_FUN( do_buy );
DECLARE_DO_FUN( do_cast );
DECLARE_DO_FUN( do_cedit );
DECLARE_DO_FUN( do_channels );
DECLARE_DO_FUN( do_chat );
DECLARE_DO_FUN( do_ooc );
DECLARE_DO_FUN( do_check_vnums );
DECLARE_DO_FUN( do_circle );
DECLARE_DO_FUN( do_clans );
DECLARE_DO_FUN( do_clanstat );
DECLARE_DO_FUN( do_ships );
DECLARE_DO_FUN( do_clantalk );
DECLARE_DO_FUN( do_claw );
DECLARE_DO_FUN( do_cleanroom );
DECLARE_DO_FUN( do_climb );
DECLARE_DO_FUN( do_close );
DECLARE_DO_FUN( do_cmdtable );
DECLARE_DO_FUN( do_cmenu );
DECLARE_DO_FUN( do_commands );
DECLARE_DO_FUN( do_comment );
DECLARE_DO_FUN( do_compare );
DECLARE_DO_FUN( do_config );
DECLARE_DO_FUN( do_consider );
DECLARE_DO_FUN( do_senate );
DECLARE_DO_FUN( do_addsenator );
DECLARE_DO_FUN( do_remsenator );
DECLARE_DO_FUN( do_concealment );
DECLARE_DO_FUN( do_credits );
DECLARE_DO_FUN( do_cset );
DECLARE_DO_FUN( do_deities );
DECLARE_DO_FUN( do_deny );
DECLARE_DO_FUN( do_description );
DECLARE_DO_FUN( do_destro );
DECLARE_DO_FUN( do_destroy );
DECLARE_DO_FUN( do_detrap );
DECLARE_DO_FUN( do_devote );
DECLARE_DO_FUN( do_dig );
DECLARE_DO_FUN( do_disarm );
DECLARE_DO_FUN( do_disconnect );
DECLARE_DO_FUN( do_dismount );
DECLARE_DO_FUN( do_dmesg );
DECLARE_DO_FUN( do_down );
DECLARE_DO_FUN( do_drag );
DECLARE_DO_FUN( do_drink );
DECLARE_DO_FUN( do_drop );
DECLARE_DO_FUN( do_droptroops );
DECLARE_DO_FUN( do_diagnose );
DECLARE_DO_FUN( do_dualstab );
DECLARE_DO_FUN( do_east );
DECLARE_DO_FUN( do_eat );
DECLARE_DO_FUN( do_echo );
DECLARE_DO_FUN( do_email );
DECLARE_DO_FUN( do_emote );
DECLARE_DO_FUN( do_empty );
DECLARE_DO_FUN( do_enter );
DECLARE_DO_FUN( do_equipment );
DECLARE_DO_FUN( do_examine );
DECLARE_DO_FUN( do_exempt );
DECLARE_DO_FUN( do_exits );
DECLARE_DO_FUN( do_endsimulator );
DECLARE_DO_FUN( do_feed );
DECLARE_DO_FUN( do_fill );
DECLARE_DO_FUN( do_fixchar );
DECLARE_DO_FUN( do_flee );
DECLARE_DO_FUN( do_foldarea );
DECLARE_DO_FUN( do_follow );
DECLARE_DO_FUN( do_for );
DECLARE_DO_FUN( do_force );
DECLARE_DO_FUN( do_fset );
DECLARE_DO_FUN( do_forceclose );
DECLARE_DO_FUN( do_fquit );   /* Gorog */
DECLARE_DO_FUN( do_form_password );
DECLARE_DO_FUN( do_freeze );
DECLARE_DO_FUN( do_fslay );
DECLARE_DO_FUN( do_gag );
DECLARE_DO_FUN( do_get );
DECLARE_DO_FUN( do_give );
DECLARE_DO_FUN( do_giveship );
DECLARE_DO_FUN( do_giveslug );
DECLARE_DO_FUN( do_glance );
DECLARE_DO_FUN( do_gold );
DECLARE_DO_FUN( do_goto );
DECLARE_DO_FUN( do_gouge );
DECLARE_DO_FUN( do_group );
DECLARE_DO_FUN( do_grub );
DECLARE_DO_FUN( do_gtell );
DECLARE_DO_FUN( do_guilds );
DECLARE_DO_FUN( do_guildtalk );
DECLARE_DO_FUN( do_hedit );
DECLARE_DO_FUN( do_hell );
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_hide );
DECLARE_DO_FUN( do_hitall );
DECLARE_DO_FUN( do_hlist );
DECLARE_DO_FUN( do_holylight );
DECLARE_DO_FUN( do_holonet );
DECLARE_DO_FUN( do_homepage );
DECLARE_DO_FUN( do_hset );
DECLARE_DO_FUN( do_i103 );
DECLARE_DO_FUN( do_i104 );
DECLARE_DO_FUN( do_i105 );
DECLARE_DO_FUN( do_ide );
DECLARE_DO_FUN( do_idea );
DECLARE_DO_FUN( do_idealog );
DECLARE_DO_FUN( do_immortalize );
DECLARE_DO_FUN( do_immtalk );
DECLARE_DO_FUN( do_induct );
DECLARE_DO_FUN( do_installarea );
DECLARE_DO_FUN( do_instaroom );
DECLARE_DO_FUN( do_instazone );
DECLARE_DO_FUN( do_inventory );
DECLARE_DO_FUN( do_invis );
DECLARE_DO_FUN( do_kick );
DECLARE_DO_FUN( do_kill );
DECLARE_DO_FUN( do_languages );
DECLARE_DO_FUN( do_last );
DECLARE_DO_FUN( do_leave );
DECLARE_DO_FUN( do_level );
DECLARE_DO_FUN( do_light );
DECLARE_DO_FUN( do_list );
DECLARE_DO_FUN( do_litterbug );
DECLARE_DO_FUN( do_loadarea );
DECLARE_DO_FUN( do_loadup );
DECLARE_DO_FUN( do_lock );
DECLARE_DO_FUN( do_log );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_low_purge );
DECLARE_DO_FUN( do_mailroom );
DECLARE_DO_FUN( do_make );
DECLARE_DO_FUN( do_makeboard );
DECLARE_DO_FUN( do_makeclan );
DECLARE_DO_FUN( do_makeship );
DECLARE_DO_FUN( do_makeship2 );
DECLARE_DO_FUN( do_makeguild );
DECLARE_DO_FUN( do_makerepair );
DECLARE_DO_FUN( do_makeshop );
DECLARE_DO_FUN( do_makewizlist );
DECLARE_DO_FUN( do_members );
DECLARE_DO_FUN( do_memory );
DECLARE_DO_FUN( do_mcreate );
DECLARE_DO_FUN( do_mdelete );
DECLARE_DO_FUN( do_mfind );
DECLARE_DO_FUN( do_minvoke );
DECLARE_DO_FUN( do_mlist );
DECLARE_DO_FUN( do_mount );
DECLARE_DO_FUN( do_mset );
DECLARE_DO_FUN( do_mstat );
DECLARE_DO_FUN( do_murde );
DECLARE_DO_FUN( do_murder );
DECLARE_DO_FUN( do_music );
DECLARE_DO_FUN( do_mwhere );
DECLARE_DO_FUN( do_name );
DECLARE_DO_FUN( do_newbiechat );
DECLARE_DO_FUN( do_newbieset );
DECLARE_DO_FUN( do_newzones );
DECLARE_DO_FUN( do_noemote );
DECLARE_DO_FUN( do_noresolve );
DECLARE_DO_FUN( do_north );
DECLARE_DO_FUN( do_northeast );
DECLARE_DO_FUN( do_northwest );
DECLARE_DO_FUN( do_notell );
DECLARE_DO_FUN( do_notitle );
DECLARE_DO_FUN( do_noteroom );
DECLARE_DO_FUN( do_ocreate );
DECLARE_DO_FUN( do_odelete );
DECLARE_DO_FUN( do_ofind );
DECLARE_DO_FUN( do_ogrub );
DECLARE_DO_FUN( do_oinvoke );
DECLARE_DO_FUN( do_oldmstat );
DECLARE_DO_FUN( do_oldscore );
DECLARE_DO_FUN( do_olist );
DECLARE_DO_FUN( do_open );
DECLARE_DO_FUN( do_opentourney );
DECLARE_DO_FUN( do_order );
DECLARE_DO_FUN( do_orders );
DECLARE_DO_FUN( do_ordership );
DECLARE_DO_FUN( do_orderclanship );
DECLARE_DO_FUN( do_ordertalk );
DECLARE_DO_FUN( do_oset );
DECLARE_DO_FUN( do_ostat );
DECLARE_DO_FUN( do_ot );
DECLARE_DO_FUN( do_outcast );
DECLARE_DO_FUN( do_outlaw );
DECLARE_DO_FUN( do_owhere );
DECLARE_DO_FUN( do_pager );
DECLARE_DO_FUN( do_pardon );
DECLARE_DO_FUN( do_password );
DECLARE_DO_FUN( do_peace );
DECLARE_DO_FUN( do_pick );
DECLARE_DO_FUN( do_poison_weapon );
DECLARE_DO_FUN( do_pose );
DECLARE_DO_FUN( do_practice );
DECLARE_DO_FUN( do_prompt );
DECLARE_DO_FUN( do_pull );
DECLARE_DO_FUN( do_punch );
DECLARE_DO_FUN( do_purge );
DECLARE_DO_FUN( do_push );
DECLARE_DO_FUN( do_put );
DECLARE_DO_FUN( do_qpset );
DECLARE_DO_FUN( do_quaff );
DECLARE_DO_FUN( do_quest );
DECLARE_DO_FUN( do_qui );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_rank );
DECLARE_DO_FUN( do_rat );
DECLARE_DO_FUN( do_rdelete );
DECLARE_DO_FUN( do_reboo );
DECLARE_DO_FUN( do_reboot );
DECLARE_DO_FUN( do_recho );
DECLARE_DO_FUN( do_recite );
DECLARE_DO_FUN( do_redit );
DECLARE_DO_FUN( do_regoto );
DECLARE_DO_FUN( do_remove );
DECLARE_DO_FUN( do_rent );
DECLARE_DO_FUN( do_mobrepair );
DECLARE_DO_FUN( do_repair );
DECLARE_DO_FUN( do_repairset );
DECLARE_DO_FUN( do_repairshops );
DECLARE_DO_FUN( do_repairstat );
DECLARE_DO_FUN( do_reply );
DECLARE_DO_FUN( do_report );
DECLARE_DO_FUN( do_rescue );
DECLARE_DO_FUN( do_rest );
DECLARE_DO_FUN( do_reset );
DECLARE_DO_FUN( do_resetship );
DECLARE_DO_FUN( do_restore );
DECLARE_DO_FUN( do_restorefile );
DECLARE_DO_FUN( do_restoretime );
DECLARE_DO_FUN( do_restrict );
DECLARE_DO_FUN( do_request );
DECLARE_DO_FUN( do_retire );
DECLARE_DO_FUN( do_retran );
DECLARE_DO_FUN( do_return );
DECLARE_DO_FUN( do_revert );
DECLARE_DO_FUN( do_rip );
DECLARE_DO_FUN( do_rlist );
DECLARE_DO_FUN( do_rset );
DECLARE_DO_FUN( do_rstat );
DECLARE_DO_FUN( do_sacrifice );
DECLARE_DO_FUN( do_save );
DECLARE_DO_FUN( do_savearea );
DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_scan );
DECLARE_DO_FUN( do_scanbugs );
DECLARE_DO_FUN( do_score );
DECLARE_DO_FUN( do_screenname );
DECLARE_DO_FUN( do_scribe );
DECLARE_DO_FUN( do_search );
DECLARE_DO_FUN( do_sedit );
DECLARE_DO_FUN( do_sell );
DECLARE_DO_FUN( do_sellhome );
DECLARE_DO_FUN( do_set_boot_time );
DECLARE_DO_FUN( do_setclan );
DECLARE_DO_FUN( do_setship );
DECLARE_DO_FUN( do_shops );
DECLARE_DO_FUN( do_shopset );
DECLARE_DO_FUN( do_shopstat );
DECLARE_DO_FUN( do_shout );
DECLARE_DO_FUN( do_shove );
DECLARE_DO_FUN( do_showclan );
DECLARE_DO_FUN( do_showship );
DECLARE_DO_FUN( do_showplanet );
DECLARE_DO_FUN( do_showsocial );
DECLARE_DO_FUN( do_shutdow );
DECLARE_DO_FUN( do_shutdown );
DECLARE_DO_FUN( do_silence );
DECLARE_DO_FUN( do_sit );
DECLARE_DO_FUN( do_tune );
DECLARE_DO_FUN( do_shiplock );
DECLARE_DO_FUN( do_salvage );
DECLARE_DO_FUN( do_plantbug );
DECLARE_DO_FUN( do_plantbeacon );
DECLARE_DO_FUN( do_showbugs );
DECLARE_DO_FUN( do_showbeacons );
DECLARE_DO_FUN( do_checkbeacons );
DECLARE_DO_FUN( do_nullifybeacons );
DECLARE_DO_FUN( do_juke );
DECLARE_DO_FUN( do_barrel_roll );
DECLARE_DO_FUN( do_launch2 );
DECLARE_DO_FUN( do_sabotage );
DECLARE_DO_FUN( do_freeship );
DECLARE_DO_FUN( do_hale );
DECLARE_DO_FUN( do_wwwimage );
DECLARE_DO_FUN( do_whisper );
DECLARE_DO_FUN( do_marry );
DECLARE_DO_FUN( do_propose );
DECLARE_DO_FUN( do_divorce );
DECLARE_DO_FUN( do_accept );
DECLARE_DO_FUN( do_decline );
DECLARE_DO_FUN( do_tellsnoop );
DECLARE_DO_FUN( do_makecommsystem );
DECLARE_DO_FUN( do_makedatapad );
DECLARE_DO_FUN( do_codecrack );
DECLARE_DO_FUN( do_inquire );
DECLARE_DO_FUN( do_checkprints );
DECLARE_DO_FUN( do_disableship );
DECLARE_DO_FUN( do_assignpilot );
DECLARE_DO_FUN( do_setwage );
DECLARE_DO_FUN( do_sla );
DECLARE_DO_FUN( do_slay );
DECLARE_DO_FUN( do_slicebank );
DECLARE_DO_FUN( do_sleep );
DECLARE_DO_FUN( do_slist );
DECLARE_DO_FUN( do_slog );
DECLARE_DO_FUN( do_slookup );
DECLARE_DO_FUN( do_smoke );
DECLARE_DO_FUN( do_sneak );
DECLARE_DO_FUN( do_snoop );
DECLARE_DO_FUN( do_sober );
DECLARE_DO_FUN( do_socials );
DECLARE_DO_FUN( do_south );
DECLARE_DO_FUN( do_southeast );
DECLARE_DO_FUN( do_southwest );
DECLARE_DO_FUN( do_speak );
DECLARE_DO_FUN( do_split );
DECLARE_DO_FUN( do_sset );
DECLARE_DO_FUN( do_stand );
DECLARE_DO_FUN( do_starttourney );
DECLARE_DO_FUN( do_steal );
DECLARE_DO_FUN( do_sting );
DECLARE_DO_FUN( do_stun );
DECLARE_DO_FUN( do_supplicate );
DECLARE_DO_FUN( do_switch );
DECLARE_DO_FUN( do_tail );
DECLARE_DO_FUN( do_tamp );
DECLARE_DO_FUN( do_tell );
DECLARE_DO_FUN( do_time );
DECLARE_DO_FUN( do_timecmd );
DECLARE_DO_FUN( do_title );
DECLARE_DO_FUN( do_track );
DECLARE_DO_FUN( do_transfer );
DECLARE_DO_FUN( do_transship );
DECLARE_DO_FUN( do_transshipss );
DECLARE_DO_FUN( do_trust );
DECLARE_DO_FUN( do_typo );
DECLARE_DO_FUN( do_unhell );
DECLARE_DO_FUN( do_unbind );
DECLARE_DO_FUN( do_ungag );
DECLARE_DO_FUN( do_unlock );
DECLARE_DO_FUN( do_unsilence );
DECLARE_DO_FUN( do_unoutlaw );
DECLARE_DO_FUN( do_up );
DECLARE_DO_FUN( do_users );
DECLARE_DO_FUN( do_value );
DECLARE_DO_FUN( do_viewskills );
DECLARE_DO_FUN( do_visible );
DECLARE_DO_FUN( do_vnums );
DECLARE_DO_FUN( do_vsearch );
DECLARE_DO_FUN( do_wake );
DECLARE_DO_FUN( do_wartalk );
DECLARE_DO_FUN( do_war );
DECLARE_DO_FUN( do_prototypes );
DECLARE_DO_FUN( do_setprototype );
DECLARE_DO_FUN( do_showprototype );
DECLARE_DO_FUN( do_designship );
DECLARE_DO_FUN( do_wear );
DECLARE_DO_FUN( do_weather );
DECLARE_DO_FUN( do_west );
DECLARE_DO_FUN( do_where );
DECLARE_DO_FUN( do_who );
DECLARE_DO_FUN( do_whoinvis );
DECLARE_DO_FUN( do_whois );
DECLARE_DO_FUN( do_wimpy );
DECLARE_DO_FUN( do_wizhelp );
DECLARE_DO_FUN( do_wizlist );
DECLARE_DO_FUN( do_wizlock );
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_zap );
DECLARE_DO_FUN( do_zones );

/* mob prog stuff */
DECLARE_DO_FUN( do_mp_close_passage );
DECLARE_DO_FUN( do_mp_damage );
DECLARE_DO_FUN( do_mp_restore );
DECLARE_DO_FUN( do_mp_open_passage );
DECLARE_DO_FUN( do_mp_practice );
DECLARE_DO_FUN( do_mp_slay );
DECLARE_DO_FUN( do_mpadvance );
DECLARE_DO_FUN( do_mpasound );
DECLARE_DO_FUN( do_mpat );
DECLARE_DO_FUN( do_mpdream );
DECLARE_DO_FUN( do_mp_deposit );
DECLARE_DO_FUN( do_mp_withdraw );
DECLARE_DO_FUN( do_mpecho );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat );
DECLARE_DO_FUN( do_mpedit );
DECLARE_DO_FUN( do_mrange );
DECLARE_DO_FUN( do_opedit );
DECLARE_DO_FUN( do_orange );
DECLARE_DO_FUN( do_rpconvert );
DECLARE_DO_FUN( do_rpedit );
DECLARE_DO_FUN( do_mpforce );
DECLARE_DO_FUN( do_mpinvis );
DECLARE_DO_FUN( do_mpgoto );
DECLARE_DO_FUN( do_mpjunk );
DECLARE_DO_FUN( do_mpkill );
DECLARE_DO_FUN( do_mpmload );
DECLARE_DO_FUN( do_mpmset );
DECLARE_DO_FUN( do_mpnothing );
DECLARE_DO_FUN( do_mpoload );
DECLARE_DO_FUN( do_mposet );
DECLARE_DO_FUN( do_mppurge );
DECLARE_DO_FUN( do_mpstat );
DECLARE_DO_FUN( do_opstat );
DECLARE_DO_FUN( do_rpstat );
DECLARE_DO_FUN( do_mptransfer );
DECLARE_DO_FUN( do_mpapply );
DECLARE_DO_FUN( do_mpapplyb );
DECLARE_DO_FUN( do_mppkset );
DECLARE_DO_FUN( do_mpgain );

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN( spell_null );
DECLARE_SPELL_FUN( spell_notfound );
DECLARE_SPELL_FUN( spell_acid_blast );
DECLARE_SPELL_FUN( spell_animate_dead );
DECLARE_SPELL_FUN( spell_astral_walk );
DECLARE_SPELL_FUN( spell_blindness );
DECLARE_SPELL_FUN( spell_burning_hands );
DECLARE_SPELL_FUN( spell_call_lightning );
DECLARE_SPELL_FUN( spell_cause_critical );
DECLARE_SPELL_FUN( spell_cause_light );
DECLARE_SPELL_FUN( spell_cause_serious );
DECLARE_SPELL_FUN( spell_change_sex );
DECLARE_SPELL_FUN( spell_charm_person );
DECLARE_SPELL_FUN( spell_chill_touch );
DECLARE_SPELL_FUN( spell_colour_spray );
DECLARE_SPELL_FUN( spell_control_weather );
DECLARE_SPELL_FUN( spell_create_food );
DECLARE_SPELL_FUN( spell_create_water );
DECLARE_SPELL_FUN( spell_cure_blindness );
DECLARE_SPELL_FUN( spell_cure_poison );
DECLARE_SPELL_FUN( spell_curse );
DECLARE_SPELL_FUN( spell_detect_poison );
DECLARE_SPELL_FUN( spell_dispel_evil );
DECLARE_SPELL_FUN( spell_dispel_magic );
DECLARE_SPELL_FUN( spell_dream );
DECLARE_SPELL_FUN( spell_earthquake );
DECLARE_SPELL_FUN( spell_enchant_weapon );
DECLARE_SPELL_FUN( spell_energy_drain );
DECLARE_SPELL_FUN( spell_faerie_fire );
DECLARE_SPELL_FUN( spell_faerie_fog );
DECLARE_SPELL_FUN( spell_farsight );
DECLARE_SPELL_FUN( spell_fireball );
DECLARE_SPELL_FUN( spell_flamestrike );
DECLARE_SPELL_FUN( spell_gate );
DECLARE_SPELL_FUN( spell_knock );
DECLARE_SPELL_FUN( spell_injure );
DECLARE_SPELL_FUN( spell_identify );
DECLARE_SPELL_FUN( spell_invis );
DECLARE_SPELL_FUN( spell_know_alignment );
DECLARE_SPELL_FUN( spell_lightning_bolt );
DECLARE_SPELL_FUN( spell_locate_object );
DECLARE_SPELL_FUN( spell_magic_missile );
DECLARE_SPELL_FUN( spell_mist_walk );
DECLARE_SPELL_FUN( spell_pass_door );
DECLARE_SPELL_FUN( spell_plant_pass );
DECLARE_SPELL_FUN( spell_poison );
DECLARE_SPELL_FUN( spell_polymorph );
DECLARE_SPELL_FUN( spell_possess );
DECLARE_SPELL_FUN( spell_recharge );
DECLARE_SPELL_FUN( spell_remove_curse );
DECLARE_SPELL_FUN( spell_remove_invis );
DECLARE_SPELL_FUN( spell_remove_trap );
DECLARE_SPELL_FUN( spell_shocking_grasp );
DECLARE_SPELL_FUN( spell_sleep );
DECLARE_SPELL_FUN( spell_smaug );
DECLARE_SPELL_FUN( spell_solar_flight );
DECLARE_SPELL_FUN( spell_summon );
DECLARE_SPELL_FUN( spell_teleport );
DECLARE_SPELL_FUN( spell_ventriloquate );
DECLARE_SPELL_FUN( spell_weaken );
DECLARE_SPELL_FUN( spell_word_of_recall );
DECLARE_SPELL_FUN( spell_acid_breath );
DECLARE_SPELL_FUN( spell_fire_breath );
DECLARE_SPELL_FUN( spell_frost_breath );
DECLARE_SPELL_FUN( spell_gas_breath );
DECLARE_SPELL_FUN( spell_lightning_breath );
DECLARE_SPELL_FUN( spell_spiral_blast );
DECLARE_SPELL_FUN( spell_scorching_surge );
DECLARE_SPELL_FUN( spell_helical_flow );
DECLARE_SPELL_FUN( spell_transport );
DECLARE_SPELL_FUN( spell_portal );

DECLARE_SPELL_FUN( spell_ethereal_fist );
DECLARE_SPELL_FUN( spell_spectral_furor );
DECLARE_SPELL_FUN( spell_hand_of_chaos );
DECLARE_SPELL_FUN( spell_disruption );
DECLARE_SPELL_FUN( spell_sonic_resonance );
DECLARE_SPELL_FUN( spell_mind_wrack );
DECLARE_SPELL_FUN( spell_mind_wrench );
DECLARE_SPELL_FUN( spell_revive );
DECLARE_SPELL_FUN( spell_sulfurous_spray );
DECLARE_SPELL_FUN( spell_caustic_fount );
DECLARE_SPELL_FUN( spell_acetum_primus );
DECLARE_SPELL_FUN( spell_galvanic_whip );
DECLARE_SPELL_FUN( spell_magnetic_thrust );
DECLARE_SPELL_FUN( spell_quantum_spike );
DECLARE_SPELL_FUN( spell_black_hand );
DECLARE_SPELL_FUN( spell_black_fist );
DECLARE_SPELL_FUN( spell_black_lightning );
DECLARE_SPELL_FUN( spell_calm );
DECLARE_SPELL_FUN( spell_forcepush );
DECLARE_SPELL_FUN( spell_force_disarm );
DECLARE_SPELL_FUN( spell_steal_life );
DECLARE_SPELL_FUN( spell_midas_touch );

DECLARE_SPELL_FUN( spell_suggest );
DECLARE_SPELL_FUN( spell_cure_addiction );

DECLARE_DO_FUN( fskill_convert );
DECLARE_DO_FUN( fskill_awareness );
DECLARE_DO_FUN( fskill_fdisguise );
DECLARE_DO_FUN( fskill_finfo );
DECLARE_DO_FUN( fskill_fhelp );
DECLARE_DO_FUN( fskill_finish );
DECLARE_DO_FUN( fskill_force_lightning );
DECLARE_DO_FUN( fskill_fshield );
DECLARE_DO_FUN( fskill_heal );
DECLARE_DO_FUN( fskill_identify );
DECLARE_DO_FUN( fskill_instruct );
DECLARE_DO_FUN( fskill_master );
DECLARE_DO_FUN( fskill_makelightsaber );
DECLARE_DO_FUN( fskill_promote );
DECLARE_DO_FUN( fskill_protect );
DECLARE_DO_FUN( fskill_refresh );
DECLARE_DO_FUN( fskill_squeeze );
DECLARE_DO_FUN( fskill_student );
DECLARE_DO_FUN( fskill_slash );
DECLARE_DO_FUN( fskill_whirlwind );
DECLARE_DO_FUN( fskill_makedualsaber );

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 */
#define PLAYER_DIR	"../player/"   /* Player files         */
#define BACKUP_DIR	"../backup/"   /* Backup Player files    */
#define GOD_DIR		"../gods/"  /* God Info Dir         */
#define BOARD_DIR	"../boards/"   /* Board data dir    */
#define CLAN_DIR	"../clans/" /* Clan data dir     */
#define SHIP_DIR        "../space/"
#define SPACE_DIR       "../space/"
#define SHIP_PROTOTYPE_DIR   "../ships/"
#define FORCE_DIR	"../force/"
#define FORCE_HELP_DIR	"../force/help/"
#define PLANET_DIR      "../planets/"
#define GUARD_DIR       "../planets/"
#define GUILD_DIR       "../guilds/"   /* Guild data dir               */
#define BUILD_DIR       "../building/" /* Online building save dir     */
#define SYSTEM_DIR	"../system/"   /* Main system files    */
#define PROG_DIR	"../mudprogs/" /* MUDProg files     */
#define CORPSE_DIR	"../corpses/"  /* Corpses        */
#define PROFILE_DIR	"../html/profiles/"  /* Player Profiles */
#define AREA_LIST	"area.lst"  /* List of areas     */
#define BAN_LIST        "ban.lst"   /* List of bans                 */
#define CLAN_LIST	"clan.lst"  /* List of clans     */
#define SHIP_LIST       "ship.lst"
#define PROTOTYPE_LIST  "prototype.lst"
#define PLANET_LIST      "planet.lst"
#define SPACE_LIST      "space.lst"
#define BOUNTY_LIST     "bounty.lst"
#define disintegration_LIST	"disintegration.lst"
#define SENATE_LIST	"senate.lst"   /* List of senators     */
#define GUILD_LIST      "guild.lst" /* List of guilds               */
#define GUARD_LIST	"guard.lst"
#define BOARD_FILE	"boards.txt"   /* For bulletin boards   */
#define SHUTDOWN_FILE	"shutdown.txt" /* For 'shutdown'  */
#define RIPSCREEN_FILE	SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE	SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE	SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE	SYSTEM_DIR "mudtitle.asc"
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"   /* Boot up error file  */
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"  /* For 'idea'       */
#define CHANGE_FILE	SYSTEM_DIR "changes.txt"   /* Changes file - txt  */
#define CHANGEHTML_FILE	"../html/changes.html"  /* Changes file - html */
#define DEBUG_FILE	SYSTEM_DIR "debug.txt"  /* Catch-all for debug */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"  /* For 'typo'       */
#define BUG_FILE SYSTEM_DIR "pbugs.txt"   /* For player bug command */
#define LOG_FILE	SYSTEM_DIR "log.txt" /* For talking in logged rooms */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST" /* Wizlist       */
#define WEBWIZLIST_FILE	"../html/WEBWIZLIST" /* Web Wizlist    */
#define WHO_FILE	SYSTEM_DIR "../html/WHO"   /* Who output file  */
#define WEBWHO_FILE	"../html/WEBWHO"  /* Web Who File   */
#define REQUEST_PIPE	SYSTEM_DIR "REQUESTS"   /* Request FIFO  */
#define SKILL_FILE	SYSTEM_DIR "skills.dat" /* Skill table   */
#define HERB_FILE	SYSTEM_DIR "herbs.dat"  /* Herb table       */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"   /* Socials       */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat"  /* Commands      */
#define NAMEBAN_FILE	SYSTEM_DIR "nameban.dat"   /* Nameban       */
#define USAGE_FILE	SYSTEM_DIR "usage.txt"  /* How many people are on 
                                              * every half hour - trying to
                                              * determine best reboot time */
#define TEMP_FILE	SYSTEM_DIR "charsave.tmp"  /* More char save protect */
#define SLOG_FILE	"../.slog/slog.txt"  /* Secret Log       */

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define BD	BOARD_DATA
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define DE	DEITY_DATA
#define SK	SKILLTYPE
#define SH      SHIP_DATA

/* editor.c cronel new editor */
#define start_editing( ch, data ) \
	start_editing_nolimit( ch, data, MAX_STRING_LENGTH )
void start_editing_nolimit( CHAR_DATA * ch, char *data, short max_size );
void stop_editing( CHAR_DATA * ch );
void edit_buffer( CHAR_DATA * ch, char *argument );
char *copy_buffer( CHAR_DATA * ch );
void set_editor_desc( CHAR_DATA * ch, char *desc );
void editor_desc_printf( CHAR_DATA * ch, const char *desc_fmt, ... );

/* pfiles.c */
void remove_member( char *name, char *shortname );
void add_member( char *name, char *shortname );

/* act_comm.c */
bool check_parse_name( const char *name );
void sound_to_room( ROOM_INDEX_DATA * room, const char *argument );
bool circle_follow( CHAR_DATA * ch, CHAR_DATA * victim );
void add_follower( CHAR_DATA * ch, CHAR_DATA * master );
void stop_follower( CHAR_DATA * ch );
void die_follower( CHAR_DATA * ch );
bool is_same_group( CHAR_DATA * ach, CHAR_DATA * bch );
void send_rip_screen( CHAR_DATA * ch );
void send_rip_title( CHAR_DATA * ch );
void send_ansi_title( CHAR_DATA * ch );
void send_ascii_title( CHAR_DATA * ch );
void to_channel( const char *argument, int channel, const char *verb, short level );
void talk_auction( char *argument );
bool knows_language( CHAR_DATA * ch, int language, CHAR_DATA * cch );
bool can_learn_lang( CHAR_DATA * ch, int language );
int countlangs( int languages );
char *obj_short( OBJ_DATA * obj );

/* act_info.c */
int get_door( const char *arg );
char *format_obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort );
void show_list_to_char( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing );

/* act_move.c */
void clear_vrooms( void );
ED *find_door( CHAR_DATA * ch, const char *arg, bool quiet );
ED *get_exit( ROOM_INDEX_DATA * room, short dir );
ED *get_exit_to( ROOM_INDEX_DATA * room, short dir, int vnum );
ED *get_exit_num( ROOM_INDEX_DATA * room, short count );
ch_ret move_char( CHAR_DATA * ch, EXIT_DATA * pexit, int fall );
void teleport( CHAR_DATA * ch, int room, int flags );
short encumbrance( CHAR_DATA * ch, short move );
bool will_fall( CHAR_DATA * ch, int fall );
int wherehome( CHAR_DATA * ch );

/* act_obj.c */
obj_ret damage_obj( OBJ_DATA * obj );
short get_obj_resistance( OBJ_DATA * obj );
bool remove_obj( CHAR_DATA * ch, int iWear, bool fReplace );
void save_clan_storeroom( CHAR_DATA * ch, CLAN_DATA * clan );
void obj_fall( OBJ_DATA * obj, bool through );

/* act_wiz.c */
void close_area( AREA_DATA * pArea );
AREA_DATA *get_area( const char *argument );
RID *find_location( CHAR_DATA * ch, char *arg );
void echo_to_room( short AT_COLOR, ROOM_INDEX_DATA * room, const char *argument );
void echo_to_all( short AT_COLOR, const char *argument, short tar );
void get_reboot_string( void );
struct tm *update_time( struct tm * old_time );
void free_social( SOCIALTYPE * social );
void add_social( SOCIALTYPE * social );
void free_command( CMDTYPE * command );
void unlink_command( CMDTYPE * command );
void add_command( CMDTYPE * command );

/* boards.c */
void load_boards( void );
BD *get_board( OBJ_DATA * obj );
void free_note( NOTE_DATA * pnote );

/* build.c */
const char *flag_string( int bitvector, const char *const flagarray[] );
char *strip_cr( const char *str );
int get_vip_flag( const char *flag );
int get_wanted_flag( const char *flag );
int get_flag( const char *, const char *const flagarray[], int );
int get_otype( const char *type );
int get_atype( const char *type );
int get_aflag( const char *flag );
int get_oflag( const char *flag );
int get_wflag( const char *flag );
int get_risflag( const char *flag );
int get_attackflag( const char *flag );
int get_defenseflag( const char *flag );
int get_langnum( const char *flag );
int get_langnum_save( const char *flag );
int get_langflag( const char *flag );
int get_exflag( const char *flag );
int get_rflag( const char *flag );
int get_rflag2( const char *flag );
int get_secflag( const char *flag );
int get_actflag( const char *flag );
int get_cmdflag( const char *flag );
int get_npc_race( const char *type );
int get_pc_race( const char *type );
int get_npc_sex( const char *sex );
int get_areaflag( const char *flag );
int get_mpflag( const char *flag );
int get_trigflag( const char *flag );
int get_dir( const char *txt );
int get_partflag( const char *flag );
int get_npc_position( const char *position );

/* clans.c */
CL *get_clan( const char *name );
void load_clans( void );
void save_clan( CLAN_DATA * clan );
void load_senate( void );
void save_senate( void );
PLANET_DATA *get_planet( const char *name );
void load_planets( void );
void save_planet( PLANET_DATA * planet );
long get_taxes( PLANET_DATA * planet );

/* bounty.c */
BOUNTY_DATA *get_disintegration( const char *target );
void load_bounties( void );
void save_bounties( void );
void save_disintegrations( void );
void remove_disintegration( BOUNTY_DATA * bounty );
void claim_disintegration( CHAR_DATA * ch, CHAR_DATA * victim );
bool is_disintegration( CHAR_DATA * victim );

/* force.c */
bool check_reflect( CHAR_DATA * ch, CHAR_DATA * victim, int dam );
void write_all_forceskills( void );
void save_forceskill( FORCE_SKILL * fskill );
void write_forceskill_list( void );
bool load_forceskill( const char *forceskillfile );
void fread_forceskill( FORCE_SKILL * fskill, FILE * fp );
void write_all_forcehelps( void );
void save_forcehelp( FORCE_HELP * fhelp );
void write_forcehelp_list( void );
bool load_forcehelp( const char *forcehelpfile );
void fread_forcehelp( FORCE_HELP * fhelp, FILE * fp );
int check_force_skill( CHAR_DATA * ch, const char *command, const char *argument );
void load_force_skills( void );
void load_force_help( void );
DO_FUN *get_force_skill_function( const char *name );
FORCE_SKILL *get_force_skill( const char *argument );
FORCE_HELP *get_force_help( const char *fname, const char *type );
void force_send_to_room( CHAR_DATA * ch, CHAR_DATA * victim, const char *msg );
CHAR_DATA *force_get_victim( CHAR_DATA * ch, const char *argument, int loc );
const char *force_get_possessive( CHAR_DATA * ch );
const char *force_get_objective( CHAR_DATA * ch );
const char *force_get_pronoun( CHAR_DATA * ch );
char *force_parse_string( CHAR_DATA * ch, CHAR_DATA * victim, const char *msg );
void force_learn_from_failure( CHAR_DATA * ch, FORCE_SKILL * fskill );
void force_learn_from_success( CHAR_DATA * ch, FORCE_SKILL * fskill );
FORCE_SKILL *force_test_skill_use( const char *skill_name, CHAR_DATA * ch, int skill_type );
const char *force_get_level( CHAR_DATA * ch );
int force_promote_ready( CHAR_DATA * ch );
void draw_force_line( CHAR_DATA * ch, int length );
void draw_force_line_rev( CHAR_DATA * ch, int length );
void update_force( void );

/* space.c */
SH *get_ship( const char *name );
void load_ships( void );
void placeships( void );
void save_ship( SHIP_DATA * ship );
void load_space( void );
void save_starsystem( SPACE_DATA * starsystem );
SPACE_DATA *starsystem_from_name( const char *name );
SPACE_DATA *starsystem_from_room( ROOM_INDEX_DATA * room );
SHIP_DATA *ship_from_entrance( int vnum );
SHIP_DATA *ship_from_room( int vnum );
SHIP_DATA *ship_from_hanger( int vnum );
SHIP_DATA *ship_from_pilotseat( int vnum );
SHIP_DATA *ship_from_cockpit( int vnum );
SHIP_DATA *ship_from_turret( int vnum );
SHIP_DATA *ship_from_engine( int vnum );
SHIP_DATA *ship_from_pilot( const char *name );
SHIP_DATA *get_ship_here( const char *name, SPACE_DATA * starsystem );
void showstarsystem( CHAR_DATA * ch, SPACE_DATA * starsystem );
void update_space( void );
void recharge_ships( void );
void move_ships( void );
void update_bus( void );
void update_traffic( void );
bool check_pilot( CHAR_DATA * ch, SHIP_DATA * ship );
bool is_rental( CHAR_DATA * ch, SHIP_DATA * ship );
void echo_to_ship( int color, SHIP_DATA * ship, const char *argument );
void echo_to_cockpit( int color, SHIP_DATA * ship, const char *argument );
void echo_to_system( int color, SHIP_DATA * ship, const char *argument, SHIP_DATA * ignore );
bool extract_ship( SHIP_DATA * ship );
bool ship_to_room( SHIP_DATA * ship, int vnum );
bool ship_to_room2( SHIP_DATA * ship, ROOM_INDEX_DATA * shipto );
long get_ship_value( SHIP_DATA * ship );
bool rent_ship( CHAR_DATA * ch, SHIP_DATA * ship );
void damage_ship( SHIP_DATA * ship, int min, int max );
void damage_ship_ch( SHIP_DATA * ship, int min, int max, CHAR_DATA * ch );
void destroy_ship( SHIP_DATA * ship, CHAR_DATA * ch, const char *reason );
void ship_to_starsystem( SHIP_DATA * ship, SPACE_DATA * starsystem );
void ship_from_starsystem( SHIP_DATA * ship, SPACE_DATA * starsystem );
void new_missile( SHIP_DATA * ship, SHIP_DATA * target, CHAR_DATA * ch, int missiletype );
void extract_missile( MISSILE_DATA * missile );
SHIP_DATA *ship_in_room( ROOM_INDEX_DATA * room, const char *name );

/* morespace.c */
SHIP_PROTOTYPE *get_ship_prototype( char *name );
void load_prototypes( void );
void save_ship_protoype( SHIP_PROTOTYPE * prototype );
long int get_prototype_value( SHIP_PROTOTYPE * prototype );
void create_ship_rooms( SHIP_DATA * ship );

/* comm.c */
const char *PERS( CHAR_DATA * ch, CHAR_DATA * looker );
FELLOW_DATA *knowsof( CHAR_DATA * ch, CHAR_DATA * victim );
void close_socket( DESCRIPTOR_DATA * dclose, bool force );
bool write_to_descriptor( DESCRIPTOR_DATA * d, const char *txt, int length );
void write_to_buffer( DESCRIPTOR_DATA * d, const char *txt, size_t length );
void write_to_pager( DESCRIPTOR_DATA * d, const char *txt, size_t length );
void send_to_char( const char *txt, CHAR_DATA * ch );
void send_to_char_color( const char *txt, CHAR_DATA * ch );
void send_to_char_noand( const char *txt, CHAR_DATA * ch );
void send_to_pager( const char *txt, CHAR_DATA * ch );
void send_to_pager_color( const char *txt, CHAR_DATA * ch );
void ch_printf( CHAR_DATA * ch, const char *fmt, ... ) __attribute__ ( ( format( printf, 2, 3 ) ) );
char *chrmax( char *src, int length );
int strlen_color( char *argument );
char *format_str( char *str, int len );
void pager_printf( CHAR_DATA * ch, const char *fmt, ... ) __attribute__ ( ( format( printf, 2, 3 ) ) );
void log_string_plus( const char *str, short log_type, short level );
void log_printf_plus( short log_type, short level, const char *fmt, ... ) __attribute__ ( ( format( printf, 3, 4 ) ) );
void log_printf( const char *fmt, ... ) __attribute__ ( ( format( printf, 1, 2 ) ) );
void descriptor_printf( DESCRIPTOR_DATA * d, const char *fmt, ... ) __attribute__ ( ( format( printf, 2, 3 ) ) );
void buffer_printf( DESCRIPTOR_DATA * d, const char *fmt, ... ) __attribute__ ( ( format( printf, 2, 3 ) ) );
void act( short AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type );

/* reset.c */
RD *make_reset( char letter, int extra, int arg1, int arg2, int arg3 );
RD *add_reset( ROOM_INDEX_DATA * room, char letter, int extra, int arg1, int arg2, int arg3 );
void reset_area( AREA_DATA * pArea );

/* db.c */
char *fread_flagstring( FILE * fp );
void show_file( CHAR_DATA * ch, const char *filename );
bool is_valid_filename( CHAR_DATA * ch, const char *direct, const char *filename );
const char *centertext( const char *text, size_t size );
void boot_db( bool fCopyOver );
void area_update( void );
void add_char( CHAR_DATA * ch );
CD *create_mobile( MOB_INDEX_DATA * pMobIndex );
OD *create_object( OBJ_INDEX_DATA * pObjIndex, int level );
void clear_char( CHAR_DATA * ch );
void free_char( CHAR_DATA * ch );
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA * ed );
MID *get_mob_index( int vnum );
OID *get_obj_index( int vnum );
RID *get_room_index( int vnum );
char fread_letter( FILE * fp );
int fread_number( FILE * fp );
float fread_float( FILE * fp );
char *fread_string( FILE * fp );
char *fread_string_nohash( FILE * fp );
void fread_to_eol( FILE * fp );
char *fread_word( FILE * fp );
char *fread_line( FILE * fp );
int number_fuzzy( int number );
int number_range( int from, int to );
int number_percent( void );
int number_door( void );
int number_bits( int width );
int number_mm( void );
int dice( int number, int size );
int interpolate( int level, int value_00, int value_32 );
void smash_tilde( char *str );
const char *smash_tilde( const char *str );
char *smash_tilde_copy( const char *str );
void hide_tilde( char *str );
char *show_tilde( const char *str );
bool str_cmp( const char *astr, const char *bstr );
bool str_prefix( const char *astr, const char *bstr );
bool str_infix( const char *astr, const char *bstr );
bool str_suffix( const char *astr, const char *bstr );
char *capitalize( const char *str );
char *strlower( const char *str );
char *strupper( const char *str );
const char *aoran( const char *str );
void append_file( CHAR_DATA * ch, const char *file, const char *str );
void append_to_file( const char *file, const char *str );
void prepend_to_file( const char *file, const char *str );
void bug( const char *str, ... );
RID *make_room( int vnum, AREA_DATA * area );
RID *make_ship_room( SHIP_DATA * ship, int vnum );
OID *make_object( int vnum, int cvnum, const char *name );
MID *make_mobile( int vnum, int cvnum, const char *name );
ED *make_exit( ROOM_INDEX_DATA * pRoomIndex, ROOM_INDEX_DATA * to_room, short door );
void add_help( HELP_DATA * pHelp );
void fix_area_exits( AREA_DATA * tarea );
void load_area_file( AREA_DATA * tarea, const char *filename );
void randomize_exits( ROOM_INDEX_DATA * room, short maxdir );
void make_wizlist( void );
void tail_chain( void );
void delete_room( ROOM_INDEX_DATA * room );
void delete_obj( OBJ_INDEX_DATA * obj );
void delete_mob( MOB_INDEX_DATA * mob );
void sort_area( AREA_DATA * pArea, bool proto );
void sort_area_by_name( AREA_DATA * pArea ); /* Fireblade */

/* build.c */
bool can_rmodify( CHAR_DATA * ch, ROOM_INDEX_DATA * room );
bool can_omodify( CHAR_DATA * ch, OBJ_DATA * obj );
bool can_mmodify( CHAR_DATA * ch, CHAR_DATA * mob );
bool can_medit( CHAR_DATA * ch, MOB_INDEX_DATA * mob );
void free_reset( AREA_DATA * are, RESET_DATA * res );
void free_area( AREA_DATA * are );
void assign_area( CHAR_DATA * ch );
EDD *SetRExtra( ROOM_INDEX_DATA * room, const char *keywords );
bool DelRExtra( ROOM_INDEX_DATA * room, const char *keywords );
EDD *SetOExtra( OBJ_DATA * obj, const char *keywords );
bool DelOExtra( OBJ_DATA * obj, const char *keywords );
EDD *SetOExtraProto( OBJ_INDEX_DATA * obj, const char *keywords );
bool DelOExtraProto( OBJ_INDEX_DATA * obj, const char *keywords );
void fold_area( AREA_DATA * tarea, const char *filename, bool install );
void RelCreate( relation_type, void *, void * );
void RelDestroy( relation_type, void *, void * );

/* fight.c */
int max_fight( CHAR_DATA * ch );
void violence_update( void );
ch_ret multi_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt );
short ris_damage( CHAR_DATA * ch, short dam, int ris );
ch_ret damage( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt );
void update_pos( CHAR_DATA * victim );
void set_fighting( CHAR_DATA * ch, CHAR_DATA * victim );
void stop_fighting( CHAR_DATA * ch, bool fBoth );
void free_fight( CHAR_DATA * ch );
CD *who_fighting( CHAR_DATA * ch );
void check_killer( CHAR_DATA * ch, CHAR_DATA * victim );
void check_attacker( CHAR_DATA * ch, CHAR_DATA * victim );
void death_cry( CHAR_DATA * ch );
void stop_hunting( CHAR_DATA * ch );
void stop_hating( CHAR_DATA * ch );
void stop_fearing( CHAR_DATA * ch );
void start_hunting( CHAR_DATA * ch, CHAR_DATA * victim );
void start_hating( CHAR_DATA * ch, CHAR_DATA * victim );
void start_fearing( CHAR_DATA * ch, CHAR_DATA * victim );
bool is_hunting( CHAR_DATA * ch, CHAR_DATA * victim );
bool is_hating( CHAR_DATA * ch, CHAR_DATA * victim );
bool is_fearing( CHAR_DATA * ch, CHAR_DATA * victim );
bool is_safe( CHAR_DATA * ch, CHAR_DATA * victim );
bool is_safe_nm( CHAR_DATA * ch, CHAR_DATA * victim );
bool legal_loot( CHAR_DATA * ch, CHAR_DATA * victim );
bool check_illegal_pk( CHAR_DATA * ch, CHAR_DATA * victim );
OBJ_DATA *raw_kill( CHAR_DATA * ch, CHAR_DATA * victim );
bool in_arena( CHAR_DATA * ch );

/* makeobjs.c */
OBJ_DATA *make_corpse( CHAR_DATA * ch, char *killer );
void make_blood( CHAR_DATA * ch );
void make_bloodstain( CHAR_DATA * ch );
void make_scraps( OBJ_DATA * obj );
void make_fire( ROOM_INDEX_DATA * in_room, short timer );
OD *make_trap( int v0, int v1, int v2, int v3 );
OD *create_money( int amount );

/* misc.c */
void actiondesc( CHAR_DATA * ch, OBJ_DATA * obj, void *vo );
void jedi_checks( CHAR_DATA * ch );
void jedi_bonus( CHAR_DATA * ch );
void sith_penalty( CHAR_DATA * ch );

/* mud_comm.c */
const char *mprog_type_to_name( int type );

/* mud_prog.c */
void mprog_wordlist_check( const char *arg, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void *vo, int type );
void mprog_percent_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void *vo, int type );
void mprog_act_trigger( char *buf, CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj, void *vo );
void mprog_bribe_trigger( CHAR_DATA * mob, CHAR_DATA * ch, int amount );
void mprog_entry_trigger( CHAR_DATA * mob );
void mprog_give_trigger( CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj );
void mprog_greet_trigger( CHAR_DATA * mob );
void mprog_fight_trigger( CHAR_DATA * mob, CHAR_DATA * ch );
void mprog_hitprcnt_trigger( CHAR_DATA * mob, CHAR_DATA * ch );
void mprog_death_trigger( CHAR_DATA * killer, CHAR_DATA * mob );
void mprog_random_trigger( CHAR_DATA * mob );
void mprog_speech_trigger( const char *txt, CHAR_DATA * mob );
void mprog_script_trigger( CHAR_DATA * mob );
void mprog_hour_trigger( CHAR_DATA * mob );
void mprog_time_trigger( CHAR_DATA * mob );
void progbug( const char *str, CHAR_DATA * mob );
void rset_supermob( ROOM_INDEX_DATA * room );
void release_supermob( void );
void mpsleep_update( void );

/* player.c */
void set_title( CHAR_DATA * ch, const char *title );

/* skills.c */
bool check_skill( CHAR_DATA * ch, const char *command, const char *argument );
void learn_from_success( CHAR_DATA * ch, int sn );
void learn_from_failure( CHAR_DATA * ch, int sn );
bool check_parry( CHAR_DATA * ch, CHAR_DATA * victim );
bool check_dodge( CHAR_DATA * ch, CHAR_DATA * victim );
bool check_grip( CHAR_DATA * ch, CHAR_DATA * victim );
void disarm( CHAR_DATA * ch, CHAR_DATA * victim );
void trip( CHAR_DATA * ch, CHAR_DATA * victim );


/* handler.c */
void free_obj( OBJ_DATA * obj );
void explode( OBJ_DATA * obj );
int get_exp( CHAR_DATA * ch, int ability );
int get_exp_worth( CHAR_DATA * ch );
int exp_level( short level );
short get_trust( CHAR_DATA * ch );
short get_age( CHAR_DATA * ch );
short get_curr_str( CHAR_DATA * ch );
short get_curr_int( CHAR_DATA * ch );
short get_curr_wis( CHAR_DATA * ch );
short get_curr_dex( CHAR_DATA * ch );
short get_curr_con( CHAR_DATA * ch );
short get_curr_cha( CHAR_DATA * ch );
short get_curr_lck( CHAR_DATA * ch );
short get_curr_frc( CHAR_DATA * ch );
bool can_take_proto( CHAR_DATA * ch );
int can_carry_n( CHAR_DATA * ch );
int can_carry_w( CHAR_DATA * ch );
bool is_name( const char *str, const char *namelist );
bool is_name_prefix( const char *str, const char *namelist );
bool nifty_is_name( const char *str, const char *namelist );
bool nifty_is_name_prefix( const char *str, const char *namelist );
void affect_modify( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd );
void affect_to_char( CHAR_DATA * ch, AFFECT_DATA * paf );
void affect_remove( CHAR_DATA * ch, AFFECT_DATA * paf );
void affect_strip( CHAR_DATA * ch, int sn );
bool is_affected( CHAR_DATA * ch, int sn );
void affect_join( CHAR_DATA * ch, AFFECT_DATA * paf );
void char_from_room( CHAR_DATA * ch );
void char_to_room( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex );
OD *obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch );
void obj_from_char( OBJ_DATA * obj );
int apply_ac( OBJ_DATA * obj, int iWear );
OD *get_eq_char( CHAR_DATA * ch, int iWear );
void equip_char( CHAR_DATA * ch, OBJ_DATA * obj, int iWear );
void unequip_char( CHAR_DATA * ch, OBJ_DATA * obj );
int count_obj_list( OBJ_INDEX_DATA * pObjIndex, OBJ_DATA * list );
int count_mob_in_room( MOB_INDEX_DATA * mob, ROOM_INDEX_DATA * list );
void obj_from_room( OBJ_DATA * obj );
OD *obj_to_room( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex );
OD *obj_to_obj( OBJ_DATA * obj, OBJ_DATA * obj_to );
void obj_from_obj( OBJ_DATA * obj );
void extract_obj( OBJ_DATA * obj );
void extract_exit( ROOM_INDEX_DATA * room, EXIT_DATA * pexit );
void extract_room( ROOM_INDEX_DATA * room );
void clean_room( ROOM_INDEX_DATA * room );
void clean_obj( OBJ_INDEX_DATA * obj );
void clean_mob( MOB_INDEX_DATA * mob );
void clean_resets( ROOM_INDEX_DATA * room );
void extract_char( CHAR_DATA * ch, bool fPull );
CD *get_char_room( CHAR_DATA * ch, const char *argument );
CD *get_char_world( CHAR_DATA * ch, const char *argument );
CD *get_char_world_ooc( CHAR_DATA * ch, const char *argument );
CD *get_char_from_comfreq( CHAR_DATA * ch, const char *argument );
OD *get_obj_type( OBJ_INDEX_DATA * pObjIndexData );
OD *get_obj_list( CHAR_DATA * ch, const char *argument, OBJ_DATA * list );
OD *get_obj_list_rev( CHAR_DATA * ch, const char *argument, OBJ_DATA * list );
OD *get_obj_carry( CHAR_DATA * ch, const char *argument );
OD *get_obj_wear( CHAR_DATA * ch, const char *argument );
OD *get_obj_here( CHAR_DATA * ch, const char *argument );
OD *get_obj_world( CHAR_DATA * ch, const char *argument );
int get_obj_number( OBJ_DATA * obj );
int get_obj_weight( OBJ_DATA * obj );
bool room_is_dark( ROOM_INDEX_DATA * pRoomIndex );
bool room_is_private( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex );
bool can_see( CHAR_DATA * ch, CHAR_DATA * victim );
bool can_see_obj( CHAR_DATA * ch, OBJ_DATA * obj );
bool can_drop_obj( CHAR_DATA * ch, OBJ_DATA * obj );
const char *item_type_name( OBJ_DATA * obj );
const char *affect_loc_name( int location );
const char *affect_bit_name( int vector );
const char *extra_bit_name( int extra_flags );
const char *magic_bit_name( int magic_flags );
ch_ret check_for_trap( CHAR_DATA * ch, OBJ_DATA * obj, int flag );
ch_ret check_room_for_traps( CHAR_DATA * ch, int flag );
bool is_trapped( OBJ_DATA * obj );
OD *get_trap( OBJ_DATA * obj );
ch_ret spring_trap( CHAR_DATA * ch, OBJ_DATA * obj );
void name_stamp_stats( CHAR_DATA * ch );
void fix_char( CHAR_DATA * ch );
void showaffect( CHAR_DATA * ch, AFFECT_DATA * paf );
void set_cur_obj( OBJ_DATA * obj );
bool obj_extracted( OBJ_DATA * obj );
void queue_extracted_obj( OBJ_DATA * obj );
void clean_obj_queue( void );
void set_cur_char( CHAR_DATA * ch );
bool char_died( CHAR_DATA * ch );
void queue_extracted_char( CHAR_DATA * ch, bool extract );
void clean_char_queue( void );
void add_timer( CHAR_DATA * ch, short type, short count, DO_FUN * fun, int value );
TIMER *get_timerptr( CHAR_DATA * ch, short type );
short get_timer( CHAR_DATA * ch, short type );
void extract_timer( CHAR_DATA * ch, TIMER * timer );
void remove_timer( CHAR_DATA * ch, short type );
bool in_soft_range( CHAR_DATA * ch, AREA_DATA * tarea );
bool in_hard_range( CHAR_DATA * ch, AREA_DATA * tarea );
bool chance( CHAR_DATA * ch, short percent );
OD *clone_object( OBJ_DATA * obj );
void split_obj( OBJ_DATA * obj, int num );
void separate_obj( OBJ_DATA * obj );
bool empty_obj( OBJ_DATA * obj, OBJ_DATA * destobj, ROOM_INDEX_DATA * destroom );
OD *find_obj( CHAR_DATA * ch, const char *argument, bool carryonly );
bool ms_find_obj( CHAR_DATA * ch );
void worsen_mental_state( CHAR_DATA * ch, int mod );
void better_mental_state( CHAR_DATA * ch, int mod );
void boost_economy( AREA_DATA * tarea, int gold );
void lower_economy( AREA_DATA * tarea, int gold );
void economize_mobgold( CHAR_DATA * mob );
bool economy_has( AREA_DATA * tarea, int gold );
void add_kill( CHAR_DATA * ch, CHAR_DATA * mob );
int times_killed( CHAR_DATA * ch, CHAR_DATA * mob );
void check_switches( bool possess );
void check_switch( CHAR_DATA * ch, bool possess );

/* interp.c */
bool check_pos( CHAR_DATA * ch, short position );
void interpret( CHAR_DATA * ch, const char *argument );
bool is_number( const char *arg );
int number_argument( const char *argument, char *arg );
char *one_argument( char *argument, char *arg_first );
const char *one_argument( const char *argument, char *arg_first );
const char *one_argument2( const char *argument, char *arg_first );
ST *find_social( const char *command, bool exact );
CMDTYPE *find_command( char *command, bool exact );
void hash_commands( void );
void start_timer( struct timeval * sttime );
time_t end_timer( struct timeval * sttime );
void send_timer( struct timerset * vtime, CHAR_DATA * ch );
void update_userec( struct timeval * time_used, struct timerset * userec );

/* magic.c */
bool process_spell_components( CHAR_DATA * ch, int sn );
int ch_slookup( CHAR_DATA * ch, const char *name );
int find_spell( CHAR_DATA * ch, const char *name, bool know );
int find_skill( CHAR_DATA * ch, const char *name, bool know );
int find_weapon( CHAR_DATA * ch, const char *name, bool know );
int find_tongue( CHAR_DATA * ch, const char *name, bool know );
int skill_lookup( const char *name );
int herb_lookup( const char *name );
int personal_lookup( CHAR_DATA * ch, const char *name );
int slot_lookup( int slot );
int bsearch_skill( const char *name, int first, int top );
int bsearch_skill_exact( const char *name, int first, int top );
bool saves_poison_death( int level, CHAR_DATA * victim );
bool saves_wand( int level, CHAR_DATA * victim );
bool saves_para_petri( int level, CHAR_DATA * victim );
bool saves_breath( int level, CHAR_DATA * victim );
bool saves_spell_staff( int level, CHAR_DATA * victim );
ch_ret obj_cast_spell( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj );
int dice_parse( CHAR_DATA * ch, int level, char *xexp );
SK *get_skilltype( int sn );

/* request.c */
void init_request_pipe( void );
void check_requests( void );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
void write_corpses( CHAR_DATA * ch, char *name );
void save_char_obj( CHAR_DATA * ch );
void save_clone( CHAR_DATA * ch );
void save_profile( CHAR_DATA * ch );
bool load_char_obj( DESCRIPTOR_DATA * d, char *name, bool preload, bool hotboot );
void set_alarm( long seconds );
void requip_char( CHAR_DATA * ch );
void fwrite_obj( CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest, short os_type, bool hotboot );
void fread_obj( CHAR_DATA * ch, FILE * fp, short os_type );
void de_equip_char( CHAR_DATA * ch );
void re_equip_char( CHAR_DATA * ch );
void save_home( CHAR_DATA * ch );

/* shops.c */

/* special.c */
SF *spec_lookup( const char *name );

/* tables.c */
int get_skill( const char *skilltype );
char *spell_name( SPELL_FUN * spell );
char *skill_name( DO_FUN * skill );
void load_skill_table( void );
void save_skill_table( int delnum );
void sort_skill_table( void );
void load_socials( void );
void save_socials( void );
void load_commands( void );
void save_commands( void );
SPELL_FUN *spell_function( const char *name );
DO_FUN *skill_function( const char *name );
void load_herb_table( void );
void save_herb_table( void );

/* track.c */
void found_prey( CHAR_DATA * ch, CHAR_DATA * victim );
void hunt_victim( CHAR_DATA * ch );

/* update.c */
void advance_level( CHAR_DATA * ch, int ability );
void gain_exp( CHAR_DATA * ch, int gain, int ability );
void gain_exp2( CHAR_DATA * ch, int gain, int ability );
void gain_condition( CHAR_DATA * ch, int iCond, int value );
void update_handler( void );
void reboot_check( time_t reset );
void auction_update( void );
void remove_portal( OBJ_DATA * portal );
int max_level( CHAR_DATA * ch, int ability );

/* hashstr.c */
char *str_alloc( const char *str );
char *quick_link( const char *str );
int str_free( const char *str );
void show_hash( int count );
char *hash_stats( void );
char *check_hash( const char *str );
void hash_dump( int hash );
void show_high_hash( int top );
bool in_hash_table( const char *str );

/* ships.c */
void load_ship_prototypes( void );
int load_prototype( const char *prototypefile, int prototype );
bool load_prototype_rooms( FILE * fp, int prototype );
bool fread_prototype_room( FILE * fp, int prototype );
bool load_prototype_header( FILE * fp, int prototype );
void write_all_prototypes( void );
void write_prototype_list( void );
void save_prototype( int prototype );
int find_vnum_block( int num_needed );
int make_prototype_rooms( int ship_type, int vnum, AREA_DATA * tarea, char *Sname );
int get_sp_rflag( char *flag );
SHIP_DATA *make_prototype_ship( int ship_type, int vnum, CHAR_DATA * ch, char *ship_name );
void write_ship_list( void );
void resetship( SHIP_DATA * ship );
char *parse_prog_string( char *inp, int ship_type, int vnum );
void make_rprogs( int ship_type, int vnum );

/* functions.c */
char *strrep( const char *src, const char *sch, const char *rep );
char *strlinwrp( const char *src, int length );
char *remand( const char *arg );
char *rembg( const char *arg );
const char *htmlcolor( const char *arg );

/* newscore.c */
const char *get_race( CHAR_DATA * ch );

#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/* ships.c */
void load_market_list( void );
void save_market_list( void );
void add_market_ship( SHIP_DATA * ship );
void remove_market_ship( BMARKET_DATA * marketship );
void make_random_marketlist( void );

/*
 * defines for use with this get_affect function
 */

#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30

/*
 * mudprograms stuff
 */
extern CHAR_DATA *supermob;
extern OBJ_DATA *supermob_obj;

void oprog_speech_trigger( const char *txt, CHAR_DATA * ch );
void oprog_random_trigger( OBJ_DATA * obj );
void oprog_wear_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
bool oprog_use_trigger( CHAR_DATA * ch, OBJ_DATA * obj, CHAR_DATA * vict, OBJ_DATA * targ, void *vo );
void oprog_remove_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_sac_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_damage_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_repair_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_drop_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
//void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
char *oprog_type_to_name( int type );
int rprog_custom_trigger( const char *command, const char *argument, CHAR_DATA * ch );
int mprog_custom_trigger( const char *command, const char *argument, CHAR_DATA * ch );
int oprog_custom_trigger( const char *command, const char *argument, CHAR_DATA * ch );
void oprog_greet_trigger( CHAR_DATA * ch );
void oprog_get_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_examine_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_pull_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_push_trigger( CHAR_DATA * ch, OBJ_DATA * obj );

/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           BV00
#define SPEECH_PROG        BV01
#define RAND_PROG          BV02
#define FIGHT_PROG         BV03
#define RFIGHT_PROG        BV03
#define DEATH_PROG         BV04
#define RDEATH_PROG        BV04
#define HITPRCNT_PROG      BV05
#define ENTRY_PROG         BV06
#define ENTER_PROG         BV06
#define GREET_PROG         BV07
#define RGREET_PROG	   BV07
#define OGREET_PROG        BV07
#define ALL_GREET_PROG	   BV08
#define GIVE_PROG	   BV09
#define BRIBE_PROG	   BV10
#define HOUR_PROG	   BV11
#define TIME_PROG	   BV12
#define WEAR_PROG          BV13
#define REMOVE_PROG        BV14
#define SAC_PROG           BV15
#define LOOK_PROG          BV16
#define EXA_PROG           BV17
#define CUSTOM_PROG        BV18
#define GET_PROG 	   BV19
#define DROP_PROG	   BV20
#define DAMAGE_PROG	   BV21
#define REPAIR_PROG	   BV22
#define RANDIW_PROG	   BV23
#define SPEECHIW_PROG	   BV24
#define PULL_PROG	   BV25
#define PUSH_PROG	   BV26
#define SLEEP_PROG         BV27
#define REST_PROG          BV28
#define LEAVE_PROG         BV29
#define SCRIPT_PROG	   BV30
#define USE_PROG           BV31

void rprog_leave_trigger( CHAR_DATA * ch );
void rprog_enter_trigger( CHAR_DATA * ch );
void rprog_sleep_trigger( CHAR_DATA * ch );
void rprog_rest_trigger( CHAR_DATA * ch );
void rprog_rfight_trigger( CHAR_DATA * ch );
void rprog_death_trigger( CHAR_DATA * killer, CHAR_DATA * ch );
void rprog_speech_trigger( const char *txt, CHAR_DATA * ch );
void rprog_random_trigger( CHAR_DATA * ch );
void rprog_time_trigger( CHAR_DATA * ch );
void rprog_hour_trigger( CHAR_DATA * ch );
char *rprog_type_to_name( int type );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( char *buf, OBJ_DATA * mobj, CHAR_DATA * ch, OBJ_DATA * obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA * room, CHAR_DATA * ch, OBJ_DATA * obj, void *vo );
#endif

#define GET_BETTED_ON(ch)    ((ch)->betted_on)
#define GET_BET_AMT(ch) ((ch)->bet_amt)
#define IN_ARENA(ch)            (IS_SET((ch)->in_room->room_flags2, ROOM_ARENA))

#define VCHECK_ROOM 0
#define VCHECK_OBJ 1
#define VCHECK_MOB 2
bool is_valid_vnum( int vnum, short type );

/*
 *  Defines for the command flags. --Shaddai
 */
#define CMD_POSSESS           BV00
#define CMD_ACTION            BV01  /* Samson 7-7-00 */
#define CMD_MUDPROG           BV02  /* Command is only used by mudprogs. Prevents display on help/commands. Samson 11-26-03 */
#define CMD_NOFORCE           BV03  /* Command can't be forced using either the force command or mpforce - Samson 3-3-04 */
#define CMD_OOC               BV04
#define CMD_BUILD             BV05
#define CMD_PLR_ONLY          BV06
#define CMD_FULLNAME          BV07  //You have to use the fullname of the command

#define IS_CMD_MPROG(cmd)          (IS_SET( (cmd)->flags, CMD_MUDPROG) )
#define IS_CMDFLAG( cmd, flag )    ( IS_SET( (cmd)->flags, (flag) ) )
extern bool DONT_UPPER;
