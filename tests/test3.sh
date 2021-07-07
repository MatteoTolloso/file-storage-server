#!/bin/bash


./server ./tests/config3.txt &
SERVER_PID=$!
./client -p -t 200 -f mysock -D tests/fileEvict -w tests/files 

kill -s SIGINT ${SERVER_PID}

sleep 1
exit 0