#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>

// TO DO: free location change, cmd destruct to be in for loop

#define CMDLINE_MAX  512
#define TOKEN_MAX    32
#define ARGS_MAX     16
#define PIPE_CMD_MAX 4

typedef struct Cmd {
	/* Parse-related members */
	char *original_txt;
	char **args;
	int arg_cnt;

	/* Output Redirection */
	bool redir;
	char *redir_filename; 
} Cmd;

typedef struct CmdStorage
{
	Cmd **cmd_arr;
	int cmd_cnt;
	int pipe_cnt;
} CmdStorage;

void cmd_construct(Cmd *cmd_st) {
	cmd_st->original_txt = (char*) malloc(CMDLINE_MAX * sizeof(char));
	cmd_st->redir_filename = (char*) malloc(FILENAME_MAX * sizeof(char));
	cmd_st->args = (char**) calloc(ARGS_MAX , sizeof(char*));
	cmd_st->redir = false;
	cmd_st->arg_cnt = 0;
	return;
} 

void cmd_destruct(Cmd *cmd_st) {
	free(cmd_st->original_txt);
	free(cmd_st->redir_filename);
	for(int i = 0; i < cmd_st->arg_cnt; i++) {
		free(cmd_st->args[i]);
	}
	free(cmd_st->args);
	return;	
}

/* Parses text given in command line */
void parse(Cmd **cmd_arr, char *cmd_txt) {
	char *arg_buf;

	/*** Pipeline parsing ***/
	if(strchr(cmd_txt, '|')) {
		char piped_commands[PIPE_CMD_MAX][ARGS_MAX];
		int cmd_count = 0;
		// Parse/extract command strings delimited by |
		// and store in piped_commands string array
		arg_buf = strtok(cmd_txt, "|");
		for(int i = 0; arg_buf != NULL; i++) {
			// Parse commands individually and store to a Cmd struct
			strcpy(piped_commands[i], arg_buf);
			cmd_count++;
			arg_buf = strtok(NULL, "|");
		}
		for(int i = 0; i < cmd_count; i++) {
			strcpy(cmd_txt, piped_commands[i]);
			arg_buf = strtok(cmd_txt, " ");
			for(int j = 0; arg_buf != NULL; j++) {
				cmd_arr[i]->args[j] = calloc(TOKEN_MAX , sizeof(char));
				cmd_arr[i]->arg_cnt++;

				strcpy(cmd_arr[i]->args[j], arg_buf);
				arg_buf = strtok(NULL, " ");
			}
		}
		return;
	}

	/*** Non-pipeline parsing ***/
	Cmd *cmd_st = cmd_arr[0];

	/* Redirection parsing */
	if(strchr(cmd_txt, '>')) {
		cmd_st->redir = true;
		char *inst = strchr(cmd_txt, '>');  // inst holds the instruction to shell e.g. "> file.txt"

		/* Gets file name of file to which output is redirected */
		arg_buf = strtok(inst, "> ");
		while(arg_buf != NULL) {
			strcpy(cmd_st->redir_filename, arg_buf);
			arg_buf = strtok(NULL, "> ");
		}

		/* Extract only arguments from cmd_txt */
		for(int i = 0; i < (int) strlen(cmd_txt); i++) {
			// Replaces > with null character --> cmd_txt = string of entire command w/o >
			if(cmd_txt[i] == '>') {
				cmd_txt[i] = '\0';
				break;
			}
		}

		parse(cmd_arr, cmd_txt);
		return;
	}

	/* Path parsing */
	if(strstr(cmd_txt, "cd ")) {
		int i;
		arg_buf = strtok(cmd_txt, " /");
		for(i = 0; arg_buf != NULL; i++) {
			cmd_st->args[i] = malloc(TOKEN_MAX * sizeof(char));
			cmd_st->arg_cnt++;

			strcpy(cmd_st->args[i], arg_buf);
			arg_buf = strtok(NULL, " /");
		}
		return;
	}

	/* General case parsing */
	arg_buf = strtok(cmd_txt, " ");
	for(int i = 0; arg_buf != NULL; i++) {
		cmd_st->args[i] = calloc(TOKEN_MAX , sizeof(char));
		cmd_st->arg_cnt++;

		strcpy(cmd_st->args[i], arg_buf);
		arg_buf = strtok(NULL, " ");
	}

	return;
}

