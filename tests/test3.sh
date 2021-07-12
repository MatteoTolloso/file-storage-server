#!/bin/bash

./server tests/config3.txt &
SERVER_PID=$!
export SERVER_PID
bash -c 'sleep 5 && kill -s SIGINT ${SERVER_PID}' &

pids=()
for i in {1..10}; do
    bash -c './tests/test3support.sh' &
    pids+=($!)
    sleep 0.1
done

sleep 3

for i in "${pids[@]}"; do
    kill -9 ${i}
    wait ${i}
done

wait ${SERVER_PID}
exit 0