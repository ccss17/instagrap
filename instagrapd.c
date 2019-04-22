#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "instagrap.h"
#define HASHSIZE 101

const char * ARG_PARSER = ":p:w:h";

typedef struct {
    int clnt_sd; 
    char * worker_ip; 
    char * worker_port; 
    char * path_testcase;
} instagrapd_args;

struct dict { 
    struct dict *next;
    char *name;
    char *defn;
};

static struct dict *dictionary[HASHSIZE]; /* pointer table */

unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
    hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

/* lookup: look for s in dictionary */
struct dict *lookup(char *s)
{
    struct dict *np;
    for (np = dictionary[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np; /* found */
    return NULL; /* not found */
}


/* install: put (name, defn) in dictionary */
struct dict *install(char *name, char *defn)
{
    struct dict *np;
    unsigned hashval;
    if ((np = lookup(name)) == NULL) { /* not found */
        np = (struct dict *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL)
            return NULL;
        hashval = hash(name);
        np->next = dictionary[hashval];
        dictionary[hashval] = np;
    } else /* already there */
        free((void *) np->defn); /*free previous defn */
    if ((np->defn = strdup(defn)) == NULL)
        return NULL;
    return np;
}


void need_help() { fprintf(stderr, "enter './instagrapd -h' for help message\n"); }
void help() { fprintf(stderr, "Usage: ./instagrapd -p <PORT> -w <IP>:<WPORT> <DIR>\n"); }

void * instagrapd(void * arg) {
    instagrapd_args * args = (instagrapd_args *) arg;
    char * buf;
    data_set * targetc;
    char stdid[IDENTIFIER_SIZE+1] = {'\0', };
    char pw[IDENTIFIER_SIZE+1] = {'\0', };
    data_set * testcase_in;
    data_set * testcase_out;
    size_t testcase_size;
    char * path_testcase_tmp;
    int worker_sock;
    char * flag;
    data_set * output_set;
    int pass_count = 0;
    int i;

    path_testcase_tmp = (char *) malloc(strlen(args->path_testcase) + 10);
    flag = (char *) malloc(sizeof(char) * 2);

    // EXCEPTION #1 rejects connection if it give worng password (different from the one that is given at the submission)

    // SECTION #1 receive data from submitter
    // data: STD_ID, PW, target.c
    read(args->clnt_sd, stdid, IDENTIFIER_SIZE );
    read(args->clnt_sd, pw, IDENTIFIER_SIZE );

    struct dict * pdict = lookup(stdid);
    if (pdict == NULL) {
        // nonexistent STDID, so put stdid and pw
        install(stdid, pw);
    }
    else if (strcmp(pdict->defn, pw) != 0) {
        // existent STDID, but incorrect password
        fprintf(stderr, "incorrect password for %s/%s\n", pdict->name, pw);
        strcpy(flag, "4");
        write(args->clnt_sd, flag, 1);
        return NULL;
    }

    targetc = receive_data(args->clnt_sd);

#if DEBUG
    printf("stdid:%s/%ld\n", stdid, strlen(stdid));
    printf("pw:%s/%ld\n", pw, strlen(pw));
    printf("\n");
#endif

    // LOOP: SECTION #2 ~ #4 until checking process is complete
    for (i = 1; i <= TESTCASE_COUNT; i++) {
        // SECTION #2 send data to worker
        // data: testcase_in(1.in, 2.in, ...), target.c
        strcpy(path_testcase_tmp, args->path_testcase);
        strcat(path_testcase_tmp, "/");
        strcat(path_testcase_tmp, itoa(i));
        strcat(path_testcase_tmp, ".in");
        testcase_in = readfile(path_testcase_tmp);
#if DEBUG
        printf("testcase_in size:%ld\n", testcase_in->size);
        printf("testcase_in:%s\n", testcase_in->data);
#endif

        worker_sock = establish_connection (args->worker_ip, args->worker_port);
        send_dataset(worker_sock, testcase_in);
        send_dataset(worker_sock, targetc);

        // SECTION #3 receive output from worker
        // output: stdout OR build failure OR runtim error OR timeout error
        read(worker_sock, flag, 1);
        flag[1] = '\0';
        switch(flag[0]) {
            case '0':
                // PROGRAM EXIT NORMALLY
                output_set = receive_data(worker_sock);
                strcpy(path_testcase_tmp, args->path_testcase);
                strcat(path_testcase_tmp, "/");
                strcat(path_testcase_tmp, itoa(i));
                strcat(path_testcase_tmp, ".out");
                testcase_out = readfile(path_testcase_tmp);
                // SECTION #4 compare between output and n.out(1.out, 2.out, ....)
                // count passes number
                if(strcmp(output_set->data, testcase_out->data) == 0) {
                    // n.out == output
                    printf("pass test for %d.in %d.out\n", i, i);
                    pass_count++;
                } else {
                    printf("FAIL to pass test for %d.in %d.out\n", i, i);
                }
#if DEBUG
                printf("output from worker:%ld/%s\n", output_set->size, output_set->data);
                printf("testcase_out:%ld/%s\n", testcase_out->size, testcase_out->data);
                printf("compare result:%d\n", strcmp(output_set->data, testcase_out->data));
                printf("\n");
#endif
                break;
            case '1': // BUILD FAILED
                fprintf(stderr, "build failed\n");
                write(args->clnt_sd, flag, 1);
                return NULL;
            case '2': // RUNTIME ERROR
                fprintf(stderr, "runtime error\n");
                write(args->clnt_sd, flag, 1);
                return NULL;
            case '3': // TIMEOUT ERROR
                fprintf(stderr, "timeout error\n");
                write(args->clnt_sd, flag, 1);
                return NULL;
            default: // UNKNOWN ERRO
                fprintf(stderr, "unknown error\n");
                write(args->clnt_sd, flag, 1);
                return NULL;
        }
    }

    // SECTION #5 send result to submitter
    // result: number of test cases that target.c passes OR build failure
#if DEBUG
    printf("pass count:%d/%ld\n", pass_count, strlen(itoa(pass_count)));
#endif
    write(args->clnt_sd, flag, 1);
    write(args->clnt_sd, itoa(pass_count), strlen(itoa(pass_count)));

    shutdown(args->clnt_sd, SHUT_WR); 
    close(args->clnt_sd);
    return NULL;
}

int main(int argc, char * argv[]) {
    if (argc == 1) {
        help();
        return 1;
    }

    int opt;
    int serv_sd = -2;
    int threads_index = 0;
    pthread_t threads[THREAD_LIMIT];
    char ** ipport;
    instagrapd_args * args;

    args = (instagrapd_args *) malloc( sizeof(instagrapd_args));

    while((opt = getopt(argc, argv, ARG_PARSER)) != -1){
        switch(opt) {
            case 'p':
                serv_sd = init_serv_sock(optarg);
                break;  
            case 'w':
                ipport = str_split(optarg, ':');
                args->worker_ip = *(ipport);
                args->worker_port = *(ipport + 1);
                break;  
            case 'h':
                help();
                exit(1);
            case ':': 
                fprintf(stderr, "option needs a value\n");
                exit(1);
            case '?':  
                fprintf(stderr, "unknown option: %c\n", optopt); 
                break;  
        }
    }

    if (argc - optind != 1 ) {
        help();
        return 1;
    }

    if (serv_sd == -1) {
        fprintf(stderr, "error occur when initializing server socket\n");
        return 1;
    } else if (serv_sd == -2) {
        fprintf(stderr, "-p option must be specified\n");
        help();
        return 1;
    }

    args->path_testcase = argv[optind];

    while (1) {
        args->clnt_sd = accept_connection(serv_sd);
        pthread_create(&threads[threads_index], NULL, instagrapd, (void *) args);
        threads_index++;
        if (threads_index == THREAD_LIMIT) 
            threads_index = 0;
    }

    free(args);
    free(*(ipport));
    free(*(ipport + 1));
    free(ipport);

    return 0;
}
