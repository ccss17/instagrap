#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "instagrap.h"

volatile int flag_timeout = 0;
volatile int flag_child_done = 0;

void child_handler(int sig) { flag_child_done = 1; }
void alarm_handler(int sig) { flag_timeout = 1; }

sock_set * init_sock(char * listen_port){
    DPRINT(printf("port:%s\n", listen_port));
    sock_set * sc_sd;
    int serv_sd, clnt_sd;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    sc_sd = (sock_set *) malloc(sizeof(sock_set));

    sc_sd->serv_sd = socket(PF_INET, SOCK_STREAM, 0);   
    if (sc_sd->serv_sd == -1) error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(listen_port));

    if(bind(sc_sd->serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if(listen(sc_sd->serv_sd, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz=sizeof(clnt_adr);    
    sc_sd->clnt_sd=accept(sc_sd->serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    return sc_sd;
}

void cleanup_socket(sock_set * sc_sd) {
    shutdown(sc_sd->clnt_sd, SHUT_WR); 
    close(sc_sd->clnt_sd);
    close(sc_sd->serv_sd);
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
    data_s->data = (char *)malloc(sizeof(char) * data_s->size);
    tmp_size = data_s->size;

    tmp_size -= BUF_SIZE;
    read(sock, buf, BUF_SIZE );
    strcpy(data_s->data, buf);

    while(1) {
        if (tmp_size <= 0)
            break;
        // get data piece
        else if (tmp_size < BUF_SIZE) {
            read(sock, buf, tmp_size );
            strcat(data_s->data, buf);
            break;
        } else {
            tmp_size -= BUF_SIZE;
            read(sock, buf, BUF_SIZE );
            strcat(data_s->data, buf);
        }
    }
    return data_s;
}


void save_file(const char * filename, data_set *data_s) {
    FILE * fp;

    DPRINT(printf("saved filename:%s\n", filename));
    DPRINT(printf("saved data:%s\n", data_s->data));

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
    execute(CMD_BUILD);

    if (exists(DEFAULT_OUTPUT_FILE)){
        // BUILD SUCCESS
        return 0;
    } else {
        // BUILD FAIL
        return -1;
    }
}

char ** execute(char * args[]) {
    char ** result;
    int pipes[PIPE_COUNT];

    result = (char **) malloc(sizeof(char *) * PIPE_COUNT - 1);
    _fork_subprocess(pipes, args);

    // result[0] -> stdout of child child process (actually execute program)
    // result[1] -> stderr of child child process
    // result[2] -> stdout of child process (check exit status of child child process)
    result[0] = _read_pipe(pipes[1], 0);
    result[1] = _read_pipe(pipes[2], 0); 
    result[2] = _read_pipe(pipes[3], 1); 

    return result;
}

pid_t _fork_subprocess(int * pipes, char ** args)
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

    result = read(pfd, data, BUF_SIZE);
    if (check_runtimerror) return data;

    for(;;){
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
    DPRINT(printf("STDOUT:%s\n", result[0]));
    DPRINT(printf("STDERR:%s\n", result[1]));
    DPRINT(printf("RUNTIMEERROR:%d\n", atoi(result[2])));
    DPRINT(printf("STDOUTlen:%ld\n", strlen(result[0])));
    DPRINT(printf("STDERRlen:%ld\n", strlen(result[1])));

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