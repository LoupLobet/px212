
debugflags= -Wall -g -fsanitize=address
objets= display.o ia.o input.o loader.o move.o util.o

sokoban : main.c $(objets)
	gcc -o main main.c $(objets) $(debugflags)

%.o : %.c
	gcc -o $@ -c $< $(debugflags)

clear:
	-rm *.o
	-rm sokoban
