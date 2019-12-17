/* scores.c
*
*  readboard()     Function to read in the scoreboard into a static buffer
*  writeboard()    Function to write the scoreboard from readboard()'s buffer
*  makeboard()     Function to create a new scoreboard (wipe out old one)
*  hashewon()   Function to return 1 if player has won a game before, else 0
*  int paytaxes(x)     Function to pay taxes if any are due
*  winshou()       Subroutine to print out the winning scoreboard
*  shou(x)         Subroutine to print out the non-winners scoreboard
*  showscores()        Function to show the scoreboard on the terminal
*  showallscores() Function to show scores and the iven lists that go with them
*  sortboard()     Function to sort the scoreboard
*  newscore(score, whoo, whyded, winner)   Function to add entry to scoreboard
*  new1sub(score,i,whoo,taxes)           Subroutine to put player into a
*  new2sub(score,i,whoo,whyded)          Subroutine to put player into a
*  died(x)     Subroutine to record who played larn, and what the score was
*  diedsub(x) Subroutine to print out a line showing player when he is killed
*  diedlog()   Subroutine to read a log file and print it out in ascii format
*  getplid(name)       Function to get players id # from id file
*/

#if defined WINDOWS || WINDOWS_VS
#include <io.h>
#endif

#ifdef NIX
#include <unistd.h>
#include <fcntl.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "includes/bill.h"
#include "includes/larn.h"
#include "includes/inventory.h"
#include "includes/scores.h"
#include "includes/sysdep.h"
#include "includes/io.h"

struct scofmt			/*  This is the structure for the scoreboard        */
{
  int score;			/* the score of the player                          */
  int what;			/* the number of the monster that killed player     */
  int level;			/* the level player was on when he died             */
  int hardlev;			/* the level of difficulty player played at         */
  int order;			/* the relative ordering place of this entry        */
  char who[LOGNAMESIZE];	/* the name of the character                        */
  int sciv[26][2];		/* this is the inventory list of the character      */
};

struct wscofmt			/* This is the structure for the winning scoreboard */
{
  int score;			/* the score of the player                          */
  int timeused;			/* the time used in mobuls to win the game          */
  long taxes;			/* taxes he owes to LRS                             */
  int hardlev;			/* the level of difficulty player played at         */
  int order;			/* the relative ordering place of this entry        */
  int hasmail;			/* 1 if mail is to be read, 0 otherwise */
  char who[LOGNAMESIZE];	/* the name of the character                        */
};

struct log_fmt			/* 102 bytes struct for the log file                */
{
  long score;			/* the players score                                */
  time_t diedtime;		/* time when game was over                          */
  int cavelev;			/* level in caves                                   */
  int diff;			/* difficulty player played at                      */
#ifdef EXTRA
  int elapsedtime;		/* real time of game in seconds                     */
  int bytout;			/* bytes input and output                           */
  int bytin;
  int moves;			/* number of moves made by player                   */
  int ac;			/* armor class of player                            */
  int hp, hpmax;		/* players hitpoints                                */
  int killed, spused;		/* monsters killed and spells cast                  */
  int usage;			/* usage of the cpu in %                            */
  int lev;			/* player level                                     */
#endif
  char who[12];			/* player name                                      */
  char what[46];		/* what happened to player                          */
};


static int winshou (void);
static int shou (int);
static int sortboard (void);
static void newscore (int, char *, int, int);
static void new1sub (int, int, char *, int);
static void new2sub (int, int, char *, int);
static void diedsub (int);
static int readboard (void);
static int writeboard (void);
static struct scofmt sco[SCORESIZE];	/* the structure for the scoreboard  */
static struct wscofmt winr[SCORESIZE];	/* struct for the winning scoreboard */

