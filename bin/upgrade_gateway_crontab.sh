#!/bin/bash

echo "check newer package"
is_gateway_package=`sudo ls -t /tmp/Lbeacon-Gateway-*.tar.gz | head -1 | wc -l`

if [ "_$is_gateway_package" = "_1" ]
then
    echo "has pakcage, checking version"

    version_info=`sudo ls -tr /home/bedis/Lbeacon-Gateway/*.txt | head -1 | cut -d "/" -f 5 | cut -d "." -f 1-2`
    build_info=`sudo ls -tr /home/bedis/Lbeacon-Gateway/*.txt | head -1 | cut -d "/" -f 5 | cut -d "." -f 3`
    
    new_version_info=`sudo ls -t /tmp/Lbeacon-Gateway-*.tar.gz | head -1 | cut -d "-" -f 3 | cut -d "." -f 1-2`
    new_build_info=`sudo ls -t /tmp/Lbeacon-Gateway-*.tar.gz | head -1 | cut -d "-" -f 3 | cut -d "." -f 3`
    
    echo "$version_info $build_info"
    echo "$new_version_info $new_build_info"
 
    if [ "_$version_info" = "_$new_version_info" ] && [ "$new_build_info" -gt "$build_info" ]
    then 
        echo "continue to apply newer package"
    else
        echo "no newer package"
        exit 0
    fi
else
    echo "no package"
    exit 0
fi

echo "stop running process"
cd /home/bedis/Lbeacon-Gateway/bin/
sudo chmod 755 /home/bedis/Lbeacon-Gateway/bin/kill_Gateway.sh
sudo /home/bedis/Lbeacon-Gateway/bin/kill_Gateway.sh

echo "backup existing configration file"
sudo cp /home/bedis/Lbeacon-Gateway/config/gateway.conf /home/bedis/upgrade-Gateway/gateway_save.conf 

echo "backup existing self_check.sh"
sudo cp /home/bedis/Lbeacon-Gateway/bin/self_check.sh /home/bedis/upgrade-Gateway/self_check.sh.bak

echo "remove existing version files"
sudo rm -f /home/bedis/Lbeacon-Gateway/*.txt

echo "upgrade package"
filename=`sudo ls -t /tmp/Lbeacon-Gateway-*.tar.gz | head -1 | cut -d "/" -f 3-`
sudo cp /tmp/$filename /home/bedis/$filename
cd /home/bedis
sudo tar zxvf $filename

echo "trigger upgrade program inside newer package"
sudo chmod 755 /home/bedis/Lbeacon-Gateway/bin/upgrade_gateway.sh
sudo /home/bedis/Lbeacon-Gateway/bin/upgrade_gateway.sh

echo "leave an upgrade record"
upgraded_info=`sudo ls -tr /home/bedis/Lbeacon-Gateway/*.txt | head -1`
now=`date`
echo "$now - $upgraded_info" >> /home/bedis/Lbeacon-Gateway/upgrade_history
