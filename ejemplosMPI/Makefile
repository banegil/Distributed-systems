OPTIONS= -Wall

all: bcast_example hello_example point_to_point_example scatter_example reduce_example

clean:
	rm -f bcast_example hello_example point_to_point_example scatter_example reduce_example
	rm -f *.o
	
	
bcast_example:
	mpicc -o bcast_example bcast_example.c
	
hello_example:
	mpicc -o hello_example hello_example.c
	
point_to_point_example:
	mpicc -o point_to_point_example point_to_point_example.c
	
scatter_example:
	mpicc -o scatter_example scatter_example.c
	
reduce_example:
	mpicc -o reduce_example reduce_example.c