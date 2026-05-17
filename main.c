/* main.c */
#include <stdio.h>
#include "includes/larn.h"
#include "includes/tok.h"
#include "includes/create.h"
#include "includes/diag.h"
#include "includes/display.h"
#include "includes/fortune.h"
#include "includes/global.h"
#include "includes/help.h"
#include "includes/inventory.h"
#include "includes/io.h"
#include "includes/main.h"
#include "includes/moreobj.h"
#include "includes/movem.h"
#include "includes/object.h"
#include "includes/regen.h"
#include "includes/scores.h"
#include "includes/spells.h"
#include "includes/spheres.h"
#include "includes/nap.h"

		/* needed for hack fix to handle endwin()
				   not being called after process commandline */	

static void parse (void);

static void randmonst (void);

static void run (int);

static void wield (void);

static void ydhi (int);

static void wear (void);

static void dropobj (void);

static int floor_consume (int, char *);

static void consume (int, char *, int (*)(void));

static int whatitem (char *);

int dropflag = 0;		/* if 1 then don't lookforobject() next round */
int rmst = 80;			/*  random monster creation counter     */
int nomove = 0;			/* if (nomove) then don't count next iteration as a move */
int nowelcome = 0;		/* if (nowelcome) then skip welcome message at start of game */	
static char viewflag = 0;	/* if viewflag then we have done a 99 stay here
				   and don't showcell in the main loop */
int restorflag = 0;		/* 1 means restore has been done    */

static char cmdhelp[] = "\
Cmd line format: larn [-sih] [-##]\n\
-s   show the scoreboard\n\
-i   show scoreboard with inventories of dead characters\n\
-##  specify level of difficulty (example: -5)\n\
-h   print this help text\n\
";

signed int save_mode = 0;	/* 1 if doing a save game */

