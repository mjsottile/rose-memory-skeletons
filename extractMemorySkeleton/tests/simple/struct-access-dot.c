
typedef struct record_t {
  int mem1;
  int mem2;
} record;


int main() {
  int i;
  int j;
  record rec;

  rec.mem1 = 10;
  rec.mem2 = 0;

  i = rec.mem1;
  j = rec.mem1 + rec.mem2;

  rec.mem1 = i*j;
  rec.mem1 = rec.mem1 * rec.mem2;
}
