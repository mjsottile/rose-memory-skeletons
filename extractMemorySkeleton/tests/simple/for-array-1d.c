#include <stdlib.h>

int main() {
  double d[500];
  int i;
  int y;
  int j;

  y = 1;
  for (j=0;j<5;j++) {
  for (i=0;i<500;i++) {
    d[i] = y;
  }
  }
}

