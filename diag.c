/*
 * Larn — Copyright © 1986 Noah Morgan
 *        Copyright © 2014-2026 Gibbon
 *
 * This file is part of Larn and is distributed under
 * The Noah Licence, Version 1.0.
 *
 * You may use, modify, and redistribute this code for
 * non‑commercial purposes, provided that:
 *   - this notice is preserved,
 *   - The Noah Licence accompanies all redistributions, and
 *   - no profit is made from Larn or derivative works
 *     without explicit permission from the copyright holder.
 *
 * Larn is provided “AS IS”, without warranty of any kind.
 *
 * See the 'LICENSE.txt' file in the 'docs' folder.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"
#include "global.h"
#include "io.h"
#include "savelev.h"
#include "scores.h"
#include "store.h"
#include "nap.h"
#include "create.h"
#include "diag.h"

static time_t zzz = 0;

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

  lprintf ("\n\nPlayer attributes:\n\nHit points: %2d(%2d)", (int) c[HP],
	   (int) c[HPMAX]);
  lprintf
    ("\ngold: %d  Experience: %d  Character level: %d  Level in caverns: %d",
     (int) c[GOLD], (int) c[EXPERIENCE], (int) c[LEVEL],
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
  lprcat ("\n    damage = rund(8) + WC/2 + STR - c[HARDGAME] - 4");
  lprcat ("\n    to hit:  if rnd(22) < to hit  then player hits\n");
  lprcat
    ("\n    Each entry is as follows:  to hit / damage / number hits to kill\n");
  lprcat ("\n          monster     WC = 4         WC = 20        WC = 40");
  lprcat
    ("\n---------------------------------------------------------------");
  for (i = 0; i <= MAXMONST + 8; i++)
    {
      hit = 2 * monster[i].armorclass + 2 * monster[i].level + 16;
      dam = 16 - c[HARDGAME];
      lprintf ("\n%20s   %2d/%2d/%2d       %2d/%2d/%2d       %2d/%2d/%2d",
	       monster[i].name,
	       (int) (hit / 2), (int) max_math_larn (0, dam + 2),
	       (int) (monster[i].hitpoints / (dam + 2) + 1),
	       (int) ((hit + 2) / 2), (int) max_math_larn (0, dam + 10),
	       (int) (monster[i].hitpoints / (dam + 10) + 1),
	       (int) ((hit + 5) / 2), (int) max_math_larn (0, dam + 20),
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

  lprcat ("\n\nFor the c[] array:\n");
  for (j = 0; j < 100; j += 10)
    {
      lprintf ("\nc[%2d] = ", (int) j);
      for (i = 0; i < 9; i++)
	lprintf ("%5d ", (int) c[i + j]);
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
	if ((k = mitem[j][i]))
	  lprc (monstnamelist[k]);
	else
	  lprc (objnamelist[item[j][i]]);
      lprc ('\n');
    }
}
#endif

/* save cel-by-cel, portable and clean. */
static void
save_cel(const struct cel* cp)
{
    lprint(cp->hitp);
    lprint(cp->mitem);
    lprint(cp->item);
    lprint(cp->iarg);
    lprint(cp->know);
    lprc((char)cp->erosion);
    lprc((char)cp->lavaheat);
}

/* load cel-by-cel, portable and clean. */
static void
load_cel(struct cel* cp)
{
    cp->hitp = larint();
    cp->mitem = larint();
    cp->item = larint();
    cp->iarg = larint();
    cp->know = larint();
    cp->erosion = (unsigned char)lgetc();
    cp->lavaheat = (unsigned char)lgetc();
}

/*
to save the game in a file
*/
int
savegame(char* fname)
{
    int i, k;
    int sphcount = 0;
    struct sphere* sp;
    time_t temptime;

    lflush();
    savelevel();
    ointerest();
    if (lcreat(fname) < 0)
    {
        lcreat((char*)0);
        lprintf("\nCan't open file <%s> to save game\n", fname);
        return -1;
    }

    set_score_output();

    lwrite(logname, LOGNAMESIZE);

    for (k = 0; k < MAXLEVEL + MAXVLEVEL; ++k)
        lprint(beenhere[k]);

    /* per level cells */
    for (k = 0; k < MAXLEVEL + MAXVLEVEL; ++k)
    {
        if (!beenhere[k])
            continue;

        {
            struct cel* base = &cell[k * MAXX * MAXY];
            int idx;
            for (idx = 0; idx < MAXX * MAXY; ++idx)
                save_cel(&base[idx]);
        }
    }

    for (i = 0; i < 100; ++i)
        lprint((int)c[i]);

    lprint((int)gtime);

    /* level, playerx, playery */
    lprint(level);
    lprint(playerx);
    lprint(playery);

    /* inventory */
    for (i = 0; i < 26; ++i)
        lprint(iven[i]);
    for (i = 0; i < 26; ++i)
        lprint(ivenarg[i]);

    /* scroll/potion */
    for (k = 0; k < MAXSCROLL; ++k)
        lprc(scrollname[k][0]);
    for (k = 0; k < MAXPOTION; ++k)
        lprc(potionname[k][0]);

    /* spell knowledge */
    for (i = 0; i < SPNUM; ++i)
        lprint(spelknow[i]);

    lprint(wizard);
    lprint(rmst);

    for (i = 0; i < 90; ++i)
        lprint(dnd_item[i].qty);

    for (i = 0; i < 25; ++i)
        lprint(course[i]);

    lprint(cheat);

    /* monster genocided flags */
    for (i = 0; i < MAXMONST; ++i)
        lprint(monster[i].genocided);

    /* spheres write count, then each sphere field by field */
    for (sp = spheres; sp; sp = sp->p)
        ++sphcount;
    lprint(sphcount);

    for (sp = spheres; sp; sp = sp->p)
    {
        lprint(sp->x);
        lprint(sp->y);
        lprint(sp->lev);
        lprint(sp->dir);
        lprint(sp->lifetime);
    }

    /* time delta */
    time(&zzz);
    temptime = zzz - initialtime;
    lprint((int)temptime);

    /* version info */
    lprint(VERSION);
    lprint(SUBVERSION);

    lwclose();

    lastmonst[0] = 0;
    setscroll();
    lcreat((char*)0);
    return 0;
}

