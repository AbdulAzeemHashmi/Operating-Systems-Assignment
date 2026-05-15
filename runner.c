#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

double get_time_diff(struct timeval start, struct timeval end) {
    return (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
}

// 
Logic to handle quoted arguments
int parse_line(char *line, char **args) {
    int count = 0;
    char *p = line;
    
    while (*p && count < MAX_ARGS - 1) {
        while (*p == ' ') 
            p++; // Skip spaces
            
        if (*p == '\0') 
            break;

        if (*p == '\"') { // Start of quoted string
            p++;
            args[count++] = p;
            
            while (*p && *p != '\"') 
                p++;
        } 
        
        else { // Regular word
            args[count++] = p;
            while (*p && *p != ' ') 
                p++;
        }

        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }
    args[count] = NULL;
    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <file> <timeout> <mode>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    
    if (!file) { 
        perror("File error"); 
        return 1; 
    }

    int timeout = atoi(argv[2]);
    bool strict = (strcmp(argv[3], "strict") == 0);
    char line[MAX_CMD_LEN];
    int idx = 0, total = 0, ok = 0, fail = 0, to_count = 0, sig_count = 0;

    while (fgets(line, sizeof(line), file)) {
    
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 0 || line[0] == ' ') 
            continue;

        char original_line[MAX_CMD_LEN];
        strcpy(original_line, line); // Copy for printing

        char *args[MAX_ARGS];
        if (parse_line(line, args) == 0) 
            continue;

        total++; 
        idx++;
        struct timeval start, end;
        gettimeofday(&start, NULL);

        pid_t pid = fork();
        
        if (pid == 0) {
            execvp(args[0], args);
            perror("Exec failed");
            exit(127);
        } 
        
        else {
            int status;
            bool timed_out = false;
            
            while (1) {
                pid_t res = waitpid(pid, &status, WNOHANG);
                gettimeofday(&end, NULL);
                
                if (res > 0) 
                    break;
                    
                if (get_time_diff(start, end) >= timeout) {
                    timed_out = true;
                    kill(pid, SIGTERM);
                    sleep(1);
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);
                    break;
                }
                usleep(10000);
            }

            double final_time = get_time_diff(start, end);
            printf("[%d] CMD=\"%s\" => ", idx, original_line);

            bool is_failure = false;
            
            if (timed_out) {
                printf("RESULT=TIMEOUT TIME=%.2f\n", final_time);
                to_count++; 
                is_failure = true;
            } 
            
            else if (WIFEXITED(status)) {
                int code = WEXITSTATUS(status);
                printf("RESULT=EXIT(%d) TIME=%.2f\n", code, final_time);
                
                if (code == 0) 
                    ok++; 
                    
                else { 
                    fail++; 
                    is_failure = true; 
                }
            } 
            
            else if (WIFSIGNALED(status)) {
                printf("RESULT=SIGNAL(%d) TIME=%.2f\n", WTERMSIG(status), final_time);
                sig_count++;
                is_failure = true;
            }

            if (strict && is_failure) {
                fprintf(stderr, "Strict mode: Aborting.\n");
                fclose(file);
                abort();
            }
        }
    }

    if (!strict) {
        printf("SUMMARY total=%d ok=%d fail=%d timeout=%d signaled=%d\n", 
                total, ok, fail, to_count, sig_count);
    }
    
    fclose(file);
    return 0;
}
