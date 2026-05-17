#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"
#include <time.h>
#include "io.h"
#include "moreobj.h"

/*
classname[c[LEVEL]-1] gives the correct name of the players experience level
*/
static char aa1[] = " mighty evil master";
static char aa2[] = "apprentice demi-god";
static char aa3[] = "  minor demi-god   ";
static char aa4[] = "  major demi-god   ";
static char aa5[] = "    minor deity    ";
static char aa6[] = "    major deity    ";
static char aa7[] = "  novice guardian  ";
static char aa8[] = "apprentice guardian";
static char aa9[] = "    The Creator    ";
char* classname[] = { "  novice explorer  ", "apprentice explorer", " practiced explorer",	/*  -3 */
  "   expert explorer ", "  novice adventurer", "     adventurer    ",	/*  -6 */
  "apprentice conjurer", "     conjurer      ", "  master conjurer  ",	/*  -9 */
  "  apprentice mage  ", "        mage       ", "  experienced mage ",	/* -12 */
  "     master mage   ", " apprentice warlord", "   novice warlord  ",	/* -15 */
  "   expert warlord  ", "   master warlord  ", " apprentice gorgon ",	/* -18 */
  "       gorgon      ", "  practiced gorgon ", "   master gorgon   ",	/* -21 */
  "    demi-gorgon    ", "    evil master    ", " great evil master ",	/* -24 */
  aa1, aa1, aa1,		/* -27 */
  aa1, aa1, aa1,		/* -30 */
  aa1, aa1, aa1,		/* -33 */
  aa1, aa1, aa1,		/* -36 */
  aa1, aa1, aa1,		/* -39 */
  aa2, aa2, aa2,		/* -42 */
  aa2, aa2, aa2,		/* -45 */
  aa2, aa2, aa2,		/* -48 */
  aa3, aa3, aa3,		/* -51 */
  aa3, aa3, aa3,		/* -54 */
  aa3, aa3, aa3,		/* -57 */
  aa4, aa4, aa4,		/* -60 */
  aa4, aa4, aa4,		/* -63 */
  aa4, aa4, aa4,		/* -66 */
  aa5, aa5, aa5,		/* -69 */
  aa5, aa5, aa5,		/* -72 */
  aa5, aa5, aa5,		/* -75 */
  aa6, aa6, aa6,		/* -78 */
  aa6, aa6, aa6,		/* -81 */
  aa6, aa6, aa6,		/* -84 */
  aa7, aa7, aa7,		/* -87 */
  aa8, aa8, aa8,		/* -90 */
  aa8, aa8, aa8,		/* -93 */
  "  earth guardian   ", "   air guardian    ", "   fire guardian   ",	/* -96 */
  "  water guardian   ", "  time guardian    ", " ethereal guardian ",	/* -99 */
  aa9, aa9, aa9,		/* -102 */
};

/*
table of experience needed to be a certain level of player
skill[c[LEVEL]] is the experience required to attain the next level
*/
#define MEG 1000000
long skill[] = {
  0, 10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120,	/*  1-11 */
  10240, 20480, 40960, 100000, 200000, 400000, 700000, 1 * MEG,	/* 12-19 */
  2 * MEG, 3 * MEG, 4 * MEG, 5 * MEG, 6 * MEG, 8 * MEG, 10 * MEG,	/* 20-26 */
  12 * MEG, 14 * MEG, 16 * MEG, 18 * MEG, 20 * MEG, 22 * MEG, 24 * MEG, 26 * MEG, 28 * MEG,	/* 27-35 */
  30 * MEG, 32 * MEG, 34 * MEG, 36 * MEG, 38 * MEG, 40 * MEG, 42 * MEG, 44 * MEG, 46 * MEG,	/* 36-44 */
  48 * MEG, 50 * MEG, 52 * MEG, 54 * MEG, 56 * MEG, 58 * MEG, 60 * MEG, 62 * MEG, 64 * MEG,	/* 45-53 */
  66 * MEG, 68 * MEG, 70 * MEG, 72 * MEG, 74 * MEG, 76 * MEG, 78 * MEG, 80 * MEG, 82 * MEG,	/* 54-62 */
  84 * MEG, 86 * MEG, 88 * MEG, 90 * MEG, 92 * MEG, 94 * MEG, 96 * MEG, 98 * MEG, 100 * MEG,	/* 63-71 */
  105 * MEG, 110 * MEG, 115 * MEG, 120 * MEG, 125 * MEG, 130 * MEG, 135 * MEG, 140 * MEG,	/* 72-79 */
  145 * MEG, 150 * MEG, 155 * MEG, 160 * MEG, 165 * MEG, 170 * MEG, 175 * MEG, 180 * MEG,	/* 80-87 */
  185 * MEG, 190 * MEG, 195 * MEG, 200 * MEG, 210 * MEG, 220 * MEG, 230 * MEG, 240 * MEG,	/* 88-95 */
  250 * MEG, 260 * MEG, 270 * MEG, 280 * MEG, 290 * MEG, 300 * MEG	/* 96-101 */
};

#undef MEG

char* lpbuf, * lpnt, * inbuffer, * lpend;	/* input/output pointers to the buffers */

struct cel* cell;		/*  pointer to the dungeon storage  */

int hitp[MAXX][MAXY];		/*  monster hp on level     */
int iarg[MAXX][MAXY];		/*  arg for the item array  */
int item[MAXX][MAXY];		/*  objects in maze if any  */
int know[MAXX][MAXY];		/*  1 or 0 if here before   */
int mitem[MAXX][MAXY];		/*  monster item array      */
int stealth[MAXX][MAXY];	/*  0=sleeping 1=awake monst */
char lastmonst[40];		/*  this has the name of the current monster    */
int beenhere[MAXLEVEL + MAXVLEVEL];	/*  1 if have been on this level */
int VERSION = VER;		/*  this is the present version # of the program    */
int SUBVERSION = SUB_VERSION;
int predostuff = 0;		/*  2 means that the trap handling routines must do a
				   showplayer() after a trap.  0 means don't showplayer()
				   0 - we are in create player screen
				   1 - we are in welcome screen
				   2 - we are in the normal game   */

