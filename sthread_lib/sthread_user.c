/* Simplethreads Instructional Thread Package
 * 
 * sthread_user.c - Implements the sthread API using user-level threads.
 *
 *    You need to implement the routines in this file.
 *
 * Change Log:
 * 2002-04-15        rick
 *   - Initial version.
 * 2005-10-12        jccc
 *   - Added semaphores, deleted conditional variables
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <sthread.h>
#include <sthread_user.h>
#include <sthread_ctx.h>
#include <sthread_time_slice.h>
#include <sthread_user.h>
#include "queue.h"


struct _sthread {
  sthread_ctx_t *saved_ctx;
  sthread_start_func_t start_routine_ptr;
  long wake_time;
  int join_tid;
  void* join_ret;
  void* args;
  int tid;          /* meramente informativo */
  unsigned int clientID;
  int prio;
  int nice;
  unsigned long state;
  unsigned long vruntime;
  unsigned long runtime;
  int min_delay;
  int initprio;
  unsigned long waittime;
  unsigned long sleeptime;
};


static queue_t *exe_thr_list;         /* lista de threads executaveis */
static queue_t *dead_thr_list;        /* lista de threads "mortas" */
static queue_t *sleep_thr_list;
static queue_t *join_thr_list;
static queue_t *zombie_thr_list;
static struct _sthread *active_thr;   /* thread activa */
static int tid_gen;                   /* gerador de tid's */


#define CLOCK_TICK 10000
static long Clock;


/*********************************************************************/
/* Part 1: Creating and Scheduling Threads                           */
/*********************************************************************/

int sthread_nice(int nice){
	if(nice >=0 && nice<=10)
	  active_thr->nice = nice;
	  
	return active_thr->nice+active_thr->initprio;
}

void sthread_user_free(struct _sthread *thread);

void sthread_aux_start(void){
  splx(LOW);
  active_thr->start_routine_ptr(active_thr->args);
  sthread_user_exit((void*)0);
}

void sthread_user_dispatcher(void);

void sthread_user_init(void) {

  exe_thr_list = create_queue();
  dead_thr_list = create_queue();
  sleep_thr_list = create_queue();
  join_thr_list = create_queue();
  zombie_thr_list = create_queue();
  tid_gen = 1;

  struct _sthread *main_thread = malloc(sizeof(struct _sthread));
  main_thread->start_routine_ptr = NULL;
  main_thread->args = NULL;
  main_thread->saved_ctx = sthread_new_blank_ctx();
  main_thread->wake_time = 0;
  main_thread->join_tid = 0;
  main_thread->join_ret = NULL;
  main_thread->tid = tid_gen++;
  main_thread->clientID = 0;
  main_thread->prio = 10;
  main_thread->nice = 0;
  main_thread->state = 0;
  main_thread->vruntime = 0;
  main_thread->min_delay = 0;
  main_thread->waittime = 0;
  main_thread->sleeptime =0;
  main_thread->initprio=0;
  
  active_thr = main_thread;

  Clock = 1;
  sthread_time_slices_init(sthread_user_dispatcher, CLOCK_TICK);
}

sthread_t sthread_user_create(sthread_start_func_t start_routine, void *arg, int clientID, int priority, int nice) 
//sthread_t sthread_user_create(sthread_start_func_t start_routine, void *arg)
{
  struct _sthread *new_thread = (struct _sthread*)malloc(sizeof(struct _sthread));
  sthread_ctx_start_func_t func = sthread_aux_start;
  new_thread->args = arg;
  new_thread->start_routine_ptr = start_routine;
  new_thread->wake_time = 0;
  new_thread->join_tid = 0;
  new_thread->join_ret = NULL;
  new_thread->saved_ctx = sthread_new_ctx(func);
  new_thread->clientID = clientID;
  new_thread->nice = nice;
  new_thread->state = 0;
  new_thread->vruntime = 0;
  new_thread->min_delay = 0;
  new_thread->waittime = 0;
  new_thread->sleeptime =0;
  
  if( priority >= 1 && priority <=10){
    new_thread->prio = priority;
    new_thread->initprio = priority;
  }
  
  splx(HIGH);
  new_thread->tid = tid_gen++;
  queue_insert(exe_thr_list, new_thread);
  splx(LOW);
  return new_thread;
}


