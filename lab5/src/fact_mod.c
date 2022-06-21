#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/time.h>

size_t fact = 1;
size_t p = 1;

pthread_mutex_t mutex_fact = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_p = PTHREAD_MUTEX_INITIALIZER;

struct TFact {
    size_t k;
    size_t mod;
    size_t begin;
    size_t end;
};

void *ThreadSum(void *args) {
    struct TFact *fargs = (struct TFact *)args;
    size_t result = 1;
    for (size_t i = fargs->begin; i <= fargs->end; i++) {
        if (i%fargs->mod == 0) {
            pthread_mutex_lock(&mutex_p);
            result *= p;
            p++;
            pthread_mutex_unlock(&mutex_p);
        } else {
            result *= i%fargs->mod;
        }
		result %= fargs->mod;
    }
    pthread_mutex_lock(&mutex_fact);
    fact *= result % fargs->mod;
    pthread_mutex_unlock(&mutex_fact);
    return 0;
}

int main(int argc, char **argv) {

    uint32_t pnum = -1;
    uint32_t mod = -1;
    uint32_t k = -1;

    while (true) {
        static struct option options[] = {{"pnum", required_argument, 0, 0},
                                          {"mod", required_argument, 0, 0},
                                          {"k", required_argument, 0, 'k'},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "k:", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        pnum = atoi(optarg);
                        if (pnum < 0) {
                            printf("--pnum must be positive value or zero\n");
                            return 1;
                        }
                        break;
                    case 1:
                        mod = atoi(optarg);
                        if (mod <= 0) {
                            printf("--mod must be positive value and greater zero\n");
                            return 1;
                        }
                        break;
                    case 2:
                        k = atoi(optarg);
                        if (k <= 0) {
                            printf("-k must be positive value and greater zero\n");
                            return 1;
                        }
                        break;

                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;

            case '?':
                break;

            case 'k':
                k = atoi(optarg);
                if (k <= 0) {
                    printf("-k must be positive value and greater zero\n");
                    return 1;
                }
                break;

            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
    }

    if (mod == -1 || pnum == -1 || k == -1) {
        printf("Usage: %s --pnum \"num\" --mod \"num\" -k \"num\" \n",
               argv[0]);
        return 1;
    }

    pthread_t threads[pnum];
    struct TFact args[pnum];

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    for (uint32_t i = 0; i < pnum; i++) {
        args[i].k = k;
        args[i].mod = mod;

        args[i].begin = i*(k/pnum) + 1;
        args[i].end = (i == pnum - 1) ? k :(i+1)*(k/pnum);
        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            return 1;
        }
    }

    for (uint32_t i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    printf("%d! mod %d = %ld, Time: %fms\n", k, mod, fact%mod, elapsed_time);
    return 0;
}