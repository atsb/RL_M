#if defined(_WIN32) && !defined(WINDOWS_VS)
#include <ncursesw/curses.h>
#elif defined (WINDOWS_VS)
#include <curses.h>
#else
#include <curses.h>
#endif

void ansiterm_init (void);
void ansiterm_clean_up (void);

int ansiterm_getch (void);
int ansiterm_getche (void);
void ansiterm_delch (void);
