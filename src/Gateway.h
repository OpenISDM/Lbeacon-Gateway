/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and conditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

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

#ifndef GATEWAY_H
#define GATEWAY_H

#define _GNU_SOURCE

#include "BeDIS.h"


/* Gateway Config File Location and define about config file. */

/* File path of the config file of the LBeacon */
#define CONFIG_FILE_NAME "../config/gateway.conf"

/* Maximum number of nodes (LBeacons) per star network rooted at a gateway */
#define MAX_NUMBER_NODES 32

/*
  Maximum length of time in second low priority message lists are starved
  of attention.
 */
#define MAX_STARVATION_TIME 20

/* The length of period in number of seconds for polling data from LBeacon. */
#define MAX_POLLING_TIME 60

/* Maximum number of worker threads */
#define MAX_NUM_WORK_THREADS 5

/* Number of lines in the config file */
#define CONFIG_FILE_LENGTH 11

/* Names of priority levels */
#define CRITICAL_PRIORITY -4
#define HIGH_PRIORITY -2
#define NORMAL_PRIORITY 0
#define LOW_PRIORITY 2


/* The configuration file structure */
typedef struct Config {

    /* The IP address of server for WiFi netwok connection. */
    char IPaddress[NETWORK_ADDR_LENGTH];

    /* The number of LBeacon nodes in the star network of this gateway */
    int allowed_number_nodes;

    /* The time period for gateway sending health report requests to LBeacon */
    int Period_between_RFHR;

    /*The number of worker threads used by the communication unit for sending
      and receiving packets to and from LBeacons and the sever.*/
    int Number_worker_threads;

    /* Priority levels at which worker threads execute. */
    int Number_priority_levels;

    char WiFi_SSID[WIFI_SSID_LENGTH];

    char WiFi_PASS[WIFI_PASS_LENGTH];

    char SERVER_IP[NETWORK_ADDR_LENGTH];

} GatewayConfig;


/*  A struct linking network address assigned to a LBeacon to its UUID,
    coordinates, and location description. */
typedef struct{

    char beacon_uuid[UUID_LENGTH];

    /* network address of wifi link to the LBeacon*/
    char net_address[NETWORK_ADDR_LENGTH];

    Coordinates beacon_coordinates;

}Address_map;


typedef struct{

    /* A per list lock */
    pthread_mutex_t list_lock;

    /* Current number of msg buffers in the list */
    int num_in_list;

    Address_map Address_map_list[MAX_NUMBER_NODES];

}Address_map_head;


/* A node of buffer to store received data and/or data to be send */
typedef struct BufferNode{

    struct List_Entry buffer_entry;

    /* network address of the source or destination */
    char net_address[NETWORK_ADDR_LENGTH];

    /* point to where the data is stored. */
    char content[WIFI_MESSAGE_LENGTH];

    int content_size;


} BufferNode;


/* A Head of a list of msg buffer */
typedef struct buffer_list_head{

    /* A per list lock */
    pthread_mutex_t list_lock;

    struct List_Entry priority_entry;

    struct List_Entry buffer_entry;

    /* Number of levels relative to normal priority */
    int priority_boast;

    /* function pointer */
    void (*function)(void* arg);

    /* function's argument */
    void *arg;

} BufferListHead;


// Global variables

/* A Gateway config struct stored config from the config file */
GatewayConfig config;

/* Struct for storing necessary objects for Wifi connection */
sudp_config udp_config;

/* mempool of node for Gateway */
Memory_Pool node_mempool;

/* A list of address maps */
Address_map_head LBeacon_Address_Map;

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
List_Entry Priority_buffer_list_head;

// Flags

/*
  Initialization of gateway components invole network activates that may take
  time. These flags enable each module to inform the main thread when its
  initialization completes.
 */
bool NSI_initialization_complete;
bool CommUnit_initialization_complete;
bool BHM_initialization_complete;

bool initialization_failed;

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
ErrorCode get_config(GatewayConfig *config, char *file_name);


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
void init_buffer(BufferListHead *buffer, void (*function_p)(void*),
								int priority_boast);


/*
  init_Address_map:

     The function initialize the head of the Address_map.

  Parameters:

     LBeacon_map - The head of the Address_map.

  Return value:

     None
 */
void init_Address_map(Address_map_head *LBeacon_map);


/*
  Initialize_network:

     This function initializes and the star network with the gateway at the root
     connecting LBeacons in the network. This the function executed by the
     network_setup_initialize module of the gateway.

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
     LBeacons after the NSI module has initialized WiFi network. It creates
     threads to carry out the communication process.

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
void *Process_message(void *buffer_head);


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
void beacon_join_request(char *ID, char *address
                       , Coordinates Beacon_Coordinates);


/*
  Wifi_init:

     This function initilizes the Wifi's necessory object.

  Parameters:

     IPaddress - The address of the local server

  Return value:

      int - The error code for the corresponding error or successful

 */
int Wifi_init(char *IPaddress);


/*
  Wifi_free:

     When called, this function frees the necessory element.

  Parameters:

     None

  Return value:

     None

 */
void Wifi_free();


/*
  wifi_send:

     This function sends the file to the sever via Wi-Fi.

  Parameters:

     buffer - A pointer of the buffer to be modified.

  Return value:

     None
 */
void *wifi_send(void *buffer_head);


/*
  wifi_recive:

     This function listens the request or command received from the server.
     After getting the message, push the data in to the buffer.

  Parameters:

     buffer - A pointer of the buffer to be modified.

  Return value:

     None
 */
void *wifi_receive();


#endif
