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

#include <stdlib.h>
#include <string.h>
#include "action.h"
#include "larn.h"
#include "create.h"
#include "display.h"
#include "global.h"
#include "io.h"
#include "monster.h"
#include "larncons.h"
#include "larndata.h"
#include "larnfunc.h"

#define nlprc(_ch) lprc(_ch)

static void seepage(void);
static int minx, maxx, miny, maxy;
static int is_metal_armor(int);
static int d_xmin = 0, d_xmax = MAXX, d_ymin = 0, d_ymax = MAXY;	/* for limited screen drawing */
static int d_flag;
static int lincount, count;

int diroffx[] = { 0, 0, 1, 0, -1, 1, -1, 1, -1 };
int diroffy[] = { 0, 1, 0, -1, 0, -1, -1, 1, 1 };

int compare_color_better = COLOR_GREEN;
chtype compare_attr_better  = 0;

int compare_color_worse  = COLOR_RED;
chtype compare_attr_worse   = 0;

int compare_color_equal  = COLOR_CYAN;
chtype compare_attr_equal   = 0;

int compare_color_none = COLOR_YELLOW;
chtype compare_attr_none  = 0;

int player_color = COLOR_WHITE;
chtype player_attr = 0;

/* The entire bot_xx rendering has been rewritten
* it was a legacy of the 80's and assumptions about terminal
* windows and saving CPU cycles (updating incrementally).
* It was broken, messy, and was unmaintainable.
* Now it makes sense, can be extended and just updates the bottom line when needed.
* 
* Nobody else has ever done this.  You're welcome Larners! ~Gibbon
*/

/*
bottomline()

now for the bottom line of the display
*/
void
bottomline(void)
{
    recalc();
    statusmessage_draw_lines();
    statusmessage_draw_right_panel();
}

void
bottomhp(void)
{
    bottomline();
}

void
bottomspell(void)
{
    bottomline();
}

/*
special subroutine to update only the gold number on the bottomlines
called from ogold()
*/
void
bottomgold(void)
{
    bottomline();
}

/*
common subroutine for a more economical bottomline()
*/
struct bot_side_def
{

    int typ;
    const char* string;

};

static struct bot_side_def bot_data[] = {
  {STEALTH, "stealth"},
  {UNDEADPRO, "undead pro"},
  {SPIRITPRO, "spirit pro"},
  {CHARMCOUNT, "Charm"},
  {TIMESTOP, "Time Stop"},
  {HOLDMONST, "Hold Monst"},
  {GIANTSTR, "Giant Str"},
  {FIRERESISTANCE, "Fire Resit"},
  {DEXCOUNT, "Dexterity"},
  {STRCOUNT, "Strength"},
  {SCAREMONST, "Scare"},
  {HASTESELF, "Haste Self"},
  {CANCELLATION, "Cancel"},
  {INVISIBILITY, "Invisible"},
  {ALTPRO, "Protect 3"},
  {PROTECTIONTIME, "Protect 2"},
  {WTW, "Wall-Walk"}
};

/*
*  subroutine to draw only a section of the screen
*  only the top section of the screen is updated.  If entire lines are being
*  drawn, then they will be cleared first.
*/
void
draws(int xmin, int xmax, int ymin, int ymax)
{
    int i, idx;

    if (xmin == 0 && xmax == MAXX)	/* clear section of screen as needed */
    {
        if (ymin == 0)
            cl_up(79, ymax);
        else
            for (i = ymin; i < ymin; i++)
                cl_line(1, i + 1);
        xmin = -1;
    }
    d_xmin = xmin;
    d_xmax = xmax;
    d_ymin = ymin;
    d_ymax = ymax;		/* for limited screen drawing */
    drawscreen();
    if (xmin <= 0 && xmax == MAXX)	/* draw stuff on right side of screen as needed */
    {
        for (i = ymin; i < ymax; i++)
        {
            idx = bot_data[i].typ;
            if (c[idx])
            {
                cursor(70, i + 1);
                lprcat(bot_data[i].string);
            }
            cbak[idx] = c[idx];
        }
    }
}

