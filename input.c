#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <unistd.h>

#include "input.h"
#include "termios.h"

int
configureTerminal(void)
{
	struct termios new;

	if (tcgetattr(0,&new)==-1) {
		perror("tcgetattr");
		return -1;
	}
	new.c_lflag &= ~(ICANON); /* canonical termianal */
	new.c_lflag &= ~(ECHO);   /* do not display keyboard entries anymore */
	new.c_cc[VMIN]=1;
	new.c_cc[VTIME]=0;
	if (tcsetattr(0,TCSANOW,&new)==-1) {
		perror("tcsetattr");
		return 0;
	}
	return 1;
}

int
io(void) {
	char r[MAXSIZE];
	int c;
	int key;

	if ((c = read(0,r,3)) == - 1 )
		return 0;
	switch (r[0]) {
	case 27 :
		if (c == 3 && r[1] == 91){
			key = (r[2] - 64);
			switch (key) {
			case UP:
				return 'U';
			case DOWN:
				return 'D';
			case LEFT:
				return 'L';
			case RIGHT:
				return 'R';
			default:
				return 'P';
			}
		} else if (c == 1 && r[0] == 27)
			return 27;
		else
			return 0;
		break;
	default:
		return r[0];
		break;
	}
}
