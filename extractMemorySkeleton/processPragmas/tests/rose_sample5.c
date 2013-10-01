/* TODO: support these, static
#pragma skel initializer repeat(3)
int   foobar_a [5/3];
#pragma skel initializer repeat(7)
int   foobar_b [5];  // warning!
#pragma skel initializer repeat(7)
float foobar_c [5];
*/
#pragma aaa

void f1(int x)
{
  
#pragma def
  int y = 5;
  
#pragma skel initializer repeat ( 85 )
  int foobar_c[x * 2];
  unsigned long _k_1 = 0UL;
  for (; _k_1 < x * 2; _k_1++) 
    foobar_c[_k_1] = 85;
}

void f2(int x)
{
  
#pragma skel initializer repeat ( 1 / 2 )
  int foobar_d[x++];
  unsigned long _k_2 = 0UL;
  for (; _k_2 < x++; _k_2++) 
    foobar_d[_k_2] = 1 / 2;
}

void f3(int x)
{
  
#pragma skel initializer repeat ( 67 )
  char foobar_e[20UL];
  unsigned long _k_3 = 0UL;
  for (; _k_3 < 20UL; _k_3++) 
    foobar_e[_k_3] = 67;
}

void f4(int x)
{
  
#pragma skel initializer repeat ( 0 )
  double foobar_f[20UL];
  unsigned long _k_4 = 0UL;
  for (; _k_4 < 20UL; _k_4++) 
    foobar_f[_k_4] = 0;
}
/* LATER
  #pragma skel initializer repeat('c')
  char foobar_j [20][30];
  #pragma skel initializer repeat(5)
  static int foobar_c [10];
  #pragma skel initializer repeat(5.0)
  float foobar_j [20][30];
  */
