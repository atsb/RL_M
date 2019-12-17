/*
*  monster.c
*
*  createmonster(monstno)      Function to create a monster next to the player
*      int monstno;
*
*  int cgood(x,y,itm,monst)    Function to check location for emptiness
*      int x,y,itm,monst;
*
*  createitem(it,arg)          Routine to place an item next to the player
*      int it,arg;
*
*  vxy(x,y)            Routine to verify/fix (*x,*y) for being within bounds
*      int *x,*y;
*
*  hitmonster(x,y)     Function to hit a monster at the designated coordinates
*      int x,y;
*
*  hitm(x,y,amt)       Function to just hit a monster at a given coordinates
*      int x,y,amt;
*
*  hitplayer(x,y)      Function for the monster to hit the player from (x,y)
*      int x,y;
*
*  dropsomething(monst)    Function to create an object when a monster dies
*      int monst;
*
*  dropgold(amount)        Function to drop some gold around player
*      int amount;
*
*  something(level)        Function to create a random item around player
*      int level;
*
*  newobject(lev,i)        Routine to return a randomly selected new object
*      int lev,*i;
*
*  spattack(atckno,xx,yy)  Function to process special attacks from monsters
*      int atckno,xx,yy;
*
*  checkloss(x)    Routine to subtract hp from user and flag bottomline display
*      int x;
*
*/
#include <stdlib.h>
#include <ctype.h>
#include <curses.h>
#include "includes/ansiterm.h"
#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include <stdio.h>
#include "includes/display.h"
#include "includes/global.h"
#include "includes/inventory.h"
#include "includes/io.h"
#include "includes/monster.h"
#include "includes/spells.h"
#include "includes/sysdep.h"

static int cgood (int, int, int, int);

static void dropsomething (int);

static int spattack (int, int, int);

/*
*  createmonster(monstno)      Function to create a monster next to the player
*      int monstno;
*
*  Enter with the monster number (1 to MAXMONST+8)
*  Returns no value.
*/
void
createmonster (int mon)
{
  int x, y, k, i;

  if (mon < 1 || mon > MAXMONST + 8)
    {				/* check for monster number out of bounds */
      lprintf ("\ncan't createmonst(%d)\n", mon);
      nap (3000);
      return;
    }
  while (monster[mon].genocided && mon < MAXMONST)
    mon++;			/* skip genocided */
  for (k = rnd (8), i = -8; i < 0; i++, k++)
    {
      /* choose rnd direction, then try all */
      if (k > 8)
	k = 1;			/* wraparound the diroff arrays */
      x = playerx + diroffx[k];
      y = playery + diroffy[k];
      if (cgood (x, y, 0, 1))
	{			/* if we can create here */
	  mitem[x][y] = mon;
	  hitp[x][y] = monster[mon].hitpoints;
	  stealth[x][y] = 0;
	  know[x][y] &= ~KNOWHERE;
	  switch (mon)
	    {
	    case ROTHE:
	    case POLTERGEIST:
	    case VAMPIRE:
	      stealth[x][y] = 1;
	    };
	  return;
	}
    }
}



