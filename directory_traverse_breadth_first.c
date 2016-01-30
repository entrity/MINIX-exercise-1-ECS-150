#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

int depth = 0;
int isDepthReached = 0;
int matchesN = 0;
struct stat status;
struct node_t {
	char * path;
	struct node_t * next;
};
struct node_t * queue;
struct node_t * queueEnd;
struct node_t * matches;
struct node_t * matchesEnd;

void append (char * path, struct node_t ** start_p, struct node_t ** end_p)
{
	struct node_t * node = (struct node_t *) malloc(sizeof(struct node_t));
	// make a new copy of the path so that the original can be freed. why? b/c this eliminates the need to record whether 0, 1, or 2 different node_t instances have a pointer to the string
	node->path = malloc(strlen(path)+1);
	strcpy(node->path, path);
	node->next = NULL;
	if (*end_p)
		(*end_p)->next = node;
	else
		*start_p = node;
	*end_p = node;
}

void enqueue (char * path)
{
	append(path, &queue, &queueEnd);
}

void addMatch (char * path)
{
	append(path, &matches, &matchesEnd);
	matchesN++;
}

int isDir (char * path)
{
	if (stat(path, &status)) {
		perror("Error in stat-ing entity");
		exit(errno);
	}
	return S_ISDIR(status.st_mode);
}

int main (int argc, char * argv[])
{
	DIR * dp;
	struct dirent * dip;
	char * fullpath = NULL;
	// check format or params
	if (argc < 3) {
		fprintf(stderr, "Usage %s <dirpath> <filename>\n", argv[0]);
		exit(E2BIG);
	}
	// check existence of path
	if (access(argv[1], F_OK)) {
		perror("path does not exist");
		exit(ENOENT);
	}
	if (access(argv[1], R_OK)) {
		perror("no read permissions on given filepath");
		exit(EACCES);
	}
	if (!isDir(argv[1])) {
		perror("path is not a directory");
		exit(ENOENT);
	}

	fullpath = malloc(strlen(argv[1] + 1));
	strcpy(fullpath, argv[1]);
	if (fullpath[strlen(fullpath)-1] == '/')
		fullpath[strlen(fullpath)-1] = '\0';
	enqueue(fullpath);
	printf("%s\n", queue->path);

	// loop
	while (queue) {
		if (!(dp = opendir(queue->path))) {
			perror("unable to open dir at top of loop");
			fprintf(stderr, "%s\n", queue->path);
			exit(errno);
		}
		// iterate items in current directory
		while(dip = readdir(dp)) {
			if (0 == strcmp(".", dip->d_name) || 0 == strcmp("..", dip->d_name) )
				continue;
			// formulate fullpath of dirname and basename
			fullpath = malloc(strlen(queue->path) + strlen(dip->d_name) + 2);
			sprintf(fullpath, "%s/%s", queue->path, dip->d_name);
			printf("%s\n", fullpath);
			// if match, save match
			if (0 == strcmp(dip->d_name, argv[2]))
				addMatch(fullpath);
			// if no match but isdir, enqueue
			if (0 == access(fullpath, F_OK) && isDir(fullpath))
				enqueue(fullpath);
			// cleanup
			free(fullpath);
		}
		// cleanup; advance queue
		closedir(dp);
		struct node_t * prev = queue;
		queue = queue->next;
		if (prev) {
			free(prev->path);
			free(prev);
		}
	}

	// output results
	printf(" == %d matches found ==\n", matchesN);
	while (matches) {
		printf("%s\n", matches->path);
		struct node_t * prev = matches;
		matches = matches->next;
		if (prev) {
			free(prev->path);
			free(prev);
		}
	}

	return 0;
}