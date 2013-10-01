#include <stdio.h>

#define N 793

int skeleton_main(int argc, char ** argv)
{
  float A[N][N];
  float B[N][N];
  for(int j = 0; j < N; ++j){
    for(int i = 0; i < N; ++i){
      float tmp = A[j-1][i-1] + A[j-1][i  ] + A[j-1][i+1]
                + A[j  ][i-1] + A[j  ][i  ] + A[j  ][i+1]
                + A[j+1][i-1] + A[j+1][i  ] + A[j+1][i+1];
      B[j][i] = tmp / 9;
    }
  }

  return 0;
}