/*
drawscreen()

subroutine to redraw the whole screen as the player knows it
*/
void
drawscreen(void)
{
    int i, j, ileft, iright;

    if (d_xmin == 0 && d_xmax == MAXX && d_ymin == 0 && d_ymax == MAXY)
    {
        /* clear the screen */
        d_flag = 1;
        screen_clear();
    }
    else
    {
        d_flag = 0;
        cursor(1, 1);
    }

    if (d_xmin < 0)
    {
        /* d_xmin=-1 means display all without bottomline */
        d_xmin = 0;
    }

    /* display lines of the screen */
    for (j = d_ymin; j < d_ymax; j++)
    {
        ileft = d_xmin - 1;

        while (++ileft < d_xmax)
        {
            if (know[ileft][j])
            {
                break;
            }
        }

        /* if blank line ... */
        if (ileft >= d_xmax)
        {
            continue;
        }

        /* scan from right of line until we reach a location that the
            user knows.
        */
        iright = d_xmax;

        while (--iright > ileft)
        {
            if (know[iright][j])
            {
                break;
            }
        }

        /*
        * now print the line, after positioning the cursor.
        * print the line with bold objects in a different
        * loop for effeciency
        */
        cursor(ileft + 1, j + 1);

        for (i = ileft; i <= iright; i++)
        {
            /* we still need to check for the location being known,
                for we might have an unknown spot in the middle of
                an otherwise known line.
            */
            if (know[i][j] == 0)
            {
                nlprc(' ');
            }
            else if (know[i][j] & HAVESEEN)
            {
                /*
                * if monster there and the user still knows the place,
                * then show the monster.  Otherwise, show what was
                * there before.
                */
                if (i == playerx && j == playery)
                {
                    nlprc(' ');
                    continue;
                }

                show1cell(i, j);
            }
            else
            {
                /*
                * error condition.  recover by resetting location
                * to an 'unknown' state.
                */
                nlprc(' ');
                mitem[i][j] = item[i][j] = 0;
            }
        }
    }

    resetbold();

    if (d_flag)
    {
        statusmessage_draw_right_panel();
        statusmessage_draw_lines();
    }

    /* for limited screen drawing */
    d_xmin = d_ymin = 0;
    d_xmax = MAXX;
    d_ymax = MAXY;
}

static void
isbetter(void)
{
    if (has_colors())
    {
        attron(COLOR_PAIR(compare_color_better));
        attron(compare_attr_better);
    }

    lprcat("(better) ");

    if (has_colors())
    {
        attroff(compare_attr_better);
        attroff(COLOR_PAIR(compare_color_better));
    }
}

static void
isworse(void)
{
    if (has_colors())
    {
        attron(COLOR_PAIR(compare_color_worse));
        attron(compare_attr_worse);
    }

    lprcat("(worse) ");

    if (has_colors())
    {
        attroff(compare_attr_worse);
        attroff(COLOR_PAIR(compare_color_worse));
    }
}

static void
isequal(void)
{
    if (has_colors())
    {
        attron(COLOR_PAIR(compare_color_equal));
        attron(compare_attr_equal);
    }

    lprcat("(equal) ");

    if (has_colors())
    {
        attroff(compare_attr_equal);
        attroff(COLOR_PAIR(compare_color_equal));
    }
}

static void
isnone(void)
{
    if (has_colors())
    {
        attron(COLOR_PAIR(compare_color_none));
        attron(compare_attr_none);
    }

    lprcat("(none equipped) ");

    if (has_colors())
    {
        attroff(compare_attr_none);
        attroff(COLOR_PAIR(compare_color_none));
    }
}