static struct log_fmt logg;	/* structure for the log file        */
static char *whydead[] = {
  "quit", "suspended", "self - annihilated", "shot by an arrow",
  "hit by a dart", "fell into a pit", "fell into a bottomless pit",
  "a winner", "trapped in solid rock", "killed by a missing save file",
  "killed by an old save file", "caught by the greedy cheater checker trap",
  "killed by a protected save file",
  "killed his family and committed suicide",
  "erased by a wayward finger", "fell through a bottomless trap door",
  "fell through a trap door", "drank some poisonous water",
  "fried by an electric shock", "slipped on a volcano shaft",
  "killed by a stupid act of frustration", "attacked by a revolting demon",
  "hit by his own magic", "demolished by an unseen attacker",
  "fell into the dreadful sleep", "killed by an exploding chest",
  "teleported into solid rock",
  /*26 */ "killed by a missing maze data file", "annihilated in a sphere",
  "died a post mortem death", "wasted by a malloc() failure"
};


/*	Rewriting the scoring (read, write and make board functions) into plain 'ol C
	because the previous implementations were shockingly awful, messy,
	non-standard garbage. ~Gibbon
*/


/*
*  readboard()     Function to read in the scoreboard into a static buffer
*
*  returns -1 if unable to read in the scoreboard, returns 0 if all is OK
*/
static int
readboard (void)
{
	FILE *pFile;
	int b;

	pFile = fopen(scorefile, "rb");
	if (pFile == NULL)
	{
		printf("ERROR: scorefile is not readable \n");
      	lflush();
      	return(-1);
    }
    b = fread(sco,sizeof(struct scofmt)*SCORESIZE,1,pFile);
    if (b != 1)
    {
    	printf("ERROR: Loosers scoreboard is not readable\n");
    	fclose(pFile);
    	return(-1);
    }
    b = fread(winr,sizeof(struct wscofmt)*SCORESIZE,1,pFile);
    if (b != 1)
    {
    	printf("ERROR: Winners scoreboard is not readable\n");
    	fclose(pFile);
    	return(-1);
    }
    fclose(pFile);
    return 0;
}

/*
*  writeboard()    Function to write the scoreboard from readboard()'s buffer
*
*  returns -1 if unable to write the scoreboard, returns 0 if all is OK
*/
static int
writeboard (void)
{
	FILE *pFile;
	int b;
	set_score_output();

	pFile = fopen(scorefile, "wb");
	if (pFile == NULL)
	{
		printf("ERROR: scorefile is not writable \n");
      	lflush();
      	return(-1);
    }
    b = fwrite(sco,sizeof(struct scofmt)*SCORESIZE,1,pFile);
    if (b != 1)
    {
    	printf("ERROR: Loosers scoreboard is not writable\n");
    	fclose(pFile);
    	return(-1);
    }
    b = fwrite(winr,sizeof(struct wscofmt)*SCORESIZE,1,pFile);
    if (b != 1)
    {
    	printf("ERROR: Winners scoreboard is not writable\n");
    	fclose(pFile);
    	return(-1);
    }
    fclose(pFile);
    return 0;
}

/*
*  makeboard()         Function to create a new scoreboard (wipe out old one)
*
*  returns -1 if unable to write the scoreboard, returns 0 if all is OK
*/
int
makeboard (void)
{
  int i;
  for (i = 0; i < SCORESIZE; i++)
    {
    	winr[i].taxes = winr[i].score = sco[i].score = 0;
    	winr[i].order = sco[i].order = (short) i;
    }
    if (writeboard())
    {
    	printf("ERROR: unable to write a new scoreboard\n");
    	return(-1);
  	}
/* Why bother redefining a function?  Just use standard *NIX functions
	and be done with it. Windows won't even need to set permissions anyway
	so no need for 'cross platform' here. Sheesh.. ~Gibbon
*/
#if defined NIX
  chmod(scorefile, 0666);
#endif
  return(0);
}

/*
*  hashewon()   Function to return 1 if player has won a game before, else 0
*
*  This function also sets cdesc[HARDGAME] to appropriate value -- 0 if not a
*  winner, otherwise the next level of difficulty listed in the winners
*  scoreboard.  This function also sets outstanding_taxes to the value in
*  the winners scoreboard.
*/
int
hashewon (void)
{
  int i;

  cdesc[HARDGAME] = 0;
  if (readboard () < 0)
    return (0);			/* can't find scoreboard */
  for (i = 0; i < SCORESIZE; i++)	/* search through winners scoreboard */
    if (strcmp (winr[i].who, logname) == 0)
      if (winr[i].score > 0)
	{
	  cdesc[HARDGAME] = winr[i].hardlev + 1;
	  /* outstanding_taxes = winr[i].taxes; */
	  return (1);
	}
  return (0);
}



