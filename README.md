# Lbeacon-Gateway

The LBeacon Gateway is used for managing LBeacons in a Star Network and connecting them with the BeDIS Server for the purposes of  maintening and monitoring the health of the LBeacons and object tracking by the BeDIS Server.

## General Installation

### Installing OS and Update on Raspberry Pi

Download [Raspbian Jessie Lite](http://downloads.raspberrypi.org/raspbian_lite/images/) for the Raspberry Pi and follow its [installation guide](https://www.raspberrypi.org/documentation/installation/installing-images/README.md).

In Raspberry Pi, install packages by running the following command:
```sh
sudo apt-get update
sudo apt-get dist-upgrade -y
sudo apt-get install git make build-essential
```

### Change Username

To avoid path issues, Change username "pi" to "bedis"

### Installing Gateway

Clone code and related packages from Github
```sh
git clone https://github.com/OpenISDM/libEncrypt.git
git clone https://github.com/OpenISDM/Lbeacon-Gateway.git
git checkout "wifi"
```

Install libEncrypt and move the file to the specified path
```sh
mv ./libEncrypt/key/Key_example.h ./libEncrypt/key/Key.h
cd ./libEncrypt/src/
make
cd
mkdir bot-encrypt
mv ./libEncrypt/src/libEncrypt.so ./bot-encrypt/
```

Add the path of the dynamic library

In file `/etc/ld.so.conf` add a new line `/home/bedis/bot-encrypt/`
```sh
sudo ldconfig -v
```

Entering to Lbeacon-Gateway and use make to start installation
```sh
cd ./Lbeacon-Gateway
sudo make all
```
## Configuration

In file `./Lbacon-Gateway/config/gateway.conf` change the following configuration.
* `IP address` to current pi wlan0 IP address
* `server_ip` to current Server IP.

After finishing all step, add the following line to `/etc/rc.local`.
```sh
/home/bedis/Lbeacon-Gateway/bin/Gateway.out > /dev/null 2>&1 &
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
