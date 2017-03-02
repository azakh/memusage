memusage : main.o
	gcc -o memusage main.o -lc

main.o : main.c
	gcc -c main.c

clean :
	rm memusage main.o
