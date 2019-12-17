/* object.c */
#include <stdlib.h>
#include <curses.h>
#include "includes/action.h"
#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/create.h"
#include "includes/display.h"
#include "includes/fortune.h"
#include "includes/global.h"
#include "includes/inventory.h"
#include "includes/io.h"
#include "includes/main.h"
#include "includes/monster.h"
#include "includes/moreobj.h"
#include "includes/object.h"
#include "includes/regen.h"
#include "includes/scores.h"
#include "includes/spells.h"
#include "includes/sysdep.h"

static void ostairs (int);

static void opotion (int);

static void oscroll (int);

static void opit (void);
static void obottomless (void);
static void ostatue (void);
static void omirror (void);
static void obook (void);

static void ocookie (void);

static void ogold (int);

static void prompt_enter (void);

static void prompt_volshaft (int);

static void o_open_door (void);
static void o_closed_door (void);




/* LOOK_FOR_OBJECT
subroutine to look for an object and give the player his options if an object
was found.

do_ident;   identify item: T/F 
do_pickup;  pickup item:   T/F 
do_action;  prompt for actions on object: T/F 
*/
void
lookforobject (char do_ident, char do_pickup, char do_action)
{
  int i, j;

  /* can't find objects if time is stopped    */
  if (cdesc[TIMESTOP])
    return;
  i = item[playerx][playery];
  if (i == 0)
    return;
  j = iarg[playerx][playery];
  showcell (playerx, playery);
  cursors ();
  yrepcount = 0;
  switch (i)
    {
    case OGOLDPILE:
    case OMAXGOLD:
    case OKGOLD:
    case ODGOLD:
      lprcat ("\nYou have found some gold!");
      ogold (i);
      break;

    case OPOTION:
      if (do_ident)
	{
	  lprcat ("\nYou have found a magic potion");
	  if (potionname[j][0])
	    lprintf (" of %s", &potionname[j][1]);
	}
      if (do_pickup)
	if (take (OPOTION, j) == 0)
	  forget ();
      if (do_action)
	opotion (j);
      break;

    case OSCROLL:
      if (do_ident)
	{
	  lprcat ("\nYou have found a magic scroll");
	  if (scrollname[j][0])
	    lprintf (" of %s", &scrollname[j][1]);
	}
      if (do_pickup)
	if (take (OSCROLL, j) == 0)
	  forget ();
      if (do_action)
	oscroll (j);
      break;

    case OALTAR:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nThere is a Holy Altar here!");
      if (do_action)
	oaltar ();
      break;

    case OBOOK:
      if (do_ident)
	lprcat ("\nYou have found a book.");
      if (do_pickup)
	if (take (OBOOK, j) == 0)
	  forget ();
      if (do_action)
	obook ();
      break;

    case OCOOKIE:
      if (do_ident)
	lprcat ("\nYou have found a fortune cookie.");
      if (do_pickup)
	if (take (OCOOKIE, 0) == 0)
	  forget ();
      if (do_action)
	ocookie ();
      break;

    case OTHRONE:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprintf ("\nThere is %s here!", objectname[i]);
      if (do_action)
	othrone (0);
      break;

    case OTHRONE2:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprintf ("\nThere is %s here!", objectname[i]);
      if (do_action)
	othrone (1);
      break;

    case ODEADTHRONE:
      if (do_ident)
	lprintf ("\nThere is %s here!", objectname[i]);
      if (do_action)
	odeadthrone ();
      break;

    case OPIT:
      /* always perform these actions. */
      lprcat ("\nYou're standing at the top of a pit.");
      opit ();
      break;

    case OSTAIRSUP:		/* up */
      if (do_ident)
	lprcat ("\nThere is a circular staircase here");
      if (do_action)
	ostairs(1);
	refresh();
      break;

    case OFOUNTAIN:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nThere is a fountain here");
      if (do_action)
	ofountain ();
      break;

    case OSTATUE:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nYou are standing in front of a statue");
      if (do_action)
	ostatue ();
      break;

    case OCHEST:
      if (do_ident)
	lprcat ("\nThere is a chest here");
      if (do_pickup)
	if (take (OCHEST, j) == 0)
	  forget ();
      if (do_action)
	ochest ();
      break;

    case OSCHOOL:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nYou have found the College of Larn.");
      if (do_action)
	prompt_enter ();
      break;

    case OMIRROR:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nThere is a mirror here");
      if (do_action)
	omirror ();
      break;

    case OBANK2:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nYou have found a branch office of the bank of Larn.");
      if (do_action)
	prompt_enter ();
      break;

    case OBANK:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nYou have found the bank of Larn.");
      if (do_action)
	prompt_enter ();
      break;

    case ODEADFOUNTAIN:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nThere is a dead fountain here");
      break;

    case ODNDSTORE:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nThere is a DND store here.");
      if (do_action)
	prompt_enter ();
      break;

    case OSTAIRSDOWN:		/* down */
      if (do_ident)
	lprcat ("\nThere is a circular staircase here");
      if (do_action)
	ostairs(-1);
	refresh();
      break;

    case OOPENDOOR:
      if (do_ident)
	lprintf ("\nYou have found %s", objectname[i]);
      if (do_action)
	o_open_door ();
      break;

    case OCLOSEDDOOR:
      if (do_ident)
	lprintf ("\nYou have found %s", objectname[i]);
      if (do_action)
	o_closed_door ();
      break;

    case OENTRANCE:
      if (do_ident)
	lprcat ("\nYou have found ");
      lprcat (objectname[i]);
      if (do_action)
	prompt_enter ();
      break;

    case OVOLDOWN:
      if (do_ident)
	lprcat ("\nYou have found ");
      lprcat (objectname[i]);
      if (do_action)
	prompt_volshaft (-1);
      break;

    case OVOLUP:
      if (do_ident)
	lprcat ("\nYou have found ");
      lprcat (objectname[i]);
      if (do_action)
	prompt_volshaft (1);
      break;

    case OIVTELETRAP:
      if (rnd (11) < 6)
	return;
      item[playerx][playery] = OTELEPORTER;
      know[playerx][playery] = KNOWALL;
      /* fall through to OTELEPORTER case below!!! */

    case OTELEPORTER:
      lprcat ("\nZaaaappp!  You've been teleported!\n");
      nap (NAPTIME);
      oteleport (0);
      refresh();
      break;

    case OTRAPARROWIV:		/* for an arrow trap */
      if (rnd (17) < 13)
	return;
      item[playerx][playery] = OTRAPARROW;
      know[playerx][playery] = 0;
      /* fall through to OTRAPARROW case below!!! */

    case OTRAPARROW:
      lprcat ("\nYou are hit by an arrow");
      lastnum = 259;
      losehp (rnd (10) + level);
      bottomhp ();
      return;

    case OIVDARTRAP:		/* for a dart trap */
      if (rnd (17) < 13)
	return;
      item[playerx][playery] = ODARTRAP;
      know[playerx][playery] = 0;
      /* fall through to ODARTTRAP case below!!! */

    case ODARTRAP:
      lprcat ("\nYou are hit by a dart");
      lastnum = 260;
      losehp (rnd (5));
      if ((--cdesc[STRENGTH]) < 3)
	cdesc[STRENGTH] = 3;
      bottomline ();
      return;

    case OIVTRAPDOOR:		/* for a trap door */
      if (rnd (17) < 13)
	return;
      item[playerx][playery] = OTRAPDOOR;
      know[playerx][playery] = KNOWALL;
      /* fall through to OTRAPDOOR case below!!! */

    case OTRAPDOOR:
      lastnum = 272;		/* a trap door */
      if ((level == MAXLEVEL - 1) || (level == MAXLEVEL + MAXVLEVEL - 1))
	{
	  lprcat ("\nYou fell through a bottomless trap door!");
	  nap (NAPTIME);
	  died (271);
	}
      i = rnd (5 + level);
      lprintf ("\nYou fall through a trap door!  You lose %d hit points.",
	       (long) i);
      losehp(i);
      nap(NAPTIME);
      newcavelevel(level + 1);
      draws(0, MAXX, 0, MAXY);
      bot_linex();
      return;

    case OTRADEPOST:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nYou have found the Larn trading Post.");
      if (do_action)
	prompt_enter ();
      return;

    case OHOME:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nYou have found your way home.");
      if (do_action)
	prompt_enter ();
      return;

    case OWALL:
      break;

    case OANNIHILATION:
      died (283);		/* annihilated by sphere of annihilation */
      return;

    case OLRS:
      if (nearbymonst ())
	return;
      if (do_ident)
	lprcat ("\nThere is an LRS office here.");
      if (do_action)
	prompt_enter ();
      break;

    default:
      if (do_ident)
	{
	  lprintf ("\nYou have found %s ", objectname[i]);
	  switch (i)
	    {
	    case ODIAMOND:
	    case ORUBY:
	    case OEMERALD:
	    case OSAPPHIRE:
	    case OSPIRITSCARAB:
	    case OORBOFDRAGON:
	    case OCUBEofUNDEAD:
	    case ONOTHEFT:
	      break;

	    default:
	      if (j > 0)
		lprintf ("+ %d", (int) j);
	      else if (j < 0)
		lprintf (" %d", (int) j);
	      break;
	    }
	}
      if (do_pickup)
	if (take (i, j) == 0)
	  forget ();
      if (do_action)
	{
	  char tempc = 0;

	  lprcat ("\nDo you want to (t) take it");
	  iopts ();
	  while (tempc != 't' && tempc != 'i' && tempc != '\33')
	    tempc = ttgetch ();
	  if (tempc == 't')
	    {
	      lprcat ("take");
	      if (take (i, j) == 0)
		forget ();
	      return;
	    }
	  ignore ();
	}
      break;
    };
}