/*
************
MAIN PROGRAM
************
*/
int
main (int argc, char *argv[])
{
  int i = 0;
  int hard = -1;

  FILE *pFile;

  /*
   * Portable Larn file paths using LARNHOME = "." (same as 12.0)
   * Works on all systems, all architectures.  Using 12.0 names.
   */

  strcpy(savefilename, LARNHOME);
  strcat(savefilename, "/Larn.sav");

  strcpy(scorefile, LARNHOME);
  strcat(scorefile, "/lscore");

  strcpy(logfile, LARNHOME);
  strcat(logfile, "/llog");

  strcpy(fortfile, LARNHOME);
  strcat(fortfile, "/lfortune");

  strcpy(playerids, LARNHOME);
  strcat(playerids, "/playerids");

  strcpy(mazefile, LARNHOME);
  strcat(mazefile, "/larnmaze");

  strcpy(diagfile, LARNHOME);
  strcat(diagfile, "/Diagfile");

  strcpy(holifile, LARNHOME);
  strcat(holifile, "/holidays");

  strcpy(optsfile, LARNHOME);
  strcat(optsfile, "/larnopts");

  strcpy(helpfile, LARNHOME);
  strcat(helpfile, "/larn.help");

  strcpy(ckpfile, LARNHOME);
  strcat(ckpfile, "/Larn.ckp");

  strcpy(colourfile, LARNHOME);
  strcat(colourfile, "/larn.clr");

  if (argv[i][0] == '+')
  {
	  clear();
	  restorflag = 1;
	  if (argv[i][1] == '+')
	  {
		  hitflag = 1;
		  restoregame(ckpfile); /* restore checkpointed game */
	  }
	  i = argc;
  }

  readopts(); /* read the options file if there is one */

  readcolors(); /* reads the larn.clr file */
	
  /*init curses ~Gibbon */
  init_term ();			/* setup the terminal */
  scbr ();

  /* Set up the input and output buffers.
   */
  lpbuf = (char *) malloc ((5 * BUFBIG) >> 2);	/* output buffer */
  inbuffer = (char *) malloc ((5 * MAXIBUF) >> 2);	/* output buffer */
  if ((lpbuf == 0) || (inbuffer == 0))
    died (-285);		/* malloc() failure */

  /*
   *  now make scoreboard if it is not there (don't clear) 
   */

  pFile = fopen (scorefile, "r");
  if (pFile == 0)		/* not there */
    makeboard ();
  else
    fclose (pFile);

  /*
   *  now process the command line arguments 
   */
  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-')
		  switch (argv[i][1])
	  {
	  case 's':		/* show scoreboard   */
	    showscores ();
	    lprcat ("Press any key to exit...");
	    ttgetch ();
		clearvt100();
	    exit (EXIT_SUCCESS);

	  case 'i':		/* show all scoreboard */
	    showallscores ();
	    lprcat ("Press any key to exit...");
	    ttgetch ();
		clearvt100();
	    exit (EXIT_SUCCESS);

	  case 'n': /* no welcome msg */
		  nowelcome = 1;
		  break;

	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':		/* for hardness */
		  hard = atol(&argv[i][1]);
	    break;

	  case 'h':		/* print out command line arguments */
	  case '?':
	    lprcat(cmdhelp);
		lprcat("Press any key to exit...");
		ttgetch();
		clearvt100();
		exit (EXIT_SUCCESS);
	  default:
		clearvt100();
		printf("Unknown option <%s>\n", argv[i]);
	    puts (cmdhelp);
	    exit (EXIT_SUCCESS);
	  };
    }
  
  if (dayplay == 0) /* check for not-during-daytime-hours */
  {
	  if (playable())
	  {
		  lprcat("Sorry, Larn can not be played during working hours.\n");
		  lflush();
		  nap(3000);
		  exit(EXIT_SUCCESS);
	  }
  }


  /*
   *  He really wants to play, so malloc the memory for the dungeon.
   */
  cell = malloc ((sizeof *cell) * (MAXLEVEL + MAXVLEVEL) * MAXX * MAXY);
  if (cell == NULL)
    {

      /* malloc failure */
      died (-285);
    }

  newgame ();			/*  set the initial clock  */

  pFile = fopen (savefilename, "r");
  if (pFile != 0)		/* restore game if need to */
    {
      fclose (pFile);
      screen_clear();
      restorflag = 1;
      hitflag = 1;
      restoregame (savefilename);	/* restore last game    */
      remove (savefilename);
    }

  setupvt100();

  if (c[HP] == 0)		/* create new game */
    {
      predostuff = 1;		/* tell signals that we are in the welcome screen */
	  if (nowelcome == 0)
		  welcome(); /* welcome the player to the game */

      makeplayer ();		/*  make the character that will play           */
      sethard (hard);		/* set up the desired difficulty                */

      newcavelevel (0);		/*  make the dungeon                            */

      /* Display their mail if they've just won the previous game
       */
      checkmail ();

    }

  lprc (T_INIT);		/* Reinit the screen because of welcome and check mail
				 * having embedded escape sequences.*/

  drawscreen ();		/*  show the initial dungeon */

  /* tell the trap functions that they must do a showplayer() from here on */
  predostuff = 2;

  yrepcount = hit2flag = 0;

  /* 
   * init previous player position to be current position, so we don't
   * reveal any stuff on the screen prematurely.
   */
  oldx = playerx;
  oldy = playery;
  /* gtime = -1; */

  /* MAINLOOP
     find objects, move stuff, get commands, regenerate
   */
  for (;;)
    {
	  /* water animation is now in a real time state machine - animates every 2 seconds */
	  long now = time(NULL);
	  if (now - last_water_anim >= 2)
	  {
		  last_water_anim = now;
		  water_anim_toggle = !water_anim_toggle;
		  expand_puddle();

		  /* redraw only puddles for performance reasons (low cpu usage) */
		  for (int yy = 0; yy < MAXY; yy++)
			  for (int xx = 0; xx < MAXX; xx++)
				  if (item[xx][yy] == OWATER || item[xx][yy] == OSHOREWATER)
					  show1cell(xx, yy);

		  refresh();
	  }
	  /* lava animation is now in a real time state machine - animates every 3 seconds */
	  if (now - last_lava_anim >= 3)
	  {
		  last_lava_anim = now;
		  lava_anim_toggle = !lava_anim_toggle;

		  /* redraw only lava for performance reasons (low cpu usage) */
		  for (int yy = 0; yy < MAXY; yy++)
			  for (int xx = 0; xx < MAXX; xx++)
				  if (item[xx][yy] == OLAVA)
					  show1cell(xx, yy);

		  refresh();
	  }

	  if (dropflag == 0 && nomove == 0)
	{
	  /* see if there is an object here.

	     If in prompt mode, identify and prompt; else
	     identify, never prompt.
	   */
	  lookforobject (TRUE, FALSE, FALSE);
	}
      else
	{
	  dropflag = 0;		/* don't show it just dropped an item */
	}

      /* handle global activity
         update game time, move spheres, move walls, move monsters
         all the stuff affected by TIMESTOP and HASTESELF
       */
	  if (c[TIMESTOP] <= 0 && nomove == 0)
	if (c[HASTESELF] == 0 || (c[HASTESELF] & 1) == 0)
	  {
	    gtime++;
	    movsphere ();

	    if (hitflag == 0)
	      {
		if (c[HASTEMONST])
		  movemonst ();
		movemonst ();
	      }
	  }

      /* show stuff around the player
       */
      if (viewflag == 0)
	showcell (playerx, playery);
      else
	viewflag = 0;

	  /* Reset water entry flags when stepping out of water */
	  {
		  int tile = item[playerx][playery];

		  if (tile != OWATER)
			  in_water = 0;

		  if (tile != OSHOREWATER)
			  in_shorewater = 0;

		  if (tile != OLAVA)
			  in_lava = 0;
	  }

	  if (hit3flag)
		  flushinp();
      hitflag = hit3flag = 0;
      bottomline ();		/* update bottom line */

      /* get commands and make moves
       */
	  nomove = 1;

	  if (hit3flag)
		  flushinp();

	  nomove = 0;
	  parse();

	  if (c[TIMESTOP] == 0 && nomove == 0)
		  regen ();			/*  regenerate hp and spells            */

	  if (c[TIMESTOP] == 0 && nomove == 0)
	if (--rmst <= 0)
	  {
	    rmst = 120 - (level << 2);
	    fillmonst (makemonst (level));
	  }
    }
}

