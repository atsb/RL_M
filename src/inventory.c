#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/display.h"
#include "includes/global.h"
#include "includes/inventory.h"
#include "includes/io.h"
#include <curses.h>

static int qshowstr (char);

static void t_setup (int);
static void t_endup (int);

static int show2 (int);



/* Allow only 26 items (a to z) in the player's inventory */
#define MAXINVEN 26

/* The starting limit to the number of items the player can carry.  
The limit should probably be based on player strength and the
weight of the items.
*/
#define MIN_LIMIT 15

/* define a sentinel to place at the end of the sorted inventory.
(speeds up display reads )
*/
#define END_SENTINEL 255

/* declare the player's inventory.  These should only be referenced
in this module.
iven     - objects in the player's inventory
ivenarg  - attribute of each item ( + values, etc )
ivensort - sorted inventory (so we don't sort each time)
*/
int iven[MAXINVEN];

int ivenarg[MAXINVEN];

int ivensort[MAXINVEN + 1];	/* extra is for sentinel */


static char srcount = 0;	/* line counter for showstr() */



/*
* Initialize the player's inventory
*/
void
init_inventory (void)
{
  int i;

  for (i = 0; i < MAXINVEN; i++)
    {
      iven[i] = ivenarg[i] = 0;
      ivensort[i] = END_SENTINEL;
    }
  ivensort[MAXINVEN] = END_SENTINEL;

  /* For zero difficulty games, start the player out with armor and weapon.
     We can sort the inventory right away because a dagger is 'later' than
     leather armor.
   */
  if (cdesc[HARDGAME] <= 0)
    {
      iven[0] = OLEATHER;
      iven[1] = ODAGGER;
      ivenarg[0] = ivenarg[1] = cdesc[WEAR] = ivensort[0] = 0;
      ivensort[1] = cdesc[WIELD] = 1;
    }
}



/*
* show character's inventory
*/
int
showstr (char select_allowed)
{
  int i, number, item_select;

  for (number = 3, i = 0; i < MAXINVEN; i++)
    if (iven[i])
      number++;			/* count items in inventory */
  t_setup (number);
  item_select = qshowstr (select_allowed);
  t_endup (number);

  return item_select;
}



static int
qshowstr (char select_allowed)
{
  int i, j, k, itemselect = 0;

  srcount = 0;

  if (cdesc[GOLD])
    {
      lprintf (".)   %d gold pieces", (int) cdesc[GOLD]);
      srcount++;
    }
  for (k = (MAXINVEN - 1); k >= 0; k--)
    if (iven[k])
      {
	for (i = 22; i < 84; i++)
	  for (j = 0; j <= k; j++)
	    if (i == iven[j])
	      {
		itemselect = show2 (j);
		if (itemselect && select_allowed)
		  goto quitit;
	      }
	k = 0;
      }

  /*lprintf("\nElapsed time is %d.  You have %d mobuls left",(int)((gtime+99)/100+1),(int)((TIMELIMIT-gtime)/100)); */
  lprintf ("\nElapsed time is %d.  You have %d mobuls left", gtime / 100,
	   (TIMELIMIT - gtime) / 100);
  itemselect = more (select_allowed);
quitit:
  if (select_allowed)
    return ((itemselect > 0) ? itemselect : 0);
  else
    return (0);
}



/*
* subroutine to clear screen depending on # lines to display
*/
static void
t_setup (int count)
{

  /* how do we clear the screen? */
  if (count < 20)
    {

      cl_up (79, count);
      cursor (1, 1);

    }
  else
    {

      resetscroll ();
      screen_clear();
    }
}



/*
* subroutine to restore normal display screen depending on t_setup()
*/
static void
t_endup (int count)
{

  /* how did we clear the screen? */
  if (count < 18)
    {

      draws (0, MAXX, 0, (count > MAXY) ? MAXY : count);

    }
  else
    {

      drawscreen ();
      setscroll ();
    }
}



/*
* function to show the things player is wearing only
*/
int
showwear (void)
{
  int i, count, itemselect;

  itemselect = 0;

  srcount = 0;

  for (count = 2, i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OLEATHER:
	case OPLATE:
	case OCHAIN:
	case ORING:
	case OSTUDLEATHER:
	case OSPLINT:
	case OPLATEARMOR:
	case OSSPLATE:
	case OSHIELD:
	  ++count;
	  break;
	}
    }

  t_setup (count);

  for (i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OLEATHER:
	case OPLATE:
	case OCHAIN:
	case ORING:
	case OSTUDLEATHER:
	case OSPLINT:
	case OPLATEARMOR:
	case OSSPLATE:
	case OSHIELD:
	  itemselect = show2 (i);
	  break;
	}

      if (itemselect)
	break;
    }

  if (!itemselect)
    {

      itemselect = more (TRUE);
    }

  t_endup (count);

  if (itemselect > 1)
    {

      return itemselect;

    }

  return 0;
}



