#include "mpi.h"
#include <stdio.h>
#define BUFFER_SIZE 3

int main(int argc, char **argv){

   // Total procesos y rank
   int size, rank;

   // Número de elementos para enviar/recibir y emisor
   int sendcount, recvcount, source;

   // Buffer de envío
   float sendbuf[BUFFER_SIZE][BUFFER_SIZE] = {
      {6.5, 1.6, 9.4},
      {4.6, 3.5, 3.555},
      {23.6, 42.3, 235545.5}};

   // Buffer de recepción
   float recvbuf[BUFFER_SIZE];

   // Init...
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);

   // Comprobar el número de procesos
   if (size == BUFFER_SIZE) {
      source = 1;
      sendcount = BUFFER_SIZE;
      recvcount = BUFFER_SIZE;

      // Repartimos el mensaje entre los procesos...
      MPI_Scatter(sendbuf,
    		  	  sendcount,
    		  	  MPI_FLOAT,
    		  	  recvbuf,
    		  	  recvcount,
    		  	  MPI_FLOAT,
    		  	  source,
    		  	  MPI_COMM_WORLD);

      printf("rank= %d  Results: %f %f %f\n",
    		  	  rank,
    		  	  recvbuf[0],
    		  	  recvbuf[1],
    		  	  recvbuf[2]);

      }

   else
      printf("Esta prueba funciona para %d procesos.\n",BUFFER_SIZE);

   MPI_Finalize();
}