/*
* subroutine to randomly create monsters if needed
*/
static void
randmonst (void)
{

  /*  don't make monsters if time is stopped  */
  if (c[TIMESTOP])
    {

      return;
    }

  if (--rmst <= 0)
    {

      rmst = 120 - (level << 2);

      fillmonst (makemonst (level));
    }
}



/*
* parse()
*
* get and execute a command
*/
static void
parse (void)
{
  int i, j, k, flag;

  for (;;)
    {
      k = yylex ();
	  if (k == 0) {

		  /* no input this tick, continue with main loop */
		  nomove = 1;
		  nap(50);   /* sleep 50 millis for CPU runaway prevention */
		  return;
	  }
      switch (k)		/*  get the token from the input and switch on it   */
	{
	case 'h':
	  moveplayer (4);
	  return;		/*  west        */
	case 'H':
	  run (4);
	  return;		/*  west        */
	case 'l':
	  moveplayer (2);
	  return;		/*  east        */
	case 'L':
	  run (2);
	  return;		/*  east        */
	case 'j':
	  moveplayer (1);
	  return;		/*  south       */
	case 'J':
	  run (1);
	  return;		/*  south       */
	case 'k':
	  moveplayer (3);
	  return;		/*  north       */
	case 'K':
	  run (3);
	  return;		/*  north       */
	case 'u':
	  moveplayer (5);
	  return;		/*  northeast   */
	case 'U':
	  run (5);
	  return;		/*  northeast   */
	case 'y':
	  moveplayer (6);
	  return;		/*  northwest   */
	case 'Y':
	  run (6);
	  return;		/*  northwest   */
	case 'n':
	  moveplayer (7);
	  return;		/*  southeast   */
	case 'N':
	  run (7);
	  return;		/*  southeast   */
	case 'b':
	  moveplayer (8);
	  return;		/*  southwest   */
	case 'B':
	  run (8);
	  return;		/*  southwest   */

	case '.':		/*  stay here       */
	  if (yrepcount)
	    viewflag = 1;
	  return;

	case 'c':
	  yrepcount = 0;
	  cast ();
	  return;		/*  cast a spell    */

	case 'd':
	  yrepcount = 0;
	  if (c[TIMESTOP] == 0 && nomove == 0)
	    dropobj ();
	  return;		/*  to drop an object   */

	case 'e':
	  yrepcount = 0;
	  if (c[TIMESTOP] == 0 && nomove == 0)
	    if (!floor_consume (OCOOKIE, "eat"))
	      consume (OCOOKIE, "eat", showeat);
	  return;		/*  to eat a fortune cookie */

	case 'g':
	  yrepcount = 0;
	  cursors ();
	  lprintf ("\nThe stuff you are carrying presently weighs %d units",
		   (int) packweight ());
	  break;

	case 'i':		/* inventory */
	  yrepcount = 0;
	  nomove = 1;
	  showstr ();
	  return;

	case 'p':		/* pray at an altar */
	  yrepcount = 0;
	  pray_at_altar ();
	  return;

	case 'q':		/* quaff a potion */
	  yrepcount = 0;
	  if (c[TIMESTOP] == 0 && nomove == 0)
	    if (!floor_consume (OPOTION, "quaff"))
	      consume (OPOTION, "quaff", showquaff);
	  return;

	case 'r':
	  yrepcount = 0;
	  if (c[BLINDCOUNT])
	    {
	      cursors ();
	      lprcat ("\nYou can't read anything when you're blind!");
	    }
	  else if (c[TIMESTOP] == 0 && nomove == 0)
	    if (!floor_consume (OSCROLL, "read"))
	      if (!floor_consume (OBOOK, "read"))
		consume (OSCROLL, "read", showread);
	  return;		/*  to read a scroll    */

	case 's':
	  yrepcount = 0;
	  sit_on_throne ();
	  return;

	case 't':		/* Tidy up at fountain */
	  yrepcount = 0;
	  wash_fountain ();
	  return;

	case 'v':
	  yrepcount = 0;
	  nomove = 1;
	  cursors ();
	  lprintf ("\nLarn, Version %d.%d.%d, Diff=%d", (int) VERSION,
		   (int) SUBVERSION, (int) PATCHLEVEL, (int) c[HARDGAME]);

	  if (wizard)
	    lprcat (" Wizard");
	  if (cheat)
	    lprcat (" Cheater");
	  return;

	case 'w':		/*  wield a weapon */
	  yrepcount = 0;
	  wield ();
	  return;

	case 'A':
	  yrepcount = 0;
	  desecrate_altar ();
	  return;

	case 'C':		/* Close something */
	  yrepcount = 0;
	  close_something ();
	  return;

	case 'D':		/* Drink at fountain */
	  yrepcount = 0;
	  drink_fountain ();
	  return;
	  
	case '?':
		yrepcount=0;	
		help();
		nomove = 1;
		return;	/*give the help screen*/

	case 'E':		/* Enter a building */
	  yrepcount = 0;
	  enter ();
	  break;

	case 'I':		/*  list spells and scrolls */
	  yrepcount = 0;
	  seemagic (0);
	  nomove = 1;
	  return;

	case 'O':		/* Open something */
	  yrepcount = 0;
	  open_something ();
	  return;

	case 'P':
	  cursors ();
	  yrepcount = 0;
	  nomove = 1;
	  if (outstanding_taxes > 0)
	    lprintf ("\nYou presently owe %d gp in taxes.",
		     (int) outstanding_taxes);
	  else
	    lprcat ("\nYou do not owe any taxes.");
	  return;

	case 'Q':		/*  quit        */
	  yrepcount = 0;
	  quit();
	  nomove = 1;
	  clearvt100();
	  return;

	case 'R':		/* remove gems from a throne */
	  yrepcount = 0;
	  remove_gems ();
	  return;

	case 'S':
	  /* And do the save.
	   */
	  cursors();
	  lprintf("\nSaving to `%s' . . . Larn will now quit. ", savefilename);
	  lflush();
	  save_mode = 1;
	  savegame(savefilename);
	  nap(1000);
	  lflush();
	  clearvt100();
	  exit(EXIT_SUCCESS);
	  break;


	case 'T':
	  yrepcount = 0;
	  cursors ();
	  if (c[SHIELD] != -1)
	    {
	      c[SHIELD] = -1;
	      lprcat ("\nYour shield is off");
	      bottomline ();
	    }
	  else if (c[WEAR] != -1)
	    {
	      c[WEAR] = -1;
	      lprcat ("\nYour armor is off");
	      bottomline ();
	    }
	  else
	    lprcat ("\nYou aren't wearing anything");
	  return;

	case 'W':
	  yrepcount = 0;
	  wear ();
	  return;		/*  wear armor  */

	case 'Z':
	  yrepcount = 0;
	  if (c[LEVEL] > 9)
	    {
	      oteleport (1);
	      return;
	    }
	  cursors ();
	  lprcat
	    ("\nAs yet, you don't have enough experience to use teleportation");
	  return;		/*  teleport yourself   */

	case ' ':
	  yrepcount = 0;
	  nomove = 1;
	  return;

	case 'L' - 64:
	  yrepcount = 0;
	  drawscreen ();
	  nomove = 1;
	  return;		/*  look        */

#if WIZID
#ifdef EXTRA
	case 'A' - 64:
	  yrepcount = 0;
	  nomove = 1;
	  if (wizard)
	    {
	      diag ();
	      return;
	    }			/*   create diagnostic file */
	  return;
#endif
#endif

	case '<':		/* Go up stairs or vol shaft */
	  yrepcount = 0;
	  up_stairs ();
	  return;

	case '>':		/* Go down stairs or vol shaft */
	  yrepcount = 0;
	  down_stairs ();
	  return;
            
    case ',':        /* pick up an object_identification */
      yrepcount = 0; /* pickup, don't identify or prompt for action */
      lookforobject (FALSE, TRUE, FALSE);
      return;

	case ':':		/* look at object */
	  yrepcount = 0;
	  /* identify, don't pick up or prompt for action */
	  lookforobject (TRUE, FALSE, FALSE);
	  nomove = 1;		/* assumes look takes no time */
	  return;

	case '/':		/* identify object/monster */
	  specify_object ();
	  nomove = 1;
	  yrepcount = 0;
	  return;

	case '^':		/* identify traps */
	  flag = yrepcount = 0;
	  cursors ();
	  lprc ('\n');
	  for (j = playery - 1; j < playery + 2; j++)
	    {
	      if (j < 0)
		j = 0;
	      if (j >= MAXY)
		break;
	      for (i = playerx - 1; i < playerx + 2; i++)
		{
		  if (i < 0)
		    i = 0;
		  if (i >= MAXX)
		    break;
		  switch (item[i][j])
		    {
		    case OTRAPDOOR:
		    case ODARTRAP:
		    case OTRAPARROW:
		    case OTELEPORTER:
		    case OPIT:
		      lprcat ("\nIts ");
		      lprcat (objectname[item[i][j]]);
		      flag++;
		    };
		}
	    }
	  if (flag == 0)
	    lprcat ("\nNo traps are visible");
	  return;

#if WIZID
	case '_':		/*  this is the fudge player password for wizard mode */
	  yrepcount = 0;
	  cursors ();
	  nomove = 1;
	  wizard = 1;		/* disable to easily test win condition */
	  scbr ();		/* system("stty -echo cbreak"); */
	  for (i = 0; i < 6; i++)
	    {
	      c[i] = 70;
	    }
	  iven[0] = iven[1] = 0;
	  take (OPROTRING, 50);
	  take (OLANCE, 25);
	  c[WIELD] = 1;
	  c[LANCEDEATH] = 1;
	  c[WEAR] = c[SHIELD] = -1;
	  raiseexperience (6000000L);
	  c[AWARENESS] += 25000;
	  {
	    for (i = 0; i < MAXY; i++)
	      for (j = 0; j < MAXX; j++)
		know[j][i] = KNOWALL;
	    for (i = 0; i < SPNUM; i++)
	      spelknow[i] = 1;
	    for (i = 0; i < MAXSCROLL; i++)
	      scrollname[i][0] = ' ';
	    for (i = 0; i < MAXPOTION; i++)
	      potionname[i][0] = ' ';
	  }
	  for (i = 0; i < MAXSCROLL; i++)
	    /* no null items */
	    if (strlen (scrollname[i]) > 2)
	      {
		item[i][0] = OSCROLL;
		iarg[i][0] = i;
	      }
	  for (i = MAXX - 1; i > MAXX - 1 - MAXPOTION; i--)
	    /* no null items */
	    if (strlen (potionname[i - MAXX + MAXPOTION]) > 2)
	      {
		item[i][0] = OPOTION;
		iarg[i][0] = i - MAXX + MAXPOTION;
	      }
	  for (i = 1; i < MAXY; i++)
	    {
	      item[0][i] = i;
	      iarg[0][i] = 0;
	    }
	  for (i = MAXY; i < MAXY + MAXX; i++)
	    {
	      item[i - MAXY][MAXY - 1] = i;
	      iarg[i - MAXY][MAXY - 1] = 0;
	    }
	  for (i = MAXX + MAXY; i < MAXOBJECT; i++)
	    {
	      item[MAXX - 1][i - MAXX - MAXY] = i;
	      iarg[MAXX - 1][i - MAXX - MAXY] = 0;
	    }
	  c[GOLD] += 250000;
	  drawscreen ();
	  return;
#endif

	};
    }
}



