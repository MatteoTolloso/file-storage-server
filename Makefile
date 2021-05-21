
flags = -g -O3 -Wall -I ./includes -std=c99 -pedantic #-Werror

obj = ./obj/main.o ./obj/parser.o ./obj/sharedqueue.o ./obj/connection.o ./obj/handler.o

src = ./src/main.c ./src/parser.c ./src/sharedqueue.c ./src/connection.c ./src/handler.c

includes = ./includes/myconnection.h ./includes/myhandler.h \
			./includes/myparser.h ./includes/mysharedqueue.h \
			./includes/myutil.h

objpath = ./obj/

srcpath = ./src/

.PHONY: clean test

#all 

all: server

#linking

./server: $(obj)
	gcc $(obj) -o $@ -pthread

#obj file

$(objpath)main.o: $(srcpath)main.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)parser.o: $(srcpath)parser.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)sharedqueue.o: $(srcpath)sharedqueue.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)connection.o: $(srcpath)connection.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)handler.o: $(srcpath)handler.c $(includes)
	gcc $< $(flags) -c -o $@ -pthread

cleanall	: 
	rm -f $(obj) ./server

test	:
	valgrind --leak-check=full --show-leak-kinds=all ./server config.txt 