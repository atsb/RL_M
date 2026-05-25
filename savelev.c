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

#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"
#include "savelev.h"

/*
 *  routine to save the present level into storage
 */
void
savelevel (void)
{
  struct cel *pcel;
  int *pitem, *pknow, *pmitem;
  int *phitp, *piarg;
  struct cel *pecel;
  unsigned char *perosion;
  unsigned char *plavaheat;

  /* pointer to this level's cells */
  pcel = &cell[level * MAXX * MAXY];

  /* pointer to past end of this level's cells */
  pecel = pcel + MAXX * MAXY;

  pitem = item[0];
  piarg = iarg[0];
  pknow = know[0];
  pmitem = mitem[0];
  phitp = hitp[0];
  perosion = erosion[0];
  plavaheat = lavaheat[0];

  while (pcel < pecel)
    {

      pcel->mitem    = *pmitem++;
      pcel->hitp     = *phitp++;
      pcel->item     = *pitem++;
      pcel->know     = *pknow++;
      pcel->iarg     = *piarg++;
      pcel->erosion  = *perosion++;
      pcel->lavaheat = *plavaheat++;
      pcel++;
    }
}

/*
 * routine to restore a level from storage
 */
void
getlevel (void)
{
  struct cel *pcel;
  int *pitem;
  int *pknow;
  int *pmitem;
  int *phitp, *piarg;
  struct cel *pecel;
  unsigned char *perosion;
  unsigned char *plavaheat;

  /* pointer to this level's cells */
  pcel = &cell[level * MAXX * MAXY];

  /* pointer to past end of this level's cells */
  pecel = pcel + MAXX * MAXY;

  pitem     = item[0];
  piarg     = iarg[0];
  pknow     = know[0];
  pmitem    = mitem[0];
  phitp     = hitp[0];
  perosion  = erosion[0];
  plavaheat = lavaheat[0];

  while (pcel < pecel)
    {

      *pmitem++    = pcel->mitem;
      *phitp++     = pcel->hitp;
      *pitem++     = pcel->item;
      *pknow++     = pcel->know;
      *piarg++     = pcel->iarg;
      *perosion++  = pcel->erosion;
      *plavaheat++ = pcel->lavaheat;
      pcel++;
    }
}
