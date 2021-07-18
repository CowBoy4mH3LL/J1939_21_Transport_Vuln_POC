all: main.c
	gcc -w -g -o build/main main.c -lm

clean:
	rm build/*