void
compareequipped(int obj)
{
    int wielded = iven[c[WIELD]];
    int worn = iven[c[WEAR]];
    int newv = 0, oldv = 0;

    switch (obj)
    {
    case OSWORDofSLASHING:
    case OHAMMER:
    case OSWORD:
    case O2SWORD:
    case OHSWORD:
    case OSPEAR:
    case ODAGGER:
    case OBATTLEAXE:
    case OLONGSWORD:
    case OLANCE:
    case OVORPAL:
    case OSLAYER:

        if (wielded <= 0)
        {
            isnone();
            return;
        }

    default:
        break;

        switch (obj)
        {
        case OSWORDofSLASHING:
            newv = 14;
            break;

        case OHAMMER:
            newv = 12;
            break;

        case OSWORD:
            newv = 10;
            break;

        case O2SWORD:
            newv = 15;
            break;

        case OHSWORD:
            newv = 25;
            break;

        case OSPEAR:
            newv = 6;
            break;

        case ODAGGER:
            newv = 3;
            break;

        case OBATTLEAXE:
            newv = 18;
            break;

        case OLONGSWORD:
            newv = 22;
            break;

        case OLANCE:
            newv = 30;
            break;

        case OVORPAL:
            newv = 40;
            break;

        case OSLAYER:
            newv = 50;
            break;

        default:
            break;
        }
        newv += ivenarg[obj];

        switch (wielded)
        {
        case OSWORDofSLASHING:
            oldv = 14;
            break;

        case OHAMMER:
            oldv = 12;
            break;

        case OSWORD:
            oldv = 10;
            break;

        case O2SWORD:
            oldv = 15;
            break;

        case OHSWORD:
            oldv = 25;
            break;

        case OSPEAR:
            oldv = 6;
            break;

        case ODAGGER:
            oldv = 3;
            break;

        case OBATTLEAXE:
            oldv = 18;
            break;

        case OLONGSWORD:
            oldv = 22;
            break;

        case OLANCE:
            oldv = 30;
            break;

        case OVORPAL:
            oldv = 40;
            break;

        case OSLAYER:
            oldv = 50;
            break;

        default:
            oldv = 0;
            break;
        }
        oldv += ivenarg[wielded];

        if (newv > oldv)
            isbetter();
        else if (newv < oldv)
            isworse();
        else
            isequal();

        return;
    }

    switch (obj)
    {
    case OPLATE:
    case OCHAIN:
    case OLEATHER:
    case ORING:
    case OSTUDLEATHER:
    case OSPLINT:
    case OPLATEARMOR:
    case OSSPLATE:
    case OELVENCHAIN:

        if (worn <= 0)
        {
            isnone();
            return;
        }

    default:
        break;

        switch (obj)
        {
        case OPLATE:
            newv = 7;
            break;

        case OCHAIN:
            newv = 5;
            break;

        case OLEATHER:
            newv = 2;
            break;

        case ORING:
            newv = 4;
            break;

        case OSTUDLEATHER:
            newv = 3;
            break;

        case OSPLINT:
            newv = 6;
            break;

        case OPLATEARMOR:
            newv = 8;
            break;

        case OSSPLATE:
            newv = 9;
            break;

        case OELVENCHAIN:
            newv = 10;
            break;

        default:
            break;
        }
        newv += ivenarg[obj];

        switch (worn)
        {
        case OPLATE:
            oldv = 7;
            break;

        case OCHAIN:
            oldv = 5;
            break;

        case OLEATHER:
            oldv = 2;
            break;

        case ORING:
            oldv = 4;
            break;

        case OSTUDLEATHER:
            oldv = 3;
            break;

        case OSPLINT:
            oldv = 6;
            break;

        case OPLATEARMOR:
            oldv = 8;
            break;

        case OSSPLATE:
            oldv = 9;
            break;

        case OELVENCHAIN:
            oldv = 10;
            break;

        default:
            oldv = 0;
            break;
        }
        oldv += ivenarg[worn];

        if (newv > oldv)
            isbetter();
        else if (newv < oldv)
            isworse();
        else
            isequal();

        return;
    }
}

