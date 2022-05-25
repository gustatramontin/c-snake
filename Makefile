
all: 
	gcc -o bin/snake snake.c -lncursesw -lm -D_GNU_SOURCE -D_DEFAULT_SOURCE 