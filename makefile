default: build run

build: main.c
	gcc list.c main.c  -o prog -lpthread -lm -std=c11

run: 
	./prog 90 80