void
parse2 (void)
{
	if (nomove != 0)
	{
		return;
	}

  /* move the monsters */
  if (c[HASTEMONST])
    {

      movemonst ();
    }

  movemonst ();

  randmonst ();

  if (c[TIMESTOP] == 0 && nomove == 0)
regen ();
}

static void
run(int dir)
{
	int i = 1;

	while (i)
	{
		i = moveplayer(dir);

		if (hitflag)
			i = 0;

		if (i != 0)
			showcell(playerx, playery);
	}
}

/*
* function to wield a weapon
*/
static void
wield(void)
{
	int i, obj;

	for (;;) {

		i = whatitem("wield (- for nothing)");
		if (i == '\33')
			return;

		if (i == '.')
			continue;

		if (i == '*') {
			i = showwield();
			cursors();
		}

		if (i == '-')
		{
			c[WIELD] = -1;
			cursors();
			lprcat("\nYou stop wielding anything.");
			bottomline();
			return;
		}

		if (!i)
			continue;

		obj = iven[i - 'a'];

		if (obj == 0) {
			ydhi(i);
			return;
		}

		/* forbid wielding scrolls/potions/armor/etc */
		switch (obj) {
		case OSWORDofSLASHING:
		case OHAMMER:
		case OSWORD:
		case O2SWORD:
		case OHSWORD:
		case OSPEAR:
		case ODAGGER:
		case OBATTLEAXE:
		case OLONGSWORD:
		case OLANCE:
		case OVORPAL:
		case OSLAYER:
			break; /* valid weapon */
		default:
			cursors();
			lprcat("\nYou cannot wield that!");
			return;
		}

		/* shield conflicts */
		if (c[SHIELD] != -1 && obj == O2SWORD) {
			lprcat("\nBut one arm is busy with your shield!");
			return;
		}
		if (c[SHIELD] != -1 && obj == OHSWORD) {
			lprcat("\nA longsword of slashing cannot be used while a shield is equipped!");
			return;
		}

		/* success */
		c[WIELD] = i - 'a';
		c[LANCEDEATH] = (obj == OLANCE);

		cursors();
		lprintf("\nYou wield %s.", objectname[obj]);
		bottomline();
		return;
	}
}

