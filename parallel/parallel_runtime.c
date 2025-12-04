#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdatomic.h>
#include <limits.h>

// --- DOACROSS CONFIGURATION ---

// Maximum number of dependencies (IDs) the loop can have.
#define MAX_TRACKED_IDS 1024

// Synchronization Window.
// Threads can drift apart by at most this many iterations.
// Must be > (num_threads * max_dependence_distance).
#define SYNC_WINDOW 4096

// The Synchronization Table: [ID][Iteration % Window]
// Stores the "last completed iteration" for a given ID and slot.
volatile atomic_long completion_table[MAX_TRACKED_IDS][SYNC_WINDOW];

// Global to handle boundary conditions (e.g., waiting for iter -1)
static int64_t g_loop_start = 0;
void doacross_init(int64_t start)
{
    g_loop_start = start;

    // Initialize all flags to a value lower than any possible iteration.
    // This ensures that 'wait' will block until the actual post happens.
    long init_val = start - 1000;

    // Note: For a production compiler, you'd optimize this to not clear
    // the whole massive array every time, but for now this ensures correctness.
    for (int id = 0; id < MAX_TRACKED_IDS; id++)
    {
        for (int i = 0; i < SYNC_WINDOW; i++)
        {
            atomic_store(&completion_table[id][i], init_val);
        }
    }
}

// --- WAIT FUNCTION ---
// Compiler calls: doacross_wait(indvar, dist, id)
void doacross_wait(int64_t iter, int64_t dist, int32_t id)
{
    // Safety check for ID bounds
    if (id >= MAX_TRACKED_IDS)
    {
        fprintf(stderr, "Runtime Error: doacross ID %d exceeds limit %d\n", id, MAX_TRACKED_IDS);
        exit(1);
    }

    int64_t dep_iter = iter - dist;

    // BOUNDARY CHECK:
    // If we depend on an iteration BEFORE the loop started (e.g., A[i-1] at i=0),
    // we don't need to wait. The data is already there (from outside the loop).
    if (dep_iter < g_loop_start)
    {
        return;
    }

    // Map the huge iteration number to our small window
    int slot = dep_iter % SYNC_WINDOW;

    // SPIN LOOP
    // Wait until the slot contains a value >= the iteration we need.
    // We use atomic_load to ensure we see updates from other threads.
    while (atomic_load(&completion_table[id][slot]) < dep_iter)
    {
// Hint to CPU that we are spinning (saves power/improves hyperthreading)
#if defined(__x86_64__) || defined(__i386__)
        __builtin_ia32_pause();
#endif
    }
}

// --- POST FUNCTION ---
// Compiler calls: doacross_post(indvar, id)
void doacross_post(int64_t iter, int32_t id)
{
    if (id >= MAX_TRACKED_IDS)
        return;

    int slot = iter % SYNC_WINDOW;

    // Mark this iteration as complete for this ID.
    // We store the actual iteration number (not just 1/0) to handle the
    // circular buffer wrapping around correctly.
    atomic_store(&completion_table[id][slot], iter);
}

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

    doacross_init(start);
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
