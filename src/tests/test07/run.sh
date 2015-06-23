#!/bin/bash
./server &
sleep 1
SPID=$!
./client
RES=$?
kill -SIGSTOP $SPID
[[ $RES == 0 ]] || exit -1 
exit 0
