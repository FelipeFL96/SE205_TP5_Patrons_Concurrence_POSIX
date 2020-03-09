#include <stdio.h>
#include <sys/time.h>

#include "executor.h"
#include "utils.h"

pthread_mutex_t mts0;
pthread_cond_t  cvts0;

// Main for threads executing callables
void * main_pool_thread (void * arg);

// Allocate and initialize executor. First, allocate and initialize a
// thread pool. Second, allocate and initialize a blocking queue to
// store pending callables.
executor_t * executor_init (int core_pool_size,
			    int max_pool_size,
			    long keep_alive_time,
			    int callable_array_size) {
  executor_t * executor;
  executor = (executor_t *) malloc (sizeof(executor_t));

  executor->keep_alive_time = keep_alive_time;
  executor->thread_pool = thread_pool_init (core_pool_size, max_pool_size);
  // Create a protected buffer for futures. Use the implementation
  // based on cond variables (first parameter sem_impl set to false).
  executor->futures = protected_buffer_init (0, callable_array_size);
  
  return executor;
}

// Associate a thread from thread pool to callable. Then invoke
// callable. Otherwise, store it in the blocking queue.
future_t * submit_callable (executor_t * executor, callable_t * callable) {
  future_t * future = (future_t *) malloc (sizeof(future_t));

  callable->executor = executor;
  future->callable  = callable;
  future->completed = 0;

  // Future must include synchronisation objects to block threads
  // until the result of the callable computation becames available.

  // Try to create a thread, but do not force to exceed core_pool_size
  // (last parameter set to false).
  if (pool_thread_create (executor->thread_pool, main_pool_thread, future, 0))
    return future;
  
  // When there are already enough created threads, queue the callable
  // in the blocking queue.

  // When the queue is full, pop the first future from the queue and
  // push the current one.
  future_t * first = protected_buffer_remove(executor->futures);
  if (first != NULL) {
    protected_buffer_add(executor->futures, future);
    future = first;
  }
  
  // Try to create a thread, but allow to exceed core_pool_size (last
  // parameter set to true).
  return NULL;
}

// Get result from callable execution. Block if not available.
void * get_callable_result (future_t * future) {
  void * result;

  // Protect against concurrent accesses. Block until the callable has
  // completed.

  result = (void *) future->result;
  
  // Unprotect against concurrent accesses

  // Do not bother to deallocate future
  return result;
}

// Define main procedure to execute callables. The arg parameter
// provides the first future object to be executed. Once it is
// executed, the main procedure may pick a pending callable from the
// executor blocking queue.
void * main_pool_thread (void * arg) {
  future_t           * future = (future_t *) arg;
  callable_t         * callable;
  executor_t         * executor;
  struct timespec      ts_deadline;
  struct timeval       tv_deadline;

  gettimeofday (&tv_deadline, NULL);
  TIMEVAL_TO_TIMESPEC (&tv_deadline, &ts_deadline);

  while (future != NULL) {
    callable = (callable_t *) future->callable;
    executor = (executor_t *) callable->executor;
    
    while (1) {
      future->result = callable->main (callable->params);

      // When the callable is not periodic, leave first inner
      // loop. The callable will not be executed again.
      if (callable->period == 0) {
        
        // As the callable is completed, the completed attribute and
        // the synchronisation objects should be updated to resume
        // threads waiting for the result.
        
        break;
      }

      // When the callable is periodic, wait for the next release time.

      // Even when this callable is periodic, check whether the
      // executor requested a shutdown
      if (get_shutdown(executor->thread_pool)) break;
    }

    future = NULL;
    if (executor->keep_alive_time == FOREVER) {
      // If the executor does not deallocate pool threads after being
      // inactive for a xhile, just wait for the next available
      // callable / future.
      
      // If there is no callable to handle, remove the current pool
      // thread from the pool. 
      if ((future == NULL) && pool_thread_remove(executor->thread_pool))
        break;
      
    } else {
      // If the executor is configured to release a thread when it is
      // idle for keep_alive_time milliseconds, try to get a new
      // callable / future during at most keep_alive_time ms.
      
      // If there is no callable to handle, remove the current pool
      // thread from the pool. And then, complete.
      if ((future == NULL) && pool_thread_remove (executor->thread_pool))
        break;

    }
  }
  return NULL;
}

// Wait for pool threads to be completed
void executor_shutdown (executor_t * executor) {
  thread_pool_t * thread_pool = executor->thread_pool;
  thread_pool_shutdown(thread_pool);
  
  // Fill the queue of null futures to unblock potential threads
  wait_thread_pool_empty(executor->thread_pool);
  printf ("%06ld [executor_shutdown]\n", relative_clock());
}

