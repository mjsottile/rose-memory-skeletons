#include <stdlib.h>

int main() {
  double d[100][100];
  double x;
  int i;
  int j;

  i = 20;
  j = 0;
  x = d[i][j];
  x = d[j][i] + x;
  x = d[i][j] + 2*x;
}

