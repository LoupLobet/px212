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

int	 configterm();
int	 io(void);
int	 resetterm(void);

