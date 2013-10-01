#include <iostream>
typedef float Real;

int main()
{
  int x = 0;
  
#pragma skel loop iterate atleast ( 10 )
{
    unsigned long _k_1 = 0UL;
    for (int i = 0; x < 100 || _k_1 < 10; _k_1++) {{
        x = (i + 1);
        if ((x % 2)) 
          x += 5;
      }
      rose_label__2:
      i++;
    }
  }
  int j;
  
#pragma skel loop iterate atmost ( 30 )
{
    unsigned long _k_3 = 0UL;
    for (j = 0; x < 100 && _k_3 < 30; _k_3++) {{
        x = (j + 1);
      }
      rose_label__4:
      j++;
    }
  }
  return x;
}
