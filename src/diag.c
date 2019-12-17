/* diag.c */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/global.h"
#include "includes/io.h"
#include "includes/savelev.h"
#include "includes/scores.h"
#include "includes/store.h"
#include "includes/sysdep.h"
#include "includes/create.h"
#include "includes/diag.h"

#ifdef ANTICHEAT
static void greedy (void);
static void fsorry (void);
static void fcheat (void);
#endif


/*
***************************
DIAG -- dungeon diagnostics
***************************

subroutine to print out data for debugging
*/
#ifdef EXTRA
static int rndcount[16];

int
diag (void)
{
  int i, j;
  int hit, dam;

  cursors ();
  lwclose ();
  if (lcreat (diagfile) < 0)	/*  open the diagnostic file    */
    {
      lcreat ((char *) 0);
      lprcat ("\ndiagnostic failure\n");
      return (-1);
    }

  write (1, "\nDiagnosing . . .\n", 18);
  lprcat ("\n\nBeginning of DIAG diagnostics ----------\n");

  /*  for the character attributes    */

  lprintf ("\n\nPlayer attributes:\n\nHit points: %2d(%2d)", (int) cdesc[HP],
	   (int) cdesc[HPMAX]);
  lprintf
    ("\ngold: %d  Experience: %d  Character level: %d  Level in caverns: %d",
     (int) cdesc[GOLD], (int) cdesc[EXPERIENCE], (int) cdesc[LEVEL],
     (int) level);
  lprintf ("\nTotal types of monsters: %d", (int) MAXMONST + 8);

  lprcat ("\f\nHere's the dungeon:\n\n");

  i = level;
  for (j = 0; j < MAXLEVEL + MAXVLEVEL; j++)
    {
      newcavelevel (j);
      lprintf ("\nMaze for level %s:\n", levelname[level]);
      diagdrawscreen ();
    }
  newcavelevel (i);

  lprcat ("\f\nNow for the monster data:\n\n");
  lprcat
    ("   Monster Name      LEV  AC   DAM  ATT  DEF    GOLD   HP     EXP   \n");
  lprcat
    ("--------------------------------------------------------------------------\n");
  for (i = 0; i <= MAXMONST + 8; i++)
    {
      lprintf ("%19s  %2d  %3d ", monster[i].name, (int) monster[i].level,
	       (int) monster[i].armorclass);
      lprintf (" %3d  %3d  %3d  ", (int) monster[i].damage,
	       (int) monster[i].attack, (int) monster[i].defense);
      lprintf ("%6d  %3d   %6d\n", (int) monster[i].gold,
	       (int) monster[i].hitpoints, (int) monster[i].experience);
    }

  lprcat ("\n\nHere's a Table for the to hit percentages\n");
  lprcat
    ("\n     We will be assuming that players level = 2 * monster level");
  lprcat ("\n     and that the players dexterity and strength are 16.");
  lprcat
    ("\n    to hit: if (rnd(22) < (2[monst AC] + your level + dex + WC/8 -1)/2) then hit");
  lprcat ("\n    damage = rund(8) + WC/2 + STR - cdesc[HARDGAME] - 4");
  lprcat ("\n    to hit:  if rnd(22) < to hit  then player hits\n");
  lprcat
    ("\n    Each entry is as follows:  to hit / damage / number hits to kill\n");
  lprcat ("\n          monster     WC = 4         WC = 20        WC = 40");
  lprcat
    ("\n---------------------------------------------------------------");
  for (i = 0; i <= MAXMONST + 8; i++)
    {
      hit = 2 * monster[i].armorclass + 2 * monster[i].level + 16;
      dam = 16 - cdesc[HARDGAME];
      lprintf ("\n%20s   %2d/%2d/%2d       %2d/%2d/%2d       %2d/%2d/%2d",
	       monster[i].name,
	       (int) (hit / 2), (int) max (0, dam + 2),
	       (int) (monster[i].hitpoints / (dam + 2) + 1),
	       (int) ((hit + 2) / 2), (int) max (0, dam + 10),
	       (int) (monster[i].hitpoints / (dam + 10) + 1),
	       (int) ((hit + 5) / 2), (int) max (0, dam + 20),
	       (int) (monster[i].hitpoints / (dam + 20) + 1));
    }

  lprcat ("\n\nHere's the list of available potions:\n\n");
  for (i = 0; i < MAXPOTION; i++)
    lprintf ("%20s\n", &potionname[i][1]);
  lprcat ("\n\nHere's the list of available scrolls:\n\n");
  for (i = 0; i < MAXSCROLL; i++)
    lprintf ("%20s\n", &scrollname[i][1]);
  lprcat ("\n\nHere's the spell list:\n\n");
  lprcat ("spell          name           description\n");
  lprcat
    ("-------------------------------------------------------------------------------------------\n\n");
  for (j = 0; j < SPNUM; j++)
    {
      lprc (' ');
      lprcat (spelcode[j]);
      lprintf (" %21s  %s\n", spelname[j], speldescript[j]);
    }

  lprcat ("\n\nFor the cdesc[] array:\n");
  for (j = 0; j < 100; j += 10)
    {
      lprintf ("\nc[%2d] = ", (int) j);
      for (i = 0; i < 9; i++)
	lprintf ("%5d ", (int) cdesc[i + j]);
    }

  lprcat ("\n\nTest of random number generator ----------------");
  lprcat ("\n    for 25,000 calls divided into 16 slots\n\n");

  for (i = 0; i < 16; i++)
    rndcount[i] = 0;
  for (i = 0; i < 25000; i++)
    rndcount[rund (16)]++;
  for (i = 0; i < 16; i++)
    {
      lprintf (" %d: %5d", i,(int) rndcount[i]);
      if (i == 7)
	lprc ('\n');
    }

  lprcat ("Done\n\n");
  lwclose ();
  lcreat ((char *) 0);
  lprcat ("Done Diagnosing . . .");
  return (0);
}


