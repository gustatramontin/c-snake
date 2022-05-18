#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

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
} Snake;

void logic();
void grow();
void input();

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

int main(int argc, char **argv) {

    Snake.body = (Snake_section *) malloc(sizeof(Snake_section) * 5);
    Snake.body[0].x = 5;
    Snake.body[0].y = 5;
    Snake.body[0].is_head = 1;
    Snake.body_size += 1;

    initscr();
 
    noecho();
    cbreak();

    getmaxyx(stdscr, h , w);

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
            waddch(win, '#');
            
        }

        logic();

        wrefresh(win);
        sleep(1);
        clear();
    }

    endwin();
    return 0;
}