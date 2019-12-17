/* store.c */
/*
This module contains data and routines to handle buildings at the home level.
Routines:

dnd_2hed
dnd_hed
dndstore      The DND store main routine
handsfull     To tell the player he can carry no more
out_of_stock  To tell the player an item is out of stock
no_gold       To tell the player he has no gold
dnditem       Display DND store items
sch_head      print the school header
oschool       main school routine
obank         Larn National Bank
obank2        5th level branch of the bank
banktitle     bank header
ointerest     accrue interest to bank account
obanksub      bank main subroutine
otradhead     trading post header
otradepost    trading post main function
cnsitm
olrs          larn revenue service function
*/

#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/display.h"
#include "includes/global.h"
#include "includes/inventory.h"
#include "includes/io.h"
#include "includes/main.h"
#include "includes/object.h"
#include "includes/scores.h"
#include "includes/store.h"
#include "includes/sysdep.h"
#include <curses.h>

static void dnd_2hed (void);
static void dnd_hed (void);
static void handsfull (void);
static void outofstock (void);
static void nogold (void);
static void dnditem (int i);
static void sch_hed (void);
static void banktitle (char *);
static void obanksub (void);
static void otradhead (void);
static void otradiven (void);
static void cleartradiven (int);
static void cnsitm (void);

static int dndcount = 0, dnditm = 0;

/* new function for displaying gold in inventory inside trading posts.
 * part of feature request by hymie0. ~Gibbon */
static int
amtgoldtrad(void)
{
	lprintf ("You have");
	attron(COLOR_PAIR(3));
	lprintf(" %-6d ",(int) cdesc[GOLD]);
	attroff(COLOR_PAIR(3));
	lprintf("gold pieces.");
	return(0);
}
/*Fix for bug #23 ~Gibbon*/
static int
lrs_welcome_text(void)
{
	screen_clear();
	resetscroll();
	lprintf("Welcome to the Larn Revenue Service\n");
	return(0);
}
/* number of items in the dnd inventory table   */
#define MAXITM 83

/*  this is the data for the stuff in the dnd store */
struct _itm dnd_item[90] = {
  /*cost   iven name   iven arg   how
     gp      iven[]    ivenarg[]  many */

  {2, OLEATHER, 0, 3},
  {10, OSTUDLEATHER, 0, 2},
  {40, ORING, 0, 2},
  {85, OCHAIN, 0, 2},
  {220, OSPLINT, 0, 1},
  {400, OPLATE, 0, 1},
  {900, OPLATEARMOR, 0, 1},
  {2600, OSSPLATE, 0, 1},
  {150, OSHIELD, 0, 1},

  /*cost    memory    iven name   iven arg   how
     gp     pointer      iven[]    ivenarg[]  many */

  {2, ODAGGER, 0, 3},
  {20, OSPEAR, 0, 3},
  {150, OBATTLEAXE, 0, 2},
  {450, OLONGSWORD, 0, 2},
  {1000, O2SWORD, 0, 2},
  {5000, OSWORD, 0, 1},

  /* lets make it twice as expensive. ~Gibbon */
  {30000, OGREATSWORD, 0, 1},
  {6000, OSWORDofSLASHING, 0, 0},
  {10000, OHAMMER, 0, 0},

  /*cost    memory    iven name   iven arg   how
     gp     pointer      iven[]    ivenarg[]  many */

  {150, OPROTRING, 1, 1},
  {85, OSTRRING, 1, 1},
  {120, ODEXRING, 1, 1},
  {120, OCLEVERRING, 1, 1},
  {180, OENERGYRING, 0, 1},
  {125, ODAMRING, 0, 1},
  {220, OREGENRING, 0, 1},
  {1000, ORINGOFEXTRA, 0, 1},

  {280, OBELT, 0, 1},

  {400, OAMULET, 0, 1},

  {6500, OORBOFDRAGON, 0, 0},
  {5500, OSPIRITSCARAB, 0, 0},
  {5000, OCUBEofUNDEAD, 0, 0},
  {6000, ONOTHEFT, 0, 0},

  {590, OCHEST, 6, 1},
  {200, OBOOK, 8, 1},
  {10, OCOOKIE, 0, 3},

  /*cost    memory    iven name   iven arg   how
     gp     pointer      iven[]    ivenarg[]  many */

