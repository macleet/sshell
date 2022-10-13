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

	/* Path args */
	if(strstr(cmd_txt, "cd ")) {
		int i;
		arg_buf = strtok(cmd_txt, " /");
		for(i = 0; arg_buf != NULL; i++) {
			cmd_st->args[i] = arg_buf;
			cmd_st->arg_cnt++;
			arg_buf = strtok(NULL, " /");
		}
		// cmd_st->path_cnt = i;
	}

	/* General args */
	arg_buf = strtok(cmd_txt, " ");
	for(int i = 0; arg_buf != NULL; i++) {
		cmd_st->args[i] = arg_buf;
		// strcpy(cmd_st->args[i], arg_buf);
		cmd_st->arg_cnt++;
		arg_buf = strtok(NULL, " ");
	}

	return;
}

int sys(Cmd *cmd_st) {
	pid_t pid;
	int fd = -1;
	if(cmd_st->redir) {
		fd = open(cmd_st->redir_filename, O_WRONLY | O_CREAT, 0644);
		dup2(fd, STDOUT_FILENO);
	}

	pid = fork();
	if (pid == 0) {
		/* Child */
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

	if(cmd_st->redir) {
		dup2(STDERR_FILENO, STDOUT_FILENO);
		close(fd);
	}

	return 0;
}

int main(void)
{
    char cmd_txt[CMDLINE_MAX];

	while (1) {
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
		if (nl) { *nl = '\0'; }

		/* Allocate memory for Command struct that will contain needed and parsed data */
		Cmd *cmd_st = malloc( sizeof(Cmd) );
		cmd_construct(cmd_st);

		/* Parse phase */
		strcpy(cmd_st->original_txt, cmd_txt);  // saving original command line text into original_txt member
		parse(cmd_st, cmd_txt);   // stores parsed value in struct cmd_st

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