char logname[LOGNAMESIZE];	/* the player's name */
char loginname[20];
int cheat = 0;			/*  1 if the player has fudged save file            */
int sex = 1;			/* default is a man 0=woman */
int ckpflag = 0;		/* 1 if want checkpointing of game, 0 otherwise */
int level = 0;			/*  cavelevel player is on = c[CAVELEVEL]           */
int wizard = 0;			/*  the wizard mode flag                            */
int lastnum = 0;		/* the number of the monster last hitting player    */
int hitflag = 0;		/*  flag for if player has been hit when running    */
int hit2flag = 0;		/*  flag for if player has been hit when running    */
int hit3flag = 0;		/*  flag for if player has been hit flush input     */
int playerx, playery;		/*  the room on the present level of the player     */
int lastpx, lastpy;		/*  0 --- MAXX-1  or  0 --- MAXY-1                  */
int oldx, oldy;
int prayed = 1;			/* did player pray at an altar (command mode)? needs
				   to be saved, but I don't want to add incompatibility
				   right now.  KBR 1/11/90 */
int lasthx = 0, lasthy = 0;	/* location of monster last hit by player       */
unsigned long lrandx = 33601;	/*  the random number seed                      */
time_t initialtime = 0;		/* time playing began                           */
long gtime = 0;			/*  the clock for the game                      */
long outstanding_taxes = 0;	/* present tax bill from score file             */
long c[100], cbak[100];	/*  the character description arrays            */
int enable_scroll = 0;		/* constant for enabled/disabled scrolling regn */
char aborted[] = " aborted";
struct sphere* spheres = 0;	/*pointer to linked list for spheres of annihilation */

char* levelname[] =
{ " H", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", "V1",
"V2", "V3", "V4"
};

char objnamelist[MAXOBJECT + 37] = ".ATOP<^F&^+M=>^$$f*OD#~][[)))(((||||||||{?!BC}o:@.%%%%EVV))([[]]](^ [H*** ^^ S tsTLc`ulhatwpV~=~";
char monstnamelist[] = ".BGHJKOScjtAELNQRZabhiCTYdegmvzFWflorXV.pqsyUkMwDDPxnDDuD........,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,";

char floorc = '.';
char wallc = '#';
char* objectname[] =
{ 0, "a holy altar", "a handsome jewel encrusted throne", "the orb",
"a pit",
"a staircase leading upwards", "an elevator going up",
"a bubbling fountain",
"a great marble statue", "a teleport trap", "the college of Larn",
"a mirror", "the DND store", "a staircase going down",
"an elevator going down",
"the bank of Larn", "the 5th branch of the Bank of Larn",
"a dead fountain", "gold", "an open door", "a closed door",
"a wall", "The Eye of Larn", "plate mail", "chain mail", "leather armor",
"a sword of slashing", "Bessman's flailing hammer", "a sunsword",
"a two handed sword", "a spear", "a dagger",
"ring of extra regeneration", "a ring of regeneration",
"a ring of protection",
"an energy ring", "a ring of dexterity", "a ring of strength",
"a ring of cleverness", "a ring of increase damage", "a belt of striking",
"a magic scroll", "a magic potion", "a book", "a chest",
"an amulet of invisibility", "an orb of dragon slaying",
"a scarab of negate spirit", "a cube of undead control",
"device of theft prevention", "a brilliant diamond", "a ruby",
"an enchanting emerald", "a sparkling sapphire", "the dungeon entrance",
"a volcanic shaft leaning downward", "the base of a volcanic shaft",
"a battle axe", "a longsword", "a longsword of slashing", "ring mail",
"studded leather armor",
"splint mail", "plate armor", "stainless plate armor",
"a lance of death",
"an arrow trap", "an arrow trap", "a shield", "your home",
"gold", "gold", "gold", "a dart trap",
"a dart trap", "a trapdoor", "a trapdoor", "the local trading post",
"a teleport trap", "a massive throne",
"a sphere of annihilation", "a handsome jewel encrusted throne",
"the Larn Revenue Service", "a fortune cookie"
};

/*
*  for the monster data
*
*  array to do rnd() to create monsters <= a given level
*/
int monstlevel[] = { 5, 11, 17, 22, 27, 33, 39, 42, 46, 50, 53, 56, 59 };

struct monst monster[] = {
	/*  NAME            LV  AC  DAM ATT DEF GEN INT GOLD    HP  EXP
	   ----------------------------------------------------------------- */
	{"", 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0},
	{"bat", 1, 0, 1, 0, 0, 0, 3, 0, 1, 1, 0},
	{"gnome", 1, 10, 1, 0, 0, 0, 8, 30, 2, 2, 0},
	{"hobgoblin", 1, 14, 2, 0, 0, 0, 5, 25, 3, 2, 0},
	{"jackal", 1, 17, 1, 0, 0, 0, 4, 0, 1, 1, 0},
	{"kobold", 1, 20, 1, 0, 0, 0, 7, 10, 1, 1, 0},

	{"orc", 2, 12, 1, 0, 0, 0, 9, 40, 4, 2, 0},
	{"snake", 2, 15, 1, 0, 0, 0, 3, 0, 3, 1, 0},
	{"giant centipede", 2, 14, 0, 4, 0, 0, 3, 0, 1, 2, 0},
	{"jaculi", 2, 20, 1, 0, 0, 0, 3, 0, 2, 1, 0},
	{"troglodyte", 2, 10, 2, 0, 0, 0, 5, 80, 4, 3, 0},
	{"giant ant", 2, 8, 1, 4, 0, 0, 4, 0, 5, 5, 0},

	/*  NAME            LV  AC  DAM ATT DEF GEN INT GOLD    HP  EXP
	   ----------------------------------------------------------------- */

