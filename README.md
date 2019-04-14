# Lbeacon-Gateway

The LBeacon Gateway is used for managing LBeacons in a Star Network and connecting them with the BeDIS Server for the purposes of  maintening and monitoring the health of the LBeacons and object tracking by the BeDIS Server.

## General Installation

### Installing OS and Update on Raspberry Pi

Download [Raspbian Jessie Lite](http://downloads.raspberrypi.org/raspbian/images/raspbian-2017-07-05/2017-07-05-raspbian-jessie.zip) for the Raspberry Pi and follow its [installation guide](https://www.raspberrypi.org/documentation/installation/installing-images/README.md).

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

In file `./Lbacon-Gateway/config/gateway.conf` change the following configuration.
* `IP address` to current pi wlan0 IP address
* `server_ip` to current Server IP.

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

## How to set up a config
* When setting any config, no space allow before or afeter "=", e.g. `is_polled_by_server=0`

Name                            | Description
--------------------------------|--------------------------------------------------------------------------------------
is_polled_by_server             | If tracked object data and Health reports is polled by the server, please set 1 else 0. 
IP_address                      | The IP address of the server.
allowed_number_of_nodes         | The number of the LBeacon is allowed to join.
period_between_RFHR             | The period of time the Gateway polling health report.<br />(Work in `is_polled_by_server=0`)
period_between_RFTOD            | The period of time the Gateway polling tracked object data.<br />(Work in `is_polled_by_server=0`)
period_between_join_request     | The period of time the Gateway join the Server.<br />(Work in `is_polled_by_server=0`)
number_worker_thread            | The number of worker threads is allowed to use. 
server_ip                       | The Server IP address.
send_port                       | The port which the Server is prepared to receive messages.
recv_port                       | The port which the Gateway is prepare to receive messages.
critical_priority               | The priority nice for critical priority.
high_priority                   | The priority nice for higher priority.
normal_priority                 | The priority nice for normal priority.
low_priority                    | The priority nice for lower priority.