/*
* function to show the things player can wield only
*/
int
showwield (void)
{
  int i, count, itemselect;

  itemselect = 0;

  srcount = 0;

  for (count = 2, i = 0; i < MAXINVEN; i++)
    {
      if (iven[i] != 0)
	{
	  switch (iven[i])
	    {
	    case ODIAMOND:
	    case ORUBY:
	    case OEMERALD:
	    case OSAPPHIRE:
	    case OBOOK:
	    case OCHEST:
	    case OLARNEYE:
	    case ONOTHEFT:
	    case OSPIRITSCARAB:
	    case OCUBEofUNDEAD:
	    case OPOTION:
	    case OSCROLL:
	      break;

	    default:
	      ++count;
	    }
	}
    }

  t_setup (count);

  for (i = 0; i < MAXINVEN; i++)
    {
      if (iven[i] != 0)
	{
	  switch (iven[i])
	    {
	    case ODIAMOND:
	    case ORUBY:
	    case OEMERALD:
	    case OSAPPHIRE:
	    case OBOOK:
	    case OCHEST:
	    case OLARNEYE:
	    case ONOTHEFT:
	    case OSPIRITSCARAB:
	    case OCUBEofUNDEAD:
	    case OPOTION:
	    case OSCROLL:
	      break;

	    default:
	      itemselect = show2 (i);
	    }
	}

      if (itemselect)
	break;
    }

  if (!itemselect)
    {

      itemselect = more (TRUE);
    }

  t_endup (count);

  if (itemselect > 1)
    {

      return itemselect;

    }

  return 0;
}



/*
* function to show the things player can read only
*/
int
showread (void)
{
  int i, count, itemselect;

  itemselect = 0;

  srcount = 0;

  for (count = 2, i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OBOOK:
	case OSCROLL:
	  ++count;
	  break;
	}
    }

  t_setup (count);

  for (i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OBOOK:
	case OSCROLL:
	  itemselect = show2 (i);
	  break;
	}

      if (itemselect)
	break;
    }

  if (!itemselect)
    {

      itemselect = more (TRUE);
    }

  t_endup (count);

  if (itemselect > 1)
    {

      return itemselect;

    }

  return 0;
}



/*
*  function to show the things player can eat only
*/
int
showeat (void)
{
  int i, count, itemselect;

  itemselect = 0;

  srcount = 0;

  for (count = 2, i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OCOOKIE:
	  ++count;
	  break;
	}
    }

  t_setup (count);

  for (i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OCOOKIE:
	  itemselect = show2 (i);
	  break;
	}

      if (itemselect)
	break;
    }

  if (!itemselect)
    {

      itemselect = more (TRUE);
    }

  t_endup (count);

  if (itemselect > 1)
    {

      return itemselect;

    }

  return 0;
}



/*
* function to show the things player can quaff only
*/
int
showquaff (void)
{
  int i, count, itemselect;

  itemselect = 0;

  srcount = 0;

  for (count = 2, i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OPOTION:
	  ++count;
	  break;
	}
    }

  t_setup (count);

  for (i = 0; i < MAXINVEN; i++)
    {

      switch (iven[i])
	{

	case OPOTION:
	  itemselect = show2 (i);
	  break;
	}

      if (itemselect)
	break;
    }

  if (!itemselect)
    {

      itemselect = more (TRUE);
    }

  t_endup (count);

  if (itemselect > 1)
    {

      return itemselect;

    }

  return 0;
}

/*some backporting from 12.3 with my own changes and fixes.
 *Cleaned up and squashed a bug with duplication of text shown. ~Gibbon
 */
void
show1 (int idx)
{
  lprc ('\n');
  cltoeoln ();

  /*not totally needed but it is cleaner.  Inventory will not be == 0 due to starting items. ~Gibbon
   */
  if (iven[idx] != 0)
    {
    	attron(COLOR_PAIR(4));
      lprintf ("%c) ",idx + 'a');
      attroff(COLOR_PAIR(4));
      lprintf("%s", objectname[iven[idx]]);
    }

  /*we can remove the index to object name and concatenate the above with the below for scrolls and potions.
   *since OPOTION and OSCROLL (object names) will be stuffed into the above, we can focus on identified names. ~Gibbon
   */
  if (iven[idx] == OPOTION && potionname[ivenarg[idx]][0] != '\0')
    {
      lprintf (" of%s", potionname[ivenarg[idx]]);
    }
  else if (iven[idx] == OSCROLL && scrollname[ivenarg[idx]][0] != '\0')
    {
      lprintf (" of%s", scrollname[ivenarg[idx]]);
    }
}

int
show3 (int index)
{

  srcount = 0;

  return show2 (index);
}

