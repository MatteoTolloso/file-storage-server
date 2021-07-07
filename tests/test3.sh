#!/bin/bash


./server ./tests/config3.txt &
SERVER_PID=$!
./client -t 0 -f mysock -D tests/fileEvict -w tests/files &
./client -t 0 -f mysock -d tests/fileLetti/ -W tests/files/test1,tests/files/test2 -r tests/files/test1,tests/files/test2 &
./client -t 0 -f mysock  -l tests/files/test1,tests/files/test2 -u tests/files/test1,tests/files/test2 &
./client -t 0 -f mysock  -c tests/files/test1,tests/files/test2

kill -s SIGINT ${SERVER_PID}

sleep 1
exit 0