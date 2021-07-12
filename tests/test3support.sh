#!/bin/bash
while true
do
./client -f mysock -D tests/fileEvict/ -w tests/files 
./client -f mysock -d tests/fileLetti/ -r ./tests/files/100mb_file,./tests/files/30mb_file
./client -f mysock -d tests/fileLetti/ -l ./tests/files/100mb_file,./tests/files/30mb_file
done