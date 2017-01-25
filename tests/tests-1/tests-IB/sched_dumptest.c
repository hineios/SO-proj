/*
 * Threads with different priorities testing dump
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sthread.h>

void *thread_1(void *);
void *thread_2(void *);
void *thread_3(void *);

int main(int argc, char **argv)
{
  sthread_init();

  if (sthread_create(thread_1, (void*)0, 1, 10, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(1);
  }

  if (sthread_create(thread_2, (void*)0, 1, 5, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(1);
  }

  if (sthread_create(thread_3, (void*)0, 1, 2, 0) == NULL) {
    printf("sthread_create failed\n");
    exit(1);
  }

  printf("created threads\n");


  sthread_dump();

  printf("dump should be:\n1 active thread with priority 1\n3 executable threads with prioritis 2, 5, 10\n");

  return 0;
}


void *thread_1(void *arg)
{
  int i;

  for (i = 0; i < 1000000; i++);

  return 0;
}

void *thread_2(void *arg)
{
  int i;

  for (i = 0; i < 1000000; i++);

  return 0;
}

void *thread_3(void *arg)
{
  int i;

  for (i = 0; i < 1000000; i++);

  return 0;
}