void
restoregame(char* fname)
{
    int i, k;
    int sphcount;
    struct sphere* sp, * sp2;
    time_t temptime;

    cursors();
    lprcat("\nRestoring . . .");
    lflush();
    if (lopen(fname) <= 0)
    {
        lcreat((char*)0);
        lprintf("\nCan't open file <%s> to restore game\n", fname);
        nap(NAPTIME);
        c[GOLD] = c[BANKACCOUNT] = 0;
        died(-265);
        return;
    }

    lrfill(logname, LOGNAMESIZE);

    for (k = 0; k < MAXLEVEL + MAXVLEVEL; ++k)
        beenhere[k] = larint();

    /* per level cells */
    for (k = 0; k < MAXLEVEL + MAXVLEVEL; ++k)
    {
        if (!beenhere[k])
            continue;

        {
            struct cel* base = &cell[k * MAXX * MAXY];
            int idx;
            for (idx = 0; idx < MAXX * MAXY; ++idx)
                load_cel(&base[idx]);
        }
    }

    for (i = 0; i < 100; ++i)
        c[i] = (long)larint();

    gtime = (long)larint();

    /* level, playerx, playery */
    level = larint();
    c[CAVELEVEL] = level;
    playerx = larint();
    playery = larint();

    /* inventory */
    for (i = 0; i < 26; ++i)
        iven[i] = larint();
    for (i = 0; i < 26; ++i)
        ivenarg[i] = larint();

    /* scroll/potion first letters */
    for (k = 0; k < MAXSCROLL; ++k)
        scrollname[k][0] = lgetc();
    for (k = 0; k < MAXPOTION; ++k)
        potionname[k][0] = lgetc();

    /* spell knowledge */
    for (i = 0; i < SPNUM; ++i)
        spelknow[i] = larint();

    wizard = larint();
    rmst = larint();

    for (i = 0; i < 90; ++i)
        dnd_item[i].qty = larint();

    for (i = 0; i < 25; ++i)
        course[i] = larint();

    /* cheat flag */
    cheat = larint();

    /* monster genocided flags */
    for (i = 0; i < MAXMONST; ++i)
        monster[i].genocided = larint();

    /* spheres */
    spheres = 0;
    sphcount = larint();
    sp = 0;

    for (i = 0; i < sphcount; ++i)
    {
        sp2 = sp;
        sp = (struct sphere*)malloc(sizeof(struct sphere));
        if (!sp)
        {
            fprintf(stderr, "Can't malloc() for sphere space\n");
            break;
        }

        sp->x = larint();
        sp->y = larint();
        sp->lev = larint();
        sp->dir = larint();
        sp->lifetime = larint();
        sp->p = 0;

        if (i == 0)
            spheres = sp;
        else
            sp2->p = sp;
    }

    /* time delta */
    time(&zzz);
    temptime = (time_t)larint();
    initialtime = zzz - temptime;

    /* version check */
    if (VERSION != larint() || SUBVERSION != larint())
    {
        lrclose();
        cheat = 1;
        cursor(1, 23);
        lprcat("Sorry, But your save file is for an older version of larn\n");
        lflush();
        nap(NAPTIME);
        c[GOLD] = c[BANKACCOUNT] = 0;
        died(-266);
        return;
    }

    lrclose();

    oldx = oldy = 0;

    if (c[HP] < 0)
    {
        died(284);
        return;
    }

    if (c[LEVEL] == 25 && c[EXPERIENCE] > skill[24])
    {
        long tmp;
        tmp = c[EXPERIENCE] - skill[24];
        c[EXPERIENCE] = skill[24];
        raiseexperience(tmp);
    }

    getlevel();
    gtime -= 1;
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
  c[GOLD] = c[BANKACCOUNT] = 0;
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
  c[GOLD] = c[BANKACCOUNT] = 0;
  died (-268);
}

#endif
