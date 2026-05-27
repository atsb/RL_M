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

#include <curses.h>

void setupvt100 (void);
void clearvt100 (void);
int ttgetch_noblock(void);
char ttgetch (void);
void scbr (void);
void sncbr (void);
void newgame (void);
void lprintf (const char *, ...);
void lprint (int);
void lprc (char);
void lwrite (char *, int);
char lgetc (void);
int larint (void);
void lrfill (char *, int);
char *lgetw (void);
char *lgetl (void);
int lcreat (char *);
int lopen (char *);
int lappend (char *);
void lrclose (void);
void lwclose (void);
void lprcat (const char *);
void cursor (int, int);
void cursors (void);
void init_term (void);
void cl_line (int, int);
void cl_up (int, int);
void cl_dn (int, int);
void lstandout (const char *);
void set_score_output (void);
void lflush (void);
char *tmcapcnv (char *, char *);
