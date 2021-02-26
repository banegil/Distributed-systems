struct arguments {
	int param1;
	int param2;
};

program CALCULATOR {
	version CALCULATORVER {
		int add (arguments) = 1;
		int sub (arguments) = 2; 
		int mul (arguments) = 3; 
		int div (arguments) = 4; 
	} = 1;
} = 99;
