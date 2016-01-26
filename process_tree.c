/*

	Takes an integer argument n from the command line and creates a binary
	tree of processes of depth n. After the tree is created, each process
	should display the following phrase and then terminate.

		I am process x; my process pid is <pid>;
		My parent's pid is <ppid>	

	The original process does not terminate until all of its children have died.  

*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define LEFT 0
#define RIGHT 1

int n;
int waitStatus;
pid_t topPid;
int x = 1; // process identifier
struct sigaction act;
int child1, child2;
int isWaitingLeft, isWaitingRight;

int spawnOne (int, int);
void handleInterrupt (int);
void swansong ();

void handleInterrupt (int sig)
{
	switch (sig) {
		case SIGUSR1:
			isWaitingRight = 0;
			break;
		case SIGUSR2:
			isWaitingLeft = 0;
			break;
		default:
			swansong();
			break;
	}
}

void swansong ()
{
	printf("I am process %d; my process pid is %d\nMy parent's process pid is %d\n\n", x, getpid(), getppid());
	if (child1) {
		kill(child1, SIGALRM);
		waitpid(child1, &waitStatus, 0);
	}
	if (child2) {
		kill(child2, SIGALRM);
		waitpid(child2, &waitStatus, 0);
	}
}

int runProc (int isRight) {
	if (--n > 0) { // non-leaf node
		// printf("spawn %d\n", x);
		isWaitingLeft = 1;
		isWaitingRight = 1;
		if ((child1 = spawnOne(2*x, LEFT)) == -1)
			exit(errno);
		if ((child2 = spawnOne(2*x+1, RIGHT)) == -1)
			exit(errno);
		while (isWaitingLeft || isWaitingRight)
			pause();
		swansong();
	} else if (x == 1) {
		swansong();
	} else { // leaf node
		isWaitingLeft = 0;
		isWaitingRight = 0;
		child1 = child2 = 0;
		// printf("leaf node %d\n", x);
		kill( getppid(), isRight ? SIGUSR1 : SIGUSR2 );
		pause();
	}
}

int spawnOne (int newX, int isRight )
{
	int pid;
	switch (pid = fork()) {
		case -1: // error
			perror("bad fork\n");
			exit(errno);
			break;
		case 0: // child
			x = newX;
			runProc(isRight);
			break;
		default: // parent
			return pid;
	}
}

int main (int argc, char * argv[])
{
	topPid = getpid();
	// Enforce usage
	if (argc < 2) {
		printf("Usage %s <integer>\n\tYou supplied only %d args\n", argv[0], argc);
		return E2BIG;
	}
	// Set signal handler
	sigfillset(&act.sa_mask);
	act.sa_handler = handleInterrupt;
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	sigaction(SIGALRM, &act, NULL);
	// Get user input
	sscanf(argv[1], "%d", &n);
	// Start tree of processes
	runProc(0);
	// Success
	return 0;
}
