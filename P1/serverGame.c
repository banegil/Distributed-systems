#include "serverGame.h"
#include <pthread.h>

void showError(const char *msg){
	perror(msg);
	exit(0);
}


void *threadProcessing(void *threadArgs){

	tSession session;		
	int messageLength, c = 0, change = 0;
	int endOfGame, code, code2, codeRival;				
	unsigned int card, num;	
	tPlayer currentPlayer;
	int socketPlayer1, socketPlayer2;
	t_Data *data;

		// Detach resources
		pthread_detach(pthread_self()) ;

		//Get sockets for players
		data = (t_Data *) threadArgs;
		socketPlayer1 = data->socketPlayer1;
		socketPlayer2 = data->socketPlayer2;

		// Receive player 1 info
 		messageLength=recv(socketPlayer1,  &session.player1Name,  sizeof(session.player1Name), 0);
			if(messageLength<0)showError("ERROR while reading from socket");

		// Receive player 2 info
 		messageLength=recv(socketPlayer2,  &session.player2Name,  sizeof(session.player2Name), 0);
			if(messageLength<0)showError("ERROR while reading from socket");

		// Init...
		endOfGame = FALSE;
		initSession (&session);
		currentPlayer = player1;

		while (!endOfGame){
		  printSession (&session);
/*********************************************START jugA*********************************************/
		  if(currentPlayer == player1){
			if(c == 0){
				//Envio codigo y stack a  JugA
				code = TURN_BET;
				messageLength=send(socketPlayer1,&code, sizeof(code), 0);
				 if(messageLength<0)showError("ERROR while writing to socket");
				messageLength=send(socketPlayer1,&session.player1Stack, sizeof(session.player1Stack), 0);
				if(messageLength<0)showError("ERROR while writing to socket");
	
				//Recibo apuesta y doy ok
				while(code != TURN_BET_OK){
	 				messageLength=recv(socketPlayer1,  &session.player1Bet,  sizeof(session.player1Bet ), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
					if(session.player1Bet > 0 && session.player1Bet <= MAX_BET && session.player1Bet <= session.player1Stack)
						code = TURN_BET_OK;
					messageLength=send(socketPlayer1, &code, sizeof(code), 0);
					if(messageLength<0)showError("ERROR while writing to socket");	
				}
				session.player1Stack -= session.player1Bet;

				//Envio codigo y stack a  JugB
				codeRival = TURN_BET;
				messageLength=send(socketPlayer2, &codeRival, sizeof(codeRival), 0);
					 if(messageLength<0)showError("ERROR while writing to socket");
				messageLength=send(socketPlayer2, &session.player2Stack, sizeof(session.player2Stack), 0);
					 if(messageLength<0)showError("ERROR while writing to socket");
	
					while(codeRival != TURN_BET_OK){
					messageLength=recv(socketPlayer2,  &session.player2Bet,  sizeof( session.player2Bet), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
					if(session.player2Bet > 0 && session.player2Bet <= MAX_BET && session.player2Bet <= session.player2Stack)
						codeRival = TURN_BET_OK;
					messageLength=send(socketPlayer2, &codeRival, sizeof(codeRival), 0);
					if(messageLength<0)showError("ERROR while writing to socket");	
				}
				session.player2Stack -= session.player2Bet;

				//Envio primeras 2 cartas
				for(int i = 0; i < 2; i++){
					card = getRandomCard(&session.gameDeck);
					num = session.player1Deck.numCards;
					session.player1Deck.cards[num] = card;
					session.player1Deck.numCards++;
					card = getRandomCard(&session.gameDeck);
					num = session.player2Deck.numCards;
					session.player2Deck.cards[num] = card;
					session.player2Deck.numCards++;
				}
				printSession (&session);
			}

			c = calculatePoints(&session.player1Deck) ;
			if (send(socketPlayer1,&c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer2,&c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");

			//Envio  stack,  deck, TURN_PLAY a jugA y TURN_PLAY_WAIT a jugB
			code = TURN_PLAY;
			if (send(socketPlayer1, &code, sizeof(code), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer1,&session.player1Stack, sizeof(session.player1Stack), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer1,&session.player1Deck, sizeof(session.player1Deck), 0)  <  0)
				showError("ERROR while writing to socket");

			//JugB
			codeRival = TURN_PLAY_WAIT;
			if (send(socketPlayer2, &codeRival, sizeof(codeRival), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer2,&session.player1Stack, sizeof(session.player1Stack), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer2,&session.player1Deck, sizeof(session.player1Deck), 0)  <  0)
				showError("ERROR while writing to socket");

			//Recibo TURN_PLAY_HIT o TURN_PLAY _STAND y respondo
 			if (recv(socketPlayer1, &code2,  sizeof(code2), 0) < 0)  
				showError("ERROR while 2reading from socket");
			if (send(socketPlayer2, &code2, sizeof(code2), 0)  <  0)
					showError("ERROR while writing to socket");

			//Juego
			while(code2 == TURN_PLAY_HIT  && code != TURN_PLAY_OUT) {
				card = getRandomCard(&session.gameDeck);
				num = session.player1Deck.numCards;
				session.player1Deck.cards[num] = card;
				session.player1Deck.numCards++;
				c = calculatePoints(&session.player1Deck) ;
				if(c > 21) code = TURN_PLAY_OUT;
				else code = TURN_PLAY;

				//JugB
				if (send(socketPlayer1,&session.player1Deck, sizeof(session.player1Deck), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer1,&session.player1Stack, sizeof(session.player1Stack), 0)  <  0)
					showError("ERROR while writing to socket");

				//JugA
				if (send(socketPlayer2,&session.player1Deck, sizeof(session.player1Deck), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer2,&session.player1Stack, sizeof(session.player1Stack), 0)  <  0)
					showError("ERROR while writing to socket");
				
				//Envio calculo puntos
				if (send(socketPlayer1,&c, sizeof(c), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer2,&c, sizeof(c), 0)  <  0)
					showError("ERROR while writing to socket");

				//Envio code
				if (send(socketPlayer1, &code, sizeof(code), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer2, &code, sizeof(code), 0)  <  0)
					showError("ERROR while writing to socket");

				if(code != 	TURN_PLAY_OUT){
					//Recibo TURN_PLAY_HIT o TURN_PLAY _STAND y respondo
 					if (recv(socketPlayer1, &code2,  sizeof(code2), 0) < 0)  
						showError("ERROR while 2reading from socket");
					if (send(socketPlayer2, &code2, sizeof(code2), 0)  <  0)
						showError("ERROR while writing to socket");
				}
			}

			if(change == 1 || c > 21) updateStacks(&session);
			if (send(socketPlayer1, &c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer2, &c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");

			if(session.player1Stack == 0 && (change == 1 || c > 21)) {
				code = TURN_GAME_LOSE;
				codeRival = TURN_GAME_WIN;
				endOfGame = TRUE;
			}
			if(session.player2Stack == 0 && (change == 1 || c > 21)) {
				code = TURN_GAME_WIN;
				codeRival = TURN_GAME_LOSE;
				endOfGame = TRUE;
			}

			if (send(socketPlayer1, &code, sizeof(code), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer2, &codeRival, sizeof(codeRival), 0)  <  0)
				showError("ERROR while writing to socket");
			
		  }
/*********************************************END jugA*********************************************/

/*********************************************START jugB*********************************************/
		  else {
			if(c == 0){
				//Envio codigo y stack a  JugB
				codeRival = TURN_BET;
				messageLength=send(socketPlayer2, &codeRival, sizeof(codeRival), 0);
					 if(messageLength<0)showError("ERROR while writing to socket");
				messageLength=send(socketPlayer2, &session.player2Stack, sizeof(session.player2Stack), 0);
					 if(messageLength<0)showError("ERROR while writing to socket");
	
				//Recibo apuesta y doy ok
				while(codeRival != TURN_BET_OK){
					messageLength=recv(socketPlayer2,  &session.player2Bet,  sizeof( session.player2Bet), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
					if(session.player2Bet > 0 && session.player2Bet <= MAX_BET && session.player2Bet <= session.player2Stack)
						codeRival = TURN_BET_OK;
					messageLength=send(socketPlayer2, &codeRival, sizeof(codeRival), 0);
					if(messageLength<0)showError("ERROR while writing to socket");	
				}
				session.player2Stack -= session.player2Bet;

				//Envio codigo y stack a  JugA
				code = TURN_BET;
				messageLength=send(socketPlayer1,&code, sizeof(code), 0);
				 if(messageLength<0)showError("ERROR while writing to socket");
				messageLength=send(socketPlayer1,&session.player1Stack, sizeof(session.player1Stack), 0);
				if(messageLength<0)showError("ERROR while writing to socket");	

				while(code != TURN_BET_OK){
	 				messageLength=recv(socketPlayer1,  &session.player1Bet,  sizeof(session.player1Bet ), 0);
					if(messageLength<0)showError("ERROR while reading from socket");
					if(session.player1Bet > 0 && session.player1Bet <= MAX_BET && session.player1Bet <= session.player1Stack)
						code = TURN_BET_OK;
					messageLength=send(socketPlayer1, &code, sizeof(code), 0);
					if(messageLength<0)showError("ERROR while writing to socket");	
				}
				session.player1Stack -= session.player1Bet;

				//Envio primeras 2 cartas
				for(int i = 0; i < 2; i++){
					card = getRandomCard(&session.gameDeck);
					num = session.player1Deck.numCards;
					session.player1Deck.cards[num] = card;
					session.player1Deck.numCards++;
					card = getRandomCard(&session.gameDeck);
					num = session.player2Deck.numCards;
					session.player2Deck.cards[num] = card;
					session.player2Deck.numCards++;
				}	
				printSession (&session);
			}

			c = calculatePoints(&session.player2Deck) ;
			if (send(socketPlayer2,&c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer1,&c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");

			//Envio  stack,  deck, TURN_PLAY a jugB y TURN_PLAY_WAIT a jugA
			code = TURN_PLAY;
			if (send(socketPlayer2, &code, sizeof(code), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer2,&session.player2Stack, sizeof(session.player2Stack), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer2,&session.player2Deck, sizeof(session.player2Deck), 0)  <  0)
				showError("ERROR while writing to socket");

			//JugA
			codeRival = TURN_PLAY_WAIT;
			if (send(socketPlayer1, &codeRival, sizeof(codeRival), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer1,&session.player2Stack, sizeof(session.player2Stack), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer1,&session.player2Deck, sizeof(session.player2Deck), 0)  <  0)
				showError("ERROR while writing to socket");

			//Recibo TURN_PLAY_HIT o TURN_PLAY _STAND y respondo
 			if (recv(socketPlayer2, &code2,  sizeof(code2), 0) < 0)  
				showError("ERROR while 2reading from socket");
			if (send(socketPlayer1, &code2, sizeof(code2), 0)  <  0)
					showError("ERROR while writing to socket");

			//Juego
			while(code2 == TURN_PLAY_HIT  && code != TURN_PLAY_OUT) {
				card = getRandomCard(&session.gameDeck);
				num = session.player2Deck.numCards;
				session.player2Deck.cards[num] = card;
				session.player2Deck.numCards++;
				c = calculatePoints(&session.player2Deck) ;
				if(c > 21) code = TURN_PLAY_OUT;
				else code = TURN_PLAY;

				//JugB
				if (send(socketPlayer2,&session.player2Deck, sizeof(session.player2Deck), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer2,&session.player2Stack, sizeof(session.player2Stack), 0)  <  0)
					showError("ERROR while writing to socket");

				//JugA
				if (send(socketPlayer1,&session.player2Deck, sizeof(session.player2Deck), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer1,&session.player2Stack, sizeof(session.player2Stack), 0)  <  0)
					showError("ERROR while writing to socket");
				
				//Envio calculo puntos
				if (send(socketPlayer2,&c, sizeof(c), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer1,&c, sizeof(c), 0)  <  0)
					showError("ERROR while writing to socket");

				//Envio code
				if (send(socketPlayer2, &code, sizeof(code), 0)  <  0)
					showError("ERROR while writing to socket");
				if (send(socketPlayer1, &code, sizeof(code), 0)  <  0)
					showError("ERROR while writing to socket");

				if(code != 	TURN_PLAY_OUT){
					//Recibo TURN_PLAY_HIT o TURN_PLAY _STAND y respondo
 					if (recv(socketPlayer2, &code2,  sizeof(code2), 0) < 0)  
						showError("ERROR while 2reading from socket");
					if (send(socketPlayer1, &code2, sizeof(code2), 0)  <  0)
						showError("ERROR while writing to socket");
				}
			}

			if(change == 1 || c > 21) updateStacks(&session);
			if (send(socketPlayer2, &c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer1, &c, sizeof(c), 0)  <  0)
				showError("ERROR while writing to socket");

			if(session.player1Stack == 0 && (change == 1 || c > 21)) {
				code = TURN_GAME_LOSE;
				codeRival = TURN_GAME_WIN;
				endOfGame = TRUE;
			}
			else if(session.player2Stack == 0 && (change == 1 || c > 21)) {
				code = TURN_GAME_WIN;
				codeRival = TURN_GAME_LOSE;
				endOfGame = TRUE;
			}

			if (send(socketPlayer2, &code, sizeof(code), 0)  <  0)
				showError("ERROR while writing to socket");
			if (send(socketPlayer1, &codeRival, sizeof(codeRival), 0)  <  0)
				showError("ERROR while writing to socket");
			
		  }
/*********************************************ENDjugB*********************************************/

		 if(change % 2 == 0 || c > 21) {
		 	currentPlayer = getNextPlayer(currentPlayer);
			change = 0;
		 }
		change++;
		if(change == 2 || c > 21) {
			if(c > 21) change = 0;
			c = 0; 
			clearDeck (&(session.player1Deck));
			clearDeck (&(session.player2Deck));
			session.player1Bet = 0;
			session.player2Bet = 0;
		}
	}

	// Close sockets
	close (socketPlayer1);
	close (socketPlayer2);

	return (NULL) ;
}

int acceptConnection (int socketServer){

	int clientSocket;
	struct sockaddr_in clientAddress;
	unsigned int clientAddressLength;

		// Get length of client address
		clientAddressLength = sizeof(clientAddress);

		// Accept
		if ((clientSocket = accept(socketServer, (struct sockaddr *) &clientAddress, &clientAddressLength)) < 0)
			showError("Error while accepting connection");

		printf("Connection established with client: %s\n", inet_ntoa(clientAddress.sin_addr));

	return clientSocket;
}

int createBindListenSocket (unsigned short port){

	int socketId;
	struct sockaddr_in echoServAddr;

		if ((socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			showError("Error while creating a socket") ;

		// Set server address
		memset(&echoServAddr, 0, sizeof(echoServAddr));
		echoServAddr.sin_family = AF_INET;
		echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		echoServAddr.sin_port = htons(port);

		// Bind
		if (bind(socketId, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
			showError ("Error while binding");

		if (listen(socketId, MAX_CONNECTIONS) < 0)
			showError("Error while listening") ;

	return socketId;
}

int main(int argc, char *argv[]){

	int serverSocket;
	unsigned short listeningPort;
	pthread_t threadIDvector[MAX_CONNECTIONS];
	t_Data threadArgs[MAX_CONNECTIONS];
	int i = 0;

		// Check arguments
		if (argc != 2){
			fprintf(stderr, "Usage: %s port\n", argv[0]);
			exit(1);
		}

		// Get the port
		listeningPort = atoi(argv[1]);

		// Create a socket (also bind and listen)
		serverSocket = createBindListenSocket (listeningPort);

		// Infinite loop...
		while(1){

			// Establish connection with a client
			threadArgs[i].socketPlayer1 = acceptConnection(serverSocket);
			threadArgs[i].socketPlayer2  = acceptConnection(serverSocket);

			// Create a new thread
			if (pthread_create(&threadIDvector[i], NULL, threadProcessing,  &threadArgs[i]) != 0)
				showError("pthread_create() failed");
			i++;
		}

}
