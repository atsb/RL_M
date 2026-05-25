/*
 * Larn — Copyright © 1986 Noah Morgan
 *        Copyright © 2014-2026 Gibbon
 *
 * This file is part of Larn and is distributed under
 * The Noah Licence, Version 1.0.
 *
 * You may use, modify, and redistribute this code for
 * non-commercial purposes, provided that:
 *   - this notice is preserved,
 *   - The Noah Licence accompanies all redistributions, and
 *   - no profit is made from Larn or derivative works
 *     without explicit permission from the copyright holder.
 *
 * Larn is provided “AS IS”, without warranty of any kind.
 *
 * See the 'LICENSE.txt' file in the 'docs' folder.
 */

#include "create.h"
#include "display.h"
#include "larn.h"
#include "global.h"
#include "io.h"
#include "monster.h"
#include "savelev.h"
#include "scores.h"

static void makemaze(int);
static int cannedlevel(int);
static void treasureroom(int);
static void troom(int, int, int, int, int, int);
static void makeobject(int);
static void fillmroom(int, int, int);
static void froom(int, int, int);
static void fillroom(int, int);
static void sethp(int);
static void checkgen(void);
static void makepuddle(int);
static void cavernsmoothing(void);
static void worldboundary(void);

unsigned char erosion[MAXX][MAXY];
long last_simulated_time[MAXLEVEL];
unsigned char lavaheat[MAXX][MAXY];
long last_lava_cool = 0;

/*
makeplayer()

subroutine to create the player and the players attributes
this is called at the beginning of a game and at no other time
*/
void
makeplayer(void)
{
    int i;

    scbr();
    screen_clear();

    /*  start player off with 15 hit points */
    c[HPMAX] = c[HP] = 10;

    /*  player starts at level one          */
    c[LEVEL] = 1;

    /*  total # spells starts off as 3  */
    c[SPELLMAX] = c[SPELLS] = 1;

    /* start regeneration correctly */
    c[REGENCOUNTER] = 16;
    c[ECOUNTER] = 96;

    c[SHIELD] = c[WEAR] = c[WIELD] = -1;

    for (i = 0; i < 26; i++)
    {

        iven[i] = 0;
    }

    /* he knows protection, magic missile */
    spelknow[0] = spelknow[1] = 1;

    if (c[HARDGAME] <= 0)
    {

        iven[0] = OLEATHER;
        iven[1] = ODAGGER;
        iven[2] = 0;
        ivenarg[1] = ivenarg[0] = c[WEAR] = 0;
        c[WIELD] = 1;
    }

    playerx = rnd(MAXX - 2);
    playery = rnd(MAXY - 2);

    /* make the attributes, ie str, int, etc. */
    for (i = 0; i < 6; i++)
    {

        c[i] = 12;
    }

    recalc();

    screen_clear();

    enter_name();
}


/*
newcavelevel(level)
int level;

function to enter a new level.  This routine must be called anytime the
player changes levels.  If that level is unknown it will be created.
A new set of monsters will be created for a new level, and existing
levels will get a few more monsters.
Note that it is here we remove genocided monsters from the present level.
*/
void
newcavelevel(int x)
{
    int i, j, t, n, cx, cy, ponds;
    long now = time(NULL);
    long elapsed = now - last_simulated_time[x];
    int water_ticks = elapsed / 3;

    if (beenhere[level])
    {
        savelevel();        /* put the level back into storage  */
        last_simulated_time[level] = time(NULL); /* store the last tic and simulate it when we return */
    }

    level = x; /* get the new level and put in working storage */

    /* store the last simulated time for water expansion while player is not on the level */
    if (beenhere[x])
    {
        getlevel();

        for (t = 0; t < water_ticks; t++)
            expand_puddle();

        last_simulated_time[x] = now;

        sethp(0);
        positionplayer();
        checkgen();
        return;
    }

    /* init erosion for a new level */
    for (i = 0; i < MAXY; i++)
        for (j = 0; j < MAXX; j++)
            erosion[j][i] = 0;

    /* init lavaheat for a new level */
    for (i = 0; i < MAXY; i++)
        for (j = 0; j < MAXX; j++)
            lavaheat[j][i] = 0;

    /* fill in new level */
    for (i = 0; i < MAXY; i++)
        for (j = 0; j < MAXX; j++)
        {
            know[j][i] = 0;
            mitem[j][i] = 0;
        }

    makemaze(x);

    /*  lava generation only on volcano levels */
    if (level >= VOLCANOLEVEL_START && level <= VOLCANOLEVEL_END)
    {
        /* small lava ponds */
        ponds = rnd(3) + 2;
        for (n = 0; n < ponds; n++)
        {
            cx = rnd(MAXX - 4) + 2;
            cy = rnd(MAXY - 4) + 2;

            if (item[cx][cy] == 0)
                make_lavapool(cx, cy);

        }

        /* crying lava pools */
        for (n = 0; n < ponds; n++)
        {
            make_lavapool(cx, cy);
            make_cryinglava(cx, cy);
        }
    }

    makeobject(x);
    beenhere[x] = 1;
    sethp(1);
    positionplayer();
    predostuff = 2;
    checkgen();         /* wipe out any genocided monsters */

#if WIZID
    if (wizard || x == 0)
#else
    if (x == 0)
#endif
        for (j = 0; j < MAXY; j++)
            for (i = 0; i < MAXX; i++)
                know[i][j] = KNOWALL;

    worldboundary();
}

