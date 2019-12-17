

/*
*
* macros
*
*/

/* macro to create scroll #'s with probability of occurrence */
#define newscroll() (scprob[rund(81)])

/* macro to return a potion # created with probability of occurrence */
#define newpotion() (potprob[rund(41)])

/* macro to return the + points on created leather armor */
#define newleather() (nlpts[rund(cdesc[HARDGAME]?13:15)])

/* macro to return the + points on chain armor */
#define newchain() (nch[rund(10)])

/* macro to return + points on plate armor */
#define newplate() (nplt[rund(cdesc[HARDGAME]?4:12)])

/* macro to return + points on new daggers */
#define newdagger() (ndgg[rund(13)])

/* macro to return + points on new swords */
#define newsword() (nsw[rund(cdesc[HARDGAME]?6:13)])

/* macro to destroy object at present location */
#define forget() (item[playerx][playery]=know[playerx][playery]=0)

/* macro to wipe out a monster at a location */
#define disappear(x,y) (mitem[x][y]=know[x][y]=0)

/* macro to turn on bold display for the terminal */
#define setbold() (*lpnt++ = ST_START)

/* macro to turn off bold display for the terminal */
#define resetbold() (*lpnt++ = ST_END)

/* macro to setup the scrolling region for the terminal */
#define setscroll() enable_scroll=1

/* macro to clear the scrolling region for the terminal */
#define resetscroll() enable_scroll=0

/* macro to clear the screen and home the cursor */
#define screen_clear() (*lpnt++ =CLEAR, regen_bottom=TRUE)

/* macro to clear to end of line */
#define cltoeoln() (*lpnt++ = CL_LINE)

/* macros to seed the random number generator */
/* This is needed on Windows which throws an error due to 'random' not being defined on MingW.  I'll clean it up later. -Gibbon */
#define srandom srand
#define random rand

/* New random seed function. -Gibbon */
#define srand srandom
#define rand random

#define rnd(x)  ((int)(rand() % (x)) + 1)
#define rund(x) ((int)(rand() % (x)))

/* min/max */
#ifndef min
#define min(x,y) (((x)>(y))?(y):(x))
#endif
#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif



/*
*
* function declarations
*
*/

#ifndef WINDOWS
#define _getch ansiterm_getch
#endif
