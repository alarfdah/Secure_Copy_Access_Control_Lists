all:
	gcc -std=c99 -pedantic -Wall -g -o get get.c
	chmod u+s get
