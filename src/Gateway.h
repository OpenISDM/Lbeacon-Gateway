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
//#define debugging

/* Gateway config file location and the config file definition. */

/* File path of the config file of the Gateway */
#define CONFIG_FILE_NAME "/home/pi/Lbeacon-Gateway/config/gateway.conf"

/* File path of the config file of the zlog */
#define ZLOG_CONFIG_FILE_NAME "/home/pi/Lbeacon-Gateway/config/zlog.conf"

/* The category of log file used for health report */
#define LOG_CATEGORY_HEALTH_REPORT "Health_Report"

#ifdef debugging
/* The category of the printf during debugging */
#define LOG_CATEGORY_DEBUG "LBeacon_Debug"

#endif


#define TEST_MALLOC_MAX_NUMBER_TIMES 5

/* Maximum timeout for join request in second */
#define JOIN_REQUEST_TIMEOUT 120


/* The configuration file structure */
typedef struct {

    /* A flag indicating whether tracked object data from Lbeacon is polled by
       the server */
    bool is_polled_by_server;

    /* The IP address of the server for WiFi netwok connection. */
    char IPaddress[NETWORK_ADDR_LENGTH];

    /* The number of LBeacon nodes in the star network of this gateway */
    int allowed_number_nodes;

    /* The time interval in seconds for gateway to send requests for health
       reports from LBeacon */
    int period_between_RFHR;

    /* The time interval in seconds for gateway to send requests for tracked
       object data from LBeacon */
    int period_between_RFTOD;

    /* The time interval in seconds for gateway to send requests for join request
       to Server */
    int period_between_join_requests;

    /*The number of worker threads used by the communication unit for sending
      and receiving packets to and from LBeacons and the sever.*/
    int number_worker_threads;

    /* The IP address of the server */
    char server_ip[NETWORK_ADDR_LENGTH];

    /* A port that LBeacons and the server are listening on and for gateway to
       send to. */
    int send_port;

    /* A port that the Gateway is listening on and for beacons and server to
       send to */
    int recv_port;

    /* Priority levels at which buffer lists are processed by the worker threads
     */
    int critical_priority;
    int high_priority;
    int normal_priority;
    int low_priority;

} GatewayConfig;





/*
  get_config:

     This function reads the specified config file line by line until the
     end of file and copies the data in each line into an element of the
     GatewayConfig struct global variable.

  Parameters:

     file_name - the name of the config file that stores gateway data

  Return value:

     config - GatewayConfig struct
 */
ErrorCode get_config(GatewayConfig *config, char *file_name);


/*
  sort_priority_list:

     The function arrange entries in the priority list in nonincreasing
     order of Priority_nice.

  Parameters:

     config - The pointer points to the structure which stored config for
              gateway.
     list_head - The pointer of the priority list head.

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


#endif
