
typedef int FunTy (int x);

int fun (int x) {
  return x*3;
}

int main() {
  FunTy* f;
  int i;
  int j;

  i = 10;
  f = fun;

  j = (*f)(j);
}
