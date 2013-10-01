#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
  ACCESS_UNKNOWN = 0,
  ACCESS_READ,
  ACCESS_WRITE,
} access_e;

typedef struct {
  access_e access;
  void * addr; 
} trace_t;

extern const size_t    __max_traces;

/* Only reference these if you know what you're doing. */
extern FILE    * __dump;
extern trace_t * __trace;
extern size_t    __trace_count;

/* initializes the internal handle of the trace log and the internal storage
 * for the trace. In the case of failure it calls perror() and returns a
 * negative value, the normal return value is 0.
 */
int __initialize_trace(trace_t ** trc, FILE ** outfile, size_t * trc_count, size_t max_traces);

/* Writes `trc_count` trace elements from `trc` to dump file.  On failure, this
 * reports the error with perror and a negative value.  The normal return value
 * is 0.
 */
int __write_trace(const trace_t * const trc, const size_t trc_count, FILE * const outfile);

#define initialize_trace() __initialize_trace(&__trace, &__dump, &__trace_count, __max_traces)
#define write_trace()      __write_trace(__trace, __trace_count, __dump);

/* Takes the address as x, and the acces type as y */
#define add_trace(x,y)                              \
  __trace[__trace_count].access =  (y);             \
  __trace[__trace_count].addr   = &(x);             \
  __trace_count++;                                  \
  if( __trace_count >= __max_traces ){              \
   write_trace();                                   \
   __trace_count = 0;                               \
  } 

#define writer(x) add_trace((x),ACCESS_WRITE);
#define reader(x) add_trace((x),ACCESS_READ);

bool coinflip();
int choose(int numchoices);

#endif /* TRACE_H */
