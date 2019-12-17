/* moreobj.c

Routines in this file:

oaltar()
othrone()
odeadthrone()
ochest()
ofountain()
fntchange()
fch()
drink_fountain()
wash_fountain()
enter()
remove_gems()
sit_on_throne()
up_stairs()
down_stairs()
open_something()
close_something()
desecrate_altar()
pray_at_altar()
*/
#include <stdlib.h>
#include "includes/action.h"
#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/create.h"
#include "includes/display.h"
#include "includes/global.h"
#include "includes/inventory.h"
#include "includes/io.h"
#include "includes/moreobj.h"
#include "includes/object.h"
#include "includes/spells.h"
#include "includes/store.h"

static void fch (int, long *);
static void specify_obj_nocurs (void);
static void specify_obj_cursor (void);
static void move_cursor (int *, int *, int);




/*
* subroutine to process an altar object
*/
void
oaltar (void)
{

  lprcat ("\nDo you (p) pray  (d) desecrate");
  iopts ();
  for (;;)
    {
      switch (ttgetch ())
	{
	case 'p':
	  lprcat (" pray\nDo you (m) give money or (j) just pray? ");
	  for (;;)
	    switch (ttgetch ())
	      {
	      case 'j':
		lprcat ("\n");
		act_just_pray ();
		return;

	      case 'm':
		act_donation_pray ();
		return;

	      case '\33':
		return;
	      };

	case 'd':
	  lprcat (" desecrate");
	  act_desecrate_altar ();
	  return;

	case 'i':
	case '\33':
	  ignore ();
	  act_ignore_altar ();
	  return;
	};
    }
}



/*
subroutine to process a throne object
*/
void
othrone (int arg)
{

  lprcat ("\nDo you (p) pry off jewels, (s) sit down");
  iopts ();
  for (;;)
    {
      for (;;)
	{
	  switch (ttgetch ())
	    {
	    case 'p':
	      lprcat (" pry off");
	      act_remove_gems (arg);
	      return;

	    case 's':
	      lprcat (" sit down");
	      act_sit_throne (arg);
	      return;

	    case 'i':
	    case '\33':
	      ignore ();
	      return;
	    };
	}
    }
}



void
odeadthrone (void)
{
  lprcat ("\nDo you (s) sit down");
  iopts ();
  for (;;)
    {
      for (;;)
	{
	  switch (ttgetch ())
	    {
	    case 's':
	      lprcat (" sit down");
	      act_sit_throne (1);
	      return;

	    case 'i':
	    case '\33':
	      ignore ();
	      return;
	    };
	}
    }
}



/*
* subroutine to process a chest object
*/
void
ochest (void)
{

  lprcat ("\nDo you (t) take it, (o) try to open it");
  iopts ();
  for (;;)
    {
      switch (ttgetch ())
	{
	case 'o':
	  lprcat (" open it");
	  act_open_chest (playerx, playery);
	  return;

	case 't':
	  lprcat (" take");
	  if (take (OCHEST, iarg[playerx][playery]) == 0)
	    item[playerx][playery] = know[playerx][playery] = 0;
	  return;

	case 'i':
	case '\33':
	  ignore ();
	  return;
	};
    }
}



/*
* process a fountain object
*/
void
ofountain (void)
{
  cursors ();
  lprcat ("\nDo you (d) drink, (w) wash yourself");
  iopts ();
  for (;;)
    {
      switch (ttgetch ())
	{
	case 'd':
	  act_drink_fountain ();
	  return;

	case '\33':
	case 'i':
	  ignore ();
	  return;

	case 'w':
	  act_wash_fountain ();
	  return;
	}
    }
}