/*
* subroutine to process the stair cases if dir > 0 the up else down 
*/
static void
ostairs (int dir)
{

  lprcat ("\nDo you (s) stay here ");
  if (dir > 0)
    lprcat ("or (u) go up? ");
  else
    lprcat ("or (d) go down? ");

  for (;;)
    {
      switch (ttgetch ())
	{
	case '\33':
	case 's':
	case 'i':
	  lprcat ("stay here");
	  return;

	case 'u':
	  lprcat ("go up");
	  act_up_stairs ();
	  return;

	case 'd':
	  lprcat ("go down");
	  act_down_stairs ();
	  return;
	};
    }
}



/*
* subroutine to handle a teleport trap +/- 1 level maximum 
*/
void
oteleport (int err)
{
  int tmp;
  if (err)
	if (rnd (151) < 3)
	{
		/* Fix for bug #10 ~Gibbon*/
		cursor(1,19);
		lprcat("\nYou died by teleporting into solid rock.");
		nap(4000);
  		died(264);	/* stuck in a rock */
	}
  cdesc[TELEFLAG] = 1;		/* show ?? on bottomline if been teleported    */
  if (level == 0)
    tmp = 0;
  else if (level < MAXLEVEL)
    {
      tmp = rnd (5) + level - 3;
      if (tmp >= MAXLEVEL)
	tmp = MAXLEVEL - 1;
      if (tmp < 1)
	tmp = 1;
    }
  else
    {
      tmp = rnd (3) + level - 2;
      if (tmp >= MAXLEVEL + MAXVLEVEL)
	tmp = MAXLEVEL + MAXVLEVEL - 1;
      if (tmp < MAXLEVEL)
	tmp = MAXLEVEL;
    }
  playerx = rnd (MAXX - 2);
  playery = rnd (MAXY - 2);
  if (level != tmp)
  {
  	newcavelevel (tmp);
  	positionplayer ();
  	draws (0, MAXX, 0, MAXY);
  	bot_linex ();
  	refresh();
  }
}

