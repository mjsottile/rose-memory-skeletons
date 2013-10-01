#include <stdio.h>


int main(int argc, char ** argv)
{
  int x = 3;
  switch(x){
    case 0: printf("0\n"); break;
    case 1: printf("1\n"); break;
    case 2: printf("2\n"); break;
    case 3: printf("3\n"); break;
    default: printf("default\n"); break;
  }
  return 0;
}