  {20, OPOTION, 0, 6},
  {90, OPOTION, 1, 5},
  {520, OPOTION, 2, 1},
  {100, OPOTION, 3, 2},
  {50, OPOTION, 4, 2},
  {150, OPOTION, 5, 2},
  {70, OPOTION, 6, 1},
  {30, OPOTION, 7, 7},
  {200, OPOTION, 8, 1},
  {50, OPOTION, 9, 1},
  {80, OPOTION, 10, 1},

  /*cost    memory    iven name   iven arg   how
     gp     pointer      iven[]    ivenarg[]  many */

  {30, OPOTION, 11, 3},
  {20, OPOTION, 12, 5},
  {40, OPOTION, 13, 3},
  {35, OPOTION, 14, 2},
  {520, OPOTION, 15, 1},
  {90, OPOTION, 16, 2},
  {200, OPOTION, 17, 2},
  {220, OPOTION, 18, 4},
  {80, OPOTION, 19, 6},
  {370, OPOTION, 20, 3},
  {50, OPOTION, 22, 1},
  {150, OPOTION, 23, 3},

  /*cost    memory    iven name   iven arg   how
     gp     pointer      iven[]    ivenarg[]  many */

  {100, OSCROLL, 0, 2},
  {125, OSCROLL, 1, 2},
  {60, OSCROLL, 2, 4},
  {10, OSCROLL, 3, 4},
  {100, OSCROLL, 4, 3},
  {200, OSCROLL, 5, 2},
  {110, OSCROLL, 6, 1},
  {500, OSCROLL, 7, 2},
  {200, OSCROLL, 8, 2},
  {250, OSCROLL, 9, 4},
  {20, OSCROLL, 10, 5},
  {30, OSCROLL, 11, 3},

  /*cost    memory    iven name   iven arg   how
     gp     pointer      iven[]    ivenarg[]  many */

  {340, OSCROLL, 12, 1},
  {340, OSCROLL, 13, 1},
  {300, OSCROLL, 14, 2},
  {400, OSCROLL, 15, 2},
  {500, OSCROLL, 16, 2},
  {1000, OSCROLL, 17, 1},
  {500, OSCROLL, 18, 1},
  {340, OSCROLL, 19, 2},
  {220, OSCROLL, 20, 3},
  {3900, OSCROLL, 21, 0},
  {610, OSCROLL, 22, 1},
  {3000, OSCROLL, 23, 0}
};

/*
for the college of larn
*/
int course[26];			/*  the list of courses taken   */
static char coursetime[] = { 10, 15, 10, 20, 10, 10, 10, 5 };


/*
function for the dnd store
*/
static void
dnd_2hed (void)
{
  lprcat
    ("Welcome to the Larn Thrift Shoppe.  We stock many items explorers find useful\n");
  lprcat
    (" in their adventures.  Feel free to browse to your hearts content.\n");
  lprcat ("Also be advised, if you break 'em, you pay for 'em.");
}


static void
dnd_hed (void)
{
  int i;

  for (i = dnditm; i < 26 + dnditm; i++)
    {
      dnditem (i);
    }
}


