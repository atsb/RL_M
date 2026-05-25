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

 /* io.c
*
*  setupvt100()    Subroutine to set up terminal in correct mode for game
*  clearvt100()    Subroutine to clean up terminal when the game is over
*  ttgetch()       Routine to read in one character from the terminal
*  scbr()          Function to set cbreak -echo for the terminal
*  sncbr()         Function to set -cbreak echo for the terminal
*  newgame()       Subroutine to save the initial time and seed rnd()
*
*  FILE OUTPUT ROUTINES
*
*  lprintf(format,args . . .)  printf to the output buffer
*  lprint(integer)         send binary integer to output buffer
*  lwrite(buf,len)         write a buffer to the output buffer
*  lprcat(str)         sent string to output buffer
*
*  FILE OUTPUT MACROS
*
*  lprc(character)         put the character into the output buffer
*
*  FILE INPUT ROUTINES
*
*  int lgetc()            read one character from input buffer
*  int larint()            read one integer from input buffer
*  lrfill(address,number)      put input bytes into a buffer
*  char *lgetw()           get a whitespace ended word from input
*  char *lgetl()           get a \n or EOF ended line from input
*
*  FILE OPEN / CLOSE ROUTINES
*
*  lcreat(filename)        create a new file for write
*  lopen(filename)         open a file for read
*  lappend(filename)       open for append to an existing file
*  lrclose()           close the input file
*  lwclose()           close output file
*  lflush()            flush the output buffer
*
*  Other Routines
*
*  cursor(x,y)     position cursor at [x,y]
*  cursors()       position cursor at [1,24] (saves memory)
*  cl_line(x,y)            Clear line at [1,y] and leave cursor at [x,y]
*  cl_up(x,y)          Clear screen from [x,1] to current line.
*  cl_dn(x,y)      Clear screen from [1,y] to end of display. 
*  lstandout(str)       Print the string in standout mode.
*  set_score_output()  Called when output should be literally printed.
** ttputch(ch)     Print one character in decoded output buffer.
** flush_buf()     Flush buffer with decoded output.
** init_term()     Terminal initialization
*
* Note: ** entries are available only in termcap mode.
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h> 
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <fcntl.h>		/* For O_BINARY */

#include "larn.h"

#if !defined(_WIN32)
#include <sys/ioctl.h>
#endif

#include "display.h"
#include "global.h"
#include "io.h"
#include "monster.h"
#include "scores.h"

#define LINBUFSIZE 128		/* size of the lgetw() and lgetl() buffer       */

int lfd = 0;		/*  output file numbers     */
int fd;				/*  input file numbers      */

static void flush_buf (void);
static char lgetwbuf[LINBUFSIZE];	/* get line (word) buffer               */
static char *outbuf = NULL;
static int curx = 0;
static int cury = 0;
static int available, tocopy;
static int ipoint = MAXIBUF, iepoint = MAXIBUF;	/*  input buffering pointers    */
static int (*getchfn) (void);
static int scrline = 21;	/* line # for wraparound instead of scrolling if no DL */
static int io_index = 0;

/* wgetch() is the modern way. -Gibbon */
static int
llgetch(void)
{
    int key;
    key = wgetch(stdscr);

#ifdef PDC_KEY_MODIFIER_SHIFT
    if (PDC_get_key_modifiers() & PDC_KEY_MODIFIER_SHIFT)
    {
        switch (key)
        {
        case '1':
            return 'B';
        case '2':
            return 'J';
        case '3':
            return 'N';
        case '4':
            return 'H';
        case '5':
            return '.';
        case '6':
            return 'L';
        case '7':
            return 'Y';
        case '8':
            return 'K';
        case '9':
            return 'U';
        }
    }
#endif
    switch (key)
    {
    case KEY_UP:
        return 'k';
    case KEY_DOWN:
        return 'j';
    case KEY_LEFT:
        return 'h';
    case KEY_RIGHT:
        return 'l';
    case KEY_A1:
        return 'y';
    case KEY_A3:
        return 'u';
    case KEY_C1:
        return 'b';
    case KEY_C3:
        return 'n';
    case KEY_B2:
        return '.';
    case KEY_ENTER:
        return 13;
    default:
        return key;
    }
}

