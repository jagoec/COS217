# Makefile for COS 217 Assignment 1: decomment
# Build with:  make        (or just:  make decomment)
# Remove build products with:  make clean

CC=gcc217
CFLAGS=-g

decomment: decomment.o
	$(CC) $(CFLAGS) decomment.o -o decomment

decomment.o: decomment.c
	$(CC) $(CFLAGS) -c -o decomment.o decomment.c

clean:
	rm -f decomment decomment.i decomment.s decomment.o
