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
#include "xbee_API.h"
#include "LinkedList.h"

#ifndef CommUnit_H
#define CommUnit_H

/*
* CONSTANTS
*/
#define A_SHORT_TIME 1000
#define A_LONG_TIME 5000

#define COORDINATE_LENGTH 64

/* server IP address */
//#define SERVER "140.114.71.26"

/* Gateway IP address*/
//#define PORT 3306

#define XBEE_MODE "xbeeZB"

#define XBEE_DEVICE "/dev/ttyAMA0"

#define XBEE_DATASTREAM -1

#define XBEE_CONFIG_PATH "/home/pi/Lbeacon-Gateway/config/xbee_config.conf"

/* Struct for storing necessary objects for zigbee connection */
sxbee_config xbee_config;

/* ErrorCode */

typedef enum ErrorCode{

    WORK_SUCCESSFULLY = 0,
    E_WIFI_CONNECT_FAIL = 1,
    E_ZIGBEE_INIT_FAIL = 2,
    E_XBEE_VALIDATE = 3,
    E_START_COMMUNICAT_ROUTINE_THREAD = 4,
    E_START_BHM_ROUTINE_THREAD = 5,
    E_START_TRACKING_THREAD = 6,
    E_START_THREAD = 7

} ErrorCode;

//struct errdesc {
//    int code;
//    char *message;
//} errordesc[] = {
//
//    {WORK_SUCCESSFULLY, "The code works successfullly"},
//    //{E_MALLOC, "Error allocating memory"},
//    //{E_OPEN_FILE, "Error opening file"},
//    //{E_OPEN_DEVICE, "Error opening the dvice"},
//    //{E_OPEN_SOCKET, "Error opening socket"},
//    //{E_SEND_OBEXFTP_CLIENT, "Error opening obexftp client"},
//    //{E_SEND_CONNECT_DEVICE, "Error connecting to obexftp device"},
//    //{E_SEND_PUSH_FILE, "Error pushing file to device"},
//    //{E_SEND_DISCONNECT_CLIENT, "Disconnecting the client"},
//    //{E_SCAN_SET_HCI_FILTER, "Error setting HCI filter"},
//    //{E_SCAN_SET_INQUIRY_MODE, "Error settnig inquiry mode"},
//    //{E_SCAN_START_INQUIRY, "Error starting inquiry"},
//    //{E_SEND_REQUEST_TIMEOUT, "Sending request timeout"},
//    //{E_ADVERTISE_STATUS, "LE set advertise returned status"},
//    //{E_ADVERTISE_MODE, "Error setting advertise mode"},
//    {E_START_THREAD, "Error start thread"},
//    {E_START_COMMUNICAT_ROUTINE_THREAD, "Error start CommUnit reoutine thread"},
//    {E_START_BHM_ROUTINE_THREAD, "Error start BHM routine thread"},
//    {E_START_TRACKING_THREAD, "Error start Tracking thread"},
//    //{E_INIT_THREAD_POOL, "Error initializing thread pool"},
//    //{E_INIT_ZIGBEE, "Error initializing the zigbee"},
//    //{E_ZIGBEE_CONNECT, "Error zigbee connection"},
//    //{E_EMPTY_FILE, "Empty file"},
//    //{E_ADD_WORK_THREAD, "Error adding work to the work thread"},
//    //{MAX_ERROR_CODE, "The element is invalid"}
//
//};

/* A buffer head for receiving and getting content from LBeacon or server */
typedef struct buffer_list_head{
    List_Entry buffer_entry;
    bool is_locked;
    bool is_empty;
} BufferListHead;

/* A node of buffer to store received data. Each node has its mac address of
   source Beacon and the content */
typedef struct BufferNode{

    struct List_Entry buffer_entry;
    char *mac_addr;
    char *content;

} BufferNode;

typedef struct{

  char X_coordinates[COORDINATE_LENGTH];
  char Y_coordinates[COORDINATE_LENGTH];
  char Z_coordinates[COORDINATE_LENGTH];

}Coordinates;

/*
   VARIBLES
*/

/* A buffer for storing the command message received from the sever. */
//BufferListHead receiveFromServer;

/* Two buffer lists storing the track data or health report received
   from the LBeacons. The content in this buffer is waiting to be
   sent to the sever */
//BufferListHead buffer_track_list, buffer_health_list;

/*
  init_buffer:

  The function fills the attributes of buffer storing the packets between
  gateway and server.

  Parameters:

  Node

  Return value:

  None
*/
void init_buffer(BufferListHead buffer);

/*
  wifi_reciever:

  This function listens the request or command received from the
  sever. After getting the message, push the data in to the buffer.

  Parameters:

  None

  Return value:

    None
*/
//void *wifi_receiver(BufferListHead buf);

/*
  wifi_sender:

  This function sends the file to the sever via wifi

  Parameters:

    file_name - the file name that is assigned to be sent to the sever

  Return value:

    None
*/
//void sned_via_wifi(char *file_name);


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
//void beacon_join_request(char *ID, char *mac, Coordinates Beacon_Coordinates,
//                         char *Loc_Description);

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

    none
*/
void zigbee_free();

#endif