/*
*  int cgood(x,y,itm,monst)      Function to check location for emptiness
*      int x,y,itm,monst;
*
*  Routine to return TRUE if a location does not have itm or monst there
*  returns FALSE (0) otherwise
*  Enter with itm or monst TRUE or FALSE if checking it
*  Example:  if itm==TRUE check for no item at this location
*            if monst==TRUE check for no monster at this location
*  This routine will return FALSE if at a wall,door or the dungeon exit
*  on level 1
*/
static int
cgood (int x, int y, int itm, int monst)
{

  /*
   * cannot create either monster or item if:
   * - out of bounds
   * - wall
   * - closed door
   * - dungeon entrance
   */
  if (((y < 0) || (y > MAXY - 1) || (x < 0) || (x > MAXX - 1)) ||
      (item[x][y] == OWALL) ||
      (item[x][y] == OCLOSEDDOOR) ||
      ((level == 1) && (x == 33) && (y == MAXY - 1)))
    {
      return FALSE;
    }

  /* if checking for an item, return False if one there already */
  if (itm && item[x][y])
    {
      return FALSE;
    }

  /*
   * if checking for a monster, return False if one there already _or_
   * there is a pit/trap there.
   */
  if (monst)
    {
      if (mitem[x][y])
	{
	  return FALSE;
	}
      /*
       * note: not invisible traps, since monsters are
       * not affected by them.
       */
      switch (item[x][y])
	{
	case OPIT:
	case OANNIHILATION:
	case OTELEPORTER:
	case OTRAPARROW:
	case ODARTRAP:
	case OTRAPDOOR:
	  return FALSE;
	}
    }

  /* cgood! */
  return TRUE;
}



/*
*  createitem(it,arg)      Routine to place an item next to the player
*      int it,arg;
*
*  Enter with the item number and its argument (iven[], ivenarg[])
*  Returns no value, thus we don't know about createitem() failures.
*/
void
createitem (int it, int arg)
{
  int x, y, k, i;

  if (it >= MAXOBJ)
    return;			/* no such object */
  for (k = rnd (8), i = -8; i < 0; i++, k++)
    {
      /* choose rnd direction, then try all */
      if (k > 8)
	k = 1;			/* wraparound the diroff arrays */
      x = playerx + diroffx[k];
      y = playery + diroffy[k];
      if (cgood (x, y, 1, 0))
	{			/* if we can create here */
	  item[x][y] = it;
	  know[x][y] = 0;
	  iarg[x][y] = arg;
	  return;
	}
    }
}



/*
*  vxy(x,y)       Routine to verify/fix coordinates for being within bounds
*      int *x,*y;
*
*  Function to verify x & y are within the bounds for a level
*  If *x or *y is not within the absolute bounds for a level, fix them so that
*    they are on the level.
*  Returns TRUE if it was out of bounds, and the *x & *y in the calling
*  routine are affected.
*/
int
vxy (int *x, int *y)
{
  int flag = 0;

  if (*x < 0)
    {
      *x = 0;
      flag++;
    }
  if (*y < 0)
    {
      *y = 0;
      flag++;
    }
  if (*x >= MAXX)
    {
      *x = MAXX - 1;
      flag++;
    }
  if (*y >= MAXY)
    {
      *y = MAXY - 1;
      flag++;
    }
  return (flag);
}



/*
*  hitmonster(x,y)     Function to hit a monster at the designated coordinates
*      int x,y;
*
*  This routine is used for a bash & slash type attack on a monster
*  Enter with the coordinates of the monster in (x,y).
*  Returns no value.
*/
void
hitmonster (int x, int y)
{
  int tmp, monst, flag, damag = 0;

  if (cdesc[TIMESTOP])
    return;			/* not if time stopped */
  vxy (&x, &y);			/* verify coordinates are within range */
  if ((monst = mitem[x][y]) == 0)
    return;
  hit3flag = 1;
  ifblind (x, y);
  tmp = monster[monst].armorclass + cdesc[LEVEL] +
    cdesc[DEXTERITY] + cdesc[WCLASS] / 4 - 12;
  cursors ();
  if ((rnd (20) < tmp - cdesc[HARDGAME]) || (rnd (71) < 5))	/* need at least random chance to hit */
    {
      lprcat ("\nYou hit");
      flag = 1;
      damag = fullhit (1);
      if (damag < 9999)
	damag = rnd (damag) + 1;
    }
  else
    {
      lprcat ("\nYou missed");
      flag = 0;
    }
  lprcat (" the ");
  attron(COLOR_PAIR(2));
  lprcat (lastmonst);
  attroff(COLOR_PAIR(2));
  if (flag)			/* if the monster was hit */
    if ((monst == RUSTMONSTER) || (monst == DISENCHANTRESS)
	|| (monst == CUBE))
      if (cdesc[WIELD] >= 0)
	if (ivenarg[cdesc[WIELD]] > -10)
	  {
	    lprintf ("\nYour weapon is dulled by the %s", lastmonst);
	    --ivenarg[cdesc[WIELD]];

	    /* fix for dulled rings of strength, cleverness, dexterity bug. */
	    switch (iven[cdesc[WIELD]])
	      {
	      case ODEXRING:
		cdesc[DEXTERITY]--;
		break;
	      case OSTRRING:
		cdesc[STREXTRA]--;
		break;
	      case OCLEVERRING:
		cdesc[INTELLIGENCE]--;
		break;
	      }
	    /* */
	  }
  if (flag)
    hitm (x, y, damag);
  if (monst == VAMPIRE)
    if (hitp[x][y] < 25)
      {				/* vampire turns into bat */
	mitem[x][y] = BAT;
	know[x][y] = 0;
      }
}



