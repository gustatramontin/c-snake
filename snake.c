#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <sys/ioctl.h>


// waddch add character
// mvaddch move and add character
// waddstr add string
// wprintw formated output


#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define clamp(a, mi, ma) min(max(a, mi), ma)

int h, w;
int direction[2] = { 0,0 }; 

typedef struct snake_part {
    int x, y, is_head;
} Snake_section;

struct snake {
    Snake_section * body;
    size_t body_size;
    char ch;
} Snake;

struct fruit {
    int x,y;
    char ch;
} Fruit;

void logic();
void grow();
void input();
int new_x();
int new_y();

void logic() {
    if (Snake.body_size <=5)
        grow();


    for (int i=Snake.body_size-1; i >= 1; i--) {
        
        Snake.body[i].x = Snake.body[i-1].x;
        Snake.body[i].y = Snake.body[i-1].y;
    }

    Snake.body[0].x += direction[0];
    Snake.body[0].y += direction[1];

    Snake.body[0].x = clamp(Snake.body[0].x, 0, w);
    Snake.body[0].y = clamp(Snake.body[0].y, 0, h);

 
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
            direction[0] = 1;
            direction[1] = 0;
            break;
        case 'a':
            direction[0] = -1;
            direction[1] = 0;
            break;
        case 'w':
            direction[0] = 0;
            direction[1] = -1;
            break;
        case 's':
            direction[0] = 0;
            direction[1] = 1;
            break;
    }
}

int new_x() {
    int new_x = rand() % w;

    return new_x;
}

int new_y() {
    int new_y = rand() % h;
    return new_y;

}

int main(int argc, char **argv) {
    
    srand((unsigned) time(NULL));

    struct winsize wins;
    ioctl(0, TIOCGWINSZ, &wins);

    w = wins.ws_col;
    h = wins.ws_row;


    Snake.body = (Snake_section *) malloc(sizeof(Snake_section) * 5);
    Snake.body[0].x = new_x();
    Snake.body[0].y = new_y();
    Snake.body[0].is_head = 1;
    Snake.body_size += 1;
    Snake.ch = '#';
    Fruit.x = new_x();
    Fruit.y = new_y();
    Fruit.ch = '@';

  
    initscr();
 
    noecho();
    cbreak();


    WINDOW * win = newwin(h, w, 0,0);
   

    wmove(win, 0,0);


    int ch;
    nodelay(stdscr, TRUE);

    for (;;) {
        if ((ch = getch()) != ERR) {
            input(ch);
        }

        for (int i=0; i < Snake.body_size; i++) {
            wmove(win, Snake.body[i].y, Snake.body[i].x);
            waddch(win, Snake.ch);
            
        }


        wmove(win, Fruit.y, Fruit.x);
        waddch(win, Fruit.ch);

        logic();

        wrefresh(win);
        sleep(1);
        clear();
    }

    endwin();
    return 0;
}