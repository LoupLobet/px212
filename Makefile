CC=cc
CFLAGS=-Wall -g -fsanitize=address
LDFLAGS=-fsanitize=address
OBJS=display.o ia.o input.o loader.o move.o util.o main.o

all: clean sokoban

sokoban : $(OBJS)
	$(CC) -o sokoban $(OBJS) $(LDFLAGS)

display.o: display.c
# display.c generates warning with pedantic because of '\e'
	$(CC) -c display.c $(CFLAGS)

.c.o:
	${CC} -c $< $(CFLAGS) -pedantic

clean:
	rm -f *.o
	rm -f sokoban