/*
makecorridor_caverns(int x1, int y1, int x2, int y2)

subroutine to make the caverns corridor connections.
*/
static void
makecorridor_caverns(int x1, int y1, int x2, int y2)
{
    int x, y;
    int steps;

    x = x1;
    y = y1;
    steps = 0;

    while ((x != x2 || y != y2) && steps < 500)
    {
        if (x > 0 && x < MAXX - 1 && y > 0 && y < MAXY - 1)
        {
            item[x][y] = 0;

            /* widen corridor */
            if (rnd(2))
                item[x+1][y] = 0;
            if (rnd(2))
                item[x-1][y] = 0;
            if (rnd(2))
                item[x][y+1] = 0;
            if (rnd(2))
                item[x][y-1] = 0;
        }
        if (rnd(100) < 60)
        {
            if (x < x2)
                x++;
            else if (x > x2)
                x--;
            else if (y < y2)
                y++;
            else if (y > y2)
                y--;
        }
        else
        {
            if (y < y2)
                y++;
            else if (y > y2)
                y--;
            else if (x < x2)
                x++;
            else if (x > x2)
                x--;
        }
        steps++;
    }
}

/*
makemaze_caverns(level)

subroutine to make the caverns for a given level.
*/
static void
makemaze_caverns(int k)
{
    int i, j;
    int cell_w, cell_h;
    int rx[3][3], ry[3][3], rw[3][3], rh[3][3];
    int gone[3][3];
    int gx, gy;
    int max_w, max_h;
    int cell_x, cell_y;
    int off_x_max, off_y_max;
    int x1, y1, x2, y2;
    int xx, yy;

    /* SEA OF WALL!!! */
    for (i = 0; i < MAXY; i++)
        for (j = 0; j < MAXX; j++)
            item[j][i] = OINNERWALL;

    /* solid outer boundary */
    for (i = 0; i < MAXY; i++)
    {
        item[0][i] = OWALL;
        item[MAXX - 1][i] = OWALL;
    }
    for (j = 0; j < MAXX; j++)
    {
        item[j][0] = OWALL;
        item[j][MAXY - 1] = OWALL;
    }

    /* 3x3 grid */
    cell_w = (MAXX - 2) / 3;
    cell_h = (MAXY - 2) / 3;
    if (cell_w < 6) cell_w = 6;
    if (cell_h < 6) cell_h = 6;

    /* gone rooms */
    for (gy = 0; gy < 3; gy++)
        for (gx = 0; gx < 3; gx++)
            gone[gx][gy] = (rnd(4) == 0);

    /* carve rooms */
    for (gy = 0; gy < 3; gy++)
    {
        for (gx = 0; gx < 3; gx++)
        {
            if (gone[gx][gy])
            {
                rw[gx][gy] = 0;
                rh[gx][gy] = 0;
                continue;
            }

            max_w = cell_w - 2;
            max_h = cell_h - 2;
            if (max_w < 4) max_w = 4;
            if (max_h < 4) max_h = 4;

            rw[gx][gy] = rnd(max_w - 3) + 4;
            rh[gx][gy] = rnd(max_h - 3) + 4;

            cell_x = 1 + gx * cell_w;
            cell_y = 1 + gy * cell_h;

            off_x_max = cell_w - rw[gx][gy];
            off_y_max = cell_h - rh[gx][gy];
            if (off_x_max < 1) off_x_max = 1;
            if (off_y_max < 1) off_y_max = 1;

            rx[gx][gy] = cell_x + rnd(off_x_max);
            ry[gx][gy] = cell_y + rnd(off_y_max);

            /* carve interior */
            for (i = 0; i < rh[gx][gy]; i++)
                for (j = 0; j < rw[gx][gy]; j++)
                {
                    xx = rx[gx][gy] + j;
                    yy = ry[gx][gy] + i;
                    if (xx > 0 && xx < MAXX - 1 && yy > 0 && yy < MAXY - 1)
                        item[xx][yy] = 0;
                }
        }
    }

    /* horizontal connections */
    for (gy = 0; gy < 3; gy++)
    {
        for (gx = 0; gx < 2; gx++)
        {
            if (rw[gx][gy] && rw[gx + 1][gy])
            {
                x1 = rx[gx][gy] + rw[gx][gy] / 2;
                y1 = ry[gx][gy] + rh[gx][gy] / 2;
                x2 = rx[gx + 1][gy] + rw[gx + 1][gy] / 2;
                y2 = ry[gx + 1][gy] + rh[gx + 1][gy] / 2;
                makecorridor_caverns(x1, y1, x2, y2);
            }
        }
    }

    /* vertical connections */
    for (gx = 0; gx < 3; gx++)
    {
        for (gy = 0; gy < 2; gy++)
        {
            if (rh[gx][gy] && rh[gx][gy + 1])
            {
                x1 = rx[gx][gy] + rw[gx][gy] / 2;
                y1 = ry[gx][gy] + rh[gx][gy] / 2;
                x2 = rx[gx][gy + 1] + rw[gx][gy + 1] / 2;
                y2 = ry[gx][gy + 1] + rh[gx][gy + 1] / 2;
                makecorridor_caverns(x1, y1, x2, y2);
            }
        }
    }

    /* diagonal connections */
    for (gy = 0; gy < 2; gy++)
    {
        for (gx = 0; gx < 2; gx++)
        {
            if (rw[gx][gy] && rw[gx + 1][gy + 1] && rnd(100) < 40)
            {
                x1 = rx[gx][gy] + rw[gx][gy] / 2;
                y1 = ry[gx][gy] + rh[gx][gy] / 2;
                x2 = rx[gx + 1][gy + 1] + rw[gx + 1][gy + 1] / 2;
                y2 = ry[gx + 1][gy + 1] + rh[gx + 1][gy + 1] / 2;
                makecorridor_caverns(x1, y1, x2, y2);
            }
        }
    }

    if (k == 1)
    {
        item[33][MAXY - 1] = OENTRANCE;
    }

    /* 
    * calling this twice reiterates over, making the smoothness
    * more realistic
    */
    cavernsmoothing();
    cavernsmoothing();

    worldboundary();
}

