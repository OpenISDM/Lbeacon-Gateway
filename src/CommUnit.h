/*
* Copyright (c) 2016 Academia Sinica, Institute of Information Science
*
* License:
*
*      GPL 3.0 : The content of this file is subject to the terms and
*      cnditions defined in file 'COPYING.txt', which is part of this source
*      code package.
*
* Project Name:
*
*      BeDIPS
*
* File Description:
*
*       Communication unit: In the alpha version,the sole function of this 
*       component is to support the communication of NSI and BHM with location 
*       beacons and the BeDIS server. (In a later version that contains iGaD,
*       this components also receives commands from iGaD in the gateway and 
*       the BeDIS server and broadcasts the commands tolocal LBeacons.) 
*       Messages exchange happens in CommUnit. This file contain the 
*       formats of every kind of message and the buffers which store 
*       messages.And provide with functions which are executed according 
*       the messages received.
*       
* File Name:
*
*     CommUnit.h
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
*
*      Hank Kung, hank910140@gmail.com
*      
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include "xbee_API.h"

#ifndef CommUnit_H
#define CommUnit_H

/* 
* CONSTANTS 
*/
#define A_SHORT_TIME 1000
#define A_LONG_TIME 5000

/* Maximum number of nodes (LBeacons) per star network */
#define MAX_NUMBER_NODES 32

/*Length of the beacon's UUID*/
#define UUID_LENGTH 32

/*Length of the address of the network */
#define NETWORK_ADD_LENGTH 16

/* Maximum number of characters in location description*/
#define MAX_LENGTH_LOC_DESCRIPTION  64

#define COORDINATE_LENGTH 64

#define BARCODE_SIZE 64


/* A buffer head for receiving and getting content from LBeacon or server */
typedef struct buffer{
    struct List_Entry sc_list_entry;
    bool is_locked;
    bool is_empty;
}BufferHead;

/*
*   Variables
*/
BufferHead sendToBeacon, recieveFromBeacon;
BufferHead sendToServer, recieveFromServer;



/*
*  init_buffer:
*
*  The function fills the attributes of buffer storing the packets between
*  gateway and server.
*
*  Parameters:
*
*  Node
*
*  Return value:
*
*  None
*/
void init_buffer(Buffer buffer);

/*
*  buffer_dequeue:
*
*
*
*  Parameters:
*
*  buffer - 
*
*  Return value:
*
*  None
*/
void *buffer_dequeue(Buffer buffer);

/*
*  buffer_enqueue:
*
*
*
*  Parameters:
*
*  buffer -
*  *item -
*
*  Return value:
*
*  None
*/
void buffer_enqueue(Buffer buffer, FILE *item);

/*
*  is_buffer_empty:
*
*
*
*  Parameters:
*
*  buffer - 
*
*  Return value:
*
*  None
*/
bool is_buffer_empty(Buffer buffer);

/*
*  RFHR
*
*  Request For Health Report. This function scans each beacon in the gateway by
*  sending Zigbee singal to detect. If times to scan beacon successfully equals
*  the total beacon number then break.
*
*  Parameters:
*
*  Node
*
*  Return value:
*
*  None
*/
void RFHR();

/*
*  wifi_reciever:
*
*
*  Parameters:
*
*  None
*
*  Return value:
*
*  O
*/
void *wifi_receiver(Buffer buf);

/*
*  wifi_sender:
*
*
*  Parameters:
*
*  None
*
*  Return value:
*
*  O
*/
void *wifi_sender(Buffer buffer);

/*
*  zigbee_receiver:
*
*
*  Parameters:
*
*  None
*
*  Return value:
*
*  O
*/
void *zigbee_receiver(Buffer buffer);

/*
*  zigbee_sender:
*
*
*  Parameters:
*
*  None
*
*  Return value:
*
*  O
*/
void *zigbee_sender(Buffer buffer);


void generate_command(const char *command, const char *type);

#endif