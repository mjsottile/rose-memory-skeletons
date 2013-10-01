
int main()
{
  int x = 0;
  int i;
  int j;
  
#pragma skel loop iterate atleast ( 10 )
{
    unsigned long _k_1 = 0UL;
    do {{
        x = (i + 1);
        if ((x % 2) != 0) 
          x += 5;
        i++;
      }
      _k_1++;
    }while (x < 100 || _k_1 < 10);
  }
  
#pragma skel loop iterate atmost ( 30 )
{
    unsigned long _k_2 = 0UL;
    while(x != 512 && _k_2 < 30){
      x += 11;
      _k_2++;
    }
  }
  
#pragma skel loop iterate exactly ( 10 * 10 )
{
    unsigned long _k_3 = 0UL;
    while((x != 512 , _k_3 < 10 * 10)){
      x += 11;
      _k_3++;
    }
  }
  return x;
}
