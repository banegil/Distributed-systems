CC = gcc
CFLAGS = -g -DRPC_SVC_FG
RPCGEN_FLAG = -C

all: calculator.h calculator_Client calculator_Server

calculator.h: calculator.x
	rpcgen $(RPCGEN_FLAG) calculator.x

calculator_Client: calculator_Client.o calculator_clnt.o calculator_xdr.o
	$(CC) -o calculator_Client calculator_Client.o calculator_clnt.o calculator_xdr.o

calculator_Server: calculator_Server.o calculator_svc.o calculator_xdr.o
	$(CC) -o calculator_Server calculator_Server.o calculator_svc.o calculator_xdr.o

calculator_Server.o: calculator_Server.c calculator.h
	$(CC) $(CFLAGS) -c calculator_Server.c

calculator_Client.o: calculator_Client.c calculator.h
	$(CC) $(CFLAGS) -c calculator_Client.c

calculator_svc.o: calculator_svc.c calculator.h
	$(CC) $(CFLAGS) -c calculator_svc.c 
		
calculator_clnt.o: calculator_clnt.c calculator.h
	$(CC) $(CFLAGS) -c calculator_clnt.c 
	
calculator_xdr.o: calculator_xdr.c calculator.h 
	$(CC) $(CFLAGS) -c calculator_xdr.c

clean:
	rm -f calculator_Client calculator_Client.o calculator_Server calculator_Server.o calculator_clnt.* calculator_svc.* calculator_xdr.* calculator.h

