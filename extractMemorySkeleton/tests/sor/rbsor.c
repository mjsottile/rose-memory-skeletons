/*
 * red-black successive over relaxation
 * 
 * Fall 2006 : matt@lanl.gov
 *
 * (current contact: matt@galois.com)
 */ 
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _PARALLEL_
#include <pthread.h>
#endif

typedef enum { RED, BLACK } color_t;

#define IDX(i,j,n) (((i)*(n))+(j)) 
#define is_even(x) (!((x)&0x1))

#ifdef _PARALLEL_ 

typedef struct thread_descriptor {
  double *d;
  double omega;
  int m;
  int n;
  double error;
  color_t color;
  int tnum,ttot;
} thread_desc_t;

void init_desc(thread_desc_t *desc, double *d, double omega,
               int m, int n, color_t color, int tnum, int ttot) {
  desc->d = d;
  desc->omega = omega;
  desc->m = m;
  desc->n = n;
  desc->error = 0.0;
  desc->color = color;
  desc->tnum = tnum;
  desc->ttot = ttot;
}

#endif

void dumper(double *d, int m, int n, char *fname) {
  int i,j;
  FILE *fp = fopen(fname,"w");

  for (i=0;i<m;i++) {
    for (j=0;j<n;j++) {
      fprintf(fp,"%f ",d[IDX(i,j,n)]);
    }
    fprintf(fp,"\n");
  }
  fclose(fp);
}

#ifdef _PARALLEL_

void *parallel_iterate(void *arg) {
  double *in;
  double omega;
  int m,n;
  int i,j;
  int m_hi,m_lo,n_hi,n_lo;
  int ttot,tnum,tmin,tmax;
  double tmp,tmp2;
  double err = 0.0;
  color_t color;
  thread_desc_t *desc = (thread_desc_t *)arg;

  m = desc->m;
  n = desc->n;
  in = desc->d;
  omega = desc->omega;
  color = desc->color;
  tnum = desc->tnum;
  ttot = desc->ttot;

  tmin = tnum*(m/ttot);
  tmax = (tnum+1)*(m/ttot);

  tmin += tnum>0 ? 1 : 0;

  printf("[%d %d %d]\n",tnum,tmin,tmax);

  m = tmax;

  switch (color) {
  case RED:
    m_lo = tmin;
    n_lo = 1;
    m_hi = is_even(m) ? m-2 : m-1;
    n_hi = is_even(n) ? n-2 : n-1;
    break;
  case BLACK:
    m_lo = tmin;
    n_lo = 2;
    m_hi = is_even(m) ? m-2 : m-1;
    n_hi = is_even(n) ? n-1 : n-2;
    break;
  default:
    assert('r'=='b');
    break;
  }
  
  for (i=m_lo;i<=m_hi;i+=2) {
    for (j=n_lo;j<=n_hi;j+=2) {
      tmp = (in[IDX(i+1,j+1,n)] + in[IDX(i-1,j+1,n)] + 
             in[IDX(i-1,j-1,n)] + in[IDX(i+1,j-1,n)]) / 4.0;
      tmp2 = in[IDX(i,j,n)];
      in[IDX(i,j,n)] += omega*(tmp - tmp2);
      err += abs(tmp2 - in[IDX(i,j,n)]);
    }
  }
    
  switch (color) {
  case RED:
    m_lo = tmin+1;
    n_lo = 2;
    m_hi = is_even(m) ? m-1 : m-2;
    n_hi = is_even(n) ? n-1 : n-2;
    break;
  case BLACK:
    m_lo = tmin+1;
    n_lo = 1;
    m_hi = is_even(m) ? m-1 : m-2;
    n_hi = is_even(n) ? n-2 : n-1;
    break;
  default:
    assert('r'=='b');
    break;
  }
  
  for (i=m_lo;i<=m_hi;i+=2) {
    for (j=n_lo;j<=n_hi;j+=2) {
      tmp = (in[IDX(i+1,j+1,n)] + in[IDX(i-1,j+1,n)] + 
             in[IDX(i-1,j-1,n)] + in[IDX(i+1,j-1,n)]) / 4.0;
        tmp2 = in[IDX(i,j,n)];
        in[IDX(i,j,n)] += omega*(tmp - tmp2);
        err += abs(tmp2 - in[IDX(i,j,n)]);
    }
  }
  
  desc->error = err;

  return NULL;
}