/*
showcell(x,y)

subroutine to display a cell location on the screen
*/
void
showcell(int x, int y)
{
    int i, j, k, m;

    if (c[BLINDCOUNT])
        return;			/* see nothing if blind     */
    if (c[AWARENESS])
    {
        minx = x - 3;
        maxx = x + 3;
        miny = y - 3;
        maxy = y + 3;
    }
    else if (iven[c[WIELD]] == OHSWORD
        && ivenarg[c[WIELD]] >= 0)
    {
        minx = x - 2;
        maxx = x + 2;
        miny = y - 2;
        maxy = y + 2;
    }
    else
    {
        minx = x - 1;
        maxx = x + 1;
        miny = y - 1;
        maxy = y + 1;
    }

    if (minx < 0)
        minx = 0;
    if (maxx > MAXX - 1)
        maxx = MAXX - 1;
    if (miny < 0)
        miny = 0;
    if (maxy > MAXY - 1)
        maxy = MAXY - 1;

    for (j = miny; j <= maxy; j++)
        for (m = minx; m <= maxx; m++)
            if ((know[m][j] & KNOWHERE) == 0)
            {
                cursor(m + 1, j + 1);
                x = maxx;
                while (know[x][j] & KNOWHERE)
                    --x;
                for (i = m; i <= x; i++)
                {
                    if ((k = mitem[i][j]) != 0)
                    {
                        int id = k;

                        if (has_colors())
                        #if !defined(_WIN32) || defined(__DOS4G__) || defined(__OS2V2__)
                            attrset((int)(COLOR_PAIR(moncolor[id]) | monattr[id]));
                        #else
                            attrset(COLOR_PAIR(moncolor[id]) | monattr[id]);
                        #endif
                        lprc(monstnamelist[id]);

                        if (has_colors())
                            attrset(COLOR_PAIR(0));
                    }
                    else
                    {
                        int id = item[i][j];

                        if (has_colors())
                        #if !defined(_WIN32) || defined(__DOS4G__) || defined(__OS2V2__)
                            attrset((int)(COLOR_PAIR(objcolor[id]) | objattr[id]));
                        #else
                            attrset(COLOR_PAIR(objcolor[id]) | objattr[id]);
                        #endif
                        lprc(objnamelist[id]);

                        if (has_colors())
                            attrset(COLOR_PAIR(0));
                    }

                    know[i][j] = KNOWALL;
                }
                m = maxx;

            }
}