/*
makemaze(int)
subroutine to make the caverns for a given level.  only walls are made.
*/
static void
makemaze(int k)
{
    int i, j, tmp;
    int z;
    int my  = rnd(11) + 2;
    int myl = my - rnd(2);
    int myh = my + rnd(2);
    int mx, mxl, mxh;
    int tmp2 = rnd(3) + 3;

    /* ~30% chance to generate a caverns style maze */
    if (k > 0 && k != MAXLEVEL - 1 && rnd(100) < 30)
    {
        makemaze_caverns(k);
        return;
    }

    /* canned levels */
    if (k > 1 &&
        (rnd(17) <= 4 || k == MAXLEVEL - 1 ||
         k == MAXLEVEL + MAXVLEVEL - 1))
    {
        if (cannedlevel(k))
            return;
    }

    /* level 0 (town) uses no walls */
    if (k == 0)
    {
        for (i = 0; i < MAXY; i++)
            for (j = 0; j < MAXX; j++)
                item[j][i] = 0;
        return;
    }

    for (i = 0; i < MAXY; i++)
        for (j = 0; j < MAXX; j++)
            item[j][i] = OINNERWALL;

    for (i = 0; i < MAXX; i++)
    {
        item[i][0] = OWALL;
        item[i][MAXY - 1] = OWALL;
    }
    for (j = 0; j < MAXY; j++)
    {
        item[0][j] = OWALL;
        item[MAXX - 1][j] = OWALL;
    }

    /* carve the maze */
    eat(1, 1);

    /* entrance on level 1 */
    if (k == 1)
        item[33][MAXY - 1] = OENTRANCE;

    /*  now for open spaces -- not on level 10  */
    if (k != MAXLEVEL - 1)
    {
        for (tmp = 0; tmp < tmp2; tmp++)
        {

            if (k < MAXLEVEL)
            {
                mx  = rnd(44) + 5;
                mxl = mx - rnd(4);
                mxh = mx + rnd(12) + 3;
                z   = 0;
            }
            else
            {
                mx  = rnd(60) + 3;
                mxl = mx - rnd(2);
                mxh = mx + rnd(2);
                z   = makemonst(k);
            }

            for (i = mxl; i < mxh; i++)
            {
                for (j = myl; j < myh; j++)
                {
                    item[i][j] = 0;

                    /* do not spawn mobs in lava except red dragons */
                    if (item[i][j] == OLAVA)
                    {
                        mitem[i][j] = REDDRAGON;
                        hitp[i][j]  = monster[REDDRAGON].hitpoints;
                        monster[REDDRAGON].resistance |= FIRERESISTANCE;
                        continue;
                    }

                    mitem[i][j] = z;
                    if (z != 0)
                        hitp[i][j] = monster[z].hitpoints;
                }
            }
        }
    }

    /* horizontal corridor -- not on level 10 */
    if (k != MAXLEVEL - 1)
    {
        my = rnd(MAXY - 2);
        for (i = 1; i < MAXX - 1; i++)
            item[i][my] = 0;
    }

    if (k > 1)
        treasureroom(k);

    worldboundary();
}

/* this enforces the OWALL boundary and generation edge cases */
static void
worldboundary(void)
{
    int x, y;
    int ex, ey;

    /* level 0 has no walls */
    if (level == 0)
        return;

    /* boundary walls on top and bottom */
    for (x = 0; x < MAXX; x++) {

        /* preserve the dungeon entrance on level 1 */
        if (level == 1 && x == 33)
            continue;

        item[x][0]        = OWALL;
        item[x][MAXY - 1] = OWALL;
    }

    /* boundary walls on left and right */
    for (y = 0; y < MAXY; y++) {
        item[0][y]        = OWALL;
        item[MAXX - 1][y] = OWALL;
    }

    /* keep the entrance clear */
    if (level == 1) {

        ex = 33;
        ey = MAXY - 1;

        item[ex][ey] = OENTRANCE;

        /* clear a 5-tile radius around the entrance */
        for (y = ey - 5; y <= ey + 5; y++) {
            for (x = ex - 5; x <= ex + 5; x++) {

                if (x < 1 || x >= MAXX - 1 || y < 1 || y >= MAXY - 1)
                    continue;

                if (item[x][y] == OINNERWALL)
                    item[x][y] = 0;
            }
        }
    }
}

static void
cavernsmoothing(void)
{
    int x, y, count;
    unsigned char tmp[MAXX][MAXY];

    for (y = 1; y < MAXY - 1; y++)
        for (x = 1; x < MAXX - 1; x++)
        {
            count = 0;

            if (item[x][y] != 0) count++;

            if (item[x-1][y] != 0) count++;
            if (item[x+1][y] != 0) count++;
            if (item[x][y-1] != 0) count++;
            if (item[x][y+1] != 0) count++;

            if (count >= 3)
                tmp[x][y] = OINNERWALL;
            else
                tmp[x][y] = 0;
        }

    for (y = 1; y < MAXY - 1; y++)
        for (x = 1; x < MAXX - 1; x++)
            item[x][y] = tmp[x][y];
}

