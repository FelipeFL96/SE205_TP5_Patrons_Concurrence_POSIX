#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

// Start time as a timespec
struct timespec start_time;

void init_utils(){
}

// Add msec milliseconds to timespec ts (seconds, nanoseconds)
void add_millis_to_timespec (struct timespec * ts, long msec) {
  long nsec = (msec % (long) 1E3) * 1E6;
  long  sec = msec / 1E3;
  ts->tv_nsec = ts->tv_nsec + nsec;
  if (1E9 <= ts->tv_nsec) {
    ts->tv_nsec = ts->tv_nsec - 1E9;
    ts->tv_sec++;
  }
  ts->tv_sec = ts->tv_sec + sec;
}

// Delay until an absolute time. Translate the absolute time into a
// relative one and use nanosleep. This is incorrect (we fix that).
void delay_until(struct timespec * deadline) {
  struct timeval  tv_now;
  struct timespec ts_now;
  struct timespec ts_sleep;

  gettimeofday(&tv_now, NULL);
  TIMEVAL_TO_TIMESPEC(&tv_now, &ts_now);
  ts_sleep.tv_nsec = deadline->tv_nsec - ts_now.tv_nsec;
  ts_sleep.tv_sec = deadline->tv_sec - ts_now.tv_sec;
  if (ts_sleep.tv_nsec < 0) {
    ts_sleep.tv_nsec = 1E9 + ts_sleep.tv_nsec;
    ts_sleep.tv_sec--;
  }
  if (ts_sleep.tv_sec < 0) return;
  
  nanosleep (&ts_sleep, &ts_now);
}

// Compute time elapsed from start time
long relative_clock() {
  struct timeval  tv_now;
  struct timespec ts_now;

  gettimeofday(&tv_now, NULL);
  TIMEVAL_TO_TIMESPEC(&tv_now, &ts_now);
  
  ts_now.tv_nsec = ts_now.tv_nsec - start_time.tv_nsec;
  ts_now.tv_sec = ts_now.tv_sec - start_time.tv_sec;
  if (ts_now.tv_nsec < 0) {
    ts_now.tv_sec = ts_now.tv_sec - 1;
    ts_now.tv_nsec = ts_now.tv_nsec + 1E9;
  }
  return (ts_now.tv_sec * 1E3) + (ts_now.tv_nsec / 1E6);
}

// Return the start time
struct timespec get_start_time() {
  return start_time;
}

// Store current time as the start time
void set_start_time() {
  struct timeval  tv_start_time; // start time as a timeval
  
  gettimeofday(&tv_start_time, NULL);
  TIMEVAL_TO_TIMESPEC(&tv_start_time, &start_time);
}

// Read string in file f and store it in s. If there is an error,
// provide filename and line number (file:line).
int get_string (FILE * f, char * s, char * file, int line) {
  char b[64];
  char * c;;
  
  while (fgets (b, 64, f) != NULL) {
    c = strchr (b, '\n');
    *c = '\0';
    if (strcmp (s, b) == 0)
      return 1;
  }
  printf ("getString failed to catch %s in %s:%d\n", s, file, line);
  exit (1);
  return 0;
}

// Read long in file f and store it in l. If there is an error,
// provide filename and line number (file:line).
int get_long (FILE * f, long * l, char * file, int line) {
  char b[64];
  char * c;;
  
  if (fgets (b, 64, f) != NULL) {
    c = strchr (b, '\n');
    *c = '\0';
    *l = strtol (b, NULL, 10);
    if ((*l != 0) || (errno != EINVAL))
      return 1;
  }
  return 0;
}

void print_task_activity(char * action, int * data) {};

#ifdef DARWIN
int sem_timedwait(sem_t *restrict sem, const struct timespec * restrict abs_timeout){
  struct timeval  tv_now;
  struct timespec ts_now;
  
  while (1) {
    if (0 <= sem_trywait (sem)) {
      return 0;
    }

    // Poll every 1ms
    gettimeofday(&tv_now, NULL);
    tv_now.tv_usec = tv_now.tv_usec + 1E3;
    if (1E6 <= tv_now.tv_usec) {
      tv_now.tv_usec = tv_now.tv_usec - 1E6;
      tv_now.tv_sec++;
    }
    TIMEVAL_TO_TIMESPEC(&tv_now, &ts_now);
    if ((ts_now.tv_sec > abs_timeout->tv_sec) ||
	((ts_now.tv_sec == abs_timeout->tv_sec) &&
	 (ts_now.tv_nsec > abs_timeout->tv_nsec))) {
      return -1;
    }
    ts_now.tv_sec = 0;
    ts_now.tv_nsec = 1E6;
    nanosleep (&ts_now, NULL);
  }
}
int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex,
                            const struct timespec * restrict abs_timeout){
  struct timeval  tv_now;
  struct timespec ts_now;

  while (1) {
    if (0 <= pthread_mutex_trylock(mutex)) {
      return 0;
    }

    // Poll every 1ms
    gettimeofday(&tv_now, NULL);
    tv_now.tv_usec = tv_now.tv_usec + 1E3;
    if (1E6 <= tv_now.tv_usec) {
      tv_now.tv_usec = tv_now.tv_usec - 1E6;
      tv_now.tv_sec++;
    }
    TIMEVAL_TO_TIMESPEC(&tv_now, &ts_now);
    if ((ts_now.tv_sec > abs_timeout->tv_sec) ||
        ((ts_now.tv_sec == abs_timeout->tv_sec) &&
         (ts_now.tv_nsec > abs_timeout->tv_nsec))) {
      return -1;
    }
    ts_now.tv_sec = 0;
    ts_now.tv_nsec = 1E6;
    nanosleep (&ts_now, NULL);
  }
}
#endif