void parallel_sor(double *in, int m, int n, double eps, int numthreads) {
  double omega,error;
  pthread_t thread[numthreads];
  thread_desc_t descs[numthreads];
  int rc,status,i;
  pthread_attr_t attr;

  status = 0;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  omega = 2.0 / (1.0 + sqrt(1.0 - ((cos(M_PI/n) + cos(M_PI/m))/2.0) * 
                            ((cos(M_PI/n) + cos(M_PI/m))/2.0)));
  error = 0.0;

  for (i=0;i<numthreads;i++) {
    init_desc(&descs[i],in,omega,m,n,RED,i,numthreads);
  }

  do {
    error = 0.0;

    for (i=0;i<numthreads;i++) {
      descs[i].color = RED;
      rc = pthread_create(&thread[i], &attr, parallel_iterate, &descs[i]);
    }

    for (i=0;i<numthreads;i++) {
      rc = pthread_join(thread[i], (void **)status);
      error += descs[i].error;
    }

    for (i=0;i<numthreads;i++) {
      descs[i].color = BLACK;
      rc = pthread_create(&thread[i], &attr, parallel_iterate, &descs[i]);
    }

    for (i=0;i<numthreads;i++) {
      rc = pthread_join(thread[i], (void **)status);
      error += descs[i].error;
      descs[i].error = 0.0;
    }

  } while (error > eps);
}

#endif


/**
 * iterate()
 *
 * assume in and out are arrays of (m+2)*(n+2) size to deal with boundary
 * cells
 */
double iterate(double *in, int m, int n, double omega, color_t color) {
  int i,j;
  int m_hi,m_lo,n_hi,n_lo;
  double tmp,tmp2;
  double err = 0.0;

  switch (color) {
  case RED:
    m_lo = 1;
    n_lo = 1;
    m_hi = is_even(m) ? m-2 : m-1;
    n_hi = is_even(n) ? n-2 : n-1;
    break;
  case BLACK:
    m_lo = 1;
    n_lo = 2;
    m_hi = is_even(m) ? m-2 : m-1;
    n_hi = is_even(n) ? n-1 : n-2;
    break;
  default:
    assert('r'=='b');
    break;
  }

  for (i=m_lo;i<=m_hi;i+=2) {
    for (j=n_lo;j<=n_hi;j+=2) {
      tmp = (in[IDX(i+1,j+1,n)] + in[IDX(i-1,j+1,n)] + 
             in[IDX(i-1,j-1,n)] + in[IDX(i+1,j-1,n)]) / 4.0;
      tmp2 = in[IDX(i,j,n)];
      in[IDX(i,j,n)] += omega*(tmp - tmp2);
      err += abs(tmp2 - in[IDX(i,j,n)]);
    }
  }

  switch (color) {
  case RED:
    m_lo = 2;
    n_lo = 2;
    m_hi = is_even(m) ? m-1 : m-2;
    n_hi = is_even(n) ? n-1 : n-2;
    break;
  case BLACK:
    m_lo = 2;
    n_lo = 1;
    m_hi = is_even(m) ? m-1 : m-2;
    n_hi = is_even(n) ? n-2 : n-1;
    break;
  default:
    assert('r'=='b');
    break;
  }

  for (i=m_lo;i<=m_hi;i+=2) {
    for (j=n_lo;j<=n_hi;j+=2) {
      tmp = (in[IDX(i+1,j+1,n)] + in[IDX(i-1,j+1,n)] + 
             in[IDX(i-1,j-1,n)] + in[IDX(i+1,j-1,n)]) / 4.0;
      tmp2 = in[IDX(i,j,n)];
      in[IDX(i,j,n)] += omega*(tmp - tmp2);
      err += abs(tmp2 - in[IDX(i,j,n)]);
    }
  }

  return err;
}

void sor(double *in, int m, int n, double eps) {
  double omega,error;

  omega = 2.0 / (1.0 + sqrt(1.0 - ((cos(M_PI/n) + cos(M_PI/m))/2.0) * 
                            ((cos(M_PI/n) + cos(M_PI/m))/2.0)));
  error = 0.0;

  do {
    error = iterate(in,m,n,RED,omega);
    error += iterate(in,m,n,BLACK,omega); 
  } while (error > eps);
}

int main(int argc, char **argv) {
  double *d;
  int m, n;
  int i;
#ifdef _PARALLEL_
  int numthreads;

  if (argc == 2) {
    numthreads = atoi(argv[1]);
  } else {
    numthreads = 2;
  }
  printf("%d threads\n",numthreads);
#endif // _PARALLEL_

  m = n = 10000;

  d = calloc(m*n,sizeof(double));
  for (i=0;i<n;i++) {
    d[i] = 99.23497432;
  }

#ifdef _PARALLEL_
  parallel_sor(d,m,n,0.0001,numthreads);
#else
  sor(d,m,n,0.0001);
#endif

  dumper(d,m,n,"final.dat");

  free(d);
  exit(EXIT_SUCCESS);
}
