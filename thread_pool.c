#include <stdio.h>
#include <unistd.h>

#include "thread_pool.h"
#include "utils.h"

// Create a thread pool. This pool must be protected against
// concurrent accesses.
thread_pool_t * thread_pool_init(int core_pool_size, int max_pool_size) {
  thread_pool_t * thread_pool;

  thread_pool = (thread_pool_t *) malloc(sizeof(thread_pool_t));
  thread_pool->core_pool_size = core_pool_size;
  thread_pool->max_pool_size  = max_pool_size;
  thread_pool->size           = 0;
  return thread_pool;
}

// Create a thread. If the number of threads created is not greater
// than core_pool_size, create a new thread. If it is and force is set
// to true, create a new thread. If a thread is created, use run as a
// main procedure and future as run parameter.
int pool_thread_create (thread_pool_t * thread_pool,
			main_func_t     main,
			void          * future,
			int             force) {
  int done = 0;
  pthread_t thread;

  // Protect structure against concurrent accesses

  // Always create a thread as long as there are less then
  // core_pool_size threads created.
  if (thread_pool->size < thread_pool->core_pool_size) {
  }

  // Do not protect the structure against concurrent accesses anymore

  if (done)
    printf("%06ld [pool_thread] created\n", relative_clock());
  return done;
}

void thread_pool_shutdown(thread_pool_t * thread_pool) {
  thread_pool->shutdown = 1;
}

// When a thread wants to be deallocated, check whether the number of
// threads already allocated is large enough. If so, decrease threads
// number and broadcast update. Protect against concurrent accesses.
int pool_thread_remove (thread_pool_t * thread_pool) {
  int done = 1;

  // Protect against concurrent accesses and check whether the thread
  // can be deallocated.
  if (done)
    printf("%06ld [pool_thread] terminated\n", relative_clock());
  return done;
}  

// Wait until thread number equals zero. Protect the thread pool
// structure against concurrent accesses.
void wait_thread_pool_empty (thread_pool_t * thread_pool) {
}  

int get_shutdown(thread_pool_t * thread_pool) {
  return thread_pool->shutdown;
}
