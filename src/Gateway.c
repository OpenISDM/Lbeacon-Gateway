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

/* The genernal timeout for waiting in number of millisconds */
#define TIMEOUT 3000

/* Maximum number of nodes (LBeacons) per star network */
#define MAX_NUMBER_NODES 32

/*Length of the beacon's UUID*/
#define UUID_LENGTH 32

/*Length of address of the network in number of bits */
#define NETWORK_ADD_LENGTH 16

/* Maximum number of characters in location description */
#define MAX_LENGTH_LOC_DESCRIPTION  64

/* Length of coordinates in number of bits */
#define COORDINATE_LENGTH 64

/*
  TYPEDEF STRUCTS
*/

/* The configuration file structure */

typedef struct Config {

   /* The number of LBeacon nodes in the star network of this gateway */
   int allowed_number_of_nodes;

   /* The flag is true when health reports from LBeacon are requested by the
   BeDIS sever. */
   bool is_health_reporting_polled;

   /* The time period for gateway sending requests to LBeacon */
   int period_between_RFHR;

   /* The number of worker threads used by the communication unit for sending
   and receiving packets to and from LBeacons and the sever. */
   int number_worker_thread;

   /* The number of priority levels at which worker threads execute. */
   int number_priority_levels;


} GatewayConfig;


typedef struct{

  char X_coordinates[COORDINATE_LENGTH];
  char Y_coordinates[COORDINATE_LENGTH];
  char Z_coordinates[COORDINATE_LENGTH];

}Coordinates;


/* A struct linking network address assigned to a LBeacon to its UUID,
   coordinates, and location description. */
typedef struct{

  char beacon_uuid[UUID_LENGTH];
  char* mac_addr;
  Coordinates beacon_coordinates;
  char loc_description[MAX_LENGTH_LOC_DESCRIPTION];

}Address_map;


/* A node of buffer to store received data. Each node has its mac address of
   source Beacon and the content */
typedef struct BufferNode{

    struct List_Entry buffer_entry;
    char *net_address; /* zigbee network address */
    char *content;

} BufferNode;


/* A buffer head for receiving and getting content from LBeacon or server */
typedef struct buffer_list_head{

    struct List_Entry buffer_entry;
    pthread_mutex_t list_lock; /* A per list lock */
    int num_in_list; /* Current number of msg buffers in the list */

} BufferListHead;


/*
  GLOBAL VARIABLES
 */

/* Gateway config struct */
GatewayConfig config;

/* A global flag that is initially false and is set by main thread to true
   when initialization completes Afterward, the flag is used by other threads
   to inform the main thread the need to shutdown.
 */
bool ready_to_work;

/* Initialization of gateway components invole network activates that may
   take time. These flags enable each module to inform the main thread when
   its initialization completes.
 */
bool NSI_initialization_complete;
bool BHM_initialization_complete;
bool CommUnit_initialization_complete;
bool initialization_failed;

/* Message buffer list heads */
BufferListHead per_LBeacon_buffer_list_head[MAX_NUMBER_NODES];
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
  FUNCTIONS
*/

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

  This function initializes the threads.

  Parameters:

  threads - name of the thread
  thfunct - the function for thread to do
  arg - the argument for thread's function

  Return value:

  Error_code: The error code for the corresponding error
*/
ErrorCode startThread(pthread_t* threads, void* (*thfunct)(void*), void* arg);

/*
  Initialize_network:

  Initialize and set up all the necessary component for the zigee
  and wifi network.

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
  LBeacons after the NSI moudle initializes WiFi and Zigbee networks. It
  creates threads to supervise the communication process.

  Parameters:

    Node

  Return value:

    None

*/
void *CommUnit_routine();



/*
  BHM_routine:

  This function integrates the health report collected from all the LBeacons
  and write them in the file. After that send this file to the sever via
  comminication unit.

  Parameters:

    None

  Return value:

    None

*/
void *BHM_routine();



#endif
