/* help.c */
#ifdef _WIN32
#include <Windows.h>
#endif
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"
#include "display.h"
#include "help.h"
#include "io.h"
#include "nap.h"

/* I coded a nice line-reader for this.  Unfortunately it's very hard to get a good
 *  cursor and text position when reading a file from inside a curses program.
 *  so I did this instead, I actually prefer it. -Gibbon
 * 
 * 2026.05.13: So I decided to add back how it originally was, but converted the file to ANSI C and added a new 
 * function to interpret ANSI escapes.  This allows the help file to be formatted with 
 * ANSI codes, and for it to be displayed properly in the curses interface. -Gibbon
 */

 /* Interpret ANSI escapes and print using curses */
static void
ansihelp(const char *s)
{
    const char *p;
    char params[32];
    char buf[32];
    char *tok;
    int pi;
    char cmd;

    p = s;

    while (*p) {
        if (*p == '\033') {      /* ESC */
            p++;

            if (*p == '[') {     /* CSI */
                p++;

                pi = 0;
                while (*p && pi < 30 &&
                       ((*p >= '0' && *p <= '9') || *p == ';'))
                {
                    params[pi++] = *p++;
                }
                params[pi] = '\0';

                cmd = *p++;

                if (cmd == 'm') {
                    strcpy(buf, params);
                    tok = strtok(buf, ";");

                    while (tok) {
                        switch (atoi(tok)) {
                        case 0:
                            standend();
                            break;
                        case 1:
                            attron(A_BOLD);
                            break;
                        case 4:
                            attron(A_UNDERLINE);
                            break;
                        case 7:
                            standout();
                            break;
                        default:
                            break;
                        }
                        tok = strtok(NULL, ";");
                    }
                }
                continue;
            }
            continue;
        }

        lprc(*p++);
    }
}

/*
* help function to display the help info
*
* format of the larn.help file
*
* 1st character of file: # of pages of help available (ascii digit)
* page (23 lines) for the introductory message (not counted in above)
* pages of help text (23 lines per page)
*/
int
help(void)
{
    FILE *fp;
    char line[512];
    int pages;
    int i;
    int getchkey;

    pages = openhelp(&fp);
    if (pages < 0)
        return 0;

    for (i = 0; i < 23; i++)
        (void)fgets(line, sizeof(line), fp);

    while (pages > 0)
    {
        screen_clear();
        cursor(1, 1);

        for (i = 0; i < 23; i++) {
            if (!fgets(line, sizeof(line), fp))
                break;

            line[strcspn(line, "\r\n")] = '\0';
            ansihelp(line);
            standend();
            attrset(A_NORMAL);
            lprc('\n');
        }

        if (pages > 1) {
            lprcat(" ---- Press ");
            lstandout("return");
            lprcat(" to exit, ");
            lstandout("space");
            lprcat(" for more help ---- ");

            getchkey = ttgetch();
            while (getchkey != ' ' && getchkey != '\n' && getchkey != '\033')
                getchkey = ttgetch();

            if (getchkey == '\n' || getchkey == '\033') {
                fclose(fp);
                setscroll();
                drawscreen();
                return 0;
            }
        }

        pages--;
    }

    fclose(fp);
    retcont();
    drawscreen();
    return 0;
}

/*
* function to display the welcome message and background
*/
int
welcome(void)
{
    FILE *fp;
    char line[512];
    int pages;
    int i;

    pages = openhelp(&fp);
    if (pages < 0)
        return 0;

    screen_clear();
    cursor(1, 1);

    for (i = 0; i < 23; i++) {
        if (!fgets(line, sizeof(line), fp))
            break;

        line[strcspn(line, "\r\n")] = '\0';
        ansihelp(line);
        standend();
        attrset(A_NORMAL);
        lprc('\n');
    }

    fclose(fp);
    retcont();
    return 0;
}

/*
* function to say press return to continue and reset scroll when done
*/
int
retcont(void)
{
    cursor(1, 24);
    lprcat("Press ");
    lstandout("return");
    lprcat(" to continue: ");

    while (ttgetch() != '\n')
        ;

    setscroll();
    return 0;
}

/*
* routine to open the help file and return the first character - '0'
*/
int
openhelp(FILE** fp_out)
{
    FILE* fp = fopen("larn.help", "r");
    int pages;

    if (!fp) {
        cursors();
        lprintf("Can't open help file \"%s\"\n", "larn.help");
        lflush();
#ifdef _WIN32
        Sleep(4000);
#else
        sleep(4);
#endif
        drawscreen();
        setscroll();
        return -1;
    }

    /* read first character # of pages */
    pages = fgetc(fp);
    if (pages == EOF) {
        fclose(fp);
        return -1;
    }

    /* consume newline after the number */
    fgetc(fp);

    resetscroll();
    *fp_out = fp;
    return pages - '0';
}
