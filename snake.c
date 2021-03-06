#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>
#include <signal.h>
#include <locale.h>


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
float dash = 1;

wchar_t block_char = L'█';

char error[50];

enum Scenes {
    GAME,
    GAME_OVER
};

enum Scenes current_scene = GAME;

int is_game_stopped = 0;

int num_of_maps=0;// = 3;
int map_number;
int **maps = NULL; //= {map1, map2, map3};
int **maps_size = NULL;// = { map1_size, map2_size, map3_size};


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
    wchar_t ch;
} Fruit;

void handle_resize(int signal);
void redefine_size();
void logic();
void grow();
void game_scene_input();
void game_scene_loop();
void game_over_scene_loop();
void new_fruit();
void fruit_logic();
int new_x();
int new_y();
void draw_game();
wchar_t nearest_neighbor_scale(int x, int y);
void load_maps();
void reset();

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
    
    int self_death = 0;
    for (int i=1; i < Snake.body_size; i++) {
        if (Snake.body[0].x == Snake.body[i].x && Snake.body[0].y == Snake.body[i].y )
            self_death = 1;
            
    }

    for (int i=Snake.body_size-1; i >= 1; i--) {
        
        Snake.body[i].x = Snake.body[i-1].x;
        Snake.body[i].y = Snake.body[i-1].y;
    }
    if (nearest_neighbor_scale(Snake.body[0].x,
                                Snake.body[0].y) == block_char 
            || self_death) {
        current_scene = GAME_OVER;
    }
    Snake.body[0].x+=direction[0];
    Snake.body[0].y+=direction[1];

    if (Snake.body[0].x < 0 || Snake.body[0].x > w)
        Snake.body[0].x = w-Snake.body[0].x;
    if (Snake.body[0].y < 1 || Snake.body[0].y > h)
        Snake.body[0].y = h-Snake.body[0].y+1;
    
    fruit_logic();
}

void new_fruit() {
    Fruit.x = new_x();
    Fruit.y = new_y();

    if (nearest_neighbor_scale(Fruit.x, Fruit.y) == block_char) 
        new_fruit();
    
};

void fruit_logic() {
    if (Snake.body[0].x == Fruit.x && Snake.body[0].y == Fruit.y) {
        grow();
        new_fruit();
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

void game_scene_input(int key) {
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

        case 'D':
            MOVE(0,2);
            direction[1] = 0;
            break;
        case 'A':
            MOVE(0,-2);
            direction[1] = 0;
            break;
        case 'W':
            direction[0] = 0;
            MOVE(1,-2);
            break;
        case 'S':
            direction[0] = 0;
            MOVE(1,2);
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

int new_map() {
    return rand() % num_of_maps;
}

wchar_t nearest_neighbor_scale(int x, int y) {

    int *map = maps[map_number];
    int *map_size = maps_size[map_number];

    float x_ratio = w / map_size[0];
    float y_ratio = (h-1) / map_size[1];

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
            return block_char;
            break;
    }
}

void draw_game() {

    // Draw map
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            wmove(win, j,i);
            wprintw(win, "%lc", nearest_neighbor_scale(i,j));
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
    wprintw(win, "%lc", Fruit.ch);
    FRUIT_COLOR_OFF
}

void draw_menu() {
    char buf[10];
    snprintf(buf, sizeof(buf), "SCORE %03d", score);
    mvwaddstr(win, 0,2, buf);
    mvwaddstr(win, 0,2+sizeof(buf), error);
}

void reset() {
    free(Snake.body);
    Snake.body = (Snake_section *) malloc(sizeof(Snake_section) * 5);
    Snake.body[0].x = new_x();
    Snake.body[0].y = new_y();
    Snake.body_size = 1;
    Fruit.x = new_x();
    Fruit.y = new_y();
    direction[0] = 0;
    direction[1] = 0;
    map_number = new_map();
}

void game_over_scene_loop() {
    int ch;
    if ((ch = getch()) != ERR) {
            switch (ch)
            {
            case 'q':
            case 27: // Esc
                is_game_stopped = 1;
                break;
            
            default:
                current_scene = GAME;
                reset();
                break;
            }
        }

    mvwaddstr(win, 0,2, "GAME OVER");
    mvwaddstr(win, h-1,2, "Press Esc|q to exit");
    mvwaddstr(win, 2,2, "Press any key to start");


}

void game_scene_loop() {
    int ch;
    if ((ch = getch()) != ERR) {
            game_scene_input(ch);
        }

        draw_game();
        draw_menu();
        logic();

}

void load_maps(const char * map_file) {
    FILE *fp;

    fp = fopen(map_file, "r");
    
    int line = 0;
    
    char buff[20];

    int map_cols;
    int map_rows;
    int *map;
    int next = 0;
    while (fgets(buff, sizeof buff, fp) != NULL) {
        if (buff[0] == 'd') {
            sscanf(buff, "d %d,%d", &map_cols, &map_rows);
            map = (int *) malloc(sizeof(int) * (map_cols * map_rows));
            next = 0;
        } else {
            for (int i = 0; i < map_cols; i++) {
                switch (buff[i])
                {
                case '.':
                    map[next] = 0;
                    next++;
                    break;
                case '#':
                    map[next] = 1;
                    next++;
                    break; 
                default:
                    break;
                }
            }

            if (next == (map_cols * map_rows)) {
                num_of_maps++;
                int * new_size = (int *) malloc(sizeof(int)*2);
                new_size[0] = map_cols;
                new_size[1] = map_rows;
                maps_size = (int **) realloc(maps_size, num_of_maps*sizeof(int));
                maps_size[num_of_maps-1] = new_size;

                maps = (int **) realloc(maps, sizeof(int)*num_of_maps);
                maps[num_of_maps-1] = map;                

            }

        }

        line++;
    }

    snprintf(error, sizeof error, "%d,%d", maps_size[0][0], maps_size[0][1]);

    fclose(fp);
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
    Fruit.ch = L'Ó';
    
    load_maps("maps.txt");
    map_number = new_map();

    setlocale(LC_ALL, "");
    initscr();
    start_color();
 
    noecho();
    cbreak();


    win = newwin(h, w, 0,0);

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
        if (is_game_stopped) break;

        switch (current_scene)
        {
        case GAME:
            game_scene_loop();
            break;
        case GAME_OVER:
            game_over_scene_loop();
        
        default:
            break;
        }

        wrefresh(win);

        int delay = 64*1000;
        if (direction[1] != 0)
            delay *= 1.5;
        usleep(delay);
        clear();
    }

    endwin();
    return 0;
}