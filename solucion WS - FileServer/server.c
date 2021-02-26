#include "soapH.h"
#include "wsFileServer.nsmap"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define DEBUG 1

int calculateFileSize (char* fileName);


int main(int argc, char **argv){ 

  int primarySocket, secondarySocket;
  struct soap soap;

  	if (argc < 2) {
    	printf("Usage: %s <port>\n",argv[0]); 
		exit(-1); 
  	}

	// Init environment
  	soap_init(&soap);

	// Bind to the specified port	
	primarySocket = soap_bind(&soap, NULL, atoi(argv[1]), 100);

	// Check result of binding		
	if (primarySocket < 0) {
  		soap_print_fault(&soap, stderr); 
		exit(-1); 
	}

	// Listen to next connection
	while (1) { 

		// accept
	  	secondarySocket = soap_accept(&soap);    

	  	if (secondarySocket < 0) {
			soap_print_fault(&soap, stderr); exit(-1);
	  	}

		// Execute invoked operation
	  	soap_serve(&soap);

		// Clean up!
	  	soap_end(&soap);
	}

  return 0;
}


int calculateFileSize (char* fileName){

	struct stat st;
	int result;

		if (stat(fileName, &st) == -1){
			printf ("[calculateFileSize] Error while executing stat(%s)\n", fileName);
			result = -1;
		}
		else{
			result = st.st_size;
		}

	return(result);
}


int wsFilens__getFileSize (struct soap *soap, wsFilens__tFileName fileName, int *result){

	char name [NAME_LENGTH];

	memset (name, 0, NAME_LENGTH);
	memcpy (name, fileName.__ptr, fileName.__size); 

	*result = calculateFileSize (name);

	if (DEBUG)
		printf ("wsFilens__getFileSize (%s) = %d\n", name, *result);

	return(SOAP_OK);
}


int wsFilens__createFile (struct soap *soap, wsFilens__tFileName fileName, int *result){

	int fd;
	char name [NAME_LENGTH];

		memset (name, 0, NAME_LENGTH);
		memcpy (name, fileName.__ptr, fileName.__size);

		*result = 0;
			
		// Create file	
		fd = open(name, O_WRONLY | O_TRUNC | O_CREAT, 0777);

		// Error?
		if (fd < 0){
			printf ("[wsFilens__createFile] Error while creating(%s)\n", name);
			*result = -1;
		}
		else
			close (fd);

		if (DEBUG)
			printf ("wsFilens__createFile (%s) = %d\n", name, *result);


	return (SOAP_OK);
}


int wsFilens__readFromFile(struct soap *soap, wsFilens__tFileName fileName, int offset, int size, wsFilens__tData *data){

	int fd, remainingBytes, readBytes, result, fileSize;
	unsigned char *ptr;
	char name [NAME_LENGTH];

		memset (name, 0, NAME_LENGTH);
		memcpy (name, fileName.__ptr, fileName.__size);
	

		// Error opening the file
		if ((fd = open (name, O_RDONLY)) < 0){
			printf ("[wsFilens__readFromFile] Error while open(%s)\n", name);
			result = -1;
		}

		// File opened!
		else{

			// Error while positioning in the file
			if ((result =  lseek (fd, offset, SEEK_SET)) < 0){
				printf ("[wsFilens__readFromFile] Error while lseek(%s)\n", name);
				result = -1;
			}

			// lseek done!
			else{

				if ((fileSize = calculateFileSize (name)) < 0){
					printf ("[wsFilens__readFromFile] Error while calculateFileSize(%s)\n", name);
					result = -1;
				}

				else{
				
					// Alloc
					data->__ptr = (unsigned char*) malloc (size);

					// Calculates the number of bytes to read
					if ((fileSize - offset) > size)
						remainingBytes = size;
					else
						remainingBytes = fileSize - offset;

					// Bytes to read
					data->__size = remainingBytes;

					// Init pointer
					ptr = data->__ptr;

					// Send file contents
					while (remainingBytes > 0){

						// Read from file
						readBytes = read (fd, ptr, remainingBytes);

						if (readBytes <= 0){
							printf ("[readFromFile] ERROR while reading file\n");
							result = remainingBytes = -1;
						}
						else{
							remainingBytes -= readBytes;
							ptr += readBytes;
						}
					}
				}
			}

			// Check for errors...
			if (result == -1){
				data->__size = -1;
				memset (data->__ptr, 0, MAX_DATA_SIZE);
			}

			// Close file
			close (fd);
		}

	return(SOAP_OK);
}


int wsFilens__writeToFile(struct soap *soap, wsFilens__tFileName fileName, int offset, wsFilens__tData data, int *result){

	int fd, remainingBytes, writeBytes, fileSize;
	unsigned char *ptr;
	char name [NAME_LENGTH];

		memset (name, 0, NAME_LENGTH);
		memcpy (name, fileName.__ptr, fileName.__size);

		// Error opening the file
		if ((fd = open (name, O_WRONLY)) < 0){
			printf ("[wsFilens__writeToFile] Error while open(%s)\n", name);
			*result = -1;
		}

		// File opened!
		else{

			// Error while positioning in the file
			if ((*result =  lseek (fd, offset, SEEK_SET)) < 0){
				printf ("[wsFilens__writeToFile] Error while lseek(%s)\n", name);
				*result = -1;
			}

			// lseek done!
			else{

				if ((fileSize = calculateFileSize (name)) < 0){
					printf ("[wsFilens__writeToFile] Error while calculateFileSize(%s)\n", name);
					*result = -1;
				}

				else{

					// Calculates the number of bytes to write
					remainingBytes = data.__size;
					*result = 0;

					// Init pointer
					ptr = data.__ptr;

					// Send file contents
					while (remainingBytes > 0){

						// Read from file
						writeBytes = write (fd, ptr, remainingBytes);

						if (writeBytes <= 0){
							printf ("[wsFilens__writeToFile] ERROR while writing file\n");
							*result = -1;
						}
						else{
							remainingBytes -= writeBytes;
							*result += writeBytes;
							ptr += writeBytes;
						}
					}
				}
			}

			// Close file
			close (fd);
		}

	return(SOAP_OK);
}

