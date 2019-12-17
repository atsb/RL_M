/************************************************************************
 *                                  *
 *          Copyright (c) 1982, Fred Fish           *
 *              All Rights Reserved             *
 *                                  *
 *  This software and/or documentation is released for public   *
 *  distribution for personal, non-commercial use only.     *
 *  Limited rights to use, modify, and redistribute are hereby  *
 *  granted for non-commercial purposes, provided that all      *
 *  copyright notices remain intact and all changes are clearly *
 *  documented.  The author makes no warranty of any kind with  *
 *  respect to this product and explicitly disclaims any implied    *
 *  warranties of merchantability or fitness for any particular *
 *  purpose.                            *
 *                                  *
 ************************************************************************
 */

/*
 *  LIBRARY FUNCTION
 *
 *  tgoto   expand cursor addressing string from cm capability
 *
 *  KEY WORDS
 *
 *  termcap
 *
 *  SYNOPSIS
 *
 *  char *tgoto(cm,destcol,destline)
 *  char *cm;
 *  int destcol;
 *  int destline;
 *
 *  DESCRIPTION
 *
 *  Returns cursor addressing string, decoded from the cm
 *  capability string, to move cursor to column destcol on
 *  line destline.
 *
 *  The following sequences uses one input argument, either
 *  line or column, and place the appropriate substitution
 *  in the output string:
 *
 *      %d  substitute decimal value (in ASCII)
 *      %2  like %d but forces field width to 2
 *      %3  like %d but forces field width to 3
 *      %.  like %c
 *      %+x like %c but adds ASCII value of x
 *
 *  The following sequences cause processing modifications
 *  but do not "use up" one of the arguments.  If they
 *  act on an argument they act on the next one to
 *  be converted.
 *
 *      %>xy    if next value to be converted is
 *          greater than value of ASCII char x
 *          then add value of ASCII char y.
 *      %r  reverse substitution of line
 *          and column (line is substituted
 *          first by default).
 *      %i  causes input values destcol and
 *          destline to be incremented.
 *      %%  gives single % character in output.
 *
 *  BUGS
 *
 *  Does not implement some of the more arcane sequences for
 *  radically weird terminals (specifically %n, %B, & %D).
 *  If you have one of these you deserve whatever happens.
 *
 */

/*
 *  Miscellaneous stuff
 */

#include <stdio.h>
#include <string.h>
#include "includes/tgoto.h"

#define MAXARGS 2

static void process (void);


static const char *in;		/* Internal copy of input string pointer */
static char *out;		/* Pointer to output array */
static int args[MAXARGS];	/* Maximum number of args to convert */
static int pcount;		/* Count of args processed */
static char output[64];		/* Converted string */

/*
 *  PSEUDO CODE
 *
 *  Begin tgoto
 *      If no string to process then
 *      Return pointer to error string.
 *      Else
 *      Initialize pointer to input string.
 *      Initialize pointer to result string.
 *      First arg is line number by default.
 *      Second arg is col number by default.
 *      No arguments processed yet.
 *      While there is another character to process
 *          If character is a not a % character then
 *          Simply copy to output.
 *          Else
 *          Process the control sequence.
 *          End if
 *      End while
 *      TERMINATE STRING!  (rde)
 *      Return pointer to static output string.
 *      End if
 *  End tgoto
 *
 */

const char *
atgoto (const char *cm, int destcol, int destline)
{
  if (cm == NULL)
    {
      return ("OOPS");
    }
  else
    {
      in = cm;
      out = output;
      args[0] = destline;
      args[1] = destcol;
      pcount = 0;
      while (*in != '\0')
	{
	  if (*in != '%')
	    {
	      *out++ = *in++;
	    }
	  else
	    {
	      process ();
	    }
	}
      *out = '\0';		/* rde 18-DEC-86: don't assume out was all zeros */
      return (output);
    }
}

/*
 *  INTERNAL FUNCTION
 *
 *  process   process the conversion/command sequence
 *
 *  SYNOPSIS
 *
 *  static process()
 *
 *  DESCRIPTION
 *
 *  Processes the sequence beginning with the % character.
 *  Directly manipulates the input string pointer, the
 *  output string pointer, and the arguments.  Leaves
 *  the input string pointer pointing to the next character
 *  to be processed, and the output string pointer pointing
 *  to the next output location.  If conversion of
 *  one of the numeric arguments occurs, then the pcount
 *  is incremented.
 *
 */

/*
 *  PSEUDO CODE
 *
 *  Begin process
 *      Skip over the % character.
 *      Switch on next character after %
 *      Case 'd':
 *      Process %d type conversion (variable width).
 *      Reinitialize output pointer.
 *      Break;
 *      Case '2':
 *      Process %d type conversion (width 2).
 *      Reinitialize output pointer.
 *      Break;
 *      Case '3':
 *      Process %d type conversion (width 3).
 *      Reinitialize output pointer.
 *      Break;
 *      Case '.'
 *      Process %c type conversion.
 *      Break;
 *      Case '+':
 *      Process %c type conversion with offset.
 *      Break;
 *      Case '>':
 *      Process argument modification.
 *      Break;
 *      Case 'r':
 *      Process argument reversal.
 *      Break;
 *      Case 'i':
 *      Increment argument values.
 *      Break;
 *      Case '%':
 *      Copy to output, incrementing pointers.
 *      Break;
 *      End switch
 *  End process
 *
 */

static void
process (void)
{
  int temp;

  in++;
  switch (*in++)
    {
    case 'd':
      sprintf (out, "%d", args[pcount++]);
      out = &output[strlen (output)];
      break;
    case '2':
      sprintf (out, "%02d", args[pcount++]);
      out += 2;
/*
    out = &output[strlen(output)];
*/
      break;
    case '3':
      sprintf (out, "%03d", args[pcount++]);
      out = &output[strlen (output)];
      break;
    case '.':
      *out++ = (char) args[pcount++];
      break;
    case '+':
      *out++ = (char) args[pcount++] + *in++;
      break;
    case '>':
      if (args[pcount] > *in++)
	{
	  args[pcount] += *in++;
	}
      else
	{
	  in++;
	}
      break;
    case 'r':
      temp = args[pcount];
      args[pcount] = args[pcount + 1];
      args[pcount + 1] = temp;
      break;
    case 'i':
      args[pcount]++;
      args[pcount + 1]++;
      break;
    case '%':
      *out++ = '%';
      break;
    }
}
