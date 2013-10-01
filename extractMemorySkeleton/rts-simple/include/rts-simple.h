#ifndef RTS_SIMPLE_H
#define RTS_SIMPLE_H

#define writer(x) x = 42
#define reader(x) x

#include <stdbool.h>

int choose(int numchoices);
bool coinflip();

#endif /* RTS_SIMPLE_H */
