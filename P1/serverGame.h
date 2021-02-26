#include "game.h"

/** Debug server? */
#define SERVER_DEBUG 1

// Maximum number of connections
#define MAX_CONNECTIONS 10

/** Sockets of a game used by a thread in the server */
typedef struct ThreadArgs{
	int socketPlayer1;
	int socketPlayer2;
}t_Data;

/**
 * Function that shows an error message.
 *
 * @param msg Error message.
 */
void showError(const char *msg);


/**
 * Function executed by each thread.
 *
 * @param threadArgs Argument that contains the socket of the players.
 */
void *threadProcessing(void *threadArgs);
