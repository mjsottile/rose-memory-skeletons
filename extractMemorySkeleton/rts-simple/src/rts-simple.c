#include "rts-simple.h"

#include <stdlib.h>

#define DEBUG_TRACE 0

extern int skeleton_main(int argc, char** argv);

int main(int argc, char** argv)
{
#if DEBUG_TRACE
  printf("inside main()\n");
#endif
  srand(42/* TODO: make the seed configurable? */);
  int i = skeleton_main(argc, argv);
  return i;
}

int choose(int numchoices)
{
  return ((double)rand()/(RAND_MAX+1.0))*numchoices;
}

bool coinflip()
{
  return choose(2);
}

