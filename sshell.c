#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define TOKEN_MAX   32
#define ARGS_MAX    16

typedef struct Cmd {
	char original_txt[CMDLINE_MAX];
	char *args[TOKEN_MAX];
} Cmd;

/* Parses text given in command line */
void parse(Cmd *cmd_st, char *cmd_txt) {
	char *arg_buf = strtok(cmd_txt, " ");
	for(int i = 0; arg_buf != NULL; i++) {
		cmd_st->args[i] = arg_buf;
		arg_buf = strtok(NULL, " ");
	}
	return;
}

int sys(Cmd *cmd_st) {
	pid_t pid;

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

		Cmd *cmd_st = malloc( sizeof(Cmd) + sizeof(char[ARGS_MAX][TOKEN_MAX]) );
		strcpy(cmd_st->original_txt, cmd_txt);
		parse(cmd_st, cmd_txt);   // stores parsed value in struct cmd_st

		/* Builtin commands */
		if (!strcmp(cmd_st->args[0], "exit")) {
			fprintf(stderr, "Bye...\n");
			break;
		}
		else if (!strcmp(cmd_st->args[0], "pwd")) {
			char cwd[CMDLINE_MAX];
			if(getcwd(cwd, sizeof(cwd)) != NULL) {
				fprintf(stdout, "%s\n", cwd);
				fprintf(stderr, "+ completed '%s' [0]\n", cmd_st->original_txt); // hardcoded 0 successful return; is OK?
			}
			continue;
		}
		else if (!strcmp(cmd_st->args[0], "cd")) {
			continue;
		}

		/* Regular commands */
		sys(cmd_st);

		free(cmd_st);
	}
	return EXIT_SUCCESS;
}