/*
* get char
*/
int
term_getch(void)
{
    return llgetch();
}

/*
* get char (with echo)
*/
int
term_getche(void)
{
    int key;
    echo();
    key = llgetch();
    noecho();
    return key;
}

void
term_delch(void)
{
    delch();
}

static void
cleanup_term(void)
{
    /* restore terminal modes */
    nocbreak();
    echo();
    nl();

    /* ensure cursor is visible again */
    printf("\033[0m");
    printf("\033[?25h");
    fflush(stdout);

    endwin();
}

/*
*  Subroutine to set up terminal in correct mode for game
*
*  Attributes off, clear screen, set scrolling region, set tty mode 
*/
void
setupvt100 (void)
{
  screen_clear();

  setscroll ();
}

/*
*  Subroutine to clean up terminal when the game is over
*
*  Attributes off, clear screen, unset scrolling region, restore tty mode 
*/
void
clearvt100 (void)
{
  cleanup_term();

  resetscroll ();
}

/*
* ttgetch()       Routine to read in one character from the terminal
*/
char
ttgetch (void)
{
  char byt;

#ifdef EXTRA
  c[BYTESIN]++;
#endif

  lflush ();			/* be sure output buffer is flushed */

  byt = (char) (*getchfn) ();

  if (byt == '\r')
    {

      byt = '\n';
    }

  return byt;
}

/* non‑blocking ttgetch: returns -1 if no key available */
int
ttgetch_noblock(void)
{
    int ch;

    lflush();

    nodelay(stdscr, TRUE);
    ch = (*getchfn)();
    nodelay(stdscr, FALSE);

    if (ch == ERR)
        return -1;

    if (ch == '\r')
        ch = '\n';

    /* count real input bytes for checkpointing */
    c[BYTESIN]++;

    return ch;
}

/*
* scbr()      Function to set cbreak -echo for the terminal
*
* like: system("stty cbreak -echo")
*/
void
scbr (void)
{

  /* 
   * Set up to use the direct console input call which may
   * read from the keypad;
   */
  getchfn = term_getch;
  curs_set(0);
}

/*
* sncbr()     Function to set -cbreak echo for the terminal
*
* like: system("stty -cbreak echo")
*/
void
sncbr (void)
{

  /* 
   * Set up to use the direct console input call with echo, getche()
   */
  getchfn = term_getche;
  curs_set(1);
}

/*
* newgame()       Subroutine to save the initial time and seed rnd()
*/
void
newgame (void)
{
  long *p, *pe;

  for (p = c, pe = c + 100; p < pe; p++)
    *p = 0;

  time (&initialtime);

  srand ((unsigned)initialtime);

  lcreat ((char *) 0);		/* open buffering for output to terminal */
}

/*
*  lprintf(format,args . . .)      printf to the output buffer
*      char *format;
*      ??? args . . .
*
*  Enter with the format string in "format", as per printf() usage
*      and any needed arguments following it
*  Note: lprintf() only supports %s, %c and %d, with width modifier and left
*      or right justification.
*  No correct checking for output buffer overflow is done, but flushes 
*      are done beforehand if needed.
*  Returns nothing of value.
*/
void
lprintf (const char *fmt, ...)
{
  va_list vl;
  char buffer[STRING_BUFFER_SIZE];
  const char *p;

  va_start (vl, fmt);
  vsprintf (buffer, fmt, vl);
  va_end (vl);

  p = buffer;

  while (*p != '\0')
    {
      lprc (*p);
      ++p;
    }
}

