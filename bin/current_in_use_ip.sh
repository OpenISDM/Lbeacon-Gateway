#! /bin/bash

sudo ip -s -s neigh flush all | grep "192.168."
arp -an | grep "192.168"

exit 0
