#include "input.h"


int io(void) {

      int key;
      int ch;
    

      ch = _getch();
      if (ch == 0 || ch == 224)
          key =  _getch ();//key code has two keys - read the second one
      else if (ch == 27)
          return 27;
	  else if (ch == 122)
	  	  return 'z';
          
      switch (key) {
    
        case UP: return 'U'; break;
        case DOWN: return 'D'; break;
        case LEFT: return 'L'; break;
        case RIGHT:return 'R'; break;
        default:  return 'P'; break;
      }
}

   
 

