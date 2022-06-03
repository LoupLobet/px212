

testdisplay : util.o display.o testdisplay.c
	gcc -Wall -g -fsanitize=address testdisplay.c util.o display.o -o testdisplay

util.o : util.c
	gcc -Wall -g -fsanitize=address -c util.c

display.o : display.c
	gcc -Wall -g -fsanitize=address -c display.c


clear:
	rm *.o
