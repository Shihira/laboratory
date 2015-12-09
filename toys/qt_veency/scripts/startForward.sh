#!/bin/bash
./closeForward 2>/dev/null
nohup iproxy 15900 5900 > /tmp/iproxy-veency.log &
echo $! > /tmp/iproxy-veency.pid