/*
this routine shows only the spot that is given it.  the spaces around
these coordinated are not shown
used in godirect() in monster.c for missile weapons display
*/
void
show1cell(int x, int y)
{
    int k;

    /* do not reveal unseen tiles */
    if (know[x][y] == 0)
        return;

    cursor(x + 1, y + 1);

    /* see nothing if blind, but clear previous player position */
    if (c[BLINDCOUNT])
    {
        if (x == oldx && y == oldy)
            lprc(' ');
        return;
    }

    /* mobs */
    k = mitem[x][y];
    if (k)
    {
        int id = k;

        if (has_colors()) {
            attron(COLOR_PAIR(moncolor[id]));
            attron(monattr[id]);
        }

        lprc(monstnamelist[id]);

        if (has_colors()) {
            attroff(monattr[id]);
            attroff(COLOR_PAIR(moncolor[id]));
        }

        know[x][y] = KNOWALL;
        return;
    }

    /* real time animated water tiles */
    if (item[x][y] == OWATER || item[x][y] == OSHOREWATER)
    {
        static const char water_chars[] = { '~', '=', '~', '=' };
        int idx = (x * 7 + y * 13 + water_anim_toggle) & 3;
        int id = item[x][y];

        if (has_colors()) {
            attron(COLOR_PAIR(objcolor[id]));
            attron(objattr[id]);
        }

        lprc(water_chars[idx]);

        if (has_colors()) {
            attroff(objattr[id]);
            attroff(COLOR_PAIR(objcolor[id]));
        }

        know[x][y] = KNOWALL;
        return;
    }

    /* real time animated + cooled lava tiles */
    if (item[x][y] == OLAVA)
    {
        /* animated lava */
        static const char lava_chars[] = { '~', '^', '"', '`' };
        int idx = (x * 13 + y * 7 + lava_anim_toggle) & 3;
        int id = item[x][y];

        if (has_colors()) {
            attron(COLOR_PAIR(objcolor[id]));
            attron(objattr[id]);
        }

        lprc(lava_chars[idx]);

        if (has_colors()) {
            attroff(objattr[id]);
            attroff(COLOR_PAIR(objcolor[id]));
        }

        know[x][y] = KNOWALL;
        return;
    }

    /* cooled lava tile */
    if (item[x][y] == OCOOLEDLAVA)
    {
        /* cooled lava */
        static const char cooled_lava_chars[] = { '`', '`', '`', '`' };
        int idx = (x * 13 + y * 7 + lava_anim_toggle) & 3;
        int id = item[x][y];

        if (has_colors()) {
            attron(COLOR_PAIR(objcolor[id]));
            attron(objattr[id]);
        }

        lprc(cooled_lava_chars[idx]);

        if (has_colors()) {
            attroff(objattr[id]);
            attroff(COLOR_PAIR(objcolor[id]));
        }

        know[x][y] = KNOWALL;
        return;
    }

    /* OWALL and OINNER WALL */
    if (item[x][y] == OWALL || item[x][y] == OINNERWALL)
    {
        int id = item[x][y];

        if (has_colors()) {
            attron(COLOR_PAIR(objcolor[id]));
            attron(objattr[id]);
        }
        
        lprcat(wallchar);

        if (has_colors()) {
            attroff(objattr[id]);
            attroff(COLOR_PAIR(objcolor[id]));
        }

        know[x][y] = KNOWALL;
        return;
    }

    /* object tiles */
    k = item[x][y];
    {
        int id = k;

        if (has_colors()) {
            attron(COLOR_PAIR(objcolor[id]));
            attron(objattr[id]);
        }

        lprc(objnamelist[id]);

        if (has_colors()) {
            attroff(objattr[id]);
            attroff(COLOR_PAIR(objcolor[id]));
        }
    }

    know[x][y] = KNOWALL;
}

static void
cursor_block(void)
{
    curs_set(0);

    if (has_colors())
    {
        attron(COLOR_PAIR(player_color));
        attron(player_attr);
    }

    addch(' ' | A_REVERSE);

    if (has_colors())
    {
        attroff(player_attr);
        attroff(COLOR_PAIR(player_color));
    }
}

/*
showplayer()

subroutine to show where the player is on the screen
cursor values start from 1 up
*/
void
showplayer(void)
{
    curs_set(0);
    show1cell(oldx, oldy);
    cursor(playerx + 1, playery + 1);
    cursor_block();
    cursor(playerx + 1, playery + 1);
    oldx = playerx;
    oldy = playery;
    refresh();
}

