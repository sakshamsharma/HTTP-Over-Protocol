#!/bin/zsh

MINN=2000
serverport=0

while [ "$serverport" -le $MINN ]
do
  serverport=$RANDOM
  echo "Server on: $serverport"
done

rm myfile
./hop 8084 localhost $serverport &
sleep 0.2
nc -l $serverport >! myfile&
sleep 0.2
cat abc | nc localhost 8084
pkill hop
pkill '^nc'
wc -l myfile
wc -l abc