void
dndstore (void)
{
  int i;

  dnditm = 0;
  screen_clear();
  dnd_2hed ();

  if (outstanding_taxes > 0)
    {
      lprcat
	("\n\nThe Larn Revenue Service has ordered us to not do business with tax evaders.\n");
      lprintf
	("They have also told us that you owe %d gp in back taxes, and as we must\n",
	 (int) outstanding_taxes);
      lprcat
	("comply with the law, we cannot serve you at this time.  Soo Sorry.\n");
      cursors ();
      lprcat ("\nPress ");
      lstandout ("escape");
      lprcat (" to leave: ");
      lflush ();

      i = 0;
      while (i != '\33')
	{
	  i = ttgetch ();
	}

      drawscreen ();
      return;
    }

  dnd_hed ();

  for (;;)
    {

      cursor (1, 19);
      lprcat ("You have ");
      attron(COLOR_PAIR(3));
      lprintf ("%ld ",cdesc[GOLD]);
      attroff(COLOR_PAIR(3));
      lprintf("gold pieces");
	
      cltoeoln ();
      cl_dn (1, 20);

      lprcat ("\nEnter your transaction [");
      lstandout ("space");
      lprcat (" for more, ");
      lstandout ("escape");
      lprcat (" to leave]? ");

      i = 0;
      while ((i < 'a' || i > 'z') && (i != ' ') && (i != '\33') && (i != 12))
	{
	  i = ttgetch ();
	}

      if (i == 12)
	{
	  screen_clear();
	  dnd_2hed ();
	  dnd_hed ();
	}
      else if (i == '\33')
	{
	  drawscreen ();
	  return;
	}
      else if (i == ' ')
	{

	  cl_dn (1, 4);

	  if ((dnditm += 26) >= MAXITM)
	    {

	      dnditm = 0;
	    }

	  dnd_hed ();

	}
      else
	{			/* buy something */
	  lprc ((char) i);	/* echo the byte */
	  i += dnditm - 'a';
	  if (i >= MAXITM)
	    outofstock ();
	  else if (dnd_item[i].qty <= 0)
	    outofstock ();
	  else if (pocketfull ())
	    handsfull ();
	  else if (cdesc[GOLD] < (dnd_item[i].price) * 10L)
	    nogold ();
	  else
	    {

	      if (dnd_item[i].obj == OPOTION)
		{
		  potionname[dnd_item[i].arg][0] = ' ';
		}
	      else if (dnd_item[i].obj == OSCROLL)
		{
		  scrollname[dnd_item[i].arg][0] = ' ';
		}
	      cdesc[GOLD] -= dnd_item[i].price * 10;
	      dnd_item[i].qty--;
	      take (dnd_item[i].obj, dnd_item[i].arg);
	      if (dnd_item[i].qty == 0)
		dnditem (i);
	      lflush ();
	      nap (NAPTIME);
	    }
	}

    }
}




/*
function for the players hands are full
*/
static void
handsfull (void)
{

  lprcat ("\nYou can't carry anything more!");
  lflush ();
  nap (NAPTIME);
}


static void
outofstock (void)
{

  lprcat ("\nSorry, but we are out of that item.");
  lflush ();
  nap (NAPTIME);
}

static void
nogold (void)
{
  lprcat ("\nYou don't have enough gold to pay for that!");
  lflush ();
  nap (2200);
}


/*
dnditem(index)

to print the item list;  used in dndstore() enter with the index into itm
*/
static void
dnditem (int i)
{

  int j, k;
  int price;

  if (i >= MAXITM)
    {

      return;
    }

  cursor ((j = (i & 1) * 40 + 1), (k = ((i % 26) >> 1) + 5));

  if (dnd_item[i].qty == 0)
    {
    	attron(COLOR_PAIR(2));
      lprintf ("%39s", "");
      attroff(COLOR_PAIR(2));

      return;
    }
attron(COLOR_PAIR(4));
  lprintf ("%c) ", (i % 26) + 'a');
attroff(COLOR_PAIR(4));
  if (dnd_item[i].obj == OPOTION)
    {
      lprcat ("potion of ");
      lprintf ("%s", &potionname[dnd_item[i].arg][1]);
    }
  else if (dnd_item[i].obj == OSCROLL)
    {
      lprcat ("scroll of ");
      lprintf ("%s", &scrollname[dnd_item[i].arg][1]);
    }
  else
    {
      lprintf ("%s", objectname[dnd_item[i].obj]);
    }
    

  cursor (j + 31, k);

  price = ((int) dnd_item[i].price) * 10L;
attron(COLOR_PAIR(3));
  lprintf ("%6ld", price);
attroff(COLOR_PAIR(3));
}



/*
function to display the header info for the school
*/
static void
sch_hed (void)
{

  screen_clear();
  lprcat
    ("The College of Larn offers the exciting opportunity of higher education to\n");
  lprcat
    ("all inhabitants of the caves.  Here is a list of the class schedule:\n\n\n");
  lprcat ("\t\t    Course Name \t       Time Needed\n\n");

  if (course[0] == 0)
    lprcat ("\t\ta)  Fighters Training I         10 mobuls");	/*line 7 of crt */
  lprc ('\n');
  if (course[1] == 0)
    lprcat ("\t\tb)  Fighters Training II        15 mobuls");
  lprc ('\n');
  if (course[2] == 0)
    lprcat ("\t\tc)  Introduction to Wizardry    10 mobuls");
  lprc ('\n');
  if (course[3] == 0)
    lprcat ("\t\td)  Applied Wizardry            20 mobuls");
  lprc ('\n');
  if (course[4] == 0)
    lprcat ("\t\te)  Behavioral Psychology       10 mobuls");
  lprc ('\n');
  if (course[5] == 0)
    lprcat ("\t\tf)  Faith for Today             10 mobuls");
  lprc ('\n');
  if (course[6] == 0)
    lprcat ("\t\tg)  Contemporary Dance          10 mobuls");
  lprc ('\n');
  if (course[7] == 0)
    lprcat ("\t\th)  History of Larn              5 mobuls");

  lprcat ("\n\n\t\tAll courses cost 250 gold pieces.");
  cursor (1, 19);
  lprcat ("You are presently carrying ");
}



