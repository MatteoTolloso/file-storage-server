#!/bin/bash
 
 
valgrind --leak-check=full ./server tests/config3.txt &
SERVER_PID=$!
export SERVER_PID
bash -c 'sleep 3 && kill -s SIGHUP ${SERVER_PID}' &
./client -p -t 200 -f mysock -W tests/files/test1,tests/files/test2
./client -p -t 200 -f mysock -d tests/fileLetti/ -R n=0
./client -p -t 200 -f mysock  -l tests/files/test1,tests/files/test2 -u tests/files/test1,tests/files/test2
./client -p -t 200 -f mysock  -c tests/files/test1,tests/files/test2 
sleep 1
exit 0