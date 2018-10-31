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

      Han Wang     , hollywang@iis.sinica.edu.tw
      Hank Kung    , hank910140@gmail.com
      Ray Chao     , raychao5566@gmail.com
      Gary Xiao    , garyh0205@hotmail.com

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

/* The configuration file structure */

typedef struct Config {

    /* The number of LBeacon nodes in the star network of this gateway */
    int allowed_number_nodes;

    /*
        The flag is true when health reports from LBeacon are requested by the
        BeDIS sever.
     */
    bool Is_health_reporting_polled;

    /* The time period for gateway sending requests to LBeacon */
    int  Period_between_RFHR;

    /*
        The number of worker threads used by the communication unit for sending
        and receiving packets to and from LBeacons and the sever.
     */
    int  Number_worker_threads;

    /* Priority levels at which worker threads execute. */
    int  Number_priority_levels;

} GatewayConfig;


typedef struct{

    char X_coordinates[COORDINATE_LENGTH];
    char Y_coordinates[COORDINATE_LENGTH];
    char Z_coordinates[COORDINATE_LENGTH];

}Coordinates;

/*
    A struct linking network address assigned to a LBeacon to its UUID,
    coordinates, and location description.
 */
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

    /* Current number of msg buffers in the list */
    int               num_in_list;

} BufferListHead;

/* A node of buffer to store received data and/or data to be send */
typedef struct BufferNode{

    struct List_Entry buffer_entry;

    /* Zigbee network address of the source or destination */
    char             *net_address;

    /* point to where the data is stored. */
    char             *content;


} BufferNode;

/* mempool for Gateway */
Memory_Pool Gateway_MemPool;

/* A Gateway config struct stored config from the config file */
GatewayConfig config;

/*
    A global flag which is initially false and is set by main thread to true
    to tell other threads to shutdown, i.e. clean up and return
 */
bool system_is_shutting_down;

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
bool BHM_initialization_complete;
bool CommUnit_initialization_complete;
bool initialization_failed;

/* Message buffer list heads */
BufferListHead LBeacon_send_buffer_list_head;
BufferListHead LBeacon_receive_buffer_list_head;

BufferListHead NSI_receive_buffer_list_head;
BufferListHead NSI_send_buffer_list_head;

BufferListHead BHM_receive_buffer_list_head;
BufferListHead BHM_send_buffer_list_head;

BufferListHead Command_msg_buffer_list_head;

/* An array of address maps */
Address_map Lbeacon_addresses[MAX_NUMBER_NODES];

/* Current number of LBeacons */
int LBeacon_count;

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

/*
  BHM_routine:

      This function integrates the health report collected from all the LBeacons
      and writes them to a file and then have the file send to the sever by
      comminication unit.

  Parameters:

      None

  Return value:

      None

*/
void *BHM_routine();

#endif
