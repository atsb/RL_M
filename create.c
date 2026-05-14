/* create.c */

// #define PUDDLE_TEST // Uncomment or define via compiler flags to enable debug puddle

#include "includes/create.h"
#include "includes/larn.h"
#include "includes/global.h"
#include "includes/io.h"
#include "includes/monster.h"
#include "includes/savelev.h"
#include "includes/scores.h"

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

    regen_bottom = TRUE;

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
    int i, j;

    if (beenhere[level])
        savelevel();		/* put the level back into storage  */
    level = x;			/* get the new level and put in working storage */
    if (beenhere[x])
    {
        getlevel();
        sethp(0);
        positionplayer();
        checkgen();
        return;
    }

    /* fill in new level
     */
    for (i = 0; i < MAXY; i++)
        for (j = 0; j < MAXX; j++)
            know[j][i] = mitem[j][i] = 0;
    makemaze(x);
    makeobject(x);
    beenhere[x] = 1;
    sethp(1);
    positionplayer();
    checkgen();			/* wipe out any genocided monsters */

#if WIZID
    if (wizard || x == 0)
#else
    if (x == 0)
#endif
        for (j = 0; j < MAXY; j++)
            for (i = 0; i < MAXX; i++)
                know[i][j] = KNOWALL;
}

/*
makecorridor_caverns(int x1, int y1, int x2, int y2)

subroutine to make the caverns corridoor connections.
*/
static void
makecorridor_caverns(int x1, int y1, int x2, int y2)
{
    int x = x1, y = y1;
    int steps = 0;

    while ((x != x2 || y != y2) && steps < 500) {
        if (x > 0 && x < MAXX - 1 && y > 0 && y < MAXY - 1)
            item[x][y] = 0;

        if (rnd(100) < 60) {
            if (x < x2) x++;
            else if (x > x2) x--;
            else if (y < y2) y++;
            else if (y > y2) y--;
        }
        else {
            if (y < y2) y++;
            else if (y > y2) y--;
            else if (x < x2) x++;
            else if (x > x2) x--;
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

    /* SEA OF WALL!!! */
    for (i = 0; i < MAXY; i++)
        for (j = 0; j < MAXX; j++)
            item[j][i] = OWALL;

    /* solid outer boundary */
    for (i = 0; i < MAXY; i++) {
        item[0][i] = OWALL;
        item[MAXX - 1][i] = OWALL;
    }
    for (j = 0; j < MAXX; j++) {
        item[j][0] = OWALL;
        item[j][MAXY - 1] = OWALL;
    }

    /* 3x3 grid */
    int cell_w = (MAXX - 2) / 3;
    int cell_h = (MAXY - 2) / 3;
    if (cell_w < 6) cell_w = 6;
    if (cell_h < 6) cell_h = 6;

    int rx[3][3], ry[3][3], rw[3][3], rh[3][3];
    int gone[3][3];

    /* gone rooms */
    for (int gy = 0; gy < 3; gy++)
        for (int gx = 0; gx < 3; gx++)
            gone[gx][gy] = (rnd(4) == 0);

    /* carve rooms */
    for (int gy = 0; gy < 3; gy++) {
        for (int gx = 0; gx < 3; gx++) {

            if (gone[gx][gy]) {
                rw[gx][gy] = rh[gx][gy] = 0;
                continue;
            }

            int max_w = cell_w - 2;
            int max_h = cell_h - 2;
            if (max_w < 4) max_w = 4;
            if (max_h < 4) max_h = 4;

            rw[gx][gy] = rnd(max_w - 3) + 4;
            rh[gx][gy] = rnd(max_h - 3) + 4;

            int cell_x = 1 + gx * cell_w;
            int cell_y = 1 + gy * cell_h;

            int off_x_max = cell_w - rw[gx][gy];
            int off_y_max = cell_h - rh[gx][gy];
            if (off_x_max < 1) off_x_max = 1;
            if (off_y_max < 1) off_y_max = 1;

            rx[gx][gy] = cell_x + rnd(off_x_max);
            ry[gx][gy] = cell_y + rnd(off_y_max);

            /* carve interior */
            for (i = 0; i < rh[gx][gy]; i++)
                for (j = 0; j < rw[gx][gy]; j++) {
                    int x = rx[gx][gy] + j;
                    int y = ry[gx][gy] + i;
                    if (x > 0 && x < MAXX - 1 && y > 0 && y < MAXY - 1)
                        item[x][y] = 0;
                }
        }
    }

    /* horizontal connections */
    for (int gy = 0; gy < 3; gy++) {
        for (int gx = 0; gx < 2; gx++) {
            if (rw[gx][gy] && rw[gx + 1][gy]) {
                int x1 = rx[gx][gy] + rw[gx][gy] / 2;
                int y1 = ry[gx][gy] + rh[gx][gy] / 2;
                int x2 = rx[gx + 1][gy] + rw[gx + 1][gy] / 2;
                int y2 = ry[gx + 1][gy] + rh[gx + 1][gy] / 2;
                makecorridor_caverns(x1, y1, x2, y2);
            }
        }
    }

    /* vertical connections */
    for (int gx = 0; gx < 3; gx++) {
        for (int gy = 0; gy < 2; gy++) {
            if (rh[gx][gy] && rh[gx][gy + 1]) {
                int x1 = rx[gx][gy] + rw[gx][gy] / 2;
                int y1 = ry[gx][gy] + rh[gx][gy] / 2;
                int x2 = rx[gx][gy + 1] + rw[gx][gy + 1] / 2;
                int y2 = ry[gx][gy + 1] + rh[gx][gy + 1] / 2;
                makecorridor_caverns(x1, y1, x2, y2);
            }
        }
    }

    /* diagonal connections */
    for (int gy = 0; gy < 2; gy++) {
        for (int gx = 0; gx < 2; gx++) {
            if (rw[gx][gy] && rw[gx + 1][gy + 1] && rnd(100) < 40) {
                int x1 = rx[gx][gy] + rw[gx][gy] / 2;
                int y1 = ry[gx][gy] + rh[gx][gy] / 2;
                int x2 = rx[gx + 1][gy + 1] + rw[gx + 1][gy + 1] / 2;
                int y2 = ry[gx + 1][gy + 1] + rh[gx + 1][gy + 1] / 2;
                makecorridor_caverns(x1, y1, x2, y2);
            }
        }
    }

    if (k == 1)
    {
        item[33][MAXY - 1] = OENTRANCE;
    }
}

/*
makemaze(level)
int level;

subroutine to make the caverns for a given level.  only walls are made.
*/
static int mx, mxl, mxh, my, myl, myh, tmp2;

static void
makemaze(int k)
{
    int i, j, tmp;
    int z;

    /* ~30% chance to generate a caverns style maze */
    if (k > 0 && k != MAXLEVEL - 1 && rnd(100) < 30) {
        makemaze_caverns(k);
        return;
    }

    if (k > 1
        && (rnd(17) <= 4 || k == MAXLEVEL - 1
            || k == MAXLEVEL + MAXVLEVEL - 1))
    {
        /* read maze from data file */
        if (cannedlevel(k))
        {
            return;
        }
    }

    if (k == 0)
    {

        tmp = 0;

    }
    else
    {

        tmp = OWALL;
    }

    for (i = 0; i < MAXY; i++)
    {
        for (j = 0; j < MAXX; j++)
        {

            item[j][i] = tmp;
        }
    }

    if (k == 0)
    {
        return;
    }

    eat(1, 1);

    if (k == 1)
    {

        item[33][MAXY - 1] = OENTRANCE;
    }

    /*  now for open spaces -- not on level 10  */
    if (k != MAXLEVEL - 1)
    {
        tmp2 = rnd(3) + 3;
        for (tmp = 0; tmp < tmp2; tmp++)
        {
            my = rnd(11) + 2;
            myl = my - rnd(2);
            myh = my + rnd(2);
            if (k < MAXLEVEL)
            {
                mx = rnd(44) + 5;
                mxl = mx - rnd(4);
                mxh = mx + rnd(12) + 3;
                z = 0;
            }
            else
            {
                mx = rnd(60) + 3;
                mxl = mx - rnd(2);
                mxh = mx + rnd(2);
                z = makemonst(k);
            }
            for (i = mxl; i < mxh; i++)
                for (j = myl; j < myh; j++)
                {
                    item[i][j] = 0;

                    mitem[i][j] = z;
                    if (mitem[i][j] != 0)
                    {
                        hitp[i][j] = monster[z].hitpoints;
                    }
                }
        }
    }
    if (k != MAXLEVEL - 1)
    {
        my = rnd(MAXY - 2);
        for (i = 1; i < MAXX - 1; i++)
            item[i][my] = 0;
    }
    if (k > 1)
        treasureroom(k);
}



/*
* function to eat away a filled in maze
*/
void
eat(int xx, int yy)
{
    int dir, try;

    dir = rnd(4);

    try = 2;

    while (try)
    {
        switch (dir)
        {
        case 1:
            if (xx <= 2)
                break;		/*  west    */
            if ((item[xx - 1][yy] != OWALL) || (item[xx - 2][yy] != OWALL))
                break;
            item[xx - 1][yy] = item[xx - 2][yy] = 0;
            eat(xx - 2, yy);
            break;

        case 2:
            if (xx >= MAXX - 3)
                break;		/*  east    */
            if ((item[xx + 1][yy] != OWALL) || (item[xx + 2][yy] != OWALL))
                break;
            item[xx + 1][yy] = item[xx + 2][yy] = 0;
            eat(xx + 2, yy);
            break;

        case 3:
            if (yy <= 2)
                break;		/*  south   */
            if ((item[xx][yy - 1] != OWALL) || (item[xx][yy - 2] != OWALL))
                break;
            item[xx][yy - 1] = item[xx][yy - 2] = 0;
            eat(xx, yy - 2);
            break;

        case 4:
            if (yy >= MAXY - 3)
                break;		/*  north   */
            if ((item[xx][yy + 1] != OWALL) || (item[xx][yy + 2] != OWALL))
                break;
            item[xx][yy + 1] = item[xx][yy + 2] = 0;
            eat(xx, yy + 2);
            break;
        };
        if (++dir > 4)
        {
            dir = 1;
            --try;
        }
    }
}

/*
* makestream(level)
*
* subroutine to make a long winding horizontal river
* or a more subdued stream.
*/
static void
makestream(int level)
{
    int x, y, dx, dy, len, i;
    int width = 2;

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
                int wiggle = (rnd(2) ? 1 : -1);
                if (y + wiggle > 1 && y + wiggle < MAXY - 2)
                    y += wiggle;
            }

            /* create river */
            for (i = 0; i < width; i++)
            {
                int sx = x;
                int sy = y + i;

                if (sx < 1 || sx >= MAXX - 1 || sy < 1 || sy >= MAXY - 1)
                    continue;

                if (mitem[sx][sy] != 0)
                    continue;

                /* avoid destroying structures */
                switch (item[sx][sy])
                {
                case OENTRANCE:
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
    int max_len = 10 + rnd(25);

    /* random starting point */
    x = rnd(MAXX - 4) + 2;
    y = rnd(MAXY - 4) + 2;

    /* randomised dominancy*/
    int dominant = rnd(2);

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
            /* this may look dead but has a
            chance to prevent the wiggle,
            so it goes straight */
        }
        /* 20% chance to wiggle */
        else if (rnd(10) < 9) {
            if (dominant == 0) {
                /* horizontal stream up/down */
                dy = (rnd(2) ? 1 : -1);
                dx = (rnd(2) ? 1 : -1); /* diagonal */
            }
            else {
                /* vertical stream left/right */
                dx = (rnd(2) ? 1 : -1);
                dy = (rnd(2) ? 1 : -1);
            }
        }
        /* 10% chance for 90 degrees turn */
        else {
            int t = dx;
            dx = -dy;
            dy = t;
        }

        x += dx;
        y += dy;

        /* boundary check analysis */
        if (x < 1 || x >= MAXX - 2 || y < 1 || y >= MAXY - 2)
            break;

        /* create stream */
        for (i = 0; i < width; i++)
        {
            int sx = x;
            int sy = y;

            if (dx != 0) sy += i;  /* horizontal */
            else
                sx += i; /* vertical */

            if (sx < 1 || sx >= MAXX - 1 || sy < 1 || sy >= MAXY - 1)
                continue;

            if (mitem[sx][sy] != 0)
                continue;

            /* avoid overwriting structures */
            switch (item[sx][sy])
            {
            case OENTRANCE:
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
    const int MAX_RADIUS = 4;

    /* Chance per move that a puddle will grow */
    if (rnd(6) != 1) return;

    for (int y = 1; y < MAXY - 1; y++)
    {
        for (int x = 1; x < MAXX - 1; x++)
        {
            if (item[x][y] != OWATER)
                continue;

            /* valid growth directions */
            int gx[4], gy[4], gcount = 0;

            static const int dx[4] = { 0, 0, -1, 1 };
            static const int dy[4] = { -1, 1, 0, 0 };

            for (int d = 0; d < 4; d++)
            {
                int nx = x + dx[d];
                int ny = y + dy[d];

                /* bounds check */
                if (nx < 1 || nx >= MAXX - 1 || ny < 1 || ny >= MAXY - 1)
                    continue;

                /* do NOT overwrite walls */
                if (item[nx][ny] == OWALL)
                    continue;

                /* do NOT overwrite monsters */
                if (mitem[nx][ny] != 0)
                    continue;

                /* do NOT overwrite special structures */
                switch (item[nx][ny])
                {
                case OENTRANCE:
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

                /* maximum radius */
                int puddle_count = 0;
                for (int yy = y - MAX_RADIUS; yy <= y + MAX_RADIUS; yy++)
                    for (int xx = x - MAX_RADIUS; xx <= x + MAX_RADIUS; xx++)
                        if (xx > 0 && xx < MAXX && yy > 0 && yy < MAXY)
                            if (item[xx][yy] == OWATER)
                                puddle_count++;

                if (puddle_count > (MAX_RADIUS * MAX_RADIUS))
                    continue;

                /* growth direction */
                gx[gcount] = nx;
                gy[gcount] = ny;
                gcount++;
            }

            /* if no valid directions then puddle is blocked */
            if (gcount == 0)
                continue;

            /* valid direction at random */
            int pick = rnd(gcount) - 1;
            int nx = gx[pick];
            int ny = gy[pick];

            /* grow puddle */
            /* if touching non-water, it's a shore tile */
            if (item[x][y] == OWATER &&
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
* makepuddle(level)
*
* subroutine to make a puddle of a given size
*/
static void
makepuddle(int level)
{
    int puddle_width, puddle_height, px, py, x, y, attempts;
    _Bool suitable;

    puddle_width = 2 + rnd(7);  /* 2 to 8 */
    puddle_height = 2 + rnd(5); /* 2 to 6 */

    for (attempts = 0; attempts < 10; attempts++)
    {
        px = rnd(MAXX - puddle_width - 2) + 1; /* ensure px + puddle_width < MAXX - 1 */
        py = rnd(MAXY - puddle_height - 2) + 1; /* ensure py + puddle_height < MAXY - 1 */

        suitable = 1; /* true */
        for (y = py; y < py + puddle_height; y++)
        {
            for (x = px; x < px + puddle_width; x++)
            {
                if (item[x][y] != 0 || mitem[x][y] != 0)
                {
                    suitable = 0; /* false */
                    break;
                }
            }
            if (!suitable)
            {
                break;
            }
        }

        if (suitable)
        {
            for (y = py; y < py + puddle_height; y++)
            {
                for (x = px; x < px + puddle_width; x++)
                {
                    if (x == px || x == px + puddle_width - 1 ||
                        y == py || y == py + puddle_height - 1)
                    {
                        item[x][y] = OSHOREWATER;
                    }
                    else
                    {
                        item[x][y] = OWATER;
                    }
                    iarg[x][y] = 0;
                }
            }
            break; /* Puddle created, exit attempts loop */
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
    char* row;
    int i, j;
    int it, arg, mit, marg;

    if (lopen(mazefile) < 0)
    {
        fprintf(stderr, "Can't open the maze data file\n");
        died(-282);
        return (0);
    }
    i = lgetc();
    if (i <= '0')
    {
        died(-282);
        return (0);
    }
    for (i = 18 * rund(i - '0'); i > 0; i--)
        lgetl();			/* advance to desired maze */
    for (i = 0; i < MAXY; i++)
    {
        row = lgetl();
        for (j = 0; j < MAXX; j++)
        {
            it = mit = arg = marg = 0;
            switch (*row++)
            {
            case '#':
                it = OWALL;
                break;
            case 'D':
                it = OCLOSEDDOOR;
                arg = rnd(30);
                break;
            case '~':
                if (k != MAXLEVEL - 1)
                    break;
                it = OLARNEYE;
                mit = rund(8) + DEMONLORD;
                marg = monster[mit].hitpoints;
                break;
            case '!':
                if (k != MAXLEVEL + MAXVLEVEL - 1)
                    break;
                it = OPOTION;
                arg = 21;
                mit = DEMONLORD + 7;
                marg = monster[mit].hitpoints;
                break;
            case '.':
                if (k < MAXLEVEL)
                    break;
                mit = makemonst(k + 1);
                marg = monster[mit].hitpoints;
                break;
            case '-':
                it = newobject(k + 1, &arg);
                break;
            };
            item[j][i] = it;
            iarg[j][i] = arg;
            mitem[j][i] = mit;
            hitp[j][i] = marg;

#if WIZID
            know[j][i] = (wizard) ? KNOWALL : 0;
#else
            know[j][i] = 0;
#endif
        }
    }
    lrclose();
    return (1);
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
            item[i][j] = OWALL;
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

    // Generate puddles of water
    if (j > 0) // Not in town
    {
        // And perhaps not on special levels if desired, e.g.
        // if (j > 0 && j != MAXLEVEL -1 && j != MAXLEVEL + MAXVLEVEL - 1)
        int num_puddles = rnd(3) + 1; // Generate 1 to 3 puddles
        int num_streams = rnd(2); // Generate 1 to 2 streams
        int i;
        for (i = 0; i < num_puddles; i++)
        {
            makepuddle(j);
        }
        for (i = 0; i < num_streams; i++)
        {
            makestream(j);
        }
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
