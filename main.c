/* main.c */
#include <stdio.h>
#if defined WINDOWS_VS
#include "includes/larnwin32.h"
#endif
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

		/* needed for hack fix to handle endwin()
				   not being called after process commandline */	

static void parse (void);

static void randmonst (void);

static void run (int);

static void wield (void);

static void ydhi (int);
static void ycwi (int);

static void wear (void);

static void dropobj (void);

static int floor_consume (int, char *);

static void consume (int, char *, int (*)(void));

static int whatitem (char *);

int dropflag = 0;		/* if 1 then don't lookforobject() next round */
int rmst = 80;			/*  random monster creation counter     */
int nomove = 0;			/* if (nomove) then don't count next iteration as a
				   move */
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
#if defined WINDOWS_VS
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
#endif

#if defined NIX
int
main (int argc, char *argv[])
#endif
{
  int i;
  int hard = -1;

  FILE *pFile;
	
  

#if defined WINDOWS_VS
  strcpy(savefilename,getenv("APPDATA"));
  strcat(savefilename,"/Larn/larnsavefile");

  strcpy(scorefile,getenv("APPDATA"));
  strcat(scorefile,"/Larn/larnscorefile");

  strcpy(logfile,getenv("APPDATA"));
  strcat(logfile,"/Larn/larnlogfile");

  strcpy(fortfile,getenv("APPDATA"));
  strcat(fortfile,"/Larn/larnforts");

  strcpy(playerids,getenv("APPDATA"));
  strcat(playerids,"/Larn/larnplayerid");

  strcpy(mazefile,getenv("APPDATA"));
  strcat(mazefile,"/Larn/larnmazefile");
#ifdef EXTRA
  strcpy(diagfile,getenv("APPDATA"));
  strcat(diagfile,"/Larn/larndiagfile");
#endif
#else

//herausfinden, welcher save verwendet werden soll
int savespace = 0;
char str_savespace[10];
printf("Which Save should be loaded?\n\n");

// LOAD SAVINGSFILE
char savespaceinfo[100][2][100];
char savespaceinfofile[100];
strcpy(savespaceinfofile,getenv("HOME"));
strcat(savespaceinfofile,"/.larn/larnsavefileinfo");

FILE *datei = fopen(savespaceinfofile, "r");
if (datei == NULL) {
    strcpy(savespaceinfo[0][0],"the end");
} else {
	char buffer[100];  // Ein Puffer zum Lesen von Zeichenketten

	int zaehlervariable_saveinfo = 0;

	// Zeichenketten aus der Datei lesen und anzeigen
	while (fgets(buffer, sizeof(buffer), datei) != NULL) {
		buffer[strcspn(buffer, "\n")] = '\0';
    	printf("%s", buffer);
		if (zaehlervariable_saveinfo%2 != 0) printf("\n");
		else printf("\t");
		strcpy(savespaceinfo[zaehlervariable_saveinfo/2][zaehlervariable_saveinfo%2], buffer);
		zaehlervariable_saveinfo++;
	}
	fclose(datei);
	strcpy(savespaceinfo[zaehlervariable_saveinfo/2][0],"the end");
}

// ENTER THE NUMBER
printf("\nEnter number and press ENTER: ");
if (scanf("%d", &savespace)!=1) {
	printf("\n\nFehler bei der Eingabe!!!!\nABBRUCH...");
	return -27;
}
printf("\nYou entered %d.\n", savespace);
sprintf(str_savespace, "%d", savespace);

// CHECK IF NEW 
int laufvar = 0;
while (1==1){
	if ( strcmp(savespaceinfo[laufvar][0], "the end") == 0 ) {
		strcpy(savespaceinfo[laufvar+1][0],"the end");
		strcpy(savespaceinfo[laufvar][0],str_savespace);

		printf("\nEnter commentary(1 word) to this game you want to start. Then press ENTER.\nCommentary: ");
		char temparr[100];
		scanf("%99s",temparr);
		strcpy(savespaceinfo[laufvar][1],temparr);

		break;
	}
	if ( savespace == atoi(savespaceinfo[laufvar][0]) ) break;
	laufvar++;
}

// SAVE SAVINGSFILE
datei = fopen(savespaceinfofile, "w");

if (datei == NULL) {
	perror("Fehler beim Öffnen der Datei");
	return 1;
}

// String-Array in die Datei schreiben
int zaehlervariable_saveinfo = 0;
while (1 == 1) {
	if ( strcmp(savespaceinfo[zaehlervariable_saveinfo/2][0], "the end") == 0 ) break;
	fprintf(datei, "%s\n", savespaceinfo[zaehlervariable_saveinfo/2][zaehlervariable_saveinfo%2]);
	zaehlervariable_saveinfo++;
}

// Datei schließen
fclose(datei);


  strcpy(savefilename,getenv("HOME"));
  strcat(savefilename,"/.larn/larnsavefile");
  strcat(savefilename,str_savespace);

  strcpy(scorefile,getenv("HOME"));
  strcat(scorefile,"/.larn/larnscorefile");

  strcpy(logfile,getenv("HOME"));
  strcat(logfile,"/.larn/larnlogfile");

  strcpy(fortfile,getenv("HOME"));
  strcat(fortfile,"/.larn/larnforts");

  strcpy(playerids,getenv("HOME"));
  strcat(playerids,"/.larn/larnplayerid");

  strcpy(mazefile,getenv("HOME"));
  strcat(mazefile,"/.larn/larnmazefile");
#ifdef EXTRA
  strcpy(diagfile,getenv("HOME"));
  strcat(diagfile,"/.larn/larndiagfile");
#endif
#endif


  /*
   *  first task is to identify the player
   */
  /*init curses ~Gibbon */
  init_term ();			/* setup the terminal (find out what type) for termcap */
  scbr ();
  /*
   *  second task is to prepare the pathnames the player will need
   */


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

#if defined WINDOWS_VS
  LPWSTR* szArgList;
  int argCount;

  szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
#endif

  /*
   *  now process the command line arguments 
   */
#if defined WINDOWS_VS
  for (i = 1; i < argCount; i++)
#else
  for (i = 1; i < argc; i++)
#endif
    {
#if defined WINDOWS_VS
	  if (szArgList[i][0] == '-')
		  switch (szArgList[i][1])
#else
      if (argv[i][0] == '-')
		  switch (argv[i][1])
#endif
	  {
	  case 's':		/* show scoreboard   */
	    showscores ();
	    lprcat ("Press any key to exit...");
	    ttgetch ();
	    ansiterm_clean_up ();	/* hacky way */
	    exit (EXIT_SUCCESS);

	  case 'i':		/* show all scoreboard */
	    showallscores ();
	    lprcat ("Press any key to exit...");
	    ttgetch ();
	    ansiterm_clean_up ();
	    exit (EXIT_SUCCESS);

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
#if defined WINDOWS_VS
		  hard = atol(&szArgList[i][1]);
#else
		  hard = atol(&argv[i][1]);
#endif
	    break;

	  case 'h':		/* print out command line arguments */
	  case '?':
#if defined WINDOWS_VS
		  MessageBox(NULL, L"-s = Show scores\n-i = Show all scores (including inventory at time of death)\n-0 to -9 = Difficulty setting\n-h or -? = This help text", L"HELP TEXT", MB_OK);
#else
	    ansiterm_clean_up ();
	    puts (cmdhelp);
		lprcat("Press any key to exit...");
		ttgetch();
#endif
		exit (EXIT_SUCCESS);
	  default:
	    ansiterm_clean_up ();
#if defined WINDOWS_VS
		MessageBox(NULL, L"Unknown command line option.  Use -h or -? for help.\n", L"UNKNOWN OPTION", MB_OK);
#else
		printf("Unknown option <%s>\n", argv[i]);
#endif
	    puts (cmdhelp);
	    exit (EXIT_SUCCESS);
	  };
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

  lcreat ((char *) 0);
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

  setupvt100 ();		/*  setup the terminal special mode             */
  if (cdesc[HP] == 0)		/* create new game */
    {
      predostuff = 1;		/* tell signals that we are in the welcome screen */
      welcome ();		/* welcome the player to the game */

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
      if (dropflag == 0)
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
      if (cdesc[TIMESTOP] <= 0)
	if (cdesc[HASTESELF] == 0 || (cdesc[HASTESELF] & 1) == 0)
	  {
	    gtime++;
	    movsphere ();

	    if (hitflag == 0)
	      {
		if (cdesc[HASTEMONST])
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

      if (hit3flag)
#if defined WINDOWS_VS
lflushall();
#endif

#if defined NIX
fflush(NULL);
#endif 
      hitflag = hit3flag = 0;
      bot_linex ();		/* update bottom line */

      /* get commands and make moves
       */
      nomove = 1;
      while (nomove)
	{
	  if (hit3flag)
#if defined WINDOWS_VS
lflushall();
#endif

#if defined NIX
fflush(NULL);
#endif 
	  nomove = 0;
	  parse ();
	}
      regen ();			/*  regenerate hp and spells            */
      if (cdesc[TIMESTOP] == 0)
	if (--rmst <= 0)
	  {
	    rmst = 120 - (level << 2);
	    fillmonst (makemonst (level));
	  }
    }
#if defined WINDOWS_VS
  return 0;
#endif
}

/*
* subroutine to randomly create monsters if needed
*/
static void
randmonst (void)
{

  /*  don't make monsters if time is stopped  */
  if (cdesc[TIMESTOP])
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
	  if (cdesc[TIMESTOP] == 0)
	    dropobj ();
	  return;		/*  to drop an object   */

	case 'e':
	  yrepcount = 0;
	  if (cdesc[TIMESTOP] == 0)
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
	  showstr (FALSE);
	  return;

	case 'p':		/* pray at an altar */
	  yrepcount = 0;
	  pray_at_altar ();
	  return;

	case 'q':		/* quaff a potion */
	  yrepcount = 0;
	  if (cdesc[TIMESTOP] == 0)
	    if (!floor_consume (OPOTION, "quaff"))
	      consume (OPOTION, "quaff", showquaff);
	  return;

	case 'r':
	  yrepcount = 0;
	  if (cdesc[BLINDCOUNT])
	    {
	      cursors ();
	      lprcat ("\nYou can't read anything when you're blind!");
	    }
	  else if (cdesc[TIMESTOP] == 0)
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
		   (int) SUBVERSION, (int) PATCHLEVEL, (int) cdesc[HARDGAME]);

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
		display_help_text();
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
	  return;

	case 'R':		/* remove gems from a throne */
	  yrepcount = 0;
	  remove_gems ();
	  return;

	case 'S':
	  /* And do the save.
	   */
	  cursors();
	  lprintf("\nSaving to `%s' . . . ", savefilename);
	  lflush();
	  save_mode = 1;
	  savegame(savefilename);
	  screen_clear();
	  lflush();
	  wizard = 1;
	  died(-257);		/* doesn't return */
	  break;


	case 'T':
	  yrepcount = 0;
	  cursors ();
	  if (cdesc[SHIELD] != -1)
	    {
	      cdesc[SHIELD] = -1;
	      lprcat ("\nYour shield is off");
	      bottomline ();
	    }
	  else if (cdesc[WEAR] != -1)
	    {
	      cdesc[WEAR] = -1;
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
	  if (cdesc[LEVEL] > 9)
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
	      cdesc[i] = 70;
	    }
	  iven[0] = iven[1] = 0;
	  take (OPROTRING, 50);
	  take (OLANCE, 25);
	  cdesc[WIELD] = 1;
	  cdesc[LANCEDEATH] = 1;
	  cdesc[WEAR] = cdesc[SHIELD] = -1;
	  raiseexperience (6000000L);
	  cdesc[AWARENESS] += 25000;
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
	  cdesc[GOLD] += 250000;
	  drawscreen ();
	  return;
#endif

	};
    }
}



