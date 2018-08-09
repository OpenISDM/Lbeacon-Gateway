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
#include "../Lbeacon-zigbee-Alpha_version/src/xbee_API.h"

#ifndef CommUnit_H
#define CommUnit_H

#define A_SHORT_TIME 1000
#define A_LONG_TIME 5000

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
  ERROR CODE
*/
typedef enum ErrorCode {

    WORK_SCUCESSFULLY = 0,
    E_MALLOC = 1,
    E_OPEN_FILE = 2,
    E_OPEN_DEVICE = 3,
    E_OPEN_SOCKET = 4,
    E_SEND_OBEXFTP_CLIENT = 5,
    E_SEND_CONNECT_DEVICE = 6,
    E_SEND_PUT_FILE = 7,
    E_SEND_DISCONNECT_CLIENT = 8,
    E_SCAN_SET_HCI_FILTER = 9,
    E_SCAN_SET_INQUIRY_MODE = 10,
    E_SCAN_START_INQUIRY = 11,
    E_SEND_REQUEST_TIMEOUT = 12,
    E_ADVERTISE_STATUS = 13,
    E_ADVERTISE_MODE = 14,
    E_START_THREAD = 15,
    E_INIT_THREAD_POOL = 16,
    E_INIT_ZIGBEE = 17,
    E_ZIGBEE_CONNECT = 18,
    MAX_ERROR_CODE = 19

} ErrorCode;

ErrorCode return_error_value;

struct _errordesc {
    int code;
    char *message;
} errordesc[] = {

    {WORK_SCUCESSFULLY, "The code works successfullly"},
    {E_MALLOC, "Error allocating memory"},
    {E_OPEN_FILE, "Error opening file"},
    {E_OPEN_DEVICE, "Error opening the dvice"},
    {E_OPEN_SOCKET, "Error opening socket"},
    {E_SEND_OBEXFTP_CLIENT, "Error opening obexftp client"},
    {E_SEND_CONNECT_DEVICE, "Error connecting to obexftp device"},
    {E_SEND_PUT_FILE, "Error putting file"},
    {E_SEND_DISCONNECT_CLIENT, "Disconnecting the client"},
    {E_SCAN_SET_HCI_FILTER, "Error setting HCI filter"},
    {E_SCAN_SET_INQUIRY_MODE, "Error settnig inquiry mode"},
    {E_SCAN_START_INQUIRY, "Error starting inquiry"},
    {E_SEND_REQUEST_TIMEOUT, "Timeout for sending request"},
    {E_ADVERTISE_STATUS, "LE set advertise returned status"},
    {E_ADVERTISE_MODE, "Error setting advertise mode"},
    {E_START_THREAD, "Error creating thread"},
    {E_INIT_THREAD_POOL, "Error initializing thread pool"},
    {E_INIT_ZIGBEE, "Error initializing the zigbee"},
    {E_ZIGBEE_CONNECT, "Error zigbee connection"},
    {MAX_ERROR_CODE, "The element is invalid"}

};

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
extern bool zigbee_is_ready;
extern bool wifi_is_ready;

extern struct sockaddr_in si_other;
extern int s, i, slen;

extern pkt_ptr pkt_queue;
extern struct xbee *xbee;
extern struct xbee_con *con;

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