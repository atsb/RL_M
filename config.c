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

 /*
 *  config.c    --  This defines the installation dependent variables.
 *                  Some strings are modified later.  ANSI C would
 *                  allow compile time string concatenation, we must
 *                  do runtime concatenation, in main.
 */
#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"

#ifndef WIZID
#define WIZID   0
#endif

/*
 *  All these strings will be appended to in main() to be complete filenames
 */


/* Make LARNHOME readable from the larnopt file into a lardir variable.
 */
char savefilename[PATHLEN];
char scorefile[PATHLEN];
char logfile[PATHLEN];
char mazefile[PATHLEN];
char fortfile[PATHLEN];
char playerids[PATHLEN];
char diagfile[PATHLEN];		/* the diagnostic filename  */
char holifile[PATHLEN];
char optsfile[PATHLEN];
char ckpfile[PATHLEN];
