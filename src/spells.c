/*
cast()             Subroutine called by parse to cast a spell for the user
speldamage(x)      Function to perform spell functions cast by the player
loseint()          Routine to decrement your int (intelligence) if > 3
isconfuse()        Routine to check to see if player is confused
nospell(x,monst)   Routine to return 1 if a spell doesn't affect a monster
fullhit(xx)        Function to return full damage against a monst (aka web)
direct(spnum,dam,str,arg)   Routine to direct spell damage 1 square in 1 dir
godirect(spnum,dam,str,delay,cshow)     Function to perform missile attacks
ifblind(x,y)       Routine to put "monster" or the monster name into lastmosnt
tdirect(spnum)     Routine to teleport away a monster
omnidirect(sp,dam,str)  Routine to damage all monsters 1 square from player
dirsub(x,y)        Routine to ask for direction, then modify x,y for it
dirpoly(spnum)     Routine to ask for a direction and polymorph a monst
annihilate()   Routine to annihilate monsters around player, playerx,playery
genmonst()         Function to ask for monster and genocide from game
*/
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <curses.h>

#include "includes/create.h"
#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/display.h"
#include "includes/global.h"
#include "includes/io.h"
#include "includes/main.h"
#include "includes/monster.h"
#include "includes/object.h"
#include "includes/scores.h"
#include "includes/spells.h"
#include "includes/spheres.h"
#include "includes/sysdep.h"

/* used for altar reality */
struct isave
{

  int type;			/* 0=item,  1=monster */
  int id;			/* item number or monster number */

  int arg;			/* the type of item or hitpoints of monster */
};



static void speldamage (int);

static void create_guardian (int, int, int);

static void loseint (void);

static int isconfuse (void);

static int nospell (int, int);

static void direct (int, int, char *, int);

static void tdirect (int);

static void omnidirect (int, int, char *);

static void dirpoly (int);

static void genmonst (void);



/*
*  cast()      Subroutine called by parse to cast a spell for the user
*
*  No arguments and no return value.
*/
static char eys[] = "\nEnter your spell: ";

void
cast (void)
{
  int i, j, a, b, d;

  cursors ();

  if (cdesc[SPELLS] <= 0)
    {

      lprcat ("\nYou don't have any spells!");

      return;
    }

  lprcat (eys);

  --cdesc[SPELLS];

  while ((a = ttgetch ()) == 'I')
    {

      seemagic (-1);
      cursors ();
      lprcat (eys);
    }

  if (a == '\33')
    goto over;			/*  to escape casting a spell   */
  if ((b = ttgetch ()) == '\33')
    goto over;			/*  to escape casting a spell   */
  if ((d = ttgetch ()) == '\33')
    {
    over:
      lprcat (aborted);
      cdesc[SPELLS]++;
      return;
    }				/*  to escape casting a spell   */
#ifdef EXTRA
  cdesc[SPELLSCAST]++;
#endif
  for (lprc ('\n'), j = -1, i = 0; i < SPNUM; i++)	/*seq search for his spell, hash? */
    if ((spelcode[i][0] == a) && (spelcode[i][1] == b)
	&& (spelcode[i][2] == d))
      if (spelknow[i])
	{
	  speldamage (i);
	  j = 1;
	  i = SPNUM;
	}

  if (j == -1)
    lprcat ("  Nothing Happened ");
  bottomline ();
}



