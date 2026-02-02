#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

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

static void *worker_main(void *_arg)
{
    workers_args_t *a = (workers_args_t *)_arg;
    int tid = a->tid;
    int nthreads = a->nthreads;
    int64_t start = a->start;
    int64_t end = a->end;
    int64_t step = a->step;

    if (step == 0)
        return NULL;

    int64_t i;
    if (step < 0)
    {
        for (i = start + (int64_t)tid * step; i > end; i += (int64_t)nthreads * step)
        {
            if (a->body)
                a->body(i, a->env);
        }
    }
    else
    {
        for (i = start + (int64_t)tid * step; i < end; i += (int64_t)nthreads * step)
        {
            // exit(0); // temp
            printf("Iteration %d by thread %d\n", (int)i, tid);
            if (a->body)
                a->body(i, a->env);
        }
    }

    return NULL;
}

void parallel_for_runtime(int64_t start, int64_t end, int64_t step, loop_body_fn body, void *env)
{
    if (step == 0 || !body)
        return;

    int nthreads = 4;
    if (nthreads <= 0)
        nthreads = 1;

    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    workers_args_t *args = malloc(nthreads * sizeof(workers_args_t));
    if (!threads || !args)
    {
        free(threads);
        free(args);
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
        // possible problem
        pthread_create(&threads[i], NULL, worker_main, &args[i]);
    }

    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
}
