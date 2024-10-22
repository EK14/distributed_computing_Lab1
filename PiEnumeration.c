#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

struct ThreadData {
    long thread_id;
    long ntrials;
    long* hits;
};

void* monte_carlo(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    long hits = 0;
    for (long i = 0; i < data->ntrials; i++) {
        double x = (double)rand() / RAND_MAX * 2 - 1;
        double y = (double)rand() / RAND_MAX * 2 - 1;
        if (x * x + y * y <= 1) {
            hits++;
        }
    }
    data->hits[data->thread_id] = hits;
    return NULL;
}

int main(int argc, char** argv) {
    long nthreads = strtol(argv[1], NULL, 10);
    long ntrials = strtol(argv[2], NULL, 10);

    if (nthreads <= 0 || ntrials <= 0) {
        fprintf(stderr, "nthreads and ntrials must be positive integers\n");
        return 1;
    }

    pthread_t* thread_handles = malloc(nthreads * sizeof(pthread_t));
    struct ThreadData* thread_data = malloc(nthreads * sizeof(struct ThreadData));
    long* hits = malloc(nthreads * sizeof(long));

    if (thread_handles == NULL || thread_data == NULL || hits == NULL) {
        perror("malloc");
        return 1;
    }

    long trials_per_thread = ntrials / nthreads;

    for (long i = 0; i < nthreads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].ntrials = trials_per_thread;
        thread_data[i].hits = hits;
        int err = pthread_create(&thread_handles[i], NULL, monte_carlo, (void*)&thread_data[i]);
        if (err != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (long i = 0; i < nthreads; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    long total_hits = 0;
    for (long i = 0; i < nthreads; i++) {
        total_hits += hits[i];
    }

    double pi_estimate = 4.0 * total_hits / ntrials;
    printf("Estimated value of π: %f\n", pi_estimate);

    free(thread_handles);
    free(thread_data);
    free(hits);

    return 0;
}

