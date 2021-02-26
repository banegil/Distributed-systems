CC = gcc
CFLAGS = -g -m32

all: matrixScatter matrixDynamic

matrixScatter: matrixScatter.c
	mpicc matrixScatter.c -o matrixScatter -lm	

matrixDynamic: matrixDynamic.c
	mpicc matrixDynamic.c -o matrixDynamic -lm

clean:
	rm -f matrixScatter matrixDynamic
