#include "clientGame.h"

void showError(const char *msg){
	perror(msg);
	exit(0);
}

void showReceivedCode (unsigned int code){

	tString string;

		if (1){

			// Reset
			memset (string, 0,sizeof(STRING_LENGTH));

			switch(code){

				case TURN_BET:
					strcpy (string, "TURN_BET");
					break;

				case TURN_BET_OK:
					strcpy (string, "TURN_BET_OK");
					break;

				case TURN_PLAY:
					strcpy (string, "TURN_PLAY");
					break;

				case TURN_PLAY_HIT:
					strcpy (string, "TURN_PLAY_HIT");
					break;

				case TURN_PLAY_STAND:
					strcpy (string, "TURN_PLAY_STAND");
					break;

				case TURN_PLAY_OUT:
					strcpy (string, "TURN_PLAY_OUT");
					break;

				case TURN_PLAY_WAIT:
					strcpy (string, "TURN_PLAY_WAIT");
					break;

				case TURN_PLAY_RIVAL_DONE:
					strcpy (string, "TURN_PLAY_RIVAL_DONE");
					break;

				case TURN_GAME_WIN:
					strcpy (string, "TURN_GAME_WIN");
					break;

				case TURN_GAME_LOSE:
					strcpy (string, "TURN_GAME_LOSE");
					break;

				default:
					strcpy (string, "UNKNOWN CODE");
					break;
			}

			printf ("Received:%s\n", string);
		}
}

unsigned int readBet (){

	int i, isValid, bet;
	tString enteredMove;

		// Init...
		bet = 0;

		// While player does not enter a correct bet...
		do{

			// Init...
			bzero (enteredMove,sizeof(STRING_LENGTH));
			isValid = TRUE;

			// Show input message
			printf ("Enter a bet:");

			// Read move
			fgets(enteredMove,sizeof(STRING_LENGTH-1), stdin);

			// Remove new-line char
			enteredMove[strlen(enteredMove)-1] = 0;

			// Check if each character is a digit
			for (i=0; i<strlen(enteredMove) && isValid; i++){

				if (!isdigit(enteredMove[i]))
					isValid = FALSE;
			}

			// Entered move is not a number
			if (!isValid){
				printf ("Entered bet is not correct. It must be a number greater than 0\n");
			}

			// Entered move is a number...
			else{

				// Conver entered text to an int
				bet = atoi (enteredMove);
			}

		}while (!isValid);

		printf ("\n");

	return ((unsigned int) bet);
}

unsigned int readOption (){

	int i, isValid, option;
	tString enteredMove;

		// Init...
		option = 0;

		// While player does not enter a correct bet...
		do{

			// Init...
			bzero (enteredMove,sizeof(STRING_LENGTH));
			isValid = TRUE;

			// Show input message
			printf ("Press 1 to hit a card and 0 to stand:");

			// Read move
			fgets(enteredMove,sizeof(STRING_LENGTH-1), stdin);

			// Remove new-line char
			enteredMove[strlen(enteredMove)-1] = 0;

			// Check if each character is a digit
			for (i=0; i<strlen(enteredMove) && isValid; i++){

				if (!isdigit(enteredMove[i]))
					isValid = FALSE;
			}

			// Entered move is not a number
			if (!isValid){
				printf ("Wrong option!\n");
			}

			// Entered move is a number...
			else{

				// Conver entered text to an int
				option = atoi (enteredMove);

				if ((option != TURN_PLAY_HIT-2) && (option != TURN_PLAY_STAND-4)){
					printf ("Wrong option!\n");
					isValid = FALSE;
				}
			}

		}while (!isValid);

		if(option == TURN_PLAY_HIT-2) option +=2;
		else option += 4;
		printf ("\n");
	
	return ((unsigned int) option);
}


