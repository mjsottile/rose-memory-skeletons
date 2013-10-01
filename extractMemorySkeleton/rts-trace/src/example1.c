#include <stdio.h>

#define N 1000

int skeleton_main(int argc, char** argv)
{
  int A[N];
  for(int i = 1; i < N; i++){
    A[i] = A[i-1] + 1;
  }
  return 0;
}
