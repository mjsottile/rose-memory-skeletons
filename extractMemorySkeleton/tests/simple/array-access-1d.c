#include <stdlib.h>

int main() {
  double d[100];
  double x;
  int i;

  for (i=0;i<100;i++) d[i] = 0;

  i = 20;
  x = d[i];
  x = d[i+3] + i;
  //printf("%lf\n",x);
}