	{"floating eye", 3, 8, 1, 0, 0, 0, 3, 0, 5, 2, 0},
	{"leprechaun", 3, 3, 0, 8, 0, 0, 3, 1500, 13, 45, 0},
	{"nymph", 3, 3, 0, 14, 0, 0, 9, 0, 18, 45, 0},
	{"quasit", 3, 5, 3, 0, 0, 0, 3, 0, 10, 15, 0},
	{"rust monster", 3, 4, 0, 1, 0, 0, 3, 0, 18, 25, 0},
	{"zombie", 3, 12, 2, 0, 0, 0, 3, 0, 6, 7, 0},

	{"assassin bug", 4, 9, 3, 0, 0, 0, 3, 0, 20, 15, 0},
	{"bugbear", 4, 5, 4, 15, 0, 0, 5, 40, 20, 35, 0},
	{"hell hound", 4, 5, 2, 2, 0, 0, 6, 0, 16, 35, 0},
	{"ice lizard", 4, 11, 2, 10, 0, 0, 6, 50, 16, 25, 0},
	{"centaur", 4, 6, 4, 0, 0, 0, 10, 40, 24, 45, 0},

	/*  NAME            LV  AC  DAM ATT DEF GEN INT GOLD    HP  EXP
	   ----------------------------------------------------------------- */

	{"troll", 5, 4, 5, 0, 0, 0, 9, 80, 50, 300, 0},
	{"yeti", 5, 6, 4, 0, 0, 0, 5, 50, 35, 100, 0},
	{"white dragon", 5, 2, 4, 5, 0, 0, 16, 500, 55, 1000, 0},
	{"elf", 5, 8, 1, 0, 0, 0, 15, 50, 22, 35, 0},
	{"gelatinous cube", 5, 9, 1, 0, 0, 0, 3, 0, 22, 45, 0},

	{"metamorph", 6, 7, 3, 0, 0, 0, 3, 0, 30, 40, 0},
	{"vortex", 6, 4, 3, 0, 0, 0, 3, 0, 30, 55, 0},
	{"ziller", 6, 15, 3, 0, 0, 0, 3, 0, 30, 35, 0},
	{"violet fungi", 6, 12, 3, 0, 0, 0, 3, 0, 38, 100, 0},
	{"wraith", 6, 3, 1, 6, 0, 0, 3, 0, 30, 325, 0},
	{"forvalaka", 6, 2, 5, 0, 0, 0, 7, 0, 50, 280, 0},

	/*  NAME            LV  AC  DAM ATT DEF GEN INT GOLD    HP  EXP
	   ----------------------------------------------------------------- */

	{"lama nobe", 7, 7, 3, 0, 0, 0, 6, 0, 35, 80, 0},
	{"osequip", 7, 4, 3, 16, 0, 0, 4, 0, 35, 100, 0},
	{"rothe", 7, 15, 5, 0, 0, 0, 3, 100, 50, 250, 0},
	{"xorn", 7, 0, 6, 0, 0, 0, 13, 0, 60, 300, 0},
	{"vampire", 7, 3, 4, 6, 0, 0, 17, 0, 50, 1000, 0},
	{"invisible stalker", 7, 3, 6, 0, 0, 0, 5, 0, 50, 350, 0},

	{"poltergeist", 8, 1, 4, 0, 0, 0, 3, 0, 50, 450, 0},
	{"disenchantress", 8, 3, 0, 9, 0, 0, 3, 0, 50, 500, 0},
	{"shambling mound", 8, 2, 5, 0, 0, 0, 6, 0, 45, 400, 0},
	{"yellow mold", 8, 12, 4, 0, 0, 0, 3, 0, 35, 250, 0},
	{"umber hulk", 8, 3, 7, 11, 0, 0, 14, 0, 65, 600, 0},

	/*  NAME            LV  AC  DAM ATT DEF GEN INT GOLD    HP  EXP
	   ----------------------------------------------------------------- */

	{"gnome king", 9, -1, 10, 0, 0, 0, 18, 2000, 100, 3000, 0},
	{"mimic", 9, 5, 6, 0, 0, 0, 8, 0, 55, 99, 0},
	{"water lord", 9, -10, 15, 7, 0, 0, 20, 0, 150, 15000, 0},
	{"bronze dragon", 9, 2, 9, 3, 0, 0, 16, 300, 80, 4000, 0},
	{"green dragon", 9, 3, 8, 10, 0, 0, 15, 200, 70, 2500, 0},
	{"purple worm", 9, -1, 11, 0, 0, 0, 3, 100, 120, 15000, 0},
	{"xvart", 9, -2, 12, 0, 0, 0, 13, 0, 90, 1000, 0},

	{"spirit naga", 10, -20, 12, 12, 0, 0, 23, 0, 95, 20000, 0},
	{"silver dragon", 10, -1, 12, 3, 0, 0, 20, 700, 100, 10000, 0},
	{"platinum dragon", 10, -5, 15, 13, 0, 0, 22, 1000, 130, 24000, 0},
	{"green urchin", 10, -3, 12, 0, 0, 0, 3, 0, 85, 5000, 0},
	{"red dragon", 10, -2, 13, 3, 0, 0, 19, 800, 110, 14000, 0},

	{"type I demon lord", 12, -30, 18, 0, 0, 0, 20, 0, 140, 50000, 0},
	{"type II demon lord", 13, -30, 18, 0, 0, 0, 21, 0, 160, 75000, 0},
	{"type III demon lord", 14, -30, 18, 0, 0, 0, 22, 0, 180, 100000, 0},
	{"type IV demon lord", 15, -35, 20, 0, 0, 0, 23, 0, 200, 125000, 0},
	{"type V demon lord", 16, -40, 22, 0, 0, 0, 24, 0, 220, 150000, 0},
	{"type VI demon lord", 17, -45, 24, 0, 0, 0, 25, 0, 240, 175000, 0},
	{"type VII demon lord", 18, -70, 27, 6, 0, 0, 26, 0, 260, 200000, 0},
	{"demon prince", 25, -127, 30, 6, 0, 0, 28, 0, 345, 300000, 0}

	/*  NAME                LV  AC  DAM ATT DEF GEN INT GOLD    HP  EXP
	   --------------------------------------------------------------------- */
};

/*  name array for scrolls      */

