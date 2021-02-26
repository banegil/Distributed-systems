#include "calculator.h"

void calculator_1(char *host, char operation, int param1, int param2) {

	CLIENT *clnt;
	int  *result;
	arguments args;

		// Create the client handler
		clnt = clnt_create (host, CALCULATOR, CALCULATORVER, "udp");

		// Check
		if (clnt == NULL) {
			clnt_pcreateerror (host);
			exit (1);
		}

		// Set parameters
		args.param1 = param1;
		args.param2 = param2;

		// Chose the operation to invoke
		switch(operation){

			case 'a':
				result = add_1(&args, clnt);
			break;

			case 's':
				result = sub_1(&args, clnt);
			break;

			case 'm':
				result = mul_1(&args, clnt);
			break;

			case 'd':
				result = div_1(&args, clnt);
			break;

			default:
			break;
		}

		// Check result
		if (result == (int *) NULL) {
			clnt_perror (clnt, "call failed");
		}
		else
			printf ("The result is:%d\n", *result);


		// Destroy client handle
		clnt_destroy (clnt);
}


int main (int argc, char *argv[]){

	char *host;
	char operation;
	int param1, param2;

		// Init...
		param1 = param2 = 0;
		operation = ' ';

		// Check the number of parameters
		if (argc != 5) {
			printf ("Usage: %s server_host [(a)dd|(s)ubtract|(m)ultiply|(d)ivide] parameter_1 parameter_2\n", argv[0]);
			exit (1);
		}

		// Get the host
		host = argv[1];

		// Get operation
		operation = argv[2][0];

		if ((operation != 'a') &&
			(operation != 's') &&
			(operation != 'm') &&
			(operation != 'd')){
			printf ("Error, invalid operator[%c].\n", operation);
			exit(1);
		}

		// Get parameter 1
		param1 = atoi (argv[3]);

		// Get parameter 2
		param2 = atoi (argv[4]);

		// Execute operation
		calculator_1 (host, operation, param1, param2);

	exit (0);
}