void sthread_user_exit(void *ret) {
  splx(HIGH);
   
   int is_zombie = 1;

   // unblock threads waiting in the join list
   queue_t *tmp_queue = create_queue();   
   while (!queue_is_empty(join_thr_list)) {
      struct _sthread *thread = queue_remove(join_thr_list);
     
      //printf("Test join list: join_tid=%d, active->tid=%d\n", thread->join_tid, active_thr->tid);

      if (thread->join_tid == active_thr->tid) {
         thread->join_ret = ret;
         queue_insert(exe_thr_list,thread);
         is_zombie = 0;
      } else {
         queue_insert(tmp_queue,thread);
      }
   }
   delete_queue(join_thr_list);
   join_thr_list = tmp_queue;
 
   if (is_zombie) {
      queue_insert(zombie_thr_list, active_thr);
   } else {
      queue_insert(dead_thr_list, active_thr);
   }
   

   if(queue_is_empty(exe_thr_list)){  /* pode acontecer se a unica thread em execucao fizer */
    free(exe_thr_list);              /* sthread_exit(0). Este codigo garante que o programa sai bem. */
    delete_queue(dead_thr_list);
    sthread_user_free(active_thr);
    printf("Exec queue is empty!\n");
    exit(0);
  }

  
   // remove from exec list
   struct _sthread *old_thr = active_thr;
   active_thr = queue_remove(exe_thr_list);
   sthread_switch(old_thr->saved_ctx, active_thr->saved_ctx);

   splx(LOW);
}


void sthread_user_dispatcher(void)
{
   Clock++;
   active_thr->min_delay++;
   active_thr->runtime++;
   int n_client_threads=0;
   

   /*percorre a lista de threads que fizeram sleep*/
  if(sleep_thr_list->first != NULL){
	   queue_element_t *ptr; 
		   
	   for(ptr = sleep_thr_list->first; ptr != NULL; ptr = ptr->next){
		   ptr->thread->sleeptime++;
		   ptr->thread->state++;
		   if(ptr->thread->clientID == active_thr->clientID)
		     n_client_threads++;
	   }
   }
   
   /*percorre a lista de threads que fizeram join*/
   if(join_thr_list->first != NULL){
	   queue_element_t *ptr; 
		   
	   for(ptr = join_thr_list->first; ptr != NULL; ptr = ptr->next){
		   ptr->thread->sleeptime++;
		   ptr->thread->state++;
		   if(ptr->thread->clientID == active_thr->clientID)
		     n_client_threads++;
	   }
   }
   
   /*percorre a lista de threads executaveis*/
   if(exe_thr_list->first != NULL){
	   queue_element_t *ptr; 
		   
	   for(ptr = exe_thr_list->first; ptr != NULL; ptr = ptr->next){
		   ptr->thread->waittime++;
		   ptr->thread->state++;
		   if(ptr->thread->clientID == active_thr->clientID)
		     n_client_threads++;
	   }
   }
   /*sizeof(struct queue_t)/sizeof(int)*/
   active_thr->vruntime = (active_thr->prio+active_thr->nice)*n_client_threads;
   active_thr->state++;	
   
   
   // Preempção
    if(!queue_is_empty(exe_thr_list)){ /* tarefas por executar?*/
      if(active_thr->min_delay >= 5)
        sthread_user_yield();
    }
   
   
   queue_t *tmp_queue = create_queue();

   while (!queue_is_empty(sleep_thr_list)) {
      struct _sthread *thread = queue_remove(sleep_thr_list);
      if (thread->wake_time == Clock) {
         thread->wake_time = 0;
         thread->state = 0;
         queue_insert(exe_thr_list,thread);
      } else {
         queue_insert(tmp_queue,thread);
      }
   } 
   delete_queue(sleep_thr_list);
   sleep_thr_list = tmp_queue;
   
   /*sthread_user_yield();*/
}

