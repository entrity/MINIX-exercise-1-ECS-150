/*

	Takes an integer argument n from the command line and creates a binary
	tree of processes of depth n. After the tree is created, each process
	should display the following phrase and then terminate.

		I am process x; my process pid is <pid>;
		My parent's pid is <ppid>	

	The original process does not terminate until all of its children have died.  

*/

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int n;
pid_t topPid;
pid_t * ptree;
int leafct, leafLimit;
int x = 1; // process identifier
struct sigaction interruptAction;
struct sigaction stopAction;
int child1, child2;

void spawn2 ();
int spawn (int);
void handleInterrupt (int);
void swansong (int);

int main (int argc, char * argv[])
{
	topPid = getpid();
	// Enforce usage
	if (argc < 2) {
		printf("Usage %s <integer>\n\tYou supplied only %d args\n", argv[0], argc);
		return E2BIG;
	}
	// Set signal handler
	interruptAction.sa_handler = handleInterrupt;
	sigaction(SIGINT, &interruptAction, NULL);
	stopAction.sa_handler = swansong;
	sigaction(SIGTERM, &stopAction, NULL);
	// Get user input
	sscanf(argv[1], "%d", &n);
	leafLimit = pow(2, n - 1);
	// Start tree of processes
	spawn2();
	// Success
	return 0;
}

void outliveChild (int pid) {
	int status;
	do {
		waitpid(pid, &status, 0);
	} while (! WIFEXITED(status) );
}

void swansong (int sig)
{
	printf("swan %d %d\n", child1, child2);
	if (child1)
		kill(child1, SIGTERM);
	if (child2)
		kill(child2, SIGTERM);
	printf("I am process %d; my process pid is %d\nMy parent's process pid is %d\n\n", x, getpid(), getppid());
	if (n <= 0) // leaf
		exit(0);
}

void spawn2 ()
{
	n--;
	if (n > 0) {
		child1 = spawn(x*2);
		if (child1 <= 0) return;
		child2 = spawn(x*2+1);
		if (child2 <= 0) return;
		outliveChild(child1);
		outliveChild(child2);
	} else {
		child1 = child2 = 0;
		kill(topPid, SIGINT);
		pause();
	}
}

// Fork until a tree depth of n is reached (each process can only fork once)
int spawn (int id)
{
	int pid;
	switch (pid = fork()) {
		case -1: // error
			perror("bad fork\n");
			exit(errno);
			break;
		case 0: // child
			x = id;
			spawn2();
			break;
		default: // parent
			return pid;
	}
}

// Increment leaf count. Start output if leaf limit is reached.
void handleInterrupt (int signum)
{
	printf("\t\tkil; %d %d %d %d\n", x, signum, leafct, leafLimit);
	if (getpid() == topPid) {
		++leafct;
		if (leafct == leafLimit) {
			swansong(signum);
		}
	}
}
