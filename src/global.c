/*  global.c
*
*  raiselevel()        subroutine to raise the player one level
*  loselevel()         subroutine to lower the player by one level
*  raiseexperience(x)  subroutine to increase experience points
*  loseexperience(x)   subroutine to lose experience points
*  losehp(x)           subroutine to remove hit points from the player
*  losemhp(x)          subroutine to remove max # hit points from the player
*  raisehp(x)          subroutine to gain hit points
*  raisemhp(x)         subroutine to gain maximum hit points
*  losemspells(x)      subroutine to lose maximum spells
*  raisemspells(x)     subroutine to gain maximum spells
*  makemonst(lev)      function to return monster number for a randomly selected monster
*  positionplayer()    function to be sure player is not in a wall
*  recalc()            function to recalculate the armor class of the player
*  quit()              subroutine to ask if the player really wants to quit
*  more()
*  take()
*  drop_object()
*  enchantarmor()
*  enchweapon()
*  pocketfull()
*  nearbymonst()
*  stealsomething()
*  emptyhanded()
*  creategem()
*  adjustcvalues()
*  gettokstr()
*  getpassword()
*  getyn()
*  packweight()
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/display.h"
#include "includes/global.h"
#include "includes/inventory.h"
#include "includes/io.h"
#include "includes/monster.h"
#include "includes/scores.h"
#include "includes/sysdep.h"

/*
* raiselevel()
*
* subroutine to raise the player one level
* uses the skill[] array to find level boundarys
* uses cdesc[EXPERIENCE]  cdesc[LEVEL]
*/
void
raiselevel (void)
{

  if (cdesc[LEVEL] < MAXPLEVEL)
    raiseexperience ((skill[cdesc[LEVEL]] - cdesc[EXPERIENCE]));
}



/*
* loselevel()
* 
* subroutine to lower the players character level by one
*/
void
loselevel (void)
{
  if (cdesc[LEVEL] > 1)
    loseexperience ((cdesc[EXPERIENCE] - skill[cdesc[LEVEL] - 1] + 1));
}



/*
* raiseexperience(x)
*
* subroutine to increase experience points
*/
void
raiseexperience (long x)
{
  int i, tmp;

  i = cdesc[LEVEL];
  cdesc[EXPERIENCE] += x;
  while (cdesc[EXPERIENCE] >= skill[cdesc[LEVEL]]
	 && (cdesc[LEVEL] < MAXPLEVEL))
    {
      tmp = (cdesc[CONSTITUTION] - cdesc[HARDGAME]) >> 1;
      cdesc[LEVEL]++;
      raisemhp ((int) (rnd (3) + rnd ((tmp > 0) ? tmp : 1)));
      raisemspells ((int) rund (3));
      if (cdesc[LEVEL] < 7 - cdesc[HARDGAME])
	raisemhp ((int) (cdesc[CONSTITUTION] >> 2));
    }
  if (cdesc[LEVEL] != i)
    {
      cursors ();
      lprintf ("\nWelcome to level %d", (int) cdesc[LEVEL]);	/* if we changed levels */
    }
  bottomline ();
}



/*
* loseexperience(x)
*
* subroutine to lose experience points
*/
void
loseexperience (long x)
{
  int i, tmp;

  i = cdesc[LEVEL];
  cdesc[EXPERIENCE] -= x;
  if (cdesc[EXPERIENCE] < 0)
    cdesc[EXPERIENCE] = 0;
  while (cdesc[EXPERIENCE] < skill[cdesc[LEVEL] - 1])
    {
      if (--cdesc[LEVEL] <= 1)
	cdesc[LEVEL] = 1;	/*  down one level      */
      tmp = (cdesc[CONSTITUTION] - cdesc[HARDGAME]) >> 1;	/* lose hpoints */
      losemhp ((int) rnd ((tmp > 0) ? tmp : 1));	/* lose hpoints */
      if (cdesc[LEVEL] < 7 - cdesc[HARDGAME])
	losemhp ((int) (cdesc[CONSTITUTION] >> 2));
      losemspells ((int) rund (3));	/*  lose spells     */
    }
  if (i != cdesc[LEVEL])
    {
      cursors ();
      lprintf ("\nYou went down to level %d!", (int) cdesc[LEVEL]);
    }
  bottomline ();
}