/*
* common routine to say you don't have an item
*/
static void
ydhi (int x)
{

  cursors ();

  lprintf ("\nYou don't have item %c!", x);
}

/*
function to wear armor
*/
static void
wear(void)
{
	int i, obj;

	for (;;) {

		i = whatitem("wear");
		if (i == '\33')
			return;

		if (i == '.' || i == '-')
			continue;

		if (i == '*') {
			i = showwear();
			cursors();
		}

		if (!i || i == '.')
			continue;

		obj = iven[i - 'a'];

		if (obj == 0) {
			ydhi(i);
			return;
		}
		switch (obj) {
		case OLEATHER:
		case OCHAIN:
		case OPLATE:
		case ORING:
		case OSPLINT:
		case OPLATEARMOR:
		case OSTUDLEATHER:
		case OSSPLATE:
			/* wearing armor */
			if (c[WEAR] != -1) {
				lprcat("\nYou're already wearing some armor");
				return;
			}
			c[WEAR] = i - 'a';
			cursors();
			lprintf("\nYou equip %s.", objectname[obj]);
			bottomline();
			return;

		case OSHIELD:
			/* wearing a shield */
			if (c[SHIELD] != -1) {
				lprcat("\nYou are already wearing a shield");
				return;
			}

			/* forbid shield + two‑handed weapon */
			if (iven[c[WIELD]] == O2SWORD) {
				lprcat("\nYour hands are busy with the two‑handed sword!");
				return;
			}
			if (iven[c[WIELD]] == OHSWORD) {
				lprcat("\nYou are holding a longsword of slashing!");
				return;
			}

			c[SHIELD] = i - 'a';
			cursors();
			lprintf("\nYou equip %s.", objectname[obj]);
			bottomline();
			return;

		default:
			/* everything else is invalid */
			lprcat("\nYou can't wear that!");
			return;
		}
	}
}