void
checkmail (void)
{
  int i;
  int gold, taxes;

  if (readboard () < 0)
    return;			/* can't find scoreboard */
  for (i = 0; i < SCORESIZE; i++)	/* search through winners scoreboard */
    if (strcmp (winr[i].who, logname) == 0 && winr[i].score > 0
	&& winr[i].hasmail)
      {
	winr[i].hasmail = 0;
	gold = taxes = winr[i].taxes;
	writeboard ();

	/* Intuit the amount of gold -- should have changed
	 * the score file, but ...  TAXRATE is an fraction.
	 */
	while ((gold * TAXRATE) < taxes)
	  gold += taxes;
	readmail (gold);
      }
}



/*
*  int paytaxes(x)         Function to pay taxes if any are due
*
*  Enter with the amount (in gp) to pay on the taxes.
*  Returns amount actually paid.
*/
int
paytaxes (int x)
{
  int i;
  int amt;

  if (x < 0)
    return (0L);
  if (readboard () < 0)
    return (0L);
  for (i = 0; i < SCORESIZE; i++)
    if (strcmp (winr[i].who, logname) == 0)	/* look for players winning entry */
      if (winr[i].score > 0)	/* search for a winning entry for the player */
	{
	  amt = winr[i].taxes;
	  if (x < amt)
	    amt = x;		/* don't overpay taxes (Ughhhhh) */
	  winr[i].taxes -= amt;
	  outstanding_taxes -= amt;
	  if (writeboard () < 0)
	    return (0);
	  return (amt);
	}
  return (0L);			/* couldn't find user on winning scoreboard */
}



/*
*  winshou()       Subroutine to print out the winning scoreboard
*
*  Returns the number of players on scoreboard that were shown
*/
static int
winshou (void)
{
  struct wscofmt *p;
  int i, j, count;

  for (count = j = i = 0; i < SCORESIZE; i++)	/* is there anyone on the scoreboard? */
    if (winr[i].score != 0)
      {
	j++;
	break;
      }
  if (j)
    {
      lprcat ("\n  Score    Difficulty   Time Needed   Larn Winners List\n");

      for (i = 0; i < SCORESIZE; i++)	/* this loop is needed to print out the */
	for (j = 0; j < SCORESIZE; j++)	/* winners in order */
	  {
	    p = &winr[j];	/* pointer to the scoreboard entry */
	    if (p->order == i)
	      {
		if (p->score)
		  {
		    count++;
		    lprintf ("%10d     %2d      %5d Mobuls   %s \n",
			     (int) p->score, (int) p->hardlev,
			     (int) p->timeused, p->who);
		  }
		break;
	      }
	  }
    }
  return (count);		/* return number of people on scoreboard */
}



/*
*  Subroutine to print out the non-winners scoreboard
*
*  Enter with 0 to list the scores, enter with 1 to list inventories too
*  Returns the number of players on scoreboard that were shown
*/
static int
shou (int x)
{
  int i, j, n, k;
  int count;

  for (count = j = i = 0; i < SCORESIZE; i++)	/* is the scoreboard empty? */
    if (sco[i].score != 0)
      {
	j++;
	break;
      }
  if (j)
    {
      lprcat ("\n   Score   Difficulty   Larn Visitor Log\n");
      for (i = 0; i < SCORESIZE; i++)	/* be sure to print them out in order */
	for (j = 0; j < SCORESIZE; j++)
	  if (sco[j].order == i)
	    {
	      if (sco[j].score)
		{
		  count++;
		  lprintf ("%10ld     %2d       %s ", sco[j].score,
			   sco[j].hardlev, sco[j].who);
		  if (sco[j].what < 256)
		    lprintf ("killed by a %s", monster[sco[j].what].name);
		  else
		    lprintf ("%s", whydead[sco[j].what - 256]);
		  if (x != 263)
		    lprintf (" on %s", levelname[sco[j].level]);
		  if (x)
		    {
		      for (n = 0; n < 26; n++)
			{
			  iven[n] = sco[j].sciv[n][0];
			  ivenarg[n] = sco[j].sciv[n][1];
			}
		      for (k = 1; k < 99; k++)
			{
			  for (n = 0; n < 26; n++)
			    {
			      if (k == iven[n])
				show3 (n);
			    }
			}
		      lprcat ("\n\n");
		    }
		  else
		    lprc ('\n');
		}
	      j = SCORESIZE;
	    }
    }
  return (count);		/* return the number of players just shown */
}



