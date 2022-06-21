#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "factorial.h"

struct Server {
    char ip[255];
    int port;
};

struct ServerArgs {
    struct Server *server;
    uint64_t mod;
    uint64_t begin;
    uint64_t end;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {

    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }

    if (errno != 0)
        return false;

    *val = i;
    return true;
}

void ConvertAddressStringToIpPort(const char *str, struct Server *server) {
    char *port = strchr(str, ':');
    strncpy(server->ip, str, port - str);
    server->port = atoi(port + 1);
}

void *ThreadFunction(void *args) {
    struct ServerArgs *a = (struct ServerArgs*) args;
    struct hostent *hostname = gethostbyname(a->server->ip);
        if (hostname == NULL) {
            fprintf(stderr, "gethostbyname failed with %s\n", a->server->ip);
            exit(1);
        }

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(a->server->port);
        server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

        int sck = socket(AF_INET, SOCK_STREAM, 0);
        if (sck < 0) {
            fprintf(stderr, "Socket creation failed!\n");
            exit(1);
        }

        if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
            fprintf(stderr, "Connection failed\n");
            exit(1);
        }

        char task[sizeof(uint64_t) * 3];
        memcpy(task, &(a->begin), sizeof(uint64_t));
        memcpy(task + sizeof(uint64_t), &(a->end), sizeof(uint64_t));
        memcpy(task + 2 * sizeof(uint64_t), &(a->mod), sizeof(uint64_t));

        if (send(sck, task, sizeof(task), 0) < 0) {
            fprintf(stderr, "Send failed\n");
            exit(1);
        }

        char response[sizeof(uint64_t)];
        if (recv(sck, response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Recieve failed\n");
            exit(1);
        }

        uint64_t answer = 0;
        memcpy(&answer, response, sizeof(uint64_t));
        printf("answer: %lu\n", answer);

        close(sck);
        return (void*) (uint64_t*) answer;
}

int main(int argc, char **argv) {
    uint64_t k = -1;
    uint64_t mod = -1;
    char servers[255] = {'\0'}; // TODO: explain why 255

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"k", required_argument, 0, 0},
                                          {"mod", required_argument, 0, 0},
                                          {"servers", required_argument, 0, 0},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        if(!ConvertStringToUI64(optarg, &k)) {
							printf("Conversion error\n");
							return 1;
						}
                        break;
                    case 1:
                        if(!ConvertStringToUI64(optarg, &mod)) {
							printf("Conversion error\n");
							return 1;
						}
                        break;
                    case 2:
                        if (strlen(optarg) > 254) {
							printf("Path is to long\n");
							return 1;
						}
                        memcpy(servers, optarg, strlen(optarg));
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            } break;

            case '?':
                printf("Arguments error\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (k == -1 || mod == -1 || !strlen(servers)) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
                argv[0]);
        return 1;
    }

    FILE* fd = fopen(servers, "r");
    unsigned int servers_num = 0;

    while (!feof(fd)) {
        if (fgetc(fd) == '\n') {
            servers_num++;
        }
    }

    if (servers_num == 0) {
        printf("No server address was found\n");
        fclose(fd);
        return 1;
    }
	
	fseek(fd, (void*)NULL, SEEK_SET);

    struct Server *to = malloc(sizeof(struct Server) * servers_num);

    for (int i = 0; i < servers_num; i++) {
        char buf[255];
        fgets(buf, sizeof(buf), fd);
        ConvertAddressStringToIpPort(buf, &to[i]);
    }    
    fclose(fd);

    
    pthread_t server_threads[servers_num];
    struct ServerArgs server_args[servers_num];

    for (int i = 0; i < servers_num; i++) {
        server_args[i].server = &to[i];
        server_args[i].begin = i*(k/servers_num) + 1;
        server_args[i].end = (i == servers_num - 1) ? k :(i+1)*(k/servers_num);
        server_args[i].mod = mod;
        pthread_create(&server_threads[i], NULL, ThreadFunction, (void*)&server_args[i]);
    }
     
    uint64_t total = 1;
    for (int i = 0; i < servers_num; i++) {
        uint64_t result = 0;
        pthread_join(server_threads[i], (void**)&result);
        total = MultModulo(total, result, mod);
    }
	
	printf("%ld! mod %ld = %lu\n", k, mod, total);

    free(to);

    return 0;
}