/*
*  speldamage(x)       Function to perform spell functions cast by the player
*      int x;
*
*  Enter with the spell number, returns no value.
*  Please insure that there are 2 spaces before all messages here
*/
static void
speldamage (int x)
{
  int i, j, clev;
  int xl, xh, yl, yh;
  int *kn, *pm, *p;

  if (x >= SPNUM)
    return;			/* no such spell */
  if (cdesc[TIMESTOP])
    {
      lprcat ("  It didn't seem to work");
      return;
    }				/* not if time stopped */
  clev = cdesc[LEVEL];
  if ((rnd (23) == 7) || (rnd (18) > cdesc[INTELLIGENCE]))
    {
      lprcat ("  It didn't work!");
      return;
    }
  if (clev * 3 + 2 < x)
    {
      lprcat ("  Nothing happens.  You seem inexperienced at this");
      return;
    }

  switch (x)
    {
      /* ----- LEVEL 1 SPELLS ----- */

    case 0:
      if (cdesc[PROTECTIONTIME] == 0)
	cdesc[MOREDEFENSES] += 2;	/* protection field +2 */
      cdesc[PROTECTIONTIME] += 250;
      return;

    case 1:
      i = rnd (((clev + 1) << 1)) + clev + 3;
      godirect (x, i, (clev >= 2) ? "  Your missiles hit the %s" : "  Your missile hit the %s", 100, '+');	/* magic missile */

      return;

    case 2:
      if (cdesc[DEXCOUNT] == 0)
	cdesc[DEXTERITY] += 3;	/* dexterity   */
      cdesc[DEXCOUNT] += 400;
      return;

	/*Further fixes below for issue #36.  Removed crusty old 'C' and replaced with
		direct function calls. -Gibbon*/
    case 3:
      i = rnd (3) + 1;
      /*Fix for bug #24 added newlines to the 'msg' for web and sleep spells.
      Removed the msg and used lprcat instead plus color. ~Gibbon*/
      direct (x, fullhit (i), "\nwhile the %s slept, you hit %d times ", i);	/*    sleep   */
      return;

    case 4:			/*  charm monster   */
      cdesc[CHARMCOUNT] += cdesc[CHARISMA] << 1;
      return;

    case 5:
      godirect (x, rnd (10) + 15 + clev, "  The sound damages the %s", 70, '@');	/* sonic spear */
      return;


      /* ----- LEVEL 2 SPELLS ----- */

    case 6:
      i = rnd (3) + 2;
      direct (x, fullhit (i),"\nYou damage the %s and hit %d times ", i);			/* web */
	  return;

    case 7:
      if (cdesc[STRCOUNT] == 0)
	cdesc[STREXTRA] += 3;	/*  strength    */
      cdesc[STRCOUNT] += 150 + rnd (100);
      return;

    case 8:
      yl = playery - 5;		/* enlightenment */
      yh = playery + 6;
      xl = playerx - 15;
      xh = playerx + 16;
      vxy (&xl, &yl);
      vxy (&xh, &yh);		/* check bounds */
      for (i = yl; i <= yh; i++)	/* enlightenment */
	for (j = xl; j <= xh; j++)
	  know[j][i] = KNOWALL;
      draws (xl, xh + 1, yl, yh + 1);
      return;

    case 9:
      raisehp (20 + (clev << 1));
      return;			/* healing */

    case 10:
      cdesc[BLINDCOUNT] = 0;
      return;			/* cure blindness   */

    case 11:
      createmonster (makemonst (level + 1) + 8);
      return;

    case 12:
      if (rnd (11) + 7 <= cdesc[WISDOM])
	direct (x, rnd (20) + 20 + clev, "\nThe %s believed!", 0);
      else
	lprcat ("\n It didn't believe the illusions!");
      return;

    case 13:			/* if he has the amulet of invisibility then add more time */
      for (j = i = 0; i < 26; i++)
	if (iven[i] == OAMULET)
	  j += 1 + ivenarg[i];
      cdesc[INVISIBILITY] += (j << 7) + 12;
      return;

      /* ----- LEVEL 3 SPELLS ----- */

    case 14:
      godirect (x, rnd (25 + clev) + 25 + clev, "\nThe fireball hits the %s",
		40, '*');
      return;			/*    fireball */

    case 15:
      godirect (x, rnd (25) + 20 + clev, "\nYour cone of cold strikes the %s", 60, 'O');	/*  cold */
      return;

    case 16:
      dirpoly (x);
      return;			/*  polymorph */

    case 17:
      cdesc[CANCELLATION] += 5 + clev;
      return;			/*  cancellation    */

    case 18:
      cdesc[HASTESELF] += 7 + clev;
      return;			/* haste self  */

    case 19:
      omnidirect (x, 30 + rnd (10), "\nThe %s gasps for air");	/* cloud kill */
      return;

    case 20:
      xh = min (playerx + 1, MAXX - 2);
      yh = min (playery + 1, MAXY - 2);
      for (i = max (playerx - 1, 1); i <= xh; i++)	/* vaporize rock */
	for (j = max (playery - 1, 1); j <= yh; j++)
	  {
	    kn = &know[i][j];
	    pm = &mitem[i][j];
	    switch (*(p = &item[i][j]))
	      {
	      case OWALL:
		if (level < MAXLEVEL + MAXVLEVEL - 1)
		  *p = *kn = 0;
		break;

	      case OSTATUE:
		if (cdesc[HARDGAME] < 3)
		  {
		    *p = OBOOK;
		    iarg[i][j] = level;
		    *kn = 0;
		  }
		break;

	      case OTHRONE:
		*p = OTHRONE2;
		create_guardian (GNOMEKING, i, j);
		break;

	      case OALTAR:
		create_guardian (DEMONPRINCE, i, j);
		break;

	      case OFOUNTAIN:
		create_guardian (WATERLORD, i, j);
		break;
	      };
	    switch (*pm)
	      {
	      case XORN:
		ifblind (i, j);
		hitm (i, j, 200);
		break;		/* Xorn takes damage from vpr */
	      }
	  }
      return;

      /* ----- LEVEL 4 SPELLS ----- */

    case 21:
      direct (x, 100 + clev, "\nThe %s shrivels up", 0);	/* dehydration */
      return;

    case 22:
      godirect (x, rnd (25) + 20 + (clev << 1), "\nA lightning bolt hits the %s", 1, '~');	/*  lightning */
      return;

    case 23:
      i = min (cdesc[HP] - 1, cdesc[HPMAX] / 2);	/* drain life */
      direct (x, i + i, "", 0);
      cdesc[HP] -= i;
      return;

    case 24:
      if (cdesc[GLOBE] == 0)
	cdesc[MOREDEFENSES] += 10;
      cdesc[GLOBE] += 200;
      loseint ();		/* globe of invulnerability */
      return;

    case 25:
      omnidirect (x, 32 + clev, "\nThe %s struggles for air in your flood!");	/* flood */
      return;

    case 26:
      if (rnd (151) == 63)
	{
	  lprcat ("\nYour heart stopped!\n");
	  nap (NAPTIME);
	  died (270);
	  return;
	}
      if (cdesc[WISDOM] > rnd (10) + 10)
	direct (x, 2000, " \nThe %s's heart stopped", 0);	/* finger of death */
      else
	lprcat (" It didn't work");
      return;

      /* ----- LEVEL 5 SPELLS ----- */

    case 27:
      cdesc[SCAREMONST] += rnd (10) + clev;
      return;			/* scare monster */

    case 28:
      cdesc[HOLDMONST] += rnd (10) + clev;
      return;			/* hold monster */

    case 29:
      cdesc[TIMESTOP] += rnd (20) + (clev << 1);
      return;			/* time stop */

    case 30:
      tdirect (x);
      return;			/* teleport away */

    case 31:
      omnidirect (x, 35 + rnd (10) + clev, "\nThe %s cringes from the flame");	/* magic fire */
      return;

      /* ----- LEVEL 6 SPELLS ----- */

    case 32:
      if ((rnd (23) == 5) && (wizard == 0))	/* sphere of annihilation */
	{
	  lprcat ("\n You have been enveloped by the zone of nothingness!\n");
	  nap (NAPTIME);
	  died (258);
	  return;
	}
      xl = playerx;
      yl = playery;
      loseint ();
      i = dirsub (&xl, &yl);	/* get direction of sphere */
      newsphere (xl, yl, i, rnd (20) + 11);	/* make a sphere */
      return;

    case 33:
      genmonst ();
      spelknow[33] = 0;		/* genocide */
      loseint ();
      return;

    case 34:			/* summon demon */
      if (rnd (100) > 30)
	{
	  direct (x, 150, "\n The demon strikes at the %s", 0);
	  return;
	}
      if (rnd (100) > 15)
	{
	  lprcat (" Nothing seems to have happened");
	  return;
	}
      lprcat (" The");
      attron(COLOR_PAIR(2));
      lprcat(" demon ");
      attroff(COLOR_PAIR(2));
      lprcat("turned on you and vanished!");
      i = rnd (40) + 30;
      lastnum = 277;
      losehp (i);		/* must say killed by a demon */
      return;

    case 35:			/* walk through walls */
      cdesc[WTW] += rnd (10) + 5;
      return;

    case 36:			/* alter reality */
      {
	struct isave *save;	/* pointer to item save structure */
	int sc;
	sc = 0;			/* # items saved */
	save =
	  (struct isave *) malloc (sizeof (struct isave) * MAXX * MAXY * 2);
	if (save == NULL)
	  {
	    lprcat ("\n Polinneaus won't let you mess with his dungeon!");
	    return;
	  }
	for (j = 0; j < MAXY; j++)
	  for (i = 0; i < MAXX; i++)	/* save all items and monsters */
	    {
	      xl = item[i][j];
	      if (xl && xl != OWALL && xl != OANNIHILATION)
		{
		  save[sc].type = 0;
		  save[sc].id = item[i][j];
		  save[sc++].arg = iarg[i][j];
		}
	      if (mitem[i][j])
		{
		  save[sc].type = 1;
		  save[sc].id = mitem[i][j];
		  save[sc++].arg = hitp[i][j];
		}
	      item[i][j] = OWALL;
	      mitem[i][j] = 0;
	      if (wizard)
		know[i][j] = KNOWALL;
	      else
		know[i][j] = 0;
	    }
	eat (1, 1);
	if (level == 1)
	  item[33][MAXY - 1] = OENTRANCE;
	for (j = rnd (MAXY - 2), i = 1; i < MAXX - 1; i++)
	  item[i][j] = 0;
	while (sc > 0)		/* put objects back in level */
	  {
	    --sc;
	    if (save[sc].type == 0)
	      {
		int trys;
		for (trys = 100, i = j = 1; --trys > 0 && item[i][j];
		     i = rnd (MAXX - 1), j = rnd (MAXY - 1));
		if (trys)
		  {
		    item[i][j] = save[sc].id;
		    iarg[i][j] = save[sc].arg;
		  }
	      }
	    else
	      {			/* put monsters back in */
		int trys;
		for (trys = 100, i = j = 1;
		     --trys > 0 && (item[i][j] == OWALL || mitem[i][j]);
		     i = rnd (MAXX - 1), j = rnd (MAXY - 1));
		if (trys)
		  {
		    mitem[i][j] = save[sc].id;
		    hitp[i][j] = save[sc].arg;
		  }
	      }
	  }
	loseint ();
	draws (0, MAXX, 0, MAXY);
	if (wizard == 0)
	  spelknow[36] = 0;
	free ((char *) save);
	positionplayer ();
	return;
      }

    case 37:			/* permanence */
      adjtimel (-99999L);
      spelknow[37] = 0;		/* forget */
      loseint ();
      return;

    default:
      lprintf ("spell %d not available!", (int) x);
      return;
    };
}



