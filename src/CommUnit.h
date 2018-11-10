/*
 Copyright (c) 2016 Academia Sinica, Institute of Information Science

 License:

      GPL 3.0 : The content of this file is subject to the terms and
      cnditions defined in file 'COPYING.txt', which is part of this source
      code package.

 Project Name:

      BeDIPS

 File Description:

       Communication unit: In the alpha version,the sole function of this
       component is to support the communication of NSI and BHM with location
       beacons and the BeDIS server. (In a later version that contains iGaD,
       this components also receives commands from iGaD in the gateway and
       the BeDIS server and broadcasts the commands tolocal LBeacons.)
       Messages exchange happens in CommUnit. This file contain the
       formats of every kind of message and the buffers which store
       messages.And provide with functions which are executed according
       the messages received.

 File Name:

     CommUnit.h

 Abstract:

      BeDIPS uses LBeacons to deliver 3D coordinates and textual
      descriptions of their locations to users' devices. Basically, a
      LBeacon is an inexpensive, Bluetooth Smart Ready device. The 3D
      coordinates and location description of every LBeacon are retrieved
      from BeDIS (Building/environment Data and Information System) and
      stored locally during deployment and maintenance times. Once
      initialized, each LBeacon broadcasts its coordinates and location
      description to Bluetooth enabled user devices within its coverage
      area.

 Authors:

      Han Wang      , hollywang@iis.sinica.edu.tw
      Hank Kung     , hank910140@gmail.com
      Ray Chao      , raychao5566@gmail.com
      Gary Xiao     , garyh0205@hotmail.com

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
＃include "Mempool.h"
#include "xbee_API.h"
#include "UDP_API.h"
#include "LinkedList.h"
#include "thpool.h"

#ifndef COMMUNIT_H
#define COMMUNIT_H

/*
  CONSTANTS
 */
 /* Timeout interval in seconds */
 #define A_LONG_TIME 30000
 #define A_SHORT_TIME 5000
 #define A_VERY_SHORT_TIME 300

#define COORDINATE_LENGTH 64

#define XBEE_MODE "xbeeZB"

#define XBEE_DEVICE "/dev/ttyAMA0"

#define XBEE_CONFIG_PATH "/home/pi/Lbeacon-Gateway/config/xbee_config.conf"

#define MAX_CONTENT_LENGTH 1024


/*
  ErrorCode
*/

typedef enum ErrorCode{

    WORK_SUCCESSFULLY = 0,
    E_MALLOC = 1,
    E_WIFI_CONNECT_FAIL = 2,

    E_ZIGBEE_INIT_FAIL = 3,

    E_XBEE_VALIDATE = 4,

    E_START_COMMUNICAT_ROUTINE_THREAD = 5,
    E_START_BHM_ROUTINE_THREAD = 6,
    E_START_TRACKING_THREAD = 7,
    E_START_THREAD = 8

} ErrorCode;

typedef enum pkt_types {

    tracked_object_data = 1;
    health_report = 2;
    data_for_LBeacon = 3;
    poll_for_tracked_object_data = 9;
    RFHR_to_Lbeacons = 10;
    poll_for_RFHR_from_sever = 11;
    request_to_join = 12;
    zigbee_network_control = 13;
    maximum_type = 15;

} PktType;

/*
  TYPEDEF STRUCTS
*/
typedef struct{

    char X_coordinates[COORDINATE_LENGTH];
    char Y_coordinates[COORDINATE_LENGTH];
    char Z_coordinates[COORDINATE_LENGTH];

}Coordinates;

/*  A struct linking network address assigned to a LBeacon to its UUID,
    coordinates, and location description. */
typedef struct{

    char        beacon_uuid[UUID_LENGTH];

    /* network address of zigbee/wifi link to the LBeacon*/
    char        net_address[Address_length_Hex];

    Coordinates beacon_coordinates;

    char        location_description[MAX_LENGTH_LOC_DESCRIPTION];

}Address_map;

/* A Head of a list of msg buffer */
typedef struct buffer_list_head{

    struct List_Entry buffer_entry;

    /* A per list lock */
    pthread_mutex_t   list_lock;

    /* The index number of the buffer */
    int               buffer_id;

    /* Current number of msg buffers in the list */
    int               num_in_list;

    /* Number of levels relative to normal priority */
    int              priority_boast;

    /* function pointer */
    void             (*function)(void* arg);

    /* function's argument       */
    void             *arg;

    /* A buffer to indicate the buffer is be occupied */
    bool            is_busy;

} BufferListHead;

/* A node of buffer to store received data and/or data to be send */
typedef struct BufferNode{

    struct List_Entry buffer_entry;

    /* Zigbee network address of the source or destination */
    char             net_address[ADDRESS_LENGTH];

    /* point to where the data is stored. */
    char             *content[MAX_CONTENT_LENGTH];


} BufferNode;


/*
  GLOBAL VARIABLES
*/
/* Struct for storing necessary objects for zigbee connection */
sxbee_config xbee_config;

/* Struct for storing necessary objects for Wifi connection */
sudp_config udp_config;

/* mempool of node for Gateway */
Memory_Pool node_mempool;

/* An array of address maps */
Address_map Lbeacon_addresses[MAX_NUMBER_NODES];



/*
  init_buffer:

      The function fills the attributes of buffer storing the packets.
      Including assigning the function to the corresponding buffer list and its
      arguments and its priority level

  Parameters:

      buffer - A pointer of the buffer to be modified.
      buff_id - The index of the buffer for the priority array
      function - A function pointer to be assigned to the buffer
      priority - The priority level of the buffer

  Return value:

      None

 */
void init_buffer(BufferListHead *buffer, int buff_id, void (*function_p)(void*),
								int priority_boast);


/*
 wifi_recieve:

     This function listens the request or command received from the server.
     After getting the message, push the data in to the buffer.

 Parameters:

     buffer - A pointer of the buffer to be modified.

 Return value:

     None
 */
void *wifi_receive(BufferListHead *buffer);

/*
 wifi_send:

     This function sends the file to the sever via Wi-Fi.

 Parameters:

     buffer_array - An array of buffer to be sent.

 Return value:

     None
 */
void *wifi_send(BufferListHead *buffer_array);

/*
 zigbee_receive:

     This function listens the data received from the Beacon.
     After getting the message, push the data in to the buffer.

 Parameters:

     buffer_array - An array of buffer to be sent.

 Return value:

     None
 */
void *zigbee_receive(BufferListHead *buffer_array);

/*
 zigbee_send:

     This function sends the data to the Beacon via Zigbee.

 Parameters:

     buffer - A pointer of the buffer to be modified.

 Return value:

     None
 */
void *zigbee_send(BufferListHead *buffer);

/*
 beacon_join_request:

     This function is executed when a beacon sends command to join the gateway
     and fills the table with the inputs. Set the network_address according
     the current number of beacons.

 Parameters:

     ID - The UUIzd of the LBeacon
     mac - The mac address of the xbee
     Coordinates - Pointerto the beacon coordinates
     Loc_Description - Pointer to the beacon literal location description

 Return value:

     None

 */
void beacon_join_request(char *ID, char *mac, Coordinates Beacon_Coordinates,
                         char *Loc_Description);

/*
 zigbee_init:

     This function initilizes the zigbee's necessory object.

 Parameters:

     zigbee - the struct of necessary parameter and data

 Return value:

     int: The error code for the corresponding error or successful

 */
int zigbee_init();

/*
 zigbee_free:

     When called, this function frees the necessory element.

 Parameters:

     zigbee - the struct of necessary parameter and data

 Return value:

     None

 */
void zigbee_free();

#endif
