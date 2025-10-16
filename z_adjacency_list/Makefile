#
#
#
FLAGS = -std=c99 -Wall -pedantic-errors

all:
	gcc graph.c $(FLAGS) -o graph
	./graph

debug:
	# Test with valgrind
	gcc graph.c $(FLAGS) -g -O0 -o graph
	valgrind --leak-check=yes ./graph


clean:
	rm -f graph
