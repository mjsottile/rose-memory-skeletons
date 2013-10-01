#include <stdlib.h>

int main() {
  double *d;
  double x;
  int i;

  d = malloc(sizeof(double)*100);

  i = 0;
  d[i] = i;
  x = d[i]*10;
  d[i] = d[i]*10;
  d[i] = x*10;

  free(d);
}