/*
* a subroutine to raise or lower character levels
* if x > 0 they are raised   if x < 0 they are lowered
*/
void
fntchange (int how)
{
  int j;

  lprc ('\n');

  switch (rnd (9))
    {
    case 1:
      lprcat ("Your strength");
      fch (how, &cdesc[STRENGTH]);
      break;
    case 2:
      lprcat ("Your intelligence");
      fch (how, &cdesc[INTELLIGENCE]);
      break;
    case 3:
      lprcat ("Your wisdom");
      fch (how, &cdesc[WISDOM]);
      break;
    case 4:
      lprcat ("Your constitution");
      fch (how, &cdesc[CONSTITUTION]);
      break;
    case 5:
      lprcat ("Your dexterity");
      fch (how, &cdesc[DEXTERITY]);
      break;
    case 6:
      lprcat ("Your charm");
      fch (how, &cdesc[CHARISMA]);
      break;
    case 7:
      j = rnd (level + 1);
      if (how < 0)
	{
	  lprintf ("You lose %d hit point", (int) j);
	  if (j > 1)
	    lprcat ("s!");
	  else
	    lprc ('!');
	  losemhp ((int) j);
	}
      else
	{
	  lprintf ("You gain %d hit point", (int) j);
	  if (j > 1)
	    lprcat ("s!");
	  else
	    lprc ('!');
	  raisemhp ((int) j);
	}
      bottomline ();
      break;

    case 8:
      j = rnd (level + 1);
      if (how > 0)
	{
	  lprintf ("You just gained %d spell", (int) j);
	  raisemspells ((int) j);
	  if (j > 1)
	    lprcat ("s!");
	  else
	    lprc ('!');
	}
      else
	{
	  lprintf ("You just lost %d spell", (int) j);
	  losemspells ((int) j);
	  if (j > 1)
	    lprcat ("s!");
	  else
	    lprc ('!');
	}
      bottomline ();
      break;

    case 9:
      j = 5 * rnd ((level + 1) * (level + 1));
      if (how < 0)
	{
	  lprintf ("You just lost %d experience point", (int) j);
	  if (j > 1)
	    lprcat ("s!");
	  else
	    lprc ('!');
	  loseexperience (j);
	}
      else
	{
	  lprintf ("You just gained %d experience point", (int) j);
	  if (j > 1)
	    lprcat ("s!");
	  else
	    lprc ('!');
	  raiseexperience (j);
	}
      break;
    }
  cursors ();
}



/*
subroutine to process an up/down of a character attribute for ofountain
*/
static void
fch (int how, long *x)
{
  if (how < 0)
    {
      lprcat (" went down by one!");
      --(*x);
    }
  else
    {
      lprcat (" went up by one!");
      (*x)++;
    }
  bottomline ();
}



/*
For command mode.  Perform drinking at a fountain.
*/
void
drink_fountain (void)
{
  cursors ();
  if (item[playerx][playery] == ODEADFOUNTAIN)
    lprcat ("\nThere is no water to drink!");

  else if (item[playerx][playery] != OFOUNTAIN)
    lprcat ("\nI see no fountain to drink from here!");

  else
    act_drink_fountain ();
  return;
}




/*
For command mode.  Perform washing (tidying up) at a fountain.
*/
void
wash_fountain (void)
{
  cursors ();
  if (item[playerx][playery] == ODEADFOUNTAIN)
    lprcat ("\nThere is no water to wash in!");

  else if (item[playerx][playery] != OFOUNTAIN)
    lprcat ("\nI see no fountain to wash at here!");

  else
    act_wash_fountain ();
  return;
}



/*
For command mode.  Perform entering a building.
*/
void
enter (void)
{
  cursors ();
  switch (item[playerx][playery])
    {
    case OSCHOOL:
      oschool ();
      break;

    case OBANK:
      obank ();
      break;

    case OBANK2:
      obank2 ();
      break;

    case ODNDSTORE:
      dndstore ();
      break;

    case OENTRANCE:
      /* place player in front of entrance on level 1.  newcavelevel()
         prevents player from landing on a monster/object.
       */
      playerx = 33;
      playery = MAXY - 2;
      newcavelevel (1);
      know[33][MAXY - 1] = KNOWALL;
      mitem[33][MAXY - 1] = 0;
      draws (0, MAXX, 0, MAXY);
      showcell (playerx, playery);	/* to show around player */
      bot_linex ();
      break;

    case OTRADEPOST:
      otradepost ();
      break;

    case OLRS:
      olrs ();
      break;

    case OHOME:
      ohome ();
      break;

    default:
      lprcat ("\nThere is no place to enter here!\n");
      break;
    }
}



/*
For command mode.  Perform removal of gems from a jeweled throne.
*/
void
remove_gems (void)
{
  cursors ();
  if (item[playerx][playery] == ODEADTHRONE)
    lprcat ("\nThere are no gems to remove!");

  else if (item[playerx][playery] == OTHRONE)
    act_remove_gems (0);

  else if (item[playerx][playery] == OTHRONE2)
    act_remove_gems (1);

  else
    lprcat ("\nI see no throne here to remove gems from!");
  return;
}



/*
For command mode.  Perform sitting on a throne.
*/
void
sit_on_throne (void)
{

  cursors ();
  if (item[playerx][playery] == OTHRONE)
    act_sit_throne (0);

  else if ((item[playerx][playery] == OTHRONE2) ||
	   (item[playerx][playery] == ODEADTHRONE))
    act_sit_throne (1);

  else
    lprcat ("\nI see no throne to sit on here!");

  return;
}



