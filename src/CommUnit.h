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

#define A_SHORT_TIME 1000
#define A_LONG_TIME 5000
#define BUFFER_SIZE 1024

#define RFTF "@, Request-for-tracking-file"
#define RFHR "+, Request-for-health-report"
/*
* ENUM
*/

typedef enum commandrequest {
    /* Request for health report */
    HEALTH_REPORT = 0,
    /**/
    BEACON_JOIN_REQUEST = 1,
    /* Server web sends coordinates of the beacon */
    SET_BEACON_COORDINATES = 2,
    /**/
    REMOVE_BEACON = 3,
    /**/
    GET_BEACON_INFO = 4
}CommandRequest;

/*
* STRUCT
*/

/* */
typedef struct buffer{
    char *name;
    FILE content[BUFFER_SIZE];
    int front;
    int rear;
    bool is_locked;
    bool is_empty;
}Buffer;

/*
*   Variables
*/
Buffer sendToBeacon, recieveFromBeacon;
Buffer sendToServer, recieveFromServer;

/*
*   External Variables
*/ 

extern bool CommUnit_initialization_complete;
extern bool system_is_shutting_down;

/*
*  CommUnit_routine:
*/
void *CommUnit_routine();

/*
*  init_buffer:
*
*
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
*
*/
void *buffer_dequeue(Buffer buffer);

/*
*
*/
void buffer_enqueue(Buffer buffer, FILE *item);

/*
*
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
void *wifi_reciever();

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
void *wifi_sender();

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
void *zigbee_receiver();

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
void *zigbee_sender();