#include <stdlib.h>

int main() {
  int y;
  int i;

  y = 20;

  switch ( y ) {
  case 0:
    i = 2;
    break;

  case 1:
    i = 3;
    break;

  case 2:
  case 3:
    i = 7;
    break;

  default:
    i = -1;
  }

}

