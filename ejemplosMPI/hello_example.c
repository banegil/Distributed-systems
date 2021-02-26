#include <stdio.h>
#include "mpi.h"

main(int argc, char **argv){

	int rank,size;

	// Init
	MPI_Init(&argc,&argv); 
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Say Hellor!!!
	printf("Hello World! Process %d of %d processes\n", rank, size);
	MPI_Finalize();
}