void
oschool (void)
{
  int i;
  int time_used;

  sch_hed ();
  for (;;)
    {
      cursor (1, 19);
      attron(COLOR_PAIR(3));
      lprintf ("%d ",(int) cdesc[GOLD]);
      attroff(COLOR_PAIR(3));
      lprintf("gold pieces.");
      cursors ();
      lprcat ("\nWhat is your choice [");
      lstandout ("escape");
      lprcat (" to leave] ? ");
      yrepcount = 0;
      i = 0;
      while ((i < 'a' || i > 'h') && (i != '\33') && (i != 12))
	i = ttgetch ();
      if (i == 12)
	{
	  sch_hed ();
	  continue;
	}
      else if (i == '\33')
	{
	  drawscreen ();
	  return;
	}
      lprc ((char) i);
      if (cdesc[GOLD] < 250)
	nogold ();
      else if (course[i - 'a'])
	{
	  lprcat ("\nSorry, but that class is filled.");
	  nap (NAPTIME);
	}
      else if (i <= 'h')
	{
	  cdesc[GOLD] -= 250;
	  time_used = 0;
	  switch (i)
	    {
	    case 'a':
	      cdesc[STRENGTH] += 2;
	      cdesc[CONSTITUTION]++;
	      lprcat ("\nYou feel stronger!");
	      cl_line (16, 7);
	      break;

	    case 'b':
	      if (course[0] == 0)
		{
		  lprcat
		    ("\nSorry, but this class has a prerequisite of Fighters Training I");
		  cdesc[GOLD] += 250;
		  time_used = -10000;
		  break;
		}
	      lprcat ("\nYou feel much stronger!");
	      cl_line (16, 8);
	      cdesc[STRENGTH] += 2;
	      cdesc[CONSTITUTION] += 2;
	      break;

	    case 'c':
	      cdesc[INTELLIGENCE] += 2;
	      lprcat ("\nThe task before you now seems more attainable!");
	      cl_line (16, 9);
	      break;

	    case 'd':
	      if (course[2] == 0)
		{
		  lprcat
		    ("\nSorry, but this class has a prerequisite of Introduction to Wizardry");
		  cdesc[GOLD] += 250;
		  time_used = -10000;
		  break;
		}
	      lprcat ("\nThe task before you now seems very attainable!");
	      cl_line (16, 10);
	      cdesc[INTELLIGENCE] += 2;
	      break;

	    case 'e':
	      cdesc[CHARISMA] += 3;
	      lprcat ("\nYou now feel like a born leader!");
	      cl_line (16, 11);
	      break;

	    case 'f':
	      cdesc[WISDOM] += 2;
	      lprcat
		("\nYou now feel more confident that you can find the potion in time!");
	      cl_line (16, 12);
	      break;

	    case 'g':
	      cdesc[DEXTERITY] += 3;
	      lprcat ("\nYou feel like dancing!");
	      cl_line (16, 13);
	      break;

	    case 'h':
	      cdesc[INTELLIGENCE]++;
	      lprcat
		("\nYour instructor told you that the Eye of Larn is rumored to be guarded\n");
	      lprcat
		("by a platinum dragon who possesses psionic abilities. ");
	      cl_line (16, 14);
	      break;
	    }
	  time_used += coursetime[i - 'a'] * 100;
	  if (time_used > 0)
	    {
	      gtime += time_used;
	      course[i - 'a']++;	/*  remember that he has taken that course  */
	      cdesc[HP] = cdesc[HPMAX];
	      cdesc[SPELLS] = cdesc[SPELLMAX];	/* he regenerated */

	      if (cdesc[BLINDCOUNT])
		cdesc[BLINDCOUNT] = 1;	/* cure blindness too!  */
	      if (cdesc[CONFUSE])
		cdesc[CONFUSE] = 1;	/*  end confusion   */
	      adjtimel ((int) time_used);	/* adjust parameters for time change */
	    }
	  nap (NAPTIME);
	}
    }
}

