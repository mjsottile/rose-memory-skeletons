
typedef enum { Red, Blue, Yellow, Green } Colors;

int main() {
  Colors color = Blue;
  int i,j;

  i = 10;
  j = 0;

  if ( i == 1 ) {
    i++;
  } else {
    i = 6 * j;
  }

  while (j < 10) {
    i = i - 1;
    j = j + 1;
  }

  switch ( color ) {
    case Red: {
      i = 3;
      break;
    }
    case Blue:
    case Yellow: {
      i = 7;
      break;
    }
    case Green: {
      i = 2;
      break;
    }
    default: {
      i = 14;
      break;
    }
  }
}
