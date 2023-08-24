/**
*  @file   tetris.h
*  @brief  Terminal-based Tetris video game
*
*  @author Mikhail Zaytsev
*  @date   20230825
*/

/** Includes */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>

/** Definitions */

#define CONFIG_KEY_MOVE_LEFT    'h'
#define CONFIG_KEY_MOVE_RIGHT   'l'
#define CONFIG_KEY_ROTATE_LEFT  'u'
#define CONFIG_KEY_ROTATE_RIGHT 'i'
#define CONFIG_KEY_FALL         'j'

#define CUP_HEIGHT          20
#define CUP_WIDTH           10

#define GAME_FPS            30
#define GAME_FALL_SPEED     7
#define GAME_SECOND_IN_MS   1000

#define TILE_SPACE      "  "
#define TILE_FILLED     "[]"

#define FIGURE_SIZE         4
#define FIGURE_QTY          7

#define PRINTW_COLOR(color, ...) \
    do \
    { \
        attron(COLOR_PAIR(color)); \
        printw(__VA_ARGS__); \
        attroff(COLOR_PAIR(color)); \
    } while (0)

#define PRINTW_COLOR_BOLD(color, ...) \
    do \
    { \
        attron(A_BOLD); \
        PRINTW_COLOR(color, __VA_ARGS__); \
        attroff(A_BOLD); \
    } while (0)

#define TILE_IS_FILLED(tile) \
    ((TILE_NUM_SPACE != (tile)) && \
    (TILE_NUM_SHADOW != (tile)))

#define POINT_IS_IN_CUP(point) \
    ((0 > (point).x) || \
    (0 > (point).y) || \
    ((CUP_WIDTH - 1) < (point).x) || \
    ((CUP_HEIGHT - 1) < (point).y)) 

/** Structures and types */

typedef struct point_s
{
    int x;
    int y;
} point_s;

typedef enum tile_num_e
{
    TILE_NUM_SPACE = 0,

    TILE_NUM_SQUARE,
    TILE_NUM_STICK,
    TILE_NUM_S,
    TILE_NUM_Z,
    TILE_NUM_L,
    TILE_NUM_J,
    TILE_NUM_T,

    TILE_NUM_SHADOW
} tile_num_e;

typedef struct figure_s
{
    point_s points[FIGURE_SIZE];
    tile_num_e num;
} figure_s;

typedef struct game_ctx_s
{
    figure_s figure;
    tile_num_e next;
    point_s offset;
    int score;
    char * p_cup[CUP_HEIGHT];
} game_ctx_s;


/** Private data */

static figure_s g_figure_list[FIGURE_QTY] =
{
    /* Square */
    (figure_s){(point_s){.x = 0, .y = 0}, (point_s){.x = 0, .y = 1}, 
                (point_s){.x = 1, .y = 0}, (point_s){.x = 1, .y = 1}, 
                .num = TILE_NUM_SQUARE},
    /* Stick */
    (figure_s){(point_s){.x = -1, .y = 0}, (point_s){.x = 0, .y = 0}, 
                (point_s){.x = 1, .y = 0}, (point_s){.x = 2, .y = 0},
                .num = TILE_NUM_STICK},
    /* S */
    (figure_s){(point_s){.x = -1, .y = -1}, (point_s){.x = 0, .y = -1}, 
                (point_s){.x = 0, .y = 0}, (point_s){.x = 1, .y = 0},
                .num = TILE_NUM_S},
    /* Z */
    (figure_s){(point_s){.x = -1, .y = 0}, (point_s){.x = 0, .y = 0}, 
                (point_s){.x = 0, .y = -1}, (point_s){.x = 1, .y = -1},
                .num = TILE_NUM_Z},
    /* L */
    (figure_s){(point_s){.x = 0, .y = 1}, (point_s){.x = 0, .y = 0}, 
                (point_s){.x = 0, .y = -1}, (point_s){.x = 1, .y = -1},
                .num = TILE_NUM_L},
    /* J */
    (figure_s){(point_s){.x = 0, .y = 1}, (point_s){.x = 0, .y = 0}, 
                (point_s){.x = 0, .y = -1}, (point_s){.x = -1, .y = -1},
                .num = TILE_NUM_J},
    /* T */
    (figure_s){(point_s){.x = -1, .y = 0}, (point_s){.x = 0, .y = 0}, 
                (point_s){.x = 1, .y = 0}, (point_s){.x = 0, .y = 1},
                .num = TILE_NUM_T},
};