/*
function to drop an object
*/
static void
dropobj (void)
{
  int i;
  int *p;
  int amt;

  p = &item[playerx][playery];
  for (;;)
    {
      if ((i = whatitem ("drop")) == '\33')
	return;
      if (i == '*')
	{
	  i = showstr ();
	  cursors ();
	}
      if (i != '-')
	{
	  if (i == '.')		/* drop some gold */
	    {
		  if (*p && *p != OWATER && *p != OSHOREWATER)
		{
		  lprintf ("\nThere's something here already: %s",
			   objectname[item[playerx][playery]]);
		  dropflag = 1;
		  return;
		}
	      lprcat ("\n\n");
	      cl_dn (1, 23);
	      lprcat ("How much gold do you drop? ");
	      if ((amt = readnum ((int) c[GOLD])) == 0)
		return;
	      if (amt > c[GOLD])
		{
		  lprcat ("\n");
		  lprcat ("You don't have that much!");
		  return;
		}
	      if (amt <= 32767)
		{
		  *p = OGOLDPILE;
		  i = amt;
		}
	      else if (amt <= 327670L)
		{
		  *p = ODGOLD;
		  i = amt / 10;
		  amt = 10L * i;
		}
	      else if (amt <= 3276700L)
		{
		  *p = OMAXGOLD;
		  i = amt / 100;
		  amt = 100L * i;
		}
	      else if (amt <= 32767000L)
		{
		  *p = OKGOLD;
		  i = amt / 1000;
		  amt = 1000L * i;
		}
	      else
		{
		  *p = OKGOLD;
		  i = 32767;
		  amt = 32767000L;
		}
	      c[GOLD] -= amt;

	      lprintf ("\nYou drop %d gold pieces", (int) amt);

	      iarg[playerx][playery] = i;
	      bottomgold ();
	      know[playerx][playery] = 0;
	      dropflag = 1;
	      return;
	    }
	  if (i)
	    {
	      drop_object (i - 'a');
	      return;
	    }
	}
    }
}