/*
For command mode.  Checks that player is actually standing at a set up
up stairs or volcanic shaft.  
*/
void
up_stairs (void)
{
  cursors ();
  if (item[playerx][playery] == OSTAIRSDOWN)
    lprcat ("\nThe stairs don't go up!");

  else if (item[playerx][playery] == OVOLUP)
    act_up_shaft ();

  else if (item[playerx][playery] != OSTAIRSUP)
    lprcat ("\nI see no way to go up here!");

  else
    act_up_stairs ();
}




/*
For command mode.  Checks that player is actually standing at a set of
down stairs or volcanic shaft.
*/
void
down_stairs (void)
{
  cursors ();
  if (item[playerx][playery] == OSTAIRSUP)
    lprcat ("\nThe stairs don't go down!");

  else if (item[playerx][playery] == OVOLDOWN)
    act_down_shaft ();

  else if (item[playerx][playery] != OSTAIRSDOWN)
    lprcat ("\nI see no way to go down here!");

  else
    act_down_stairs ();
}



/*
For command mode.  Perform opening an object (door, chest).
*/
void
open_something (void)
{
  int x, y;			/* direction to open */
  char tempc;			/* result of prompting to open a chest */

  cursors ();
  /* check for confusion.
   */
  if (cdesc[CONFUSE])
    {
      lprcat ("You're too confused!");
      return;
    }

  /* check for player standing on a chest.  If he is, prompt for and
     let him open it.  If player ESCs from prompt, quit the Open
     command.
   */
  if (item[playerx][playery] == OCHEST)
    {
      lprcat ("There is a chest here.  Open it?");
      if ((tempc = getyn ()) == 'y')
	{
	  act_open_chest (playerx, playery);
	  dropflag = 1;		/* prevent player from picking back up if fail */
	  return;
	}
      else if (tempc != 'n')
	return;
    }

  /* get direction of object to open.  test 'openability' of object
     indicated, call common command/prompt mode routines to actually open.
   */
  dirsub (&x, &y);
  switch (item[x][y])
    {
    case OOPENDOOR:
      lprcat ("The door is already open!");
      break;

    case OCHEST:
      act_open_chest (x, y);
      break;

    case OCLOSEDDOOR:
      act_open_door (x, y);
      break;
		
/* This message is rephrased to handle other scenarios. -Gibbon */
    default:
      lprcat ("\nNothing happens..");
      break;
    }
}




/*
For command mode.  Perform the action of closing something (door).
*/
void
close_something (void)
{
  int x, y;

  cursors ();
  /* check for confusion.
   */
  if (cdesc[CONFUSE])
    {
      lprcat ("You're too confused!");
      return;
    }

  /* get direction of object to close.  test 'closeability' of object
     indicated.
   */
  dirsub (&x, &y);
  switch (item[x][y])
    {
    case OCLOSEDDOOR:
      lprcat ("The door is already closed!");
      break;

    case OOPENDOOR:
      if (mitem[x][y])
	{
	  lprcat ("Theres a monster in the way!");
	  return;
	}
      item[x][y] = OCLOSEDDOOR;
      know[x][y] = 0;
      iarg[x][y] = 0;
      break;

    default:
      lprcat ("You can't close that!");
      break;
    }
}




/*
* For command mode.  Perform the act of descecrating an altar.
*/
void
desecrate_altar (void)
{
  cursors ();
  if (item[playerx][playery] == OALTAR)
    act_desecrate_altar ();
  else
    lprcat ("\nI see no altar to desecrate here!");
}





/*
For command mode.  Perform the act of praying at an altar.
*/
void
pray_at_altar (void)
{

  cursors ();
  if (item[playerx][playery] != OALTAR)
    lprcat ("\nI see no altar to pray at here!");
  else
    act_donation_pray ();
  prayed = 1;
}



/*
Identify objects for the player.
*/
void
specify_object (void)
{
  cursors ();
  lprcat ("\nIdentify unknown object by cursor [ynq]?");

  for (;;)
    {

      switch (ttgetch ())
	{
	case '\33':
	case 'q':
	  return;
	case 'y':
	case 'Y':
	  specify_obj_cursor ();
	  return;
	case 'n':
	case 'N':
	  specify_obj_nocurs ();
	  return;
	}
    }
}