/*
* Create a guardian for a throne/altar/fountain, as a result of the player
* using a VPR spell or pulverization scroll on it.
*
* monst = monster code for guardian
* x, y = coords of the object being guarded
*
*/
static void
create_guardian (int monst, int x, int y)
{
  int k;

  /* prevent the guardian from being created on top of the player */
  if (x == playerx && y == playery)
    {

      k = rnd (8);

      x += diroffx[k];
      y += diroffy[k];
    }

  know[x][y] = 0;
  mitem[x][y] = monst;
  hitp[x][y] = monster[monst].hitpoints;
}



/*
*  loseint()       Routine to subtract 1 from your int (intelligence) if > 3
*
*  No arguments and no return value
*/
static void
loseint (void)
{

  if (--cdesc[INTELLIGENCE] < 3)
    {

      cdesc[INTELLIGENCE] = 3;
    }
}



/*
*  isconfuse()         Routine to check to see if player is confused
*
*  This routine prints out a message saying "You can't aim your magic!"
*  returns 0 if not confused, non-zero (time remaining confused) if confused
*/
static int
isconfuse (void)
{
  if (cdesc[CONFUSE])
    {
      lprcat (" You can't aim your magic!");
    }

  return (cdesc[CONFUSE]);
}



/*
*  nospell(x,monst)    Routine to return 1 if a spell doesn't affect a monster
*      int x,monst;
*
*  Subroutine to return 1 if the spell can't affect the monster
*    otherwise returns 0
*  Enter with the spell number in x, and the monster number in monst.
*/
static int
nospell (int x, int monst)
{
  int tmp;

  if (x >= SPNUM || monst >= MAXMONST + 8 || monst < 0 || x < 0)
    return (0);			/* bad spell or monst */
  if ((tmp = spelweird[monst - 1][x]) == 0)
    return (0);
  cursors ();
  lprc ('\n');
  lprintf (spelmes[tmp], monster[monst].name);
  return (1);
}




