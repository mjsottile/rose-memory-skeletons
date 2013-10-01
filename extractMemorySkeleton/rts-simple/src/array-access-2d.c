#include <stdlib.h>

int main() {
  volatile double d[100][100];
  volatile double x;
  volatile int i;
  volatile int j;

  i = 20;
  j = 0;
  x = d[i][j];
  x = d[j][i] + x;
  x = d[i][j] + 2*x;
}

