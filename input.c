#include "input.h"
struct termios new;
struct termios old;

/**
 * @brief We configure the terminal to read one character at a time. We don't want to wait for a newline and we don't want to echo the character.
 * 
 * @return int return 0 if the configuration is correct, -1 otherwise.
 */

int
configureTerminal(void)
{
	

	if (tcgetattr(0,&old)==-1) {
		perror("tcgetattr");
		return -1;
	}
	new = old;
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

/**
 * @brief 
 * 
 * @return int Return 0 if the configuration is correct, -1 otherwise.
 */

int
resetTerminal(void)
{
	if (tcsetattr(0,TCSANOW,&old)==-1) {
		perror("tcsetattr");
		return 0;
	}
	return 1;
}

/**
 * @brief We read one character from the terminal.
 * 
 * @return int Return the character read.
 */

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