/*
*  fullhit(xx)     Function to return full damage against a monster (aka web)
*      int xx;
*
*  Function to return hp damage to monster due to a number of full hits
*  Enter with the number of full hits being done
*/
int
fullhit (int xx)
{
  int i;

  if (xx < 0 || xx > 20)
    return (0);			/* fullhits are out of range */
  if (cdesc[GREATSWORDDEATH])
    return (10000);		/* great sword of death */
  i = xx * ((cdesc[WCLASS] >> 1) + cdesc[STRENGTH] + cdesc[STREXTRA] -
	    cdesc[HARDGAME] - 12 + cdesc[MOREDAM]);
  return ((i >= 1) ? i : xx);
}




/*
*  direct(spnum,dam,str,arg)   Routine to direct spell damage 1 square in 1 dir
*      int spnum,dam,arg;
*      char *str;
*
*  Routine to ask for a direction to a spell and then hit the monster
*  Enter with the spell number in spnum, the damage to be done in dam,
*    lprintf format string in str, and lprintf's argument in arg.
*  Returns no value.
*/
static void
direct (int spnum, int dam, char *str, int arg)
{
  int x, y;
  int m;

  /* bad arguments */
  if (spnum < 0 || spnum >= SPNUM || str == NULL)
    {

      return;
    }

  if (isconfuse ())
    {

      return;
    }

  dirsub (&x, &y);

  m = mitem[x][y];

  if (item[x][y] == OMIRROR)
    {
      if (spnum == 3)		/* sleep */
	{
	  lprcat ("You fall asleep! ");
	fool:
	  arg += 2;
	  while (arg-- > 0)
	    {
	      parse2 ();
	      nap (NAPTIME);
	    }
	  return;
	}
      else if (spnum == 6)	/* web */
	{
	  lprcat ("You get stuck in your own web! ");
	  goto fool;
	}
      else
	{
	  lastnum = 278;
	  lprintf (str, "spell caster (thats you)", (int) arg);
	  losehp (dam);
	  return;
	}
    }
  if (m == 0)
    {
      lprcat ("  There wasn't anything there!");
      return;
    }
  ifblind (x, y);
  if (nospell (spnum, m))
    {
      lasthx = x;
      lasthy = y;
      return;
    }
  lprintf (str, lastmonst, (int) arg);
  hitm (x, y, dam);
}