void sthread_user_yield(void)
{
    int int_cmp(const void *a, const void *b) 
   { 
    struct _sthread *ia = (struct _sthread *)a;
    struct _sthread *ib = (struct _sthread *)b;
    return (int)(ia->vruntime - ib->vruntime);
    }
   



  splx(HIGH);
  struct _sthread *old_thr;
  old_thr = active_thr;
  old_thr->state =  0;
  queue_insert(exe_thr_list, old_thr);
  qsort(exe_thr_list, sizeof(queue_t)/sizeof(struct _sthread), sizeof(struct _sthread), int_cmp);
  active_thr = queue_remove(exe_thr_list);
  active_thr->state = 0;
  sthread_switch(old_thr->saved_ctx, active_thr->saved_ctx);
  splx(LOW);
}




void sthread_user_free(struct _sthread *thread)
{
  sthread_free_ctx(thread->saved_ctx);
  free(thread);
}


/*********************************************************************/
/* Part 2: Join and Sleep Primitives                                 */
/*********************************************************************/

int sthread_user_join(sthread_t thread, void **value_ptr)
{
   /* suspends execution of the calling thread until the target thread
      terminates, unless the target thread has already terminated.
      On return from a successful pthread_join() call with a non-NULL 
      value_ptr argument, the value passed to pthread_exit() by the 
      terminating thread is made available in the location referenced 
      by value_ptr. When a pthread_join() returns successfully, the 
      target thread has been terminated. The results of multiple 
      simultaneous calls to pthread_join() specifying the same target 
      thread are undefined. If the thread calling pthread_join() is 
      canceled, then the target thread will not be detached. 

      If successful, the pthread_join() function returns zero. 
      Otherwise, an error number is returned to indicate the error. */

   
   splx(HIGH);
   // checks if the thread to wait is zombie
   int found = 0;
   queue_t *tmp_queue = create_queue();
   while (!queue_is_empty(zombie_thr_list)) {
      struct _sthread *zthread = queue_remove(zombie_thr_list);
      if (thread->tid == zthread->tid) {
         *value_ptr = thread->join_ret;
         queue_insert(dead_thr_list,thread);
         found = 1;
      } else {
         queue_insert(tmp_queue,thread);
      }
   }
   delete_queue(zombie_thr_list);
   zombie_thr_list = tmp_queue;
  
   if (found) {
       splx(LOW);
       return 0;
   }

   
   // search active queue
   if (active_thr->tid == thread->tid) {
      found = 1;
   }
   
   queue_element_t *qe = NULL;

   // search exe
   qe = exe_thr_list->first;
   while (!found && qe != NULL) {
      if (qe->thread->tid == thread->tid) {
         found = 1;
      }
      qe = qe->next;
   }

   // search sleep
   qe = sleep_thr_list->first;
   while (!found && qe != NULL) {
      if (qe->thread->tid == thread->tid) {
         found = 1;
      }
      qe = qe->next;
   }

   // search join
   qe = join_thr_list->first;
   while (!found && qe != NULL) {
      if (qe->thread->tid == thread->tid) {
         found = 1;
      }
      qe = qe->next;
   }

   // if found blocks until thread ends
   if (!found) {
      splx(LOW);
      return -1;
   } else {
      active_thr->join_tid = thread->tid;
      
      struct _sthread *old_thr = active_thr;
      old_thr->state = 0;
      queue_insert(join_thr_list, old_thr);
      active_thr = queue_remove(exe_thr_list);
      active_thr->state = 0;
      sthread_switch(old_thr->saved_ctx, active_thr->saved_ctx);
  
      *value_ptr = thread->join_ret;
   }
   
   splx(LOW);
   return 0;
}