/*
subroutine to draw the whole screen as the player knows it
*/
void
diagdrawscreen (void)
{
  int i, j, k;

  for (i = 0; i < MAXY; i++)

    /*  for the east west walls of this line    */
    {
      for (j = 0; j < MAXX; j++)
	if (k = mitem[j][i])
	  lprc (monstnamelist[k]);
	else
	  lprc (objnamelist[item[j][i]]);
      lprc ('\n');
    }
}
#endif

/*
to save the game in a file
*/
static time_t zzz = 0;

int
savegame (char *fname)
{
  int i, k;
  struct sphere *sp;
  time_t temptime;

  lflush ();
  savelevel ();
  ointerest ();
  if (lcreat (fname) < 0)
    {
      lcreat ((char *) 0);
      lprintf ("\nCan't open file <%s> to save game\n", fname);
      return (-1);
    }

  set_score_output ();

  lwrite (logname, LOGNAMESIZE);

  lwrite ((char *) beenhere, (sizeof (int) * (MAXLEVEL + MAXVLEVEL)));

  for (k = 0; k < MAXLEVEL + MAXVLEVEL; k++)
    {
      if (beenhere[k])
	lwrite ((char *) &cell[(int) k * MAXX * MAXY],
		sizeof (struct cel) * MAXY * MAXX);
    }

  lwrite ((char *) &cdesc[0], 100 * sizeof (long));

  lwrite ((char *) &gtime, 1 * sizeof (long));
  lprint (level);
  lprint (playerx);
  lprint (playery);

  lwrite ((char *) iven, 26 * sizeof (int));
  lwrite ((char *) ivenarg, 26 * sizeof (int));

  for (k = 0; k < MAXSCROLL; k++)
    {

      lprc (scrollname[k][0]);
    }

  for (k = 0; k < MAXPOTION; k++)
    {

      lprc (potionname[k][0]);
    }

  lwrite ((char *) spelknow, SPNUM * sizeof (int));
  lprint (wizard);
  lprint (rmst);		/*  random monster generation counter */

  for (i = 0; i < 90; i++)
    {

      lprint (dnd_item[i].qty);
    }

  lwrite ((char *) course, 25 * sizeof (int));
  lprint (cheat);

  for (i = 0; i < MAXMONST; i++)
    lprint (monster[i].genocided);	/* genocide info */

  for (sp = spheres; sp; sp = sp->p)
    lwrite ((char *) sp, sizeof (struct sphere));	/* save spheres of annihilation */

  time (&zzz);
  temptime = zzz - initialtime;
  lwrite ((char *) &temptime, sizeof (time_t));
  /* lwrite( (char*)&zzz,      sizeof(time_t) ); */

  lprint (VERSION);
  lprint (SUBVERSION);

  lwclose ();

  lastmonst[0] = 0;
  setscroll ();
  lcreat ((char *) 0);
  return (0);
}


