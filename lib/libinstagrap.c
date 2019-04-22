#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <assert.h>

#include "instagrap.h"

volatile int flag_timeout = 0;
volatile int flag_child_done = 0;

void child_handler(int sig) { flag_child_done = 1; }
void alarm_handler(int sig) { flag_timeout = 1; }

char * itoa(int integer) {
    char * str;
    int digit = 0;
    int tmp = integer;

    do digit++; while(tmp /= 10);
    str = (char *) malloc(sizeof(char) * digit);
    sprintf(str, "%d", integer);

    return str;
}

int is_digit(char digit){ return digit <= '9' && digit >= '0'; }
int is_integer(char * num) {
    int i = 0;
    if (num[0] == '-')
        i = 1;
    for (; num[i] != 0; i++) {
        if (!is_digit(num[i]))
            return 0;
    }
    return 1;
}

int establish_connection(char * ip, char * port) {
    int sd;
    FILE *fp;

    char buf[BUF_SIZE];
    int read_cnt;
    struct sockaddr_in serv_adr;

    sd=socket(PF_INET, SOCK_STREAM, 0);   

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(ip);
    serv_adr.sin_port = htons(atoi(port));

    connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));

    return sd;
}

data_set * readfile(char *filename) {
    char *buf = NULL;
    int filesize, read_size;
    FILE *file = fopen(filename, "r");
    data_set * file_data;

    file_data = (data_set *) malloc(sizeof(data_set));

    if (file) {
        fseek(file, 0, SEEK_END);
        file_data->size = ftell(file);
        rewind(file);

        file_data->data = (char*) malloc(sizeof(char) * (file_data->size + 1) );
        read_size = fread(file_data->data, sizeof(char), file_data->size, file);
        file_data->data[file_data->size] = '\0';

        if (file_data->size != read_size) {
            free(file_data->data);
            file_data->data = NULL;
        }

        fclose(file);
    }

    return file_data;
}

void send_dataset(int sock, data_set * ds) {
    char * size_indicator;

    size_indicator = (char *)malloc(sizeof(char) * FILE_SIZE_INDICATOR);
    sprintf(size_indicator, "%0.4ld", ds->size);
    write(sock, size_indicator, FILE_SIZE_INDICATOR);
    write(sock, ds->data, strlen(ds->data));
}