/*
*  losehp(x)
*  losemhp(x)
*
*  subroutine to remove hit points from the player
*  warning -- will kill player if hp goes to zero
*/
void
losehp (int x)
{
  if ((cdesc[HP] -= x) <= 0)
    {
      lprcat ("\nYou have been slain.");
      nap (NAPTIME);
      died (lastnum);
    }
}

void
losemhp (int x)
{
  cdesc[HP] -= x;
  if (cdesc[HP] < 1)
    cdesc[HP] = 1;
  cdesc[HPMAX] -= x;
  if (cdesc[HPMAX] < 1)
    cdesc[HPMAX] = 1;
}



/*
*  raisehp(x)
*  raisemhp(x)
*
*  subroutine to gain maximum hit points
*/
void
raisehp (int x)
{

  if ((cdesc[HP] += x) > cdesc[HPMAX])
    {

      cdesc[HP] = cdesc[HPMAX];
    }
}

void
raisemhp (int x)
{

  cdesc[HPMAX] += x;
  cdesc[HP] += x;
}


/*
* raisemspells(x)
*
* subroutine to gain maximum spells
*/
void
raisemspells (int x)
{

  cdesc[SPELLMAX] += x;
  cdesc[SPELLS] += x;
}


/*
* losemspells(x)
*
*  subroutine to lose maximum spells
*/
void
losemspells (int x)
{

  if ((cdesc[SPELLMAX] -= x) < 0)
    cdesc[SPELLMAX] = 0;
  if ((cdesc[SPELLS] -= x) < 0)
    cdesc[SPELLS] = 0;
}



/*
 *   makemonst(lev)
 *       int lev;
 *
 * function to return monster number for a randomly selected monster
 * for the given cave level
 * 
 * FIXED by ~Gibbon.  Randomness was removed prior to 12.4alpha2 for some reason, so
 * monsters were being taken based on the int of monstlevel (5,11 etc).
 * 
 * Basically unchanged from 12.3 but cleaned up.  Don't fix what isn't broken.
 */
int
makemonst (int lev)
{
  int x, tmp;
  if (lev < 1)
    {
      lev = 1;
    }
  else if (lev > 12)
    {
      lev = 12;
    }
  if (lev < 5)
    {
      tmp = rnd ((x = monstlevel[lev - 1]) ? x : 1);
    }
  else
    {
      tmp =
	rnd ((x =
	      monstlevel[lev - 1] - monstlevel[lev - 4]) ? x : 1) +
	monstlevel[lev - 4];
    }
  while (monster[tmp].genocided && tmp < MAXMONST)
    {
      tmp++;			/* genocided? */
    }
  return tmp;
}

/*
* positionplayer()
*
* Insure player is not in a wall or on top of a monster.  Could be more
* intelligent about what kinds of objects the player can land on.
*/
void
positionplayer (void)
{
  int z, try = 2;

  /* set the previous player x,y position to the new one, so that
     clearing the player indicator from the previous location will
     not do the wrong thing.
   */
  oldx = playerx;
  oldy = playery;

  /* short-circuit the testing if current position empty
   */
  if (!item[playerx][playery] && !mitem[playerx][playery])
    return;

  /* make at most two complete passes across the dungeon, looking
     for a clear space.  In most situations, should find a clear
     spot right around the current player position.
   */
  do
    {

      /* check all around the player position for a clear space.
       */
      for (z = 1; z < 9; z++)
	{
	  int tmpx = playerx + diroffx[z];
	  int tmpy = playery + diroffy[z];
	  if (!item[tmpx][tmpy] && !mitem[tmpx][tmpy])
	    {
	      playerx = tmpx;
	      playery = tmpy;
	      return;
	    }
	}

      /* no clear spots around the player. try another position,
         wrapping around the dungeon.
       */
      if (++playerx >= MAXX - 1)
	{
	  playerx = 1;
	  if (++playery >= MAXY - 1)
	    {
	      playery = 1;
	      try--;
	    }
	}
    }
  while (try);

  /* no spot found.
   */
  lprcat ("Failure in positionplayer\n");
}