char scrollname[MAXSCROLL + 1][MAXSCROLLNAME] = {
  "\0enchant armor",
  "\0enchant weapon",
  "\0enlightenment",
  "\0blank paper",
  "\0create monster",
  "\0create artifact",
  "\0aggravate monsters",
  "\0time warp",
  "\0teleportation",
  "\0expanded awareness",
  "\0haste monsters",
  "\0monster healing",
  "\0spirit protection",
  "\0undead protection",
  "\0stealth",
  "\0magic mapping",
  "\0hold monsters",
  "\0gem perfection",
  "\0spell extension",
  "\0identify",
  "\0remove curse",
  "\0annihilation",
  "\0pulverization",
  "\0life protection",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0zzzzzzzzzzzzzz"		/* sentinel, for the sorted known objects inventory */
};

/*  name array for magic potions    */
char potionname[MAXPOTION + 1][MAXPOTIONNAME] = {
  "\0sleep",
  "\0healing",
  "\0raise level",
  "\0increase ability",
  "\0wisdom",
  "\0strength",
  "\0raise charisma",
  "\0dizziness",
  "\0learning",
  "\0object detection",
  "\0monster detection",
  "\0forgetfulness",
  "\0water",
  "\0blindness",
  "\0confusion",
  "\0heroism",
  "\0sturdiness",
  "\0giant strength",
  "\0fire resistance",
  "\0treasure finding",
  "\0instant healing",
  " cure dianthroritis",
  "\0poison",
  "\0see invisible",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0 ",
  "\0zzzzzzzzzzzzzz"		/* sentinel, for the sorted known objects inventory */
};


/*
spell data
*/
int spelknow[SPNUM];
int splev[] = { 1, 4, 9, 14, 18, 22, 26, 29, 32, 35, 37, 37, 37, 37, 37 };

char* spelcode[SPNUM + 1] = {
  "pro", "mle", "dex", "sle", "chm", "ssp",
  "web", "str", "enl", "hel", "cbl", "cre", "pha", "inv",
  "bal", "cld", "ply", "can", "has", "ckl", "vpr",
  "dry", "lit", "drl", "glo", "flo", "fgr",
  "sca", "hld", "stp", "tel", "mfi",	/* 31 */
  "sph", "gen", "sum", "wtw", "alt", "per", "zzz"
};

char* spelname[] = {
  "protection", "magic missile", "dexterity",
  "sleep", "charm monster", "sonic spear",

  "web", "strength", "enlightenment",
  "healing", "cure blindness", "create monster",
  "phantasmal forces", "invisibility",

  "fireball", "cold", "polymorph",
  "cancellation", "haste self", "cloud kill",
  "vaporize rock",

  "dehydration", "lightning", "drain life",
  "invulnerability", "flood", "finger of death",

  "scare monster", "hold monster", "time stop",
  "teleport away", "magic fire",

  "sphere of annihilation", "genocide", "summon demon",
  "walk through walls", "alter reality", "permanence",
  ""
};

char* speldescript[] = {
	/* 1 */
	"generates a +2 protection field",
	"creates and hurls a magic missile equivalent to a + 1 magic arrow",
	"adds +2 to the casters dexterity",
	"causes some monsters to go to sleep",
	"some monsters may be awed at your magnificence",
	"causes your hands to emit a screeching sound toward what they point",
	/* 7 */
	"causes strands of sticky thread to entangle an enemy",
	"adds +2 to the casters strength for a short term",
	"the caster becomes aware of things around him",
	"restores some hp to the caster",
	"restores sight to one so unfortunate as to be blinded",
	"creates a monster near the caster appropriate for the location",
	"creates illusions, and if believed, monsters die",
	"the caster becomes invisible",
	/* 15 */
	"makes a ball of fire that burns on what it hits",
	"sends forth a cone of cold which freezes what it touches",
	"you can find out what this does for yourself",
	"negates the ability of a monster to use his special abilities",
	"speeds up the casters movements",
	"creates a fog of poisonous gas which kills all that is within it",
	"this changes rock to air",
	/* 22 */
	"dries up water in the immediate vicinity",
	"you finger will emit a lightning bolt when this spell is cast",
	"subtracts hit points from both you and a monster",
	"this globe helps to protect the player from physical attack",
	"this creates an avalanche of H2O to flood the immediate chamber",
	"this is a holy spell and calls upon your god to back you up",
	/* 28 */
	"terrifies the monster so that hopefully he wont hit the magic user",
	"the monster is frozen in his tracks if this is successful",
	"all movement in the caverns ceases for a limited duration",
	"moves a particular monster around in the dungeon (hopefully away from you)",
	"this causes a curtain of fire to appear all around you",
	/* 33 */
	"anything caught in this sphere is instantly killed.  Warning -- dangerous",
	"eliminates a species of monster from the game -- use sparingly",
	"summons a demon who hopefully helps you out",
	"allows the player to walk through walls for a short period of time",
	"god only knows what this will do",
	"makes a character spell permanent, i. e. protection, strength, etc.",
	""
};

