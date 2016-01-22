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
struct sigaction alarmAction;
struct sigaction parentAlarmAction;
int count = 0;

void display (int);
void fatal (char * str);

void main ()
{
	int i;
	parentAlarmAction.sa_handler = SIG_IGN;
	alarmAction.sa_handler = display;
	sigaction(SIGALRM, &alarmAction, NULL);
	switch (fork()) {
		case -1:
			fatal("failed fork");
			break;
		case 0: // child 1
			child1id = getpid();
			while (1)
				count ++;
			break;
		default: // parent
			switch (fork()) {
				case -1:
					fatal("failed fork");
					break;
				case 0: // child 2
					child2id = getpid();
					while (1)
						count ++;
					break;
				default: // parent
					sigaction(SIGALRM, &parentAlarmAction, NULL);
					// iterate 5 times
					for (i = 0; i < 5; i++) {
						sleep(1);
						kill(child1id, SIGALRM);
						kill(child2id, SIGALRM);
					}
					kill(child1id, SIGSTOP);
					kill(child2id, SIGSTOP);
					break;
			}
			break;
	}
}

void display (int sig)
{
	printf("Count is %9d, my pid is %d\n", count, getpid());
	count = 0;
}

void fatal (char * str)
{
	perror(str);
	exit(errno);
}
