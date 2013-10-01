#include "trace.h"

#include <stdlib.h>

#define DEBUG_TRACE 0

const size_t __max_traces = 1000;

/* Only reference these if you know what you're doing. */
FILE    * __dump;
trace_t * __trace;
size_t    __trace_count;

int __initialize_trace(trace_t ** trc, FILE ** outfile, size_t * trc_count, size_t max_traces)
{
#if DEBUG_TRACE
  printf("__initialize_trace with max_traces = %d\n",max_traces);
#endif
  /* make sure we have somewhere to write to */
  *outfile = fopen("dump.trc", "w");
  if( NULL == *outfile ){
    perror("Unable to open dump.trc");
    return -1;
  }

  /* Make sure we have internal storage */
  *trc = calloc(max_traces, sizeof(trace_t));
  if( NULL == *trc ){
    perror("Unable to allocate for traces");
    return -2;
  }
  
  /* We start out with no traces */
  *trc_count = 0;
  
  /* Looks like we're good to go */
  return 0;
}

int __write_trace(const trace_t * const trc, const size_t trc_count, FILE * const outfile)
{
#if DEBUG_TRACE
  printf("__write_trace(%p, %d, %p);\n",trc,trc_count,outfile);
#endif
  /* special case when there is no work to do */
  if( trc_count == 0 ) return 0;

  const size_t write_count = fwrite(trc, sizeof(trace_t), trc_count, outfile);
  /* Just checking ferror should be enough */
  if ( write_count < trc_count || ferror(outfile)){
    perror("Unable to write trace");
    return -1;
  }
  return 0;
}

extern int skeleton_main(int argc, char** argv);

int main(int argc, char** argv)
{
#if DEBUG_TRACE
  printf("inside main()\n");
#endif
  srand(42/* TODO: make the seed configurable? */);
  initialize_trace();
  int i = skeleton_main(argc, argv);
  write_trace();
  fclose(__dump);
  return i;
}

bool coinflip()
{
  choose(2);
}

int choose(int numchoices)
{
  return ((double)rand()/(RAND_MAX+1.0))*numchoices;
}