int accept_connection(int serv_sd) {
    int clnt_sd;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;
    char client_ip[INET_ADDRSTRLEN];

    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
    inet_ntop(AF_INET, &(clnt_adr.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("\n#######################################\n");
    printf("accept requests from IP:%s\n", client_ip);

    return clnt_sd;
}

int init_serv_sock(char * listen_port) {
    if (!is_integer(listen_port)){
        fprintf(stderr, "port number must be an integer\n");
        return -1;
    }
    int serv_sd;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in serv_adr;

    serv_sd = socket(PF_INET, SOCK_STREAM, 0);   
    if (serv_sd == -1) 
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(listen_port));

    if(bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
        exit(1);
    }
    if(listen(serv_sd, 5) == -1)
        error_handling("listen() error");

    fprintf(stdout, "start listening at port %s...\n", listen_port);
    return serv_sd;
}

data_set * receive_data( int sock ) {
    char buf[BUF_SIZE];
    data_set * data_s;
    int read_cnt;
    int tmp_size;

    data_s = (data_set *) malloc(sizeof(data_set));
    // get file size
    read(sock, buf, FILE_SIZE_INDICATOR );
    data_s->size = atoi(buf);
    data_s->data = (char *)malloc(sizeof(char) * data_s->size + 1);

    if (data_s->size <= BUF_SIZE) {
        read(sock, data_s->data, data_s->size );
        data_s->data[data_s->size] = '\0';
#if DEBUG
        printf("file size:%ld\n", data_s->size);
        printf("read from sock:%s\n", data_s->data);
#endif
        return data_s;
    }

    tmp_size = data_s->size;
    tmp_size -= BUF_SIZE;
    read(sock, buf, BUF_SIZE );
#if DEBUG
    printf("file size:%ld\n", data_s->size);
    printf("read from sock:%s\n", buf);
#endif
    strncpy(data_s->data, buf, strlen(buf));

    while(1) {
        if (tmp_size <= 0)
            break; // get data piece
        else if (tmp_size < BUF_SIZE) {
            read(sock, buf, tmp_size );
            strncat(data_s->data, buf, tmp_size);
            break;
        } else {
            tmp_size -= BUF_SIZE;
            read(sock, buf, BUF_SIZE );
            strncat(data_s->data, buf, BUF_SIZE);
        }
    }
    data_s->data[data_s->size] = '\0';
    return data_s;
}


void save_file(const char * filename, data_set *data_s) {
    FILE * fp;

#if DEBUG
    printf("saved filename:%s\n", filename);
    printf("saved data:%s\n", data_s->data);
#endif

    fp = fopen(filename, "wb");
    fwrite((void*)data_s->data, sizeof(char), data_s->size, fp);
    fclose(fp);
}

//
// FORK CHILD PROCESS 
//

int exists(char * fname) {
    if ( access( fname, F_OK) != -1 ) return 1;
    else return 0;
}

int build(char * build_target) {
    /*
    return "0..." program exit normally
    return "1" failed _build
    return "2" runtime error
    return "3" timeout error
    */
    int flag; // 0 : PROGRAM NORMALLY EXIT, -1 : PROGRAM ERROR, -2 : TIMEOUT ERROR
    char ** result;
    char * feedback;
    char * CMD_BUILD[] = {
        COMPILER, build_target, 
        "-O2", "-lm", "-static", "-DONLINE_JUDGE", "-DBOJ", NULL
    };

    feedback = (char *) malloc(sizeof(char));
    execute(CMD_BUILD, NULL);

    if (exists(DEFAULT_OUTPUT_FILE)){
        // BUILD SUCCESS
        return 0;
    } else {
        // BUILD FAIL
        return -1;
    }
}

char ** execute(char * args[], char * input) {
    char ** result;
    int pipes[PIPE_COUNT];

    result = (char **) malloc(sizeof(char *) * PIPE_COUNT - 1);
    _fork_subprocess(pipes, args, input);

    // result[0] -> stdout of child child process (actually execute program)
    // result[1] -> stderr of child child process
    // result[2] -> stdout of child process (check exit status of child child process)
    result[0] = _read_pipe(pipes[1], 0);
    result[1] = _read_pipe(pipes[2], 0); 
    result[2] = _read_pipe(pipes[3], 1); 

    return result;
}

pid_t _fork_subprocess(int * pipes, char ** args, char * input)
{
    int status, in[2], out[2], err[2], check_runtimerror[2];
    pid_t pid;

    if (pipe(in) || pipe(out) || pipe(err) || pipe(check_runtimerror))
        return -1;

    pid = fork();
    if (pid < 0) {
        close(err[0]);
        close(err[1]);
        close(out[0]);
        close(out[1]);
        close(in[0]);
        close(in[1]);
        return -1;

    } else if ( pid > 0 ) {
        // PARENT
        wait(NULL);

        close(in[0]);  
        close(out[1]);
        close(err[1]); 

        pipes[0] = in[1]; 
        pipes[1] = out[0];
        pipes[2] = err[0];
        pipes[3] = check_runtimerror[0];

        return pid;
    } else {
        if ((pid = fork()) == 0) {
            // CHILD CHILD
            close(in[1]);
            close(out[0]);
            close(err[0]);

            dup2(in[0], 0);  
            dup2(out[1], 1); 
            dup2(err[1], 2); 
            execv(args[0], args); 
        } else {
            // CHILD
            int result;

            dup2(check_runtimerror[1], 1); 
            signal(SIGALRM, alarm_handler);
            signal(SIGCHLD, child_handler);
            alarm(TIMEOUT);  // install an alarm to be fired after TIME_LIMIT
            if (input != NULL) {
                write(in[1], input, strlen(input) + 1);
            }
            pause();

            if (flag_timeout) {
                // SIGALRM
                result = waitpid(pid, &status, WNOHANG);
                if (result == 0) {
                    // child still running, so kill it
                    kill(pid, SIGKILL);
                    wait(&status);
                    printf("-1");
                    exit(0);
                } else {
                    // alarm triggered, but child finished normally
                }
            } else if (flag_child_done) {
                // SIGCHLD 
                // child finished normally
                wait(&status);
            }
            // print 1 --> PROGRAM EXIT NORMALLY
            // print 0 --> RUNTIME ERROR
            wait(NULL);
            printf("%d", WIFEXITED(status));
            exit(0);
        }
    }
}

char * _read_pipe(int pfd, int check_runtimerror) {
    int result;
    char buf[BUF_SIZE];
    char * data = (char *) malloc(BUF_SIZE * 5);

    read(pfd, data, BUF_SIZE);
    if (check_runtimerror) return data;

    for(;;) {
        result = read(pfd, buf, BUF_SIZE);
        if (result == -1) {
            error_handling("read error");
            return NULL;
        } else if ( result == 0) {
            break;
        } else {
            strcat(data, buf);
        }
    }
    return data;
}

int verify_result(char ** result) {
#if DEBUG
    printf("STDOUT:%s\n", result[0]);
    printf("STDERR:%s\n", result[1]);
    printf("RUNTIMEERROR:%d\n", atoi(result[2]));
    printf("STDOUTlen:%ld\n", strlen(result[0]));
    printf("STDERRlen:%ld\n", strlen(result[1]));
#endif

    if (atoi(result[2]) == 0 || strlen(result[1]) > 0)
        // RUNTIME ERROR || EXECUTION ERROR
        return -1;
    else if (atoi(result[2]) == -1)
        // TIMEOUT ERROR
        return -2;
    else
        /* PROGRAM EXIT NORMALLY
         * RETURN OUTPUT TO INSTAGRAPD
         */
        return 0;
}

int closecmd(const pid_t pid, int *pipes)
{
    int status;

    close(pipes[0]);
    close(pipes[1]);
    close(pipes[2]);
    waitpid(pid, &status, 0);
    return status;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

char** str_split(char* a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
    *        knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
