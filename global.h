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

void raiselevel (void);

void loselevel (void);

void raiseexperience (unsigned long);

void loseexperience (long);

void losehp (int);

void losemhp (int);

void raisehp (int);

void raisemhp (int);

void raisemspells (int);

void losemspells (int);

int makemonst (int);

void positionplayer (void);

void recalc (void);

void quit (void);

int more (char);

void enchantarmor (void);

void enchweapon (void);

int nearbymonst (void);

int stealsomething (void);

int emptyhanded (void);

void creategem (void);

void adjustcvalues (int, int);

int getpassword (void);

char getyn (void);

int packweight (void);

extern int water_anim_toggle;
extern time_t last_water_anim;
extern time_t last_lava_anim;
extern int lava_anim_toggle;
