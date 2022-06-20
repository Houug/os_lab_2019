#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *ThreadFunc1(void *args) {
    pthread_mutex_lock(&mutex1);

    printf("Thread 1\n");

    pthread_mutex_lock(&mutex2);

    printf("More Thread 1\n");

    pthread_mutex_unlock(&mutex2);

    pthread_mutex_unlock(&mutex1);
}

void *ThreadFunc2(void *args) {
    pthread_mutex_lock(&mutex2);

    printf("Thread 2\n");

    pthread_mutex_lock(&mutex1);
    
    printf("More Thread 2\n");

    pthread_mutex_unlock(&mutex1);

    pthread_mutex_unlock(&mutex2);
}

int main() {

    pthread_t t1, t2;

    pthread_create(&t1, NULL, ThreadFunc1, NULL);
    pthread_create(&t2, NULL, ThreadFunc2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);


    printf("Sad message\n");
    return 0;
}
