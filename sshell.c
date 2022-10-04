#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 512

int sys(char* cmd) {
	pid_t pid;
	char *args[] = {NULL};  // Arg set to NULL for now : Phase 1 (no argu
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
		fprintf(stdout, "Return status value for '%s': %d\n", cmd, WEXITSTATUS(status));  // Q: why WEXITSTATUS evaluation rather than directly giving status int value??
	} else {
		perror("fork");
		exit(1);
	}
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