/*
* function to eat away a filled in maze
*/
void
eat(int xx, int yy)
{
    int dir = rnd(4);
    int try = 2;

    while (try)
    {
        switch (dir)
        {
        case 1: /* west */
            if (xx <= 2)
                break;

            if (!((item[xx - 1][yy] == OWALL || item[xx - 1][yy] == OINNERWALL) &&
                  (item[xx - 2][yy] == OWALL || item[xx - 2][yy] == OINNERWALL)))
                break;

            /* carve direction */
            item[xx - 1][yy] = 0;
            item[xx - 2][yy] = 0;

            /* chance to carve sideways */
            if (rnd(10) < 3) {
                int wx = xx - 1;
                int wy = yy + (rnd(2) ? 1 : -1);
                if (wy > 1 && wy < MAXY - 2)
                    item[wx][wy] = 0;
            }

            eat(xx - 2, yy);
            break;

        case 2: /* east */
            if (xx >= MAXX - 3)
                break;

            if (!((item[xx + 1][yy] == OWALL || item[xx + 1][yy] == OINNERWALL) &&
                  (item[xx + 2][yy] == OWALL || item[xx + 2][yy] == OINNERWALL)))
                break;

            item[xx + 1][yy] = 0;
            item[xx + 2][yy] = 0;
            eat(xx + 2, yy);
            break;

        case 3: /* south */
            if (yy <= 2)
                break;

            if (!((item[xx][yy - 1] == OWALL || item[xx][yy - 1] == OINNERWALL) &&
                  (item[xx][yy - 2] == OWALL || item[xx][yy - 2] == OINNERWALL)))
                break;

            item[xx][yy - 1] = 0;
            item[xx][yy - 2] = 0;
            eat(xx, yy - 2);
            break;

        case 4: /* north */
            if (yy >= MAXY - 3)
                break;

            if (!((item[xx][yy + 1] == OWALL || item[xx][yy + 1] == OINNERWALL) &&
                  (item[xx][yy + 2] == OWALL || item[xx][yy + 2] == OINNERWALL)))
                break;

            item[xx][yy + 1] = 0;
            item[xx][yy + 2] = 0;
            eat(xx, yy + 2);
            break;
        }

        if (++dir > 4)
        {
            dir = 1;
            --try;
        }
    }
}

int
lava_blocked(int x, int y)
{
    switch (item[x][y])
    {
    case OENTRANCE:
    case OWALL:
    case ODNDSTORE:
    case OSCHOOL:
    case OBANK:
    case OVOLDOWN:
    case OHOME:
    case OTRADEPOST:
    case OLRS:
    case OSTAIRSUP:
    case OSTAIRSDOWN:
    case OVOLUP:
        return 1;
    }
    return 0;
}

void
cool_lava(void)
{
    int x, y;

    for (y = 0; y < MAXY; y++)
    {
        for (x = 0; x < MAXX; x++)
        {
            if (item[x][y] == OLAVA && lavaheat[x][y] > 0)
            {
                lavaheat[x][y]--;

                if (lavaheat[x][y] == 0)
                {
                    item[x][y] = OCOOLEDLAVA;
                    show1cell(x, y);
                }
            }
        }
    }
    refresh();
}

void
make_lavapool(int cx, int cy)
{
    int radius, x, y, dx, dy, jitter;

    radius = rnd(2);
    radius += 1;

    for (y = cy - radius - 1; y <= cy + radius + 1; y++)
    {
        for (x = cx - radius - 1; x <= cx + radius + 1; x++)
        {
            if (x < 1 || x >= MAXX - 1 || y < 1 || y >= MAXY - 1)
                continue;

            if (mitem[x][y] != 0)
                continue;

            dx = x - cx;
            dy = y - cy;

            /* wobble */
            jitter = rnd(3) - 1;

            if (dx * dx + dy * dy <= radius * radius + jitter)
                if (item[x][y] != OWALL && !lava_blocked(x, y))
                {
                    item[x][y] = OLAVA;
                    lavaheat[x][y] = rnd(100) + 50;   /* stays hot */
                }
        }
    }
}

void
make_cryinglava(int cx, int cy)
{
    int streaks, s;
    int x, y, dx, dy;
    int length, i;
    int angle, dist;

    /* streaks amount */
    streaks = rnd(5) + 4;

    for (s = 0; s < streaks; s++)
    {
        angle = rnd(360);

        /* convert angle to dx/dy */
        if (angle < 90) { dx = 1;  dy = -1; }
        else if (angle < 180) { dx = 1;  dy = 1; }
        else if (angle < 270) { dx = -1; dy = 1; }
        else { dx = -1; dy = -1; }

        /* begin just outside the pool */
        dist = rnd(2) + 1;
        x = cx + dx * dist;
        y = cy + dy * dist;

        /* streaks length */
        length = rnd(2) + 1;

        for (i = 0; i < length; i++)
        {
            if (x < 1 || x >= MAXX - 1 || y < 1 || y >= MAXY - 1)
                break;

            if (item[x][y] != OWALL && mitem[x][y] == 0 && !lava_blocked(x, y))
            {
                item[x][y] = OLAVA;
                lavaheat[x][y] = rnd(20) + 30;   /* arms cool quicker */
            }

            /* wiggle */
            if (rnd(10) < 3)
            {
                if (dx == 0) x += (rnd(2) ? 1 : -1);
                else if (dy == 0) y += (rnd(2) ? 1 : -1);
                else
                {
                    if (rnd(2)) x += (rnd(2) ? 1 : -1);
                    else        y += (rnd(2) ? 1 : -1);
                }
            }

            x += dx;
            y += dy;
        }
    }
}

