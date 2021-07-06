#!/bin/bash


./server ./tests/config2.txt &
SERVER_PID=$!
sleep 2
./client -p -t 200 -f mysock -D tests/fileEvict -w tests/files 


kill -s SIGINT ${SERVER_PID}

exit 0