/*
* function to process a potion 
*/
static void
opotion (int pot)
{

  lprcat ("\nDo you (d) drink it, (t) take it");
  iopts ();

  for (;;)
    {
      switch (ttgetch ())
	{
	case '\33':
	case 'i':
	  ignore ();
	  return;

	case 'd':
	  lprcat ("drink\n");
	  forget ();		/* destroy potion  */
	  quaffpotion (pot, TRUE);
	  return;

	case 't':
	  lprcat ("take\n");
	  if (take (OPOTION, pot) == 0)
	    forget ();
	  return;
	};
    }
}



/*
* function to drink a potion 
*
* Also used to perform the action of a potion without quaffing a potion (see
* invisible capability when drinking from a fountain). 
*/
void
quaffpotion (int pot, int set_known)
{
  int i, j, k;

  /* check for within bounds */
  if (pot < 0 || pot >= MAXPOTION)
    return;

  /*
   * if player is to know this potion (really quaffing one), make it
   * known 
   */
  if (set_known)
    potionname[pot][0] = ' ';

  switch (pot)
    {
    case 0:
      lprcat ("\nYou fall asleep. . .");
      i = rnd (11) - (cdesc[CONSTITUTION] >> 2) + 2;
      while (--i > 0)
	{
	  parse2 ();
	  nap (NAPTIME);
	}
      cursors ();
      lprcat ("\nYou woke up!");
      return;

    case 1:
      lprcat ("\nYou feel better");
      if (cdesc[HP] == cdesc[HPMAX])
	raisemhp (1);
      else if ((cdesc[HP] += rnd (20) + 20 + cdesc[LEVEL]) > cdesc[HPMAX])
	cdesc[HP] = cdesc[HPMAX];
      break;

    case 2:
      lprcat ("\nSuddenly, you feel much more skillful!");
      raiselevel ();
      raisemhp (1);
      return;

    case 3:
      lprcat ("\nYou feel strange for a moment");
      cdesc[rund (6)]++;
      break;

    case 4:
      lprcat ("\nYou feel more self confident!");
      cdesc[WISDOM] += rnd (2);
      break;

    case 5:
      lprcat ("\nWow!  You feel great!");
      if (cdesc[STRENGTH] < 12)
	cdesc[STRENGTH] = 12;
      else
	cdesc[STRENGTH]++;
      break;

    case 6:
      lprcat ("\nYour charm went up by one!");
      cdesc[CHARISMA]++;
      break;

    case 7:
      lprcat ("\nYou become dizzy!");
      if (--cdesc[STRENGTH] < 3)
	cdesc[STRENGTH] = 3;
      break;

    case 8:
      lprcat ("\nYour intelligence went up by one!");
      cdesc[INTELLIGENCE]++;
      break;

    case 9:
      lprcat ("\nYou sense the presence of objects!");
      nap (NAPTIME);
      if (cdesc[BLINDCOUNT])
	return;
      for (i = 0; i < MAXY; i++)
	for (j = 0; j < MAXX; j++)
	  switch (item[j][i])
	    {
	    case OPLATE:
	    case OCHAIN:
	    case OLEATHER:
	    case ORING:
	    case OSTUDLEATHER:
	    case OSPLINT:
	    case OPLATEARMOR:
	    case OSSPLATE:
	    case OSHIELD:
	    case OSWORDofSLASHING:
	    case OHAMMER:
	    case OSWORD:
	    case O2SWORD:
	    case OHSWORD:
	    case OSPEAR:
	    case ODAGGER:
	    case OBATTLEAXE:
	    case OLONGSWORD:
	    case OGREATSWORD:
	    case ORINGOFEXTRA:
	    case OREGENRING:
	    case OPROTRING:
	    case OENERGYRING:
	    case ODEXRING:
	    case OSTRRING:
	    case OCLEVERRING:
	    case ODAMRING:
	    case OBELT:
	    case OSCROLL:
	    case OPOTION:
	    case OBOOK:
	    case OCHEST:
	    case OAMULET:
	    case OORBOFDRAGON:
	    case OSPIRITSCARAB:
	    case OCUBEofUNDEAD:
	    case ONOTHEFT:
	    case OCOOKIE:
	      know[j][i] = HAVESEEN;
	      show1cell (j, i);
	      break;
	    }
      showplayer ();
      return;

    case 10:			/* monster detection */
      lprcat ("\nYou detect the presence of monsters!");
      nap (NAPTIME);
      if (cdesc[BLINDCOUNT])
	return;
      for (i = 0; i < MAXY; i++)
	for (j = 0; j < MAXX; j++)
	  if (mitem[j][i] && (monstnamelist[mitem[j][i]] != floorc))
	    {
	      know[j][i] = HAVESEEN;
	      show1cell (j, i);
	    }
      return;

    case 11:
      lprcat ("\nYou stagger for a moment . .");
      for (i = 0; i < MAXY; i++)
	for (j = 0; j < MAXX; j++)
	  know[j][i] = 0;
      nap (1000);
      draws (0, MAXX, 0, MAXY);	/* potion of forgetfulness */
      return;

    case 12:
      lprcat ("\nThis potion has no taste to it");
      return;

    case 13:
      lprcat ("\nYou can't see anything!");	/* blindness */
      cdesc[BLINDCOUNT] += 500;
      return;

    case 14:
      lprcat ("\nYou feel confused");
      cdesc[CONFUSE] += 20 + rnd (9);
      return;

    case 15:
      lprcat ("\nWOW!!!  You feel Super-fantastic!!!");
      if (cdesc[HERO] == 0)
	for (i = 0; i < 6; i++)
	  cdesc[i] += 11;
      cdesc[HERO] += 250;
      break;

    case 16:
      lprcat ("\nYou have a greater intestinal constitude!");
      cdesc[CONSTITUTION]++;
      break;

    case 17:
      lprcat ("\nYou now have incredibly bulging muscles!!!");
      if (cdesc[GIANTSTR] == 0)
	cdesc[STREXTRA] += 21;
      cdesc[GIANTSTR] += 700;
      break;

    case 18:
      lprcat ("\nYou feel a chill run up your spine!");
      cdesc[FIRERESISTANCE] += 1000;
      break;

    case 19:
      lprcat ("\nYou feel greedy . . .");
      nap (NAPTIME);
      if (cdesc[BLINDCOUNT])
	return;
      for (i = 0; i < MAXY; i++)
	for (j = 0; j < MAXX; j++)
	  {
	    k = item[j][i];
	    if ((k == ODIAMOND) ||
		(k == ORUBY) ||
		(k == OEMERALD) ||
		(k == OMAXGOLD) ||
		(k == OSAPPHIRE) || (k == OLARNEYE) || (k == OGOLDPILE))
	      {
		know[j][i] = HAVESEEN;
		show1cell (j, i);
	      }
	  }
      showplayer ();
      return;

    case 20:
      lprcat ("\nYou feel all better now!");
      cdesc[HP] = cdesc[HPMAX];
      break;			/* instant healing */

    case 21:
      lprcat ("\nYou don't seem to be affected");
      return;			/* cure dianthroritis */

    case 22:
      lprcat ("\nYou feel a sickness engulf you");	/* poison */
      cdesc[HALFDAM] += 200 + rnd (200);
      return;

    case 23:
      lprcat ("\nYou feel your vision sharpen");	/* see invisible */
      cdesc[SEEINVISIBLE] += rnd (1000) + 400;
      monstnamelist[INVISIBLESTALKER] = 'I';
      return;
    };
  bottomline ();		/* show new stats      */
  return;
}



