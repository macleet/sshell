#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#define CMDLINE_MAX 512
#define TOKEN_MAX   32
#define ARGS_MAX    16

typedef struct Cmd {
   char original_txt[CMDLINE_MAX];
   char *args[TOKEN_MAX];
} Cmd;

char* CDfunction(char *second, char *prev_dir){
	char cwd[CMDLINE_MAX];
	char* newDir; /*This will be new path*/
	char* curDir = getcwd(cwd, sizeof(cwd));
	
	char* newPrev_dir = malloc(strlen(curDir) + 1);
	strcpy(newPrev_dir, curDir); 
	//printf("hi %s\n", newPrev_dir);
	/*chdir("-");*/
	//printf("hi %s\n", curDir);

	//fprintf(stdout, "prevDir INSIDE CD , '%s'\n", prev_dir);


	
	if(opendir(second) == NULL){
		return NULL;
	}
	else{
		if(!chdir(second)){
			newDir = getcwd(cwd, sizeof(cwd));
			fprintf(stdout, "%s\n", newDir);
		}
	}

	if(second == NULL){
		second = getenv("HOME");
    	if(!chdir(second)){
         
			newDir = getcwd(cwd, sizeof(cwd));
			fprintf(stdout, "%s\n", newDir);
    	}
	}
	/*cd .. go to parent directory*/
	if (!strcmp(second, "..")){
		if(!chdir("..")){
    		newDir = getcwd(cwd, sizeof(cwd));
    		fprintf(stdout, "%s\n", newDir);
    	}   
	}

	if (!strcmp(second, "/")){
    	if(!chdir("/")){
        	newDir = getcwd(cwd, sizeof(cwd));
        	fprintf(stdout, "%s\n", newDir);
    	}   
	}


	if (!strcmp(second, "-")){
    	fprintf(stdout, "hello, '%s'\n", prev_dir);
      
    	if(!chdir(prev_dir)){
        	newDir = getcwd(cwd, sizeof(cwd));
        	fprintf(stdout, "%s\n", newDir);
    	}   
	}
	
   
	//fprintf(stdout, "prevCD, '%s'\n", curDir);
	return newPrev_dir;
}



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
	char prev_dir[CMDLINE_MAX];

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
			free(cmd_st);
			break;
      	}
      	else if (!strcmp(cmd_st->args[0], "pwd")) {
         	char cwd[CMDLINE_MAX];
        	if(getcwd(cwd, sizeof(cwd)) != NULL) {
        		fprintf(stdout, "%s\n", cwd);
        		fprintf(stderr, "+ completed '%s' [0]\n", cmd_st->original_txt); // hardcoded 0 successful return; is OK?
        	}
        	free(cmd_st);
        	continue;
    	}
    	else if (!strcmp(cmd_st->args[0], "cd")) {
        	char* second = cmd_st->args[1];
        	//fprintf(stdout, "prevDir before going in , '%s'\n", prev_dir);

        	char* new_dir = CDfunction(second, prev_dir);
			
			if(new_dir == NULL){
				fprintf(stderr, "Error: cannot cd into directory\n");
			}

			//fprintf(stdout, "main, '%s'\n", new_dir);
			strcpy(prev_dir, new_dir);
			//fprintf(stdout, "prevDir is, '%s'\n", new_dir);
			/*if(!strcmp(second, "..")){
			printf("here, '%s'\n", second);
			}*/
			fprintf(stderr, "+ completed '%s' [0]\n", cmd_st->original_txt);   
			free(new_dir);
			free(cmd_st);
			continue;
    	}

		/* Regular commands */
		sys(cmd_st);

		free(cmd_st);
	}
	return EXIT_SUCCESS;
}