#include <stdlib.h>

int main() {
  double d[100][100][100];
  int i, j, k;
  int y;

  y = 3;
  for (i=0;i<100;i++) {
    for (j=0;j<100;j++) {
      for (k=0;k<100;k++) {
        d[i][j][k] = y;
      }
    }
  }
}

