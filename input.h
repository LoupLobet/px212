#ifndef _INPUT_H_
#define _INPUT_H_

#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>


#define MAXSIZE 10
#define UP 1
#define LEFT 4
#define RIGHT 3
#define DOWN 2

int configureTerminal();
int io(void);

int resetTerminal(void);

#endif