/*
* function to process a magic scroll 
*/
static void
oscroll (int typ)
{
  lprcat ("\nDo you ");
  if (cdesc[BLINDCOUNT] == 0)
    lprcat ("(r) read it, ");
  lprcat ("(t) take it");
  iopts ();
  for (;;)
    {
      switch (ttgetch ())
	{
	case '\33':
	case 'i':
	  ignore ();
	  return;

	case 'r':
	  if (cdesc[BLINDCOUNT])
	    break;
	  lprcat ("read");
	  forget ();
	  if (typ == 2 || typ == 15)
	    {
	      show1cell (playerx, playery);
	      cursors ();
	    }
	  /* destroy it  */ read_scroll (typ);
	  return;

	case 't':
	  lprcat ("take");
	  if (take (OSCROLL, typ) == 0)
	    forget ();		/* destroy it  */
	  return;
	};
    }
}





/*
* data for the function to read a scroll 
*/
static int xh, yh, yl, xl;
static int curse[] = { BLINDCOUNT, CONFUSE, AGGRAVATE, HASTEMONST, ITCHING,
  LAUGHING, DRAINSTRENGTH, CLUMSINESS, INFEEBLEMENT,
  HALFDAM
};

static int exten[] = { PROTECTIONTIME, DEXCOUNT, STRCOUNT, CHARMCOUNT,
  INVISIBILITY, CANCELLATION, HASTESELF, GLOBE,
  SCAREMONST, HOLDMONST, TIMESTOP
};

