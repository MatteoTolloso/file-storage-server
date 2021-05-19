
flags = -g -O3 -Wall -I ./includes -std=c99 -pedantic #-Werror

#all 

all: server

#linking

server: ./obj/main.o ./obj/parser.o ./obj/sharedqueue.o ./obj/connection.o ./obj/handler.o
	gcc ./obj/main.o ./obj/parser.o ./obj/sharedqueue.o ./obj/connection.o ./obj/handler.o -o ./server -pthread

#obj file

./obj/main.o: ./src/main.c
	gcc ./src/main.c $(flags) -c -o ./obj/main.o

./obj/parser.o: ./src/parser.c
	gcc ./src/parser.c $(flags) -c -o ./obj/parser.o

./obj/sharedqueue.o: ./src/sharedqueue.c
	gcc ./src/sharedqueue.c $(flags) -c -o ./obj/sharedqueue.o

./obj/connection.o: ./src/connection.c
	gcc ./src/connection.c $(flags) -c -o ./obj/connection.o

./obj/handler.o: ./src/handler.c
	gcc ./src/handler.c $(flags) -c -o ./obj/handler.o -pthread

# esprimere la dipendanza con gli header