/*
*  for the first national bank of Larn
*/
int lasttime = 0;		/* last time he was in bank */

void
obank (void)
{

  banktitle ("    Welcome to the First National Bank of Larn.");
}


void
obank2 (void)
{

  banktitle ("Welcome to the 5th level branch office of "
	     "the First National Bank of Larn.");

  /* because we state the level in the title, clear the '?' in the
     level display at the bottom, if the user teleported.
   */
  cdesc[TELEFLAG] = 0;
}


static void
banktitle (char *str)
{
  screen_clear();
  lprcat (str);
  if (outstanding_taxes > 0)
    {
      int i;
      lprcat
	("\n\nThe Larn Revenue Service has ordered that your account be frozen until all\n");
      lprintf
	("levied taxes have been paid.  They have also told us that you owe %d gp in\n",
	 (int) outstanding_taxes);
      lprcat
	("taxes, and we must comply with them. We cannot serve you at this time.  Sorry.\n");
      lprcat ("We suggest you go to the LRS office and pay your taxes.\n");
      cursors ();
      lprcat ("\nPress ");
      lstandout ("escape");
      lprcat (" to leave: ");
      lflush ();
      i = 0;
      while (i != '\33')
	i = ttgetch ();
      drawscreen ();
      return;
    }
  obanksub ();
  drawscreen ();
}



/*
*  function to put interest on your bank account
*/
void
ointerest (void)
{
  int i;

  if (cdesc[BANKACCOUNT] < 0)
    cdesc[BANKACCOUNT] = 0;
  else if ((cdesc[BANKACCOUNT] > 0) && (cdesc[BANKACCOUNT] < 500000))
    {
      i = (gtime - lasttime) / 100;	/* # mobuls elapsed */
      while ((i-- > 0) && (cdesc[BANKACCOUNT] < 500000))
	cdesc[BANKACCOUNT] += cdesc[BANKACCOUNT] / 250;
      if (cdesc[BANKACCOUNT] > 500000)
	cdesc[BANKACCOUNT] = 500000;	/* interest limit */
    }
  lasttime = (gtime / 100) * 100;
}

