#include <curses.h>

void ansiterm_init (void);
void ansiterm_clean_up (void);

int ansiterm_getch (void);
int ansiterm_getche (void);
void ansiterm_delch (void);