/*
*  showscores()        Function to show the scoreboard on the terminal
*
*  Returns nothing of value
*/
static char esb[] = "The scoreboard is empty.\n";

void
showscores (void)
{
  int i, j;

  lflush ();
  lcreat ((char *) 0);

  if (readboard () < 0)
    {

      return;
    }

  i = winshou ();
  j = shou (0);

  if (i + j == 0)
    {

      lprcat (esb);

    }
  else
    {

      lprc ('\n');
    }

  lflush ();
}

/* showscores, clib version */
/*
void show_scoreboard(void)
{
	int i, j;
	FILE * fp;

	fp = fopen(scorefile, "rb");
	if (fp == NULL) {
		puts("Can't read scoreboard\n");
		return;
	}
	fread(sco,	sizeof(sco), 	1, fp);
	fread(winr,	sizeof(winr),	1, fp);
	fclose(fp);

	i = show_winscores();
	j = show_regscores(0);

	if (i + j == 0)
		puts(esb);
	else
		putchar('\n');
}
*/


/*
*  showallscores() Function to show scores and the iven lists that go with them
*
*  Returns nothing of value
*/
void
showallscores (void)
{
  int i, j;

  lflush ();
  lcreat ((char *) 0);
  if (readboard () < 0)
    return;
  cdesc[WEAR] = cdesc[WIELD] = cdesc[SHIELD] = -1;	/* not wielding or wearing anything */
  for (i = 0; i < MAXPOTION; i++)
    potionname[i][0] = ' ';
  for (i = 0; i < MAXSCROLL; i++)
    scrollname[i][0] = ' ';
  i = winshou ();
  j = shou (1);
  if (i + j == 0)
    lprcat (esb);
  else
    lprc ('\n');
  lflush ();
}



/*
*  sortboard()     Function to sort the scoreboard
*
*  Returns 0 if no sorting done, else returns 1
*/
static int
sortboard (void)
{
  int i, pos, j = 0;
  int jdat;

  for (i = 0; i < SCORESIZE; i++)
    sco[i].order = winr[i].order = -1;
  pos = 0;
  while (pos < SCORESIZE)
    {
      jdat = 0;
      for (i = 0; i < SCORESIZE; i++)
	if ((sco[i].order < 0) && (sco[i].score >= jdat))
	  {
	    j = i;
	    jdat = sco[i].score;
	  }
      sco[j].order = pos++;
    }
  pos = 0;
  while (pos < SCORESIZE)
    {
      jdat = 0;
      for (i = 0; i < SCORESIZE; i++)
	if ((winr[i].order < 0) && (winr[i].score >= jdat))
	  {
	    j = i;
	    jdat = winr[i].score;
	  }
      winr[j].order = pos++;
    }
  return (1);
}



