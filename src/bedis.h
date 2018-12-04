/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and cnditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIPS

  File Description:

     In this file, we group all the definition and declarations used in Gateway
     and LBeacon, including.

  File Name:

     BeDIS.h

  Abstract:

     BeDIPS uses LBeacons to deliver 3D coordinates and textual descriptions of
     their locations to users' devices. Basically, a LBeacon is an inexpensive,
     Bluetooth Smart Ready device. The 3D coordinates and location description
     of every LBeacon are retrieved from BeDIS (Building/environment Data and
     Information System) and stored locally during deployment and maintenance
     times. Once initialized, each LBeacon broadcasts its coordinates and
     location description to Bluetooth enabled user devices within its coverage
     area.

  Authors:

     Gary Xiao     , garyh0205@hotmail.com

 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#ifndef BEDIS_H
#define BEDIS_H

/* Length of address of the network in number of bits */
#define NETWORK_ADDR_LENGTH 16

#define MAX_CONTENT_LENGTH 1024

#define WIFI_SSID_LENGTH 10

#define WIFI_PASS_LENGTH 10

/* Length of the beacon's UUID in a number of charaters */
#define UUID_LENGTH 32

// Length of coordinates in number of bits
#define COORDINATE_LENGTH 64

//The port on which to listen for incoming data
#define UDP_LISTEN_PORT 8888

/* The timeout for waiting in number of millisconds */
#define TIMEOUT 3000

/* Timeout interval in seconds */
#define A_LONG_TIME 30000
#define A_SHORT_TIME 5000
#define A_VERY_SHORT_TIME 300

/* ErrorCode */
typedef enum ErrorCode{

    WORK_SUCCESSFULLY = 0,
    E_MALLOC = 1,
    E_INITIALIZATION_FAIL = 2,
    E_WIFI_INIT_FAIL = 3,
    E_ZIGBEE_INIT_FAIL = 4,
    E_XBEE_VALIDATE = 5,
    E_START_COMMUNICAT_ROUTINE_THREAD = 6,
    E_START_BHM_ROUTINE_THREAD = 7,
    E_START_TRACKING_THREAD = 8,
    E_START_THREAD = 9

} ErrorCode;

typedef struct{

    char X_coordinates[COORDINATE_LENGTH];
    char Y_coordinates[COORDINATE_LENGTH];
    char Z_coordinates[COORDINATE_LENGTH];

} Coordinates;

/* A flag that is used to check if CTRL-C is pressed */
bool g_done = false;

#endif