/*
* recalc()    function to recalculate the armor class of the player
*/
void
recalc (void)
{
  int i, j, k;

  cdesc[AC] = cdesc[MOREDEFENSES];
  if (cdesc[WEAR] >= 0)
    switch (iven[cdesc[WEAR]])
      {
      case OSHIELD:
	cdesc[AC] += 2 + ivenarg[cdesc[WEAR]];
	break;
      case OLEATHER:
	cdesc[AC] += 2 + ivenarg[cdesc[WEAR]];
	break;
      case OSTUDLEATHER:
	cdesc[AC] += 3 + ivenarg[cdesc[WEAR]];
	break;
      case ORING:
	cdesc[AC] += 5 + ivenarg[cdesc[WEAR]];
	break;
      case OCHAIN:
	cdesc[AC] += 6 + ivenarg[cdesc[WEAR]];
	break;
      case OSPLINT:
	cdesc[AC] += 7 + ivenarg[cdesc[WEAR]];
	break;
      case OPLATE:
	cdesc[AC] += 9 + ivenarg[cdesc[WEAR]];
	break;
      case OPLATEARMOR:
	cdesc[AC] += 10 + ivenarg[cdesc[WEAR]];
	break;
      case OSSPLATE:
	cdesc[AC] += 12 + ivenarg[cdesc[WEAR]];
	break;
      }

  if (cdesc[SHIELD] >= 0)
    if (iven[cdesc[SHIELD]] == OSHIELD)
      cdesc[AC] += 2 + ivenarg[cdesc[SHIELD]];
  if (cdesc[WIELD] < 0)
    cdesc[WCLASS] = 0;
  else
    {
      i = ivenarg[cdesc[WIELD]];
      switch (iven[cdesc[WIELD]])
	{
	case ODAGGER:
	  cdesc[WCLASS] = 3 + i;
	  break;
	case OBELT:
	  cdesc[WCLASS] = 7 + i;
	  break;
	case OSHIELD:
	  cdesc[WCLASS] = 8 + i;
	  break;
	case OSPEAR:
	  cdesc[WCLASS] = 10 + i;
	  break;
	case OBATTLEAXE:
	  cdesc[WCLASS] = 17 + i;
	  break;
	case OGREATSWORD:
	  cdesc[WCLASS] = 19 + i;
	  break;
	case OLONGSWORD:
	  cdesc[WCLASS] = 22 + i;
	  break;
	case O2SWORD:
	  cdesc[WCLASS] = 26 + i;
	  break;
	case OHSWORD:
	  cdesc[WCLASS] = 25 + i;
	  break;
	case OSWORD:
	  cdesc[WCLASS] = 32 + i;
	  break;
	case OSWORDofSLASHING:
	  cdesc[WCLASS] = 30 + i;
	  break;
	case OHAMMER:
	  cdesc[WCLASS] = 35 + i;
	  break;
	default:
	  cdesc[WCLASS] = 0;
	}
    }
  cdesc[WCLASS] += cdesc[MOREDAM];

  /*  now for regeneration abilities based on rings   */
  cdesc[REGEN] = 1;
  cdesc[ENERGY] = 0;
  j = 0;
  for (k = 25; k > 0; k--)
    if (iven[k])
      {
	j = k;
	k = 0;
      }
  for (i = 0; i <= j; i++)
    {
      switch (iven[i])
	{
	case OPROTRING:
	  cdesc[AC] += ivenarg[i] + 1;
	  break;
	case ODAMRING:
	  cdesc[WCLASS] += ivenarg[i] + 1;
	  break;
	case OBELT:
	  cdesc[WCLASS] += ((ivenarg[i] << 1)) + 2;
	  break;

	case OREGENRING:
	  cdesc[REGEN] += ivenarg[i] + 1;
	  break;
	case ORINGOFEXTRA:
	  cdesc[REGEN] += 5 * (ivenarg[i] + 1);
	  break;
	case OENERGYRING:
	  cdesc[ENERGY] += ivenarg[i] + 1;
	  break;
	}
    }
}


