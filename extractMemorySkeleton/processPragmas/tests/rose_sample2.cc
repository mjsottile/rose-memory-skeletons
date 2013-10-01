#include <iostream>
#include <stdlib.h> 
typedef float Real;

int main()
{
  int x = 0;
// initialize array
  
#pragma skel initializer repeat ( 95 )
  int myarray[10UL];
  unsigned long _k_1 = 0UL;
  for (; _k_1 < 10UL; _k_1++) 
    myarray[_k_1] = 95;
  
#pragma skel loop iterate exactly ( 10 )
{
    unsigned long _k_2 = 0UL;
    for (int i = 0; (x < 100 , _k_2 < 10); _k_2++) {{
        x = (i + 1);
// make true 60% of time.
        
#pragma skel condition prob ( 60 / 100 )
        if (((x % 2) , ((double )(rand())) / RAND_MAX < 60 / 100)) 
          x += 5;
      }
      rose_label__3:
      i++;
    }
  }
  return x;
}

void f(int x)
{
// never do.
  
#pragma skel condition prob ( 0 )
  if (((x % 2) , ((double )(rand())) / RAND_MAX < 0)) 
    x += 5;
// always do
  
#pragma skel condition prob ( 1 )
  if (((x % 2) , ((double )(rand())) / RAND_MAX < 1)) 
    x += 5;
  else 
    x -= 5;
// make true 2/3 of time.
  
#pragma skel condition prob ( 2 / 3 )
  if (((x % 2) , ((double )(rand())) / RAND_MAX < 2 / 3)) 
    x += 5;
}
