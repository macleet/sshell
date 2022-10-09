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
void parse(Cmd *cmd_st, char *cmd_txt) {
	char *arg_buf = strtok(cmd_txt, " ");
	for(int i = 0; arg_buf != NULL; i++) {
		if(i == 0) { cmd_st->name = arg_buf; }
		cmd_st->args[i] = arg_buf;
		arg_buf = strtok(NULL, " ");
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
		sys(cmd);
	}
	return EXIT_SUCCESS;
}
