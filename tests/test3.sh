#!/bin/bash


./server ./tests/config3.txt &
SERVER_PID=$!
./client -t 200 -f mysock -d tests/fileLetti/ -W tests/files/test1,tests/files/test2 -r tests/files/test1,tests/files/test2 &
./client -t 200 -f mysock  -l tests/files/test1,tests/files/test2 -u tests/files/test1,tests/files/test2 &
./client -t 200 -f mysock  -c tests/files/test1,tests/files/test2 

sleep 1
kill -s SIGINT ${SERVER_PID}

exit 0