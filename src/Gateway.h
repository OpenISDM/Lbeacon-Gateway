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

 Version:

     1.0, 20181130

 File Name:

     Gateway.h

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

 */


#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#include "Mempool.h"
#include "UDP_API.h"
#include "LinkedList.h"
#include "thpool.h"
#include "bedis.h"


#ifndef GATEWAY_H
#define GATEWAY_H


// Common define

/* Length of address of the network in number of bits */
#define NETWORK_ADDR_LENGTH 16

/* Maximum number of characters in location description */
#define MAX_LENGTH_LOC_DESCRIPTION  64

/* Length of the beacon's UUID in a number of charaters */
#define UUID_LENGTH 32

// Gateway Config File Location and define about config file.

/* File path of the config file of the LBeacon */
#define CONFIG_FILE_NAME "../config/gateway.conf"

/* Parameter that marks the start of the config file */
#define DELIMITER "="

/* Maximum number of characters in each line of config file */
#define CONFIG_BUFFER_SIZE 64

/* Number of lines in the config file */
#define CONFIG_FILE_LENGTH 11

/* Maximum number of nodes (LBeacons) per star network rooted at a gateway */
#define MAX_NUMBER_NODES 32

/* The timeout for waiting in number of millisconds */
#define TIMEOUT 3000

/* Timeout interval in seconds */
#define A_LONG_TIME 30000
#define A_SHORT_TIME 5000
#define A_VERY_SHORT_TIME 300

/*
  Maximum length of time in millisecond low priority message lists are starved
  of attention.
 */
#define MAX_STARVATION_TIME 60000

/* The length of period in number of second for polling data from LBeacon. */
#define MAX_POLLING_TIME 100

/* The number of slots in the memory pool */
#define SLOTS_IN_MEM_POOL 512

/* Maximum number of worker threads */
#define MAX_NUM_WORK_THREADS 5

/* Maximum number of buffer Lists */
#define MAX_NUM_BUFFER_LIST 6

/* Names of priority levels */
#define HIGH_PRIORITY 2
#define NORMAL_PRIORITY 0
#define LOW_PRIORITY -2


// Struct for Gateway

/* The configuration file structure */
typedef struct Config {

    /* The IP address of server for WiFi netwok connection. */
    char  IPaddress[CONFIG_BUFFER_SIZE];

    /* The number of LBeacon nodes in the star network of this gateway */
    int   allowed_number_nodes;

    /* The time period for gateway sending health report requests to LBeacon */
    int   Period_between_RFHR;

    /*The number of worker threads used by the communication unit for sending
      and receiving packets to and from LBeacons and the sever.*/
    int   Number_worker_threads;

    /* Priority levels at which worker threads execute. */
    int   Number_priority_levels;

} GatewayConfig;

typedef enum buffer_types {

    /* For tracked data from LBeacon at Geofence */
    Time_critical_LBeacon_receive_buffer = 0
    /* For data from server tobe send to LBeacon */
    LBeacon_receive_buffer = 1;
    /* For data from server to be send to LBeacons */
    LBeacon_send_buffer = 2;
    /* For data collected from LBeacons to be send to Server. */
    Server_send_buffer = 3;
    /* For polling tracked object data from Lbeacons and msgs define by BHM */
    Command_msg_buffer = 4;
    /* For health report from LBeacons */
    BHM_receive_buffer = 5;
    /* For processing health report to be send to LBeacons */
    BHM_send_buffer = 6;

} BufferType;

typedef enum pkt_types {

    tracked_object_data = 1;
    health_report = 2;
    data_for_LBeacon = 3;
    poll_for_tracked_object_data = 9;
    RFHR_to_Lbeacons = 10;
    poll_for_RFHR_from_sever = 11;
    request_to_join = 12;
    zigbee_network_control = 13;
    maximum_type = 15;

} PktType;

/*  A struct linking network address assigned to a LBeacon to its UUID,
    coordinates, and location description. */
typedef struct{

    char        beacon_uuid[UUID_LENGTH];

    /* network address of zigbee/wifi link to the LBeacon*/
    char        net_address[Address_length_Hex];

    Coordinates beacon_coordinates;

    /* network address of Wi-Fi link to the Lbeacon */
    char        location_description[MAX_LENGTH_LOC_DESCRIPTION];

}Address_map;

/* A Head of a list of msg buffer */
typedef struct buffer_list_head{

    struct List_Entry buffer_entry;

    /* A per list lock */
    pthread_mutex_t   list_lock;

    /* The index number of the buffer */
    int               buffer_id;

    /* Current number of msg buffers in the list */
    int               num_in_list;

    /* Number of levels relative to normal priority */
    int              priority_boast;

    /* function pointer */
    void             (*function)(void* arg);

    /* function's argument       */
    void             *arg;

    /* A buffer to indicate the buffer is be occupied */
    bool            is_busy;

} BufferListHead;

