#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

long hits = 0;
pthread_mutex_t mutex;

struct ThreadData {
    long thread_id;
    long ntrials;
};

void* monte_carlo(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    long local_hits = 0;
    for (long i = 0; i < data->ntrials; i++) {
        double x = (double)rand() / RAND_MAX * 2 - 1;
        double y = (double)rand() / RAND_MAX * 2 - 1;
        if (x * x + y * y <= 1) {
            local_hits++;
        }
    }

    pthread_mutex_lock(&mutex);
    hits += local_hits;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s nthreads ntrials\n", argv[0]);
        return 1;
    }

    long nthreads = strtol(argv[1], NULL, 10);
    long ntrials = strtol(argv[2], NULL, 10);

    if (nthreads <= 0 || ntrials <= 0) {
        fprintf(stderr, "nthreads and ntrials must be positive integers\n");
        return 1;
    }

    pthread_t* thread_handles = malloc(nthreads * sizeof(pthread_t));
    struct ThreadData* thread_data = malloc(nthreads * sizeof(struct ThreadData));

    if (thread_handles == NULL || thread_data == NULL) {
        perror("malloc");
        return 1;
    }

    pthread_mutex_init(&mutex, NULL);

    long trials_per_thread = ntrials / nthreads;
    long remainder = ntrials % nthreads;

    clock_t start = clock();

    for (long i = 0; i < nthreads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].ntrials = trials_per_thread + (i < remainder ? 1 : 0);
        int err = pthread_create(&thread_handles[i], NULL, monte_carlo, (void*)&thread_data[i]);
        if (err != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (long i = 0; i < nthreads; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    double pi_estimate = 4.0 * hits / ntrials;
    printf("Estimated value of π: %f\n", pi_estimate);
    printf("Time spent: %f seconds\n", time_spent);

    pthread_mutex_destroy(&mutex);

    free(thread_handles);
    free(thread_data);

    return 0;
}
