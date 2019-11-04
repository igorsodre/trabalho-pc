default: build run

build: main.c
	gcc main.c -o prog -lpthread -lm

run: 
	./prog 9 4