/*
* quit()
*
* subroutine to ask if the player really wants to quit
*/
void
quit (void)
{
  int i;

  cursors ();
  strcpy (lastmonst, "");
  lprcat ("\nDo you really want to quit (all progress will be lost)?");
  for (;;)
    {
      i = ttgetch ();
      if ((i == 'y') || (i == 'Y'))
	{
	  died (300);
	  return;
	}
      if ((i == 'n') || (i == 'N') || (i == '\33'))
	{
	  lprcat (" no");
	  lflush ();
	  return;
	}
    }
}



/*
* function to ask --more--. If the user enters a space, returns 0.  If user
* enters Escape, returns 1.  If user enters alphabetic, then returns that
*  value.
*/
int
more (char select_allowed)
{
  int i;

  lprcat ("\n  --- press ");
  lstandout ("space");
  lprcat (" to continue --- ");
  if (select_allowed)
    lprcat ("letter to select --- ");

  for (;;)
    {
      if ((i = ttgetch ()) == ' ' || i == '\n')
	return 0;
      if (i == '\x1B')
	return 1;
      if (select_allowed)
	{
	  if (isupper (i))
	    i = tolower (i);
	  if ((i >= 'a' && i <= 'z') || i == '.')
	    return i;
	}
    }
}



/*
* function to enchant armor player is currently wearing
*/
void
enchantarmor (void)
{
  int tmp;

  if (cdesc[WEAR] < 0)
    {
      if (cdesc[SHIELD] < 0)
	{
	  cursors ();
	  lprcat ("\nYou feel a sense of loss");
	  return;
	}
      else
	{
	  tmp = iven[cdesc[SHIELD]];
	  if (tmp != OSCROLL)
	    if (tmp != OPOTION)
	      {
		ivenarg[cdesc[SHIELD]]++;
		bottomline ();
	      }
	}
    }
  tmp = iven[cdesc[WEAR]];
  if (tmp != OSCROLL)
    if (tmp != OPOTION)
      {
	ivenarg[cdesc[WEAR]]++;
	bottomline ();
      }
}



/*
* function to enchant a weapon presently being wielded
*/
void
enchweapon (void)
{
  int tmp;

  if (cdesc[WIELD] < 0)
    {
      cursors ();
      lprcat ("\nYou feel a sense of loss");
      return;
    }

  tmp = iven[cdesc[WIELD]];

  if (tmp != OSCROLL)
    if (tmp != OPOTION)
      {
	ivenarg[cdesc[WIELD]]++;
	if (tmp == OCLEVERRING)
	  cdesc[INTELLIGENCE]++;
	else if (tmp == OSTRRING)
	  cdesc[STREXTRA]++;
	else if (tmp == ODEXRING)
	  cdesc[DEXTERITY]++;
	bottomline ();
      }
}



/*
* function to return 1 if a monster is next to the player else returns 0
*/
int
nearbymonst (void)
{
  int tmp, tmp2;

  for (tmp = playerx - 1; tmp < playerx + 2; tmp++)
    for (tmp2 = playery - 1; tmp2 < playery + 2; tmp2++)
      if (mitem[tmp][tmp2])
	return (1);		/* if monster nearby */

  return 0;
}



/*
* function to steal an item from the players pockets
* returns 1 if steals something else returns 0
*/
int
stealsomething (void)
{
  int i, j;

  j = 100;

  for (;;)
    {

      i = rund (26);

      if (iven[i] && cdesc[WEAR] != i &&
	  cdesc[WIELD] != i && cdesc[SHIELD] != i)
	{

	  show3 (i);
	  adjustcvalues (iven[i], ivenarg[i]);
	  iven[i] = 0;

	  return 1;
	}

      if (--j <= 0)
	{

	  return 0;
	}
    }
}



