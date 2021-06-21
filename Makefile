
flags = -g -O3 -Wall -I ./includes -std=c99 -pedantic #-Werror

obj = ./obj/server.o ./obj/parser.o ./obj/sharedqueue.o ./obj/connection.o ./obj/handler.o ./obj/filesystem.o ./obj/linkedlist.o

src = ./src/server.c ./src/parser.c ./src/sharedqueue.c ./src/connection.c ./src/handler.c ./src/filesystem.c ./src/linkedlist.c

includes = ./includes/myconnection.h ./includes/myhandler.h \
			./includes/myparser.h ./includes/mysharedqueue.h \
			./includes/myutil.h ./includes/myfilesystem.h \
			./includes/mylinkedlist.h

objpath = ./obj/

srcpath = ./src/

.PHONY: clean test

#all 

all: ./server ./client

#linking

./server: $(obj)
	gcc $(obj) -o $@ -pthread

./client: ./obj/client.o
	gcc $< -o $@ -pthread

#obj file for server

$(objpath)server.o: $(srcpath)server.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)parser.o: $(srcpath)parser.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)sharedqueue.o: $(srcpath)sharedqueue.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)connection.o: $(srcpath)connection.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)handler.o: $(srcpath)handler.c $(includes)
	gcc $< $(flags) -c -o $@ -pthread

$(objpath)filesystem.o: $(srcpath)filesystem.c $(includes)
	gcc $< $(flags) -c -o $@ -pthread

$(objpath)linkedlist.o: $(srcpath)linkedlist.c $(includes)
	gcc $< $(flags) -c -o $@ -pthread

#obj file for client

./obj/client.o: ./src/client.c
	gcc $< $(flags) -c -o $@ 

cleanall	: 
	rm -f $(obj) ./server

test	:
	valgrind --leak-check=full --show-leak-kinds=all ./server config.txt 