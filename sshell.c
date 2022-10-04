#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 512

int sys(char* cmd){
        pid_t pid;
        char *args[] = {NULL};

        pid = fork();
        if (pid == 0) {
                /* Child */
                execvp(cmd, args);
                perror("execvp");
                exit(1);
        } else if (pid > 0) {
                /* Parent */
                int status;
                waitpid(pid, &status, 0);
                //printf("%d\n", WEXITSTATUS(status)); 
                fprintf(stdout, "Return status value for '%s': %d\n", cmd, WEXITSTATUS(status));
        } else {
                perror("fork");
                exit(1);
        }
        
        return 0;
}

char* parse(cmd){
    //char cmd[] = "echo hello world"; 
    char space[] = " ";
    char *ptr = strtok(cmd, space);
    char *parsed_array[3]; //temporarily set the size of array to 3
    int i = 0;

    while(ptr != NULL){
        parsed_array[i] = ptr;
        ptr = strtok(NULL, space);
        i++;
    }

    /*printf("here\n");  
    for(int i = 0; i < sizeof(parsed_array); i++){
        printf("%s\n", parsed_array[i]);
    }*/
    //Parsed_array is successfully made
    
    
    return parsed_array; //seg_fault
}


int main(void)
{
        char cmd[CMDLINE_MAX];
        char new_cmd = parse(cmd);
        printf("%s\n", new_cmd);

        while (1) {
                char *nl;
                //int retval;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                if (fgets(cmd, CMDLINE_MAX, stdin) == NULL) {
                        break;
                }

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* Regular command */
                sys(cmd);
        }

        return EXIT_SUCCESS;
}