/*
*  hitm(x,y,amt)       Function to just hit a monster at a given coordinates
*      int x,y,amt;
*
*  Returns the number of hitpoints the monster absorbed
*  This routine is used to specifically damage a monster at a location (x,y)
*  Called by hitmonster(x,y)
*/
int
hitm (int x, int y, int amt)
{
  int monst;
  int hpoints, amt2;

  vxy (&x, &y);			/* verify coordinates are within range */
  amt2 = amt;			/* save initial damage so we can return it */
  monst = mitem[x][y];
  if (cdesc[HALFDAM])
    amt >>= 1;			/* if half damage curse adjust damage points */
  if (amt <= 0)
    amt2 = amt = 1;
  lasthx = x;
  lasthy = y;
  stealth[x][y] = 1;		/* make sure hitting monst breaks stealth condition */
  cdesc[HOLDMONST] = 0;		/* hit a monster breaks hold monster spell  */
  switch (monst)
    {				/* if a dragon and orb(s) of dragon slaying   */
    case WHITEDRAGON:
    case REDDRAGON:
    case GREENDRAGON:
    case BRONZEDRAGON:
    case PLATINUMDRAGON:
    case SILVERDRAGON:
      amt *= 1 + (cdesc[SLAYING] << 1);
      break;
    }
  /* invincible monster fix is here */
  if (hitp[x][y] > monster[monst].hitpoints)
    hitp[x][y] = monster[monst].hitpoints;
  if ((hpoints = hitp[x][y]) <= amt)
    {
#ifdef EXTRA
      cdesc[MONSTKILLED]++;
#endif
      lprintf ("\nThe");
      attron(COLOR_PAIR(2));
      lprintf(" %s ",lastmonst);
      attroff(COLOR_PAIR(2));
      lprintf("died!\n");
      raiseexperience (monster[monst].experience);
      amt = monster[monst].gold;
      if (amt > 0)
	dropgold (rnd (amt) + amt);
      dropsomething (monst);
      disappear (x, y);
      bottomline ();
      return (hpoints);
    }
  hitp[x][y] = hpoints - amt;
  return (amt2);
}


