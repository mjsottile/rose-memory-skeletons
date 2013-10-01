/*
 * simple heap in C
 *
 * matt@galois.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct heap_element {
  int priority;
  void *data;
} heap_element_t;

#pragma skel memory
typedef struct heap {
#pragma skel memory
  int maxcapacity;
#pragma skel memory
  int n;
#pragma skel memory
  heap_element_t *data; // region of interest
} heap_t;

heap_t *newHeap(int cap) { // RW HEAD
  heap_t *h;

  h = malloc(sizeof(heap_t));
  assert(h != NULL);
  h->maxcapacity = cap;
  h->n = 0;
  h->data = malloc(sizeof(heap_element_t)*cap); // W HEAD
  assert(h->data != NULL);                      // R HEAD

  return h;
}

void deleteHeap(heap_t *h) { // RW HEAD
  assert(h != NULL);

  free(h->data); // RW HEAD
  free(h);
}

void growHeap(heap_t *h, int newcap) { // RW HEAD
  heap_element_t *newdata;

  assert(h != NULL);
  assert(h->maxcapacity < newcap);

  newdata = realloc(h->data, sizeof(heap_element_t)*newcap); // R HEAD
  assert(newdata != NULL);

  h->data = newdata;                                         // W HEAD
  h->maxcapacity = newcap;
}

#define GROWTHFACTOR 1.5

#define PARENT(i) (((i)-1) >> 1)
#define LCHILD(i) (2*(i) + 1)
#define RCHILD(i) (2*(i) + 2)

void swap(heap_t *h, int i, int j) { // RW i, RW j
  heap_element_t tmp;

  assert(h != NULL);
  assert(i < h->n);
  assert(j < h->n);

  tmp = h->data[i];        // R i
  h->data[i] = h->data[j]; // R j, W i
  h->data[j] = tmp;        // W j
  /*
  read[h->data[i]];
  read[h->data[j]];
  write[h->data[i]];
  write[h->data[j]];
   */
}

void heap_down(heap_t *h, int i) {
  int lchild, rchild, cur;
  heap_element_t l_elt, r_elt;

  assert(h != NULL);

  cur = i;                     // R i, cur == i

  while (cur < h->n) {
    rchild = RCHILD(cur);      // rchild == LCHILD(cur)
    lchild = LCHILD(cur);      // lchild == RCHILD(cur)
    if (rchild < h->n) { 
      l_elt = h->data[lchild]; // R LCHILD(cur)
      r_elt = h->data[rchild]; // R RCHILD(cur)
      int max = (l_elt.priority > r_elt.priority) ? lchild : rchild;
                               // max = oneOf { LCHILD(cur), RCHILD(cur) }
      if (h->data[cur].priority < h->data[max].priority) { // R max, R cur
        swap(h,max,cur); // RW max, RW cur
        cur = max;       // cur == max
      } else {
        break;
      }
    } else {
      if (lchild < h->n) { // R LCHILD(cur)
        if (h->data[cur].priority < h->data[lchild].priority) {
          swap(h,lchild,cur); // RW LCHILD(cur), RW cur
          cur = lchild;       // W cur, R LCHILD(cur), cur == LCHILD(cur)
        } else {
          break;
        }
      } else {
        break;
      }
    }
  }
}

void heap_up(heap_t *h, int i) {
  int parent, cur;

  assert(h != NULL);
  
  cur = i;
  if (cur==0) return;
  
  parent = PARENT(cur);     // R cur, W PARENT(cur)
  
  while (cur > 0) {
    if (h->data[parent].priority < h->data[cur].priority) {
      swap(h,parent,cur);   // RW parent, RW cur
      cur = parent;         // R parent, W cur, cur == parent
      parent = PARENT(cur); // W parent, R PARENT(cur == parent)
    } else {
      break;
    }
  }
}

void insert(heap_t *h, heap_element_t e) {
  assert(h != NULL);

  if (h->maxcapacity == h->n) {
    growHeap(h, (int)(h->maxcapacity * GROWTHFACTOR)); // RW HEAD
  }

  h->data[h->n] = e;    // W HEAD
  h->n++;
  heap_up(h, h->n - 1); // Depends upon how annotations are aggregated
                        // from conditionals and loops in heap_up
}

heap_element_t delete(heap_t *h) {
  assert(h != NULL);
  assert(h->n > 0);
  
  heap_element_t retval = h->data[0]; // R HEAD
  h->n--;
  h->data[0] = h->data[h->n];         // W HEAD
  heap_down(h, 0);                    // Depends upon how annotations are
                                      // aggregated from conditionals and
                                      // loops in heap_down

  return retval;
}

int main(int argc, char **argv) {
  // test harness - heap sort!
  int n, i;
  heap_t *h;

  n = 1000000;

  h = newHeap(10000000);

  for (i=0;i<n;i++) {
    heap_element_t e;
    e.priority = rand() % n;
    e.data = NULL;
    printf("IN: %d\n",e.priority);
    insert(h,e);                    // > W i
  }

  for (i=0;i<n;i++) {
    heap_element_t e = delete(h);
    printf("OUT: %d\n",e.priority); // > RW i
  }

  deleteHeap(h);                    // RW HEAD
  exit(EXIT_SUCCESS);
}