/*
* function to return 1 is player carrys nothing else return 0
*/
int
emptyhanded (void)
{
  int i;

  for (i = 0; i < 26; i++)
    {

      if (iven[i] && i != cdesc[WIELD] &&
	  i != cdesc[WEAR] && i != cdesc[SHIELD])
	{

	  return 0;
	}
    }

  return 1;
}



/*
* function to create a gem on a square near the player
*/
void
creategem (void)
{
  int i, j;

  switch (rnd (4))
    {
    case 1:
      i = ODIAMOND;
      j = 50;
      break;
    case 2:
      i = ORUBY;
      j = 40;
      break;
    case 3:
      i = OEMERALD;
      j = 30;
      break;
    default:
      i = OSAPPHIRE;
      j = 20;
      break;
    };

  createitem (i, rnd (j) + j / 10);
}



/*
* function to change character levels as needed when dropping an object
* that affects these characteristics
*/
void
adjustcvalues (int itm, int arg)
{
  int flag;

  flag = 0;

  switch (itm)
    {
    case ODEXRING:
      cdesc[DEXTERITY] -= arg + 1;
      flag = 1;
      break;
    case OSTRRING:
      cdesc[STREXTRA] -= arg + 1;
      flag = 1;
      break;
    case OCLEVERRING:
      cdesc[INTELLIGENCE] -= arg + 1;
      flag = 1;
      break;
    case OHAMMER:
      cdesc[DEXTERITY] -= 10;
      cdesc[STREXTRA] -= 10;
      cdesc[INTELLIGENCE] += 10;
      flag = 1;
      break;
    case OSWORDofSLASHING:
      cdesc[DEXTERITY] -= 5;
      flag = 1;
      break;
    case OORBOFDRAGON:
      --cdesc[SLAYING];
      return;
    case OSPIRITSCARAB:
      --cdesc[NEGATESPIRIT];
      return;
    case OCUBEofUNDEAD:
      --cdesc[CUBEofUNDEAD];
      return;
    case ONOTHEFT:
      --cdesc[NOTHEFT];
      return;
    case OGREATSWORD:
      cdesc[GREATSWORDDEATH] = 0;
      return;
    case OPOTION:
    case OSCROLL:
      return;

    default:
      flag = 1;
    };

  if (flag)
    {

      bottomline ();
    }
}

/*
* subroutine to get a yes or no response from the user
* returns y or n
*/
char
getyn (void)
{
  int i = 0;

  while (i != 'y' && i != 'n' && i != '\33')
    i = ttgetch ();

  return (char) i;
}



/*
* function to calculate the pack weight of the player
* returns the number of pounds the player is carrying
*/
int
packweight (void)
{
  int i, j = 25, k;

  k = cdesc[GOLD] / 1000;
  while ((iven[j] == 0) && (j > 0))
    --j;
  for (i = 0; i <= j; i++)
    switch (iven[i])
      {
      case 0:
	break;
      case OSSPLATE:
      case OPLATEARMOR:
	k += 40;
	break;
      case OPLATE:
	k += 35;
	break;
      case OHAMMER:
	k += 30;
	break;
      case OSPLINT:
	k += 26;
	break;
      case OSWORDofSLASHING:
      case OCHAIN:
      case OBATTLEAXE:
      case O2SWORD:
      case OHSWORD:
	k += 23;
	break;
      case OLONGSWORD:
      case OSWORD:
      case ORING:
	k += 20;
	break;
      case OGREATSWORD:
      case OSTUDLEATHER:
	k += 15;
	break;
      case OLEATHER:
      case OSPEAR:
	k += 8;
	break;
      case OORBOFDRAGON:
      case OBELT:
	k += 4;
	break;
      case OSHIELD:
	k += 7;
	break;
      case OCHEST:
	k += 30 + ivenarg[i];
	break;
      default:
	k++;
	break;
      };
  return (k);
}	
