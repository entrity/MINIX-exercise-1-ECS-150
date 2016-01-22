/*
A program pipe_test (an exercise for system calls) which creates three processes, P1, P2, P3 and pipes
as needed.

The first process
	- displays its pid,
	- prompts the user for a string,
	- and then sends it to the second process.
The second process
	- displays its pid,
	- displays the string it received,
	- reverses the string,
	- displays the reversed string,
	- and sends it to the third process.
The third process
	- displays its pid,
	- displays the string it received,
	- converts the string to uppercase,
	- displays the string the string it generated,
	- and sends it back to the first process.
When the first process gets the processed string, it
	- displays its pid
	- and displays the string it received it to the terminal.
When all this is done,
	- all processes terminate but display
		- their pid
		- and a message announcing their imminent death.
*/

#include <stdlib.h> // for the exit call
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#define BUFF_SIZE 256
#define WRITE_PIPE_ERR 131
#define READ_PIPE_ERR 132
#define WRITE_STD_ERR 133
#define READ_STD_ERR 134
#define PIPE_MAKE_ERR 135
#define NEWLINE display("\n", 1)

char buffer1[BUFF_SIZE];
char buffer2[BUFF_SIZE];
char buffer3[BUFF_SIZE];
struct sigaction interruptAction;
char p;

void fatal (char * msg, int errcode);
int getUserInput ();
int intToChars (char buff[], int val, int bufflen);
void testIntToChars (int value);
void printPid ();
void interruptHandler(int n);
inline void toUpcase (char buff[], int n);
inline void display (char buff[], int n);
inline void reverse (char buff[], int n);

void interruptHandler(int n)
{
	wait();
	printPid();
	display("I am about to die", 17);
	NEWLINE;
}

int main ()
{
	int pfdsA[2]; // pipe between p1 and p2
	int pfdsB[2]; // pipe between p2 and p3
	int pfdsC[2]; // pipe between p1 and p3
	pid_t pid;
	int nreadA, nreadB, nreadC;

	// config for SIGINT
	interruptAction.sa_handler = interruptHandler;
	sigaction(SIGINT, &interruptAction, NULL);

	if ((pipe(pfdsA)) == -1)
		fatal("Unable to create pipe A", PIPE_MAKE_ERR);
	if ((pipe(pfdsC)) == -1)
		fatal("Unable to create pipe A", PIPE_MAKE_ERR);

	p = '1';

	switch (pid = fork()) { // fork p1 and p2
		case -1: // error
			fatal("Unable to fork A", 2);
			break;
		case 0: // child : p2
			p = '2';
			close(pfdsC[0]); // close read pipe C
			close(pfdsA[1]); // close write end of pipe A
			if (pipe(pfdsB) == -1)
				fatal("Unable to create pipe B", PIPE_MAKE_ERR);
			switch (pid = fork()) { // fork p2 and p3
				case -1: // error
					fatal("Unable to fork B", 129);
					break;
				case 0: // child : p3
					p = '3';
					sigaction(SIGINT, &interruptAction, NULL);
					close(pfdsB[1]); // close write end of pipe B
					if ((nreadC = read(pfdsB[0], buffer3, BUFF_SIZE)) == -1) // read from pipe, blocking until parent supplies data
						fatal("Unable to read pipe", READ_PIPE_ERR);
					printPid(); // - displays its pid,
					display("I just received the string ", 27);
					display(buffer3, nreadC); // - displays the string it received,
					NEWLINE;
					toUpcase(buffer3, nreadC); // - converts the string to uppercase,
					display("I am sending ", 13);
					display(buffer3, nreadC); // - displays the string the string it generated,
					display(" to P1", 6);
					NEWLINE;
					if (write(pfdsC[1], buffer3, nreadC) == -1) // - and sends it back to the first process.
						fatal("Unable to write pipe c", WRITE_PIPE_ERR);
					pause();
					break;
				default: // parent : p2
					close(pfdsC[1]); // close write end of pipe C
					close(pfdsB[0]); // close read end of pipe B
					if ((nreadB = read(pfdsA[0], buffer2, BUFF_SIZE)) == -1) // read from pipe, blocking until parent supplies data
						fatal("Unable to read pipe", READ_PIPE_ERR);
					printPid(); // - displays its pid,
					display("I just received the string ", 27);
					display(buffer2, nreadB); // - displays the string,
					NEWLINE;
					reverse(buffer2, nreadB); // - reverses the string,
					display("I am sending ", 13);
					display(buffer2, nreadB); // - displays the reversed string,
					display(" to P3", 6);
					NEWLINE;
					if (write(pfdsB[1], buffer2, nreadB) == -1) // - and sends it to the third process.
						fatal("Unable to write pipe b", WRITE_PIPE_ERR);
					pause();
					break;
			}
			break;
		default: // parent : p1
			close(pfdsC[1]);
			printPid(); // - displays its pid,
			display("Please enter a string: ", 24);
			if ((nreadA = read(0, buffer1, BUFF_SIZE)) == -1)
				fatal("Unable to read stdin", READ_STD_ERR);
			if (write(pfdsA[1], buffer1, nreadA-1) == -1) // - and then sends it to the second process.
				fatal("Unable to write pipe a", WRITE_PIPE_ERR);
			if ((nreadA = read(pfdsC[0], buffer1, BUFF_SIZE)) == -1) // read from pipe, blocking until p3 supplies data
				fatal("Unable to read pipe", READ_PIPE_ERR);
			// Display pid
			printPid();
			// Display string
			display("I just received the string ", 27);
			display(buffer1, nreadA);
			NEWLINE;
			// Signal death of children
			kill(0, SIGINT);
			break;
	}



}

inline void toUpcase (char buff[], int n)
{
	int i;
	for (i = 0; i < n; i++)
		if (buff[i] >= 0x61)
			buff[i] -= 0x20;
}

inline void display (char buff[], int n)
{
	if (write(1, buff, n) == -1) // - displays the string it received,
		fatal("Unable to write to stdout", WRITE_STD_ERR);
}

inline void reverse (char buff[], int n)
{
	int j = 0;
	char swap;
	for (j = 0; j < n/2; j++) {
		swap = buff[n-1-j];
		buff[n-1-j] = buff[j] ;
		buff[j] = swap;
	}
}

int intToChars (char buff[], int val, int bufflen)
{
	int i = 0;
	while (val > 0 && i < bufflen) {
		buff[i++] = (char) (val % 10) + 0x30;
		val /= 10;
	}
	if (val > 0)
		fatal("integer exceeds buffer size", 126);
	reverse(buff, i);
	return i;
}

void printPid ()
{
	int n;
	char decimal[6];
	NEWLINE;
	display("I am process P", 14);
	write(1, &p, 1);
	NEWLINE;
	display("My pid is ", 10);
	n = intToChars(decimal, getpid(), 6);
	display(decimal, n);
	NEWLINE;
}

void fatal (char * msg, int errcode)
{
	perror(msg);
	exit(errno);	
}

void testIntToChars (int value) {
	char buff[6];
	int i = intToChars(buff, value, 6);
	int x = i + 0x30;
	write(1, &x, 1);
	write(1, "   \n", 5);
	write(1, buff, i);
	write(1, "---\n", 5);
}
