#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define TOKEN_MAX   32
#define ARGS_MAX    16

typedef struct Cmd {
	char *name;
	char *args[TOKEN_MAX];
} Cmd;

/* Parses text given in command line */
void parse(Cmd *cmd_st, char *cmd_txt) {
	char *arg_buf = strtok(cmd_txt, " ");
	for(int i = 0; arg_buf != NULL; i++) {
		if(i == 0) { cmd_st->name = arg_buf; }
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
		execvp(cmd_st->name, cmd_st->args);
		perror("execvp");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		/* Parent */
		int status;
		waitpid(pid, &status, 0);
		fprintf(stderr, "+ completed '%s' [%d]", cmd_st->args, WEXITSTATUS(status));
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
		parse(cmd_st, cmd_txt);   // stores parsed value in struct cmd_st

		/* Builtin commands */
		if (!strcmp(cmd_st->name, "exit")) {
			fprintf(stderr, "Bye...\n");
			break;
		}
		else if (!strcmp(cmd_st->name, "pwd")) {
			char cwd[CMDLINE_MAX] = getcwd(cwd, sizeof(cwd)); // CMDLINE_MAX correct use here? should there be another max macro
			if(cwd != NULL) {
				fprintf(stdout, "%s\n", cwd);
				fprintf(stderr, "+ completed '%s' [0]\n", cmd_txt); // hardcoded 0 successful return; is OK?
			}
		}
		else if (!strcmp(cmd_st->name, "cd")) {
			
		}
		
		/* Regular commands */
		sys(cmd_st);

		free(cmd_st);
	}
	return EXIT_SUCCESS;
}