/*
 * test-mutex.c
 *
 * Simple test of mutexes. Checks that they do, in fact, provide 
 * mutual exclusion.
 *
 * Mutexes are implemented as semaphores initialized with 1 unit.
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sthread.h>

#define LENGHT 9

static int count, numWrites;
static int vect[LENGHT+1] ;
static sthread_rwlock_t rwlock;
static int t1_complete = 0;
static int tw_complete = 0;
static int failed = 0;
 

void *thread_writer(void *);
void *thread_reader(void *);


int main(int argc, char **argv)
{
	int i;
	
	printf("Testing sthread_rwlock_*, impl: %s\n",
		   (sthread_get_impl() == STHREAD_PTHREAD_IMPL) ? "pthread" : "user");
	
	for(i=0 ; i<LENGHT; i++){
	   vect[i]= -1;
	}/*for*/
	
	sthread_init();
	
	rwlock = sthread_rwlock_init();

	count = argc > 1 ? atoi(argv[1]) : 1;     /* number of write and read threads*/
	numWrites = argc > 2 ? atoi(argv[2]) : 25;  /* number of writes*/

   printf("Testing readwrite lock impl: %s\n",
      (sthread_get_impl() == STHREAD_PTHREAD_IMPL) ? "pthread" : "user");

   sthread_init();

   for(i=1; i< count+1 ; i++){

       if (sthread_create(thread_reader, (void*)numWrites, 1, 1,0) == NULL) {
	   printf("sthread_create failed\n");
	   exit(1);
       }
       
       if (sthread_create(thread_writer, (void*)numWrites, 1, 1,0) == NULL) {
	   printf("sthread_create failed\n");
	   exit(1);
       }
   }/*for*/
 
  printf("created %d threads\n", 2*count);

   while(!t1_complete || !(tw_complete <= count));
//   for(;;);
   //sthread_sleep(30000);
   if (0 == failed) {printf("PASSED the test\n");}
   else {printf("FAILED the test\n");}

   return 0;
}


/*  */
void *thread_reader(void *arg){
	
	int tmp, j;
		   	   
	while( tmp != (int)arg -1){
	    sthread_rwlock_rdlock(rwlock);
//	    printf(".");
	    tmp = vect[0];
	    for (j=0; j <  LENGHT; j++){
		if(vect[j] != tmp){
		    failed = 1; /* failed the test */
//		    printf("error: thread %d read wrong value %d in pos %d - >>>>> FAILED <<<<<\n",(int) arg, vect[i], i);
		    return 0;
		}
	   }	   
	    sthread_rwlock_unlock(rwlock);
	}/*for*/
	t1_complete = 1;
return 0;
}

/*  */
void *thread_writer(void *arg){
    int i,j, tmp;
    		   	   
	for(i= 0; i<  (int) arg; i++){
	   sthread_rwlock_wrlock(rwlock);
	   tmp = vect[0];
	   for (j=0; j <  LENGHT; j++){
	       if(vect[j] != tmp){
		   failed = 1; /* failed the test */
//		   printf("error: thread %d read wrong value %d in pos %d - >>>>> FAILED <<<<<\n",(int) arg, vect[i], i);
	       }
	   }	   
	   for (j=0; j < LENGHT; j++){
	       vect[j] = i; 
	       sthread_sleep(50);
	   }	   
	   sthread_rwlock_unlock(rwlock);
	   printf("\nWrote %d", i);
	}/*for*/

	printf("\nFinished writing\n");
	sthread_rwlock_wrlock(rwlock);
	tw_complete ++;
	sthread_rwlock_unlock(rwlock);
	return 0;
}