/* perform the actions of identifying the object/monster associated with a
character typed by the user.  assumes cursors().
*/
static void
specify_obj_nocurs (void)
{
  char i;
  int j, flag;

  lprcat ("\nType object character:");
  switch (i = ttgetch ())
    {
    case '\33':
    case '\n':
      return;
    case '@':
      lprintf ("\n@: %s", logname);
      return;
    case ' ':
      lprintf ("\n : An as-yet-unseen place in the dungeon");
      return;
    default:
      if (i == floorc)
	{
	  lprc ('\n');
	  lprc (floorc);
	  lprintf (": the floor of the dungeon");
	  return;
	}
      flag = FALSE;
      for (j = 0; j < MAXMONST + 8; j++)
	if (i == monstnamelist[j])
	  {
	    lprintf ("\n%c: %s", i, monster[j].name);
	    flag = TRUE;
	  }
      /* check for spurious object character
       */
      if (i != '_')
	for (j = 0; j < MAXOBJECT; j++)
	  if (i == objnamelist[j])
	    {
	      lprc ('\n');
	      lprc (i);
	      lprintf (": %s", objectname[j]);
	      flag = TRUE;
	    }
      if (!flag)
	lprintf ("\n%c: unknown monster/object", i);
      return;
    }
}


static void
specify_obj_cursor (void)
{
  int objx, objy;
  int i;

  lprcat ("\nMove the cursor to an unknown item.");
  lprcat ("\n(For instructions type a ?)");

  objx = playerx;
  objy = playery;
  cursor (objx + 1, objy + 1);
  /* make cursor visible.
   */
  for (;;)
    {
      switch (ttgetch ())
	{
	case '?':
	  cursors ();
	  lprcat
	    ("\nUse [hjklnbyu] to move the cursor to the unknown object.");
	  lprcat ("\nType a . when the cursor is at the desired place.");
	  lprcat ("\nType q, Return, or Escape to exit.");
	  cursor (objx + 1, objy + 1);
	  break;

	case '\33':
	case 'q':
	case '\n':
	  /* reset cursor
	   */
	  cursor (playerx + 1, playery + 1);
	  return;
	case '.':
	  /* reset cursor
	   */
	  cursor (playerx + 1, playery + 1);
	  cursors ();

	  if ((objx == playerx) && (objy == playery))
	    {
	      lprintf ("\n@: %s", logname);
	      return;
	    }

	  i = mitem[objx][objy];
	  if (i && (know[objx][objy] & KNOWHERE))

	    /* check for invisible monsters and not display
	     */
	    if (monstnamelist[i] != floorc)
	      {
		lprintf ("\n%c: %s", monstnamelist[i], monster[i].name);
		return;
	      }

	  /* handle floor separately so as not to display traps, etc.
	   */
	  i = item[objx][objy];
	  if (i == 0)
	    {
	      lprc ('\n');
	      lprc (floorc);
	      lprintf (": the floor of the dungeon");
	      return;
	    }

	  if (know[objx][objy] & HAVESEEN)
	    {
	      lprc ('\n');
	      lprc (objnamelist[i]);
	      lprintf (": %s", objectname[i]);
	      return;
	    }

	  lprintf ("\n : An as-yet-unseen place in the dungeon");
	  return;

	case 'H':
	case 'h':
	  move_cursor (&objx, &objy, 4);
	  break;
	case 'J':
	case 'j':
	  move_cursor (&objx, &objy, 1);
	  break;
	case 'K':
	case 'k':
	  move_cursor (&objx, &objy, 3);
	  break;
	case 'L':
	case 'l':
	  move_cursor (&objx, &objy, 2);
	  break;
	case 'B':
	case 'b':
	  move_cursor (&objx, &objy, 8);
	  break;
	case 'N':
	case 'n':
	  move_cursor (&objx, &objy, 7);
	  break;
	case 'Y':
	case 'y':
	  move_cursor (&objx, &objy, 6);
	  break;
	case 'U':
	case 'u':
	  move_cursor (&objx, &objy, 5);
	  break;
	default:
	  break;
	}
    }
}



static void
move_cursor (int *xx, int *yy, int cdir)
{

  *xx += diroffx[cdir];
  *yy += diroffy[cdir];

  if (*yy < 0)
    *yy = MAXY - 1;
  if (*yy > MAXY - 1)
    *yy = 0;
  if (*xx < 0)
    *xx = MAXX - 1;
  if (*xx > MAXX - 1)
    *xx = 0;

  cursor (*xx + 1, *yy + 1);
}