/*
*  godirect(spnum,dam,str,delay,cshow)     Function to perform missile attacks
*      int spnum,dam,delay;
*      char *str,cshow;
*
*  Function to hit in a direction from a missile weapon and have it keep
*  on going in that direction until its power is exhausted
*  Enter with the spell number in spnum, the power of the weapon in hp,
*    lprintf format string in str, the # of milliseconds to delay between
*    locations in delay, and the character to represent the weapon in cshow.
*  Returns no value.
*/
void
godirect (int spnum, int dam, char *str, int delay, char cshow)
{
  int *p;
  int x, y, m;
  int dx, dy;

  /* bad args */
  if (spnum < 0 || spnum >= SPNUM || str == 0 || delay < 0)
    {

      return;
    }

  if (isconfuse ())
    {

      return;
    }

  dirsub (&dx, &dy);

  x = dx;
  y = dy;

  dx = x - playerx;
  dy = y - playery;

  x = playerx;
  y = playery;

  while (dam > 0)
    {

      x += dx;
      y += dy;

      if ((x > MAXX - 1) || (y > MAXY - 1) || (x < 0) || (y < 0))
	{

	  dam = 0;

	  /* out of bounds */
	  break;
	}

      /* if energy hits player */
      if ((x == playerx) && (y == playery))
	{

	  cursors ();
	  lprcat ("\nYou are hit by your own magic!");
	  lastnum = 278;
	  losehp (dam);

	  return;
	}

      /* if not blind show effect */
      if (cdesc[BLINDCOUNT] == 0)
	{

	  cursor (x + 1, y + 1);
	  lprc (cshow);
	  nap (delay);
	  show1cell (x, y);
	}

      m = mitem[x][y];

      /* is there a monster there? */
      if (m != 0)
	{

	  ifblind (x, y);

	  if (nospell (spnum, m))
	    {

	      lasthx = x;
	      lasthy = y;

	      return;
	    }

	  cursors ();
	  lprc ('\n');
	  lprintf (str, lastmonst);
	  dam -= hitm (x, y, dam);
	  show1cell (x, y);
	  nap (NAPTIME);

	  x -= dx;
	  y -= dy;
	}
      else

	switch (*(p = &item[x][y]))
	  {

	  case OWALL:
	    cursors ();
	    lprc ('\n');
	    lprintf (str, "wall");
	    if (
		 /* enough damage? */
		 dam >= 50 + cdesc[HARDGAME] &&
		 /* not on V3 */
		 level < MAXLEVEL + MAXVLEVEL - 1 &&
		 x < MAXX - 1 && y < MAXY - 1 && x != 0 && y != 0)
	      {

		lprcat ("  The wall crumbles");
		*p = 0;
		know[x][y] = 0;
		show1cell (x, y);
	      }

	    dam = 0;
	    break;


	  case OCLOSEDDOOR:

	    cursors ();
	    lprc ('\n');
	    lprintf (str, "door");

	    if (dam >= 40)
	      {
		lprcat ("  The door is blasted apart");
		*p = 0;
		know[x][y] = 0;
		show1cell (x, y);
	      }

	    dam = 0;
	    break;

	  case OSTATUE:

	    cursors ();
	    lprc ('\n');
	    lprintf (str, "statue");

	    if (cdesc[HARDGAME] < 3)
	      if (dam > 44)
		{
		  lprcat ("  The statue crumbles");
		  *p = OBOOK;
		  iarg[x][y] = level;
		  know[x][y] = 0;
		  show1cell (x, y);
		}
	    dam = 0;
	    break;

	  case OTHRONE:
	    cursors ();
	    lprc ('\n');
	    lprintf (str, "throne");
	    if (dam > 39)
	      {
		*p = OTHRONE2;
		create_guardian (GNOMEKING, x, y);
		show1cell (x, y);
	      }
	    dam = 0;
	    break;

	  case OALTAR:
	    cursors ();
	    lprc ('\n');
	    lprintf (str, "altar");
	    if (dam > 75 - (cdesc[HARDGAME] >> 2))
	      {
		create_guardian (DEMONPRINCE, x, y);
		show1cell (x, y);
	      }
	    dam = 0;
	    break;

	  case OFOUNTAIN:
	    cursors ();
	    lprc ('\n');
	    lprintf (str, "fountain");
	    if (dam > 55)
	      {
		create_guardian (WATERLORD, x, y);
		show1cell (x, y);
	      }
	    dam = 0;
	    break;

	  case OMIRROR:
	    {
	      int bounce = FALSE, odx = dx, ody = dy;

	      /* spells may bounce directly back or off at an angle */
	      if (rnd (100) < 50)
		{

		  bounce = TRUE;
		  dx *= -1;
		}

	      if (rnd (100) < 50)
		{

		  bounce = TRUE;
		  dy *= -1;
		}

	      /* guarentee a bounce */
	      if (!bounce || (odx == dx && ody == dy))
		{

		  dx = -odx;
		  dy = -ody;
		}
	    }
	    break;

	  };

      dam -= 3 + (cdesc[HARDGAME] >> 1);
    }
}