static void
obanksub (void)
{
  /* the reference to screen location for each gem */
  int gemorder[26];
  /* the appraisal of the gems */
  int gemvalue[26];
  int amt;
  int i, k, gems_sold = 0;

  lprcat ("\n\n\tGemstone\t      Appraisal\t\tGemstone\t      Appraisal");

  /* credit any needed interest */
  ointerest ();

  for (k = i = 0; i < 26; i++)
    switch (iven[i])
      {
      case OLARNEYE:
      case ODIAMOND:
      case OEMERALD:
      case ORUBY:
      case OSAPPHIRE:

	if (iven[i] == OLARNEYE)
	  {
	    gemvalue[i] = 250000 - ((gtime * 7) / 100) * 100;
	    if (gemvalue[i] < 50000)
	      gemvalue[i] = 50000;
	  }
	else
	  gemvalue[i] = (255 & ivenarg[i]) * 100;
	gemorder[i] = k;
	cursor ((k % 2) * 40 + 1, (k >> 1) + 4);
	lprintf ("%c) %s", i + 'a', objectname[iven[i]]);
	cursor ((k % 2) * 40 + 33, (k >> 1) + 4);
	lprintf ("%5d", (int) gemvalue[i]);
	k++;
	break;

      default:			/* make sure player can't sell non-existant gems */
	gemvalue[i] = 0;
	gemorder[i] = 0;
      };

/* Cleaning up the awful UI here for the text. -Gibbon */

	cursor(1,13);
  	lprintf("Account Summary:");
	cursor(1,15);
  	lprintf("Gold in Bank Account");
	cursor (1, 16);
	attron(COLOR_PAIR(3));
	lprintf("%8d",(long)cdesc[BANKACCOUNT]);
	attroff(COLOR_PAIR(3));
	cursor(1,18);
  	lprintf("Gold in inventory");
	cursor (1, 19);
	attron(COLOR_PAIR(3));
	lprintf("%8d",(long)cdesc[GOLD]);
	attroff(COLOR_PAIR(3));
  if (cdesc[BANKACCOUNT] + cdesc[GOLD] >= 500000)
    lprcat
      ("\nNote:  Larndom law states that only deposits under 500,000gp  can earn interest.");
  for (;;)
    {
      cl_dn (1, 20);
      lprcat ("\nYour wish? [(");
      lstandout ("d");
      lprcat (") deposit, (");
      lstandout ("w");
      lprcat (") withdraw, (");
      lstandout ("s");
      lprcat (") sell a stone, or ");
      lstandout ("escape");
      lprcat ("]  ");
      yrepcount = 0;
      i = 0;
      while (i != 'd' && i != 'w' && i != 's' && i != '\33')
	i = ttgetch ();
      switch (i)
	{
	case 'd':
	  lprcat ("deposit\n");
	  cltoeoln ();
	  lprcat ("How much? ");
	  amt = readnum ((int) cdesc[GOLD]);
	  if (amt > cdesc[GOLD])
	    {
	      lprcat ("You don't have that much.");
	      nap (NAPTIME);
	    }
/* Added if statement for catching 0 value to deposit. -Gibbon */
	  if (amt == 0)
	    {
	      lprcat ("You cannot deposit nothing");
	      nap (NAPTIME);
	    }
	  else
	    {
	      cdesc[GOLD] -= amt;
	      cdesc[BANKACCOUNT] += amt;
	    }
	  break;

	case 'w':
	  lprcat ("withdraw\nHow much? ");
	  amt = readnum ((int) cdesc[BANKACCOUNT]);

	  if (amt > cdesc[BANKACCOUNT])
	    {
	      lprcat ("\nYou don't have that much in the bank!");
	      nap (NAPTIME);
	    }
	  else
	    {
	      cdesc[GOLD] += amt;
	      cdesc[BANKACCOUNT] -= amt;
	    }
	  break;

	case 's':
	  lprcat ("\nWhich stone would you like to sell? ");
	  i = 0;
	  while ((i < 'a' || i > 'z') && i != '*' && i != '\33')
	    i = ttgetch ();
	  if (i == '*')
	    {
	      for (i = 0; i < 26; i++)
		{
		  if (gemvalue[i])
		    {
		      gems_sold = TRUE;
		      cdesc[GOLD] += gemvalue[i];
		      iven[i] = 0;
		      gemvalue[i] = 0;
		      k = gemorder[i];
		      cursor ((k % 2) * 40 + 1, (k >> 1) + 4);
		      lprintf ("%39s", "");
		    }
		}
	      if (!gems_sold)
		{
		  lprcat ("\nYou have no gems to sell!");
		  nap (NAPTIME);
		}
	    }
	  else if (i != '\33')
	    {
	      if (gemvalue[i = i - 'a'] == 0)
		{
		  lprintf ("\nItem %c is not a gemstone!", i + 'a');
		  nap (NAPTIME);
		  break;
		}
	      cdesc[GOLD] += gemvalue[i];
	      iven[i] = 0;
	      gemvalue[i] = 0;
	      k = gemorder[i];
	      cursor ((k % 2) * 40 + 1, (k >> 1) + 4);
	      attron(COLOR_PAIR(2));
	      lprintf ("%39s", "");
	      attroff(COLOR_PAIR(2));
	    }
	  break;

	case '\33':
	  return;
	};
	/*Fix for #38 -Gibbon*/
		cursor(1,16);
		attron(COLOR_PAIR(3));
		lprintf("%8d",(long)cdesc[BANKACCOUNT]);
		attroff(COLOR_PAIR(3));
		cursor(1,19);
		attron(COLOR_PAIR(3));
		lprintf("%8d",(long)cdesc[GOLD]);
		attroff(COLOR_PAIR(3));
    }
}

/*
function for the trading post
*/
static void
otradhead (void)
{
  screen_clear();
  lprcat
    ("Welcome to the Larn Trading Post.  We buy items that explorers no longer find\n");
  lprcat
    ("useful.  Since the condition of the items you bring in is not certain,\n");
  lprcat
    ("and we incur great expense in reconditioning the items, we usually pay\n");
  lprcat
    ("only 20% of their value were they to be new.  If the items are badly\n");
  lprcat ("damaged, we will pay only 10% of their new value.\n\n");

  lprcat ("Here are the items we would be willing to buy from you:\n");
}


static int tradorder[26];	/* screen locations for trading post inventory */

