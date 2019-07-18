/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and conditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

  File Name:

     Gateway.h

  File Description:

     This is the header file containing the declarations of functions and
     variables used in the Gateway.c file.

  Version:

     1.0, 20190306

  Abstract:

     BeDIS uses LBeacons to deliver 3D coordinates and textual descriptions of
     their locations to users' devices. Basically, a LBeacon is an inexpensive,
     Bluetooth Smart Ready device. The 3D coordinates and location description
     of every LBeacon are retrieved from BeDIS (Building/environment Data and
     Information System) and stored locally during deployment and maintenance
     times. Once initialized, each LBeacon broadcasts its coordinates and
     location description to Bluetooth enabled user devices within its coverage
     area.

 Authors:

     Holly Wang   , hollywang@iis.sinica.edu.tw
     Ray Chao     , raychao5566@gmail.com
     Gary Xiao    , garyh0205@hotmail.com
     Chun Yu Lai  , chunyu1202@gmail.com

 */

#ifndef GATEWAY_H
#define GATEWAY_H

#define _GNU_SOURCE

#include "BeDIS.h"

/* Enable debugging mode. */
#define debugging


/* Gateway config file location and the config file definition. */

/* File path of the config file of the Gateway */
#define CONFIG_FILE_NAME "/home/pi/Lbeacon-Gateway/config/gateway.conf"

/* File path of the config file of the zlog */
#define ZLOG_CONFIG_FILE_NAME "/home/pi/Lbeacon-Gateway/config/zlog.conf"


/* Global variables */

/* An array of address maps */
AddressMapArray LBeacon_address_map;

/* The head of a list of buffers for polling messages and commands */
BufferListHead command_msg_buffer_list_head;

/* The head of a list of buffers for time critical messages */
BufferListHead time_critical_LBeacon_receive_buffer_list_head;

/* Variables for storing the last polling times in second*/
int last_polling_LBeacon_for_HR_time;
int last_polling_object_tracking_time;
int last_polling_join_request_time;



/*
  get_gateway_config:

     This function reads the specified config file line by line until the
     end of file and copies the data in each line into an element of the
     GatewayConfig struct global variable.

  Parameters:

     file_name - the name of the config file that stores gateway data

  Return value:

     config - GatewayConfig struct
 */
ErrorCode get_gateway_config(GatewayConfig *config, char *file_name);


/*
  Gateway_NSI_routine:

     This function is executed by worker threads when they process the buffer
     nodes in NSI receive buffer list.

  Parameters:

     _buffer_list_head - A pointer of the buffer list containing buffers to be
                         modified.

  Return value:

     None

 */
void *Gateway_NSI_routine(void *_buffer_node);


/*
  Gateway_BHM_routine:

     This function is executed by worker threads when they process the buffer
     nodes in BHM_receive_buffer_list.

  Parameters:

     _buffer_list_head - A pointer of the buffer to be modified.

  Return value:

     None

 */
void *Gateway_BHM_routine(void *_buffer_node);


/*
  Gateway_LBeacon_routine:

     This function is executed by worker threads when they remove buffer nodes
     from LBeacon_receive_buffer_list and send them to the server directly.

  Parameters:

     _buffer_list_head - A pointer of the buffer to be modified.

  Return value:

     None

 */
void *Gateway_LBeacon_routine(void *_buffer_node);


/*
  Server_routine:

     This function is executed by worker threads when they process the buffer
     nodes in Command_msg_buffer_list and broadcast to LBeacons.

  Parameters:

     _buffer_list_head - A pointer of the buffer to be modified.

  Return value:

     None

 */
void *Server_routine(void *_buffer_node);


/*
  beacon_join_request:

     This function is executed when a beacon sends a command to join the gateway
     . When executed, it fills the AddressMap with the inputs and sets the
     network_address if the number of beacons already joined the gateway does
     not excceed allowed_number_of_nodes.

  Parameters:

     address_map - The head of the AddressMap.
     uuid - The UUID of the LBeacon
     address - The mac address of the LBeacon IP.
     datetime - The last LBeacon reported datetime

  Return value:

     bool - true  : Join success.
            false : Fail to join

 */
bool beacon_join_request(AddressMapArray *address_map, char *uuid, char *address
                         , int datetime);


/*
  beacon_brocast:

     This function is executed when a command is to be broadcast to LBeacons.
     When called, this function sends msg to all LBeacons registered in the
     LBeacon_address_map.

  Parameters:
     address_map - The head of the AddressMap.
     msg - The pointer to the msg to be send to beacons.
     size - The size of the msg.

  Return value:

     None

 */
void beacon_broadcast(AddressMapArray *address_map, char *msg, int size);


/*
  Wifi_init:

     This function initializes the Wifi's objects.

  Parameters:

     IPaddress - The address of the server.

  Return value:

      ErrorCode - The error code for the corresponding error or successful

 */
ErrorCode Wifi_init(char *IPaddress);


/*
  Wifi_free:

     When called, this function frees the queue of the Wi-Fi pkts and sockets.

  Parameters:

     None

  Return value:

     None

 */
void Wifi_free();


/*
  Gateway_process_wifi_send:

     This function sends the msg in the specified buffer list to the server via
     Wi-Fi.

  Parameters:

     _buffer_list_head - A pointer to the buffer list head.

  Return value:

     None
 */
void *Gateway_process_wifi_send(void *_buffer_node);


/*
  Gateway_process_wifi_receive:

     This function listens for messages or command received from the server or
     beacons. After getting the message, put the data in the message into the
     buffer.

  Parameters:

     None

  Return value:

     None
 */
void *Gateway_process_wifi_receive();


/*
  uuid_str_to_data:

     This function converts uuid from string to unsigned integer.

  Parameters:

     uuid - The uuid in string type.

  Return value:

     unsigned int - The converted uuid in unsigned int type.
 */
unsigned int *uuid_str_to_data(char *uuid);


#endif
