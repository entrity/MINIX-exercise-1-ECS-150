directory_traverse: directory_traverse_breadth_first.c
	gcc directory_traverse_breadth_first.c && ./a.out "/home/markham/Documents/Professional" foo

target process_tree: process_tree.c
	gcc process_tree.c -lm && ./a.out 3

