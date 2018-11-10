/*
 Copyright (c) 2016 Academia Sinica, Institute of Information Science

 License:

     GPL 3.0 : The content of this file is subject to the terms and
     conditions defined in file 'COPYING.txt', which is part of this source
     code package.

 Project Name:

     BeDIPS

 File Description:

     This is the header file containing the declarations of functions and
     variables used in the Gateway.c file.

 File Name:

     Gateway.h

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

     Holly Wang   , hollywang@iis.sinica.edu.tw
     Hank Kung    , hank910140@gmail.com
     Ray Chao     , raychao5566@gmail.com
     Gary Xiao    , garyh0205@hotmail.com

 */

 /*
  INCLUDES
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>
#include "CommUnit.h"

#ifndef GATEWAY_H
#define GATEWAY_H

/*
  CONSTANTS
*/

/* File path of the config file of the LBeacon */
#define CONFIG_FILE_NAME "../config/gateway.conf"

/* Parameter that marks the start of the config file */
#define DELIMITER "="

/* Maximum number of characters in each line of config file */
#define CONFIG_BUFFER_SIZE 64

/* Number of lines in the config file */
#define CONFIG_FILE_LENGTH 11

/* The timeout for waiting in number of millisconds */
#define TIMEOUT 3000

/* Maximum number of nodes (LBeacons) per star network rooted at a gateway */
#define MAX_NUMBER_NODES 16

/* Length of the beacon's UUID in a number of charaters */
#define UUID_LENGTH 32

/* Length of address of the network in number of bits */
#define NETWORK_ADDR_LENGTH 16

/* Maximum number of characters in location description */
#define MAX_LENGTH_LOC_DESCRIPTION  64

/* Length of coordinates in number of bits */
#define COORDINATE_LENGTH 64

/* Maximum number of worker threads */
#define MAX_NUM_WORK_THREADS 5

/* Maximum number of buffers */
#define MAX_NUM_BUFFER 6

/* The time period of avoiding starvation */
#define MAX_STARVATION_TIME 50000

/* The time period of polling data from Lbeacon */
#define MAX_POLLING_TIME 100

/* The number of slots in the memory pool */
#define SLOTS_IN_MEM_POOL 512

/* Names of priority levels */
#define NORMAL_PRIORITY 0
#define HIGH_PRIORITY 2
#define LOW_PRIORITY -2

/*
  TYPEDEF STRUCTS
*/

/* The configuration file structure */
typedef struct Config {

    /* The IP address of server for WiFi netwok connection. */
    char  IPaddress[CONFIG_BUFFER_SIZE];

    /* String length needed to store IPaddress */
    int   address_length;

    /* The number of LBeacon nodes in the star network of this gateway */
    int   allowed_number_nodes;

    /* The time period for gateway sending requests to LBeacon */
    int   Period_between_RFHR;

    /*The number of worker threads used by the communication unit for sending
      and receiving packets to and from LBeacons and the sever.*/
    int   Number_worker_threads;

    /* Priority levels at which worker threads execute. */
    int   Number_priority_levels;

} GatewayConfig;

typedef enum buffer_types {

    LBeacon_receive_buffer = 0;
    Server_send_buffer = 1;
    LBeacon_send_buffer = 2;
    Command_msg_buffer = 3;
    BHM_receive_buffer = 4;
    BHM_send_buffer = 5;

} BufferType;

/*
  GLOBAL VARIABLES
*/
/* Message buffer list heads */
BufferListHead LBeacon_send_buffer_list_head;
BufferListHead LBeacon_receive_buffer_list_head;

BufferListHead Server_send_buffer_list_head;

BufferListHead BHM_receive_buffer_list_head;
BufferListHead BHM_send_buffer_list_head;

BufferListHead Command_msg_buffer_list_head;

/* An array of buffer_list_head in the priority order. */
BufferListHead *priority_list_head[MAX_NUM_BUFFER];


/* A Gateway config struct stored config from the config file */
GatewayConfig config;

/*
    A global flag that is initially false and is set by main thread to true
    when initialization completes Afterward, the flag is used by other threads
    to inform the main thread the need to shutdown.
 */
bool ready_to_work;

/*
    Initialization of gateway components invole network activates that may
    take time. These flags enable each module to inform the main thread when
    its initialization completes.
 */
bool NSI_initialization_complete;
bool CommUnit_initialization_complete;
bool initialization_failed;


long long init_time;
long long poll_LBeacon_time;

/*
  get_config:

      This function reads the specified config file line by line until the
      end of file and copies the data in the lines into the GatewayConfig
      struct global variable.

  Parameters:

      file_name - the name of the config file that stores gateway data

  Return value:

      config - GatewayConfig struct
 */
GatewayConfig get_config(char *file_name);

/*
  get_system_time:

      This helper function fetches the current time according to the system
      clock in terms of the number of milliseconds since January 1, 1970.

  Parameters:

      None

  Return value:

      system_time - system time in milliseconds
*/

long long get_system_time();

/*
  startThread:

      This function initializes the specified threads.

  Parameters:

      threads - name of the thread
      thfunct - the function for thread to execute.
      arg - the argument for thread's function

  Return value:

      Error_code: The error code for the corresponding error

 */
ErrorCode startThread(pthread_t *threads, void *( *thfunct)(void *), void *arg);

/*
  Initialize_network:

      This function initializes and sets up all the necessary component for the
      zigee and wifi networks.

  Parameters:

      None

  Return value:

      None
 */
void *Initialize_network();

/*
  CommUnit_routine:

      The function is executed by the main thread of the communication unit that
      is responsible for sending and receiving packets to and from the sever and
      LBeacons after the NSI module has initialized WiFi and Zigbee networks. It
      creates threads to carry out the communication process.

  Parameters:

      Node

  Return value:

      None

 */
void *CommUnit_routine();

#endif
