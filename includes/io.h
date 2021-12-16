void setupvt100 (void);
void clearvt100 (void);

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

void lprcat (char *);

void cursor (int, int);

void cursors (void);

void init_term (void);

void cl_line (int, int);

void cl_up (int, int);

void cl_dn (int, int);

void lstandout (char *);

void set_score_output (void);

void lflush (void);

void lflushall (void);

char *tmcapcnv (char *, char *);

void enter_name (void);
