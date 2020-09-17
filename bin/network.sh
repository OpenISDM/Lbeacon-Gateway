#!/bin/bash

#Check network status and Change .conf
server_ip=`sudo cat ../config/gateway.conf |grep "server_ip" | cut -d "=" -f2`
ping $server_ip -c 1
if [ "X$?" != "X0" ]; then
    sudo ifdown --force wlan0
    sudo ifup wlan0
fi
