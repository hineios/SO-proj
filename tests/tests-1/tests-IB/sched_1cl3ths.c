/*
 * Threads with different priorities II
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sthread.h>
#include <unistd.h>

static int counter1 = 0;
static int counter2 = 0;
static int counter3 = 0;

void *thread_1(void *);
void *thread_2(void *);
void *thread_3(void *);

int main(int argc, char **argv)
{
  int iterations=10000;
  int counter0 = 0;
  int c1, c2, c3;

  sthread_init();

  if (sthread_create(thread_1, (void*)0, 1, 10, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(1);
  }

  if (sthread_create(thread_2, (void*)0, 1, 1, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(1);
  }

  if (sthread_create(thread_3, (void*)0, 1, 1, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(1);
  }

  printf("created threads\n");

  for(; iterations > 0; iterations--)
    for(; counter0 < 300000000; counter0++);

  c1 = counter1;
  c2 = counter2;
  c3 = counter3;
  printf("thread 1: prio is 10, counter is %i \nthread 2: prio is 1, counter is %i\nthread 3: prio is 1, counter is %i\n", c1, c2, c3);

  sthread_dump();

  return 0;
}


void *thread_1(void *arg)
{
  int i;

  for (;;counter1++)
    for (i = 0; i < 1000000; i++);

  return 0;
}

void *thread_2(void *arg)
{
  int i;

  for (;;counter2++)
    for (i = 0; i < 1000000; i++);

  return 0;
}

void *thread_3(void *arg)
{
  int i;

  for (;;counter3++)
    for (i = 0; i < 1000000; i++);

  return 0;
}