/*
* makestream(lvl)
*
* subroutine to make a long winding horizontal river
* or a more subdued stream.
*/
static void
makestream(int lvl)
{
    int x, y, dx, dy, len, i;
    int width;
    int max_len;
    int dominant;
    int t;
    int sx, sy;
    int wiggle;

    /* do not create water in the volcano */
    if (lvl >= VOLCANOLEVEL_START && lvl <= VOLCANOLEVEL_END)
        return;

    width = 2;

    /* random full horizontal river */
    if (rnd(5) == 1)
    {
        /* random Y */
        y = rnd(MAXY - 4) + 2;

        /* start west edge */
        for (x = 1; x < MAXX - 1; x++)
        {
            /* vertical wiggle */
            if (rnd(10) < 2)
            {
                wiggle = (rnd(2) ? 1 : -1);
                if (y + wiggle > 1 && y + wiggle < MAXY - 2)
                    y += wiggle;
            }

            /* create river */
            for (i = 0; i < width; i++)
            {
                sx = x;
                sy = y + i;

                if (sx < 1 || sx >= MAXX - 1 || sy < 1 || sy >= MAXY - 1)
                    continue;

                if (mitem[sx][sy] != 0)
                    continue;

                /* avoid destroying structures */
                switch (item[sx][sy])
                {
                case OENTRANCE:
                case OWALL:
                case ODNDSTORE:
                case OSCHOOL:
                case OBANK:
                case OVOLDOWN:
                case OHOME:
                case OTRADEPOST:
                case OLRS:
                case OSTAIRSUP:
                case OSTAIRSDOWN:
                case OVOLUP:
                    continue;
                }

                item[sx][sy] = OWATER;

                if (item[sx - 1][sy] != OWATER ||
                    item[sx + 1][sy] != OWATER ||
                    item[sx][sy - 1] != OWATER ||
                    item[sx][sy + 1] != OWATER)
                {
                    item[sx][sy] = OSHOREWATER;
                }
                iarg[sx][sy] = 0;
            }
        }

        return; /* end river */
    }

    /* snaky streams */
    max_len = 10 + rnd(25);

    /* random starting point */
    x = rnd(MAXX - 4) + 2;
    y = rnd(MAXY - 4) + 2;

    /* randomised dominancy */
    dominant = rnd(2);

    /* dominant direction */
    if (dominant == 0) {
        dx = (rnd(2) ? 1 : -1);
        dy = 0;
    }
    else {
        dx = 0;
        dy = (rnd(2) ? 1 : -1);
    }

    for (len = 0; len < max_len; len++)
    {
        /* 70% chance to just go straight */
        if (rnd(10) < 7) {
            /* intentionally empty */
        }
        /* 20% chance to wiggle */
        else if (rnd(10) < 9) {
            if (dominant == 0) {
                dy = (rnd(2) ? 1 : -1);
                dx = (rnd(2) ? 1 : -1);
            }
            else {
                dx = (rnd(2) ? 1 : -1);
                dy = (rnd(2) ? 1 : -1);
            }
        }
        /* 10% chance for 90 degrees turn */
        else {
            t = dx;
            dx = -dy;
            dy = t;
        }

        x += dx;
        y += dy;

        /* boundary check */
        if (x < 1 || x >= MAXX - 2 || y < 1 || y >= MAXY - 2)
            break;

        /* create stream */
        for (i = 0; i < width; i++)
        {
            sx = x;
            sy = y;

            if (dx != 0)
                sy += i;  /* horizontal */
            else
                sx += i;  /* vertical */

            if (sx < 1 || sx >= MAXX - 1 || sy < 1 || sy >= MAXY - 1)
                continue;

            if (mitem[sx][sy] != 0)
                continue;

            /* avoid overwriting structures */
            switch (item[sx][sy])
            {
            case OENTRANCE:
            case OWALL:
            case ODNDSTORE:
            case OSCHOOL:
            case OBANK:
            case OVOLDOWN:
            case OHOME:
            case OTRADEPOST:
            case OLRS:
            case OSTAIRSUP:
            case OSTAIRSDOWN:
            case OVOLUP:
                continue;
            }

            item[sx][sy] = OWATER;

            if (item[sx - 1][sy] != OWATER ||
                item[sx + 1][sy] != OWATER ||
                item[sx][sy - 1] != OWATER ||
                item[sx][sy + 1] != OWATER)
            {
                item[sx][sy] = OSHOREWATER;
            }
            iarg[sx][sy] = 0;
        }
    }
}

/*
* expand_puddle(void)
*
* subroutine to make a puddle of a given size and slowly expand it to simulate dripping.
*/
void
expand_puddle(void)
{
    int y, x, d;
    int nx, ny;
    int gx[4], gy[4], gcount;
    int dx[4] = { 0, 0, -1, 1 };
    int dy[4] = { -1, 1, 0, 0 };
    int pick;
    int puddle_count;
    int xx, yy;
    int MAX_RADIUS;

    MAX_RADIUS = 10;

    if (level >= VOLCANOLEVEL_START && level <= VOLCANOLEVEL_END)
        return;

    if (rnd(6) != 1)
        return;

    for (y = 1; y < MAXY - 1; y++)
    {
        for (x = 1; x < MAXX - 1; x++)
        {
            if (item[x][y] != OWATER)
                continue;

            gcount = 0;

            for (d = 0; d < 4; d++)
            {
                nx = x + dx[d];
                ny = y + dy[d];

                if (nx < 1 || nx >= MAXX - 1 || ny < 1 || ny >= MAXY - 1)
                    continue;

                if (mitem[nx][ny] != 0)
                    continue;

                switch (item[nx][ny])
                {
                case OENTRANCE:
                case OWALL:
                case ODNDSTORE:
                case OSCHOOL:
                case OBANK:
                case OVOLDOWN:
                case OHOME:
                case OTRADEPOST:
                case OLRS:
                case OSTAIRSUP:
                case OSTAIRSDOWN:
                case OVOLUP:
                    continue;
                }

                if (item[nx][ny] == OWALL)
                    continue;

                if (item[nx][ny] == OINNERWALL)
                {
                    erosion[nx][ny]++;

                    if (erosion[nx][ny] < 5)
                        continue;

                    if (rnd(20) > erosion[nx][ny])
                        continue;

                    item[nx][ny] = 0;
                    know[nx][ny] = 0;
                    show1cell(nx, ny);
                    erosion[nx][ny] = 0;

                    gx[gcount] = nx;
                    gy[gcount] = ny;
                    gcount++;
                    continue;
                }

                puddle_count = 0;
                for (yy = y - MAX_RADIUS; yy <= y + MAX_RADIUS; yy++)
                {
                    for (xx = x - MAX_RADIUS; xx <= x + MAX_RADIUS; xx++)
                    {
                        if (xx > 0 && xx < MAXX && yy > 0 && yy < MAXY)
                        {
                            if (item[xx][yy] == OWATER ||
                                item[xx][yy] == OSHOREWATER)
                                puddle_count++;
                        }
                    }
                }

                if (puddle_count > (MAX_RADIUS * MAX_RADIUS))
                    continue;

                gx[gcount] = nx;
                gy[gcount] = ny;
                gcount++;
            }

            if (gcount == 0)
                continue;

            pick = rnd(gcount) - 1;
            nx = gx[pick];
            ny = gy[pick];

            if (item[nx][ny] == OSHOREWATER)
            {
                item[nx][ny] = OWATER;
            }
            else if (item[x][y] == OWATER &&
                     (item[nx - 1][ny] != OWATER ||
                      item[nx + 1][ny] != OWATER ||
                      item[nx][ny - 1] != OWATER ||
                      item[nx][ny + 1] != OWATER))
            {
                item[nx][ny] = OSHOREWATER;
            }
            else
            {
                item[nx][ny] = OWATER;
            }

            iarg[nx][ny] = 0;
        }
    }
}

