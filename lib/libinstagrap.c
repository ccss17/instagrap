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

//
// ERROR MESSAGE
//

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

//
// GET FILE
//

void cleanup_socket(sock_set * sc_sd) {
    shutdown(sc_sd->clnt_sd, SHUT_WR); 
    close(sc_sd->clnt_sd);
    close(sc_sd->serv_sd);
}

sock_set * init_sock(int argc, char * argv[]) {
    sock_set * sc_sd;
    int serv_sd, clnt_sd;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc!=2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    sc_sd = (sock_set *) malloc(sizeof(sock_set));

    sc_sd->serv_sd = socket(PF_INET, SOCK_STREAM, 0);   
    if (sc_sd->serv_sd == -1) error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(sc_sd->serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if(listen(sc_sd->serv_sd, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz=sizeof(clnt_adr);    
    sc_sd->clnt_sd=accept(sc_sd->serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    return sc_sd;
}

int _get_size(int clnt_sd) {
    char buf[FILE_SIZE_INDICATOR];
    read(clnt_sd, buf, FILE_SIZE_INDICATOR );
    return atoi(buf);
}

void get_file(const char * filename, int clnt_sd ){
    FILE * fp;
    char buf[BUF_SIZE];
    int read_cnt;
    int file_size;

    fp = fopen(filename, "wb");
    file_size = _get_size(clnt_sd);
    DPRINT(printf("file size : %d\n", file_size));

    while(1){
        if (file_size < BUF_SIZE) {
            _get_file(fp, clnt_sd, buf, file_size, read_cnt);
            break;
        } else {
            file_size -= BUF_SIZE;
            _get_file(fp, clnt_sd, buf, BUF_SIZE, read_cnt);
        }
    }

    fclose(fp);
}

void _get_file(FILE * fp, int clnt_sd, char buf[], int size, int read_cnt) {
    read_cnt=read(clnt_sd, buf, size );
    DPRINT(printf("buf : %s\n", buf));
    fwrite((void*)buf, 1, read_cnt, fp);
}

//
// FORK CHILD PROCESS 
//

int is_exists(char * fname) {
    if( access( fname, F_OK) != -1 ) {
        return 1;
    } else {
        return 0;
    }
}

char * build_target() {
    /*
    return "0..." program exit normally
    return "1" failed build
    return "2" runtime error
    return "3" timeout error
    */
    int flag; // 0 : PROGRAM NORMALLY EXIT, -1 : PROGRAM ERROR, -2 : TIMEOUT ERROR
    int build_success;
    char ** result;
    char * feedback;
    char * CMD_BUILD[] = {
        COMPILER, TARGET_FILE, 
        "-O2", "-lm", "-static", "-DONLINE_JUDGE", "-DBOJ", NULL
    };
    char * CMD_EXECUTE_TARGET[] = { OUTPUT_FILE, NULL };

    result = _execute(CMD_BUILD);
    feedback = (char *) malloc(sizeof(char));
    
    build_success = is_exists(OUTPUT_FILE);

    if (build_success){
        // BUILD SUCCESS
        result = _execute(CMD_EXECUTE_TARGET);
        flag = verify_result(result);
        if (flag == -1) {
            /* RUNTIME ERROR */
            strcpy(feedback, "2");
        } else if (flag == -2){
            /* TIMEOUT ERROR */
            strcpy(feedback, "3");
        }
        else {
            /* PROGRAM EXIT NORMALLY
             * RETURN OUTPUT TO INSTAGRAPD
             */
            free(feedback);
            feedback = (char *) malloc(sizeof(char) * strlen(result[0]) + 1);
            strcpy(feedback, "0");
            strcat(feedback, result[0]);
        }
        remove(OUTPUT_FILE);
    } else {
        // BUILD FAIL
        strcpy(feedback, "1");
    }
    remove(TARGET_FILE);
    remove(TESTCASE_FILE);
    return feedback;
}

char ** _execute(char * args[]){
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
    DPRINT(printf("STDERR: %s\n", result[1]));
    DPRINT(printf("RUNTIMEERROR: %d\n", atoi(result[2])));
    DPRINT(printf("STDOUTlen: %ld\n", strlen(result[0])));
    DPRINT(printf("STDERRlen: %ld\n", strlen(result[1])));

    if (atoi(result[2]) == 0 || strlen(result[1]) > 0)
    // RUNTIME ERROR || EXECUTION ERROR
        return -1;
    else if (atoi(result[2]) == -1)
    // TIMEOUT ERROR
        return -2;
    else
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