/*
moveplayer(dir)

subroutine to move the player from one room to another
returns 0 if can't move in that direction or hit a monster or on an object
else returns 1
nomove is set to 1 to stop the next move (inadvertent monsters hitting
players when walking into walls) if player walks off screen or into wall
*/
int
moveplayer(int dir)
/*  from = present room #  direction = [1-north]
[2-east] [3-south] [4-west] [5-northeast]
[6-northwest] [7-southeast] [8-southwest]
if direction=0, don't move--just show where he is */
{
    int k, m, i, j;
    int worn_armor_idx = c[WEAR];
    int shield_idx = c[SHIELD];
    int rusted_something = 0;

    if (c[CONFUSE])
        if (c[LEVEL] < rnd(30))
            dir = rund(9);		/*if confused any dir */
    k = playerx + diroffx[dir];
    m = playery + diroffy[dir];
    if (k < 0 || k >= MAXX || m < 0 || m >= MAXY)
    {
        nomove = 1;
        return (yrepcount = 0);
    }
    i = item[k][m];
    j = mitem[k][m];

    /* prevent the player from moving onto a wall, or a closed door,
       unless the character has Walk-Through-Walls.
     */
    if ((i == OCLOSEDDOOR || i == OWALL || i == OINNERWALL) && c[WTW] == 0)
    {
        nomove = 1;
        return (yrepcount = 0);
    }
    if (k == 33 && m == MAXY - 1 && level == 1)
    {
        newcavelevel(0);
        for (k = 0; k < MAXX; k++)
            for (m = 0; m < MAXY; m++)
                if (item[k][m] == OENTRANCE)
                {
                    playerx = k;
                    playery = m;
                    positionplayer();
                    drawscreen();
                    return (0);
                }
    }
    /* hit a monster
     */
    if (j > 0)
    {
        hitmonster(k, m);
        return (yrepcount = 0);
    }

    /* check for the player walking into lava */
    if (item[playerx][playery] == OLAVA && !(c[FIRERESISTANCE]))
    {
        cursors();
        lprcat("\nYou are burned by lava!");

        losehp(rnd(12) + 12);  /* a lot of damage but not instant death */
        bottomhp();
    }
    else if(item[playerx][playery] == OLAVA && (c[FIRERESISTANCE]))
    {
        cursors();
        lprcat("\nYou are protected from the lava!");
    }

    /* check for the player ignoring an altar */
    if (item[playerx][playery] == OALTAR && !prayed)
    {
        cursors();
        lprcat("\nYou have ignored the altar!");
        act_ignore_altar();
    }
    prayed = 0;

    lastpx = playerx;
    lastpy = playery;
    playerx = k;
    playery = m;

    if (i == OWATER)
    {
        /* 25% chance to rust */
        if (rnd(4) == 1)
        {
            /* Existing rusting logic for worn armor */
            if (worn_armor_idx != -1 && is_metal_armor(iven[worn_armor_idx])) {
                if (ivenarg[worn_armor_idx] > -10) {
                    ivenarg[worn_armor_idx] -= 2;
                    if (ivenarg[worn_armor_idx] < -10)
                        ivenarg[worn_armor_idx] = -10;
                    cursors();
                    lprcat("\nYour armor rusts from the water!");
                    rusted_something = 1;
                }
            }
        }

        /* 25% chance to rust */
        if (rnd(4) == 1)
        {
            /* Existing rusting logic for shield */
            if (shield_idx != -1 && is_metal_armor(iven[shield_idx])) {
                if (ivenarg[shield_idx] > -10) {
                    ivenarg[shield_idx] -= 2;
                    if (ivenarg[shield_idx] < -10)
                        ivenarg[shield_idx] = -10;
                    cursors();
                    lprcat("\nYour shield rusts from the water!");
                    rusted_something = 1;
                }
            }
        }

        if (rusted_something) {
            recalc();
            bottomline();
        }

        return
            (yrepcount = 0); /* Must be last in this block */
    }

    if (i && i != OTRAPARROWIV && i != OIVTELETRAP && i != OIVDARTRAP
        && i != OIVTRAPDOOR)
        return (yrepcount = 0);
    else
        return (1);
}

