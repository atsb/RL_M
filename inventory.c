#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"
#include "display.h"
#include "global.h"
#include "inventory.h"
#include "io.h"

/* The inventory system has been largely rewritten into ANSI C
* the old inventory had too many assumptions about terminal drawing
* and was butchered by early Larn contributors.
* now it is clean, maintainable and modern, while fitting perfectly into
* Larn's existing 1986 codebase.
*/
static int qshowstr (void);

static void t_setup (int);
static void t_endup (int);

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

void
inventoryline_print(int idx)
{
    int obj;

    /* idx means gold line */
    if (idx == -1) {
        lprintf(".) %d gold pieces", (int)c[GOLD]);
        return;
    }

    obj = iven[idx];

    /* base item name */
    lprintf("%c) %s", idx + 'a', objectname[obj]);

    /* identified potion/scroll names */
    if (obj == OPOTION && potionname[ivenarg[idx]][0] != '\0') {
        lprintf(" of%s", potionname[ivenarg[idx]]);
    }
    else if (obj == OSCROLL && scrollname[ivenarg[idx]][0] != '\0') {
        lprintf(" of%s", scrollname[ivenarg[idx]]);
    }
    /* +N / -N modifier names */
    else if (ivenarg[idx] > 0) {
        lprintf(" + %d", ivenarg[idx]);
    }
    else if (ivenarg[idx] < 0) {
        lprintf(" %d", ivenarg[idx]);
    }

    /* equipped markers */
    if (c[WIELD] == idx)
        lprcat(" (weapon in hand)");
    if (c[WEAR] == idx || c[SHIELD] == idx)
        lprcat(" (being worn)");
}

static int
inventoryline_page(const int *indices, int count, char select_allowed)
{
    int page_height;
    int start;
    int itemselect;
    int lines_this_page;
    int y;
    int i;
    int idx;

    page_height = 22;
    start = 0;
    itemselect = 0;

    while (start < count)
    {
        if (count - start > page_height)
            lines_this_page = page_height;
        else
            lines_this_page = count - start;

        /* clear only the lines we will use */
        for (y = 1; y <= lines_this_page + 2; y++) {
            move(y - 1, 0);
            clrtoeol();
        }
        cursor(1, 1);

        /* print this page */
        for (i = 0; i < lines_this_page; i++) {
            idx = indices[start + i];
            inventoryline_print(idx);
            lprc('\n');
        }

        /* elapsed time line */
        lprintf("Elapsed time is %d.  You have %d mobuls left",
                gtime / 100, (TIMELIMIT - gtime) / 100);

        /* wait for space or selection */
        itemselect = more(select_allowed);

        if (itemselect && select_allowed)
            return (itemselect > 0) ? itemselect : 0;

        start += lines_this_page;
    }

    return 0;
}
static int
inventory_show(int (*predicate)(int))
{
    int indices[MAXINVEN + 1];
    int n;
    int i;
    int count;
    int sel;

    n = 0;

    for (i = 0; i < MAXINVEN; i++)
        if (iven[i] && predicate(iven[i]))
            indices[n++] = i;

    count = n + 2;  /* header and footer */

    t_setup(count);
    sel = inventoryline_page(indices, n, TRUE);
    t_endup(count);

    return (sel > 1) ? sel : 0;
}

static int
iswearable(int obj)
{
    switch (obj) {
    case OLEATHER:
    case OPLATE:
    case OCHAIN:
    case ORING:
    case OSTUDLEATHER:
    case OSPLINT:
    case OPLATEARMOR:
    case OSSPLATE:
    case OSHIELD:
        return 1;
    }
    return 0;
}

static int
iswieldable(int obj)
{
    switch (obj) {
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
        return 0;
    }
    return 1;
}

static int
isreadable(int obj)
{
    return (obj == OBOOK || 
        obj == OSCROLL);
}

static int
isedible(int obj)
{
    return (obj == OCOOKIE);
}

static int
isquaffable(int obj)
{
    return (obj == OPOTION);
}

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
  if (c[HARDGAME] <= 0)
    {
      iven[0] = OLEATHER;
      iven[1] = ODAGGER;
      ivenarg[0] = ivenarg[1] = c[WEAR] = ivensort[0] = 0;
      ivensort[1] = c[WIELD] = 1;
    }
}