static int time_change[] =
  { HASTESELF, HERO, ALTPRO, PROTECTIONTIME, DEXCOUNT,
  STRCOUNT, GIANTSTR, CHARMCOUNT, INVISIBILITY,
  CANCELLATION, HASTESELF, AGGRAVATE, SCAREMONST,
  STEALTH, AWARENESS, HOLDMONST, HASTEMONST,
  FIRERESISTANCE, GLOBE, SPIRITPRO, UNDEADPRO,
  HALFDAM, SEEINVISIBLE, ITCHING, CLUMSINESS, WTW
};



/*
* function to adjust time when time warping and taking courses in school
*/
void
adjtimel (int tim)
{
  int j;

  for (j = 0; j < 26; j++)	/* adjust time related parameters */
    if (cdesc[time_change[j]])
      if ((cdesc[time_change[j]] -= tim) < 1)
	cdesc[time_change[j]] = 1;

  regen ();
}



/*
* function to read a scroll 
*/
void
read_scroll (int typ)
{
  int i, j;

  if (typ < 0 || typ >= MAXSCROLL)
    return;			/* be sure we are within bounds */
  scrollname[typ][0] = ' ';
  switch (typ)
    {
    case 0:
      lprcat ("\nYour armor glows for a moment");
      enchantarmor ();
      return;

    case 1:
      lprcat ("\nYour weapon glows for a moment");
      enchweapon ();
      return;			/* enchant weapon */

    case 2:
      lprcat ("\nYou have been granted enlightenment!");
      yh = min (playery + 7, MAXY);
      xh = min (playerx + 25, MAXX);
      yl = max (playery - 7, 0);
      xl = max (playerx - 25, 0);
      for (i = yl; i < yh; i++)
	for (j = xl; j < xh; j++)
	  know[j][i] = KNOWALL;
      draws (xl, xh, yl, yh);
      refresh();
      return;

    case 3:
      lprcat ("\nThis scroll seems to be blank");
      return;

    case 4:
      createmonster (makemonst (level + 1));
      return;			/* this one creates a monster  */

    case 5:
      something (level);	/* create artifact     */
      return;

    case 6:
      lprcat ("\nSomething isn't right...");
      cdesc[AGGRAVATE] += 800;
      return;			/* aggravate monsters */

    case 7:
      gtime += (i = rnd (1000) - 850);	/* time warp */
      if (i >= 0)
	lprintf ("\nYou went forward in time by %d mobuls",
		 (int) ((i + 99) / 100));
      else
	lprintf ("\nYou went backward in time by %d mobuls",
		 (int) (-(i + 99) / 100));
      adjtimel ((int) i);	/* adjust time for time warping */
      return;

    case 8:
      lprcat ("\nYour surroundings change");
      oteleport (0);
      refresh();
      return;			/* teleportation */

    case 9:
      lprcat ("\nYou feel extra alert");
      cdesc[AWARENESS] += 1800;
      return;			/* expanded awareness   */

    case 10:
      lprcat ("\nSomething isn't right...");
      cdesc[HASTEMONST] += rnd (55) + 12;
      return;			/* haste monster */

    case 11:
      lprcat ("\nSomething isn't right...");
      for (i = 0; i < MAXY; i++)
	for (j = 0; j < MAXX; j++)
	  if (mitem[j][i])
	    hitp[j][i] = monster[mitem[j][i]].hitpoints;
      return;			/* monster healing */
    case 12:
      cdesc[SPIRITPRO] += 300 + rnd (200);
      bottomline ();
      return;			/* spirit protection */

    case 13:
      cdesc[UNDEADPRO] += 300 + rnd (200);
      bottomline ();
      return;			/* undead protection */

    case 14:
      cdesc[STEALTH] += 250 + rnd (250);
      bottomline ();
      return;			/* stealth */

    case 15:
      lprcat ("\nYou have been granted enlightenment!");	/* magic mapping */
      for (i = 0; i < MAXY; i++)
	for (j = 0; j < MAXX; j++)
	  know[j][i] = KNOWALL;
      draws (0, MAXX, 0, MAXY);
      cdesc[TELEFLAG] = 0;		/* magic map tells you what floor you're on */
      refresh();
      return;

    case 16:
      cdesc[HOLDMONST] += 30;
      bottomline ();
      return;			/* hold monster */

    case 17:
      lprcat ("\nYou feel someone eyeing your belongings");
      for (i = 0; i < 26; i++)	/* gem perfection */
	switch (iven[i])
	  {
	  case ODIAMOND:
	  case ORUBY:
	  case OEMERALD:
	  case OSAPPHIRE:
	    j = ivenarg[i];
	    j &= 255;
	    j <<= 1;
	    if (j > 255)
	      j = 255;		/* double value */
	    ivenarg[i] = j;
	    break;
	  }
      break;

    case 18:
      lprcat ("\nYou feel a twitch at the base of your skull");
      for (i = 0; i < 11; i++)
	cdesc[exten[i]] <<= 1;	/* spell extension */
      break;

    case 19:
      lprcat ("\nYou feel someone eyeing your belongings");
      for (i = 0; i < 26; i++)	/* identify */
	{
	  if (iven[i] == OPOTION)
	    potionname[ivenarg[i]][0] = ' ';
	  if (iven[i] == OSCROLL)
	    scrollname[ivenarg[i]][0] = ' ';
	}
      break;

    case 20:
      lprcat ("\nYou sense a benign presence");
      for (i = 0; i < 10; i++)	/* remove curse */
	if (cdesc[curse[i]])
	  cdesc[curse[i]] = 1;
      break;

    case 21:
      annihilate ();
      break;			/* scroll of annihilation */

    case 22:
      godirect (22, 150, "The ray hits the %s", 0, ' ');	/* pulverization */
      break;
    case 23:
      lprcat ("\nYou sense a benign presence");
      cdesc[LIFEPROT]++;
      break;			/* life protection */
    };
}