bool parse_err_handle(Cmd **cmd_arr, int cmd_cnt) {
	for(int i = 0; i < cmd_cnt; i++) {
		if(!strcmp(cmd_arr[i]->args[0], ">") || !strcmp(cmd_arr[i]->args[0], "|")) {
			/* Missing command */
			perror("Error: missing command\n");
			return true;
		}
		else if(cmd_arr[i]->arg_cnt > ARGS_MAX) {
			/* Exceed max number of args */
			perror("Error: too many process arguments\n");
			return true;
		}
		// else if() {
		// 	/* No output file */
		// }
		// else if() {
		// 	/* Mislocated output redirection */
		// }
	}
	return false;
}

int sys(Cmd *cmd_st) {
	pid_t pid;
	int fd = -1;

	pid = fork();
	if (pid == 0) {
		/* Child */
		if(cmd_st->redir) {
			fd = open(cmd_st->redir_filename, O_WRONLY | O_TRUNC | O_CREAT , 0644);
			dup2(fd, STDOUT_FILENO);
			fflush(stdout);
			close(fd);
		}
		execvp(cmd_st->args[0], cmd_st->args);
		perror("execvp");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		/* Parent */
		int status;
		waitpid(pid, &status, 0);
		fprintf(stderr, "+ completed '%s' [%d]\n", cmd_st->original_txt, WEXITSTATUS(status));
	} else {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	return 0;
}

void pipeline(CmdStorage *cmd_storage) {
	Cmd **cmd_arr = cmd_storage->cmd_arr;
	Cmd *first_cmd = cmd_arr[0];

    int status[4];
    static int exitstatus[4];
    int* fd_arr[3];  // need three fd when there are 3 pipes

    pid_t pid1;  // basic pid for single pipe
    pid_t pid2;  // pid2 reused in the for loop below, no need to make more pids

    int fd[2]; 
    int fd2[2];
    int fd3[2];
 
    fd_arr[0] = fd;  // keep fds in fd array to access inside the loop
    fd_arr[1] = fd2;
    fd_arr[2] = fd3;

    for(int i = 0; i < cmd_storage->pipe_cnt; i++){ 
        pipe(fd_arr[i]);  // pipe twice, if there are two pipes
    }

	pid1 = fork();
    if(pid1 == 0){ 
		/* child */
        close(fd_arr[0][0]);  // fd_arr[0] is fd1
		dup2(fd_arr[0][1], STDOUT_FILENO);
		close(fd_arr[0][1]);
		execvp(first_cmd->args[0], first_cmd->args);
        perror("execvp");
		exit(EXIT_FAILURE);          
    }
	else{ /* parent */
    	waitpid(pid1, &status[0], 0);
        exitstatus[0] = WEXITSTATUS(status[0]);
	}
    
    for(int j = 0; j < cmd_storage->pipe_cnt; j++){
        pid2 = fork();
	    if(pid2 != 0) {  
			/* parent */ 
            close(fd_arr[j][0]);  // fd_arr[0] = first fd
            close(fd_arr[j][1]);
            waitpid(pid2, &status[j + 1], 0);  
            exitstatus[j + 1] = WEXITSTATUS(status[j + 1]);
                           
        }
	    else {  
			/* child */
            close(fd_arr[j][1]);  // fd_arr[0] = first fd
            dup2(fd_arr[j][0], STDIN_FILENO);
            close(fd_arr[j][0]);
            if(j != (cmd_storage->pipe_cnt - 1)) {  // when j is NOT at the last value, you also have to close next fd
                close(fd_arr[j + 1][0]);
                dup2(fd_arr[j + 1][1], STDOUT_FILENO);
                close(fd_arr[j + 1][1]);
            }
            execvp(cmd_arr[j + 1]->args[0], cmd_arr[j + 1]->args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
	fprintf(stderr, "+ completed '%s' [%d][%d][%d]\n", first_cmd->original_txt, exitstatus[0], exitstatus[1], exitstatus[2]);
	return;	
}

int getPipeCnt(char* cmd_txt) {
	int count = 0;
	for(int i = 0; i <= (int) strlen(cmd_txt); i++) {
		if(cmd_txt[i] == '|') count++;
	}
	return count;
}

int main(void) {
	while (1) {
	    char *cmd_txt = calloc(CMDLINE_MAX , sizeof(char));

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

		/* Get command line */
		if (fgets(cmd_txt, CMDLINE_MAX, stdin) == NULL) {
			break;
		}

		/* Print command line if stdin is not provided by terminal */
		if (!isatty(STDIN_FILENO)) {
			printf("%s", cmd_txt);
			fflush(stdout);
		}

		/* Remove trailing newline from command line */
		char *nl = strchr(cmd_txt, '\n');
		if (nl) { 
			*nl = '\0'; 
		}
		
		CmdStorage *cmd_storage = malloc(sizeof(CmdStorage));
		cmd_storage->cmd_cnt = 1; // 1 bc always at least 1 command at this point
		cmd_storage->cmd_cnt += getPipeCnt(cmd_txt);  // cmd_cnt essentially used to check if commands are pipelined 
		cmd_storage->pipe_cnt = cmd_storage->cmd_cnt - 1;

		/* Allocate memory for CmdStorage and its contents that will contain needed and parsed data */
		if(cmd_storage->cmd_cnt > 1) {
			cmd_storage->cmd_arr = calloc(cmd_storage->cmd_cnt+1 , sizeof(Cmd*));
			for(int i = 0; i < cmd_storage->cmd_cnt; i++) {
				cmd_storage->cmd_arr[i] = malloc(sizeof(Cmd));
				cmd_construct(cmd_storage->cmd_arr[i]);
			}
		}
		else {
			cmd_storage->cmd_arr = calloc(2 , sizeof(Cmd*));
			cmd_storage->cmd_arr[0] = malloc(sizeof(Cmd));
			cmd_construct(cmd_storage->cmd_arr[0]);
		}
		
		Cmd *cmd_st = cmd_storage->cmd_arr[0];  // Used for non-pipe command + improves clarity

		/* Parse phase */
		strcpy(cmd_st->original_txt, cmd_txt);  // saving original command line text into original_txt member as it is lost while parsing
		parse(cmd_storage->cmd_arr, cmd_txt);  // parsed values --> cmd_storage
		if(cmd_st->args[0] == NULL) continue;
		if(parse_err_handle(cmd_storage->cmd_arr, cmd_storage->cmd_cnt)) continue;

		/* Pipelined commands */
		if(cmd_storage->cmd_cnt > 1) {
			pipeline(cmd_storage);
			
			// Deallocate HERE!!
			continue;
		}

		/* Builtin commands */
		// Buitin commands never run with pipe
		if (!strcmp(cmd_st->args[0], "exit")) {
			fprintf(stderr, "Bye...\n");
			fprintf(stderr, "+ completed 'exit' [0]\n");

			// Deallocate memory before break
			cmd_destruct(cmd_st);
			free(cmd_txt);
			free(cmd_st);
			free(cmd_storage->cmd_arr);
			free(cmd_storage);
			break;
		}
		if (!strcmp(cmd_st->args[0], "pwd")) {
			char *cwd = calloc(CMDLINE_MAX , sizeof(char));

			if(getcwd(cwd, CMDLINE_MAX * sizeof(char)) != NULL) {
				fprintf(stdout, "%s\n", cwd);
				fprintf(stderr, "+ completed '%s' [0]\n", cmd_st->original_txt);
			} 
			else {
				fprintf(stderr, "+ completed '%s' [1]\n", cmd_st->original_txt);
			}

			// Deallocate memory before continue
		 	cmd_destruct(cmd_st);
			free(cmd_st);
			free(cmd_txt);
			free(cwd);
			free(cmd_storage->cmd_arr);
			free(cmd_storage);
			continue;
		}
		if(!strcmp(cmd_st->args[0], "cd")) {
			int error = -1;
			bool not_found = false;

			// +3 "skips" the "cd " and accesses from the beginning of path string
			// e.g. "cd /home/jbond/supersecretfolder" --> "/home/jbond/supersecretfolder" 
			error = chdir((cmd_st->original_txt)+3);
			if(error == -1) not_found = true;

			/* Error handling */
			if(not_found) {
				perror("Error: cannot cd into directory\n");
			}
			if(error == 0) {
				fprintf(stderr, "+ completed '%s' [0]\n", cmd_st->original_txt);
			} 
			else {
				fprintf(stderr, "+ completed '%s' [1]\n", cmd_st->original_txt);
			}

			// Deallocate memory before continue
			cmd_destruct(cmd_st);
			free(cmd_txt);
			free(cmd_st);
			free(cmd_storage->cmd_arr);
			free(cmd_storage);
			continue;
		}

		/* Regular commands */
		sys(cmd_st);

		/* Deallocate memory */
		cmd_destruct(cmd_st);
		free(cmd_txt);
		free(cmd_st);
		free(cmd_storage->cmd_arr);
		free(cmd_storage);
	}
	return EXIT_SUCCESS;
}