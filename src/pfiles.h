/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud (c)1997-2002 Alsherok. Contributors: Samson, Dwip, Whir,          *
 * Cyberfox, Karangi, Rathian, Cam, Raine, and Tarl.                        *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *                          Pfile Pruning Module                            *
 ****************************************************************************/

/* Used to interact with other snippets */
#define PFILECODE

#ifndef FCLOSE
#define FCLOSE(fp)  fclose(fp); fp=NULL;
#endif

#ifndef MSL
#define MSL MAX_STRING_LENGTH
#endif

#ifndef MIL
#define MIL MAX_INPUT_LENGTH
#endif

extern time_t pfile_time;
extern HOUR_MIN_SEC *set_pfile_time;
extern struct tm *new_pfile_time;
extern time_t new_pfile_time_t;
extern short num_pfiles;

void check_pfiles args( ( time_t reset ) );
void init_pfile_scan_time args( ( void ) );

DECLARE_DO_FUN( do_pfiles );