int main(int argc, char *argv[]){

	int socketfd;						/** Socket descriptor */
	unsigned int port;					/** Port number (server) */
	struct sockaddr_in server_address;	/** Server address structure */
	char* serverIP;						/** Server IP */
	unsigned int endOfGame;				/** Flag to control the end of the game */
	tString playerName;					/** Name of the player */
	unsigned int code;				/** Code */
	unsigned int bet;
	unsigned int stack;
        tDeck deck;
	int messageLength;
	unsigned int option, change = 0;
	unsigned int p = 0, saveCode;

		// Check arguments!
		if (argc != 3){
			fprintf(stderr,"ERROR wrong number of arguments\n");
			fprintf(stderr,"Usage:\n$>%s serverIP port\n", argv[0]);
			exit(0);
		}

		// Get the server address
		serverIP = argv[1];

		// Get the port
		port = atoi(argv[2]);

		// Create socket
		socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		// Fill server address structure
		memset(&server_address, 0, sizeof(server_address));
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = inet_addr(serverIP);
		server_address.sin_port = htons(port);

		// Connect with server
		if (connect(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
			showError("ERROR while establishing connection");
		
		printf ("Connection established with server!\n");

		// Init player's name
		do{
			memset(&playerName, 0,sizeof(playerName));
			printf ("Enter player name:");
			fgets(playerName,sizeof(playerName), stdin);

			// Remove '\n'
			playerName[strlen(playerName)-1] = 0;

		}while (strlen(playerName) <= 2);


		// Init
		endOfGame = FALSE;

		// Game starts
		printf ("Game starts!\n\n");

		messageLength=send(socketfd,&playerName, strlen(playerName), 0);
			 if(messageLength<0)showError("ERROR while writing to socket");

		// While game continues...
		while (!endOfGame){
			if(change == 0){
				//Recibo code y stack
 				messageLength=recv(socketfd, &code, sizeof(code), 0);
				if(messageLength<0)showError("ERROR while reading from socket");

				messageLength=recv(socketfd, &stack, sizeof(stack), 0);
				if(messageLength<0)showError("ERROR while reading from socket");
			
				//Envio apuesta y recibo TURN_BET_OK
				printf("Your stack is %u chips \n",stack);
				while(code != TURN_BET_OK){
					printf("The bet must be >0 and <=%u \n",stack);
					bet=readBet();
					messageLength=send(socketfd,&bet, sizeof(bet), 0);
					if(messageLength<0)showError("ERROR while writing to socket");
				
	 				messageLength=recv(socketfd,  &code, sizeof(code), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
				}
			}

			//Recibo primeras 2 cartas
			messageLength=recv(socketfd,  &p, sizeof(p), 0);
			if(messageLength<0)showError("ERROR while reading from socket");		

			//Recibo TURN_PLAY o TURN_PLAY_WAIT
 			messageLength=recv(socketfd,  &code, sizeof( code ), 0);
			if(messageLength<0)showError("ERROR while reading from socket");
			saveCode = code;

			messageLength=recv(socketfd, &stack, sizeof(stack), 0);
			if(messageLength<0)showError("ERROR while reading from socket");

			messageLength=recv(socketfd, &deck, sizeof(deck), 0);
			if(messageLength<0)showError("ERROR while reading from socket");

			//Print
			if(saveCode == TURN_PLAY){
				printf("Player has %u points \n",p);
				printf("Deck: ");
			}
			else {
				printf("Rival has %u points \n",p);
				printf("Rival's deck: ");
			}
			printDeck(&deck);
			printf("\n");

			if(code == TURN_PLAY) {
				option= readOption();
				messageLength=send(socketfd,&option, sizeof(option), 0);
				if(messageLength<0)showError("ERROR while 1writing to socket");
			}
			else{
				messageLength=recv(socketfd,  &option, sizeof(option), 0);
				if(messageLength<0)showError("ERROR while reading from socket");
			}

			//Juego
			while(option == TURN_PLAY_HIT && code != TURN_PLAY_OUT ){
				if(saveCode == TURN_PLAY){
					messageLength=recv(socketfd, &deck, sizeof(deck), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
					messageLength=recv(socketfd, &stack, sizeof(stack ), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
				}
				else{
					messageLength=recv(socketfd, &deck, sizeof(deck), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
					messageLength=recv(socketfd, &stack,  sizeof(stack), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
				}

				messageLength=recv(socketfd,  &p, sizeof(p), 0);
				if(messageLength<0)showError("ERROR while reading from socket");

				//Print
				if(saveCode == TURN_PLAY){
					printf("Player has %u points \n",p);
					printf("Deck: ");
				}
				else {
					printf("Rival has %u points \n",p);
					printf("Rival's deck: ");
				}
				printDeck(&deck);
				printf("\n");

				//Recibo code
				messageLength=recv(socketfd,  &code, sizeof(code), 0);
				if(messageLength<0)showError("ERROR while reading from socket");

				if(code != TURN_PLAY_OUT){
					if(saveCode == TURN_PLAY) {
						option = readOption();
						messageLength=send(socketfd,&option, sizeof(option), 0);
						if(messageLength<0)showError("ERROR while writing to socket");
					}
					else{
						messageLength=recv(socketfd,  &option, sizeof(option), 0);
						if(messageLength<0)showError("ERROR while reading from socket");
					}
				}
			}

			messageLength=recv(socketfd,  &p, sizeof(p), 0);
			if(messageLength<0)showError("ERROR while reading from socket");

			if(code == TURN_PLAY_OUT && saveCode == TURN_PLAY) printf("Player is out!!! %u points \n \n", p);
			else if(saveCode == TURN_PLAY_WAIT) printf("Rival is done! Now its your turn... \n \n \n");

			messageLength=recv(socketfd,  &code, sizeof( code), 0);
			if(messageLength<0)showError("ERROR while reading from socket");

			if(code == TURN_GAME_LOSE || code == TURN_GAME_WIN) {
				if(code == TURN_GAME_LOSE) printf("Player Loses! \n \n");
				else printf("Player Wins! \n \n");
				endOfGame = TRUE;
			}
			change++;
			if(change == 2 || p > 21) change = 0;
		}

	// Close socket
	close (socketfd);
}