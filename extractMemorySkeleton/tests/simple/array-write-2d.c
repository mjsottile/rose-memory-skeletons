#include <stdlib.h>

int main() {
  double d[100][100];
  double x;
  int i;
  int j;

  i = 0;
  j = 20;
  d[i][j] = i;
  x = d[i][j]*10;
  d[i][j] = x*10;
  d[i][j] = d[j][i]*10;
}