/*
*  newscore(score, whoo, whyded, winner)   Function to add entry to scoreboard
*      int score, winner, whyded;
*      char *whoo;
*
*  Enter with the total score in gp in score,  players name in whoo,
*      died() reason # in whyded, and TRUE/FALSE in winner if a winner
*  ex.     newscore(1000, "player 1", 32, 0);
*/
static void
newscore (int score, char *whoo, int whyded, int winner)
{
  int i;
  int taxes;

  if (readboard () < 0)
    return;			/*  do the scoreboard   */
  /* if a winner then delete all non-winning scores */
  if (cheat)
    winner = 0;			/* if he cheated, don't let him win */
  if (winner)
    {
      for (i = 0; i < SCORESIZE; i++)
	if (strcmp (sco[i].who, logname) == 0)
	  sco[i].score = 0;
      taxes = score * TAXRATE;
      score += 100000 * cdesc[HARDGAME];	/* bonus for winning */
      /* if he has a slot on the winning scoreboard update it if greater score */
      for (i = 0; i < SCORESIZE; i++)
	if (strcmp (winr[i].who, logname) == 0)
	  {
	    new1sub (score, i, whoo, taxes);
	    return;
	  }
      /* he had no entry. look for last entry and see if he has a greater score */
      for (i = 0; i < SCORESIZE; i++)
	if (winr[i].order == SCORESIZE - 1)
	  {
	    new1sub (score, i, whoo, taxes);
	    return;
	  }
    }
/* for not winning scoreboard */
  else if (!cheat)
#if defined MULTIPLE_SCORE_ENTRY
/* a copy of the below score checking without the hard-coded limitation of a single entry (if the user wants it defined).  Otherwise remove the define from the makefile. -Gibbon */
{
      for (i = 0; i < SCORESIZE; i++)
	if (sco[i].order == SCORESIZE - 1)
	  {
	    new2sub (score, i, whoo, whyded);
	    return;
	  }
	}
}
#else
    {
      /* if he has a slot on the scoreboard update it if greater score.*/
      for (i = 0; i < SCORESIZE; i++)
	if (strcmp (sco[i].who, logname) == 0)
	  {
	    new2sub (score, i, whoo, whyded);
	    return;
	  }
/* he had no entry. look for last entry and see if he has a greater score */
      for (i = 0; i < SCORESIZE; i++)
	if (sco[i].order == SCORESIZE - 1)
	  {
	    new2sub (score, i, whoo, whyded);
	    return;
	  }
	}
}
#endif

/*
*  new1sub(score,i,whoo,taxes)       Subroutine to put player into a
*      int score,i,whyded,taxes;         winning scoreboard entry if his score
*      char *whoo;                       is high enough
*
*  Enter with the total score in gp in score,  players name in whoo,
*      died() reason # in whyded, and TRUE/FALSE in winner if a winner
*      slot in scoreboard in i, and the tax bill in taxes.
*  Returns nothing of value
*/
static void
new1sub (int score, int i, char *whoo, int taxes)
{
  struct wscofmt *p;

  p = &winr[i];
  p->taxes += taxes;
  if ((score >= p->score) || (cdesc[HARDGAME] > p->hardlev))
    {
      strcpy (p->who, whoo);
      p->score = score;
      p->hardlev = cdesc[HARDGAME];
      p->timeused = gtime / 100;
      p->hasmail = 1;
    }
}




/*
*  new2sub(score,i,whoo,whyded)          Subroutine to put player into a
*      int score,i,whyded,taxes;         non-winning scoreboard entry if his
*      char *whoo;                       score is high enough
*
*  Enter with the total score in gp in score,  players name in whoo,
*      died() reason # in whyded, and slot in scoreboard in i.
*  Returns nothing of value
*/
static void
new2sub (int score, int i, char *whoo, int whyded)
{
  int j;
  struct scofmt *p;

  p = &sco[i];
  if ((score >= p->score) || (cdesc[HARDGAME] > p->hardlev))
    {
      strcpy (p->who, whoo);
      p->score = score;
      p->what = whyded;
      p->hardlev = cdesc[HARDGAME];
      p->level = level;
      for (j = 0; j < 26; j++)
	{
	  p->sciv[j][0] = iven[j];
	  p->sciv[j][1] = ivenarg[j];
	}
    }
}