/*
*  function to show what magic items have been discovered thus far
*  enter with -1 for just spells, anything else will give scrolls & potions
*/
void
seemagic(int arg)
{
    int i, j, k, number;
    int sort[SPNUM + 1];		/* OK as long as SPNUM > MAXSCROLL,MAXPOTION */

    count = lincount = 0;

    /* count and sort the known spell codes
     */
    for (j = 0; j <= SPNUM; j++)
        sort[j] = SPNUM;
    for (number = i = 0; i < SPNUM; i++)
        if (spelknow[i])
        {
            number++;
            j = 0;
            while (strncmp(spelcode[sort[j]], spelcode[i], 3) < 0)
                j++;
            k = number - 1;
            while (k > j)
                sort[k] = sort[k - 1], k--;
            sort[j] = i;
        }

    if (arg == -1)		/* if display spells while casting one */
    {
        cl_up(79, ((number + 2) / 3 + 4));	/* lines needed for display */
        cursor(1, 1);
    }
    else
    {
        resetscroll();
        screen_clear();
    }

    lprcat("The magic spells you have discovered thus far:\n\n");
    for (i = 0; i < number; i++)
    {
        lprintf("%s %-20s ", spelcode[sort[i]], spelname[sort[i]]);
        seepage();
    }

    if (arg == -1)
    {
        seepage();
        more(FALSE);
        draws(0, MAXX, 0, ((number + 2) / 3 + 4));
        return;
    }

    lincount += 3;
    if (count != 0)
    {
        count = 2;
        seepage();
    }

    /* count and sort the known scrolls
     */
    for (j = 0; j <= MAXSCROLL; j++)
        sort[j] = MAXSCROLL;
    for (number = i = 0; i < MAXSCROLL; i++)
        if (scrollname[i][0])
        {
            number++;
            j = 0;
            while (strcmp(&scrollname[sort[j]][1], &scrollname[i][1]) < 0)
                j++;
            k = number - 1;
            while (k > j)
                sort[k] = sort[k - 1], k--;
            sort[j] = i;
        }

    lprcat("\nThe magic scrolls you have found to date are:\n\n");
    count = 0;
    for (i = 0; i < number; i++)
    {
        lprintf("%-26s", &scrollname[sort[i]][1]);
        seepage();
    }

    lincount += 3;
    if (count != 0)
    {
        count = 2;
        seepage();
    }

    /* count and sort the known potions
     */
    for (j = 0; j <= MAXPOTION; j++)
        sort[j] = MAXPOTION;
    for (number = i = 0; i < MAXPOTION; i++)
        if (potionname[i][0])
        {
            number++;
            j = 0;
            while (strcmp(&potionname[sort[j]][1], &potionname[i][1]) < 0)
                j++;
            k = number - 1;
            while (k > j)
                sort[k] = sort[k - 1], k--;
            sort[j] = i;
        }

    lprcat("\nThe magic potions you have found to date are:\n\n");
    count = 0;
    for (i = 0; i < number; i++)
    {
        lprintf("%-26s", &potionname[sort[i]][1]);
        seepage();
    }

    if (lincount != 0)
        more(FALSE);
    setscroll();
    drawscreen();
}

/*
*  subroutine to paginate the seemagic function
*/
static void
seepage(void)
{
    if (++count == 3)
    {
        lincount++;
        count = 0;
        lprc('\n');
        if (lincount > 17)
        {
            lincount = 0;
            more(FALSE);
            screen_clear();
        }
    }
}

static int
is_metal_armor(int item_id) {
    switch (item_id) {
    case OPLATE:
    case OCHAIN:
    case OSPLINT:
    case ORING:
    case OPLATEARMOR:
    case OSSPLATE:
    case OELVENCHAIN:
    case OSHIELD:
        return 1;
    default:
        return 0;
    }
}