/* FIXES for bool bug */
static void
opit (void)
{
  int i;
  if (rnd (101) < 81)
    {
      if (rnd (70) > 9 * cdesc[DEXTERITY] - packweight () || rnd (101) < 5)
	{
	  if (level == MAXLEVEL - 1)
	    obottomless ();
	  else if (level == MAXLEVEL + MAXVLEVEL - 1)
	    obottomless ();
	  else
	    {
	      if (rnd (101) < 20)
		{
		  i = 0;
		  lprcat
		    ("\nYou fell into a pit!  Your fall is cushioned by an unknown force\n");

		}
	      else
		{
		  i = rnd (level * 3 + 3);
		  lprintf
		    ("\nYou fell into a pit!  You suffer %ld hit points damage",
		     (long) i);
		  lastnum = 261;	/* if he dies scoreboard * will say so */

		}
	      losehp (i);
	      nap (2000);
	      newcavelevel (level + 1);
	      draws (0, MAXX, 0, MAXY);

	    }

	}

    }

}

static void
obottomless (void)
{
  lprcat ("\nYou fell into a bottomless pit!");
  nap (3000);
  died (262);
}


static void
ostatue (void)
{

}

/* I will add something here, for now it's a placeholder -Gibbon*/
static void
omirror (void)
{
  lprcat ("\nMirror mirror on the wall..");
}

