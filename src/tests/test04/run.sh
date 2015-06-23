#!/bin/bash
./server &
sleep 1
SPID=$!

declare -a CPID
for (( i=0 ; i<10 ; i++ ))
do
./client &
CPID[$i]=$!
echo ${CPID[$i]}
done

fail=0;
for(( i=0 ; i<10 ; i++ ))
do
    wait ${CPID[$i]} || { echo Client ${CPID[$i]} failed; let "fail++"; }
    echo ${CPID[$i]} finished
done
kill -SIGSTOP $SPID
[[ $fail == 0 ]] || exit -1 
exit 0
