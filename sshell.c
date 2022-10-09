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

// struct

/* Parses text given in command line */
void parse(CmdLine *cmd, char *cmdtxt) {
	char *arg_buf = strtok(cmdtxt, " ");
	for(int i = 0; arg_buf != NULL; i++) {
		if(i == 0) { cmd->name = arg_buf; }
		cmd->args[i] = arg_buf;
		arg_buf = strtok(NULL, " ");
	}
	return;
}

/* Built-in Commands: exit, pwd, cd */
int builtinCmds(char *cmd) {
	if (!strcmp(cmd, "exit")) {
		fprintf(stderr, "Bye...\n");
		return 1;
	}
	else {
		char cwd[CMDLINE_MAX] = getcwd(cwd, sizeof(cwd));
		if (!strcmp(cmd, "pwd")) {
			if(cwd != NULL) {
				fprintf(stdout, "%s\n", cwd);
				fprintf(stderr, "+ completed '%s' [0]\n", cmd);
			}
		}
		else if (!strcmp(cmd, "cd")) {
			
		}
	}
	return 0;
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
		fprintf(stderr, "+ completed '%s' [%d]", cmdtxt, WEXITSTATUS(status));
		// fprintf(stderr, "Return status value for '%s': %d\n", cmdtxt, WEXITSTATUS(status));
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
		char *nl = strchr(cmd, '\n');
		if (nl) { *nl = '\0'; }

		/* Builtin commands */
		if(builtinCmds(cmd) == 1) break;  // returns 1 on exit cmd
		else continue;					  // else cmd is pwd or cd

		/* Regular commands */
		sys(cmd);
	}
	return EXIT_SUCCESS;
}
