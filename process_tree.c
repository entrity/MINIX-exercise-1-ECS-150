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
int leafct = 0; // keeps count of how many leaves exist currently
int leafLimit; // how many leaves will exist for full tree

void spawn ();
void handleInterrupt (int cpid);

int main (int argc, char * argv[])
{
	topPid = getpid();
	// Enforce usage
	if (argc < 2) {
		printf("Usage %s <integer>\n\tYou supplied only %d args\n", argv[0], argc);
		return E2BIG;
	}
	// Get user input
	sscanf(argv[1], "%d", &n);
	leafLimit = pow(2, n);
	// Start tree of processes
	if (--n > 0) {
		spawn();
		spawn();
		pause();
	}
	// Success
	return 0;
}

// Fork until a tree depth of n is reached (each process can only fork once)
void spawn ()
{
	if (n > 0) { // this is a nonterminal node
		switch (fork()) {
			case -1: // error
				exit(errno);
				break;
			case 0: // child
				printf("spawend  %d %d\n", getpid(), n);
				n--;
				spawn();
				spawn();
				pause();
				break;
			default: // parent
				break;
		}
	} else {
		printf("terminal %d %d\n", getpid(), n);
		// this is a terminal node
		kill(SIGINT, topPid);
	}
}

// Increment leaf count. Start output if leaf limit is reached.
void handleInterrupt (int cpid)
{
	if (++leafct == leafLimit)
		printf("leafct %d\n", leafct		);
	else
		pause();
}