/*
*  ifblind(x,y)    Routine to put "monster" or the monster name into lastmosnt
*      int x,y;
*
*  Subroutine to copy the word "monster" into lastmonst if the player is blind
*  Enter with the coordinates (x,y) of the monster
*  Returns no value.
*/
void
ifblind (int x, int y)
{
  char *p;

  /* verify correct x, y coordinates */
  vxy (&x, &y);

  if (cdesc[BLINDCOUNT])
    {

      lastnum = 279;
      p = "monster";

    }
  else
    {

      lastnum = mitem[x][y];
      p = monster[lastnum].name;
    }

  strcpy (lastmonst, p);
}



/*
*  tdirect(spnum)      Routine to teleport away a monster
*      int spnum;
*
*  Routine to ask for a direction to a spell and then teleport away monster
*  Enter with the spell number that wants to teleport away
*  Returns no value.
*/
static void
tdirect (int spnum)
{
  int x, y, m;

  /* bad args */
  if (spnum < 0 || spnum >= SPNUM)
    {

      return;
    }

  if (isconfuse ())
    {

      return;
    }

  dirsub (&x, &y);

  m = mitem[x][y];

  if (m == 0)
    {

      lprcat ("  There wasn't anything there!");

      return;
    }

  ifblind (x, y);

  if (nospell (spnum, m))
    {

      lasthx = x;
      lasthy = y;

      return;
    }

  fillmonst (m);

  mitem[x][y] = 0;
  know[x][y] &= ~KNOWHERE;
}