/*
*  lprint(int-integer)                send binary integer to output buffer
*      int integer;
*
*      +---------+---------+---------+---------+
*      |   high  |         |         |   low   |
*      |  order  |         |         |  order  |
*      |   byte  |         |         |   byte  |
*      +---------+---------+---------+---------+
*     31  ---  24 23 --- 16 15 ---  8 7  ---   0
*
*  The save order is low order first, to high order (4 bytes total)
*      and is written to be system independent.
*  No checking for output buffer overflow is done, but flushes if needed!
*  Returns nothing of value.
* 
* Made Endian independent and more efficient. ~Gibbon
*/
void lprint(int x)
{
    unsigned char b0 = (unsigned char)(x & 0xFF);
    unsigned char b1 = (unsigned char)((x >> 8) & 0xFF);
    unsigned char b2 = (unsigned char)((x >> 16) & 0xFF);
    unsigned char b3 = (unsigned char)((x >> 24) & 0xFF);

    if (lpnt + 4 > lpend)
        lflush();

    lpnt[0] = b0;
    lpnt[1] = b1;
    lpnt[2] = b2;
    lpnt[3] = b3;

    lpnt += 4;
}

/* 
* output one byte to the output buffer 
*/
/* message scroll ring: lines 21–24 */
void
lprc(char ch)
{
    if (lfd > 2) {
        unsigned char* p = (unsigned char*)lpnt;
        *p = (unsigned char)ch;
        lpnt = (void*)(p + 1);

        if (lpnt >= lpend)
            lflush();
        return;
    }

    if (ch == '\n' && enable_scroll) {
        if (cury >= 20) {
            scrline++;
            if (scrline > 24)
                scrline = 21;
            move(scrline - 1, 0);
            clrtoeol();
            return;
        }
        else {
            addch('\n');
            return;
        }
    }
    else if (ch == '\t')
        addstr("    ");
    else
        addch(ch);

#ifdef EXTRA
    c[BYTESOUT]++;
#endif
}

/*
*  lwrite(buf,len)         write a buffer to the output buffer
*      char *buf;
*      int len;
*  
*  Enter with the address and number of bytes to write out
*  Returns nothing of value
*/void
lwrite(char* buf, int len)
{
    unsigned char* src = (unsigned char*)buf;
    int i;

    if (len > 399) {
#ifdef EXTRA
        c[BYTESOUT] += len;
#endif
        while (len--)
            lprc(*src++);
        return;
    }

    while (len) {
        if (lpnt >= lpend)
            lflush();

        int num2 = (int)(lpend - lpnt);
        if (num2 > len)
            num2 = len;

        unsigned char* dst = (unsigned char*)lpnt;

        for (i = 0; i < num2; i++)
            dst[i] = src[i];
        lpnt = (char*)(dst + num2);
        src += num2;
        len -= num2;
    }
}

/*
*  int lgetc()        Read one character from input buffer
*
*  Returns 0 if EOF, otherwise the character
*/
char
lgetc(void)
{
    if (ipoint >= iepoint)
    {
        int n = read(fd, inbuffer, MAXIBUF);
        if (n <= 0)
        {
            return EOF;   /* distinguishable */
        }
        ipoint = 0;
        iepoint = n;
    }

    return inbuffer[ipoint++];
}

/*
*  int larint()            Read one integer from input buffer
*
*      +---------+---------+---------+---------+
*      |   high  |         |         |   low   |
*      |  order  |         |         |  order  |
*      |   byte  |         |         |   byte  |
*      +---------+---------+---------+---------+
*     31  ---  24 23 --- 16 15 ---  8 7  ---   0
*
*  The save order is low order first, to high order (4 bytes total)
*  Returns the int read
*/
int
larint (void)
{
  int i;
  i = 255 & lgetc ();
  i |= (255 & lgetc ()) << 8;
  i |= (255 & lgetc ()) << 16;
  i |= (255 & lgetc ()) << 24;
  return (i);
}