void
readcolors(void)
{
    FILE *fp;
    char line[256];
    char key[64];
    char *eq;
    char *rhs;
    char *tok;
    int color;
    chtype attr;
    size_t i;

    fp = fopen("larn.clr", "r");
    if (!fp)
        return;

    while (fgets(line, sizeof(line), fp))
    {
        /* skip comments and blank lines */
        if (line[0] == '#' || line[0] == '\n')
            continue;

        /* find '=' */
        eq = strchr(line, '=');
        if (!eq)
            continue;

        /* split key and rhs */
        *eq = '\0';
        rhs = eq + 1;

        /* trim key */
        sscanf(line, "%63s", key);

        /* tokenize COLOR_* and A_* */
        color = -1;
        attr  = 0;

        tok = strtok(rhs, " \t|()\r\n");
        while (tok)
        {
            /* colours */
            if (!strcmp(tok, "COLOR_BLACK"))        color = COLOR_BLACK;
            else if (!strcmp(tok, "COLOR_RED"))     color = COLOR_RED;
            else if (!strcmp(tok, "COLOR_GREEN"))   color = COLOR_GREEN;
            else if (!strcmp(tok, "COLOR_YELLOW"))  color = COLOR_YELLOW;
            else if (!strcmp(tok, "COLOR_BLUE"))    color = COLOR_BLUE;
            else if (!strcmp(tok, "COLOR_MAGENTA")) color = COLOR_MAGENTA;
            else if (!strcmp(tok, "COLOR_CYAN"))    color = COLOR_CYAN;
            else if (!strcmp(tok, "COLOR_WHITE"))   color = COLOR_WHITE;

            /* attributes */
            else if (!strcmp(tok, "A_BOLD"))        attr |= A_BOLD;
            else if (!strcmp(tok, "A_DIM"))         attr |= A_DIM;
            else if (!strcmp(tok, "A_REVERSE"))     attr |= A_REVERSE;
            else if (!strcmp(tok, "A_STANDOUT"))    attr |= A_STANDOUT;
            else if (!strcmp(tok, "A_UNDERLINE"))   attr |= A_UNDERLINE;

            tok = strtok(NULL, " \t|()\r\n");
        }

        if (color < 0)
            continue;

        /* monsters */
        for (i = 0; monster_map[i].name; i++)
        {
            if (!strcmp(key, monster_map[i].name))
            {
                moncolor[monster_map[i].id] = color;
                monattr[monster_map[i].id]  = attr;
            }
        }

        /* objects */
        for (i = 0; object_map[i].name; i++)
        {
            if (!strcmp(key, object_map[i].name))
            {
                objcolor[object_map[i].id] = color;
                objattr[object_map[i].id]  = attr;
            }
        }

        /* comparisons */
        if (!strcmp(key, "COMPARE_BETTER")) {
            compare_color_better = color;
            compare_attr_better  = attr;
        }
        else if (!strcmp(key, "COMPARE_WORSE")) {
            compare_color_worse = color;
            compare_attr_worse  = attr;
        }
        else if (!strcmp(key, "COMPARE_EQUAL")) {
            compare_color_equal = color;
            compare_attr_equal  = attr;
        }
        else if (!strcmp(key, "COMPARE_NONE")) {
            compare_color_none = color;
            compare_attr_none  = attr;
        }
        else if (!strcmp(key, "COOKIE_MESSAGE")) {
            cookie_color = color;
            cookie_attr  = attr;
        }
        else if (!strcmp(key, "MAGIC_MISSILE")) {
            missile_color = color;
            missile_attr  = attr;
        }
        else if (!strcmp(key, "PLAYER")) {
            player_color = color;
            player_attr  = attr;
        }
    }
    fclose(fp);
}

/* the new and improved UX of Larn! ~Gibbon */
void
statusmessage_draw_lines(void)
{
    /* spells, AC, WC, level, exp, class */
    cursor(1, 18);
    lprintf("Spells:%3d(%2d)  AC:%-3d  WC:%-3d  Level:%-2d  Exp:%-9d %s",
        (int)c[SPELLS],
        (int)c[SPELLMAX],
        (int)c[AC],
        (int)c[WCLASS],
        (int)c[LEVEL],
        (int)c[EXPERIENCE],
        classname[c[LEVEL] - 1]);

    /* HP, stats, level name, gold */
    cursor(1, 19);
    lprintf("HP:%3d(%3d)  STR:%-2d  INT:%-2d  WIS:%-2d  CON:%-2d  DEX:%-2d  CHA:%-2d  LV:%s  Gold:%-6d",
        (int)c[HP],
        (int)c[HPMAX],
        (int)(c[STRENGTH] + c[STREXTRA]),
        (int)c[INTELLIGENCE],
        (int)c[WISDOM],
        (int)c[CONSTITUTION],
        (int)c[DEXTERITY],
        (int)c[CHARISMA],
        levelname[level],
        (int)c[GOLD]);

    /* player name */
    cursor(1, 20);
    lprintf("%s", logname);
}

void
statusmessage_draw_right_panel(void)
{
    int i, idx;

    for (i = 0; i < 17; i++)
    {
        idx = bot_data[i].typ;
        cursor(70, i + 1);

        if (c[idx])
            lprintf("%-10s", bot_data[i].string);
        else
            lprintf("%-10s", "");
    }
}
