/* fortune.c */
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#if defined NIX
#include <fcntl.h>
#endif

#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/fortune.h"
#include "includes/global.h"
#include "includes/io.h"

static void msdosfortune (void);
static int fortsize = 0;

void
outfortune (void)
{

  lprcat ("\nThe cookie was delicious.");

  if (cdesc[BLINDCOUNT])
    {

      return;
    }

  msdosfortune ();
}



/* 
* Rumors has been entirely rewritten to be disk based.  This is marginally
* slower, but requires no mallocked memory.  Notice this in only valid for
* files smaller than 32K.
*/
static void
msdosfortune (void)
{
  int status, i;
  FILE *fp;
  char buf[BUFSIZ], ch;

  /* We couldn't open fortunes */
  if (fortsize < 0)
    {
      return;
    }

  fp = fopen (fortfile, "r");

  if (fp == 0)
    {

      /* Don't try opening it again */
      fortsize = -1;

      return;
    }

  if (fortsize == 0)
    {
      fseek (fp, 0L, SEEK_END);
      fortsize = ftell (fp);
    }

  if (fseek (fp, (long) rund (fortsize), SEEK_SET) < 0)
    {
      return;
    }

  /* 
   * Skip to next newline or EOF
   */
  do
    {
      status = fread (&ch, sizeof (char), 1, fp);
    }
  while (status != EOF && ch != '\n');

  if (status == EOF)
    {
      /* back to the beginning */
      if (fseek (fp, 0L, SEEK_SET) < 0)
	{
	  return;
	}
    }

  /* 
   * Read in the line.  
   */
  for (i = 0; i < BUFSIZ - 1; i++)
    {
      if (fread (&buf[i], sizeof (char), 1, fp) != 1 || buf[i] == '\n')
	{
	  break;
	}
    }
  buf[i] = '\0';

  /* 
   * And spit it out
   */
  lprcat ("  Inside you find a scrap of paper that says:\n");
  lprcat (buf);

  fclose (fp);
}