int spelweird[MAXMONST + 8][SPNUM] = {
	/*                      p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
	/*                      r l e l h s    e t n e b r h n    a l l a a k p    r i r l l g    c l t e f    p e u t l e */
	/*                      o e x e m p    b r l l l e a v    l d y n s l r    y t l o o r    a d p l i    h n m w t r */


	/*            bat */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			  0, 0},
			  /*          gnome */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						0, 0},
						/*      hobgoblin */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								  0, 0},
								  /*         jackal */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
											0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
											0, 0},
											/*         kobold */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
													  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
													  0, 0},

													  /*            orc */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																0, 0},
																/*          snake */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																		  0, 0},
																		  /*giant centipede */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																					0, 0},
																					/*         jaculi */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																							  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																							  0, 0},
																							  /*     troglodyte */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																										0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																										0, 0},

																										/*      giant ant */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																												  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																												  0, 0},
																												  /*   floating eye */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																															0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																															0, 0},
																															/*     leprechaun */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																	  0, 0},
																																	  /*          nymph */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																				0, 0},
																																				/*         quasit */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																						  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																						  0, 0},

																																						  /*   rust monster */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																									0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																									0, 0},
																																									/*         zombie */ {0, 0, 0, 8, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																											  0, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																											  0, 0},
																																											  /*   assassin bug */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																														0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																														0, 0},
																																														/*        bugbear */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
																																																  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																  0, 0},
																																																  /*     hell hound */ {0, 6, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																			0, 0},

																																																			/*     ice lizard */ {0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0,
																																																					  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																					  0, 0, 0},
																																																					  /*        centaur */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																								0, 0},
																																																								/*          troll */ {0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
																																																										  0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																										  0, 0},
																																																										  /*           yeti */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0,
																																																													0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																													0, 0},
																																																													/*   white dragon */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 15, 0,
																																																															  0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																															  0, 0, 0},

																																																															  /*            elf */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 5, 0, 0, 0, 0,
																																																																		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																		0, 0},
																																																																		/*gelatinous cube */ {0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																				  0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																				  0, 0},
																																																																				  /*      metamorph */ {0, 13, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																							0, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																							0, 0},
																																																																							/*         vortex */ {0, 13, 0, 0, 0, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																									  0, 0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																									  0, 0, 0},
																																																																									  /*         ziller */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																												0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																												0, 0},

																																																																												/*   violet fungi */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																														  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																														  0, 0},
																																																																														  /*         wraith */ {0, 0, 0, 8, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																	0, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																	0, 0},
																																																																																	/*      forvalaka */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
																																																																																			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																			  0, 0},
																																																																																			  /*      lama nobe */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																						0, 0},
																																																																																						/*        osequip */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																								  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																								  0, 0},

																																																																																								  /*          rothe */ {0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
																																																																																											0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																											0, 0},
																																																																																											/*           xorn */ {0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
																																																																																													  0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																													  0, 0},
																																																																																													  /*        vampire */ {0, 0, 0, 8, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																0, 0},
																																																																																																/*invisible staker */ {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																		   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																		   0, 0},
																																																																																																		   /*    poltergeist */ {0, 13, 0, 8, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0,
																																																																																																					 0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																					 0, 0},

																																																																																																					 /* disenchantress */ {0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																							   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																							   0, 0},
																																																																																																							   /*shambling mound */ {0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																										 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																										 0, 0},
																																																																																																										 /*    yellow mold */ {0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0,
																																																																																																												   0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																												   0, 0},
																																																																																																												   /*     umber hulk */ {0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
																																																																																																															 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																															 0, 0},
																																																																																																															 /*     gnome king */ {0, 7, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 9, 0,
																																																																																																																	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																	   0, 0},

																																																																																																																	   /*          mimic */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																				 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																				 0, 0},
																																																																																																																				 /*     water lord */ {0, 13, 0, 8, 3, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0,
																																																																																																																						   0, 4, 0, 0, 0, 0, 0, 16, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																						   0, 0},
																																																																																																																						   /*  bronze dragon */ {0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																									 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																									 0, 0},
																																																																																																																									 /*   green dragon */ {0, 7, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																											   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																											   0, 0},
																																																																																																																											   /*    purple worm */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																														 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																														 0, 0},

																																																																																																																														 /*          xvart */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																   0, 0},
																																																																																																																																   /*    spirit naga */ {0, 13, 0, 8, 3, 4, 1, 0, 0, 0, 0, 0, 0, 5, 0, 4, 9, 0,
																																																																																																																																			 0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																			 0, 0},
																																																																																																																																			 /*  silver dragon */ {0, 6, 0, 9, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																					   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																					   0, 0},
																																																																																																																																					   /*platinum dragon */ {0, 7, 0, 9, 0, 0, 11, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0,
																																																																																																																																								 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																								 0, 0, 0},
																																																																																																																																								 /*   green urchin */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																										   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																										   0, 0},
																																																																																																																																										   /*     red dragon */ {0, 6, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																													 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
																																																																																																																																													 0, 0},

																																																																																																																																													 /*                      p m d s c s    w s e h c c p i    b c p c h c v    d l d g f f    s h s t m    s g s w a p */
																																																																																																																																													 /*                      r l e l h s    e t n e b r h n    a l l a a k p    r i r l l g    c l t e f    p e u t l e */
																																																																																																																																													 /*                      o e x e m p    b r l l l e a v    l d y n s l r    y t l o o r    a d p l i    h n m w t r */

	/*     demon lord */ {0, 7, 0, 4, 3, 0, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
			  0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 9, 0, 0, 0,
			  0, 0},
			  /*     demon lord */ {0, 7, 0, 4, 3, 0, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
						0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 9, 0, 0, 0,
						0, 0},
						/*     demon lord */ {0, 7, 0, 4, 3, 0, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
								  0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 9, 0, 0, 0,
								  0, 0},
								  /*     demon lord */ {0, 7, 0, 4, 3, 0, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
											0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 9, 0, 0, 0,
											0, 0},
											/*     demon lord */ {0, 7, 0, 4, 3, 0, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
													  0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 9, 0, 0, 0,
													  0, 0},
													  /*     demon lord */ {0, 7, 0, 4, 3, 0, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
																0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 9, 0, 0, 0,
																0, 0},
																/*     demon lord */ {0, 7, 0, 4, 3, 0, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
																		  0, 4, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 9, 0, 0, 0,
																		  0, 0},
																		  /*   demon prince */ {0, 7, 0, 4, 3, 9, 1, 0, 0, 0, 0, 0, 14, 5, 0, 0, 4, 0,
																					0, 4, 0, 4, 0, 0, 0, 4, 4, 4, 0, 0, 0, 4, 9, 0, 0, 0,
																					0, 0}

};

char* spelmes[] = { "",
/*  1 */ "the web had no effect on the %s",
/*  2 */ "the %s changed shape to avoid the web",
/*  3 */ "the %s isn't afraid of you",
/*  4 */ "the %s isn't affected",
/*  5 */ "the %s can see you with his infravision",
/*  6 */ "the %s vaporizes your missile",
/*  7 */ "your missile bounces off the %s",
/*  8 */ "the %s doesn't sleep",
/*  9 */ "the %s resists",
/* 10 */ "the %s can't hear the noise",
/* 11 */ "the %s's tail cuts it free of the web",
/* 12 */ "the %s burns through the web",
/* 13 */ "your missiles pass right through the %s",
/* 14 */ "the %s sees through your illusions",
/* 15 */ "the %s loves the cold!",
/* 16 */ "the %s loves the water!"
};