static void
otradiven (void)
{
  int i, j;

  /* Print user's inventory like bank */
  for (j = i = 0; i < 26; i++)
    if (iven[i])
      {
	cursor ((j % 2) * 40 + 1, (j >> 1) + 8);
	tradorder[i] = 0;	/* init position on screen to zero */
	switch (iven[i])
	  {
	  case OPOTION:
	    if (potionname[ivenarg[i]][0] != 0)
	      {
		tradorder[i] = j++;	/* will display only if identified */
		lprintf ("%c) %s", i + 'a', objectname[iven[i]]);
		lprintf (" of%s", potionname[ivenarg[i]]);
	      }
	    break;
	  case OSCROLL:
	    if (scrollname[ivenarg[i]][0] != 0)
	      {
		tradorder[i] = j++;	/* will display only if identified */
		lprintf ("%c) %s", i + 'a', objectname[iven[i]]);
		lprintf (" of%s", scrollname[ivenarg[i]]);
	      }
	    break;
	  case OLARNEYE:
	  case OBOOK:
	  case OSPIRITSCARAB:
	  case ODIAMOND:
	  case ORUBY:
	  case OEMERALD:
	  case OCHEST:
	  case OSAPPHIRE:
	  case OCUBEofUNDEAD:
	  case OCOOKIE:
	  case ONOTHEFT:
	    tradorder[i] = j++;	/* put on screen */
	    attron(COLOR_PAIR(4));
	    lprintf ("%c) ",i + 'a');
	    attroff(COLOR_PAIR(4));
	    lprintf("%s",objectname[iven[i]]);
	    break;
	  default:
	    tradorder[i] = j++;	/* put on screen */
	    attron(COLOR_PAIR(4));
	    lprintf ("%c) ",i + 'a'); 
	    attroff(COLOR_PAIR(4));
	    lprintf("%s",objectname[iven[i]]);
	    if (ivenarg[i] > 0)
	    {
	      lprintf (" +%d", (int) ivenarg[i]);
	      }
	    else if (ivenarg[i] < 0) {
	      lprintf (" %d", (int) ivenarg[i]);
	      }
	    break;
	  }
      }
    else
      tradorder[i] = 0;		/* make sure order array is clear */
}



static void
cleartradiven (int i)
{
  int j;

  j = tradorder[i];

  cursor ((j % 2) * 40 + 1, (j >> 1) + 8);
attron(COLOR_PAIR(2));
  lprintf ("%39s", "");
  attroff(COLOR_PAIR(2));

  tradorder[i] = 0;
}