static int
show2 (int index)
{
  int itemselect = 0;

  switch (iven[index])
    {
    case OPOTION:
    case OSCROLL:
    case OLARNEYE:
    case OBOOK:
    case OSPIRITSCARAB:
    case ODIAMOND:
    case ORUBY:
    case OCUBEofUNDEAD:
    case OEMERALD:
    case OCHEST:
    case OCOOKIE:
    case OSAPPHIRE:
    case ONOTHEFT:
      show1 (index);
      break;

    default:
      lprc ('\n');
      cltoeoln ();
      attron(COLOR_PAIR(4));
      lprintf ("%c) ",index + 'a');
      attroff(COLOR_PAIR(4));
      lprintf("%s",objectname[iven[index]]);
      if (ivenarg[index] > 0)
	lprintf (" + %d", (long) ivenarg[index]);
      else if (ivenarg[index] < 0)
	lprintf (" %d", (long) ivenarg[index]);
      break;
    }
  if (cdesc[WIELD] == index)
    lprcat (" (weapon in hand)");
  if ((cdesc[WEAR] == index) || (cdesc[SHIELD] == index))
    lprcat (" (being worn)");
  if (++srcount >= 22)
    {
      srcount = 0;
      itemselect = more (TRUE);
      screen_clear();
    }
  return (itemselect);
}


/*
* function to put something in the players inventory
* returns 0 if success, 1 if a failure
*/
int
take (int itm, int arg)
{
  int i, limit;

  /*  cursors(); */
  if ((limit = 15 + (cdesc[LEVEL] >> 1)) > MAXINVEN)
    limit = MAXINVEN;
  for (i = 0; i < limit; i++)
    if (iven[i] == 0)
      {
	iven[i] = itm;
	ivenarg[i] = arg;
	limit = 0;
	switch (itm)
	  {
	  case OPROTRING:
	  case ODAMRING:
	  case OBELT:
	    limit = 1;
	    break;
	  case ODEXRING:
	    cdesc[DEXTERITY] += ivenarg[i] + 1;
	    limit = 1;
	    break;
	  case OSTRRING:
	    cdesc[STREXTRA] += ivenarg[i] + 1;
	    limit = 1;
	    break;
	  case OCLEVERRING:
	    cdesc[INTELLIGENCE] += ivenarg[i] + 1;
	    limit = 1;
	    break;
	  case OHAMMER:
	    cdesc[DEXTERITY] += 10;
	    cdesc[STREXTRA] += 10;
	    cdesc[INTELLIGENCE] -= 10;
	    limit = 1;
	    break;

	  case OORBOFDRAGON:
	    cdesc[SLAYING]++;
	    break;
	  case OSPIRITSCARAB:
	    cdesc[NEGATESPIRIT]++;
	    break;
	  case OCUBEofUNDEAD:
	    cdesc[CUBEofUNDEAD]++;
	    break;
	  case ONOTHEFT:
	    cdesc[NOTHEFT]++;
	    break;
	  case OSWORDofSLASHING:
	    cdesc[DEXTERITY] += 5;
	    limit = 1;
	    break;
	  };
	lprcat ("\nYou pick up:");
	show3 (i);
	if (limit)
	  bottomline ();
	return (0);
      }
  lprcat ("\nYou can't carry anything else");
  return (1);
}



/*
* subroutine to drop an object  returns 1 if something there already else 0
*/
int
drop_object (int k)
{
  int itm;

  if ((k < 0) || (k >= MAXINVEN))
    return (0);
  itm = iven[k];
  cursors ();
  if (itm == 0)
    {
      lprintf ("\nYou don't have item %c! ", k + 'a');
      return (1);
    }
  if (item[playerx][playery])
    {
      lprintf ("\nThere's something here already: %s",
	       objectname[item[playerx][playery]]);
      dropflag = 1;
      return (1);
    }
  if (playery == MAXY - 1 && playerx == 33)
    return (1);			/* not in entrance */
  item[playerx][playery] = itm;
  iarg[playerx][playery] = ivenarg[k];
  lprcat ("\n  You drop:");
  show3 (k);			/* show what item you dropped */
  know[playerx][playery] = 0;
  iven[k] = 0;
  if (cdesc[WIELD] == k)
    cdesc[WIELD] = -1;
  if (cdesc[WEAR] == k)
    cdesc[WEAR] = -1;
  if (cdesc[SHIELD] == k)
    cdesc[SHIELD] = -1;
  adjustcvalues (itm, ivenarg[k]);
  dropflag = 1;			/* say dropped an item so wont ask to pick it up right away */
  return (0);
}

/*
* routine to tell if player can carry one more thing
* returns 1 if pockets are full, else 0
*/
int
pocketfull (void)
{
  int i, limit;

  limit = MIN_LIMIT + (cdesc[LEVEL] >> 1);

  if (limit > MAXINVEN)
    {

      limit = MAXINVEN;
    }

  for (i = 0; i < limit; i++)
    {

      if (iven[i] == 0)
	{

	  return 0;
	}
    }

  return 1;
}
