#!/bin/bash

echo "Upgrade configuration file"
config_new="/home/bedis/Lbeacon-Gateway/config/gateway.conf"
config_save="/home/bedis/upgrade-Gateway/gateway_save.conf"
config_result="/home/bedis/Lbeacon-Gateway/config/gateway_result.conf"

sudo cat /dev/null > $config_result

while IFS= read -r line
do
    config_key=`echo $line | cut -d "=" -f 1`
    echo "handle config key: $config_key"    
    
    is_exist=`sudo cat $config_save | grep "$config_key" | wc -l`
    if [ "_$is_exist" = "_1" ] 
    then
        config_value_save=`sudo cat $config_save | grep $config_key | cut -d "=" -f 2`
        echo "$config_key=$config_value_save" >> $config_result
    else
        echo "$line" >> $config_result
    fi
done < "$config_new"

sudo mv $config_result $config_new
