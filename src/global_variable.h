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

 */

#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H
/* Parameter that marks the start of the config file */
#define DELIMITER "="

/* Parameter that marks the start of fracton part of float number */
#define FRACTION_DOT "."

/* Maximum number of characters in each line of config file */
#define CONFIG_BUFFER_SIZE 64

/* Number of times to retry open file, because file openning operation may have
   transient failure. */
#define FILE_OPEN_RETRIES 5

/* Number of times to retry getting a dongle, because this operation may have
   transient failure. */
#define DONGLE_GET_RETRIES 5

/* Number of times to retry opening socket, because socket openning operation
   may have transient failure. */
#define SOCKET_OPEN_RETRIES 5

/* The number of slots in the memory pool */
#define SLOTS_IN_MEM_POOL 1024

/* Length of the IP address in byte */
#define NETWORK_ADDR_LENGTH 16

/* Length of the IP address in Hex */
#define NETWORK_ADDR_LENGTH_HEX 8

/* Maximum length of message to be sent over WiFi in bytes */
#define MAXINUM_WIFI_MESSAGE_LENGTH 4096

/* Minimum Wi-Fi message size (One byte for data type and one byte for a space)
 */
#define MINIMUM_WIFI_MESSAGE_LENGTH 2

/* The size of the array to store Wi-Fi SSID */
#define WIFI_SSID_LENGTH 10

/* The size of the array to store Wi-Fi Password */
#define WIFI_PASS_LENGTH 10

/* Length of the LBeacon's UUID in number of characters */
#define LENGTH_OF_UUID 33

/* Length of coordinates in number of bits */
#define COORDINATE_LENGTH 64

/* The port on which to listen for incoming data */
#define UDP_LISTEN_PORT 8888

/* Number of bytes in the string format of epoch time */
#define LENGTH_OF_EPOCH_TIME 11

/* Time interval in seconds for busy-wait checking in threads */
#define INTERVAL_FOR_BUSY_WAITING_CHECK_IN_SEC 3

/* Time interval in micro seconds for busy-wait checking in threads */
#define INTERVAL_FOR_BUSY_WAITING_CHECK_IN_MICRO_SECONDS 500000

/* Minimum Timeout interval in nano seconds */
#define BUSY_WAITING_TIME 100000

/* Time interval in seconds for reconnect to Gateway */
#define INTERVAL_FOR_RECONNECT_GATEWAY_IN_SEC 120

/* Timeout in seconds for UDP receive socket */
#define TIMEOUT_UDP_RECEIVCE_IN_SEC 5

/* Maximum number of nodes (LBeacons) per star network rooted at a gateway */
#define MAX_NUMBER_NODES 16

/*
  Maximum length of time in seconds low priority message lists are starved
  of attention. */
#define MAX_STARVATION_TIME 600

#define TEST_MALLOC_MAX_NUMBER_TIMES 5

/* Maximum timeout for join request in second */
#define JOIN_REQUEST_TIMEOUT 120

/* zlog category name */
/* The category of log file used for health report */
#define LOG_CATEGORY_HEALTH_REPORT "Health_Report"

/* The category of the printf during debugging */
#define LOG_CATEGORY_DEBUG "LBeacon_Debug"


/* Time interval in seconds for idle status of the Wifi connection between the
gateway and server. Usually, the Wifi connection being idle for longer than
the specified time interval is impossible in BeDIS Object tracker solution. So
we treat the condition as a network connection failure. When this happens,
gateway sends UDP join_request to the server again.
*/
#define INTERVAL_RECEIVE_MESSAGE_FROM_SERVER_IN_SEC 180

/* Time interval in seconds for reconnect to server */
#define INTERVAL_FOR_RECONNECT_SERVER_IN_SEC 120


/*  A struct linking network address assigned to a LBeacon to its UUID,
    coordinates, and location description. */
typedef struct {

    char uuid[LENGTH_OF_UUID];

    /* network address of wifi link to the LBeacon*/
    char net_address[NETWORK_ADDR_LENGTH];

    /* The last LBeacon reported datetime */
    int last_lbeacon_datetime;

    /* The last join request time */
    int last_request_time;

} AddressMap;


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
    int critical_priority;
    int high_priority;
    int normal_priority;
    int low_priority;

} GatewayConfig;

/* Global variables */
/* A Gateway config struct for storing config parameters from the config file */
GatewayConfig config;


#endif