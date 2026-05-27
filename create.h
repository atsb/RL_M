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

void makeplayer (void);
void newcavelevel (int);
void eat (int, int);
int fillmonst (int);
void expand_puddle(void);
void cool_lava(void);
void make_cryinglava(int cx, int cy);
void make_lavapool(int cx, int cy);
int lava_blocked(int x, int y);