/* minimum sleep time of 1 clocktick.
  1 clock tick, value 10 000 = 10 ms */

int sthread_user_sleep(int time)
{
   splx(HIGH);
   
   long num_ticks = time / CLOCK_TICK;
   if (num_ticks == 0) {
      splx(LOW);
      
      return 0;
   }
   
   active_thr->wake_time = Clock + num_ticks;
   active_thr->state = 0;
   queue_insert(sleep_thr_list,active_thr); 
   
   if(queue_is_empty(exe_thr_list)){  /* pode acontecer se a unica thread em execucao fizer */
       free(exe_thr_list);              /* sleep(x). Este codigo garante que o programa sai bem. */
       delete_queue(dead_thr_list);
       sthread_user_free(active_thr);
       printf("Exec queue is empty! too much sleep...\n");
       exit(0);
   }
   sthread_t old_thr = active_thr;
   active_thr = queue_remove(exe_thr_list);
   active_thr->state = 0;
   sthread_switch(old_thr->saved_ctx, active_thr->saved_ctx);
   
   splx(LOW);
   return 0;
}

/* --------------------------------------------------------------------------*
 * Synchronization Primitives                                                *
 * ------------------------------------------------------------------------- */

/*
 * Mutex implementation
 */

struct _sthread_mutex
{
  lock_t l;
  struct _sthread *thr;
  queue_t* queue;
};

sthread_mutex_t sthread_user_mutex_init()
{
  sthread_mutex_t lock;

  if(!(lock = malloc(sizeof(struct _sthread_mutex)))){
    printf("Error in creating mutex\n");
    return 0;
  }

  /* mutex initialization */
  lock->l=0;
  lock->thr = NULL;
  lock->queue = create_queue();
  
  return lock;
}

void sthread_user_mutex_free(sthread_mutex_t lock)
{
  delete_queue(lock->queue);
  free(lock);
}

void sthread_user_mutex_lock(sthread_mutex_t lock)
{

  while(atomic_test_and_set(&(lock->l))) {}

  if(lock->thr == NULL){
    lock->thr = active_thr;

    atomic_clear(&(lock->l));
    
  } else {
    queue_insert(lock->queue, active_thr);
   
    atomic_clear(&(lock->l));

    splx(HIGH);
    struct _sthread *old_thr;
    old_thr = active_thr;
    queue_insert(exe_thr_list, old_thr);
    active_thr = queue_remove(exe_thr_list);
    active_thr->state = 0;
    sthread_switch(old_thr->saved_ctx, active_thr->saved_ctx);
    
    splx(LOW);
  }
}

void sthread_user_mutex_unlock(sthread_mutex_t lock)
{
  
  if(lock->thr!=active_thr){
    printf("unlock without lock!\n");
    return;
  }

  while(atomic_test_and_set(&(lock->l))) {}

  if(queue_is_empty(lock->queue)){
    lock->thr = NULL;
    
  } else {
    lock->thr = queue_remove(lock->queue);
    lock->thr->state = 0;
    queue_insert(exe_thr_list, lock->thr);
   
  }

  atomic_clear(&(lock->l));
}

/*------------
Semaphore Implementation 
*/

struct _sthread_sem {
  int counter;
  int l;
  queue_t* queue_threads;
};

sthread_sem_t sthread_user_sem_init (unsigned int initial_count) {
  sthread_sem_t sem = malloc(sizeof(struct _sthread_sem));
  sem->counter = initial_count; 
  sem->queue_threads = create_queue();
  sem->l= 0;
    
  return sem;
}

