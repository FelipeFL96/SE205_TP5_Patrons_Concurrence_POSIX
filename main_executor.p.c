#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "executor.h"
#include "scenario.h"
#include "utils.h"

callable_t * callables;
future_t ** futures;

void * main_job (void * arg) {
  job_t * job = (job_t *) arg;
  struct timespec ts1, ts2;
  
  ts1.tv_sec  = job->exec_time / 1000;
  ts1.tv_nsec = (job->exec_time % 1000) * 1000000;

  printf("%06ld [main_job] initiate execution=%ld period=%ld\n",
         relative_clock(), job->exec_time, period);
  nanosleep(&ts1, &ts2);
  printf("%06ld [main_job] complete execution=%ld period=%ld\n",
         relative_clock(), job->exec_time, period);
  return NULL;
}

int main(int argc, char *argv[]) {
  int i;

  if (argc != 2) {
    printf("Usage : %s <scenario file>\n", argv[0]);
    exit(1);
  }

  init_utils();

  // Read the configuration parameters of the scenario
  readFile(argv[1]);

  // To each job is associated a callable and a future. Callables and
  // Futures correspond to their definition in Java Executor. A
  // Callable is similar to a Runnable for which a result is produced
  // and stored in a Future when available.
  callables = (callable_t *) malloc(sizeof(callable_t) * job_table_size);
  futures = (future_t **) malloc(sizeof(future_t *) * job_table_size);

  set_start_time();

  // Create an executor composed of a thread pool configured with
  // core_pool_size and max_pool_size, a blocking queue (for pending
  // callables) of size blocking_queue_size and a timeout
  // keep_alive_time used to detect the idle threads that have to be
  // deallocated.
  executor_t * executor =
    executor_init
    (core_pool_size,
     max_pool_size,
     keep_alive_time,
     blocking_queue_size);

  // Each job is associated to a callable. This callable is submitted
  // to the executor which will execute it when a thread from its
  // threadpool becomes available.
  for (i = 0; i < job_table_size; i++) {
    callables[i].params = (void *) &jobs[i];
    callables[i].main   = main_job;
    callables[i].period = period;

    // Submit callable to executor
    futures[i] = submit_callable (executor, &callables[i]);
    if (futures[i] == NULL)
      printf ("%06ld [submit_callable] id %d failed\n", relative_clock(), i);
    else
      printf ("%06ld [submit_callable] id %d\n", relative_clock(), i);
  }

  // When the callables are periodic, there is no result to wait for.
  if (period == 0) {
    void * result;
    for (i = 0; i < job_table_size; i++) {
      if (futures[i] != NULL) {
        
        // Get result from future associated to callable. Suspend until
        // result becomes available.
        result = get_callable_result (futures[i]);
        printf ("%06ld [get_callable_result] id %d\n", relative_clock(), i);
      }
    }
  }
  sleep (10);
  executor_shutdown(executor);
}


