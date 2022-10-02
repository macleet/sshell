 CFLAGS = -Wall -Wextra -Werror

	ifeq ($(D),1)
	CFLAGS += -g # Enable debugging
	else
	CFLAGS += -O2 # Enable optimization
	endif


sshell: sshell.c
	gcc $(CFLAGS) -o sshell sshell.c

clean:
	rm -f sshell