/*
* makepuddle(lvl)
*
* subroutine to make a puddle of a given size
*/
static void
makepuddle(int lvl)
{
    int puddle_width, puddle_height;
    int px, py;
    int x, y;
    int attempts;
    int suitable;

    if (lvl >= VOLCANOLEVEL_START && lvl <= VOLCANOLEVEL_END)
        return;

    puddle_width = 2 + rnd(7);
    puddle_height = 2 + rnd(5);

    for (attempts = 0; attempts < 10; attempts++)
    {
        px = rnd(MAXX - puddle_width - 2) + 1;
        py = rnd(MAXY - puddle_height - 2) + 1;

        suitable = 1;

        for (y = py; y < py + puddle_height; y++)
        {
            for (x = px; x < px + puddle_width; x++)
            {
                if (item[x][y] != 0 || mitem[x][y] != 0)
                {
                    suitable = 0;
                    break;
                }
            }
            if (!suitable)
                break;
        }

        if (suitable)
        {
            for (y = py; y < py + puddle_height; y++)
            {
                for (x = px; x < px + puddle_width; x++)
                {
                    if (x == px || x == px + puddle_width - 1 ||
                        y == py || y == py + puddle_height - 1)
                        item[x][y] = OSHOREWATER;
                    else
                        item[x][y] = OWATER;

                    iarg[x][y] = 0;
                }
            }
            break;
        }
    }
}

/*
*  function to read in a maze from a data file
*
*  Format of maze data file:  1st character = # of mazes in file (ascii digit)
*              For each maze: 18 lines (1st 17 used) 67 characters per line
*
*  Special characters in maze data file:
*
*      #   wall            D   door            .   random monster
*      ~   eye of larn     !   cure dianthroritis
*      -   random object
*/
static int
cannedlevel(int k)
{
    char *row;
    int i, j;
    int it, arg, mit, marg;
    int x, y;

    if (lopen(mazefile) < 0)
    {
        fprintf(stderr, "Can't open the maze data file\n");
        died(-282);
        return 0;
    }

    i = lgetc();
    if (i <= '0')
    {
        died(-282);
        return 0;
    }

    for (i = 18 * rund(i - '0'); i > 0; i--)
        lgetl();

    for (i = 0; i < MAXY; i++)
    {
        row = lgetl();
        for (j = 0; j < MAXX; j++)
        {
            it = 0;
            mit = 0;
            arg = 0;
            marg = 0;

            switch (*row++)
            {
            case '#': it = OINNERWALL; break;
            case 'D': it = OCLOSEDDOOR; arg = rnd(30); break;
            case '~':
                if (k == MAXLEVEL - 1)
                {
                    it = OLARNEYE;
                    mit = rund(8) + DEMONLORD;
                    marg = monster[mit].hitpoints;
                }
                break;
            case '!':
                if (k == MAXLEVEL + MAXVLEVEL - 1)
                {
                    it = OPOTION;
                    arg = 21;
                    mit = DEMONLORD + 7;
                    marg = monster[mit].hitpoints;
                }
                break;
            case '.':
                if (k >= MAXLEVEL)
                {
                    mit = makemonst(k + 1);
                    marg = monster[mit].hitpoints;
                }
                break;
            case '-':
                it = newobject(k + 1, &arg);
                break;
            }

            /* enforce boundary walls */
            for (x = 0; x < MAXX; x++)
            {
                item[x][0] = OWALL;
                item[x][MAXY - 1] = OWALL;
            }
            for (y = 0; y < MAXY; y++)
            {
                item[0][y] = OWALL;
                item[MAXX - 1][y] = OWALL;
            }

            if (k >= VOLCANOLEVEL_START && k <= VOLCANOLEVEL_END)
            {
                if (it == OWATER || it == OSHOREWATER)
                    it = 0;
            }

            item[j][i] = it;
            iarg[j][i] = arg;
            mitem[j][i] = mit;
            hitp[j][i] = marg;

#if WIZID
            know[j][i] = (wizard ? KNOWALL : 0);
#else
            know[j][i] = 0;
#endif
        }
    }

    lrclose();
    worldboundary();
    return 1;
}