void
parse2 (void)
{

  /* move the monsters */
  if (cdesc[HASTEMONST])
    {

      movemonst ();
    }

  movemonst ();

  randmonst ();

  regen ();
}



static void
run (int dir)
{
  int i;

  i = 1;

  while (i)
    {

      i = moveplayer (dir);

      if (i > 0)
	{

	  if (cdesc[HASTEMONST])
	    {

	      movemonst ();
	    }

	  movemonst ();
	  randmonst ();
	  regen ();
	}

      if (hitflag)
	{

	  i = 0;
	}

      if (i != 0)
	{

	  showcell (playerx, playery);
	}
  gtime++;
    }
}



/*
* function to wield a weapon
*/
static void
wield (void)
{
  int i;

  for (;;)
    {

      i = whatitem ("wield (- for nothing)");
      if (i == '\33')
	return;


      if (i != '.')
	{

	  if (i == '*')
	    {

	      i = showwield ();
	      cursors ();
	    }

	  if (i == '-')
	    {

	      cdesc[WIELD] = -1;
	      bottomline ();

	      return;
	    }

	  if (!i || i == '.')
	    {

	      continue;
	    }

	  if (iven[i - 'a'] == 0)
	    {

	      ydhi (i);
	      return;

	    }
	  else if (iven[i - 'a'] == OPOTION)
	    {

	      ycwi (i);
	      return;

	    }
	  else if (iven[i - 'a'] == OSCROLL)
	    {

	      ycwi (i);
	      return;

	    }
	  else if (cdesc[SHIELD] != -1 && iven[i - 'a'] == O2SWORD)
	    {

	      lprcat ("\nBut one arm is busy with your shield!");
	      return;

	    }
	  else if (cdesc[SHIELD] != -1 && iven[i - 'a'] == OHSWORD)
	    {

	      lprcat ("\nA longsword of slashing cannot be used while a shield is equipped!");
	      return;

	    }
	  else
	    {

	      cdesc[WIELD] = i - 'a';

	      if (iven[i - 'a'] == OLANCE)
		{

		  cdesc[LANCEDEATH] = 1;

		}
	      else
		{

		  cdesc[LANCEDEATH] = 0;
		}

	      bottomline ();
	      return;
	    }
	}
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
* common routine to say you can't wield an item
*/
static void
ycwi (int x)
{
  cursors ();

  lprintf ("\nYou can't wield item %c!", x);
}



/*
function to wear armor
*/
static void
wear (void)
{
  int i;

  for (;;)
    {
      if ((i = whatitem ("wear")) == '\33')
	return;
      if (i != '.' && i != '-')
	{
	  if (i == '*')
	    {
	      i = showwear ();
	      cursors ();
	    }
	  if (i && i != '.')
	    switch (iven[i - 'a'])
	      {
	      case 0:
		ydhi (i);
		return;
	      case OLEATHER:
	      case OCHAIN:
	      case OPLATE:
	      case ORING:
	      case OSPLINT:
	      case OPLATEARMOR:
	      case OSTUDLEATHER:
	      case OSSPLATE:
		if (cdesc[WEAR] != -1)
		  {
		    lprcat ("\nYou're already wearing some armor");
		    return;
		  }
		cdesc[WEAR] = i - 'a';
		bottomline ();
		return;
	      case OSHIELD:
		if (cdesc[SHIELD] != -1)
		  {
		    lprcat ("\nYou are already wearing a shield");
		    return;
		  }
		if (iven[cdesc[WIELD]] == O2SWORD)
		  {
		    lprcat
		      ("\nYour hands are busy with the two handed sword!");
		    return;
		  }
		  if (iven[cdesc[WIELD]] == OHSWORD)
		  {
		    lprcat("\nYou are holding a longsword of slashing!");
			return;
		  }
		cdesc[SHIELD] = i - 'a';
		bottomline ();
		return;
	      default:
		lprcat ("\nYou can't wear that!");
	      };
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
	  i = showstr (TRUE);
	  cursors ();
	}
      if (i != '-')
	{
	  if (i == '.')		/* drop some gold */
	    {
	      if (*p)
		{
		  lprintf ("\nThere's something here already: %s",
			   objectname[item[playerx][playery]]);
		  dropflag = 1;
		  return;
		}
	      lprcat ("\n\n");
	      cl_dn (1, 23);
	      lprcat ("How much gold do you drop? ");
	      if ((amt = readnum ((int) cdesc[GOLD])) == 0)
		return;
	      if (amt > cdesc[GOLD])
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
	      cdesc[GOLD] -= amt;

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
      outfortune ();
      forget ();
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
		  outfortune ();
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
