#!/bin/bash

echo "upgarde configuration file"
sudo chmod 755 /home/bedis/Lbeacon-Gateway/bin/upgrade_gateway_config.sh
sudo /home/bedis/Lbeacon-Gateway/bin/upgrade_gateway_config.sh

echo "upgrate other things"

echo "start process"
cd /home/bedis/Lbeacon-Gateway/bin/
sudo chmod 755 /home/bedis/Lbeacon-Gateway/bin/auto_Gateway.sh
sudo /home/bedis/Lbeacon-Gateway/bin/auto_Gateway.sh
