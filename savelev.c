/* savelev.c */
#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/savelev.h"




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

  /* pointer to this level's cells */
  pcel = &cell[level * MAXX * MAXY];

  /* pointer to past end of this level's cells */
  pecel = pcel + MAXX * MAXY;

  pitem = item[0];
  piarg = iarg[0];
  pknow = know[0];
  pmitem = mitem[0];
  phitp = hitp[0];

  while (pcel < pecel)
    {

      pcel->mitem = *pmitem++;
      pcel->hitp = *phitp++;
      pcel->item = *pitem++;
      pcel->know = *pknow++;
      pcel->iarg = *piarg++;
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

  /* pointer to this level's cells */
  pcel = &cell[level * MAXX * MAXY];

  /* pointer to past end of this level's cells */
  pecel = pcel + MAXX * MAXY;

  pitem = item[0];
  piarg = iarg[0];
  pknow = know[0];
  pmitem = mitem[0];
  phitp = hitp[0];

  while (pcel < pecel)
    {

      *pmitem++ = pcel->mitem;
      *phitp++ = pcel->hitp;
      *pitem++ = pcel->item;
      *pknow++ = pcel->know;
      *piarg++ = pcel->iarg;
      pcel++;
    }
}
