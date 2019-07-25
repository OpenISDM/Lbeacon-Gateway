/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and cnditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

  File Name:

     global_variable.h

  File Description:

     This file include the global variables and definition that used in BeDIS 
     and Server but not always commonly used in Gateway and LBeacon.

  Version:

     2.0, 20190718

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
      
     Jia Ying Shi  , littlestone1225@yahoo.com.tw
 */

#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H


/* Gateway config file location and the config file definition. */

/* File path of the config file of the Gateway */
#define CONFIG_FILE_NAME "/home/pi/Lbeacon-Gateway/config/gateway.conf"

/* File path of the config file of the zlog */
#define ZLOG_CONFIG_FILE_NAME "/home/pi/Lbeacon-Gateway/config/zlog.conf"

/* Length of the LBeacon's UUID in number of characters */
#define LENGTH_OF_UUID 33

/* Time interval in seconds for idle status of the Wifi connection between the
gateway and server. Usually, the Wifi connection being idle for longer than
the specified time interval is impossible in BeDIS Object tracker solution. So
we treat the condition as a network connection failure. When this happens,
gateway sends UDP join_request to the server again.
*/
#define INTERVAL_RECEIVE_MESSAGE_FROM_SERVER_IN_SEC 180

/* Time interval in seconds for reconnect to server */
#define INTERVAL_FOR_RECONNECT_SERVER_IN_SEC 120


/* The configuration file structure */
typedef struct {

    /* A flag indicating whether this Gateway is responsible for geofence feature.*/
    bool is_geofence;
	
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
    int time_critical_priority;
    int high_priority;
    int normal_priority;
    int low_priority;

} GatewayConfig;

/* Global variables */
/* A Gateway config struct for storing config parameters from the config file */
GatewayConfig config;


#endif