/*
*  hitplayer(x,y)      Function for the monster to hit the player from (x,y)
*      int x,y;
*
*  Function for the monster to hit the player with monster at location x,y
*  Returns nothing of value.
*/
void
hitplayer (int x, int y)
{
  int dam, tmp, mster, bias;

  vxy (&x, &y);			/* verify coordinates are within range */
  lastnum = mster = mitem[x][y];
  /*  spirit naga's and poltergeist's do nothing if scarab of negate spirit   */
  if (cdesc[NEGATESPIRIT] || cdesc[SPIRITPRO])
    if ((mster == POLTERGEIST) || (mster == SPIRITNAGA))
      return;
  /*  if undead and cube of undead control    */
  if (cdesc[CUBEofUNDEAD] || cdesc[UNDEADPRO])
    if ((mster == VAMPIRE) || (mster == WRAITH) || (mster == ZOMBIE))
      return;
  if ((know[x][y] & KNOWHERE) == 0)
    show1cell (x, y);
  bias = (cdesc[HARDGAME]) + 1;
  hitflag = hit2flag = hit3flag = 1;
  yrepcount = 0;
  cursors ();
  ifblind (x, y);
  if (cdesc[INVISIBILITY])
    if (rnd (33) < 20)
      {
	lprintf ("\nThe");
	attron(COLOR_PAIR(2));
	lprintf(" %s ",lastmonst);
	attroff(COLOR_PAIR(2));
	lprintf("misses wildly\n");
	return;
      }
  if (cdesc[CHARMCOUNT])
    if (rnd (30) + 5 * monster[mster].level - cdesc[CHARISMA] < 30)
      {
	lprintf ("\nThe");
	attron(COLOR_PAIR(2));
	lprintf(" %s ",lastmonst);
	attroff(COLOR_PAIR(2));
	lprintf("is awestruck at your magnificence!\n");
	return;
      }
  if (mster == BAT)
    dam = 1;
  else
    {
      dam = monster[mster].damage;
      dam += rnd ((int) ((dam < 1) ? 1 : dam)) + monster[mster].level;
    }
  tmp = 0;
  if (monster[mster].attack > 0)
    if (((dam + bias + 8) > cdesc[AC])
	|| (rnd ((int) ((cdesc[AC] > 0) ? cdesc[AC] : 1)) == 1))
      {
	if (spattack (monster[mster].attack, x, y))
	  {
#if defined WINDOWS || WINDOWS_VS
lflushall();
#endif

#if defined NIX
fflush(NULL);
#endif
	    return;
	  }
	tmp = 1;
	bias -= 2;
	cursors ();
      }
  if (((dam + bias) > cdesc[AC])
      || (rnd ((int) ((cdesc[AC] > 0) ? cdesc[AC] : 1)) == 1))
    {
      lprintf ("\nThe");
      attron(COLOR_PAIR(2));
      lprintf(" %s ",lastmonst);
      attroff(COLOR_PAIR(2));
      lprintf("hit you");
      tmp = 1;
      if ((dam -= cdesc[AC]) < 0)
	dam = 0;
      if (dam > 0)
	{
	  losehp (dam);
	  bottomhp ();
#if defined WINDOWS || WINDOWS_VS
lflushall();
#endif

#if defined NIX
fflush(NULL);
#endif
	}
    }
  if (tmp == 0)
  {
  	lprintf("\nThe");
    attron(COLOR_PAIR(2));
    lprintf(" %s ",lastmonst);
    attroff(COLOR_PAIR(2));
    lprintf("missed");
  }
}

/*
*  dropsomething(monst)    Function to create an object when a monster dies
*      int monst;
*
*  Function to create an object near the player when certain monsters are killed
*  Enter with the monster number
*  Returns nothing of value.
*/
static void
dropsomething (int monst)
{

  switch (monst)
    {
    case ORC:
    case NYMPH:
    case ELF:
    case TROGLODYTE:
    case TROLL:
    case ROTHE:
    case VIOLETFUNGI:
    case PLATINUMDRAGON:
    case GNOMEKING:
    case REDDRAGON:
      something (level);
      return;

    case LEPRECHAUN:
      if (rnd (101) >= 75)
	creategem ();
      if (rnd (5) == 1)
	dropsomething (LEPRECHAUN);
      return;
    }
}




/*
*  dropgold(amount)    Function to drop some gold around player
*      int amount;
*
*  Enter with the number of gold pieces to drop
*  Returns nothing of value.
*/
void
dropgold (int amount)
{

  if (amount > 250)
    createitem (OMAXGOLD, amount / 100);
  else
    createitem (OGOLDPILE, amount);
}



