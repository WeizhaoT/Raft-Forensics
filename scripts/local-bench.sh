#!/bin/bash

USER=ubuntu
IPS=("127.0.0.1" "127.0.0.1" "127.0.0.1")
PORT=33445
KEY=~/ctrl/raft-rsa.pem

if [ $# -lt 4 ]; then
    freq=20000
    payload=4096
    complexity=0
    workdir="$HOME/forensics-raft/raft_debug"
    duration=10
    printf "setting default params\n\tfreq: \t\t%s\n\tpayload: \t%s\n" $freq $payload
else
    freq=$1
    payload=$2
    complexity=$3
    workdir="raft-lv$complexity-$payload/$freq-$4"
    duration=20
fi

sleeper=2
tail=8
gap=20
cthreads=1
parallel=500

ips=""
id=2

for IP in "${IPS[@]}"; do
    echo "$id: $IP"
    PORTF=$((PORT + id))
    ssh "$USER@$IP" -i $KEY -o LogLevel=QUIET -t \
        "tmux send-keys -t \"raft:$((id - 1))\" \"~/forensics-raft/build/tests/raft_bench $id 127.0.0.1:$PORTF $((duration + sleeper + gap)) $complexity $workdir\" Enter"
    id=$((id + 1))
    ips="$ips $IP:$PORTF"
    echo "$id"
done

sleep $sleeper

tmux send-keys -t "raft:0" "$HOME/forensics-raft/build/tests/raft_bench 1 127.0.0.1:$PORT \
    $duration $complexity $workdir $freq $cthreads $parallel $payload $ips" Enter

sleep $duration
sleep $tail

id=2
for IP in "${IPS[@]}"; do
    ssh "$USER@$IP" -i $KEY -o LogLevel=QUIET -t "tmux send-keys -t raft:$((id - 1)) C-c"
    id=$((id + 1))
done
