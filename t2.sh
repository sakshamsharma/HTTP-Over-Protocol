#!/bin/zsh

MINN=2000
serverport=0

while [ "$serverport" -le $MINN ]
do
  serverport=$RANDOM
  echo "Server on: $serverport"
done

rm -f filer
./hop 8084 localhost $serverport SERVER &
sleep 0.2
nc -l $serverport >! filer &
sleep 0.2
cat httpabc | nc localhost 8084
pkill hop
pkill '^nc'
wc -l filer
wc -l httpabc