/*
*  something(level)    Function to create a random item around player
*      int level;
*
*  Function to create an item from a designed probability around player
*  Enter with the cave level on which something is to be dropped
*  Returns nothing of value.
*/
void
something (int lv)
{
  int j, i;

  if (lv < 0 || lv > MAXLEVEL + MAXVLEVEL)
    return;			/* correct level? */
  if (rnd (101) < 8)
    something (lv);		/* possibly more than one item */
  j = newobject (lv, &i);
  createitem (j, i);
}



/*
*  newobject(lev,i)    Routine to return a randomly selected new object
*      int lev,*i;
*
*  Routine to return a randomly selected object to be created
*  Returns the object number created, and sets *i for its argument
*  Enter with the cave level and a pointer to the items arg
*/
static int nobjtab[] = { 0, OSCROLL, OSCROLL, OSCROLL, OSCROLL, OPOTION,
  OPOTION, OPOTION, OPOTION, OGOLDPILE, OGOLDPILE, OGOLDPILE, OGOLDPILE,
  OBOOK, OBOOK, OBOOK, OBOOK, ODAGGER, ODAGGER, ODAGGER, OLEATHER, OLEATHER,
  OLEATHER, OREGENRING, OPROTRING, OENERGYRING, ODEXRING, OSTRRING, OSPEAR,
  OBELT, ORING, OSTUDLEATHER, OSHIELD, OCOOKIE, OCHAIN, OBATTLEAXE,
  OSPLINT, O2SWORD, OHSWORD, OCLEVERRING, OPLATE, OLONGSWORD
};


int
newobject (int lev, int *i)
{
  int tmp = 33, j, hacktmp = 0;

  if (level < 0 || level > MAXLEVEL + MAXVLEVEL)
    return (0);			/* correct level? */
  if (lev > 6)
    tmp = 41;
  else if (lev > 4)
    tmp = 39;
  j = nobjtab[tmp = rnd (tmp)];	/* the object type */
  switch (tmp)
    {
    case 1:
    case 2:
    case 3:
    case 4:			/* scroll */
      *i = newscroll ();
      break;
    case 5:
    case 6:
    case 7:
    case 8:			/* potion */
      *i = newpotion ();
      break;
    case 9:
    case 10:
    case 11:
    case 12:			/* gold */
      *i = rnd ((lev + 1) * 10) + lev * 10 + 10;
      break;
    case 13:
    case 14:
    case 15:
    case 16:			/* book */
      *i = lev;
      break;
    case 17:
    case 18:
    case 19:			/* dagger */
      hacktmp = (*i = newdagger ());
      if (!hacktmp)
	return (0);
      break;
    case 20:
    case 21:
    case 22:			/* leather armor */
      hacktmp = (*i = newleather ());
      if (!hacktmp)
	return (0);
      break;
    case 23:
    case 32:
    case 38:			/* regen ring, shield, 2-hand sword */
      *i = rund (lev / 3 + 1);
      break;
    case 24:
    case 26:			/* prot ring, dexterity ring */
      *i = rnd (lev / 4 + 1);
      break;
    case 25:			/* energy ring */
      *i = rund (lev / 4 + 1);
      break;
    case 27:
    case 39:			/* strength ring, cleverness ring */
      *i = rnd (lev / 2 + 1);
      break;
    case 30:
    case 34:			/* ring mail, flail */
      *i = rund (lev / 2 + 1);
      break;
    case 28:
    case 36:			/* spear, battleaxe */
      *i = rund (lev / 3 + 1);
      if (*i == 0)
	return (0);
      break;
    case 29:
    case 31:
    case 37:			/* belt, studded leather, splint */
      *i = rund (lev / 2 + 1);
      if (*i == 0)
	return (0);
      break;
    case 33:			/* fortune cookie */
      *i = 0;
      break;
    case 35:			/* chain mail */
      *i = newchain ();
      break;
    case 40:			/* plate mail */
      *i = newplate ();
      break;
    case 41:			/* longsword */
      *i = newsword ();
      break;
    }
  return (j);
}



