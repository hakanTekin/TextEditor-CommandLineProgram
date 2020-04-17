hakanAhmetTekinMain: main.o
	gcc main.c -o hakanAhmetTekin

main.o: main.c
	gcc -O -Wall -c main.c - pthread

clean:
	rm -rf *o hakanAhmetTekinMain main.o
