build:
	gcc -o tema2 tema2.c

run: build
	./tema2

clean:
	rm -f tema2