/*
*  function to create scroll numbers with appropriate probability of
*  occurrence
*
*  0 - armor           1 - weapon      2 - enlightenment   3 - paper
*  4 - create monster  5 - create item 6 - aggravate       7 - time warp
*  8 - teleportation   9 - expanded awareness              10 - haste monst
*  11 - heal monster   12 - spirit protection      13 - undead protection
*  14 - stealth        15 - magic mapping          16 - hold monster
*  17 - gem perfection 18 - spell extension        19 - identify
*  20 - remove curse   21 - annihilation           22 - pulverization
*  23 - life protection
*/
int scprob[] = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3,
  3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9,
  9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14,
  15, 15, 16, 16, 16, 17, 17, 18, 18, 19, 19, 19, 20, 20, 20, 20, 21, 22,
  22, 22, 23
};

/*
*  function to return a potion number created with appropriate probability
*  of occurrence
*
*  0 - sleep               1 - healing                 2 - raise level
*  3 - increase ability    4 - gain wisdom             5 - gain strength
*  6 - increase charisma   7 - dizziness               8 - learning
*  9 - object detection    10 - monster detection      11 - forgetfulness
*  12 - water              13 - blindness              14 - confusion
*  15 - heroism            16 - sturdiness             17 - giant strength
*  18 - fire resistance    19 - treasure finding       20 - instant healing
*  21 - cure dianthroritis 22 - poison                 23 - see invisible
*/
int potprob[] = { 0, 0, 1, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 9,
  10, 10, 10, 11, 11, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 19,
  20, 20, 22, 22, 23, 23
};

int nlpts[] = { 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7 };
int nch[] = { 0, 0, 0, 1, 1, 1, 2, 2, 3, 4 };
int nplt[] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 4 };
int ndgg[] = { 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5 };
int nsw[] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 3 };

/* Colours for Monsters and Objects */

/* One colour per monster ID */
int moncolor[MAXMONST + 9] = {
	[BAT] = COLOR_BLUE,
	[GNOME] = COLOR_GREEN,
	[HOBGOBLIN] = COLOR_RED,
	[JACKAL] = COLOR_YELLOW,
	[KOBOLD] = COLOR_CYAN,
	[ORC] = COLOR_MAGENTA,
	[SNAKE] = COLOR_GREEN,
	[CENTIPEDE] = COLOR_RED,
	[JACULI] = COLOR_BLUE,
	[TROGLODYTE] = COLOR_WHITE,
	[ANT] = COLOR_RED,
	[EYE] = COLOR_CYAN,
	[LEPRECHAUN] = COLOR_GREEN,
	[NYMPH] = COLOR_MAGENTA,
	[QUASIT] = COLOR_RED,
	[RUSTMONSTER] = COLOR_YELLOW,
	[ZOMBIE] = COLOR_GREEN,
	[ASSASSINBUG] = COLOR_RED,
	[BUGBEAR] = COLOR_YELLOW,
	[HELLHOUND] = COLOR_RED,
	[ICELIZARD] = COLOR_CYAN,
	[CENTAUR] = COLOR_WHITE,
	[TROLL] = COLOR_GREEN,
	[YETI] = COLOR_WHITE,
	[WHITEDRAGON] = COLOR_WHITE,
	[ELF] = COLOR_GREEN,
	[CUBE] = COLOR_CYAN,
	[METAMORPH] = COLOR_MAGENTA,
	[VORTEX] = COLOR_BLUE,
	[ZILLER] = COLOR_RED,
	[VIOLETFUNGI] = COLOR_MAGENTA,
	[WRAITH] = COLOR_CYAN,
	[FORVALAKA] = COLOR_RED,
	[LAMANOBE] = COLOR_WHITE,
	[OSEQUIP] = COLOR_YELLOW,
	[ROTHE] = COLOR_RED,
	[XORN] = COLOR_MAGENTA,
	[VAMPIRE] = COLOR_RED,
	[INVISIBLESTALKER] = COLOR_WHITE,
	[POLTERGEIST] = COLOR_CYAN,
	[DISENCHANTRESS] = COLOR_MAGENTA,
	[SHAMBLINGMOUND] = COLOR_GREEN,
	[YELLOWMOLD] = COLOR_YELLOW,
	[UMBERHULK] = COLOR_RED,
	[GNOMEKING] = COLOR_GREEN,
	[MIMIC] = COLOR_WHITE,
	[WATERLORD] = COLOR_BLUE,
	[BRONZEDRAGON] = COLOR_YELLOW,
	[GREENDRAGON] = COLOR_GREEN,
	[PURPLEWORM] = COLOR_MAGENTA,
	[XVART] = COLOR_CYAN,
	[SPIRITNAGA] = COLOR_RED,
	[SILVERDRAGON] = COLOR_WHITE,
	[PLATINUMDRAGON] = COLOR_CYAN,
	[GREENURCHIN] = COLOR_GREEN,
	[REDDRAGON] = COLOR_RED,
	[DEMONLORD] = COLOR_MAGENTA,
	[DEMONPRINCE] = COLOR_RED
};

