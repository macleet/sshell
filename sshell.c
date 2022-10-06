#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define TOKEN_MAX   32
#define ARGS_MAX    16

typedef struct CmdLine {
	char *name;
	char *args[TOKEN_MAX];
} CmdLine;

void parse(CmdLine *cmd, char *cmdtxt) {
	char *buffer = strtok(cmdtxt, " ");
	for(int i = 0; buffer != NULL; i++) {
		if(i == 0) { cmd->name = buffer; }
		cmd->args[i] = buffer;
		buffer = strtok(NULL, " ");
	}
	return;
}

int sys(char* cmdtxt) {
	pid_t pid;
	CmdLine *cmd = malloc( sizeof(CmdLine) + sizeof(char[16][32]) );

	parse(cmd, cmdtxt);
	pid = fork();
	if (pid == 0) {
		/* Child */
		execvp(cmd->name, cmd->args);
		perror("execvp");
		exit(1);
	} else if (pid > 0) {
		/* Parent */
		int status;
		waitpid(pid, &status, 0);
		fprintf(stdout, "Return status value for '%s': %d\n", cmdtxt, WEXITSTATUS(status));
	} else {
		perror("fork");
		exit(1);
	}
	free(cmd);
	return 0;
}

int main(void)
{
    char cmd[CMDLINE_MAX];

	while (1) {
		char *nl;

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
		if (nl) {
			*nl = '\0';
		}

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
