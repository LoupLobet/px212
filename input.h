#include <termios.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdbool.h>


#define MAXSIZE 10 
#define UP 1
#define LEFT 4
#define RIGHT 3
#define DOWN 2

int io(void);
int configureTerminal();