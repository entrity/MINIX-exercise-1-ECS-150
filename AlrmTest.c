/*

Creates two child processes that each increments a process-local variable
count in an infinite loop. The parent is just a for loop of 5 iterations,
where in each interation it sleeps for a second and then sends the SIGALRM
signal to each child. Upon receiving this signal, each child is to print out
the value of count along with its process id and reset the count to zero. Your
output might look like:

*/

#include <stdlib.h> // for the exit call
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

pid_t child1id, child2id;
struct sigaction printAction;
sigset_t block_mask;
int count = 0;

inline void display (int);
inline void fatal (char * str);

void main ()
{
	int i; // counter for parent's for loop
	// build sigaction for children
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGINT);
	printAction.sa_mask = block_mask;
	printAction.sa_handler = display;
	sigaction(SIGALRM, &printAction, NULL);
	// fork
	switch (child1id = fork()) {
		case -1:
			fatal("failed fork");
			break;
		case 0: // child 1
			while (1)
				count ++;
			break;
		default: // parent
			switch (child2id = fork()) {
				case -1:
					fatal("failed fork");
					break;
				case 0: // child 2
					while (1)
						count ++;
					break;
				default: // parent
					// iterate 5 times
					for (i = 0; i < 5; i++) {
						sleep(1);
						kill(child1id, SIGALRM);
						kill(child2id, SIGALRM);
					}
					fflush(stdout);
					kill(child1id, SIGINT);
					kill(child2id, SIGINT);
					break;
			}
			break;
	}
}

inline void display (int sig)
{
	printf("Count is %10d, my pid is %d\n", count, getpid());
	count = 0;
}

inline void fatal (char * str)
{
	perror(str);
	exit(errno);
}