/*
*  lrfill(address,number)          put input bytes into a buffer
*      char *address;
*      int number;
*
*  Reads "number" bytes into the buffer pointed to by "address".
*  Returns nothing of value
*/
void
lrfill(char* adr, int num)
{
    while (num > 0)
    {
        if (ipoint >= iepoint)
        {
            int n = read(fd, inbuffer, MAXIBUF);
            if (n <= 0)
            {
                fprintf(stderr, "lrfill: unexpected EOF or read error\n");
                memset(adr, 0, num);
                return;
            }
            ipoint = 0;
            iepoint = n;
        }

        available = iepoint - ipoint;
        tocopy = (available < num) ? available : num;

        memcpy(adr, inbuffer + ipoint, tocopy);

        adr += tocopy;
        ipoint += tocopy;
        num -= tocopy;
    }
}

/*
*  char *lgetw()           Get a whitespace ended word from input
*
*  Returns pointer to a buffer that contains word.  If EOF, returns a NULL
*/
char *
lgetw (void)
{
  char *lgp, cc;
  int n = LINBUFSIZE, quote = 0;

  lgp = lgetwbuf;
  do
    cc = lgetc ();
  while ((cc <= 32) && (cc > '\0'));	/* eat whitespace */
  for (;; --n, cc = lgetc ())
    {
      if ((cc == '\0') && (lgp == lgetwbuf))
	return (NULL);		/* EOF */
      if ((n <= 1) || ((cc <= 32) && (quote == 0)))
	{
	  *lgp = '\0';
	  return lgetwbuf;
	}
      if (cc != '"')
	*lgp++ = cc;
      else
	quote ^= 1;
    }
}

/*
*  char *lgetl()       Function to read in a line ended by newline or EOF
*
*  Returns pointer to a buffer that contains the line.  If EOF, returns NULL
*/
char *
lgetl (void)
{
  int i = LINBUFSIZE;
  char ch;
  char *str = lgetwbuf;

  for (;; --i)
    {
      *str++ = ch = lgetc ();
      if (ch == 0)
	{
	  if (str == lgetwbuf + 1)
	    return (NULL);	/* EOF */
	ot:*str = 0;
	  return (lgetwbuf);	/* line ended by EOF */
	}
      if ((ch == '\n') || (i <= 1))
	goto ot;		/* line ended by \n */
    }
}

/* Just for the record, I removed all those _setmodes for *nix
 * systems a while ago.  No need to declare these
 * things to make it work on them,
 * these systems just don't require it. ~Gibbon
*/

/*
*  lcreat(filename)            Create a new file for write
*      char *filename;
*
*  lcreat((char*)0); means to the terminal
*  Returns -1 if error, otherwise the file descriptor opened.
*  
*  Modernised this function and made it cleaner. ~Gibbon
*/
int
lcreat(char* str)
{
    lpnt = lpbuf;
    lpend = lpbuf + BUFBIG;

    if (str == NULL)
        return -1;

#ifdef _WIN32
    lfd = open(str, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0666);
#else
    lfd = open(str, O_RDWR | O_CREAT | O_TRUNC, 0666);
#endif

    if (lfd < 0)
    {
        fprintf(stderr, "lcreat: cannot create %s\n", str);
        return -1;
    }

    return lfd;
}

/*
*  lopen(filename)         Open a file for read
*      char *filename;
*
*  lopen(0) means from the terminal
*  Returns -1 if error, otherwise the file descriptor opened.
*/
int
lopen(char* str)
{
    ipoint = iepoint = 0;

    if (str == NULL)
        return -1;

#ifdef _WIN32
    fd = open(str, O_RDONLY | O_BINARY);
#else
    fd = open(str, O_RDONLY);
#endif

    if (fd < 0)
    {
        fprintf(stderr, "lopen: cannot open %s\n", str);
        return -1;
    }

    return fd;
}

/*
*  lappend(filename)       Open for append to an existing file
*      char *filename;
*
*  lappend(0) means to the terminal
*  Returns -1 if error, otherwise the file descriptor opened.
*/
int
lappend (char *str)
{
  lpnt = lpbuf;
  lpend = lpbuf + BUFBIG;
  if (str == NULL)
    return (lfd = 1);
	if ((lfd = open(str, 2)) < 0)
    {
      lfd = 1;
      return (-1);
    }
	lseek (lfd, 0L, 2);		/* seek to end of file */
  return lfd;
}