static int
floor_consume (int search_item, char *cons_verb)
{
  int i;
  char tempc;

  cursors ();
  i = item[playerx][playery];

  /* item not there, quit
   */
  if (i != search_item)
    return (0);

  /* item there.  does the player want to consume it?
   */
  lprintf ("\nThere is %s", objectname[i]);
  if (i == OSCROLL)
    if (scrollname[iarg[playerx][playery]][0])
      lprintf (" of%s", scrollname[iarg[playerx][playery]]);
  if (i == OPOTION)
    if (potionname[iarg[playerx][playery]][0])
      lprintf (" of%s", potionname[iarg[playerx][playery]]);
  lprintf (" here.  Do you want to %s it?", cons_verb);

  if ((tempc = getyn ()) == 'n')
    return (0);			/* item there, not consumed */
  else if (tempc != 'y')
    {
      lprcat (" aborted");
      return (-1);		/* abort */
    }

  /* consume the item.
   */
  switch (i)
    {
    case OCOOKIE:
		lprcat("\n\nYou have found a fortune cookie.");
		ocookie();
      break;
    case OBOOK:
      readbook (iarg[playerx][playery]);
      forget ();
      break;
    case OPOTION:
      quaffpotion (iarg[playerx][playery], 1);
      forget ();
      break;
    case OSCROLL:
      /* scrolls are tricky because of teleport.
       */
      i = iarg[playerx][playery];
      know[playerx][playery] = 0;
      item[playerx][playery] = iarg[playerx][playery] = 0;
      read_scroll (i);
      break;
    }
  return (1);
}



