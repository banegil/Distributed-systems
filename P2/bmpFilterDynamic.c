#include "bmpBlackWhite.h"
#include "mpi.h"

/** Show log messages */
#define SHOW_LOG_MESSAGES 1

/** Enable output for filtering information */
#define DEBUG_FILTERING 0

/** Show information of input and output bitmap headers */
#define SHOW_BMP_HEADERS 0

#define END_OF_PROCESSING 0


int main(int argc, char** argv){

	tBitmapFileHeader imgFileHeaderInput;			/** BMP file header for input image */
	tBitmapInfoHeader imgInfoHeaderInput;			/** BMP info header for input image */
	tBitmapFileHeader imgFileHeaderOutput;			/** BMP file header for output image */
	tBitmapInfoHeader imgInfoHeaderOutput;			/** BMP info header for output image */
	char* sourceFileName;							/** Name of input image file */
	char* destinationFileName;						/** Name of output image file */
	int inputFile, outputFile;						/** File descriptors */
	unsigned char *outputBuffer;					/** Output buffer for filtered pixels */
	unsigned char *inputBuffer;						/** Input buffer to allocate original pixels */
	unsigned char *auxPtr;							/** Auxiliary pointer */
	unsigned int rowSize;							/** Number of pixels per row */
	unsigned int rowsPerProcess;					/** Number of rows to be processed (at most) by each worker */
	unsigned int rowsSentToWorker;					/** Number of rows to be sent to a worker process */
	unsigned int receivedRows;						/** Total number of received rows */
	unsigned int threshold;							/** Threshold */
	unsigned int currentRow;						/** Current row being processed */
	unsigned int processedRows;
	unsigned int currentPixel;						/** Current pixel being processed */
	unsigned int outputPixel;						/** Output pixel */
	unsigned int readBytes;							/** Number of bytes read from input file */
	unsigned int writeBytes;						/** Number of bytes written to output file */
	unsigned int totalBytes;						/** Total number of bytes to send/receive a message */
	unsigned int numPixels;							/** Number of neighbour pixels (including current pixel) */
	unsigned int currentWorker;						/** Current worker process */
	unsigned int *processIDs;
	tPixelVector vector;							/** Vector of neighbour pixels */
	int imageDimensions[2];							/** Dimensions of input image */
	double timeStart, timeEnd;						/** Time stamps to calculate the filtering time */
	int size, rank, tag;							/** Number of process, rank and tag */
	MPI_Status status;								/** Status information for received messages */
	unsigned int i, j;


		// Init
		MPI_Init(&argc, &argv);
		MPI_Comm_size(MPI_COMM_WORLD, &size);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		tag = 1;
		srand(time(NULL));

		// Check the number of processes
		if (size<2){

			if (rank == 0)
				printf ("This program must be launched with (at least) 2 processes\n");

			MPI_Finalize();
			exit(0);
		}

		// Check arguments
		if (argc != 5){

			if (rank == 0)
				printf ("Usage: ./bmpFilterDynamic sourceFile destinationFile threshold numRows\n");

			MPI_Finalize();
			exit(0);
		}

		// Get input arguments...
		sourceFileName = argv[1];
		destinationFileName = argv[2];
		threshold = atoi(argv[3]);
		rowsPerProcess = atoi(argv[4]);

		// Allocate memory for process IDs vector
		processIDs = (unsigned int *) malloc (size*sizeof(unsigned int));

		// Master process
		if (rank == 0){

			// Process starts
			timeStart = MPI_Wtime();

			// Read headers from input file
			readHeaders (sourceFileName, &imgFileHeaderInput, &imgInfoHeaderInput);
			readHeaders (sourceFileName, &imgFileHeaderOutput, &imgInfoHeaderOutput);

			// Write header to the output file
			writeHeaders (destinationFileName, &imgFileHeaderOutput, &imgInfoHeaderOutput);

			// Calculate row size for input and output images
			rowSize = (((imgInfoHeaderInput.biBitCount * imgInfoHeaderInput.biWidth) + 31) / 32 ) * 4;

			// Show info before processing
			if (SHOW_LOG_MESSAGES){
				printf ("[MASTER] Applying filter to image %s (%dx%d) with threshold %d. Generating image %s\n", sourceFileName,
						rowSize, imgInfoHeaderInput.biHeight, threshold, destinationFileName);
				printf ("Number of workers:%d -> Each worker calculates (at most) %d rows\n", size-1, rowsPerProcess);
			}

			// Show headers...
			if (SHOW_BMP_HEADERS){
				printf ("Source BMP headers:\n");
				printBitmapHeaders (&imgFileHeaderInput, &imgInfoHeaderInput);
				printf ("Destination BMP headers:\n");
				printBitmapHeaders (&imgFileHeaderOutput, &imgInfoHeaderOutput);
			}

			// Open source image
			if((inputFile = open(sourceFileName, O_RDONLY)) < 0){
				printf("ERROR: Source file cannot be opened: %s\n", sourceFileName);
				exit(1);
			}

			// Open target image
			if((outputFile = open(destinationFileName, O_WRONLY | O_APPEND, 0777)) < 0){
				printf("ERROR: Target file cannot be open to append data: %s\n", destinationFileName);
				exit(1);
			}

			// Allocate memory to copy the bytes between the header and the image data
			outputBuffer = (unsigned char*) malloc ((imgFileHeaderInput.bfOffBits-BIMAP_HEADERS_SIZE) * sizeof(unsigned char));

			// Copy bytes between headers and pixels
			lseek (inputFile, BIMAP_HEADERS_SIZE, SEEK_SET);
			read (inputFile, outputBuffer, imgFileHeaderInput.bfOffBits-BIMAP_HEADERS_SIZE);
			write (outputFile, outputBuffer, imgFileHeaderInput.bfOffBits-BIMAP_HEADERS_SIZE);

			//El (size - 1) es porque el Master no procesa, solo envia y recibe
			processedRows = currentRow = 0;

			// Allocate memory for input and output buffers
			inputBuffer = (unsigned char *) malloc (rowsPerProcess * rowSize);	
			outputBuffer = (unsigned char *) malloc (imgInfoHeaderInput.biSizeImage);	
			processIDs = (unsigned int *) malloc ((size - 1) * sizeof(unsigned int));			

			//Envio
			for(i = 1; i < size; i++){
				processIDs[i - 1] = currentRow;

				MPI_Send(&rowsPerProcess, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
				MPI_Send(&rowSize, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
					
				// Read current row data to input buffer
				if ((readBytes = read (inputFile, inputBuffer, rowSize * rowsPerProcess)) != rowSize * rowsPerProcess)
					showError ("Error while reading from source file");
					
				MPI_Send(inputBuffer, rowSize * rowsPerProcess, MPI_CHAR, i, tag, MPI_COMM_WORLD);
				currentRow += rowsPerProcess;
			}

			//Recibo
			rowsSentToWorker = rowsPerProcess;
			while(processedRows < imgInfoHeaderInput.biSizeImage / rowSize){
				MPI_Recv (&rowsPerProcess, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
				auxPtr = outputBuffer + processIDs[status.MPI_SOURCE - 1] * rowSize;

				MPI_Recv (auxPtr, rowSize * rowsPerProcess, MPI_CHAR, status.MPI_SOURCE, tag, MPI_COMM_WORLD, &status);

				//Envio
				if(currentRow < imgInfoHeaderInput.biSizeImage / rowSize){
					processIDs[status.MPI_SOURCE - 1] = currentRow;
			
					if(currentRow + rowsPerProcess > (imgInfoHeaderInput.biSizeImage / rowSize))
						rowsSentToWorker = (imgInfoHeaderInput.biSizeImage / rowSize) - currentRow;

					MPI_Send(&rowsSentToWorker, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
					MPI_Send(&rowSize, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
					
					// Read current row data to input buffer
					if ((readBytes = read (inputFile, inputBuffer, rowSize * rowsSentToWorker)) != rowSize * rowsSentToWorker)
						showError ("Error while reading from source file");
					
					MPI_Send(inputBuffer, rowSize * rowsSentToWorker, MPI_CHAR, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
					currentRow += rowsSentToWorker;
				}

				processedRows += rowsPerProcess;
			}

			rowsPerProcess = END_OF_PROCESSING;
			for(i = 1; i < size; i++) MPI_Send(&rowsPerProcess, 1, MPI_INT, i, tag, MPI_COMM_WORLD);

			// Write to output file
			if ((writeBytes = write (outputFile, outputBuffer, imgInfoHeaderInput.biSizeImage)) != imgInfoHeaderInput.biSizeImage)
					showError ("Error while writing to destination file");
			
			// Close files
			close (inputFile);
			close (outputFile);

			// Process ends
			timeEnd = MPI_Wtime();

			// Show processing time
			printf("Filtering time: %f\n",timeEnd-timeStart);
		}


		// Worker process
		else{
			do{

				MPI_Recv (&rowsPerProcess, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

				if(rowsPerProcess > 0){
					MPI_Recv (&rowSize, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
			
					// Allocate memory for input and output buffers
					inputBuffer = (unsigned char *) malloc (rowSize * rowsPerProcess);
					outputBuffer = (unsigned char*) malloc (rowSize * rowsPerProcess);
			
					MPI_Recv (inputBuffer, rowSize * rowsPerProcess, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);

					// For each pixel in the current rows...
					for (currentPixel = 0; currentPixel < rowSize * rowsPerProcess; currentPixel++){
						// Current pixel
						numPixels = 0;
						vector[numPixels] = inputBuffer[currentPixel];
						numPixels++;

						// Not the first pixel
						if (currentPixel > 0){
							vector[numPixels] = inputBuffer[currentPixel-1];
							numPixels++;
						}

				 		// Not the last pixel
						if (currentPixel < (imgInfoHeaderInput.biWidth-1)){
							vector[numPixels] = inputBuffer[currentPixel+1];
							numPixels++;
						}
						// Store current pixel value
						outputBuffer[currentPixel] = calculatePixelValue(vector, numPixels, threshold, 0);
					}
			
					MPI_Send (&rowsPerProcess, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
					MPI_Send (outputBuffer, rowSize * rowsPerProcess, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
				}

			} while(rowsPerProcess > 0);
		}

		// Finish MPI environment
		MPI_Finalize();


}
