
int main()
{
  int x = 0;
  int i;
  
#pragma skel loop iterate exactly ( 12 )
{
    unsigned long _k_1 = 0UL;
    for (i = 0; (x < 100 , _k_1 < 12); _k_1++) {{
        if (x == 7) {
          goto rose_label__2;
        }
        if ((x % 2) != 0) 
          x += 5;
      }
      rose_label__2:
      i++;
    }
  }
  return x;
}
