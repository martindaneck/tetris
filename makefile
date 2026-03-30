tetris: tetris.c
	gcc -o tetris tetris.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -g