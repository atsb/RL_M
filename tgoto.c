/* My changes to atgoto based on public domain togo.c from libtermlib 
 *(taken from freelarn but licensed under Noah's License). -Gibbon
 */
#include <stdio.h>
#include <string.h>
#include "includes/tgoto.h"

static const char* in;		/* Internal copy of input string pointer */
static char* rp;			/* Pointer to ret array */
static int args[MAXARGS];	/* Maximum number of args to convert */
static int numval;			/* Count of args processed */
static char ret[24];		/* Converted string */

const char* atgoto(const char* cm, int destcol, int destline) {
	in = cm;
	rp = ret;
	args[0] = destline;
	args[1] = destcol;
	numval = 0;
	while (*in != '\0') {
		if (*in != '%') {
			*rp++ = *in++;
		}
		else {
			term_conversion();
		}
	}
	*rp = '\0';
	return (ret);
}

void term_conversion(void) {
	int incr = 0;
	int temp;
	in++;
	switch (*in++) {
	case 'd':
		sprintf(rp, "%d", args[numval++]);
		numval = (numval + incr) % 1000;
		rp = &ret[strlen(ret)];
		if (numval > 99) {
			*rp++ = '0' + (numval / 100);
		}
		if (numval > 9) {
			*rp++ = '0' + (numval / 10) % 10;
		}
		*rp++ = '0' + (numval % 10);
		break;
	case '2':
		sprintf(rp, "%02d", args[numval++]);
		numval = (numval + incr) % 100;
		rp += 2;
		break;
	case '+':
		*rp++ = args[numval++] + *in++;
		break;
	case 'r':
		temp = args[numval];
		args[numval] = args[numval + 1];
		args[numval + 1] = temp;
		break;
	case 'i':
		args[numval]++;
		args[numval + 1]++;
		break;
	case '%':
		*rp++ = '%';
		break;
	}
}
