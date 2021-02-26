/** Length for buffer */
const MAXSIZE = 4096;

/** Length for tString */
const STRING_LENGTH = 128;
  
/** Type for file names */
typedef char tString [STRING_LENGTH];
 
struct t_request {
	tString fileName;
};
 
struct t_readParams {
	tString fileName;
	int offset;
	int size;
};

struct t_data{
	opaque data[MAXSIZE];
	int size;
};

struct t_writeParams {
	tString fileName;
	int offset;
	t_data data;
};
 
program FILESERVER {
  version FILESERVER_VER{
		int getFileSize (t_request) = 1;		
		int createFile (t_request) = 2;
		t_data readFromFile (t_readParams) = 3;
		int writeToFile (t_writeParams) = 4;		
  } = 1;
} = 9966;

