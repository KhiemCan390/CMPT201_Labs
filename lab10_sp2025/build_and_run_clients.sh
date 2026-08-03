#!/bin/bash

cd build
make
./client &
./client &
./client &
./client
# run 4 clients simultaneously, with 1 client in the foreground and 3 in the background (having the
# & following it)
wait
