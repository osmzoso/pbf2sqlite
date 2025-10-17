#
#
#
FLAGS = -std=c99 -Wall -pedantic-errors

all:
	gcc graph.c $(FLAGS) -O2 -o graph

debug:
	gcc graph.c $(FLAGS) -g -O0 -o graph
	# Test with valgrind
	valgrind --leak-check=yes ./graph demo
	echo
	valgrind --leak-check=yes ./graph rand 1000000 5000000
	echo


clean:
	rm -f graph