static game_ctx_s g_ctx;

/** Private functions prototypes */

static void game_start(void);
static void game_loop(void);
static bool game_is_over(void);
static void game_over(void);

static void window_create(void);
static void window_refresh(void);
static void window_destroy(void);

static void cup_create(void);
static void cup_destroy(void);
static void cup_legend(void);
static void cup_bottom(void);
static void cup_process(void);
static bool cup_tile_process(int x, int y);
static void cup_line_delete(int num);
static bool cup_is_game_over(void);

static bool point_coll_check(const point_s * p_point);
static void point_print(const point_s * p_point, char ch);

static void figure_spawn(void);
static void figure_off_add(const point_s * p_offset);
static bool figure_coll_check(const point_s * p_point);
static bool figure_cup_check(void);

static void figure_operate(char key);
static bool figure_fall(void);
static bool figure_move_left(void);
static bool figure_move_right(void);
static bool figure_rotate_left(void);
static bool figure_rotate_right(void);

static void figure_print(void);
static void figure_shadow_print(void);

/** Public functions */

/**
*  @brief      Game entry point
*
*  @return     int   Error code
*/
int main(void)
{
    game_start();
    while (false == game_is_over())
    {
        game_loop();
    }
    game_over();

    return 0;
}

/** Private functions */

static void game_start(void)
{
    window_create();
    cup_create();
    figure_spawn();
}

static void game_loop(void)
{
    static int frame = 0;
    static bool is_fallen = false;
    figure_operate(getch());

    if (true == is_fallen)
    {
        is_fallen = false;
        figure_print();
        figure_spawn();
    }
    figure_shadow_print();
    cup_process();
    window_refresh();
    timeout(GAME_SECOND_IN_MS / GAME_FPS);

    if (0 == (frame % GAME_FALL_SPEED))
    {
        is_fallen = !figure_fall();
    }
    frame++;
}

static bool game_is_over(void)
{
    return cup_is_game_over();
}

static void game_over(void)
{
    cup_destroy();
    erase();
    attron(A_BOLD);
    mvprintw(0, 0, "\nGame over!\n");
    printw("Your score: %d\n", g_ctx.score);
    attroff(A_BOLD);

    window_refresh();
    getchar();
    window_destroy();
}

static void window_create(void)
{
    WINDOW * p_window = initscr();
    cbreak();
    noecho();
    nodelay(p_window, TRUE);
    if (true == has_colors())
    {
       start_color();
       init_pair(TILE_NUM_SQUARE, COLOR_YELLOW, COLOR_BLACK);
       init_pair(TILE_NUM_STICK, COLOR_CYAN, COLOR_BLACK);
       init_pair(TILE_NUM_S, COLOR_GREEN, COLOR_BLACK);
       init_pair(TILE_NUM_Z, COLOR_RED, COLOR_BLACK);
       init_pair(TILE_NUM_L, COLOR_YELLOW, COLOR_BLACK);
       init_pair(TILE_NUM_J, COLOR_BLUE, COLOR_BLACK);
       init_pair(TILE_NUM_T, COLOR_MAGENTA, COLOR_BLACK);
       init_pair(TILE_NUM_SHADOW, COLOR_WHITE, COLOR_BLACK);
    }
}

static void window_refresh(void)
{
    refresh();
}

static void window_destroy(void)
{
    endwin();
}