void
restoregame (char *fname)
{
  int i, k;
  struct sphere *sp, *sp2;
  /*struct stat filetimes; */
  time_t temptime;

  cursors ();
  lprcat ("\nRestoring . . .");
  lflush ();
  if (lopen (fname) <= 0)
    {
      lcreat ((char *) 0);
      lprintf ("\nCan't open file <%s>to restore game\n", fname);
      nap (NAPTIME);
      cdesc[GOLD] = cdesc[BANKACCOUNT] = 0;
      died (-265);
      return;
    }

  lrfill (logname, LOGNAMESIZE);

  lrfill ((char *) beenhere, (sizeof (int) * (MAXLEVEL + MAXVLEVEL)));

  for (k = 0; k < MAXLEVEL + MAXVLEVEL; k++)
    {
      if (!beenhere[k])
	continue;
      lrfill ((char *) &cell[(int) k * MAXX * MAXY],
	      sizeof (struct cel) * MAXY * MAXX);
    }

  lrfill ((char *) &cdesc[0], 100 * sizeof (long));

  lrfill ((char *) &gtime, 1 * sizeof (long));
  level = cdesc[CAVELEVEL] = larint ();
  playerx = larint ();
  playery = larint ();

  lrfill ((char *) iven, 26 * sizeof (int));
  lrfill ((char *) ivenarg, 26 * sizeof (int));

  for (k = 0; k < MAXSCROLL; k++)
    {

      scrollname[k][0] = lgetc ();
    }

  for (k = 0; k < MAXPOTION; k++)
    {

      potionname[k][0] = lgetc ();
    }

  lrfill ((char *) spelknow, SPNUM * sizeof (int));
  wizard = larint ();
  rmst = larint ();		/*  random monster creation flag */

  for (i = 0; i < 90; i++)
    {

      dnd_item[i].qty = larint ();
    }

  lrfill ((char *) course, 25 * sizeof (int));
  cheat = larint ();


  for (i = 0; i < MAXMONST; i++)
    monster[i].genocided = larint ();	/* genocide info */

  for (sp = 0, i = 0; i < cdesc[SPHCAST]; i++)
    {
      sp2 = sp;
      sp = (struct sphere *) malloc (sizeof (struct sphere));
      if (sp == 0)
	{
	  fprintf (stderr, "Can't malloc() for sphere space\n");
	  break;
	}
      lrfill ((char *) sp, sizeof (struct sphere));	/* get spheres of annihilation */
      sp->p = 0;		/* null out pointer */
      if (i == 0)
	spheres = sp;		/* beginning of list */
      else
	sp2->p = sp;
    }

  time (&zzz);
  lrfill ((char *) &temptime, sizeof (time_t));
  initialtime = zzz - temptime;

  /* I don't care if ppl feel the need to cheat - edwin */
  /*  get the creation and modification time of file  */
  /* fstat(fd,&filetimes);   
     lrfill( (char*)&zzz, sizeof(time_t) );  
     zzz += 6;
     if (filetimes.st_ctime > zzz) {
     fsorry(); 
     } else if (filetimes.st_mtime > zzz) {
     fsorry(); 
     } */

  /*  version check  */
  if (VERSION != larint () || SUBVERSION != larint ())
    {
      lrclose ();
      cheat = 1;
      cursor (1, 23);
      lprcat ("Sorry, But your save file is for an older version of larn\n");
      lflush ();
      nap (NAPTIME);
      cdesc[GOLD] = cdesc[BANKACCOUNT] = 0;
      died (-266);
      return;
    }

  lrclose ();

  oldx = oldy = 0;

  if (cdesc[HP] < 0)
    {
      died (284);
      return;
    }				/* died a post mortem death */

#ifdef ANTICHEAT
  if (_unlink (fname) < 0)
    fcheat ();
  for (k = 0; k < 6; k++)
    if (cdesc[k] > 99)
      greedy ();
  if (cdesc[HPMAX] > 999 || cdesc[SPELLMAX] > 125)
    greedy ();
#endif

  /* if patch up lev 25 player */
  if (cdesc[LEVEL] == 25 && cdesc[EXPERIENCE] > skill[24])
    {
      long tmp;
      tmp = cdesc[EXPERIENCE] - skill[24];	/* amount to go up */
      cdesc[EXPERIENCE] = skill[24];
      raiseexperience (tmp);
    }
  getlevel ();
  gtime -= 1;			/* HACK for time advancing either on save or reload */
  lasttime = gtime - 1;
}

#ifdef ANTICHEAT

/*
* subroutine to not allow greedy cheaters
*/
static void
greedy (void)
{

#if WIZID
  if (wizard)
    {

      return;
    }
#endif

  lprcat
    ("\n\nI am so sorry, but your character is a little TOO good!  Since this\n");
  lprcat
    ("cannot normally happen from an honest game, I must assume that you cheated.\n");
  lprcat
    ("In that you are GREEDY as well as a CHEATER, I cannot allow this game\n");
  lprcat ("to continue.\n");

  nap (5000);
  cdesc[GOLD] = cdesc[BANKACCOUNT] = 0;
  died (-267);
}


/*
* subroutine to not allow altered save files and terminate the attempted
* restart
*/
static void
fsorry (void)
{

  lprcat ("\nSorry, but your savefile has been altered.\n");
  lprcat ("However, seeing as I am a good sport, I will let you play.\n");
  lprcat ("Be advised though, you won't be placed on the normal scoreboard.");

  cheat = 1;
  nap (4000);
}


/*
* subroutine to not allow game if save file can't be deleted
*/
static void
fcheat (void)
{

#if WIZID
  if (wizard)
    {

      return;
    }
#endif

  lprcat
    ("\nSorry, but your savefile can't be deleted.  This can only mean\n");
  lprcat
    ("that you tried to CHEAT by protecting the directory the savefile\n");
  lprcat
    ("is in.  Since this is unfair to the rest of the larn community, I\n");
  lprcat ("cannot let you play this game.\n");

  nap (5000);
  cdesc[GOLD] = cdesc[BANKACCOUNT] = 0;
  died (-268);
}

#endif
