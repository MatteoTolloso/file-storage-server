#!/bin/bash


valgrind --leak-check=full ./server ./tests/config1.txt &
SERVER_PID=$!
./client -p -t 200 -f mysock -d tests/fileLetti/ -W tests/files/test1,tests/files/test2 -r tests/files/test1,tests/files/test2
./client -p -t 200 -f mysock  -l tests/files/test1,tests/files/test2 -u tests/files/test1,tests/files/test2
./client -p -t 200 -f mysock  -c tests/files/test1,tests/files/test2 

kill -s SIGHUP ${SERVER_PID}

exit 0