/*
*  lrclose()                       close the input file
*
*  Returns nothing of value.
*/
void
lrclose (void)
{
  if (fd > 0)
    {
		close(fd);
    }
}

/*
*  lwclose()                       close output file flushing if needed
*
*  Returns nothing of value.
*/
void
lwclose (void)
{
  lflush();
  if (lfd > 2)
    {
		close(lfd);
    }
}

/*
*  lprcat(string)                  append a string to the output buffer
*                                  avoids calls to lprintf (time consuming)
*/
void
lprcat(char* str)
{
    if (lfd > 2) {
        unsigned char* dst = (unsigned char*)lpnt;
        unsigned char* src = (unsigned char*)str;

        while (*src) {
            if (dst >= (unsigned char*)lpend)
                lflush();

            *dst++ = *src++;
        }

        if (dst >= (unsigned char*)lpend)
            lflush();

        *dst++ = '\0';

        lpnt = (char*)dst;
        lflush();
        return;
    }
    while (*str)
        lprc(*str++);
}


/*
* cursor(x,y)    Put cursor at specified coordinates staring at [1,1] (termcap)
*/
void
cursor (int x, int y)
{
  curx = x-1;
  cury = y-1;
  move(cury, curx);
}

/*
*  Routine to position cursor at beginning of 24th line
*/
void
cursors (void)
{
  curs_set(0);
  cursor(1, 21);
}

