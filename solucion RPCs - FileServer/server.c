#include "fileServer.h"
#include "server.h"



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


int* getfilesize_1_svc(t_request *argp, struct svc_req *rqstp){

	static int result;

		result = calculateFileSize (argp->fileName);

	return(&result);
}


int *createfile_1_svc(t_request *argp, struct svc_req *rqstp){

	static int result;
	int fd;

		result = 0;
		fd = open(argp->fileName, O_WRONLY | O_TRUNC | O_CREAT, 0777);

		if (fd < 0){
			printf ("[createFile_1] Error while creating(%s)\n", argp->fileName);
			result = -1;
		}
		else
			close (fd);


	return (&result);
}


t_data *readfromfile_1_svc(t_readParams *argp, struct svc_req *rqstp){

	static t_data data;
	int fd, remainingBytes, readBytes, result, fileSize;
	char *ptr;

		// Error opening the file
		if ((fd = open (argp->fileName, O_RDONLY)) < 0){
			printf ("[readfromfile_1] Error while open(%s)\n", argp->fileName);
			result = -1;
		}

		// File opened!
		else{

			// Error while positioning in the file
			if ((result =  lseek (fd, argp->offset, SEEK_SET)) < 0){
				printf ("[readfromfile_1] Error while lseek(%s)\n", argp->fileName);
				result = -1;
			}

			// lseek done!
			else{

				if ((fileSize = calculateFileSize (argp->fileName)) < 0){
					printf ("[readfromfile_1] Error while calculateFileSize(%s)\n", argp->fileName);
					result = -1;
				}

				else{

					// Calculates the number of bytes to read
					if ((fileSize-argp->offset) > argp->size)
						remainingBytes = argp->size;
					else
						remainingBytes = fileSize - argp->offset;

					// Bytes to read
					data.size = remainingBytes;

					// Init pointer
					ptr = data.data;

					// Send file contents
					while (remainingBytes > 0){

						// Read from file
						readBytes = read (fd, ptr, remainingBytes);

						if (readBytes <= 0){
							printf ("[readfromfile_1] ERROR while reading file\n");
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
				data.size = -1;
				memset (data.data, 0, MAXSIZE);
			}

			// Close file
			close (fd);
		}

	return(&data);
}


int *writetofile_1_svc(t_writeParams *argp, struct svc_req *rqstp){

	static int result;
	int fd, remainingBytes, writeBytes, fileSize;
	char *ptr;

		// Error opening the file
		if ((fd = open (argp->fileName, O_WRONLY)) < 0){
			printf ("[writetofile_1] Error while open(%s)\n", argp->fileName);
			result = -1;
		}

		// File opened!
		else{

			// Error while positioning in the file
			if ((result =  lseek (fd, argp->offset, SEEK_SET)) < 0){
				printf ("[writetofile_1] Error while lseek(%s)\n", argp->fileName);
				result = -1;
			}

			// lseek done!
			else{

				if ((fileSize = calculateFileSize (argp->fileName)) < 0){
					printf ("[writetofile_1] Error while calculateFileSize(%s)\n", argp->fileName);
					result = -1;
				}

				else{

					// Calculates the number of bytes to write
					remainingBytes = argp->data.size;
					result = 0;

					// Init pointer
					ptr = argp->data.data;

					// Send file contents
					while (remainingBytes > 0){

						// Read from file
						writeBytes = write (fd, ptr, remainingBytes);

						if (writeBytes <= 0){
							printf ("[writetofile_1] ERROR while writing file\n");
							result = -1;
						}
						else{
							remainingBytes -= writeBytes;
							result += writeBytes;
							ptr += writeBytes;
						}
					}
				}
			}

			// Close file
			close (fd);
		}

	return(&result);

}