/*
* show character's inventory
*/
int
showstr(void)
{
    int count;
    int i;
    int sel;

    count = 3;

    if (c[GOLD])
        count++;

    for (i = 0; i < MAXINVEN; i++)
        if (iven[i])
            count++;

    t_setup(count);
    sel = qshowstr();
    t_endup(count);

    return sel;
}

static int
qshowstr(void)
{
    int indices[MAXINVEN + 1];
    int n;
    int i;

    n = 0;

    /* gold first */
    if (c[GOLD])
        indices[n++] = -1;

    /* then items */
    for (i = 0; i < MAXINVEN; i++)
        if (iven[i])
            indices[n++] = i;

    return inventoryline_page(indices, n, FALSE);
}

/*
* subroutine to clear screen depending on # lines to display
*/
static void
t_setup(int count)
{
    int y;

    for (y = 1; y <= count; y++) {
        move(y - 1, 0);
        clrtoeol();
    }
    cursor(1, 1);
}

/*
* subroutine to restore normal display screen depending on t_setup()
*/
static void
t_endup(int count)
{
    /* redraw only the top count lines of the map */
    draws(0, MAXX, 0, (count > MAXY ? MAXY : count));

    /* redraw status + name + right panel */
    bottomline();
    statusmessage_draw_right_panel();
}

/*
* function to show the things player is wearing only
*/
int showwear(void)
{
    return inventory_show(iswearable);
}

/*
* function to show the things player can wield only
*/
int showwield(void)
{
    return inventory_show(iswieldable);
}

/*
* function to show the things player can read only
*/
int showread(void)
{
    return inventory_show(isreadable);
}

/*
*  function to show the things player can eat only
*/
int showeat(void)
{
    return inventory_show(isedible);
}

/*
* function to show the things player can quaff only
*/
int showquaff(void)
{
    return inventory_show(isquaffable);
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
  if ((limit = 15 + (c[LEVEL] >> 1)) > MAXINVEN)
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
	    c[DEXTERITY] += ivenarg[i] + 1;
	    limit = 1;
	    break;
	  case OSTRRING:
	    c[STREXTRA] += ivenarg[i] + 1;
	    limit = 1;
	    break;
	  case OCLEVERRING:
	    c[INTELLIGENCE] += ivenarg[i] + 1;
	    limit = 1;
	    break;
	  case OHAMMER:
	    c[DEXTERITY] += 10;
	    c[STREXTRA] += 10;
	    c[INTELLIGENCE] -= 10;
	    limit = 1;
	    break;

	  case OORBOFDRAGON:
	    c[SLAYING]++;
	    break;
	  case OSPIRITSCARAB:
	    c[NEGATESPIRIT]++;
	    break;
	  case OCUBEofUNDEAD:
	    c[CUBEofUNDEAD]++;
	    break;
	  case ONOTHEFT:
	    c[NOTHEFT]++;
	    break;
	  case OSWORDofSLASHING:
	    c[DEXTERITY] += 5;
	    limit = 1;
	    break;
	  };
	lprcat ("\nYou pick up: ");
	inventoryline_print (i);
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

  /* dropping an item into lava destroys it */
  if (item[playerx][playery] == OLAVA)
  {
      lprcat("\nThe object has been melted by lava!");
      iven[k] = 0;
      dropflag = 1;
      return 0;
  }

  if (item[playerx][playery] &&
      item[playerx][playery] != OWATER &&
      item[playerx][playery] != OSHOREWATER &&
      item[playerx][playery] != OLAVA)
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
  lprcat ("\n  You drop: ");
  inventoryline_print (k);			/* show what item you dropped */
  know[playerx][playery] = 0;
  iven[k] = 0;
  if (c[WIELD] == k)
    c[WIELD] = -1;
  if (c[WEAR] == k)
    c[WEAR] = -1;
  if (c[SHIELD] == k)
    c[SHIELD] = -1;
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

  limit = MIN_LIMIT + (c[LEVEL] >> 1);

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
