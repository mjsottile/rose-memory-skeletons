#include <stdlib.h>

int main() {
  double d[100];
  double x;
  int i;

  i = 20;
  d[i] = 0.3;
  x = d[i]*10;
  d[i] = d[i]*10;
  d[i] = x*10;
  d[i+3] = x*2;
}

