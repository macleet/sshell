#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define TOKEN_MAX   32
#define ARGS_MAX    16

typedef struct Cmd {
	/* Parse-related members */
	char *original_txt;
	char **args;
	int arg_cnt;

	/* Output Redirection */
	bool redir;
	char *redir_filename; 
} Cmd;

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
void parse(Cmd *cmd_st, char *cmd_txt) {
	char *arg_buf;

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
			if(cmd_txt[i] == '>') {
				cmd_txt[i] = '\0';
				break;
			}
		}
		parse(cmd_st, cmd_txt);
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

bool parse_err_handle(Cmd *cmd_st) {
	if(!strcmp(cmd_st->args[0], ">") || !strcmp(cmd_st->args[0], "|")) {
		/* Missing command */
		perror("Error: missing command\n");
		return true;
	}
	else if(cmd_st->arg_cnt > ARGS_MAX) {
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
	return false;
}

int sys(Cmd *cmd_st) {
	pid_t pid;
	int fd = -1;

	pid = fork();
	if (pid == 0) {
		/* Child */
		if(cmd_st->redir) {
			fd = open(cmd_st->redir_filename, O_WRONLY | O_TRUNC, 0644);
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

int main(void)
{
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

		/* Allocate memory for Command struct that will contain needed and parsed data */
		Cmd *cmd_st = malloc( sizeof(Cmd) );
		cmd_construct(cmd_st);

		/* Parse phase */
		strcpy(cmd_st->original_txt, cmd_txt);  // saving original command line text into original_txt member
		parse(cmd_st, cmd_txt);   // stores parsed value in struct cmd_st
		if(cmd_st->args[0] == NULL) continue;
		if(parse_err_handle(cmd_st)) continue;

		/* Builtin commands */
		if (!strcmp(cmd_st->args[0], "exit")) {
			fprintf(stderr, "Bye...\n");
			fprintf(stderr, "+ completed 'exit' [0]");
			cmd_destruct(cmd_st);
			free(cmd_st);
			break;
		}
		if (!strcmp(cmd_st->args[0], "pwd")) {
			char cwd[CMDLINE_MAX];
			if(getcwd(cwd, sizeof(cwd)) != NULL) {
				fprintf(stdout, "%s\n", cwd);
				fprintf(stderr, "+ completed '%s' [0]\n", cmd_st->original_txt); // hardcoded 0 successful return; is OK? NO
			}
		 	cmd_destruct(cmd_st);
			free(cmd_st);
			continue;
		}
		if(!strcmp(cmd_st->args[0], "cd")) {
			int error = -1;
			if(strcmp(cmd_st->args[1], "..")) {
			}
			else {
				error = chdir((cmd_st->original_txt)+3);
			}

			if(error == 0) {  // successful chdir
				fprintf(stderr, "+ completed '%s' [%d]\n", cmd_st->original_txt, error);
			} 
			else {  // unsuccessful chdir

			}

			cmd_destruct(cmd_st);
			free(cmd_st);
			continue;
		}

		/* Regular commands */
		sys(cmd_st);

		/* Deallocate memory */
		cmd_destruct(cmd_st);
		free(cmd_st);
	}
	return EXIT_SUCCESS;
}