void
otradepost (void)
{
  int i, j, isub, izarg, found;
  int value;

  dnditm = dndcount = 0;
  otradhead ();
  otradiven ();

  for (;;)
    {
      cursor (1, 23);
      lprcat ("\nWhat item do you want to sell to us [");
      lstandout ("escape");
      lprcat ("] ? ");

      /* display gold in inventory ~Gibbon */
      cursor(1,22);
      amtgoldtrad();

      i = 0;
      while ((i > 'z' || i < 'a') && i != 12 && i != '\33')
	i = ttgetch ();
      if (i == '\33')
	{

	  recalc ();
	  drawscreen ();
	  return;
	}
      for (;;)			/* inner loop for simpler control */
	{
	  if (i == 12)
	    {
	      screen_clear();
	      otradhead ();
	      otradiven ();
	      break;		/* leave inner while */
	    }

	  isub = i - 'a';
	  if (iven[isub] == 0)
	    {
	      lprintf ("\nYou don't have item %c!", isub + 'a');
	      nap (NAPTIME);
	      break;		/* leave inner while */
	    }
	  if (iven[isub] == OSCROLL)
	    if (scrollname[ivenarg[isub]][0] == 0)
	      {
		cnsitm ();
		break;		/* leave inner while */
	      }
	  if (iven[isub] == OPOTION)
	    if (potionname[ivenarg[isub]][0] == 0)
	      {
		cnsitm ();
		break;		/* leave inner while */
	      }
	  if (iven[isub] == ODIAMOND ||
	      iven[isub] == ORUBY ||
	      iven[isub] == OEMERALD || iven[isub] == OSAPPHIRE)
	    value = 20L * (ivenarg[isub] & 255);
	  else if (iven[isub] == OLARNEYE)
	    {
	      value = 50000 - (((gtime * 7) / 100) * 20);
	      if (value < 10000)
		value = 10000;
	    }
	  else
	    {
	      /* find object in dnd_item[] list for price info */
	      found = MAXITM;
	      for (j = 0; j < MAXITM; j++)
		if (dnd_item[j].obj == iven[isub])
		  {
		    found = j;
		    break;	/* leave for loop */
		  }
	      if (found == MAXITM)
		{
		  lprcat
		    ("\nSo sorry, but we are not authorized to accept that item.");
		  nap (NAPTIME);
		  break;	/* leave inner while */
		}
	      if (iven[isub] == OSCROLL || iven[isub] == OPOTION)
		value = 2 * (int) dnd_item[j + ivenarg[isub]].price;
	      else
		{
		  izarg = ivenarg[isub];
		  value = dnd_item[j].price;
		  /* appreciate if a +n object */
		  if (izarg >= 0)
		    value *= 2;
		  while ((izarg-- > 0)
			 && ((value = 14 * (67 + value) / 10) < 500000));
		}
	    }
	  /* we have now found the value of the item, and dealt with any error
	     cases.  Print the object's value, let the user sell it.
	   */
	   /*Fix for bug #40, overwriting some text at the same position to cancel out
	   the previous text. -Gibbon*/
	   cursor(1,23);
	  lprintf
	    ("\nItem (%c) is worth %d gold pieces to us.  Do you want to sell it? ",
	     i, (int) value);
	  yrepcount = 0;
	  if (getyn () == 'y')
	    {
	      lprcat ("yes\n");
	      cdesc[GOLD] += value;
	      if (cdesc[WEAR] == isub)
		cdesc[WEAR] = -1;
	      if (cdesc[WIELD] == isub)
		cdesc[WIELD] = -1;
	      if (cdesc[SHIELD] == isub)
		cdesc[SHIELD] = -1;
	      adjustcvalues (iven[isub], ivenarg[isub]);
	      iven[isub] = 0;
	      cleartradiven (isub);

	      /* clear and display functions again so gold is re-calculated 
	       * part of feature request from hymie0. ~Gibbon */
	      screen_clear();
	      otradhead ();
	      otradiven ();
	    }
	  else
	    {

	      /* refresh screen when saying no ~Gibbon */
	      screen_clear();
	      otradhead ();
	      otradiven ();
	    }
	  break;		/* exit inner while */
	}			/* end of inner while */
    }				/* end of outer while */
}				/* end of routine */




static void
cnsitm (void)
{
  lprcat ("\nSorry, we can't accept unidentified objects.");

  nap (2000);
}


/*
*  for the Larn Revenue Service
*
*  I have cleaned this up and extended it with reusable functions
*  with exception of lrs_welcome_text() which simply clears the 
*  screen and redraws. ~Gibbon.
*/
void
olrs (void)
{
  int i, first;
  int amt;

  lrs_welcome_text();
  /* disable signals */
  first = 1;
  for (;;)
    {
      if (first)
	{
	  first = 0;
	  /* this is gonna go at somepoint. ~Gibbon*/
	  goto nxt;
	}
      setscroll();
      cursor(1,21);
      lprcat ("\n\nYour wish? [(");
      lstandout ("p");
      lprcat (") pay taxes, or ");
      lstandout ("escape");
      lprcat ("]  ");
      yrepcount = 0;
      i = 0;
      
      while (i != 'p' && i != '\33')
      {
      	i = ttgetch ();
      }
      switch (i)
      {
      case 'p':
	  	lprcat ("pay taxes\nHow much? ");
	  	amt = readnum ((int) cdesc[GOLD]);
	  	
	  if (amt > cdesc[GOLD])
	    {
	      lprcat ("  You don't have that much.\n");
	    }
	  else
	    {
	    	/*Fix for bug #23 ~Gibbon*/
	      cdesc[GOLD] -= paytaxes ((int) amt);
		  lrs_welcome_text();
	    }
	  break;
	  
	case '\33':
		setscroll();
	  	drawscreen();
	  	return;
	  };

    nxt:
      cursor (1, 6);
      if (outstanding_taxes > 0)
      {
      	lprintf ("You presently owe %d gp in taxes.  ",
		   (int) outstanding_taxes);
	  }
	  else
	  {
	  lprcat ("You do not owe us any taxes.           ");
	  }
	  
      cursor (1, 19);
      if (cdesc[GOLD] > 0)
      {
      	amtgoldtrad();
      }
      else
      {
	  	lprcat ("You have no gold pieces.  ");
	  }
	}
}
