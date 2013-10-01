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
void *data;}heap_element_t;
#pragma skel memory
typedef struct heap {
#pragma skel memory
int maxcapacity;
#pragma skel memory
int n;
#pragma skel memory
// region of interest
heap_element_t *data;}heap_t;
// RW HEAD

heap_t *newHeap(int cap)
{
  heap_t *h;
  h = (malloc((sizeof(heap_t ))));
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",29,"struct heap *newHeap(int)");
  h -> maxcapacity = cap;
  h -> n = 0;
// W HEAD
  h -> data = (malloc((sizeof(heap_element_t ) * cap)));
// R HEAD
  ((h -> data) != ((heap_element_t *)((void *)0)))?((void )0) : __assert_fail("h->data != ((void *)0)","heap.c",33,"struct heap *newHeap(int)");
  return h;
}
// RW HEAD

void deleteHeap(heap_t *h)
{
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",39,"void deleteHeap(struct heap *)");
// RW HEAD
  free((h -> data));
  free(h);
}
// RW HEAD

void growHeap(heap_t *h,int newcap)
{
  heap_element_t *newdata;
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",48,"void growHeap(struct heap *, int)");
  ((h -> maxcapacity) < newcap)?((void )0) : __assert_fail("h->maxcapacity < newcap","heap.c",49,"void growHeap(struct heap *, int)");
// R HEAD
  newdata = (realloc((h -> data),(sizeof(heap_element_t ) * newcap)));
  (newdata != ((heap_element_t *)((void *)0)))?((void )0) : __assert_fail("newdata != ((void *)0)","heap.c",52,"void growHeap(struct heap *, int)");
// W HEAD
  h -> data = newdata;
  h -> maxcapacity = newcap;
}
#define GROWTHFACTOR 1.5
#define PARENT(i) (((i)-1) >> 1)
#define LCHILD(i) (2*(i) + 1)
#define RCHILD(i) (2*(i) + 2)
// RW i, RW j

void swap(heap_t *h,int i,int j)
{
  heap_element_t tmp;
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",67,"void swap(struct heap *, int, int)");
  (i < (h -> n))?((void )0) : __assert_fail("i < h->n","heap.c",68,"void swap(struct heap *, int, int)");
  (j < (h -> n))?((void )0) : __assert_fail("j < h->n","heap.c",69,"void swap(struct heap *, int, int)");
// R i
  tmp = (h -> data)[i];
// R j, W i
  (h -> data)[i] = (h -> data)[j];
// W j
  (h -> data)[j] = tmp;
/*
  read[h->data[i]];
  read[h->data[j]];
  write[h->data[i]];
  write[h->data[j]];
   */
}

void heap_down(heap_t *h,int i)
{
  int lchild;
  int rchild;
  int cur;
  heap_element_t l_elt;
  heap_element_t r_elt;
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",86,"void heap_down(struct heap *, int)");
// R i, cur == i
  cur = i;
{
    while(cur < (h -> n)){
// rchild == LCHILD(cur)
      rchild = ((2 * cur) + 2);
// lchild == RCHILD(cur)
      lchild = ((2 * cur) + 1);
      if (rchild < (h -> n)) {
// R LCHILD(cur)
        l_elt = (h -> data)[lchild];
// R RCHILD(cur)
        r_elt = (h -> data)[rchild];
        int max = (l_elt.priority > r_elt.priority)?lchild : rchild;
// max = oneOf { LCHILD(cur), RCHILD(cur) }
// R max, R cur
        if ((h -> data)[cur].priority < (h -> data)[max].priority) {
// RW max, RW cur
          swap(h,max,cur);
// cur == max
          cur = max;
        }
        else {
          break; 
        }
      }
      else {
// R LCHILD(cur)
        if (lchild < (h -> n)) {
          if ((h -> data)[cur].priority < (h -> data)[lchild].priority) {
// RW LCHILD(cur), RW cur
            swap(h,lchild,cur);
// W cur, R LCHILD(cur), cur == LCHILD(cur)
            cur = lchild;
          }
          else {
            break; 
          }
        }
        else {
          break; 
        }
      }
    }
  }
}

void heap_up(heap_t *h,int i)
{
  int parent;
  int cur;
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",122,"void heap_up(struct heap *, int)");
  cur = i;
  if (cur == 0) 
    return ;
// R cur, W PARENT(cur)
  parent = ((cur - 1) >> 1);
{
    while(cur > 0){
      if ((h -> data)[parent].priority < (h -> data)[cur].priority) {
// RW parent, RW cur
        swap(h,parent,cur);
// R parent, W cur, cur == parent
        cur = parent;
// W parent, R PARENT(cur == parent)
        parent = ((cur - 1) >> 1);
      }
      else {
        break; 
      }
    }
  }
}

void insert(heap_t *h,heap_element_t e)
{
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",141,"void insert(struct heap *, struct heap_element)");
  if ((h -> maxcapacity) == (h -> n)) {
// RW HEAD
    growHeap(h,((int )((h -> maxcapacity) * 1.5)));
  }
// W HEAD
  (h -> data)[h -> n] = e;
  h -> n++;
// Depends upon how annotations are aggregated
  heap_up(h,((h -> n) - 1));
// from conditionals and loops in heap_up
}

heap_element_t delete(heap_t *h)
{
  (h != ((heap_t *)((void *)0)))?((void )0) : __assert_fail("h != ((void *)0)","heap.c",154,"struct heap_element delete(struct heap *)");
  ((h -> n) > 0)?((void )0) : __assert_fail("h->n > 0","heap.c",155,"struct heap_element delete(struct heap *)");
// R HEAD
  heap_element_t retval = (h -> data)[0];
  h -> n--;
// W HEAD
  (h -> data)[0] = (h -> data)[h -> n];
// Depends upon how annotations are
  heap_down(h,0);
// aggregated from conditionals and
// loops in heap_down
  return retval;
}

int main(int argc,char **argv)
{
// test harness - heap sort!
  int n;
  int i;
  heap_t *h;
  n = 1000000;
  h = newHeap(10000000);
  for (i = 0; i < n; i++) {
    heap_element_t e;
    e.priority = (rand() % n);
    e.data = ((void *)((void *)0));
    printf("IN: %d\n",e.priority);
// > W i
    insert(h,e);
  }
  for (i = 0; i < n; i++) {
    heap_element_t e = delete(h);
// > RW i
    printf("OUT: %d\n",e.priority);
  }
// RW HEAD
  deleteHeap(h);
  exit(0);
  return 0;
}
