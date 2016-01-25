#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

int depth = 0;
int isDepthReached = 0;
struct stat status;
struct node_t {
	char * path;
	struct node_t * next;
};
struct node_t * queue;

void enqueue (char * path) {
	struct node_t * node = (struct node_t *) malloc(sizeof(struct node_t));
	node->path = path;
	if (queue)
		queue->next = node;
	else
		queue = node;
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
		printf("Usage %s <dirpath> <filename>", argv[0]);
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
	enqueue(argv[1]);
	// loop
	while (queue) {
		printf("%s\n", queue->path);
		if (!(dp = opendir(queue->path))) {
			perror("unable to open dir");
			exit(errno);
		}
		// iterate items in current directory
		while(dip = readdir(dp)) {
			// formulate fullpath of dirname and basename
			if (fullpath)
				free(fullpath);
			sprintf(fullpath, "%s/%s", queue->path, dip->d_name);
			printf("%s\n", fullpath);
			// if match, done
			if (0 == strcmp(dip->d_name, argv[2])) {
				printf("\t...Match found!!!");
				exit(0);
			}
			// if no match but isdir, enqueue
			else if (access(fullpath, F_OK) && isDir(fullpath))
				enqueue(fullpath);
		}
		// close directory pointer
		closedir(dp);
		// advance queue
		queue = queue->next;
		free(queue);
	}
	return 0;
}