static void cup_create(void)
{
    for (int h_idx = 0; h_idx < CUP_HEIGHT; h_idx++) 
    {
        g_ctx.p_cup[h_idx] = malloc(sizeof(char) * CUP_WIDTH);
        memset(g_ctx.p_cup[h_idx], TILE_NUM_SPACE, sizeof(char) * CUP_WIDTH);
    }
}

static void cup_destroy(void)
{
    for (int h_idx = 0; h_idx < CUP_HEIGHT; h_idx++) 
    {
        free(g_ctx.p_cup[h_idx]);
    }
}

static void cup_legend(void)
{
    attron(A_BOLD);
    mvprintw(0, 0, "\nScore: %d\n", g_ctx.score);
    printw("Next: %d\n\n", g_ctx.next);
    attroff(A_BOLD);
}

static void cup_bottom(void)
{
    attron(A_BOLD);
    printw("+====================+\n");
    attroff(A_BOLD);
}

static void cup_process(void)
{
    int lines = 0;

    cup_legend();
    cup_bottom();
    for (int h_idx = 0; h_idx < CUP_HEIGHT; h_idx++) 
    {
        bool is_line = true;
        for (int w_idx = 0; w_idx < CUP_WIDTH; w_idx++) 
        {
            if (false == cup_tile_process(w_idx, h_idx))
            {
                is_line = false;
            }
        }
        if (true == is_line)
        {
            cup_line_delete(h_idx);
            lines++;
        }
        printw("\n");
    }
    cup_bottom();
    g_ctx.score += lines * lines;
}

static bool cup_tile_process(int x, int y)
{
    /** NOTE: It is not a bug */
    const point_s tmp = {.x = x, .y = y};
    if (true == figure_coll_check(&tmp))
    {
        PRINTW_COLOR_BOLD(g_ctx.figure.num, "[]");
        return false;
    }
    else
    {
        switch (g_ctx.p_cup[y][x])
        {
            case TILE_NUM_SHADOW:
                PRINTW_COLOR(0, TILE_FILLED);
                g_ctx.p_cup[y][x] = TILE_NUM_SPACE;
                return false;

            case TILE_NUM_SPACE:
                printw(TILE_SPACE);
                return false;

            default:
                PRINTW_COLOR_BOLD(g_ctx.p_cup[y][x], TILE_FILLED);
            break;
        }
    }
    return true;
}

static void cup_line_delete(int num)
{
    if ((0 > num) ||
        ((CUP_HEIGHT - 1) < num))
    {
        return;
    }

    char * p_tmp = g_ctx.p_cup[num];
    for (int idx = num; idx > 0; idx--)
    {
        g_ctx.p_cup[idx] = g_ctx.p_cup[idx - 1];
    }
    free(p_tmp);

    g_ctx.p_cup[0] = malloc(sizeof(char) * CUP_WIDTH);
    memset(g_ctx.p_cup[0], TILE_NUM_SPACE, sizeof(char) * CUP_WIDTH);
}

static bool cup_is_game_over(void)
{
    for (int w_idx = 0; w_idx < CUP_WIDTH; w_idx++) 
    {
        if (TILE_IS_FILLED(g_ctx.p_cup[0][w_idx]))
        {
            return true;
        }
    }
    return false;
}

static bool point_coll_check(const point_s * p_point)
{
    if (NULL == p_point)
    {
        return false;
    }

    return (POINT_IS_IN_CUP(*p_point) ||
        (TILE_IS_FILLED(g_ctx.p_cup[p_point->y][p_point->x])));
}

static void point_print(const point_s * p_point, char ch)
{
    if (NULL == p_point)
    {
        return;
    }

    g_ctx.p_cup[p_point->y][p_point->x] = ch;
}

static void figure_spawn(void)
{
    g_ctx.offset = (point_s){.x = 0, .y = 0};
    const point_s offset = {.x = CUP_WIDTH / 2, .y = 1};
    g_ctx.figure = g_figure_list[g_ctx.next];
    g_ctx.next = arc4random() % FIGURE_QTY;
    figure_off_add(&offset);
}