static void
obook (void)
{
  lprcat ("\nDo you ");
  if (cdesc[BLINDCOUNT] == 0)
    lprcat ("(r) read it, ");
  lprcat ("(t) take it");
  iopts ();
  for (;;)
    {
      switch (ttgetch ())
	{
	case '\33':
	case 'i':
	  ignore ();
	  return;

	case 'r':
	  if (cdesc[BLINDCOUNT])
	    break;
	  lprcat ("read");
	  /* no more book */ readbook (iarg[playerx][playery]);
	  forget ();
	  return;

	case 't':
	  lprcat ("take");
	  if (take (OBOOK, iarg[playerx][playery]) == 0)
	    forget ();		/* no more book */
	  return;
	};
    }
}



/*
* function to read a book 
*/
void
readbook (int lev)
{
  int i, tmp;

  if (lev <= 3)
    {

      tmp = splev[lev];
      if (tmp == 0)
	tmp = 1;

      i = rund (tmp);

    }
  else
    {

      tmp = splev[lev] - 9;
      if (tmp == 0)
	tmp = 1;

      i = rnd (tmp + 9);
    }

  spelknow[i] = 1;

  lprintf ("\nSpell \"%s\":  %s\n%s", spelcode[i], spelname[i],
	   speldescript[i]);

  if (rnd (10) == 4)
    {

      lprcat ("\nYour int went up by one!");
      cdesc[INTELLIGENCE]++;
      bottomline ();
    }
}



static void
ocookie (void)
{

  lprcat ("\nDo you (e) eat it, (t) take it");
  iopts ();
  for (;;)
    {
      switch (ttgetch ())
	{
	case '\33':
	case 'i':
	  ignore ();
	  return;

	case 'e':
	  lprcat ("eat");
	  forget ();		/* no more cookie */
	  outfortune ();
	  return;

	case 't':
	  lprcat ("take");
	  if (take (OCOOKIE, 0) == 0)
	    forget ();		/* no more book */
	  return;
	};
    }
}



/*
* routine to pick up some gold -- if arg==OMAXGOLD then the pile is worth
* 100* the argument
*/
static void
ogold (int arg)
{
  int i;

  i = iarg[playerx][playery];
  if (arg == OMAXGOLD)
    i *= 100;
  else if (arg == OKGOLD)
    i *= 1000;
  else if (arg == ODGOLD)
    i *= 10;
  lprintf ("\nIt is worth %d!", (int) i);
  cdesc[GOLD] += i;
  bottomgold ();
  item[playerx][playery] = know[playerx][playery] = 0;	/* destroy gold    */
}



