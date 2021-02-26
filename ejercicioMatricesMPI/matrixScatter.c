#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/** Matrix size*/
#define SIZE 1000

/** Print matrices */
#define PRINT_MATRICES 0

/** Maximum number value for generating each matrix */
#define MAX_INT_NUMBER 10

/** Master process */
#define MASTER 0

/** Matrix type */
typedef int* tMatrix;


/**
 * Generate a matrix with random int values
 */
void generateMatrix(tMatrix matrix){

  int i;
  
	for (i=0; i<(SIZE*SIZE); i++)
  		matrix[i] = (rand() % MAX_INT_NUMBER);
}


/**
 * Print a matrix
 */
void printMatrix(tMatrix matrix){

  	int i, j;

		// Init...
		i = j = 0;
	
		for (i=0; i<SIZE; i++){
			printf("\n\t| ");
			for (j=0; j<SIZE; j++)
		  		printf("%3d ", matrix[(i*SIZE)+j]);
			printf("|");
  		}
}


int main(int argc, char *argv[]){

	tMatrix matrixA;			/** First matrix */
	tMatrix matrixB;			/** Second matrix */
	tMatrix matrixC;			/** Resulting matrix */

  	int myrank, numProc;		/** Rank and number of processes */
	int tag;					/** Tag */
	MPI_Status status;			/** MPI status for receiving messages */
 	int initRow, endRow;		/** Portion of the matrix to be processed by a process */
	int i, j, k;				/** Aux variables */	
	double timeStart, timeEnd;	/** Time stamps to calculate the filtering time */
  	 

		// Init...
		tag = 1;
		srand(time(NULL));

		// Init MPI
	  	MPI_Init (&argc, &argv);
	  	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);	
	  	MPI_Comm_size(MPI_COMM_WORLD, &numProc);


	  	// Check whether SIZE is divisible by numProc
		if ((SIZE % numProc) != 0){

			if (myrank == MASTER) 
				printf("Matrix size (%d) cannot be divided by %d\n", SIZE, numProc);
    		
			MPI_Finalize();
    		exit(-1);
  		}
		
		// Divide data
  		initRow = myrank * SIZE/numProc;
  		endRow = (myrank+1) * SIZE/numProc;

		// Master process generates input matrices
		if (myrank == MASTER) {

			// Generate matrix A			
			printf ("Generating matrix A (%dx%d)\n", SIZE, SIZE);
			matrixA = (tMatrix) malloc (SIZE*SIZE*sizeof(int));
			generateMatrix(matrixA);

			if (PRINT_MATRICES){
				printf ("Matrix A:\n");
				printMatrix (matrixA);
				printf ("\n\n");
			}

			// Generate matrix B
			printf ("Generating matrix B (%dx%d)\n", SIZE, SIZE);
			matrixB = (tMatrix) malloc (SIZE*SIZE*sizeof(int));
			generateMatrix(matrixB);
			
			if (PRINT_MATRICES){			
				printf ("Matrix B:\n");
				printMatrix (matrixB);
				printf ("\n\n");
			}

			// Allocate memory for matrixC
			matrixC = (tMatrix) malloc (SIZE*SIZE*sizeof(int));

			// Process starts
			timeStart = MPI_Wtime();
		}	

		// Allocate memory for the workers
		else{
			matrixA = (tMatrix) malloc ((endRow-initRow+1)*SIZE*sizeof(int));
			matrixB = (tMatrix) malloc (SIZE*SIZE*sizeof(int));
			matrixC =  (tMatrix) malloc ((endRow-initRow+1)*SIZE*sizeof(int));
		}

		// Broadcast matrix B
  		MPI_Bcast (matrixB, SIZE*SIZE, MPI_INT, MASTER, MPI_COMM_WORLD);

		// Distribute matrix A among the processes (including MASTER)
  		MPI_Scatter (matrixA, (SIZE*SIZE)/numProc, MPI_INT, matrixA, (SIZE*SIZE)/numProc, MPI_INT, MASTER, MPI_COMM_WORLD);


		// Data arrives to each process
		printf("[Process %d] Processing row %d to %d\n", myrank, initRow, endRow-1);

		// Perform the multiplication
		
	  	for (i=0; i<(endRow-initRow); i++)
			for (j=0; j<SIZE; j++){
		  		matrixC[(i*SIZE)+j]=0;
		  		for (k=0; k<SIZE; k++)
					matrixC[(i*SIZE)+j] += matrixA[(i*SIZE)+k]*matrixB[(k*SIZE)+j];
		}

		// Gather the result
  		MPI_Gather (matrixC, (SIZE*SIZE)/numProc, MPI_INT, matrixC, (SIZE*SIZE)/numProc, MPI_INT, MASTER, MPI_COMM_WORLD);

		// Show results
 		if (myrank == MASTER){
	
			// Process ends
			timeEnd = MPI_Wtime();		

			if (PRINT_MATRICES){
				printf ("Matrix C:\n");
				printMatrix (matrixC);
				printf ("\n\n");
			}

			// Show processing time
			printf("Processing time: %f\n",timeEnd-timeStart);
  		}

		// End MPI nvironment
  		MPI_Finalize();

  return 0;
}



