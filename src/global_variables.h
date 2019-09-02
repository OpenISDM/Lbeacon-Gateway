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

     This file include the declarations and definitions of global variables 
     and definition used in BeDIS Gateway but not commonly used in Server and 
     LBeacon.

  Version:

     2.0, 20190902

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
      
     Jia Ying Shi,   littlestone1225@yahoo.com.tw
     Chun-Yu Lai,    chunyu1202@gmail.com
 */

#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

/* The configuration file structure */
typedef struct {

    /* A flag indicating whether this Gateway is responsible for geofence feature.*/
    bool is_geofence;

    /* The IP address of the server for WiFi netwok connection. */
    char IPaddress[NETWORK_ADDR_LENGTH];

    /* The number of LBeacon nodes in the star network of this gateway */
    int allowed_number_nodes;

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
