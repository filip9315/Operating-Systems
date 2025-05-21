#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int thread_id;
    int total_threads;
    double dx;
    double* results;
} ThreadData;

double f(double x) {
    return 4.0 / (x * x + 1.0);
}

void* compute_integral(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    double dx = data->dx;
    int id = data->thread_id;
    int k = data->total_threads;

    long long total_steps = (long long)(1.0 / dx);
    long long start = id * total_steps / k;
    long long end = (id + 1) * total_steps / k;

    double local_sum = 0.0;
    for (long long i = start; i < end; i++) {
        double x = i * dx;
        local_sum += f(x) * dx;
    }

    data->results[id] = local_sum;
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <dx> <num_threads>\n", argv[0]);
        return 1;
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    double dx = atof(argv[1]);
    int k = atoi(argv[2]);

    pthread_t* threads = malloc(sizeof(pthread_t) * k);
    ThreadData* thread_data = malloc(sizeof(ThreadData) * k);
    double* results = calloc(k, sizeof(double));

    for (int i = 0; i < k; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].total_threads = k;
        thread_data[i].dx = dx;
        thread_data[i].results = results;
        pthread_create(&threads[i], NULL, compute_integral, &thread_data[i]);
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    double total = 0.0;
    for (int i = 0; i < k; i++) {
        total += results[i];
    }

    printf("Approximate integral: %.15f\n", total);

    free(threads);
    free(thread_data);
    free(results);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Execution time: %.6f seconds\n", elapsed);

    return 0;
}