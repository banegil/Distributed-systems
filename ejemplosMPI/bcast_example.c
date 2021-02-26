#include <stdio.h>
#include "mpi.h"

int main( int argc, char **argv){

	int rank, valor;    

	// Inicializamos MPI y obtenemos el ID de cada proceso    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    
   
    // Proceso Master?
	if (rank == 0){
		printf ("Introduce un valor entero y pulsa Enter\n"); 
		scanf( "%d", &valor);
   	}
	
	// Broadcast del mensaje
	MPI_Bcast(&valor, 1, MPI_INT, 0, MPI_COMM_WORLD);
	printf( "Proceso %d recibe valor [%d]\n", rank, valor);
	

    MPI_Finalize( );
    return 0;
}