/*
*  function to make a treasure room on a level
*  level 10's treasure room has the eye in it and demon lords
*  level V3 has potion of cure dianthroritis and demon prince
*/
static void
treasureroom(int lv)
{
    int tx, ty, xsize, ysize;

    for (tx = 1 + rnd(10); tx < MAXX - 10; tx += 10)
        if ((lv == MAXLEVEL - 1) || (lv == MAXLEVEL + MAXVLEVEL - 1)
            /*Increased this math to a 50 percent chance. -Gibbon */
            || rnd(10) == 5)
        {
            xsize = rnd(6) + 3;
            ysize = rnd(3) + 3;
            ty = rnd(MAXY - 9) + 1;	/* upper left corner of room */
            if (lv == MAXLEVEL - 1 || lv == MAXLEVEL + MAXVLEVEL - 1)
                troom(lv, xsize, ysize, tx =
                    tx + rnd(MAXX - 24), ty, rnd(3) + 6);
            else
                troom(lv, xsize, ysize, tx, ty, rnd(9));
        }
}



/*
*  subroutine to create a treasure room of any size at a given location
*  room is filled with objects and monsters
*  the coordinate given is that of the upper left corner of the room
*/
static void
troom(int lv, int xsize, int ysize, int tx, int ty, int glyph)
{
    int i, j;
    int tp1, tp2;

    for (j = ty - 1; j <= ty + ysize; j++)
        for (i = tx - 1; i <= tx + xsize; i++)	/* clear out space for room */
            item[i][j] = 0;
    for (j = ty; j < ty + ysize; j++)
        for (i = tx; i < tx + xsize; i++)	/* now put in the walls */
        {
            item[i][j] = OINNERWALL;
            mitem[i][j] = 0;
        }
    for (j = ty + 1; j < ty + ysize - 1; j++)
        for (i = tx + 1; i < tx + xsize - 1; i++)	/* now clear out interior */
            item[i][j] = 0;

    switch (rnd(2))		/* locate the door on the treasure room */
    {
    case 1:
        item[i = tx + rund(xsize)][j = ty + (ysize - 1) * rund(2)] =
            OCLOSEDDOOR;
        iarg[i][j] = glyph;	/* on horizontal walls */
        break;
    case 2:
        item[i = tx + (xsize - 1) * rund(2)][j = ty + rund(ysize)] =
            OCLOSEDDOOR;
        iarg[i][j] = glyph;	/* on vertical walls */
        break;
    };

    tp1 = playerx;
    tp2 = playery;
    playery = ty + (ysize >> 1);

    if (c[HARDGAME] < 2)
    {
        for (playerx = tx + 1; playerx <= tx + xsize - 2; playerx += 2)
        {
            for (i = 0, j = rnd(6); i <= j; i++)
            {
                something(lv + 2);
                createmonster(makemonst(lv + 1));
            }
        }
    }
    else
    {
        for (playerx = tx + 1; playerx <= tx + xsize - 2; playerx += 2)
        {
            for (i = 0, j = rnd(4); i <= j; i++)
            {
                something(lv + 2);
                createmonster(makemonst(lv + 3));
            }
        }
        playerx = tp1;
        playery = tp2;
    }
}

/*
* subroutine to create the objects in the maze for the given level
*/
static void
makeobject(int j)
{
    int i;

    if (j == 0)
    {

        /*  entrance to dungeon */
        fillroom(OENTRANCE, 0);

        /*  the DND STORE */
        fillroom(ODNDSTORE, 0);

        fillroom(OSCHOOL, 0);	/*  college of Larn             */
        fillroom(OBANK, 0);	/*  1st national bank of larn   */
        fillroom(OVOLDOWN, 0);	/*  volcano shaft to temple     */
        fillroom(OHOME, 0);	/*  the players home & family   */
        fillroom(OTRADEPOST, 0);	/*  the trading post            */
        fillroom(OLRS, 0);	/*  the larn revenue service    */

        return;
    }

    if (j == MAXLEVEL)
        fillroom(OVOLUP, 0);	/* volcano shaft up from the temple */

    /*  make the fixed objects in the maze STAIRS   */
    if ((j > 0) && (j != MAXLEVEL - 1) && (j != MAXLEVEL + MAXVLEVEL - 1))
        fillroom(OSTAIRSDOWN, 0);
    if ((j > 1) && (j != MAXLEVEL))
        fillroom(OSTAIRSUP, 0);

    /*  make the random objects in the maze     */

    fillmroom(rund(3), OBOOK, j);
    fillmroom(rund(3), OALTAR, 0);
    fillmroom(rund(3), OSTATUE, 0);
    fillmroom(rund(3), OPIT, 0);
    fillmroom(rund(3), OFOUNTAIN, 0);
    fillmroom(rnd(3) - 2, OIVTELETRAP, 0);
    fillmroom(rund(2), OTHRONE, 0);
    fillmroom(rund(2), OMIRROR, 0);
    fillmroom(rund(2), OTRAPARROWIV, 0);
    fillmroom(rnd(3) - 2, OIVDARTRAP, 0);
    fillmroom(rund(3), OCOOKIE, 0);
    if (j == 1)
        fillmroom(1, OCHEST, j);
    else
        fillmroom(rund(2), OCHEST, j);
    if ((j != MAXLEVEL - 1) && (j != MAXLEVEL + MAXVLEVEL - 1))
        fillmroom(rund(2), OIVTRAPDOOR, 0);
    if (j <= 10)
    {
        fillmroom((rund(2)), ODIAMOND, rnd(10 * j + 1) + 10);
        fillmroom(rund(2), ORUBY, rnd(6 * j + 1) + 6);
        fillmroom(rund(2), OEMERALD, rnd(4 * j + 1) + 4);
        fillmroom(rund(2), OSAPPHIRE, rnd(3 * j + 1) + 2);
    }
    for (i = 0; i < rnd(4) + 3; i++)
        fillroom(OPOTION, newpotion());	/*  make a POTION   */
    for (i = 0; i < rnd(5) + 3; i++)
        fillroom(OSCROLL, newscroll());	/*  make a SCROLL   */
    for (i = 0; i < rnd(12) + 11; i++)
        fillroom(OGOLDPILE, 12 * rnd(j + 1) + (j << 3) + 10);	/* make GOLD */
    if (j == 5)
        fillroom(OBANK2, 0);	/* branch office of the bank */
    froom(2, ORING, 0);		/* a ring mail          */
    froom(1, OSTUDLEATHER, 0);	/* a studded leather    */
    froom(3, OSPLINT, 0);	/* a splint mail        */
    froom(5, OSHIELD, rund(3));	/* a shield             */
    froom(2, OBATTLEAXE, rund(3));	/* a battle axe         */
    froom(5, OLONGSWORD, rund(3));	/* a long sword         */
    froom(4, OREGENRING, rund(3));	/* ring of regeneration */
    froom(1, OPROTRING, rund(3));	/* ring of protection   */
    froom(2, OSTRRING, 1 + rnd(3));	/* ring of strength     */
    froom(7, OSPEAR, rnd(5));	/* a spear              */
    froom(3, OORBOFDRAGON, 0);	/* orb of dragon slaying */
    froom(4, OSPIRITSCARAB, 0);	/* scarab of negate spirit */
    froom(4, OCUBEofUNDEAD, 0);	/* cube of undead control   */
    froom(2, ORINGOFEXTRA, 0);	/* ring of extra regen      */
    froom(3, ONOTHEFT, 0);	/* device of antitheft      */
    froom(2, OSWORDofSLASHING, 0);	/* sword of slashing */
    if (c[BESSMANN] == 0)
    {
        froom(4, OHAMMER, 0);	/*Bessman's flailing hammer */
        c[BESSMANN] = 1;
    }
    if (c[HARDGAME] < 3 || (rnd(4) == 3))
    {
        if (j > 3)
        {
            froom(3, OSWORD, 3);	/* sunsword + 3         */
            froom(5, O2SWORD, rnd(4));	/* a two handed sword */
            froom(5, OHSWORD, rnd(4));	/* a longsword of slashing */
            froom(3, OBELT, 4);	/* belt of striking     */
            froom(3, OENERGYRING, 3);	/* energy ring          */
            froom(4, OPLATE, 5);	/* platemail + 5        */
            froom(3, OCLEVERRING, 1 + rnd(2));	/* ring of cleverness */
        }
    }

    /* Generate puddles and streams of water  but not in the volcano */
    if (j > 0 && !(j >= VOLCANOLEVEL_START && j <= VOLCANOLEVEL_END))
    {
        int num_puddles = rnd(3) + 1;
        int num_streams = rnd(2);

        for (i = 0; i < num_puddles; i++)
            makepuddle(j);

        for (i = 0; i < num_streams; i++)
            makestream(j);
    }
}

