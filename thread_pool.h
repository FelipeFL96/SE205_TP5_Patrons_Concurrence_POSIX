#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

#include "protected_buffer.h"

typedef void * (*main_func_t)(void *);

typedef struct {
  int             core_pool_size;
  int             max_pool_size;
  int             size;
  int             shutdown;
} thread_pool_t;

// Create a thread pool. This pool must be protected against
// concurrent accesses.
thread_pool_t * thread_pool_init(int core_pool_size, int max_pool_size);

// If the current thread pool size is not greater than core_pool_size,
// create a new thread. If it is and force is true, create a new
// thread as well. If a thread is created, increment the current
// thread pool size. Use main as a thread main procedure.
int pool_thread_create(thread_pool_t * thread_pool,
                       main_func_t     main,
                       void          * executor,
                       int             force);

// Shutdown
void thread_pool_shutdown(thread_pool_t * thread_pool);

// Getter
int get_shutdown(thread_pool_t * thread_pool);

// Decrease thread number and broadcast update. Return whether thread
// was actually removed.
int pool_thread_remove(thread_pool_t * thread_pool);

// Wait until thread number equals zero.
void wait_thread_pool_empty(thread_pool_t * thread_pool);
#endif
