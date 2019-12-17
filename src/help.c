/* help.c */
#include <stdio.h>
#include <stdlib.h>
#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/ansiterm.h"
#include "includes/display.h"
#include "includes/help.h"
#include "includes/io.h"
#include "includes/sysdep.h"

/* I coded a nice line-reader for this.  Unfortunately it's very hard to get a good
 *  cursor and text position when reading a file from inside a curses program.
 *  so I did this instead, I actually prefer it. -Gibbon 
 
 */

int
display_help_text(void)
{

	screen_clear();
	lstandout("Help File for Larn\n");
	lprcat("b  move southwest          B  run southwest            A  desecrate an altar\n");
	lprcat("c  cast a spell            C  close a door             Z  teleport yourself	\n");
	lprcat("d  drop an item            D  drink at a fountain      <  go up stairs or	\n");
	lprcat("e  eat something           E  enter a store, dungeon      volcanic shaft	\n");
	lprcat("g  get present pack weight                             >  go down stairs or	\n");
	lprcat("h  move left               H  run left                    volcanic shaft	\n");
	lprcat("i  inventory your pockets  I  list all known items							\n");
	lprcat("j  move down               J  run down                 ^  identify a trap	\n");
	lprcat("k  move up                 K  run up                   ,  pick up item		\n");
	lprcat("l  move right              L  run right                :  look at object you\n");
	lprcat("n  move southeast          N  run southeast               are standing on	\n");
	lprcat("                           O  open a door or chest     .  stay here			\n");
	lprcat("p  pray at an altar        P  give tax status          m  move without picking\n");
	lprcat("q  quaff a potion          Q  quit the game (suicide)     up an object		\n");
	lprcat("r  read a scroll           R  remove gems from throne  /  identify objects in\n");
	lprcat("s  sit on a throne         S  save the game               the game			\n");
	lprcat("t  tidy up at a fountain   T  take off armor  								\n");
	lprcat("u  move northeast          U  run northeast									\n");
	lprcat("v  print program version													\n");
	lprcat("w  wield a weapon          W  wear armor 									\n");
	lprcat("y  move northwest          Y  run northwest            ^L redraw the screen	\n");
	retcont();
	screen_clear();
	lstandout("Special Notes\n");
	lprcat("When dropping gold, if you type '*' as your amount, all your gold gets dropped.\n");
	lprcat("In general, typing in '*' means all of what your interested in.  This is true\n");
	lprcat("when visiting the bank, or when contributing at altars.\n\n");
	lprcat("When in the store, trading post, school, or home, an <escape> will get you out.\n\n");
	lprcat("When casting a spell, if you need a list of spells you can cast, type 'I' as\n");
	lprcat("the first letter of your spell.  The available list of spells will be shown,\n");
	lprcat("after which you may enter the spell code.  This only works on the 1st letter\n");
	lprcat("of the spell you are casting.\n\n");
	lprcat("When an inventory list is on the screen from a drop, quaff, read, or similar\n");
	lprcat("command, you can type the letter of the object that you wish to act apon,\n");
	lprcat("without having to type a space to get back to the prompt.\n\n");
	lprcat("If NumLock is off, the Keypad functions in the obvious way for movement. Hold\n");
	lprcat("Shift when pressing any direction on the Keypad to run in that direction. The\n");
	lprcat("5 key on the Keypad is the same as \"stay here\", which really means to\n");
	lprcat("skip your turn.");
	retcont();
	screen_clear();
	lstandout("Explanation of the Larn scoreboard facility\n");
	lprcat("Larn supports TWO scoreboards, one for winners, and one for deceased\n");
	lprcat("characters.  Each player (by the name entered when you start the game)\n");
	lprcat("is allowed one slot on each scoreboard, if the score is in the top ten for\n");
	lprcat("that scoreboard.  This design helps insure that frequent players of Larn\n");
	lprcat("do not hog the scoreboard, and gives more players a chance for glory.\n\n");
	lprcat("Level of difficulty is also noted on the scoreboards, and this takes precedence\n");
	lprcat("over score for determining what entry is on the scoreboard.  For example:\n");
	lprcat("if \"Yar, the Bug Slayer\" has a score of 128003 on the scoreboard at diff 0,\n");
	lprcat("then a game at diff 1 and a score of 4112 would replace the previous\n");
	lprcat("entry on the scoreboard.  Note that when a player dies, the inventory is\n");
	lprcat("stored in the scoreboard so that everyone can see what items the player had\n");
	lprcat("at the time of death.");
	return_to_game();
	drawscreen();	
	setscroll(); 
	return(0);
}

/*
 * function to display the welcome message and background
 */
void
welcome (void)
{
	lprcat("Welcome to the game of Larn.\n\n  At this moment, you face a great problem.");
	lprcat("\nYour daughter has contracted a strange disease, and none of your");
	lprcat("\nhome remedies seem to have any effect. You sense that she is in");
	lprcat("\nmortal danger, and you must try to save her.");
	lprcat("\n\n  Time ago you heard of a land of great danger and opportunity.");
	lprcat("\nPerhaps here is the solution you need.");
	lprcat("\n\n  It has been said that there once was a great magician who called himself");
	lprcat("\nPolinneaus.  Many years ago after having many miraculous successes,");
	lprcat("\nPolinneaus retired to the caverns of Larn, where he devoted most of his");
	lprcat("\ntime to the creation of magic. Rumors have it that one day Polinneaus set");
	lprcat("\nout to dispel an attacking army in a forest some distance to the north.");
	lprcat("\nIt is believed that here he met his demise. The caverns of Larn it is");
	lprcat("\nthought, must be magnificent in design and contain much magic and treasure.");
	lprcat("\nOne option you have is to undertake a journey into these caverns in hope");
	lprcat("\nof finding a cure.");
	lprcat("\n");
	lprcat("\n\nGood Luck!  You're going to need it!");
	
	/* press return to continue */
	retcont();
}

/*
 * function to say press return to continue and reset scroll when done
 */
void
retcont(void)
{
  cursor(1, 24);
  lprcat("Press ");
  lstandout("return");
  lprcat(" to continue: ");
  while (ttgetch() != '\n');
  setscroll ();
}

/* Added for places where it points you back to the game. -Gibbon */
void
return_to_game(void)
{
  cursor(1, 24);
  lprcat("Press ");
  lstandout("return");
  lprcat(" to go back to the game: ");
  while (ttgetch() != '\n');
  setscroll();
}