/* A node of buffer to store received data and/or data to be send */
typedef struct BufferNode{

    struct List_Entry buffer_entry;

    /* Zigbee network address of the source or destination */
    char             net_address[ADDRESS_LENGTH];

    /* point to where the data is stored. */
    char             *content[MAX_CONTENT_LENGTH];


} BufferNode;

/* ErrorCode */

typedef enum ErrorCode{

    WORK_SUCCESSFULLY = 0,
    E_MALLOC = 1,
    E_WIFI_INIT_FAIL = 2,
    E_ZIGBEE_INIT_FAIL = 3,
    E_XBEE_VALIDATE = 4,
    E_START_COMMUNICAT_ROUTINE_THREAD = 5,
    E_START_BHM_ROUTINE_THREAD = 6,
    E_START_TRACKING_THREAD = 7,
    E_START_THREAD = 8

} ErrorCode;


// Global variables

/* A Gateway config struct stored config from the config file */
GatewayConfig config;

/* Struct for storing necessary objects for Wifi connection */
sudp_config udp_config;

/* mempool of node for Gateway */
Memory_Pool node_mempool;

/* An array of address maps */
Address_map Lbeacon_addresses[MAX_NUMBER_NODES];

/* Message buffer list heads */
BufferListHead Time_critical_LBeacon_receive_buffer_list_head;
/* For data from server to be send to LBeacons */
BufferListHead LBeacon_send_buffer_list_head;
/* For data from server tobe send to LBeacon */
BufferListHead LBeacon_receive_buffer_list_head;
/* For data collected from LBeacons to be send to Server. */
BufferListHead Server_send_buffer_list_head;
/* For health report from LBeacons */
BufferListHead BHM_receive_buffer_list_head;
/* For processing health report to be send to LBeacons */
BufferListHead BHM_send_buffer_list_head;
/* For polling tracked object data from Lbeacons and msgs define by BHM */
BufferListHead Command_msg_buffer_list_head;
/* An array of buffer_list_head in the priority order. */
BufferListHead *priority_array[MAX_NUM_BUFFER];

// Flags

/*
  A global flag that is initially false and is set by main thread to true when
  initialization completes Afterward, the flag is used by other threads to
  inform the main thread the need to shutdown.
 */
bool ready_to_work;

/*
  Initialization of gateway components invole network activates that may take
  time. These flags enable each module to inform the main thread when its
  initialization completes.
 */
bool NSI_initialization_complete;
bool CommUnit_initialization_complete;
bool BHM_initialization_complete;


long long init_time;
long long poll_LBeacon_for_HR_time;

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
     zigbee and wifi networks.

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
  Process_message:

     This is the function would be executed by worker threads which proceesed
     the data node in LBeacon_receive_buffer, Command_msg_buffer and
     BHM_receive_buffer. This function remanages the node to be added to the
     new buffer or removed from the orignal buffer.

  Parameters:

     buffer - A pointer of the buffer to be modified.

  Return value:

     None

 */
void *Process_message(BufferListHead *buffer);

/*
  init_buffer:

      The function fills the attributes of buffer storing the packets.
      Including assigning the function to the corresponding buffer list and its
      arguments and its priority level

  Parameters:

      buffer - A pointer of the buffer to be modified.
      buff_id - The index of the buffer for the priority array
      function - A function pointer to be assigned to the buffer
      priority - The priority level of the buffer

  Return value:

      None

 */
void init_buffer(BufferListHead *buffer, int buff_id, void (*function_p)(void*),
								int priority_boast);


/*
 wifi_recieve:

     This function listens the request or command received from the server.
     After getting the message, push the data in to the buffer.

 Parameters:

     buffer - A pointer of the buffer to be modified.

 Return value:

     None
 */
void *wifi_receieve(BufferListHead *buffer);

/*
 wifi_send:

     This function sends the file to the sever via Wi-Fi.

 Parameters:

     buffer_array - An array of buffer to be sent.

 Return value:

     None
 */
void *wifi_send(BufferListHead *buffer_array, int buff_id);

/*
 beacon_join_request:

     This function is executed when a beacon sends command to join the gateway
     and fills the table with the inputs. Set the network_address according
     the current number of beacons.

 Parameters:

     ID - The UUIzd of the LBeacon
     mac - The mac address of the xbee
     Coordinates - Pointerto the beacon coordinates
     Loc_Description - Pointer to the beacon literal location description

 Return value:

     None

 */
void beacon_join_request(char *ID, char *mac, Coordinates Beacon_Coordinates,
                         char *Loc_Description);


/*
 Wifi_init:

     This function initilizes the Wifi's necessory object.

 Parameters:

     IPaddress - The address of the local server

 Return value:

     int - The error code for the corresponding error or successful

 */
int Wifi_init(char IPaddress);

/*
 Wifi_free:

     When called, this function frees the necessory element.

 Parameters:

     None

 Return value:

     None

 */
void Wifi_free();

#endif