void
ohome (void)
{
  int i;


  for (i = 0; i < 26; i++)
    if (iven[i] == OPOTION)
      if (ivenarg[i] == 21)
	{
	  iven[i] = 0;		/* remove the potion of cure
				 * dianthroritis from
				 * inventory */
	  screen_clear();
	  lprcat
	    ("Congratulations.  You found a potion of cure dianthroritis.\n");
	  lprcat
	    ("\nFrankly, No one thought you could do it.  Boy!  Did you surprise them!\n");
	  if (gtime > TIMELIMIT)
	    {
	      lprcat
		("\nThe doctor has the sad duty to inform you that your daughter died");
	      lprcat
		("\nbefore your return.  There was nothing he could do without the potion.\n");
	      nap (NAPTIME);
	      died (269);
	    }
	  else
	    {
	      lprcat
		("\nThe doctor is now administering the potion, and in a few moments\n");
	      lprcat
		("your daughter should be well on her way to recovery.\n");
	      nap (NAPTIME);
	      lprcat ("\nThe potion is");
	      nap (NAPTIME);
	      lprcat (" working!  The doctor thinks that\n");
	      lprcat
		("your daughter will recover in a few days.  Congratulations!\n");
	      nap (NAPTIME);
	      died (263);
	    }
	}
  for (;;)
    {
      screen_clear();
      lprintf ("Welcome home %s.  Latest word from the doctor is not good.\n",
	       logname);

      if (gtime > TIMELIMIT)
	{
	  lprcat
	    ("\nThe doctor has the sad duty to inform you that your daughter died!\n");
	  lprcat
	    ("You didn't make it in time.  There was nothing he could do without the potion.\n");
	  nap (NAPTIME);
	  died (269);
	}
      lprcat
	("\nThe diagnosis is confirmed as dianthroritis.  He guesses that\n");
      lprintf
	("your daughter has only %d mobuls left in this world.  It's up to you,\n",
	 ((TIMELIMIT - gtime + 99) / 100));
      lprintf ("%s, to find the only hope for your daughter, the very rare\n",
	       logname);
      lprcat
	("potion of cure dianthroritis.  It is rumored that only deep in the\n");
      lprcat ("depths of the caves can this potion be found.\n\n\n");
      lprcat ("\n     ----- press ");
      lstandout ("return");
      lprcat (" to continue, ");
      lstandout ("escape");
      lprcat (" to leave ----- ");
      i = ttgetch ();
      while (i != '\33' && i != '\n')
	i = ttgetch ();
      drawscreen ();
      return;
    }
}



/* routine to save program space   */
void
iopts (void)
{

  lprcat (", or (i) ignore it? ");
}


void
ignore (void)
{

  lprcat ("ignore\n");
}





/*
* For prompt mode, prompt for entering a building.
*/
static void
prompt_enter (void)
{
  char i;

  lprcat ("\nDo you (g) go inside, or (i) stay here? ");
  i = 0;
  while ((i != 'g') && (i != 'i') && (i != '\33'))
    i = ttgetch ();
  if (i == 'g')
    enter ();
  else
    lprcat (" stay here");
}





/*
* For prompt mode, prompt for climbing up/down the volcanic shaft. 
*
* Takes one parameter: if it is negative, going down the shaft, otherwise,
* going up the shaft. 
*/
static void
prompt_volshaft (int dir)
{
  char i;

  lprcat ("\nDo you (c) climb ");
  if (dir > 0)
    lprcat ("up");
  else
    lprcat ("down");
  iopts ();

  i = 0;
  while ((i != 'c') && (i != 'i') && (i != '\33'))
    i = ttgetch ();

  if ((i == '\33') || (i == 'i'))
    {
      ignore ();
      return;
    }
  if (dir > 0)
    act_up_shaft ();
  else
    act_down_shaft ();
}




static void
o_open_door (void)
{
  char i;

  lprcat ("\nDo you (c) close it");
  iopts ();
  i = 0;
  while ((i != 'c') && (i != 'i') && (i != '\33'))
    i = ttgetch ();
  if ((i == '\33') || (i == 'i'))
    {
      ignore ();
      return;
    }
  lprcat ("close");
  forget ();
  item[playerx][playery] = OCLOSEDDOOR;
  iarg[playerx][playery] = 0;
  playerx = lastpx;
  playery = lastpy;
}



static void
o_closed_door (void)
{
  char i;

  lprcat ("\nDo you (o) try to open it");
  iopts ();
  i = 0;
  while ((i != 'o') && (i != 'i') && (i != '\33'))
    i = ttgetch ();
  if ((i == '\33') || (i == 'i'))
    {
      ignore ();
      playerx = lastpx;
      playery = lastpy;
      return;
    }
  else
    {
      lprcat ("open");
      /*
       * if he failed to open the door ... 
       */
      if (!act_open_door (playerx, playery))
	{
	  playerx = lastpx;
	  playery = lastpy;
	}
    }
}
