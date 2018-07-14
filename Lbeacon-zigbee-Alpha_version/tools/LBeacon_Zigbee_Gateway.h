/*
 * Copyright (c) 2016 Academia Sinica, Institute of Information Science
 *
 * License:
 *
 *      GPL 3.0 : The content of this file is subject to the terms and
 *      cnditions defined in file 'COPYING.txt', which is part of this
 *      source code package.
 *
 * Project Name:
 *
 *      BeDIPS
 *
 * File Description:
 *
 *   	This file contains the program to set up a star network by XBEE S2C
 *      module in order to deal with NSI(Network Setup and Initialization)
 *      and Data transmission between Gateway and LBeacon. And This file is
 *		the header for Gateway.
 *
 * File Name:
 *
 *      LBeacon_Zigbee_Gateway.c
 *
 * Abstract:
 *
 *      BeDIPS uses LBeacons to deliver 3D coordinates and textual
 *      descriptions of their locations to users' devices. Basically, a
 *      LBeacon is an inexpensive, Bluetooth Smart Ready device. The 3D
 *      coordinates and location description of every LBeacon are retrieved
 *      from BeDIS (Building/environment Data and Information System) and
 *      stored locally during deployment and maintenance times. Once
 *      initialized, each LBeacon broadcasts its coordinates and location
 *      description to Bluetooth enabled user devices within its coverage
 *      area.
 *
 * Authors:
 *      Gary Xiao		, garyh0205@hotmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xbee.h>

struct xbee *xbee;

struct xbee_con *con;

/* The address stored the destination MAC of xbee */
struct xbee_conAddress address;

/* The setting for xbee */
struct xbee_conSettings settings;

/* A variable to get error code */
xbee_err ret;

/* A variable txRet get Tx return value */
unsigned char txRet;

const char *xbee_mode = "xbeeZB";

char *xbee_device = "/dev/ttyAMA0";

int xbee_baudrate = 9600;

//A 64-bit extended PAN ID for join Network
char *PAN_ID = "0000000000000000";

//0:disable Log, 100:enable Log
int LogLevel = 100;

// A flag to indicate if all part of address are get. 
// 3 send first address 
// 2 send second address 
// 0 success

enum{finish,wait_SL,wait_SH,start};
int get_address = start;

char* Local_Address = "";

/* packet format in the Queue */
struct pkt { 
	
	//"Data"
	char *type;
	
	// Brocast:     000000000000FFFF;
	// Coordinator: 0000000000000000
	char *address;
	
	// Data
	char *content;          
    	struct pkt *next; 
}; 

typedef struct pkt pPkt;

// front point to the first of thr Pkt Queue
// rear  point to the end of the Pkt Queue
pPkt *front, *rear;

/* Connector for setup xbee connection 
 *
 * conMode:
 *    "Data"        : Send and Receive Data
 *    "GetAddress"  : Get Local Address
 */
xbee_err xbee_connector(struct xbee **xbee,	struct xbee_con **con
, char *conMode);

/* Create Packet Queue Header */
void init_Packet_Queue();

/* Add new Packet to the end of Queue */ 
void addpkt(char *raw_addr, char *type,char *content);

/* Delete the end of Queue */ 
void delq();

/* Fill the address from raw(char) to addr(Hex) */
void Fill_Address(char *raw,unsigned char addr[8]);

/* Copy raw(char) to dest(char) for addpkt */
void AddressCopy(char *raw,char *dest,int size);

/* CallBack for Data Received */
void CallBack(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt
, void **data);

/* CallBack for Get Local Address */
void CallBack_for_get_address(struct xbee *xbee, struct xbee_con *con
, struct xbee_pkt **pkt, void **data);