/*
*  spattack(atckno,xx,yy)  Function to process special attacks from monsters
*      int atckno,xx,yy;
*
*  Enter with the special attack number, and the coordinates (xx,yy)
*      of the monster that is special attacking
*  Returns 1 if must do a show1cell(xx,yy) upon return, 0 otherwise
*
* atckno   monster     effect
* ---------------------------------------------------
*  0   none
*  1   rust monster    eat armor
*  2   hell hound      breathe light fire
*  3   dragon          breathe fire
*  4   giant centipede weakening sing
*  5   white dragon    cold breath
*  6   wraith          drain level
*  7   waterlord       water gusher
*  8   leprechaun      steal gold
*  9   disenchantress  disenchant weapon or armor
*  10  ice lizard      hits with barbed tail
*  11  umber hulk      confusion
*  12  spirit naga     cast spells taken from special attacks
*  13  platinum dragon psionics
*  14  nymph           steal objects
*  15  bugbear         bite
*  16  osequip         bite
*
*  char rustarm[ARMORTYPES][2];
*  special array for maximum rust damage to armor from rustmonster
*  format is: { armor type , minimum attribute
*/
#define ARMORTYPES 6
static int rustarm[ARMORTYPES][2] = {

  {OSTUDLEATHER, -2},
  {ORING, -4},
  {OCHAIN, -5},
  {OSPLINT, -6},
  {OPLATE, -8},
  {OPLATEARMOR, -9}

};



static char spsel[] = { 1, 2, 3, 5, 6, 8, 9, 11, 13, 14 };

