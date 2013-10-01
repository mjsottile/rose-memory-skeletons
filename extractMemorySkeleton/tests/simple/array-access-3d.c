#include <stdlib.h>
#include <math.h>

int main() {
  double d[100][100][100];
  double x;
  double y;
  int i;
  int j;
  int k;

  i = 20;
  j = 30;
  k = 73;
  x = d[i][j][k];
  y = sqrt(d[i][i+1][k]);
  x = (d[j][k][i]) + y;
}