void sthread_user_sem_wait (sthread_sem_t s) {
  while(atomic_test_and_set(&(s->l))) {}


  if(s->counter > 0) {
    --s->counter;
    atomic_clear(&(s->l));
  } else {
    active_thr->state = 0;
    queue_insert(s->queue_threads, active_thr);
    atomic_clear(&(s->l));

    splx(HIGH);
    struct _sthread *old_thr;
    old_thr = active_thr;
    active_thr = queue_remove(exe_thr_list);
    active_thr->state = 0;
    sthread_switch(old_thr->saved_ctx, active_thr->saved_ctx);
    splx(LOW);
  }
}

void sthread_user_sem_post (sthread_sem_t s) {
  
  while(atomic_test_and_set(&(s->l))) { }

  if (!queue_is_empty(s->queue_threads)) {
    struct _sthread *new_thread = queue_remove(s->queue_threads);
    new_thread->state = 0;
    queue_insert(exe_thr_list, new_thread);
  } else { 
    ++s->counter;
    
  }
  atomic_clear(&(s->l));
}

void sthread_user_sem_destroy (sthread_sem_t s){
  delete_queue(s->queue_threads);
  free(s);
}


/*
 * Readers/Writer implementation
 */

struct _sthread_rwlock {
    int nleitores; 
    int /*boolean_t*/ em_escrita;
    int leitores_espera;
    int escritores_espera;
    sthread_mutex_t m;
    sthread_sem_t leitores;
    sthread_sem_t escritores;
};

void sthread_user_rwlock_destroy(sthread_rwlock_t rwlock){

    sthread_user_mutex_free(rwlock->m);
    sthread_user_sem_destroy(rwlock->leitores);
    sthread_user_sem_destroy(rwlock->escritores);
    free(rwlock);

}

sthread_rwlock_t sthread_user_rwlock_init(){

    sthread_rwlock_t rwlock;

  if(!(rwlock = malloc(sizeof(struct _sthread_rwlock)))){
    printf("Error in creating rwlock\n");
    return 0;
  }

    rwlock->leitores = sthread_user_sem_init(0) ;
    rwlock->escritores = sthread_user_sem_init(0);
    rwlock->m = sthread_user_mutex_init();
    rwlock->nleitores=0;
    rwlock->em_escrita=0; /*FALSE;*/
    rwlock->leitores_espera=0;
    rwlock->escritores_espera=0;
  
    return rwlock;

}

void sthread_user_rwlock_rdlock(sthread_rwlock_t rwlock){

    sthread_user_mutex_lock(rwlock->m);
    if (rwlock->em_escrita || rwlock->escritores_espera > 0) {
			rwlock->leitores_espera++;
			sthread_user_mutex_unlock(rwlock->m);
			sthread_user_sem_wait(rwlock->leitores);
			sthread_user_mutex_lock(rwlock->m);
    }
    else{
			rwlock->nleitores++;
    }
    sthread_user_mutex_unlock(rwlock->m);

}

void sthread_user_rwlock_wrlock(sthread_rwlock_t rwlock){

    sthread_user_mutex_lock(rwlock->m);
    if (rwlock->em_escrita || rwlock->nleitores > 0) {
			rwlock->escritores_espera++;
			sthread_user_mutex_unlock(rwlock->m);
			sthread_user_sem_wait(rwlock->escritores);
			sthread_user_mutex_lock(rwlock->m);
    }
    else{
			rwlock->em_escrita = 1; /*TRUE;*/
    }
    sthread_user_mutex_unlock(rwlock->m);   
   
}


