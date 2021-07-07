#!/bin/bash


valgrind --leak-check=full -s --leak-check=full --show-leak-kinds=all ./server ./tests/config2.txt &
SERVER_PID=$!
./client -t 200 -f mysock -D tests/fileEvict -w tests/files 


kill -s SIGINT ${SERVER_PID}

exit 0