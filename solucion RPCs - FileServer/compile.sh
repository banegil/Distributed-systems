gcc -c *.c
gcc -o client client.o fileServer_clnt.o fileServer_xdr.o
gcc -o server server.o fileServer_svc.o fileServer_xdr.o

