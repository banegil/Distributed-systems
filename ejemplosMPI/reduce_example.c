#include <stdio.h>
#include "mpi.h"

main(int argc, char **argv){

	int rank,size;
	int num, resultado;

	// Init
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Número asociado a cada proceso
	num = (rank+10)*2;
	printf ("Proceso [%d] con número: %d\n", rank, num);

	// Aplicamos la operación de reducción
	MPI_Reduce(&num, &resultado, 1, MPI_INT, MPI_PROD, 0, MPI_COMM_WORLD);

	// Master process?
	if (rank==0)
		printf("Proceso [%d] tiene el resultado: %d\n",	rank, resultado);


	MPI_Finalize();
}