/*
*  subroutine to fill in a number of objects of the same kind
*/
static void
fillmroom(int n, int what, int arg)
{
    int i;

    for (i = 0; i < n; i++)
    {

        fillroom(what, arg);
    }
}



static void
froom(int n, int itm, int arg)
{

    if (rnd(151) < n)
    {

        fillroom(itm, arg);
    }
}


/*
* subroutine to put an object into an empty room
* uses a random walk
*/
static void
fillroom(int what, int arg)
{
    int x, y;

#ifdef EXTRA
    c[FILLROOM]++;
#endif

    x = rnd(MAXX - 2);
    y = rnd(MAXY - 2);

    while (item[x][y])
    {

#ifdef EXTRA
        /* count up these random walks */
        c[RANDOMWALK]++;
#endif

        x += rnd(3) - 2;
        y += rnd(3) - 2;

        /* clamp location to within map border */
        if (x > MAXX - 2)
            x = 1;
        if (x < 1)
            x = MAXX - 2;
        if (y > MAXY - 2)
            y = 1;
        if (y < 1)
            y = MAXY - 2;
    }

    item[x][y] = what;
    iarg[x][y] = arg;
}




/*
subroutine to put monsters into an empty room without walls or other
monsters
*/
int
fillmonst(int what)
{
    int x, y, trys;

    for (trys = 5; trys > 0; --trys)	/* max # of creation attempts */
    {
        x = rnd(MAXX - 2);
        y = rnd(MAXY - 2);
        if ((item[x][y] == 0) && (mitem[x][y] == 0)
            && ((playerx != x) || (playery != y)))
        {
            mitem[x][y] = what;
            know[x][y] &= ~KNOWHERE;
            hitp[x][y] = monster[what].hitpoints;
            return (0);
        }
    }

    return -1;			/* creation failure */
}



/*
*  creates an entire set of monsters for a level
*  must be done when entering a new level
*  if sethp(1) then wipe out old monsters else leave them there
*/
static void
sethp(int flg)
{
    int i, j;

    if (flg)
    {

        for (i = 0; i < MAXY; i++)
        {
            for (j = 0; j < MAXX; j++)
            {

                stealth[j][i] = 0;
            }
        }
    }

    /* if teleported and found level 1 then know level we are on */
    if (level == 0)
    {

        c[TELEFLAG] = 0;

        return;
    }

    if (flg)
    {

        j = rnd(12) + 2 + (level >> 1);

    }
    else
    {

        j = (level >> 1) + 1;
    }

    for (i = 0; i < j; i++)
    {

        fillmonst(makemonst(level));
    }
}



/*
*  Function to destroy all genocided monsters on the present level
*/
static void
checkgen(void)
{
    int x, y;

    for (y = 0; y < MAXY; y++)
    {
        for (x = 0; x < MAXX; x++)
        {

            if (monster[mitem[x][y]].genocided)
            {

                /* no more monster */
                mitem[x][y] = 0;
            }
        }
    }
}
