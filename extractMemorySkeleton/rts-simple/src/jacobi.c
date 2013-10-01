#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define idx(i,j,w) ((i)*(w))+(j)

void dump(double *x, int w, int h) {
  FILE *fp = fopen("output.dat","w");

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      fprintf(fp,"%lf ",x[idx(i,j,w)]);
    }
    fprintf(fp,"\n");
  }

  fclose(fp);
}

int main(int argc, char **argv) {
  double *x, *y;
  double *tmp;
  int width, height;
  //double epsilon = 1e-5;
  double epsilon = 141.053516;
  double err     = epsilon + 1;

#pragma skel preserve
  width = 50;
#pragma skel preserve
  height = 50;

  x = malloc(sizeof(double)*(width+2)*(height+2));
  y = malloc(sizeof(double)*(width+2)*(height+2));

  for (int i = 0; i < (width+2)*(height+2); i++) {
    x[i] = 0.0;
    y[i] = 0.0;
  }

  for (int i = 0; i < width+2; i++) {
    x[idx(0,i,width+2)] = 100.0;
    y[idx(0,i,width+2)] = 100.0;
  }

  //int count = 1;
#pragma skel loop iterate atleast(70)
  while (err > epsilon) {
    for (int i = 1; i <= width; i++) {
      for (int j = 1; j <= height; j++) {
        y[idx(i,j,width+2)] = 0.25 * (x[idx(i-1,j,width+2)] +
          x[idx(i+1,j,width+2)] + x[idx(i,j-1,width+2)] +
          x[idx(i,j+1,width+2)]);
      }
    }
    err = 0.0;
    for (int i = 0; i < (width+2)*(height+2); i++) {
      err += fabs(x[i]-y[i]);
    }
    tmp = x;
    x = y;
    y = tmp;
    //printf("num iterations = %d\n",count);
    //printf("err = %lf\n",err);
    //count++;
  }

  //dump(x,width+2,height+2);
  free(x);
  free(y);
}
