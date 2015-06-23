#!/bin/bash
./server &
sleep 1
SPID=$!

for (( i=0 ; i<10 ; i++ ))
do
./client
RES=$?
echo client $i done.
[[ $RES == "0" ]] || exit $RES
done

kill -SIGSTOP $SPID
exit 0
