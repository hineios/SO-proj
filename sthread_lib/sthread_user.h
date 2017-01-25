/*
 * sthread_user.h - This file defines the user-level thread
 *                  implementation of sthreads. The routines
 *                  are all described in the sthread.h file.
 *
 */

#ifndef STHREAD_USER_H
#define STHREAD_USER_H 1

/* Basic Threads */
void sthread_user_init(void);
sthread_t sthread_user_create(sthread_start_func_t start_routine, void *arg, int clientID, int priority, int nice);
//sthread_t sthread_user_create(sthread_start_func_t start_routine, void *arg);
void sthread_user_exit(void *ret);
void sthread_user_yield(void);

/* Advanced Threads */
int sthread_user_sleep(int time);
int sthread_user_join(sthread_t thread, void **value_ptr);


/* Synchronization Primitives */
sthread_mutex_t sthread_user_mutex_init(void);
void sthread_user_mutex_free(sthread_mutex_t lock);
void sthread_user_mutex_lock(sthread_mutex_t lock);
void sthread_user_mutex_unlock(sthread_mutex_t lock);

/* Semaphores primitives*/
sthread_sem_t sthread_user_sem_init (unsigned int initial_count);
void sthread_user_sem_wait (sthread_sem_t s);
void sthread_user_sem_post (sthread_sem_t s);
void sthread_user_sem_destroy (sthread_sem_t s);


/* ReadersWrite primitives*/

void sthread_user_rwlock_destroy(sthread_rwlock_t rwlock);

sthread_rwlock_t sthread_user_rwlock_init();

void sthread_user_rwlock_rdlock(sthread_rwlock_t rwlock);

void sthread_user_rwlock_wrlock(sthread_rwlock_t rwlock);

void sthread_user_rwlock_unlock(sthread_rwlock_t rwlock);

/************/



sthread_mon_t sthread_user_monitor_init();
void sthread_user_monitor_free(sthread_mon_t mon);
void sthread_user_monitor_enter(sthread_mon_t mon);
void sthread_user_monitor_exit(sthread_mon_t mon);
void sthread_user_monitor_wait(sthread_mon_t mon);
void sthread_user_monitor_signal(sthread_mon_t mon);
void sthread_user_monitor_signalall(sthread_mon_t mon);


#endif /* STHREAD_USER_H */