/*
*  died(x)     Subroutine to record who played larn, and what the score was
*      int x;
*
*  if x < 0 then don't show scores
*  died() never returns! (unless cdesc[LIFEPROT] and a reincarnatable death!)
*
*      < 256   killed by the monster number
*      256     quit
*      257     suspended
*      258     self - annihilated
*      259     shot by an arrow
*      260     hit by a dart
*      261     fell into a pit
*      262     fell into a bottomless pit
*      263     a winner
*      264     trapped in solid rock
*      265     killed by a missing save file
*      266     killed by an old save file
*      267     caught by the greedy cheater checker trap
*      268     killed by a protected save file
*      269     killed his family and killed himself
*      270     erased by a wayward finger
*      271     fell through a bottomless trap door
*      272     fell through a trap door
*      273     drank some poisonous water
*      274     fried by an electric shock
*      275     slipped on a volcano shaft
*      276     killed by a stupid act of frustration
*      277     attacked by a revolting demon
*      278     hit by his own magic
*      279     demolished by an unseen attacker
*      280     fell into the dreadful sleep
*      281     killed by an exploding chest
*      282     killed by a missing maze data file
*      283     killed by a sphere of annihilation
*      284     died a post mortem death
*      285     malloc() failure
*      300     quick quit -- don't put on scoreboard
*/

static int scorerror;

void
died (int x)
{
  int f, win;
  /*char ch, *mod;
     time_t zzz; */

  if (cdesc[LIFEPROT] > 0)	/* if life protection */
    {
      switch ((x > 0) ? x : -x)
	{
	case 256:
	case 257:
	case 262:
	case 263:
	case 265:
	case 266:
	case 267:
	case 268:
	case 269:
	case 271:
	case 282:
	case 284:
	case 285:
	case 300:
	  goto invalid;		/* can't be saved */
	};
      --cdesc[LIFEPROT];
      cdesc[HP] = cdesc[HPMAX];
      --cdesc[CONSTITUTION];
      cursors ();
      lprcat ("\nYou feel wiiieeeeerrrrrd all over! ");
      lflush ();
      nap (3000);

      return;			/* only case where died() returns */
    }

  cursors ();
  lprcat ("\nPress any key to continue. ");
  ttgetch ();

invalid:
  /*clearvt100(); */
  lflush ();
  f = 0;

  /* if we are not to display the scores */
  if (x < 0)
    {
      f++;
      x = -x;
    }

  /* for quick exit or saved game */
  if ((x == 300) || (x == 257))
    {
      clearvt100 ();
      exit (EXIT_SUCCESS);
    }

  if (x == 263)
    win = 1;
  else
    win = 0;

  cdesc[GOLD] += cdesc[BANKACCOUNT];
  cdesc[BANKACCOUNT] = 0;

  /*  now enter the player at the end of the scoreboard */
  newscore (cdesc[GOLD], logname, x, win);
  diedsub (x);			/* print out the score line */
  lflush ();

  set_score_output ();
  if ((wizard == 0) && (cdesc[GOLD] > 0))	/*  wizards can't score     */
    {
      /*  now for the scoreboard maintenance -- not for a suspended game  */
      if (x != 257)
	{
	  if (sortboard ())
	    scorerror = writeboard ();
	}
    }
  if ((x == 256) || (x == 257) || (f != 0))
    {
      clearvt100 ();
      exit (EXIT_SUCCESS);
    }
  if (scorerror == 0)
    {
      lflush ();
      screen_clear();
      resetscroll ();
      showscores ();		/* if we updated the scoreboard */
      cursors ();
      lprcat ("\nPress any key to exit. ");
      scbr ();
      ttgetch ();
    }
  clearvt100 ();
  exit (EXIT_SUCCESS);
}



/*
*  diedsub(x) Subroutine to print out the line showing the player when he is killed
*      int x;
*/
static void
diedsub (int x)
{
  char ch, *mod;

  lprintf ("Score: %ld, Diff: %ld,  %s ", cdesc[GOLD], cdesc[HARDGAME],
	   logname);

  if (x < 256)
    {

      ch = *monster[x].name;

      if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u')
	{

	  mod = "an";

	}
      else
	{

	  mod = "a";
	}

      lprintf ("killed by %s %s", mod, monster[x].name);

    }
  else
    {

      lprintf ("%s", whydead[x - 256]);
    }

  if (x != 263)
    {

      lprintf (" on %s\n", levelname[level]);

    }
  else
    {

      lprc ('\n');
    }
}