void sthread_user_rwlock_unlock(sthread_rwlock_t rwlock){
    int i;
    sthread_user_mutex_lock(rwlock->m);
    
    if (/*TRUE*/ 1== rwlock->em_escrita) { /* writer unlock*/
	
	rwlock->em_escrita = 0; /*FALSE; */
	if (rwlock->leitores_espera > 0){
	    for (i=0; i< rwlock->leitores_espera; i++) {
		sthread_user_sem_post(rwlock->leitores);
		rwlock->nleitores++;
	    }
	    rwlock->leitores_espera -= i;
        }else{
           if (rwlock->escritores_espera > 0) {
	     sthread_user_sem_post(rwlock->escritores);
	     rwlock->em_escrita=1; /*TRUE;*/
	     rwlock->escritores_espera--;
	    }
	}
    }else{ /* reader unlock*/
	
	rwlock->nleitores--;
	if (rwlock->nleitores == 0 && rwlock->escritores_espera > 0){
	    sthread_user_sem_post(rwlock->escritores);
	    rwlock->em_escrita=1; /*TRUE;*/
	    rwlock->escritores_espera--;
	}
    }
    sthread_user_mutex_unlock(rwlock->m);    
}

/*
 * Monitor implementation
 */

struct _sthread_mon {
 	sthread_mutex_t mutex;
	queue_t* queue;
};

sthread_mon_t sthread_user_monitor_init()
{
  sthread_mon_t mon;
  if(!(mon = malloc(sizeof(struct _sthread_mon)))){
    printf("Error creating monitor\n");
    return 0;
  }

  mon->mutex = sthread_user_mutex_init();
  mon->queue = create_queue();
  return mon;
}

void sthread_user_monitor_free(sthread_mon_t mon)
{
  sthread_user_mutex_free(mon->mutex);
  delete_queue(mon->queue);
  free(mon);
}

void sthread_user_monitor_enter(sthread_mon_t mon)
{
  sthread_user_mutex_lock(mon->mutex);
}

void sthread_user_monitor_exit(sthread_mon_t mon)
{
  sthread_user_mutex_unlock(mon->mutex);
}

void sthread_user_monitor_wait(sthread_mon_t mon)
{
  struct _sthread *temp;

  if(mon->mutex->thr != active_thr){
    printf("monitor wait called outside monitor\n");
    return;
  }

  /* inserts thread in queue of blocked threads */
  temp = active_thr;
  queue_insert(mon->queue, temp);

  /* exits mutual exclusion region */
  sthread_user_mutex_unlock(mon->mutex);

  splx(HIGH);
  struct _sthread *old_thr;
  old_thr = active_thr;
  active_thr = queue_remove(exe_thr_list);
  sthread_switch(old_thr->saved_ctx, active_thr->saved_ctx);
  splx(LOW);
}

void sthread_user_monitor_signal(sthread_mon_t mon)
{
  struct _sthread *temp;

  if(mon->mutex->thr != active_thr){
    printf("monitor signal called outside monitor\n");
    return;
  }

  while(atomic_test_and_set(&(mon->mutex->l))) {}
  if(!queue_is_empty(mon->queue)){
    /* changes blocking queue for thread */
    temp = queue_remove(mon->queue);
    queue_insert(mon->mutex->queue, temp);
  }
  atomic_clear(&(mon->mutex->l));
}

void sthread_user_monitor_signalall(sthread_mon_t mon)
{
  struct _sthread *temp;

  if(mon->mutex->thr != active_thr){
    printf("monitor signalall called outside monitor\n");
    return;
  }

  while(atomic_test_and_set(&(mon->mutex->l))) {}
  while(!queue_is_empty(mon->queue)){
    /* changes blocking queue for thread */
    temp = queue_remove(mon->queue);
    queue_insert(mon->mutex->queue, temp);
  }
  atomic_clear(&(mon->mutex->l));
}


/* The following functions are dummies to 
 * highlight the fact that pthreads do not
 * include monitors.
 */

sthread_mon_t sthread_dummy_monitor_init()
{
   printf("WARNING: pthreads do not include monitors!\n");
   return NULL;
}


void sthread_dummy_monitor_free(sthread_mon_t mon)
{
   printf("WARNING: pthreads do not include monitors!\n");
}


void sthread_dummy_monitor_enter(sthread_mon_t mon)
{
   printf("WARNING: pthreads do not include monitors!\n");
}


void sthread_dummy_monitor_exit(sthread_mon_t mon)
{
   printf("WARNING: pthreads do not include monitors!\n");
}


