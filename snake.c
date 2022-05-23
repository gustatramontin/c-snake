#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>
#include <signal.h>


// waddch add character
// mvaddch move and add character
// waddstr add string
// wprintw formated output


#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define clamp(a, mi, ma) min(max(a, mi), ma)

#define PLAYER_1_COLOR wattron(win,COLOR_PAIR(1));
#define PLAYER_1_COLOR_OFF wattroff(win,COLOR_PAIR(1));

#define FRUIT_COLOR wattron(win,COLOR_PAIR(3));
#define FRUIT_COLOR_OFF wattroff(win,COLOR_PAIR(3));

#define curses_rgb(c) ((c*1000)/255)
#define RGB(r,g,b) curses_rgb(r), curses_rgb(g), curses_rgb(b)

#define MOVE(i,x) if (direction[i] != -x) direction[i] = x

int h, w;
int direction[2] = { 0,0 }; 
int score;
WINDOW * win;

char error[50];

int map1_size[] = {5,5};
int map1[] = {
    0,0,0,0,0,
    0,1,0,1,0,
    0,1,0,1,0,
    0,1,0,1,0,
    0,0,0,0,0,
};

int map2_size[] = {5,5};
int map2[] = {
    0,0,0,0,0,
    1,1,1,0,0,
    0,0,0,0,0,
    0,0,1,1,1,
    0,0,0,0,0,
};

int map3_size[] = {5,5};
int map3[] = {
    0,0,0,0,0,
    0,0,0,1,0,
    0,0,0,0,0,
    0,1,0,0,0,
    0,0,0,0,0,
};

int *maps[] = {map1, map2, map3};
int *maps_sizes[] = { map1_size, map2_size, map3_size};


typedef struct snake_part {
    int x, y, is_head;
} Snake_section;

struct snake {
    Snake_section * body;
    size_t body_size;
    char ch;
    char head_ch;
} Snake;

struct fruit {
    int x,y;
    char ch;
} Fruit;

void handle_resize(int signal);
void redefine_size();
void logic();
void grow();
void input();
void fruit_logic();
int new_x();
int new_y();
void draw_game();
char nearest_neighbor_scale(int x, int y);

void handle_resize(int signal) {
    endwin();
    redefine_size();
}

void redefine_size() {
    struct winsize wins;
    ioctl(0, TIOCGWINSZ, &wins);

    w = wins.ws_col;
    h = wins.ws_row;

}

void logic() {

    for (int i=Snake.body_size-1; i >= 1; i--) {
        
        Snake.body[i].x = Snake.body[i-1].x;
        Snake.body[i].y = Snake.body[i-1].y;
    }

    if (nearest_neighbor_scale(Snake.body[0].x+=direction[0], Snake.body[0].y+=direction[1]) == '+') {
        Snake.body[0].x -= direction[0];
        Snake.body[0].y -= direction[1];
    }

    Snake.body[0].x = clamp(Snake.body[0].x, 0, w);
    Snake.body[0].y = clamp(Snake.body[0].y, 1, h-1);

    fruit_logic();
}

void fruit_logic() {
    if (Snake.body[0].x == Fruit.x && Snake.body[0].y == Fruit.y) {
        Fruit.x = new_x();
        Fruit.y = new_y();
        grow();
        score += 10;
    }
}

void grow() {
    
    if (Snake.body_size > sizeof(Snake.body) / sizeof(Snake_section)) {
            Snake.body = (Snake_section *) realloc(Snake.body, (Snake.body_size+5) * sizeof(Snake_section));
    }

    Snake.body[Snake.body_size].x = 0;
    Snake.body[Snake.body_size].y = 0;

    Snake.body_size += 1;
}

void input(int key) {
    //printf("%c",key);
    switch (key) {
        case 'd':
            MOVE(0,1);
            direction[1] = 0;
            break;
        case 'a':
            MOVE(0,-1);
            direction[1] = 0;
            break;
        case 'w':
            direction[0] = 0;
            MOVE(1,-1);
            break;
        case 's':
            direction[0] = 0;
            MOVE(1,1);
            break;
    }
}

int new_x() {
    int new_x = rand() % w;

    return new_x;
}

int new_y() {
    int new_y = rand() % (h-1) + 1;
    return new_y;

}

char nearest_neighbor_scale(int x, int y) {
    int map_i = 2;

    int *map = maps[map_i];
    int *map_size = maps_sizes[map_i];

    float x_ratio = w / map_size[0];
    float y_ratio = (h) / map_size[1];

    int map_x = (int) ceil((x+1) / x_ratio)-1;

    int map_y = (int) ceil((y+1) / y_ratio)-1;

    map_x = clamp(map_x, 0, map_size[0]-1);
    map_y = clamp(map_y, 0, map_size[1]-1);

    int map_pos = map_y*(map_size[0])+map_x;

    if (map_pos > 24 )
        snprintf(error, sizeof(error), "%d %d-%d %f-%f",map_pos, map_x,map_y, x_ratio, y_ratio);
    int map_char = map[map_pos];

    switch (map_char) {
        case 0:
            return ' ';
            break;
        case 1:
            return '+';
            break;
    }
}

void draw_game() {

    // Draw map
    for (int j=1; j<h; j++) {
        for (int i=0; i<w; i++) {
            mvwaddch(win, j,i, nearest_neighbor_scale(i,j-1));
        }    
    }

    // Draw player
    PLAYER_1_COLOR
    mvwaddch(win, Snake.body[0].y, Snake.body[0].x, Snake.head_ch);
    PLAYER_1_COLOR_OFF

    for (int i=1; i < Snake.body_size; i++) {
        wmove(win, Snake.body[i].y, Snake.body[i].x);
        waddch(win, Snake.ch);
        
    }

    // Draw fruit
    FRUIT_COLOR
    wmove(win, Fruit.y, Fruit.x);
    waddch(win, Fruit.ch);
    FRUIT_COLOR_OFF
}

void draw_menu() {
    char buf[10];
    snprintf(buf, sizeof(buf), "SCORE %03d", score);
    mvwaddstr(win, 0,2, buf);
    mvwaddstr(win, 0,2+sizeof(buf), error);
}

int main(int argc, char **argv) {
    
    srand((unsigned) time(NULL));

    redefine_size();

    signal(SIGWINCH, handle_resize);

    Snake.body = (Snake_section *) malloc(sizeof(Snake_section) * 5);
    Snake.body[0].x = new_x();
    Snake.body[0].y = new_y();
    Snake.body[0].is_head = 1;
    Snake.body_size += 1;
    Snake.ch = '+';
    Snake.head_ch = '&';
    Fruit.x = new_x();
    Fruit.y = new_y();
    Fruit.ch = '@';

  
    initscr();
    start_color();
 
    noecho();
    cbreak();


    win = newwin(h, w, 0,0);
   

    int ch;
    nodelay(stdscr, TRUE);
    
    if (has_colors() == FALSE || can_change_color() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    init_color(COLOR_RED, RGB(254, 70, 58));
    init_color(COLOR_GREEN, RGB(104, 251, 68));
    init_color(COLOR_BLUE, RGB(96, 92, 250));
    init_color(COLOR_BLACK, RGB(26, 36, 30));

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    for (;;) {
        if ((ch = getch()) != ERR) {
            input(ch);
        }

        draw_menu();
        draw_game();
        logic();

        wrefresh(win);

        int delay = 64*1000;
        if (direction[1] != 0)
            delay *= 1.6;
        usleep(delay);
        clear();
    }

    endwin();
    return 0;
}