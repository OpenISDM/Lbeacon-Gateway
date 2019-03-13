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
git checkout "wifi"
```

Entering to Lbeacon-Gateway and use make to start installation
```sh
cd ./Lbeacon-Gateway
sudo make all
```
## Configuration

In file `./Lbacon-Gateway/config/gateway.conf` change `IP address` to current pi wlan0 IP address and `server_ip` to current Server IP.

After finishing all step, add the following line to `/etc/rc.local`.
```sh
/home/pi/Lbeacon-Gateway/bin/Gateway.out > /dev/null 2>&1 &
```

## How to check whether Gateway is running

When need to know Gateway is runnging or not, use the following command to check.
```sh
ps -aux | grep Gateway
```
If Gateway is running, the result looks like the following.
```sh
root      1286 99.2  4.3 217532 19364 ?        Sl   Mar1 1169:28 /home/pi/Lbeacon-Gateway/bin/Gateway.out
```
