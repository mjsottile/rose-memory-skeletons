#include <stdlib.h>

int main() {
  double ***d;
  double x;
  int i;
  int j;
  int k;

  d = malloc(sizeof(double)*100*100*100);

  i = 0;
  j = 20;
  k = 19;
  d[i][j][k] = i;
  x = d[i][j][k]*10;
  d[i][j][k] = x*10;
  d[i][j][k] = d[j][i][k]*10;

  free(d);
}

