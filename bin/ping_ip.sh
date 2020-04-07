#!/bin/bash

input="/home/bedis/Lbeacon-Gateway/log/active_lbeacon_list"
output="/home/bedis/Lbeacon-Gateway/log/abnormal_lbeacon_list"

sudo cat /dev/null > $output
while IFS= read -r line
do
    echo "try to ping $line"
    ping_ret=`ping $line -nc 1`
    if [ "X$?" != "X0" ]; then
        echo "cannot connect to" $line
        echo $line >> $output
    fi
done < "$input"
