/* tok.c */
/*
yylex()
sethard()
*/
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"
#include "tok.h"
#include "display.h"
#include "io.h"
#include "scores.h"
#include "nap.h"
#include "diag.h"
#include "main.h"

#define CHKPTINT   400

static char lastok = 0;
int yrepcount = 0, dayplay = 0;
#define MAXUM 52 /* maximum number of user re-named monsters */
#define MAXMNAME 40 /* max length of a monster re-name */
static char usermonster[MAXUM][MAXMNAME]; /* the user named monster name goes here */
static int usermpoint = 0; /* the user monster pointer */

void
readopts(void)
{
    FILE* fp = fopen(optsfile, "r");
    char line[256];
    char* p;

    name_set = 0;

    if (!fp) {
        strcpy(logname, loginname);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {

        /* strip leading whitespace */
        p = line;
        while (*p == ' ' || *p == '\t')
            p++;

        /* skip empty lines and comments */
        if (*p == '\0' || *p == '\n' || *p == '#')
            continue;

        /* name: */
        if (strncmp(p, "name", 4) == 0) {
            char* value = strchr(p, ':');
            if (value)
                value++;
            else
                value = p + 4;

            while (*value == ' ' || *value == '\t')
                value++;

            value[strcspn(value, "\r\n")] = '\0';

            if (*value) {
                strncpy(logname, value, LOGNAMESIZE - 1);
                logname[LOGNAMESIZE - 1] = '\0';
                name_set = 1;
            }
            continue;
        }

        /* savefile: */
        if (strncmp(p, "savefile:", 9) == 0) {
            char* value = p + 9;

            while (*value == ' ' || *value == '\t')
                value++;

            value[strcspn(value, "\r\n")] = '\0';

            if (*value) {
                strncpy(savefilename, value, SAVEFILENAMESIZE - 1);
                savefilename[SAVEFILENAMESIZE - 1] = '\0';
            }
            continue;
        }

        /* checkpoint: */
        if (strncmp(p, "checkpoint:", 11) == 0) {
            char* value = p + 11;

            while (*value == ' ' || *value == '\t')
                value++;

            value[strcspn(value, "\r\n")] = '\0';

            if (*value) {
                strncpy(ckpfile, value, SAVEFILENAMESIZE - 1);
                ckpfile[SAVEFILENAMESIZE - 1] = '\0';
            }
            continue;
        }

        /* monster: "name" */
        if (strncmp(p, "monster:", 8) == 0) {
            char* value = p + 8;

            while (*value == ' ' || *value == '\t')
                value++;

            value[strcspn(value, "\r\n")] = '\0';

            if (*value && usermpoint < MAXUM) {
                if (strlen(value) >= MAXMNAME)
                    value[MAXMNAME - 1] = '\0';

                strcpy(usermonster[usermpoint], value);

                /* match first letter to monster list */
                {
                    int j = usermonster[usermpoint][0];
                    int k;

                    if (isalpha(j)) {
                        for (k = 1; k < MAXMONST + 8; k++) {
                            if (monstnamelist[k] == j) {
                                monster[k].name = &usermonster[usermpoint][0];
                                break;
                            }
                        }
                    }
                }

                usermpoint++;
            }
            continue;
        }

        /* color: on/off */
        if (strncmp(p, "color:", 6) == 0) {
            char* value = p + 6;

            while (*value == ' ' || *value == '\t')
                value++;

            value[strcspn(value, "\r\n")] = '\0';

            if (!lstrcasecmp(value, "on"))
            {
                use_color = 1;
            }
            else
            {
                use_color = 0;
            }
            continue;
        }

        if (strncmp(p, "enable-checkpointing", 20) == 0) {
            ckpflag = 1;
            continue;
        }

        if (strncmp(p, "male", 4) == 0) {
            sex = 1;
            continue;
        }
        if (strncmp(p, "female", 6) == 0) {
            sex = 0;
            continue;
        }

        if (strncmp(p, "no-introduction", 15) == 0) {
            nowelcome = 1;
            continue;
        }

        if (strncmp(p, "play-day-play", 13) == 0) {
            dayplay = 1;
            continue;
        }
    }

    fclose(fp);

    if (!name_set)
        strcpy(logname, loginname);
}

/*
* lexical analyzer for larn
*/
int
yylex (void)
{
  char cc;
  char firsttime = TRUE;

  if (hit2flag)
    {
      hit2flag = 0;
      yrepcount = 0;
      return (' ');
    }
  if (yrepcount > 0)
    {
      --yrepcount;
      return (lastok);
    }
  else
    yrepcount = 0;
  if (yrepcount == 0)
    {
      bottomline();
      showplayer ();		/* show where the player is */
    }

  lflush ();
  for (;;)
    {
      c[BYTESIN]++;

      /* periodic checkpointing */
      if (ckpflag && cc != 0) {
          if ((c[BYTESIN] % 400) == 0) {
              savegame(ckpfile);
          }

          if (dayplay == 0) {
              if (playable()) {
                  cursor(1, 19);
                  lprcat("\nSorry, but it is now time for work. Your game has been saved.\n");
                  lflush();
                  savegame(savefilename);
                  wizard = 1;
                  nomove = 1;
                  nap(4000);
                  died(-257);
              }
          }
      }

      {
          int ch = ttgetch_noblock();

          if (ch == -1) {

              /* no input available */
              return 0;
          }
          cc = (char)ch;
      }

      /* get repeat count, showing to player
       */
      if ((cc <= '9') && (cc >= '0'))
	{
	  yrepcount = yrepcount * 10 + cc - '0';

	  /* show count to player for feedback
	   */
	  if (yrepcount >= 10)
	    {
	      cursors ();
	      if (firsttime)
		lprcat ("\n");
	      lprintf ("count: %d", (int) yrepcount);
	      firsttime = FALSE;
	      lflush ();	/* show count */
	    }
	}
      else
	{
	  if (yrepcount > 0)
	    --yrepcount;
	  return (lastok = cc);
	}
    }
}





/*
function to set the desired hardness
enter with hard= -1 for default hardness, else any desired hardness
*/
void
sethard (int hard)
{
  int j, k;
  int i;
  struct monst *mp;

  j = c[HARDGAME];
  hashewon ();

  /* don't set c[HARDGAME] if restoring game */
  if (restorflag == 0)
    {

      if (hard >= 0)
	{

	  c[HARDGAME] = hard;
	}

    }
  else
    {

      /* set c[HARDGAME] to proper value if restoring game */
      c[HARDGAME] = j;
    }

  k = c[HARDGAME];

  if (k == 0)
    {

      return;
    }

  for (j = 0; j <= MAXMONST + 8; j++)
    {

      mp = &monster[j];

      i = ((6 + k) * mp->hitpoints + 1) / 6;
      mp->hitpoints = (i < 0) ? 32767 : i;

      i = ((6 + k) * mp->damage + 1) / 5;
      mp->damage = (i > 127) ? 127 : i;

      i = (10 * mp->gold) / (10 + k);
      mp->gold = (i > 32767) ? 32767 : i;

      i = mp->armorclass - k;
      mp->armorclass = (i < -127) ? -127 : i;

      i = (7 * mp->experience) / (7 + k) + 1;
      mp->experience = (i <= 0) ? 1 : i;
    }
}
