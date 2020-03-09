#include "protected_buffer.h"

typedef unsigned long ulong;
typedef short bool;
typedef struct {
  int    id;
  long   exec_time;
} job_t;
  

extern long      job_table_size;
extern long      core_pool_size;
extern long      max_pool_size;
extern long      blocking_queue_size;
extern long      keep_alive_time;
extern long      period;
extern job_t  *  jobs;
#ifdef DEPS
extern bool   ** deps;
#endif

void readFile (char * filename);