/* One colour per object ID */
int objcolor[MAXOBJECT + 1] = {
	[0] = COLOR_WHITE,
	[OALTAR] = COLOR_YELLOW,
	[OTHRONE] = COLOR_MAGENTA,
	[OORB] = COLOR_CYAN,
	[OPIT] = COLOR_BLUE,
	[OSTAIRSUP] = COLOR_WHITE,
	[OELEVATORUP] = COLOR_WHITE,
	[OFOUNTAIN] = COLOR_CYAN,
	[OSTATUE] = COLOR_WHITE,
	[OTELEPORTER] = COLOR_MAGENTA,
	[OSCHOOL] = COLOR_GREEN,
	[OMIRROR] = COLOR_WHITE,
	[ODNDSTORE] = COLOR_GREEN,
	[OSTAIRSDOWN] = COLOR_WHITE,
	[OELEVATORDOWN] = COLOR_WHITE,
	[OBANK2] = COLOR_GREEN,
	[OBANK] = COLOR_GREEN,
	[ODEADFOUNTAIN] = COLOR_BLUE,
	[OMAXGOLD] = COLOR_YELLOW,
	[OGOLDPILE] = COLOR_YELLOW,
	[OOPENDOOR] = COLOR_WHITE,
	[OCLOSEDDOOR] = COLOR_YELLOW,
	[OWALL] = COLOR_WHITE,
	[OLARNEYE] = COLOR_CYAN,
	[OPLATE] = COLOR_WHITE,
	[OCHAIN] = COLOR_WHITE,
	[OLEATHER] = COLOR_YELLOW,
	[ORING] = COLOR_CYAN,
	[OSTUDLEATHER] = COLOR_YELLOW,
	[OSPLINT] = COLOR_WHITE,
	[OPLATEARMOR] = COLOR_WHITE,
	[OSSPLATE] = COLOR_WHITE,
	[OSHIELD] = COLOR_WHITE,
	[OELVENCHAIN] = COLOR_GREEN,
	[OSWORDofSLASHING] = COLOR_CYAN,
	[OHAMMER] = COLOR_YELLOW,
	[OSWORD] = COLOR_WHITE,
	[O2SWORD] = COLOR_WHITE,
	[OHSWORD] = COLOR_CYAN,
	[OSPEAR] = COLOR_WHITE,
	[ODAGGER] = COLOR_WHITE,
	[OBATTLEAXE] = COLOR_RED,
	[OLONGSWORD] = COLOR_WHITE,
	[OLANCE] = COLOR_CYAN,
	[OVORPAL] = COLOR_MAGENTA,
	[OSLAYER] = COLOR_RED,
	[ORINGOFEXTRA] = COLOR_CYAN,
	[OREGENRING] = COLOR_CYAN,
	[OPROTRING] = COLOR_CYAN,
	[OENERGYRING] = COLOR_CYAN,
	[ODEXRING] = COLOR_CYAN,
	[OSTRRING] = COLOR_CYAN,
	[OCLEVERRING] = COLOR_CYAN,
	[ODAMRING] = COLOR_CYAN,
	[OBELT] = COLOR_YELLOW,
	[OSCROLL] = COLOR_WHITE,
	[OPOTION] = COLOR_WHITE,
	[OBOOK] = COLOR_CYAN,
	[OCHEST] = COLOR_RED,
	[OAMULET] = COLOR_MAGENTA,
	[OORBOFDRAGON] = COLOR_CYAN,
	[OSPIRITSCARAB] = COLOR_MAGENTA,
	[OCUBEofUNDEAD] = COLOR_MAGENTA,
	[ONOTHEFT] = COLOR_RED,
	[ODIAMOND] = COLOR_WHITE,
	[ORUBY] = COLOR_RED,
	[OEMERALD] = COLOR_GREEN,
	[OSAPPHIRE] = COLOR_BLUE,
	[OENTRANCE] = COLOR_BLUE,
	[OVOLDOWN] = COLOR_RED,
	[OVOLUP] = COLOR_WHITE,
	[OHOME] = COLOR_CYAN,
	[OKGOLD] = COLOR_YELLOW,
	[ODGOLD] = COLOR_YELLOW,
	[OIVDARTRAP] = COLOR_RED,
	[ODARTRAP] = COLOR_RED,
	[OTRAPDOOR] = COLOR_RED,
	[OIVTRAPDOOR] = COLOR_RED,
	[OTRADEPOST] = COLOR_GREEN,
	[OIVTELETRAP] = COLOR_MAGENTA,
	[ODEADTHRONE] = COLOR_MAGENTA,
	[OANNIHILATION] = COLOR_MAGENTA,
	[OTHRONE2] = COLOR_MAGENTA,
	[OLRS] = COLOR_GREEN,
	[OCOOKIE] = COLOR_YELLOW,
	[OWATER] = COLOR_BLUE,
	[OSHOREWATER] = COLOR_CYAN,
	[OLAVA] = COLOR_RED
};

const struct color_override_entry monster_map[] = {
	{ "BAT", BAT },
	{ "GNOME", GNOME },
	{ "HOBGOBLIN", HOBGOBLIN },
	{ "JACKAL", JACKAL },
	{ "KOBOLD", KOBOLD },
	{ "ORC", ORC },
	{ "SNAKE", SNAKE },
	{ "CENTIPEDE", CENTIPEDE },
	{ "JACULI", JACULI },
	{ "TROGLODYTE", TROGLODYTE },
	{ "ANT", ANT },
	{ "EYE", EYE },
	{ "LEPRECHAUN", LEPRECHAUN },
	{ "NYMPH", NYMPH },
	{ "QUASIT", QUASIT },
	{ "RUSTMONSTER", RUSTMONSTER },
	{ "ZOMBIE", ZOMBIE },
	{ "ASSASSINBUG", ASSASSINBUG },
	{ "BUGBEAR", BUGBEAR },
	{ "HELLHOUND", HELLHOUND },
	{ "ICELIZARD", ICELIZARD },
	{ "CENTAUR", CENTAUR },
	{ "TROLL", TROLL },
	{ "YETI", YETI },
	{ "WHITEDRAGON", WHITEDRAGON },
	{ "ELF", ELF },
	{ "CUBE", CUBE },
	{ "METAMORPH", METAMORPH },
	{ "VORTEX", VORTEX },
	{ "ZILLER", ZILLER },
	{ "VIOLETFUNGI", VIOLETFUNGI },
	{ "WRAITH", WRAITH },
	{ "FORVALAKA", FORVALAKA },
	{ "LAMANOBE", LAMANOBE },
	{ "OSEQUIP", OSEQUIP },
	{ "ROTHE", ROTHE },
	{ "XORN", XORN },
	{ "VAMPIRE", VAMPIRE },
	{ "INVISIBLESTALKER", INVISIBLESTALKER },
	{ "POLTERGEIST", POLTERGEIST },
	{ "DISENCHANTRESS", DISENCHANTRESS },
	{ "SHAMBLINGMOUND", SHAMBLINGMOUND },
	{ "YELLOWMOLD", YELLOWMOLD },
	{ "UMBERHULK", UMBERHULK },
	{ "GNOMEKING", GNOMEKING },
	{ "MIMIC", MIMIC },
	{ "WATERLORD", WATERLORD },
	{ "BRONZEDRAGON", BRONZEDRAGON },
	{ "GREENDRAGON", GREENDRAGON },
	{ "PURPLEWORM", PURPLEWORM },
	{ "XVART", XVART },
	{ "SPIRITNAGA", SPIRITNAGA },
	{ "SILVERDRAGON", SILVERDRAGON },
	{ "PLATINUMDRAGON", PLATINUMDRAGON },
	{ "GREENURCHIN", GREENURCHIN },
	{ "REDDRAGON", REDDRAGON },
	{ "DEMONLORD", DEMONLORD },
	{ "DEMONPRINCE", DEMONPRINCE },
	{ NULL, 0 }
};

