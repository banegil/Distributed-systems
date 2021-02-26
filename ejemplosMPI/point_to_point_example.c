#include "mpi.h"
#include <stdio.h>


int main(int argc, char **argv){

   int size, rank, receptor, emisor, tag;
   char input, output;
   MPI_Status Stat;

   // Init
   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   tag = 1;


   // Master process
   if (rank == 0) {

	  printf ("Introduce un caracter y pulsa Enter\n");
	  scanf("%c", &output);

	  receptor = 1;
	  emisor = size-1;

      printf("Proceso [%d] enviando a proceso [%d]\n",rank, receptor);
      MPI_Send(&output, 1, MPI_CHAR, receptor, tag, MPI_COMM_WORLD);

      printf("Proceso [%d] esperando mensaje del proceso [%d]\n",rank, emisor);
      MPI_Recv(&input, 1, MPI_CHAR, emisor, tag, MPI_COMM_WORLD, &Stat);

      printf("Proceso [%d] ya ha recibido el mensaje [%d] -> %c\n",rank, emisor, input);
   }

   // Worker process
   else if (rank > 0){

	  receptor = rank+1;

	  if(receptor == size){
		  receptor = 0;
	  }

      emisor = rank-1;

	  printf("Proceso [%d] esperando a proceso [%d]\n",rank, emisor);
      MPI_Recv(&input, 1, MPI_CHAR, emisor, tag, MPI_COMM_WORLD, &Stat);
      printf("Proceso [%d] ya ha recibido el mensaje [%d] -> %c\n",rank, emisor, input);

      printf("Proceso [%d] enviando a proceso [%d]\n",rank, receptor);
	  MPI_Send(&input, 1, MPI_CHAR, receptor, tag, MPI_COMM_WORLD);
   }

   MPI_Finalize();
}
