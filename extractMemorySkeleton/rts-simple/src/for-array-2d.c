#include <stdlib.h>

int main() {
  double d[100][100];
  int i, j;
  double y;

  y = 2;
  for (i=0;i<100;i++) {
    for (j=0;j<100;j++) {
      d[i][j] = y;
    }
  }
  return 0;
}

