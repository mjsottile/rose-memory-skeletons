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
  d[i][j][k] = i;
  x = d[i][j][k]*10;
  y = sqrt(d[i][i+1][k]);
  d[i][j][k] = x*10;
  d[i][j][k] = d[j][i][k]*y;
}

