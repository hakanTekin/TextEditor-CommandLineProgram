hakanAhmetTekinMain: main.o
	gcc main.c -o hakanAhmetTekin -pthread

main.o: main.c
	gcc -O -Wall -c main.c
files: 
	batch.txt
clean:
	rm -rf *o hakanAhmetTekinMain main.o batch.txt hehe.txt