/*
*  omnidirect(sp,dam,str)   Routine to damage all monsters 1 square from player
*      int sp,dam;
*      char *str;
*
*  Routine to cast a spell and then hit the monster in all directions
*  Enter with the spell number in sp, the damage done to wach square in dam,
*    and the lprintf string to identify the spell in str.
*  Returns no value.
*/
static void
omnidirect (int spnum, int dam, char *str)
{
  int x, y, m;

  /* bad args */
  if (spnum < 0 || spnum >= SPNUM || str == 0)
    {

      return;
    }

  for (x = playerx - 1; x < playerx + 2; x++)
    {
      for (y = playery - 1; y < playery + 2; y++)
	{

	  m = mitem[x][y];

	  if (m == 0)
	    {

	      continue;
	    }

	  if (nospell (spnum, m) == 0)
	    {

	      ifblind (x, y);
	      cursors ();
	      lprc ('\n');
	      lprintf (str, lastmonst);
	      hitm (x, y, dam);
	      nap (NAPTIME);

	    }
	  else
	    {

	      lasthx = x;
	      lasthy = y;
	    }
	}
    }
}



/*
*  dirsub(x,y)      Routine to ask for direction, then modify playerx,
*                   playery for it
*      int *x,*y;
*
*  Function to ask for a direction and modify an x,y for that direction
*  Enter with the coordinate destination (x,y).
*  Returns index into diroffx[] (0-8).
*/
int
dirsub (int *x, int *y)
{
  int i;

  lprcat ("\nIn What Direction? ");

  for (i = 0;;)
    {

      switch (ttgetch ())
	{
	case 'b':
	  i++;
	case 'n':
	  i++;
	case 'y':
	  i++;
	case 'u':
	  i++;
	case 'h':
	  i++;
	case 'k':
	  i++;
	case 'l':
	  i++;
	case 'j':
	  i++;
	  goto out;
/* Added an ESC. -Gibbon */
	case '\33':
		drawscreen();
	  goto out;
	};
    }

out:
  *x = playerx + diroffx[i];
  *y = playery + diroffy[i];

  vxy (x, y);

  return i;
}



