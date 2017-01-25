/* 
 *
 * sthread_nice test
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <sthread.h>
#include <unistd.h>

static int counter0 = 0;
static int counter1 = 0;
static int counter2 = 0;
static int counter3 = 0;

int prio0;
int prio1;
int prio2;
int prio3;

void *thread0(void *);
void *thread1(void *);
void *thread2(void *);
void *thread3(void *);

int main(int argc, char **argv)
{
  int counter;
  int iterations=10000;

  sthread_init();
    
  if (sthread_create(thread0, (void*)1, 1, 2, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(-1);
  }
  if (sthread_create(thread1, (void*)1, 2, 1, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(-1);
  }
    
  if (sthread_create(thread2, (void*)1, 2, 5, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(-1);
  }
    
  if (sthread_create(thread3, (void*)1, 2, 7, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(-1);
  }
    
    
  printf("created threads\n");

  for(; iterations > 0; iterations--)
    for(; counter < 300000000; counter++);

  printf("prio0: %i\n", prio0);
  printf("prio1: %i\n", prio1);
  printf("prio2: %i\n", prio2);
  printf("prio3: %i\n", prio3);

  if (prio0 == 6 && prio1 == 6 && prio2 == 7 && prio3 == 10)
    printf("PASSED\n");
  else
    printf("FAILED\n");

  printf("counters are 1: %i, 2: %i, 3: %i 4: %i\n", counter0, counter1, counter2, counter3);

  sthread_dump();
  return 0;
}

void *thread0(void *arg)
{
  int i;
  prio0 = sthread_nice(4);
  for (;;counter0++)
    for (i = 0; i < 1000000; i++);

  return 0;
}

void *thread1(void *arg)
{
  int i;
  prio1 = sthread_nice(5);
  for (;;counter1++)
    for (i = 0; i < 1000000; i++);

  return 0;
}

void *thread2(void *arg)
{
  int i;
  prio2 = sthread_nice(2);
  for (;;counter2++)
    for (i = 0; i < 1000000; i++);

  return 0;
}

void *thread3(void *arg)
{
  int i;
  prio3 = sthread_nice(2);
  prio3 = sthread_nice(3);
  for (;;counter3++)
    for (i = 0; i < 1000000; i++);

  return 0;
}
