#!/bin/bash
./server &
sleep 1
SPID=$!
./client
RES=$?
kill -SIGSTOP $SPID
exit $RES