/*
*  dirpoly(spnum)      Routine to ask for a direction and polymorph a monst
*      int spnum;
*
*  Subroutine to polymorph a monster and ask for the direction its in
*  Enter with the spell number in spmun.
*  Returns no value.
*/
static void
dirpoly (int spnum)
{
  int x, y, m;

  /* bad args */
  if (spnum < 0 || spnum >= SPNUM)
    {

      return;
    }

  /* if he is confused, he can't aim his magic */
  if (isconfuse ())
    {

      return;
    }

  dirsub (&x, &y);

  if (mitem[x][y] == 0)
    {

      lprcat ("  There wasn't anything there!");

      return;
    }

  ifblind (x, y);

  if (nospell (spnum, mitem[x][y]))
    {

      lasthx = x;
      lasthy = y;

      return;
    }

  do
    {

      m = rnd (MAXMONST + 7);
      mitem[x][y] = m;

    }
  while (monster[m].genocided);

  hitp[x][y] = monster[m].hitpoints;

  /* show the new monster */
  show1cell (x, y);
}



/*
*  annihilate()    Routine to annihilate all monsters around player (playerx,playery)
*
*  Gives player experience, but no dropped objects
*
*/
void
annihilate (void)
{
  int i, j;
  int k;
  int *p;

  for (k = 0, i = playerx - 1; i <= playerx + 1; i++)
    {
      for (j = playery - 1; j <= playery + 1; j++)
	{

	  /* out of bounds */
	  if (vxy (&i, &j))
	    {

	      continue;
	    }

	  p = &mitem[i][j];

	  /* no monster here */
	  if (*p == 0)
	    {

	      continue;
	    }


	  if (*p < DEMONLORD + 2)
	    {

	      k += monster[*p].experience;

	      *p = know[i][j] &= ~KNOWHERE;

	    }
	  else
	    {

	      lprintf ("\nThe %s barely escapes being annihilated!",
		       monster[*p].name);

	      /* lose half hit points */
	      hitp[i][j] = (hitp[i][j] >> 1) + 1;
	    }
	}
    }

  if (k > 0)
    {
      lprcat ("\nYou hear loud screams of agony!");

      raiseexperience (k);
    }
}



/*
*  genmonst()      Function to ask for monster and genocide from game
*
*  This is done by setting a flag in the monster[] structure
*/
static void
genmonst (void)
{
  int i, j;

  cursors ();
  lprcat ("\nGenocide what monster? ");

  for (i = 0; !isalpha (i) && i != ' '; i = ttgetch ())
    {
      ;
    }

  lprint (i);

  /* search for the monster type */
  for (j = 0; j < MAXMONST; j++)
    {

      /* have we found it? */
      if (monstnamelist[j] == i)
	{

	  /* genocided from game */
	  monster[j].genocided = 1;

	  lprintf ("  There will be no more %s's", monster[j].name);

	  /* now wipe out monsters on this level */
	  newcavelevel (level);
	  draws (0, MAXX, 0, MAXY);
	  bot_linex ();

	  return;
	}
    }

  lprcat ("  You sense failure!");
}