/*
* Warning: ringing the bell is control code 7. Don't use in defines.
* Don't change the order of these defines.
* Also used in helpfiles. Codes used in helpfiles should be \E[1 to \E[7 with
* obvious meanings.
*/
static void
init_colors(void)
{
    int i;

    for (i = 0; i < MAXMONST + 9; i++)
        moncolor[i] = COLOR_WHITE;

    for (i = 0; i < MAXOBJECT + 1; i++)
        objcolor[i] = COLOR_WHITE;

    /* Monster colors */
    moncolor[BAT] = COLOR_BLUE;
    moncolor[GNOME] = COLOR_GREEN;
    moncolor[HOBGOBLIN] = COLOR_RED;
    moncolor[JACKAL] = COLOR_YELLOW;
    moncolor[KOBOLD] = COLOR_CYAN;
    moncolor[ORC] = COLOR_MAGENTA;
    moncolor[SNAKE] = COLOR_GREEN;
    moncolor[CENTIPEDE] = COLOR_RED;
    moncolor[JACULI] = COLOR_BLUE;
    moncolor[TROGLODYTE] = COLOR_WHITE;
    moncolor[ANT] = COLOR_RED;
    moncolor[EYE] = COLOR_CYAN;
    moncolor[LEPRECHAUN] = COLOR_GREEN;
    moncolor[NYMPH] = COLOR_MAGENTA;
    moncolor[QUASIT] = COLOR_RED;
    moncolor[RUSTMONSTER] = COLOR_YELLOW;
    moncolor[ZOMBIE] = COLOR_GREEN;
    moncolor[ASSASSINBUG] = COLOR_RED;
    moncolor[BUGBEAR] = COLOR_YELLOW;
    moncolor[HELLHOUND] = COLOR_RED;
    moncolor[ICELIZARD] = COLOR_CYAN;
    moncolor[CENTAUR] = COLOR_WHITE;
    moncolor[TROLL] = COLOR_GREEN;
    moncolor[YETI] = COLOR_WHITE;
    moncolor[WHITEDRAGON] = COLOR_WHITE;
    moncolor[ELF] = COLOR_GREEN;
    moncolor[CUBE] = COLOR_CYAN;
    moncolor[METAMORPH] = COLOR_MAGENTA;
    moncolor[VORTEX] = COLOR_BLUE;
    moncolor[ZILLER] = COLOR_RED;
    moncolor[VIOLETFUNGI] = COLOR_MAGENTA;
    moncolor[WRAITH] = COLOR_CYAN;
    moncolor[FORVALAKA] = COLOR_RED;
    moncolor[LAMANOBE] = COLOR_WHITE;
    moncolor[OSEQUIP] = COLOR_YELLOW;
    moncolor[ROTHE] = COLOR_RED;
    moncolor[XORN] = COLOR_MAGENTA;
    moncolor[VAMPIRE] = COLOR_RED;
    moncolor[INVISIBLESTALKER] = COLOR_WHITE;
    moncolor[POLTERGEIST] = COLOR_CYAN;
    moncolor[DISENCHANTRESS] = COLOR_MAGENTA;
    moncolor[SHAMBLINGMOUND] = COLOR_GREEN;
    moncolor[YELLOWMOLD] = COLOR_YELLOW;
    moncolor[UMBERHULK] = COLOR_RED;
    moncolor[GNOMEKING] = COLOR_GREEN;
    moncolor[MIMIC] = COLOR_WHITE;
    moncolor[WATERLORD] = COLOR_BLUE;
    moncolor[BRONZEDRAGON] = COLOR_YELLOW;
    moncolor[GREENDRAGON] = COLOR_GREEN;
    moncolor[PURPLEWORM] = COLOR_MAGENTA;
    moncolor[XVART] = COLOR_CYAN;
    moncolor[SPIRITNAGA] = COLOR_RED;
    moncolor[SILVERDRAGON] = COLOR_WHITE;
    moncolor[PLATINUMDRAGON] = COLOR_CYAN;
    moncolor[GREENURCHIN] = COLOR_GREEN;
    moncolor[REDDRAGON] = COLOR_RED;
    moncolor[DEMONLORD] = COLOR_MAGENTA;
    moncolor[DEMONPRINCE] = COLOR_RED;

    /* Object colors */
    objcolor[0] = COLOR_WHITE;
    objcolor[OALTAR] = COLOR_YELLOW;
    objcolor[OTHRONE] = COLOR_MAGENTA;
    objcolor[OORB] = COLOR_CYAN;
    objcolor[OPIT] = COLOR_BLUE;
    objcolor[OSTAIRSUP] = COLOR_WHITE;
    objcolor[OELEVATORUP] = COLOR_WHITE;
    objcolor[OFOUNTAIN] = COLOR_CYAN;
    objcolor[OSTATUE] = COLOR_WHITE;
    objcolor[OTELEPORTER] = COLOR_MAGENTA;
    objcolor[OSCHOOL] = COLOR_GREEN;
    objcolor[OMIRROR] = COLOR_WHITE;
    objcolor[ODNDSTORE] = COLOR_GREEN;
    objcolor[OSTAIRSDOWN] = COLOR_WHITE;
    objcolor[OELEVATORDOWN] = COLOR_WHITE;
    objcolor[OBANK2] = COLOR_GREEN;
    objcolor[OBANK] = COLOR_GREEN;
    objcolor[ODEADFOUNTAIN] = COLOR_BLUE;
    objcolor[OMAXGOLD] = COLOR_YELLOW;
    objcolor[OGOLDPILE] = COLOR_YELLOW;
    objcolor[OOPENDOOR] = COLOR_WHITE;
    objcolor[OCLOSEDDOOR] = COLOR_YELLOW;
    objcolor[OWALL] = COLOR_WHITE;
    objcolor[OLARNEYE] = COLOR_CYAN;
    objcolor[OPLATE] = COLOR_WHITE;
    objcolor[OCHAIN] = COLOR_WHITE;
    objcolor[OLEATHER] = COLOR_YELLOW;
    objcolor[ORING] = COLOR_CYAN;
    objcolor[OSTUDLEATHER] = COLOR_YELLOW;
    objcolor[OSPLINT] = COLOR_WHITE;
    objcolor[OPLATEARMOR] = COLOR_WHITE;
    objcolor[OSSPLATE] = COLOR_WHITE;
    objcolor[OSHIELD] = COLOR_WHITE;
    objcolor[OELVENCHAIN] = COLOR_GREEN;
    objcolor[OSWORDofSLASHING] = COLOR_CYAN;
    objcolor[OHAMMER] = COLOR_YELLOW;
    objcolor[OSWORD] = COLOR_WHITE;
    objcolor[O2SWORD] = COLOR_WHITE;
    objcolor[OHSWORD] = COLOR_CYAN;
    objcolor[OSPEAR] = COLOR_WHITE;
    objcolor[ODAGGER] = COLOR_WHITE;
    objcolor[OBATTLEAXE] = COLOR_RED;
    objcolor[OLONGSWORD] = COLOR_WHITE;
    objcolor[OLANCE] = COLOR_CYAN;
    objcolor[OVORPAL] = COLOR_MAGENTA;
    objcolor[OSLAYER] = COLOR_RED;
    objcolor[ORINGOFEXTRA] = COLOR_CYAN;
    objcolor[OREGENRING] = COLOR_CYAN;
    objcolor[OPROTRING] = COLOR_CYAN;
    objcolor[OENERGYRING] = COLOR_CYAN;
    objcolor[ODEXRING] = COLOR_CYAN;
    objcolor[OSTRRING] = COLOR_CYAN;
    objcolor[OCLEVERRING] = COLOR_CYAN;
    objcolor[ODAMRING] = COLOR_CYAN;
    objcolor[OBELT] = COLOR_YELLOW;
    objcolor[OSCROLL] = COLOR_WHITE;
    objcolor[OPOTION] = COLOR_WHITE;
    objcolor[OBOOK] = COLOR_CYAN;
    objcolor[OCHEST] = COLOR_RED;
    objcolor[OAMULET] = COLOR_MAGENTA;
    objcolor[OORBOFDRAGON] = COLOR_CYAN;
    objcolor[OSPIRITSCARAB] = COLOR_MAGENTA;
    objcolor[OCUBEofUNDEAD] = COLOR_MAGENTA;
    objcolor[ONOTHEFT] = COLOR_RED;
    objcolor[ODIAMOND] = COLOR_WHITE;
    objcolor[ORUBY] = COLOR_RED;
    objcolor[OEMERALD] = COLOR_GREEN;
    objcolor[OSAPPHIRE] = COLOR_BLUE;
    objcolor[OENTRANCE] = COLOR_BLUE;
    objcolor[OVOLDOWN] = COLOR_RED;
    objcolor[OVOLUP] = COLOR_WHITE;
    objcolor[OHOME] = COLOR_CYAN;
    objcolor[OKGOLD] = COLOR_YELLOW;
    objcolor[ODGOLD] = COLOR_YELLOW;
    objcolor[OIVDARTRAP] = COLOR_RED;
    objcolor[ODARTRAP] = COLOR_RED;
    objcolor[OTRAPDOOR] = COLOR_RED;
    objcolor[OIVTRAPDOOR] = COLOR_RED;
    objcolor[OTRADEPOST] = COLOR_GREEN;
    objcolor[OIVTELETRAP] = COLOR_MAGENTA;
    objcolor[ODEADTHRONE] = COLOR_MAGENTA;
    objcolor[OANNIHILATION] = COLOR_MAGENTA;
    objcolor[OTHRONE2] = COLOR_MAGENTA;
    objcolor[OLRS] = COLOR_GREEN;
    objcolor[OCOOKIE] = COLOR_YELLOW;
    objcolor[OWATER] = COLOR_BLUE;
    objcolor[OSHOREWATER] = COLOR_CYAN;
    objcolor[OLAVA] = COLOR_RED;
    objcolor[OINNERWALL] = COLOR_WHITE;
    objcolor[OCOOLEDLAVA] = COLOR_WHITE;
}

