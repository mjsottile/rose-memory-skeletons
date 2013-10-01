#include <stdlib.h>

int main() {
  double *d;
  int i;

  d = malloc(sizeof(double)*100);
  i = 0;
  for (i=0;i<100;i++) {
    d[i] = d[i]*10;
  }
  free(d);
}

