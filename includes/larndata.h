/* Larn is copyrighted 1986 by Noah Morgan. */

#include <time.h>

/*
*
* types
*
*/

/*  this is the structure that holds the entire dungeon specifications  */
struct cel
{
  int hitp;			/*  monster's hit points    */
  int mitem;			/*  the monster ID          */
  int item;			/*  the object's ID         */
  int iarg;			/*  the object's argument   */
  int know;			/*  have we been here before */
};

/* this is the structure for maintaining & moving the spheres of annihilation */
struct sphere
{
  struct sphere *p;		/* pointer to next structure */
  int x, y, lev;		/* location of the sphere */
  int dir;			/* direction sphere is going in */
  int lifetime;			/* duration of the sphere */
};


/*  this is the structure definition of the monster data
*/
struct monst
{
  char *name;
  int level;
  int armorclass;
  int damage;
  int attack;
  int defense;
  int genocided;
  int intelligence;		/* monsters intelligence -- used to choose movement */
  int gold;
  int hitpoints;
  unsigned long experience;
};

/*  this is the structure definition for the items in the dnd store */
struct _itm
{
  int price;
  int obj;
  int arg;
  int qty;
};


/*
*
* data declarations
*
*/
extern int regen_bottom;
extern char floorc, wallc;

/* extern char for the water. -Gibbon */
extern char waterc;

extern int VERSION, SUBVERSION;
extern int beenhere[], cheat;
extern int course[];
extern int item[MAXX][MAXY], iven[], know[MAXX][MAXY];

extern char aborted[];
extern char *classname[];
extern char lastmonst[];
extern char *lpnt, *lpbuf, *lpend, *inbuffer;

extern int level;
extern int mitem[MAXX][MAXY], monstlevel[];
extern int nch[], ndgg[], nlpts[], nomove;
extern int nplt[], nsw[];
extern int potprob[];

extern char monstnamelist[];
extern char *levelname[];
extern char objnamelist[];
extern char logname[];

extern char mazefile[];
extern char diagfile[];
extern char fortfile[];
extern char helpfile[];
extern char logfile[];
extern char playerids[];

extern int predostuff, restorflag;
extern char savefilename[];
extern char scorefile[];
extern int scprob[];
extern int screen[MAXX][MAXY], sex;
extern int spelknow[];

extern char *spelmes[];
extern char *speldescript[];
extern char *spelcode[];



extern char *spelname[];


extern int splev[], stealth[MAXX][MAXY], wizard;
extern int diroffx[], diroffy[], hitflag, hit2flag, hit3flag,
  hitp[MAXX][MAXY];
extern int iarg[MAXX][MAXY], ivenarg[], lasthx, lasthy, lastnum, lastpx,
  lastpy;
extern int oldx, oldy, playerx, playery;
extern int enable_scroll, yrepcount, wisid, lfd, fd;
extern long outstanding_taxes, skill[], gtime;
extern long cdesc[], cbak[];
extern time_t initialtime;
extern unsigned long lrandx;
extern struct cel *cell;
extern struct sphere *spheres;


extern struct monst monster[];

extern struct _itm dnd_item[];



/*
* config.c
*/
extern char *password;



/*
* data.c
*/
extern int prayed;

extern char scrollname[MAXSCROLL + 1][MAXSCROLLNAME];
extern char potionname[MAXPOTION + 1][MAXPOTIONNAME];

extern char *objectname[];


extern int spelweird[MAXMONST + 8][SPNUM];



/*
* main.c
*/
extern int rmst;
extern int dropflag;
extern int save_mode;


/*
* store.c
*/
extern int lasttime;


/*
* tok.c
*/
extern int move_no_pickup;

//extern int larn_die(char, char *);