/*
* init_term()      Terminal initialization
*/
void
init_term(void)
{
    int i, fg;

    outbuf = malloc(BUFBIG + 16);
    if (!outbuf) {
        fprintf(stderr, "Error malloc'ing memory for decoded output buffer\n");
        died(-285);
    }

    initscr();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    if (use_color && has_colors()) {
        start_color();
        use_default_colors();
        init_colors();

        readcolors(); /* reads the larn.clr file */

        for (i = 0; i < 256; i++) {
            fg = i % 8;
            init_pair(i, fg, -1);
        }
    }
    else {
        /* otherwise monochrome */
        for (i = 0; i < MAXMONST + 9; i++)
            moncolor[i] = COLOR_WHITE;

        for (i = 0; i < MAXOBJECT + 1; i++)
            objcolor[i] = COLOR_WHITE;
    }

    curs_set(0);

    refresh();

#if defined PDC_KEY_MODIFIER_SHIFT
    PDC_save_key_modifiers(1);
#endif
}

/*
* cl_line(x,y)  Clear the whole line indicated by 'y' and leave cursor at [x,y]
*/
void
cl_line (int x, int y)
{
  move (y-1, 0);
  clrtoeol();
  cursor (x, y);
}

/*
* cl_up(x,y) Clear screen from [x,1] to current position. Leave cursor at [x,y]
*/
void
cl_up (int x, int y)
{
  int i;
  for (i = 1; i <= y; i++)
    {
        move (i-1,0);
        clrtoeol();
    }
  cursor (x, y);
}

