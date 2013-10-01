#include <stdio.h>

#define N 1000

struct mystruct { int x; };

int foo(int a)
{
  return a + 1;
}

int bar(int *a)
{
  return *a + 1;
}

void baz(struct mystruct * t){}

void quux(struct mystruct t){}

int main(int argc, char** argv)
{
  int A[N];
  int i = 1;
 
  struct mystruct t; 
  while(i < N){
    /* write some math junk to A[i] */
    A[i] = A[i] + A[i-1]*4 + A[0];

    /* write foo(A[i-1]) to A[i] */
    A[i] = foo(A[i-1]);

    /* write bar(&i) to A[i] */
    A[i] = bar(&i);

    /* write 0 to A[i] */
    A[i] = 0;

    /* pass &t to baz */
    baz(&t);

    /* pass t to quux */
    quux(t);

    /* end of loop */
    ++i;
  }
  return 0;
}
