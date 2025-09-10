#define _GNU_SOURCE
#include <stdint.h>
#include <inttypes.h> /* <-- required for PRId64 */
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <errno.h>

typedef void (*loop_body_fn)(int64_t i, void *env);

typedef struct
{
    int64_t start;
    int64_t end;
    int64_t step;
    loop_body_fn body;
    void *env;
    int tid;
    int nthreads;
    int ncpus;
} workers_args_t;

/* Global file and mutex for logging thread->core mapping */
static FILE *mapping_file = NULL;
static pthread_mutex_t mapping_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *worker_main(void *_arg)
{
    workers_args_t *a = (workers_args_t *)_arg;
    int tid = a->tid;
    int nthreads = a->nthreads;
    int ncpus = a->ncpus;
    int64_t start = a->start;
    int64_t end = a->end;
    int64_t step = a->step;

    if (step == 0)
        return NULL;

    /* Pin this thread to a core: core = tid % ncpus */
    int core = (ncpus > 0) ? (tid % ncpus) : 0;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    int setret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    int saved_errno = (setret != 0) ? errno : 0;

    /* Log the mapping to the file (thread-safe) */
    pthread_mutex_lock(&mapping_mutex);
    if (mapping_file)
    {
        /* print tid, pthread id, assigned core, and result of setaffinity */
        fprintf(mapping_file,
                "worker tid=%d pthread=%lu pinned_core=%d pthread_setaffinity_np_ret=%d errno=%d\n",
                tid, (unsigned long)pthread_self(), core, setret, saved_errno);
        fflush(mapping_file);
    }
    pthread_mutex_unlock(&mapping_mutex);

    /* Execute the work assigned to this thread */
    int64_t i;
    if (step < 0)
    {
        for (i = start + (int64_t)tid * step; i > end; i += (int64_t)nthreads * step)
        {
            /* --- new: log which thread will process index `i` --- */
            pthread_mutex_lock(&mapping_mutex);
            if (mapping_file)
            {
                fprintf(mapping_file,
                        "worker tid=%d pthread=%lu core_pin=%d processing_index=%" PRId64 "\n",
                        tid, (unsigned long)pthread_self(), core, i);
                fflush(mapping_file);
            }
            pthread_mutex_unlock(&mapping_mutex);

            /* Also print to stderr for immediate visibility (optional) */
            fprintf(stderr, "[worker %d] pthread=%lu core=%d processing index %" PRId64 "\n",
                    tid, (unsigned long)pthread_self(), core, i);
            fflush(stderr);
            /* --- end new logging --- */

            if (a->body)
                a->body(i, a->env);
        }
    }
    else
    {
        for (i = start + (int64_t)tid * step; i < end; i += (int64_t)nthreads * step)
        {
            /* --- new: log which thread will process index `i` --- */
            pthread_mutex_lock(&mapping_mutex);
            if (mapping_file)
            {
                fprintf(mapping_file,
                        "worker tid=%d pthread=%lu core_pin=%d processing_index=%" PRId64 "\n",
                        tid, (unsigned long)pthread_self(), core, i);
                fflush(mapping_file);
            }
            pthread_mutex_unlock(&mapping_mutex);

            /* Also print to stderr for immediate visibility (optional) */
            fprintf(stderr, "[worker %d] pthread=%lu core=%d processing index %" PRId64 "\n",
                    tid, (unsigned long)pthread_self(), core, i);
            fflush(stderr);
            /* --- end new logging --- */

            if (a->body)
                a->body(i, a->env);
        }
    }

    return NULL;
}

void parallel_for_runtime(int64_t start, int64_t end, int64_t step, loop_body_fn body, void *env)
{
    /* Basic validation */
    if (step == 0)
        return;
    if (!body)
    {
        fprintf(stderr, "parallel_for_runtime: body is NULL — nothing to do\n");
        return;
    }

    int nthreads = 4;
    if (nthreads <= 0)
        nthreads = 1;

    /* Open mapping file and write header with nthreads */
    mapping_file = fopen("threads_mapping.txt", "w");
    if (!mapping_file)
    {
        fprintf(stderr, "Failed to open threads_mapping.txt for writing: %s\n", strerror(errno));
        /* proceed without mapping output but still run the parallel loop */
    }
    else
    {
        pthread_mutex_lock(&mapping_mutex);
        fprintf(mapping_file, "nthreads=%d (sysconf _SC_NPROCESSORS_ONLN)\n", nthreads);
        fprintf(mapping_file, "start=%" PRId64 " end=%" PRId64 " step=%" PRId64 "\n", start, end, step);
        fflush(mapping_file);
        pthread_mutex_unlock(&mapping_mutex);
    }

    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    workers_args_t *args = malloc(nthreads * sizeof(workers_args_t));
    if (!threads || !args)
    {
        fprintf(stderr, "Allocation failed\n");
        if (threads)
            free(threads);
        if (args)
            free(args);
        if (mapping_file)
        {
            fclose(mapping_file);
            mapping_file = NULL;
        }
        return;
    }

    for (int i = 0; i < nthreads; i++)
    {
        args[i].start = start;
        args[i].end = end;
        args[i].step = step;
        args[i].body = body;
        args[i].env = env;
        args[i].tid = i;
        args[i].nthreads = nthreads;
        args[i].ncpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
        if (args[i].ncpus <= 0)
            args[i].ncpus = 1;

        int cret = pthread_create(&threads[i], NULL, worker_main, &args[i]);
        if (cret != 0)
        {
            fprintf(stderr, "pthread_create failed for thread %d: %s\n", i, strerror(cret));
            /* continue — some threads may still be created */
        }
    }

    for (int i = 0; i < nthreads; i++)
    {

        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);

    if (mapping_file)
    {
        fclose(mapping_file);
        mapping_file = NULL;
    }
}
