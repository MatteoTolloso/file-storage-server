

#linking

server: main.o parser.o sharedqueue.o
	gcc ./obj/main.o ./obj/parser.o ./obj/sharedqueue.o -o server

#obj file

main.o: ./src/main.c
	gcc ./src/main.c -I ./includes -c -o ./obj/main.o

parser.o: ./src/parser.c
	gcc ./src/parser.c -I ./includes -c -o ./obj/parser.o

sharedqueue.o: ./src/sharedqueue.c
	gcc ./src/sharedqueue.c -I ./includes -c -o ./obj/sharedqueue.o