/*
 * Threads with different priorities 
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sthread.h>
#include <unistd.h>
#include <sys/wait.h>

int counter[6];

void *thread_1(void *);

static int priorityArray[6]={1,1,5,2,5,10};


void *thread_1(void* arg)
{
  int i,ind;

  int* index = (int *)arg;
  ind = *index;

  for (;;counter[ind]++)
    for (i = 0; i < 10000; i++);
  return 0;
}


int main(int argc, char **argv)
{
   int i,cID[6],index[6],count[6];
   int counter0 = 0;
   int iterations = 10000000;
  
   for (i=0;i<6;i++)
     counter[i] = 0;

   sthread_init();

   for (i=0;i<6;i++)
   {
    index[i]=i;
    cID[i]=(i>0)+(i>2)+1;

    if (sthread_create(thread_1, (void*)&index[i], cID[i], priorityArray[i], 0) == NULL) {
     printf("sthread_create failed\n");
     exit(1);
    }

    printf("created thread with cID %d and priority %d \n", cID[i], priorityArray[i]);
   }

   for(; iterations > 0; iterations--)
    for(; counter0 < 300000000; counter0++);

   for (i=0;i<6;i++)
     count[i] = counter[i];
   
   sthread_dump();
   printf("All thread must have approximately the same vruntime and a runtime = vruntime / #client to PASSE\n");

   printf("priorities at thread with cID %d are 1: %i \n", 1, priorityArray[0]);
   printf("counters at thread with cID %d are 1: %i \n", 1, count[0]);
   printf("priorities at thread with cID %d are 1: %i, 2: %i\n", 2, priorityArray[1], priorityArray[2]);
   printf("counters at thread with cID %d are 1: %i, 2: %i \n", 2, count[1],count[2]);
   printf("priorities at thread with cID %d are 1: %i, 2: %i, 3: %i\n", 3, priorityArray[3], priorityArray[4], priorityArray[5]);
   printf("counters at thread with cID %d are 1: %i, 2: %i, 3: %i\n", 3, count[3],count[4],count[5]);

   int c0=count[0]*priorityArray[0];
   int c1=count[1]*priorityArray[1]+count[2]*priorityArray[2];
   int c2=count[3]*priorityArray[3]+count[4]*priorityArray[4]+count[5]*priorityArray[5];
   float r2=((float)c2)/c1;
   float r1=((float)c1)/c0;
   printf("total cID 1: %i, total CID 2: %i, total cID 3: %i\n", c0, c1, c2);
   printf("3/2: %f, 2/1: %f\n", r2,r1);

   if (r2 >= 0.5 && r2 <= 1.6 && r1 >= 0.5 && r1 <= 1.6)
    printf("PASSED\n");
   else
    printf("NOT PASSED\n");
 
   return 0;
}

