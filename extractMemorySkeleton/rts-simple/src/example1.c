#include <stdio.h>

#define N 1000

int main(int argc, char** argv)
{
  volatile int A[N];
  int i = 1;
  
  while(i < N){
    A[i] = A[i-1] + 1;
    ++i;
  }
  return 0;
}