static void
consume (int search_item, char *prompt, int (*showfunc) (void))
{
  int i;

  for (;;)
    {
      if ((i = whatitem (prompt)) == '\33')
	return;
      if (i != '.' && i != '-')
	{
	  if (i == '*')
	    {
	      i = showfunc ();
	      cursors ();
	    }
	  if (i && i != '.')
	    {
	      switch (iven[i - 'a'])
		{
		case OSCROLL:
		  if (search_item != OSCROLL)
		    {
		      lprintf ("\nYou can't %s that.", prompt);
		      return;
		    }
		  read_scroll (ivenarg[i - 'a']);
		  break;
		case OBOOK:
		  if (search_item != OSCROLL)
		    {
		      lprintf ("\nYou can't %s that.", prompt);
		      return;
		    }
		  readbook (ivenarg[i - 'a']);
		  break;
		case OCOOKIE:
		  if (search_item != OCOOKIE)
		    {
		      lprintf ("\nYou can't %s that.", prompt);
		      return;
		    }
		  ocookie();
		  break;
		case OPOTION:
		  if (search_item != OPOTION)
		    {
		      lprintf ("\nYou can't %s that.", prompt);
		      return;
		    }
		  quaffpotion (ivenarg[i - 'a'], TRUE);
		  break;
		case 0:
		  ydhi (i);
		  return;
		default:
		  lprintf ("\nYou can't %s that.", prompt);
		  return;
		}
	      iven[i - 'a'] = 0;
	      return;
	    }
	}
    }
}



/*
function to ask what player wants to do
*/
static int
whatitem (char *str)
{
  int i = 0;

  cursors ();
  lprintf ("\nWhat do you want to %s [* for all] ? ", str);
  while (i > 'z'
	 || (i < 'a' && i != '-' && i != '*' && i != '\33' && i != '.'))
    i = ttgetch ();
  if (i == '\33')
    lprcat (" aborted");

  return (i);
}




/*
subroutine to get a number from the player
and allow * to mean return amt, else return the number entered
*/
int
readnum (int mx)
{
  int i;
  int amt = 0;

  sncbr ();
  /* allow him to say * for all gold 
   */
  if ((i = ttgetch ()) == '*')
    amt = mx;
  else
    /* read chars into buffer, deleting when requested */
    while (i != '\n')
      {
	if (i == '\033')
	  {
	    scbr ();
	    lprcat (" aborted");
	    return (0);
	  }
	if ((i <= '9') && (i >= '0') && (amt < 999999999))
	  amt = amt * 10 + i - '0';
	if ((i == '\010') || (i == '\177'))
	  amt = (int) (amt / 10);
	i = ttgetch ();
      }
  scbr ();
  return (amt);
}

/*
* routine to check the time of day and return 1 if its during work hours
* checks the file ".holidays" for forms like "mmm dd comment..."
*/
int
playable(void)
{
	FILE* fp;
	char line[128];
	char hol_month[4];
	char cur_month[4];
	int hol_day, hol_year;
	int cur_day, cur_year, cur_hour;
	time_t now;
	char* date;

	time(&now);
	date = ctime(&now);  /* example format "Fri Jul  4 00:27:56 1986\n" */

	/* year */
	cur_year = atoi(date + 20);

	/* hour */
	cur_hour = (date[11] - '0') * 10 + (date[12] - '0');

	/* day (handles leading space) */
	if (date[8] != ' ')
		cur_day = (date[8] - '0') * 10 + (date[9] - '0');
	else
		cur_day = (date[9] - '0');

	/* month */
	strncpy(cur_month, date + 4, 3);
	cur_month[3] = '\0';

	/* 8AM–5PM, Mon–Fri */
	if (cur_hour >= 8 && cur_hour < 17 &&
		strncmp(date, "Sat", 3) != 0 &&
		strncmp(date, "Sun", 3) != 0)
	{
		/* check holidays */
		fp = fopen(holifile, "r");
		if (fp) {
			while (fgets(line, sizeof(line), fp)) {

				/* expected format "Jul 04 1986 comment" */
				if (sscanf(line, "%3s %d %d", hol_month, &hol_day, &hol_year) == 3) {

					if (strcmp(hol_month, cur_month) == 0 &&
						hol_day == cur_day &&
						hol_year == cur_year)
					{
						fclose(fp);
						return 0;   /* holiday */
					}
				}
			}
			fclose(fp);
		}
		return 1;   /* not playable during work hours */
	}
	return 0;       /* outside work hours */
}