void sthread_dummy_monitor_wait(sthread_mon_t mon)
{
   printf("WARNING: pthreads do not include monitors!\n");
}


void sthread_dummy_monitor_signal(sthread_mon_t mon)
{
   printf("WARNING: pthreads do not include monitors!\n");
}

void sthread_dummy_monitor_signalall(sthread_mon_t mon)
{
   printf("WARNING: pthreads do not include monitors!\n");
}
/*******************************************************/
/*          OUTPUT: função dump                        */
/*******************************************************/

void sthread_dump(){


	printf("\n=== dump start ===\n\nactive thread\n\n");
	if(active_thr != NULL){
		printf("id: %d  request: %d  priority: %d ", active_thr->clientID, 
	  active_thr->tid ,(active_thr->prio + active_thr->nice));
	  printf("vruntime: %ld  runtime: %ld on state for: %ld sleeptime: %ld waittime: %ld\n\n", (active_thr->vruntime) ,
	  (active_thr->runtime*CLOCK_TICK), (active_thr->state*CLOCK_TICK) ,(active_thr->sleeptime*CLOCK_TICK), 
	  (active_thr->waittime*CLOCK_TICK));
	}
	else 
	printf("No active thread\n\n");
	
	printf(">>>>Scheduler<<<<\n\n");
	if(!queue_is_empty(exe_thr_list)){
	  queue_element_t *ptr;
	  for(ptr = exe_thr_list->first; ptr != NULL; ptr = ptr->next){
			printf("id: %d request: %d priority: %d ", ptr->thread->clientID, ptr->thread->tid,
		   ptr->thread->prio);
			printf("vruntime: %ld ", ptr->thread->vruntime);
		  printf("runtime: %ld on state for: %ld sleeptime: %ld waittime: %ld\n\n", (ptr->thread->runtime*CLOCK_TICK), 
		  (ptr->thread->state*CLOCK_TICK), (ptr->thread->sleeptime*CLOCK_TICK), (ptr->thread->waittime*CLOCK_TICK));
		}
	}
	else 
	printf("No threads in execute list\n\n");
	
	
	printf(">>>>BlockedList<<<<\n\n");
	if(!queue_is_empty(join_thr_list)){
	  queue_element_t *ptr;
	  for(ptr = join_thr_list->first; ptr != NULL; ptr = ptr->next){
			printf("id: %d request: %d priority: %d ", ptr->thread->clientID, ptr->thread->tid,
		   ptr->thread->prio);
			printf("vruntime: %ld", ptr->thread->vruntime);
		  printf("runtime: %ld on state for: %ld sleeptime: %ld waittime: %ld\n\n", (ptr->thread->runtime*CLOCK_TICK),
		  (ptr->thread->state*CLOCK_TICK), (ptr->thread->sleeptime*CLOCK_TICK), (ptr->thread->waittime*CLOCK_TICK));
		}
	}
	else
	printf("No threads in blocked list\n\n");
	
	
	printf(">>>>SleepList<<<<\n\n");
	if(!queue_is_empty(sleep_thr_list)){
		queue_element_t *ptr; 
		   
		for(ptr = sleep_thr_list->first; ptr != NULL; ptr = ptr->next){
			printf("id: %d request: %d priority: %d ", ptr->thread->clientID, ptr->thread->tid,
		   ptr->thread->prio);
			printf("vruntime: %ld", ptr->thread->vruntime);
		  printf("runtime: %ld on state for:%ld sleeptime: %ld waittime: %ld\n\n", (ptr->thread->runtime*CLOCK_TICK), 
			(ptr->thread->state*CLOCK_TICK),(ptr->thread->sleeptime*CLOCK_TICK), (ptr->thread->waittime*CLOCK_TICK));
		}
	}
	else 
		printf("No threads in sleep list\n\n");
	
	printf("=== dump end ===\n\n");
}