/*
*  diedlog()   Subroutine to read a log file and print it out in ascii format
*/
void
diedlog (void)
{
  int n;
  char *p;
  struct stat stbuf;

  lcreat ((char *) 0);
  if (lopen (logfile) < 0)
    {
      lprintf ("Can't locate log file <%s>\n", logfile);
      return;
    }
  if (fstat (fd, &stbuf) < 0)
    {
      lprintf ("Can't  stat log file <%s>\n", logfile);
      return;
    }
  for (n = stbuf.st_size / sizeof (struct log_fmt); n > 0; --n)
    {
      lrfill ((char *) &logg, sizeof (struct log_fmt));
      p = ctime ((time_t *) & logg.diedtime);
      p[16] = '\n';
      p[17] = 0;
      lprintf ("Score: %d, Diff: %d,  %s %s on %d at %s", (int) (logg.score),
	       (int) (logg.diff), logg.who, logg.what, (int) (logg.cavelev),
	       p + 4);
#ifdef EXTRA
      if (logg.moves <= 0)
	logg.moves = 1;
      lprintf
	("  Experience Level: %d,  AC: %d,  HP: %d/%d,  Elapsed Time: %d minutes\n",
	 (int) (logg.lev), (int) (logg.ac), (int) (logg.hp),
	 (int) (logg.hpmax), (int) (logg.elapsedtime));

      lprintf
	("  BYTES in: %d, out: %d, moves: %d, deaths: %d, spells cast: %d\n",
	 (int) (logg.bytin), (int) (logg.bytout), (int) (logg.moves),
	 (int) (logg.killed), (int) (logg.spused));
      lprintf ("  out bytes per move: %d", (int) (logg.bytout / logg.moves));
      lprintf ("\n");
#endif
    }
  lflush ();
  lrclose ();
  return;
}




/*
*  getplid(name)       Function to get players id # from id file
*
*  Enter with the name of the players character in name.
*  Returns the id # of the players character, or -1 if failure.
*  This routine will try to find the name in the id file, if its not there,
*  it will try to make a new entry in the file.  Only returns -1 if can't
*  find him in the file, and can't make a new entry in the file.
*  Format of playerids file:
*          Id # in ascii     \n     character name     \n
*/
static int havepid = -1;	/* playerid # if previously done */

int
getplid (char *nam)
{
  int fd7, high = 999, no;
  char *p, *p2;
  char name[80];

  if (havepid != -1)
    return (havepid);		/* already did it */
  lflush ();			/* flush any pending I/O */
  sprintf (name, "%s\n", nam);	/* append a \n to name */
  if (lopen (playerids) < 0)	/* no file, make it */
    {
#if defined WINDOWS
      if ((fd7 = _open(playerids, _S_IWUSR)) < 0)
      {
        return (-1);		/* can't make it */
      }
      _close (fd7);
#endif
#if defined WINDOWS_VS
      if ((fd7 = _open(playerids, _S_IREAD)) < 0)
      {
        return (-1);		/* can't make it */
      }
      _close(fd7);
#endif
#if defined NIX
      if ((fd7 = open(playerids, S_IWUSR)) < 0)
      {
        return (-1);		/* can't make it */
      }
      close (fd7);
#endif
      goto addone;		/* now append new playerid record to file */
    }
  for (;;)			/* now search for the name in the player id file */
    {
      p = lgetl ();
      if (p == NULL)
	break;			/* EOF? */
      no = atoi (p);		/* the id # */
      p2 = lgetl ();
      if (p2 == NULL)
	break;			/* EOF? */
      if (no > high)
	high = no;		/* accumulate highest id # */
      if (strcmp (p2, name) == 0)	/* we found him */
	{
	  return (no);		/* his id number */
	}
    }
  lrclose ();
  /* if we get here, we didn't find him in the file -- put him there */
addone:
  if (lappend (playerids) < 0)
    return (-1);		/* can't open file for append */
  lprintf ("%d\n%s", (int) ++high, name);	/* new id # and name */
  lwclose ();
  lcreat ((char *) 0);		/* re-open terminal channel */
  return (high);
}
