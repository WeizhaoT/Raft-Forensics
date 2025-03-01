#!/bin/bash
set -e

PORT=33445

# workdir="/data/debug"

if [ $# -lt 4 ]; then
    freq=4
    payload=16
    complexity=0
    printf "setting default params\n\tfreq: \t\t%s\n\tpayload: \t%s\n" $freq $payload
else
    freq=$1
    payload=$2
    complexity=$3
    # workdir="raft-lv$complexity-$payload/$freq-$4"
fi

sleeper=2
tail=2
gap=20
cthreads=1
duration=10
parallel=1
log_level=6

ips=""

exec=$HOME/raft/build/tests/raft_bench
dir=$HOME/raft/build

rm -f "$dir"/*.log
rm -f "$dir"/dump*.txt

for node in {2..3}; do
    PORTF=$((PORT + node))
    tmux send-keys -t "raft:$node" "$exec $node 127.0.0.1:$PORTF $((duration + sleeper + tail)) $complexity $log_level $dir" Enter
    ips="$ips 127.0.0.1:$PORTF"
done

sleep $sleeper

# tmux send-keys -t "raft:1" "valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=$dir/valgrind-dump.txt $exec 1 127.0.0.1:$PORT \
# $duration $complexity $log_level $dir $freq $cthreads $parallel $payload $ips" Enter
tmux send-keys -t "raft:1" "$exec 1 127.0.0.1:$PORT \
$duration $complexity $log_level $dir $freq $cthreads $parallel $payload $ips" Enter

sleep $duration
sleep $tail

# for node in {2..3}; do
#     tmux send-keys -t "raft:$node" C-c
# done