static int
spattack (int x, int xx, int yy)
{
  int i, j = 0, k, m;
  char *p = 0;

  if (cdesc[CANCELLATION])
    return (0);
  vxy (&xx, &yy);		/* verify x & y coordinates */
  switch (x)
    {
    case 1:			/* rust your armor, j=1 when rusting has occurred */
      m = k = cdesc[WEAR];

      i = cdesc[SHIELD];

      if (i != -1)
	{

	  if (--ivenarg[i] < -1)
	    {
	      ivenarg[i] = -1;
	    }
	  else
	    {
	      j = 1;
	    }
	}

      if ((j == 0) && (k != -1))
	{
	  m = iven[k];
	  for (i = 0; i < ARMORTYPES; i++)
	    if (m == rustarm[i][0])	/* find his armor in table */
	      {
		if (--ivenarg[k] < rustarm[i][1])
		  ivenarg[k] = rustarm[i][1];
		else
		  j = 1;
		break;
	      }
	}
      if (j == 0)		/* if rusting did not occur */
	switch (m)
	  {
	  case OLEATHER:
	    p = "\nThe %s hit you -- Your lucky you have leather on";
	    break;
	  case OSSPLATE:
	    p =
	      "\nThe %s hit you -- Your fortunate to have stainless steel armor!";
	    break;
	  }
      else
	{
	  p = "\nThe %s hit you -- your armor feels weaker";
	}
      break;

    case 2:
      i = rnd (15) + 8 - cdesc[AC];
    spout:p = "\nThe %s breathes fire at you!";
      if (cdesc[FIRERESISTANCE])
	p = "\nThe %s's flame doesn't phase you!";
      else
    spout2:if (p)
	{
	  lprintf (p, lastmonst);
	}
      checkloss (i);
      return (0);

    case 3:
      i = rnd (20) + 25 - cdesc[AC];
      goto spout;

    case 4:
      if (cdesc[STRENGTH] > 3)
	{
	  p = "\nThe %s stung you!  You feel weaker";
	  --cdesc[STRENGTH];
	}
      else
	p = "\nThe %s stung you!";
      break;

    case 5:
      p = "\nThe %s blasts you with his cold breath";
      i = rnd (15) + 18 - cdesc[AC];
      goto spout2;

    case 6:
      lprintf ("\nThe ");
      attron(COLOR_PAIR(2));
      lprintf("%s",lastmonst);
      attroff(COLOR_PAIR(2));
      lprintf(" drains you of your life energy!");
      loselevel ();
      return (0);

    case 7:
      p = "\nThe %s got you with a gusher!";
      i = rnd (15) + 25 - cdesc[AC];
      goto spout2;

    case 8:
      if (cdesc[NOTHEFT])
	return (0);		/* he has a device of no theft */
      if (cdesc[GOLD])
	{
	  p = "\nThe %s hit you -- Your purse feels lighter";
	  if (cdesc[GOLD] > 32767)
	    cdesc[GOLD] >>= 1;
	  else
	    cdesc[GOLD] -= rnd ((int) (1 + (cdesc[GOLD] >> 1)));
	  if (cdesc[GOLD] < 0)
	    cdesc[GOLD] = 0;
	}
      else
	p = "\nThe %s couldn't find any gold to steal";
      lprintf (p, lastmonst);
      disappear (xx, yy);
      bottomgold ();
      return (1);

    case 9:
      for (j = 50;;)		/* disenchant */
	{
	  i = rund (26);
	  m = iven[i];		/* randomly select item */
	  if (m > 0 && ivenarg[i] > 0 && m != OSCROLL && m != OPOTION)
	    {
	      if ((ivenarg[i] -= 3) < 0)
		ivenarg[i] = 0;
	      lprintf ("\nThe ");
      	  attron(COLOR_PAIR(2));
	      lprintf("%s ",lastmonst);
	      attroff(COLOR_PAIR(2));
	      lprintf("hits you -- you feel a sense of loss");
	      show3 (i);
	      bottomline ();
	      return (0);
	    }
	  if (--j <= 0)
	    {
	      p = "\nThe %s nearly misses";
	      break;
	    }
	  break;
	}
      break;

    case 10:
      p = "\nThe %s hit you with his barbed tail";
      i = rnd (25) - cdesc[AC];
      goto spout2;

    case 11:
      p = "\nThe %s has confused you";
      cdesc[CONFUSE] += 10 + rnd (10);
      break;

    case 12:			/*  performs any number of other special attacks    */
      return (spattack (spsel[rund (10)], xx, yy));

    case 13:
      p = "\nThe %s flattens you with his psionics!";
      i = rnd (15) + 30 - cdesc[AC];
      goto spout2;

    case 14:
      if (cdesc[NOTHEFT])
	return (0);		/* he has device of no theft */
      if (emptyhanded () == 1)
	{
	  p = "\nThe %s couldn't find anything to steal";
	  break;
	}
      lprintf ("\nThe");
      attron(COLOR_PAIR(2));
      lprintf(" %s ",lastmonst);
      attroff(COLOR_PAIR(2));
      lprintf("picks your pocket and takes:");
      if (stealsomething () == 0)
	lprcat (" nothing");
      disappear (xx, yy);
      bottomline ();
      return (1);

    case 15:
      i = rnd (10) + 5 - cdesc[AC];
    spout3:p = "\nThe %s bit you!";
      goto spout2;

    case 16:
      i = rnd (15) + 10 - cdesc[AC];
      goto spout3;
    };
  if (p)
    {
      lprintf (p, lastmonst);
      bottomline ();
    }

  return (0);
}



/*
*  checkloss(x)    Routine to subtract hp from user and flag bottomline display
*      int x;
*
*  Routine to subtract hitpoints from the user and flag the bottomline display
*  Enter with the number of hit points to lose
*  Note: if x > cdesc[HP] this routine could kill the player!
*/
void
checkloss (int x)
{

  if (x > 0)
    {

      losehp (x);
      bottomhp ();
    }
}
