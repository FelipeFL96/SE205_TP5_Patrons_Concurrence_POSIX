#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "circular_buffer.h"
#include "protected_buffer.h"
#include "utils.h"

#define EMPTY_SLOTS_NAME "/empty_slots"
#define FULL_SLOTS_NAME "/full_slots"

// Initialise the protected buffer structure above. 
protected_buffer_t * sem_protected_buffer_init(int length) {
  protected_buffer_t * b;
  b = (protected_buffer_t *)malloc(sizeof(protected_buffer_t));
  b->buffer = circular_buffer_init(length);
  // Initialize the synchronization attributes
  // Use these filenames as named semaphores
  pthread_mutex_init(&b->mutex, NULL);
  sem_unlink (EMPTY_SLOTS_NAME);
  sem_unlink (FULL_SLOTS_NAME);
  b->empty_slots = sem_open(EMPTY_SLOTS_NAME, O_CREAT, 0660, b->buffer->max_size);
  b->full_slots = sem_open(FULL_SLOTS_NAME, O_CREAT, 0660, 0);
  return b;
}

// Extract an element from buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void * sem_protected_buffer_get(protected_buffer_t * b){
  void * d;
  
  // Enforce synchronisation semantics using semaphores.
  sem_wait(b->full_slots);

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mutex);
  
  d = circular_buffer_get(b->buffer);
  print_task_activity ("get", d);

  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->empty_slots);
  
  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void sem_protected_buffer_put(protected_buffer_t * b, void * d){

  // Enforce synchronisation semantics using semaphores.
  sem_wait(b->empty_slots);

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mutex);
  
  circular_buffer_put(b->buffer, d);
  print_task_activity ("put", d);

  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->full_slots);
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, return NULL. Otherwise, return the element.
void * sem_protected_buffer_remove(protected_buffer_t * b){
  void * d = NULL;
  int    rc = -1;
  
  // Enforce synchronisation semantics using semaphores.
  rc = sem_trywait(b->full_slots);

  if (rc != 0) {
    print_task_activity ("remove", d);
    return d;
  }

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mutex);
  
  d = circular_buffer_get(b->buffer);
  print_task_activity ("remove", d);

  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->empty_slots);
  
  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, return 0. Otherwise, return 1.
int sem_protected_buffer_add(protected_buffer_t * b, void * d){
  int rc = -1;
  
  // Enforce synchronisation semantics using semaphores.
  rc = sem_trywait(b->empty_slots);
  
  if (rc != 0) {
    print_task_activity ("add", NULL);
    return 0;
  }

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mutex);
  
  circular_buffer_put(b->buffer, d);
  print_task_activity ("add", d);
  
  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->full_slots);
  return 1;
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return the element if
// successful. Otherwise, return NULL.
void * sem_protected_buffer_poll(protected_buffer_t * b, struct timespec *abstime){
  void * d = NULL;
  int    rc = -1;
  
  // Enforce synchronisation semantics using semaphores.
  rc = sem_timedwait(b->full_slots, abstime);

  if (rc != 0) {
    print_task_activity ("poll", d);
    return d;
  }

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mutex);
  
  d = circular_buffer_get(b->buffer);
  print_task_activity ("poll", d);

  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->empty_slots);

  return d;
}

// Insert an element into buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return 0 if not
// successful. Otherwise, return 1.
int sem_protected_buffer_offer(protected_buffer_t * b, void * d, struct timespec * abstime){
  int rc = -1;
  
  // Enforce synchronisation semantics using semaphores.
  rc = sem_timedwait(b->empty_slots, abstime);
  
  if (rc != 0) {
    d = NULL;
    print_task_activity ("offer", d);
    return 0;
  }

  // Enter mutual exclusion.
  pthread_mutex_lock(&b->mutex);
  
  circular_buffer_put(b->buffer, d);
  print_task_activity ("offer", d);

  // Leave mutual exclusion.
  pthread_mutex_unlock(&b->mutex);

  // Enforce synchronisation semantics using semaphores.
  sem_post(b->full_slots);

  return 1;
}

