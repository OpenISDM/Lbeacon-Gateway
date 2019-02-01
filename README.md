# Lbeacon-Gateway

LBeacon-Gateway: The Lbeacon Gateway is used for managing Lbeacons in a Star Network and connecting them with the BeDIS Server for the purposes of downloading files to the Location Beacons during reconfiguration and maintenance and monitoring the health of the Beacons by the BeDIS Server.

## General Installation

### Installing OS and Update on Raspberry Pi

[Download](https://www.raspberrypi.org/downloads/raspbian/) Raspbian Jessie lite for the Raspberry Pi and follow its installation guide.

In Raspberry Pi, install packages by running the following command:
```sh
sudo apt-get update
sudo apt-get dist-upgrade -y
sudo apt-get install git make build-essential
```

### Installing Gateway

Clone code from Github
```sh
git clone https://github.com/OpenISDM/Lbeacon-Gateway.git
```

Entering to Lbeacon-Gateway and use make to start installation
```sh
cd ./Lbeacon-Gateway
sudo make all
```
