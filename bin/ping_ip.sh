#!/bin/bash

input="/home/bedis/Lbeacon-Gateway/log/lbeacon_ips"
output="/home/bedis/Lbeacon-Gateway/log/bad_lbeacons"

cat /dev/null > $output
while IFS= read -r line
do
    ping_ret=`echo $line | xargs ping -nc 1`
    if [ "X$ping_ret" != "X0" ]; then
        echo "cannot connect to" $line
        echo $line >> $output
    fi
done < "$input"