static void figure_off_add(const point_s * p_offset)
{
    if (NULL == p_offset)
    {
        return;
    }

    g_ctx.offset.x += p_offset->x;
    g_ctx.offset.y += p_offset->y;
}

static bool figure_coll_check(const point_s * p_point)
{
    if (NULL == p_point)
    {
        return false;
    }

    for (int idx = 0; idx < FIGURE_SIZE; idx++)
    {
        if ((p_point->x == (g_ctx.figure.points[idx].x + g_ctx.offset.x)) && 
            (p_point->y == (g_ctx.figure.points[idx].y + g_ctx.offset.y)))
        {
            return true;
        }
    }
    return false;
}

static bool figure_cup_check(void)
{
    for (int idx = 0; idx < FIGURE_SIZE; idx++)
    {
        const point_s tmp_point = 
        {
            .x = g_ctx.figure.points[idx].x + g_ctx.offset.x,
            .y = g_ctx.figure.points[idx].y + g_ctx.offset.y
        };
        
        if (true == point_coll_check(&tmp_point))
        {
            return true;
        }
    }
    return false;
}

static void figure_operate(char key)
{
    switch (key)
    {
        case CONFIG_KEY_FALL:
            while (true == figure_fall());
        break;

        case CONFIG_KEY_MOVE_LEFT:
            figure_move_left();
        break;

        case CONFIG_KEY_MOVE_RIGHT:
            figure_move_right();
        break;

        case CONFIG_KEY_ROTATE_LEFT:
            figure_rotate_left();
        break;

        case CONFIG_KEY_ROTATE_RIGHT:
            figure_rotate_right();
        break;

        default:
        break;
    }
}

static bool figure_fall(void)
{
    figure_off_add(&(const point_s){.x = 0, .y = 1});
    if (true == figure_cup_check())
    {
        figure_off_add(&(const point_s){.x = 0, .y = -1});
        return false;
    }
    return true;
}

static bool figure_move_left(void)
{
    figure_off_add(&(const point_s){.x = -1, .y = 0});
    if (true == figure_cup_check())
    {
        figure_move_right();
        return false;
    }
    return true;
}

static bool figure_move_right(void)
{
    figure_off_add(&(const point_s){.x = 1, .y = 0});
    if (true == figure_cup_check())
    {
        figure_move_left();
        return false;
    }
    return true;
}

static bool figure_rotate_left(void)
{
    for (int idx = 0; idx < FIGURE_SIZE; idx++)
    {
        int tmp= -g_ctx.figure.points[idx].x;
        g_ctx.figure.points[idx].x = g_ctx.figure.points[idx].y;
        g_ctx.figure.points[idx].y = tmp;
    }
    if (true == figure_cup_check())
    {
        figure_rotate_right();
        return false;
    }
    return true;
}

static bool figure_rotate_right(void)
{
    for (int idx = 0; idx < FIGURE_SIZE; idx++)
    {
        int tmp= -g_ctx.figure.points[idx].y;
        g_ctx.figure.points[idx].y = g_ctx.figure.points[idx].x;
        g_ctx.figure.points[idx].x = tmp;
    }
    if (true == figure_cup_check())
    {
        figure_rotate_left();
        return false;
    }
    return true;
}

static void figure_print(void)
{
    for (int idx = 0; idx < FIGURE_SIZE; idx++)
    {
        const point_s tmp_point = 
        {
            .x = g_ctx.figure.points[idx].x + g_ctx.offset.x,
            .y = g_ctx.figure.points[idx].y + g_ctx.offset.y
        };
        point_print(&tmp_point, g_ctx.figure.num);
    }
}

static void figure_shadow_print(void)
{
    point_s orig_offset = g_ctx.offset;
    tile_num_e orig_num = g_ctx.figure.num;

    g_ctx.figure.num = TILE_NUM_SHADOW;

    while (true == figure_fall());
    figure_print();

    g_ctx.offset = orig_offset;
    g_ctx.figure.num = orig_num;
}
