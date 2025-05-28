#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_WAITING_PATIENTS 3
#define MAX_MEDICINE 6
#define MEDICINE_PER_CONSULT 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t doctor_wakeup = PTHREAD_COND_INITIALIZER;
pthread_cond_t pharmacy_space = PTHREAD_COND_INITIALIZER;
pthread_cond_t consultation_done = PTHREAD_COND_INITIALIZER;

int waiting_patients = 0;
int total_patients = 0;
int active_patients = 0;
int medicine_stock = MAX_MEDICINE;
int pharmacist_waiting = 0;
int done_patients = 0;
int *patient_ids;
int consult_queue[3];

void timestamp() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}

void *patient_thread(void *arg) {
    int id = *((int *)arg);
    int waiting = 1;

    while (waiting) {
        int delay = rand() % 4 + 2;
        timestamp(); printf("Pacjent(%d): Ide do szpitala, bede za %d s\n", id, delay);
        sleep(delay);

        pthread_mutex_lock(&mutex);
        if (waiting_patients >= MAX_WAITING_PATIENTS) {
            pthread_mutex_unlock(&mutex);
            int walk = rand() % 4 + 2;
            timestamp(); printf("Pacjent(%d): za dużo pacjentów, wracam później za %d s\n", id, walk);
            sleep(walk);
            continue;
        }

        consult_queue[waiting_patients] = id;
        waiting_patients++;
        timestamp(); printf("Pacjent(%d): czeka %d pacjentów na lekarza\n", id, waiting_patients);

        if (waiting_patients == 3) {
            timestamp(); printf("Pacjent(%d): budzę lekarza\n", id);
            pthread_cond_signal(&doctor_wakeup);
        }

        pthread_cond_wait(&consultation_done, &mutex);

        timestamp(); printf("Pacjent(%d): kończę wizytę\n", id);
        done_patients++;
        pthread_mutex_unlock(&mutex);
        waiting = 0;
    }

    return NULL;
}

void *pharmacist_thread(void *arg) {
    int id = *((int *)arg);
    int delay = rand() % 11 + 5;
    timestamp(); printf("Farmaceuta(%d): ide do szpitala, bede za %d s\n", id, delay);
    sleep(delay);

    pthread_mutex_lock(&mutex);
    while (medicine_stock == MAX_MEDICINE) {
        timestamp(); printf("Farmaceuta(%d): czekam na oproznienie apteczki\n", id);
        pthread_cond_wait(&pharmacy_space, &mutex);
    }

    if (medicine_stock < MEDICINE_PER_CONSULT) {
        pharmacist_waiting++;
        timestamp(); printf("Farmaceuta(%d): budzę lekarza\n", id);
        pthread_cond_signal(&doctor_wakeup);
        timestamp(); printf("Farmaceuta(%d): dostarczam leki\n", id);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *doctor_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (!((waiting_patients == 3 && medicine_stock >= MEDICINE_PER_CONSULT) || (pharmacist_waiting > 0 && medicine_stock < MEDICINE_PER_CONSULT))) {
            timestamp(); printf("Lekarz: zasypiam\n");
            pthread_cond_wait(&doctor_wakeup, &mutex);
            timestamp(); printf("Lekarz: budzę się\n");
        }

        if (waiting_patients == 3 && medicine_stock >= MEDICINE_PER_CONSULT) {
            timestamp(); printf("Lekarz: konsultuję pacjentów %d, %d, %d\n", consult_queue[0], consult_queue[1], consult_queue[2]);
            medicine_stock -= MEDICINE_PER_CONSULT;
            waiting_patients = 0;
            sleep(rand() % 3 + 2);
            pthread_cond_broadcast(&consultation_done);
            pthread_cond_broadcast(&pharmacy_space);
        } else if (pharmacist_waiting > 0 && medicine_stock < MAX_MEDICINE) {
            timestamp(); printf("Lekarz: przyjmuję dostawę leków\n");
            int added = MAX_MEDICINE - medicine_stock;
            medicine_stock = MAX_MEDICINE;
            sleep(rand() % 3 + 1);
            pharmacist_waiting--;
            timestamp(); printf("Lekarz: uzupełniono apteczkę o %d jednostek\n", added);
        }

        if (done_patients >= total_patients) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);
    }

    timestamp(); printf("Lekarz: wszyscy pacjenci obsłużeni. Kończę pracę.\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <liczba_pacjentow> <liczba_farmaceutow>\n", argv[0]);
        return 1;
    }

    total_patients = atoi(argv[1]);
    int num_pharmacists = atoi(argv[2]);

    pthread_t doctor, *patients, *pharmacists;
    patients = malloc(total_patients * sizeof(pthread_t));
    pharmacists = malloc(num_pharmacists * sizeof(pthread_t));
    patient_ids = malloc(total_patients * sizeof(int));
    srand(time(NULL));

    pthread_create(&doctor, NULL, doctor_thread, NULL);
    for (int i = 0; i < total_patients; i++) {
        patient_ids[i] = i + 1;
        pthread_create(&patients[i], NULL, patient_thread, &patient_ids[i]);
    }

    for (int i = 0; i < num_pharmacists; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&pharmacists[i], NULL, pharmacist_thread, id);
    }

    for (int i = 0; i < total_patients; i++)
        pthread_join(patients[i], NULL);

    pthread_join(doctor, NULL);

    free(patients);
    free(pharmacists);
    free(patient_ids);
    return 0;
}