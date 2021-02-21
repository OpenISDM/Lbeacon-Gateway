#!/bin/bash

#Check network status and Change .conf
default_gateway=`cat /home/bedis/Lbeacon-Gateway/config/gateway.conf | grep "default_gateway" | cut -d '=' -f2`
count_file=/home/bedis/Lbeacon-Gateway/log/error_count
restart_criteria=30
echo "$default_gateway"
ping $default_gateway -c 1 -t 3 &> /dev/null
if [ "X$?" != "X0" ]; 
then
    echo "network not ok"
    if [ -f $count_file ];
    then 
        echo "file ok"
        read current_count < $count_file
        current_count=`expr $current_count + 1`
        echo $current_count > $count_file
        echo "updated current count:"
        echo $current_count 
        echo "retart criteria:"
        echo $restart_criteria
        if [[ "$current_count" -gt "$restart_criteria" ]];
        then
           echo "restart ..."
           sudo ifdown --force wlan0
           sudo ifup wlan0
           echo "done"
        fi 
    else
        echo "no file"
        echo "1" > $count_file
    fi
else
    echo "network ok" 
    echo "0" > $count_file
fi

echo "current network count:"
cat $count_file
