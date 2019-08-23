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

/* Time interval in seconds for idle status of the Wifi connection between the
gateway and server. Usually, the Wifi connection being idle for longer than
the specified time interval is impossible in BeDIS Object tracker solution. So
we treat the condition as a network connection failure. When this happens,
gateway sends UDP join_request to the server again.
*/
#define INTERVAL_RECEIVE_MESSAGE_FROM_SERVER_IN_SEC 30

/* Time interval in seconds for reconnect to server */
#define INTERVAL_FOR_RECONNECT_SERVER_IN_SEC 30

/* Global variables */

/* An array of address maps */
AddressMapArray LBeacon_address_map;

/* The head of a list of buffers for polling messages and commands */
BufferListHead command_msg_buffer_list_head;

/* Variables for storing the last polling times in second*/\
int server_latest_polling_time;


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
  sort_priority_list:

     The function arrange entries in the priority list in nonincreasing
     order of the priority nice.

  Parameters:

     config - The pointer points to the structure which stored config for
              gateway.
     list_head - The pointer points to the priority list head.

  Return value:

     None
 */
void *sort_priority_list(GatewayConfig *config, BufferListHead *list_head);



/*
  NSI_routine:

     This function is executed by worker threads when they process the buffer
     nodes in NSI receive buffer list.

  Parameters:

     _buffer_list_head - A pointer of the buffer list containing buffers to be
                         modified.

  Return value:

     None

 */
void *NSI_routine(void *_buffer_node);


/*
  BHM_routine:

     This function is executed by worker threads when they process the buffer
     nodes in BHM_receive_buffer_list.

  Parameters:

     _buffer_list_head - A pointer of the buffer to be modified.

  Return value:

     None

 */
void *BHM_routine(void *_buffer_node);


/*
  LBeacon_routine:

     This function is executed by worker threads when they remove buffer nodes
     from LBeacon_receive_buffer_list and send them to the server directly.

  Parameters:

     _buffer_list_head - A pointer of the buffer to be modified.

  Return value:

     None

 */
void *LBeacon_routine(void *_buffer_node);


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
  send_join_request:

      This function sends join_request to server when there is no packets
      from the server for a specified long time or there are new LBeacon 
      requesting for join to this gateway.

  Parameters:

      report_all_lbeacons - specify if need to report all registered 
                            lbeacons to server
      single_lbeacon_uuid - uuid of LBeacon which need to be reported to
                            server

  Return value:

      ErrorCode - The error code for the corresponding error if the function
                  fails or WORK SUCCESSFULLY otherwise
*/

ErrorCode send_join_request(bool report_all_lbeacons, 
                            char *single_lbeacon_uuid);

/*
  handle_health_report:

      This function sends health status to server.

  Parameters:

      None

  Return value:

      ErrorCode - The error code for the corresponding error if the function
                  fails or WORK SUCCESSFULLY otherwise
*/
ErrorCode handle_health_report();


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
bool beacon_join_request(AddressMapArray *address_map, 
                         char *uuid, 
                         char *address, 
                         int datetime);


/*
  beacon_brocast:

     This function is executed when a command is to be broadcast to LBeacons.
     When called, this function sends msg to all LBeacons registered in the
     LBeacon_address_map.

  Parameters:
     address_map - The head of the AddressMap.
     pkt_type - the request type of this request command
     msg - The pointer to the msg to be send to beacons.
     size - The size of the msg.

  Return value:

     None

 */
void beacon_broadcast(AddressMapArray *address_map, 
                      int pkt_type, 
                      char *msg, 
                      int size);


/*
  Wifi_init:

     This function initializes the Wifi's objects.

  Parameters:

     None

  Return value:

      ErrorCode - The error code for the corresponding error or successful

 */
ErrorCode Wifi_init();


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
  process_wifi_send:

     This function sends the msg in the specified buffer list to the server via
     Wi-Fi.

  Parameters:

     _buffer_list_head - A pointer to the buffer list head.

  Return value:

     None
 */
void *process_wifi_send(void *_buffer_node);


/*
  process_wifi_receive:

     This function listens for messages or command received from the server or
     beacons. After getting the message, put the data in the message into the
     buffer.

  Parameters:

     None

  Return value:

     None
 */
void *process_wifi_receive();


/*
  memset:

      This function is called to fill a block of memory with specified value.

  Parameters:

     ptr    - the pointer to the block memory to fill
     value  - The value in int type: The function will fill the memory with this
              value
     number - number of bytes in the memory area starting from ptr to be
               set to value

  Return value:

     void * - a pointer points to the memory area
*/
extern void * memset(void * ptr, int value, size_t number);


/*
  pthread_attr_init:

      This function is called to initialize thread attributes object pointed
      to by attr with default attribute values

  Parameters:

      attr - pointer to the thread attributes object to be initialized

  Return value:

      0 for success. error number for error.
*/
extern int pthread_attr_init(pthread_attr_t *attr);


/*
  pthread_attr_destroy:

      This function is called to destroy the thread attributes object
      pointed to by attr

  Parameters:

      attr - the thread attributes object to be destroyed

  Return value:

      0 for success. error number for error.
*/
extern int pthread_attr_destroy(pthread_attr_t *attr);


/*
  pthread_create:

      This function is called to start a new thread in the calling process.
      The new thread starts execution by invoking start_routine.

  Parameters:

      thread - a pointer to the new thread
      attr - set thread properties
      start_routine - routine to be executed by the new thread
      arg - the parameters of the start_routine.

  Return value:

      0 for success. error number for error and the contents of *thread are
      undefined.
*/
extern int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);


/*
  pthread_detach:

      This function is called to mark the thread identified by thread as
      detached. When a detached thread returns, its resources are
      automatically released back to the system.

  Parameters:

      thread - a thread to be detached

  Return value:

      0 for success. error number for error.
*/
extern int pthread_detach(pthread_t thread);


#endif