/*
* cl_dn(x,y)   Clear screen from [1,y] to end of display. Leave cursor at [x,y]
*/
void
cl_dn (int x, int y)
{
  move (y-1,0);
  clrtobot();
  cursor (x, y);
}

/*
* lstandout(str)    Print the argument string in inverse video (standout mode).
*/
void
lstandout (char *str)
{
    attron(A_REVERSE);
    lprcat(str);
    attroff(A_REVERSE);
}

/*
* set_score_output()   Called when output should be literally printed.
*/
void
set_score_output (void)
{
	enable_scroll = -1;
}

/*
*  lflush()                        Flush the output buffer
*
*  Returns nothing of value.
*  for termcap version: Flush output in output buffer according to output
*                       status as indicated by `enable_scroll'
*/
void
lflush (void)
{
    int lpoint;
      if (lfd > 2)
	{
        if ((lpoint = lpnt - lpbuf) > 0) {
#ifdef EXTRA
        c[BYTESOUT] += lpoint;
#endif
            if (write(lfd, lpbuf, lpoint) != lpoint)
            {
              fprintf(stderr,"Error writing output file\n");
            }
          lpnt = lpbuf;		/* point back to beginning of buffer */
            }
	  flush_buf(); 

	  /* Catch write errors on save files
	   */
	  return;
	}
    else {
        refresh();
    }
}

/*
* flush_buf()          Flush buffer with decoded output.
*/
static void
flush_buf (void)
{
      if (lfd > 2)
	{
        ssize_t w = write(lfd, outbuf, io_index);
        if (w < 0)
        {
            /* flush failure is not a problem */
        }
	}
    io_index = 0;
}

void
enter_name (void)
{
    int i;
    char characternamestring;

  if (name_set)
  {
      /* Name already loaded from larnopts */
      return;
  }

  lprcat ("\n\nEnter character name:\n");

  sncbr ();

  i = 0;

  do
    {
      characternamestring = ttgetch ();

      if (characternamestring == '\n')
	break;
      if (characternamestring == 8)
	{
	  if (i > 0)
	    {
	      --i;
	      term_delch ();
	    }
	}
      else if (isprint (characternamestring))
	{
	  logname[i] = characternamestring;
	  ++i;
	}

    }
  while (i < LOGNAMESIZE - 1);

  logname[i] = '\0';

  scbr ();
}

void
cursor_block(void)
{
    curs_set(0);
    attron(A_REVERSE);
    addch(' ');
    attroff(A_REVERSE);
}

int
lstrcasecmp(const char* a, const char* b)
{
    unsigned char ca, cb;

    while (*a && *b) {
        ca = (unsigned char)tolower((unsigned char)*a);
        cb = (unsigned char)tolower((unsigned char)*b);

        if (ca != cb)
            return ca - cb;

        a++;
        b++;
    }

    return (unsigned char)*a - (unsigned char)*b;
}
