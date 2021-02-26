#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/** Matrix size*/
#define SIZE 1000

/** Number of rows to be sent to each worker */
#define NROWS 10

/** Print matrices */
#define PRINT_MATRICES 0

/** Maximum number value for generating each matrix */
#define MAX_INT_NUMBER 10

/** Master process */
#define MASTER 0

/** End of processing */
#define END_OF_PROCESSING 0

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
	tMatrix auxPtrA, auxPtrC;	/** Index pointers */
	int* indexTable;			/** Index table */

  	int myrank, numProc;		/** Rank and number of processes */
	int tag;					/** Tag */
	MPI_Status status;			/** MPI status for receiving messages */
	int sentRows;				/** Number of rows sent */
	int currentRow;				/** Current row being processed */
	int processedRows;			/** Number of currently processed rows */
	int i, j, k;				/** Aux variables */	
	double timeStart, timeEnd;	/** Time stamps to calculate the filtering time */
  	 

		// Init...
		tag = 1;
		srand(time(NULL));

		// Init MPI
	  	MPI_Init (&argc, &argv);
	  	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);	
	  	MPI_Comm_size(MPI_COMM_WORLD, &numProc);


		// Check whether at least each worker receives NROWS
		if ((NROWS*(numProc-1)) > SIZE){

			if (myrank == MASTER) 
				printf("Wrong configuration for NROWS (%d). SIZE (%d) < numWorkers(%d)*NROWS(%d)\n", NROWS, SIZE, numProc-1, NROWS);
				printf ("At least, each worker must receive %d rows to be processed\n", NROWS);
    		
			MPI_Finalize();
    		exit(-1);
  		}


		// Master process generates input matrices
		if (myrank == MASTER){

			// Index table (alloc and init)
			indexTable = (int*) malloc (numProc * sizeof (int));
			for (i=0; i<numProc; i++)
				indexTable[i] = 0;

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
			matrixA = (tMatrix) malloc (NROWS*SIZE*sizeof(int));
			matrixB = (tMatrix) malloc (SIZE*SIZE*sizeof(int));
			matrixC =  (tMatrix) malloc (NROWS*SIZE*sizeof(int));
		}

		// Broadcast matrix B
  		MPI_Bcast (matrixB, SIZE*SIZE, MPI_INT, MASTER, MPI_COMM_WORLD);



		// Master process
		if (myrank == MASTER){

			// Init...
			sentRows = 0;
			currentRow = 0;
			processedRows = 0;
			auxPtrA = matrixA;
			auxPtrC = matrixC;


			// Distribute rows...
			for (i=1; i<numProc; i++){

				// Send the number of rows to be processed
				sentRows = NROWS;
				MPI_Send (&sentRows, 1, MPI_INT, i, tag, MPI_COMM_WORLD);

				// Send the rows data
				indexTable[i] = currentRow;
				MPI_Send (auxPtrA, NROWS*SIZE, MPI_INT, i, tag, MPI_COMM_WORLD);

				// Update pointer and index
				currentRow += NROWS;		
				auxPtrA += (NROWS*SIZE);
			}


			// While there are remaining rows...
			while (processedRows < SIZE){
			
				// Receive number of rows
				MPI_Recv (&sentRows, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
				
				// Set destination buffer
				auxPtrC = matrixC + (indexTable[status.MPI_SOURCE]*SIZE);

				// Receive result data
				MPI_Recv (auxPtrC, sentRows*SIZE, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD, &status);

				// Update processed rows
				processedRows += sentRows;

				// Send remaining rows...
				if (currentRow < SIZE){

					// Send the number of rows to be processed
					if ((currentRow+NROWS) > SIZE)
						sentRows = SIZE - currentRow;
					else
						sentRows = NROWS;

					MPI_Send (&sentRows, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);

					// Send the rows data
					indexTable[status.MPI_SOURCE] = currentRow;
					MPI_Send (auxPtrA, sentRows*SIZE, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);

					// Update pointer and index
					currentRow += sentRows;		
					auxPtrA += (sentRows*SIZE);
				}
				else{
					sentRows = END_OF_PROCESSING;
					MPI_Send (&sentRows, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
				}
			}

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

		// Worker process
		else{	

			do{

				// Receive number of rows
				MPI_Recv (&sentRows, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);

				// Data arrives to each process
				printf("[Process %d] Processing %d rows\n", myrank, sentRows);

				if (sentRows>0){

					// Receive result data
					MPI_Recv (matrixA, sentRows*SIZE, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);			

					// Perform the multiplication		
				  	for (i=0; i<sentRows; i++)
						for (j=0; j<SIZE; j++){
					  		matrixC[(i*SIZE)+j]=0;
					  		for (k=0; k<SIZE; k++)
								matrixC[(i*SIZE)+j] += matrixA[(i*SIZE)+k]*matrixB[(k*SIZE)+j];
					}

					// Send results
					MPI_Send (&sentRows, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
					MPI_Send (matrixC, sentRows*SIZE, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
				}

			}while (sentRows>0);
		}
		
		// End MPI nvironment
  		MPI_Finalize();

  return 0;
}



