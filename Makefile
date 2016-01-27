CC=gcc

1: pipe_test.c
	$(CC) pipe_test.c

2: AlrmTest.c
	$(CC) AlrmTest.c

3: directory_traverse_breadth_first.c
	$(CC) directory_traverse_breadth_first.c

4: process_tree.c
	$(CC) process_tree.c

5: smallsh.c smallsh.h
	$(CC) smallsh.c

3run: 3
	 ./a.out "/home/markham/Documents/Professional" foo

4run: 4
	./a.out 3
