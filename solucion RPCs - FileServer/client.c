#include "fileServer.h"
#include "client.h"

#define DEBUG 1

void showError (char* msg){

	printf ("[Client] %s\n", msg);
	exit (1);
}


/**
 * Copy a file from server side to client side
 */
void receiveFile (char *host, char* src, char* dst){

	int fd, fileSize, remainingBytes, offset;
	int *result;
	CLIENT *clnt;
	t_request t_requestArg;
	t_readParams  t_readParamsArg;
	t_data* t_dataArg;
	char buffer[MAXSIZE];


		if (DEBUG)
			printf ("[receiveFile] host:%s - src:%s - dst:%s\n", host, src, dst);

		// Create client
		clnt = clnt_create(host, FILESERVER, FILESERVER_VER, "udp");

		if (clnt == NULL) {
			showError ("Error while creating CLNT structure!");
			clnt_pcreateerror(host);
			exit(1);
		}

		// Create file
		fd = open(dst, O_WRONLY | O_TRUNC | O_CREAT, 0777);

		// Error?
		if (fd < 0){
			showError ("Error while creating destination file!");
		}
		else{

			if (DEBUG)
				printf ("[receiveFile] File created:%s\n", src);

			// Get file size from server
			strcpy (t_requestArg.fileName, src);
			result = getfilesize_1(&t_requestArg, clnt);
			fileSize = *result;

			if (fileSize < 0){
				showError ("Error while obtaining the size of source file!");
			}

			// All OK! Start transferring the file...
			else{

				if (DEBUG)
					printf ("[receiveFile] getfilesize_1(%s) = %d bytes\n", t_requestArg.fileName, fileSize);

				remainingBytes = fileSize;
				strcpy (t_readParamsArg.fileName, src);
				t_readParamsArg.offset = 0;

				// While there are remaining bytes to read
				while (remainingBytes > 0){

					// Set parameters
					t_readParamsArg.size = remainingBytes >= MAXSIZE ? MAXSIZE : remainingBytes;

					// Invoke remote-read
					t_dataArg = readfromfile_1(&t_readParamsArg, clnt);

					// Write into local file
					write (fd, t_dataArg->data, t_dataArg->size);

					// Update offset
					t_readParamsArg.offset += t_dataArg->size;

					// Update remaining bytes
					remainingBytes -= t_dataArg->size;
				}

				close (fd);
			}
		}
}


/**
 * Copy a file from client side to server side
 */
void sendFile (char *host, char* src, char* dst){

	int fd, fileSize, remainingBytes, offset;
	int *result;
	CLIENT *clnt;
	struct stat st;
	t_request t_requestArg;
	t_writeParams t_writeParamsArg;
	char buffer[MAXSIZE];


		if (DEBUG)
			printf ("[sendFile] host:%s - src:%s - dst:%s\n", host, src, dst);

		// Create client
		clnt = clnt_create(host, FILESERVER, FILESERVER_VER, "udp");

		if (clnt == NULL) {
			showError ("Error while creating CLNT structure!");
			clnt_pcreateerror(host);
			exit(1);
		}

		// Create file in the server
		strcpy (t_requestArg.fileName, dst);
		result = createfile_1(&t_requestArg, clnt);

		// Error?
		if (*result < 0){
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
				printf ("[sendFile] stat(%s) = %d bytes\n", t_requestArg.fileName, fileSize);

			// Error opening the file?
			if ((fd = open (src, O_RDONLY)) < 0){
				showError ("Error while opening source file\n");
			}
			else{

				// Init...
				remainingBytes = fileSize;
				strcpy (t_writeParamsArg.fileName, dst);
				t_writeParamsArg.offset = 0;

				// While there are remaining bytes to read
				while (remainingBytes > 0){

					// Set parameters
					t_writeParamsArg.data.size = remainingBytes >= MAXSIZE ? MAXSIZE : remainingBytes;

					// Read from local file
					read (fd, t_writeParamsArg.data.data, t_writeParamsArg.data.size);

					// Invoke remote-write
					result = writetofile_1(&t_writeParamsArg, clnt);

					if (*result < 0){
						showError ("Error while writing data to destination file\n");
					}

					// Update offset
					t_writeParamsArg.offset += t_writeParamsArg.data.size;

					// Update remaining bytes
					remainingBytes -= t_writeParamsArg.data.size;
				}

				close (fd);
			}
		}
}


int main (int argc, char *argv[]){

	// Check arguments
	if(argc != 5) {
		printf("usage: %s serverIP [sendFile|receiveFile] sourceFile destinationFile\n", argv[0]);
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
}
