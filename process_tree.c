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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SWANSONG_LEN 256

int n;
int waitStatus;
pid_t topPid;
int x = 1; // process identifier; flipped to zero when it's okay to die
int child1, child2;
char str[SWANSONG_LEN];

int spawnOne (int);

void swansong ()
{
	int n;
	n = snprintf(&str[0], SWANSONG_LEN, "I am process %d; my process pid is %d\nMy parent's process pid is %d\n\n", x, getpid(), getppid());
	if (n == -1) {
		perror("string truncated in swansong. (shoud not actually be possible with sufficient SWANSONG_LEN)");
		exit(99);
	} else
		write(1, &str, n);
}

int runProc () {
	if (--n > 0) { // non-leaf node
		if ((child1 = spawnOne(2*x)) == -1)
			exit(errno);
		if (child1 && (child2 = spawnOne(2*x+1)) == -1) // parent of child 1
			exit(errno);
		if (child1 && child2) { // parent of child1 and child2
			while (wait(&waitStatus) != -1) // don't continue until all children are dead
				;
			swansong();
		}
	} else { // leaf node
		swansong();
	}
}

int spawnOne (int newX)
{
	int pid;
	switch (pid = fork()) {
		case -1: // error
			perror("bad fork\n");
			exit(errno);
			break;
		case 0: // child
			x = newX;
			runProc();
			break;
		default: // parent
			break;
	}
	return pid;
}

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
	// Start tree of processes
	runProc(0);
	// Success
	return 0;
}
