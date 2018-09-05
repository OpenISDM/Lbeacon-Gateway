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
#include <arpa/inet.h>
#include <unistd.h>
#include "xbee_API.h"

#ifndef CommUnit_H
#define CommUnit_H

/* 
* CONSTANTS 
*/
#define A_SHORT_TIME 1000
#define A_LONG_TIME 5000





/* server IP address */
#define SERVER "140.114.71.26"

/* Gateway IP address*/
#define PORT 3306


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

/*
*  beacon_join_request:
*  This function is executed when a beacon sends command to join the gateway
*  and fills the table with the inputs. Set the network_address according
*  the current number of beacons.
*
*  Parameters:
*
*  index - index of the address map table
*  *ID - 
*  *Coordinates - Pointerto the beacon GPS coordinates 
*  *Loc_Description - Pointer to the beacon literal location description
*  *Barcode - Pointer to the beacon Barcode 
*
*  Return value:
*
*  None
*/
void beacon_join_request(char *ID, char *mac, Coordinates Beacon_Coordinates,
                         char *Loc_Description);



#endif