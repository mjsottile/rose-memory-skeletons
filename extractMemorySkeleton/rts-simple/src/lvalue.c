int main()
{
   int j;

   // Incorrect usage: The left operand must be an lvalue (C2106).
   j * 4 = 7;
   return 0;
}
