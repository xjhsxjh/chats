#ifndef _THREAD_POOL_
#define _THREAD_POOL_

# include "lib.h"

# include <stdbool.h>
# include <pthread.h>

/* Thread pool facilities */
typedef void (*job_function_t)(void *);

struct job;
typedef struct job job_t;

struct thread_descr;
typedef struct thread_descr thread_descr_t;

struct thread_pool;
typedef struct thread_pool thread_pool_t;

thread_pool_t *thread_pool_init(size_t thread_count);
void thread_pool_stop(thread_pool_t *tp, bool wait_for_stop);
void thread_pool_post_job(thread_pool_t *tp, job_function_t job, void *ctx);

#endif /* _THREAD_POOL_ */
