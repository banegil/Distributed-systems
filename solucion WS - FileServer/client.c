#include "soapH.h"
#include "wsFileServer.nsmap"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define DEBUG 1

void showError (char* msg){

	printf ("[Client] %s\n", msg);
	exit (1);
}

/**
 * Copy a file from server side to client side
 */
void receiveFile (char *host, char* src, char* dst){

	int fd, fileSize, remainingBytes, offset, size;
	int result;
	wsFilens__tFileName fileName;
	wsFilens__tData data;
	struct soap soap;	
	
		// Init soap environment
		soap_init(&soap);

		if (DEBUG)
			printf ("invoking receiveFile (%s, %s, %s)\n", host, src, dst);

		// Create file
		fd = open(dst, O_WRONLY | O_TRUNC | O_CREAT, 0777);

		// Error?
		if (fd < 0){
			showError ("[receiveFile] Error while creating destination file!");
		}
		else{

			if (DEBUG)
				printf ("[receiveFile] File created:%s\n", dst);

			// Get file size from server
			fileName.__ptr = (char *) malloc (strlen(src) + 1);
			fileName.__size = strlen(src) + 1;
			memset (fileName.__ptr, 0, strlen(src) + 1);
			strcpy (fileName.__ptr, src);		
			soap_call_wsFilens__getFileSize (&soap, host, "", fileName, &fileSize);
			
			if (DEBUG)
					printf ("[receiveFile] getFileSize(%s) = %d bytes\n", fileName.__ptr, fileSize);

			if (fileSize < 0){
				showError ("Error while obtaining the size of source file!");
			}

			// All OK! Start transferring the file...
			else{
			

				remainingBytes = fileSize;
				offset = 0;

				// While there are remaining bytes to read
				while (remainingBytes > 0){

					// Set parameters
					size = remainingBytes >= MAX_DATA_SIZE ? MAX_DATA_SIZE : remainingBytes;

					// Invoke remote-read
					soap_call_wsFilens__readFromFile(&soap, host, "", fileName, offset, size, &data);									

					// Write into local file
					write (fd, data.__ptr, data.__size);

					// Update offset
					offset += data.__size;

					// Update remaining bytes
					remainingBytes -= data.__size;
				}

				close (fd);
			}
		}
		
}


/**
 * Copy a file from client side to server side
 */
void sendFile (char *host, char* src, char* dst){

	int fd, fileSize, readBytes, remainingBytes, result;
	int offset, size;
	struct soap soap;
	struct stat st;
	wsFilens__tFileName fileName;
	wsFilens__tData data;
		
		// Init soap environment
		soap_init(&soap);

		if (DEBUG)
			printf ("[sendFile] host:%s - src:%s - dst:%s\n", host, src, dst);
		
		// Create file in the server
		fileName.__ptr = (char *) malloc (strlen(dst) + 1);
		fileName.__size = strlen(dst) + 1;
		memset (fileName.__ptr, 0, strlen(dst) + 1);
		strcpy (fileName.__ptr, dst);
		
		if (DEBUG)
			printf ("[sendFile] invoking -> wsFilens__createFile(%s)\n", fileName.__ptr);
		
		// Create a new file in the server side
		soap_call_wsFilens__createFile (&soap, host, "", fileName, &result);
	
		// Error?
		if (result < 0){
			showError ("Error while creating destination file!");
		}
		else{

			if (DEBUG)
				printf ("[sendFile] File created in the server:%s\n", dst);

			// Get file size
			if (stat(src, &st) == -1)
				showError ("Error while executing stat in [sendFile]\n");

			fileSize = st.st_size;

			if (DEBUG)
				printf ("[sendFile] stat(%s) = %d bytes\n", fileName.__ptr, fileSize);

			// Error opening the file?
			if ((fd = open (src, O_RDONLY)) < 0){
				showError ("Error while opening source file\n");
			}
			else{

				// Init...
				remainingBytes = fileSize;
				offset = 0;
				data.__ptr = (unsigned char *) malloc (MAX_DATA_SIZE);

				// While there are remaining bytes to read
				while (remainingBytes > 0){

					// Set parameters
					data.__size = remainingBytes >= MAX_DATA_SIZE ? MAX_DATA_SIZE : remainingBytes;

					// Read from local file					
					readBytes = read (fd, data.__ptr, data.__size);
					data.__size = readBytes;

					// Invoke remote-write
					soap_call_wsFilens__writeToFile(&soap, host, "", fileName, offset, data, &result);
					
					// Error in the soap environment?
					if (soap.error) {
      					soap_print_fault(&soap, stderr); 
						exit(1);
  					}

					// Error in the server function?
					else if (result < 0){
						showError ("Error while writing data to destination file\n");
					}

					// Update offset
					offset += data.__size;

					// Update remaining bytes
					remainingBytes -= data.__size;
				}

				close (fd);
			}
		}
		
	// Clean soap environment	
	soap_destroy(&soap);
  	soap_end(&soap);
  	soap_done(&soap);
}


int main(int argc, char **argv){ 
    
  	// Check arguments
	if(argc != 5) {
		printf("Usage: %s http://server:port [sendFile|receiveFile] sourceFile destinationFile\n",argv[0]);
		exit(1);
	}

	// Check mode
	else{

		// Send file to server
		if (strcmp (argv[2], "sendFile") == 0){
			sendFile (argv[1], argv[3], argv[4]);
		}

		// Receive file from server
		else if (strcmp (argv[2], "receiveFile") == 0){
			receiveFile (argv[1], argv[3], argv[4]);
		}

		// Wrong mode!
		else{
			printf("Wrong mode!\nusage: %s serverIP [sendFile|receiveFile] sourceFile destinationFile\n", argv[0]);
			exit(1);
		}
	}	

  return 0;
}

