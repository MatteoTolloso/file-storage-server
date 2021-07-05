#!/bin/bash


./server ./tests/config2.txt &
SERVER_PID=$!
sleep 2
./client -p -t 200 -f mysock -D tests/evictSaves -w tests/files 


kill -1 ${SERVER_PID}

exit 0