const struct color_override_entry object_map[] = {
	{ "NOTHING", 0 },
	{ "OALTAR", OALTAR },
	{ "OTHRONE", OTHRONE },
	{ "OORB", OORB },
	{ "OPIT", OPIT },
	{ "OSTAIRSUP", OSTAIRSUP },
	{ "OELEVATORUP", OELEVATORUP },
	{ "OFOUNTAIN", OFOUNTAIN },
	{ "OSTATUE", OSTATUE },
	{ "OTELEPORTER", OTELEPORTER },
	{ "OSCHOOL", OSCHOOL },
	{ "OMIRROR", OMIRROR },
	{ "ODNDSTORE", ODNDSTORE },
	{ "OSTAIRSDOWN", OSTAIRSDOWN },
	{ "OELEVATORDOWN", OELEVATORDOWN },
	{ "OBANK2", OBANK2 },
	{ "OBANK", OBANK },
	{ "ODEADFOUNTAIN", ODEADFOUNTAIN },
	{ "OMAXGOLD", OMAXGOLD },
	{ "OGOLDPILE", OGOLDPILE },
	{ "OOPENDOOR", OOPENDOOR },
	{ "OCLOSEDDOOR", OCLOSEDDOOR },
	{ "OWALL", OWALL },
	{ "OLARNEYE", OLARNEYE },
	{ "OPLATE", OPLATE },
	{ "OCHAIN", OCHAIN },
	{ "OLEATHER", OLEATHER },
	{ "ORING", ORING },
	{ "OSTUDLEATHER", OSTUDLEATHER },
	{ "OSPLINT", OSPLINT },
	{ "OPLATEARMOR", OPLATEARMOR },
	{ "OSSPLATE", OSSPLATE },
	{ "OSHIELD", OSHIELD },
	{ "OELVENCHAIN", OELVENCHAIN },
	{ "OSWORDofSLASHING", OSWORDofSLASHING },
	{ "OHAMMER", OHAMMER },
	{ "OSWORD", OSWORD },
	{ "O2SWORD", O2SWORD },
	{ "OHSWORD", OHSWORD },
	{ "OSPEAR", OSPEAR },
	{ "ODAGGER", ODAGGER },
	{ "OBATTLEAXE", OBATTLEAXE },
	{ "OLONGSWORD", OLONGSWORD },
	{ "OLANCE", OLANCE },
	{ "OVORPAL", OVORPAL },
	{ "OSLAYER", OSLAYER },
	{ "ORINGOFEXTRA", ORINGOFEXTRA },
	{ "OREGENRING", OREGENRING },
	{ "OPROTRING", OPROTRING },
	{ "OENERGYRING", OENERGYRING },
	{ "ODEXRING", ODEXRING },
	{ "OSTRRING", OSTRRING },
	{ "OCLEVERRING", OCLEVERRING },
	{ "ODAMRING", ODAMRING },
	{ "OBELT", OBELT },
	{ "OSCROLL", OSCROLL },
	{ "OPOTION", OPOTION },
	{ "OBOOK", OBOOK },
	{ "OCHEST", OCHEST },
	{ "OAMULET", OAMULET },
	{ "OORBOFDRAGON", OORBOFDRAGON },
	{ "OSPIRITSCARAB", OSPIRITSCARAB },
	{ "OCUBEofUNDEAD", OCUBEofUNDEAD },
	{ "ONOTHEFT", ONOTHEFT },
	{ "ODIAMOND", ODIAMOND },
	{ "ORUBY", ORUBY },
	{ "OEMERALD", OEMERALD },
	{ "OSAPPHIRE", OSAPPHIRE },
	{ "OENTRANCE", OENTRANCE },
	{ "OVOLDOWN", OVOLDOWN },
	{ "OVOLUP", OVOLUP },
	{ "OHOME", OHOME },
	{ "OKGOLD", OKGOLD },
	{ "ODGOLD", ODGOLD },
	{ "OIVDARTRAP", OIVDARTRAP },
	{ "ODARTRAP", ODARTRAP },
	{ "OTRAPDOOR", OTRAPDOOR },
	{ "OIVTRAPDOOR", OIVTRAPDOOR },
	{ "OTRADEPOST", OTRADEPOST },
	{ "OIVTELETRAP", OIVTELETRAP },
	{ "ODEADTHRONE", ODEADTHRONE },
	{ "OANNIHILATION", OANNIHILATION },
	{ "OTHRONE2", OTHRONE2 },
	{ "OLRS", OLRS },
	{ "OCOOKIE", OCOOKIE },
	{ "OWATER", OWATER },
	{ "OSHOREWATER", OSHOREWATER },
	{ "OLAVA", OLAVA },
	{ NULL, 0 }
};

int monattr[MAXMONST + 9] = {0};
int objattr[MAXOBJECT + 1] = {0};
