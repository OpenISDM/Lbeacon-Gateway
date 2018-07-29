
/*
* Copyright (c) 2016 Academia Sinica, Institute of Information Science
*
* License:
*
*      GPL 3.0 : The content of this file is subject to the terms and
*      conditions defined in file 'COPYING.txt', which is part of this source
*      code package.
*
* Project Name:
*
*      BeDIPS
*
* File Description:
*
*      This is the header file containing the function declarations and
*      variables used in the Gateway.c file.
*
* File Name:
*
*      Gateway.h
*
* Abstract:
*
*      BeDIPS uses LBeacons to deliver 3D coordinates and textual
*      descriptions of their locations to users' devices. Basically, a
*      LBeacon is an inexpensive, Bluetooth Smart Ready device. The 3D
*      coordinates and location description of every LBeacon are retrieved
*      from BeDIS (Building/environment Data and Information System) and
*      stored locally during deployment and maintenance times. Once
*      initialized, each LBeacon broadcasts its coordinates and location
*      description to Bluetooth enabled user devices within its coverage
*      area.
*
* Authors:
*
*      Han Wang, hollywang@iis.sinica.edu.tw
*      Jake Lee, jakelee@iis.sinica.edu.tw
*      Johnson Su, johnsonsu@iis.sinica.edu.tw
*      Hank Kung, hank910140@gmail.com
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
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>
#include "CommUnit.h"



/* 
* CONSTANTS 
*/

/* Maximum number of nodes (LBeacons) per star network */
#define MAX_NUMBER_NODES 32

/*Length of the beacon's UUID*/
#define UUID_LENGTH 32

/*Length of the address of the network */
#define NETWORK_ADD_LENGTH 16

/* Maximum number of characters in location description*/
#define MAX_LENGTH_LOC_DESCRIPTION  64

#define COORDINATE_LENGTH 64

#define A_SHORT_TIME 1000

#define A_LONG_TIME 5000

/* Buffer size in gateway */
#define BUFFER_SIZE 1024

/* The time intervial between each beacon health self-testing */
#define PERIOD_TO_MONITOR 1000*100

/*  */
#define SERV_PORT 8888


/*
* TYPEDEF STRUCTS
*/

typedef struct{

  char X_coordinates[COORDINATE_LENGTH];
  char Y_coordinates[COORDINATE_LENGTH];
  char Z_coordinates[COORDINATE_LENGTH];

}Coordinates;

typedef struct{

  int16_t network_address[NETWORK_ADD_LENGTH];
  char beacon_uuid[UUID_LENGTH];
  Coordinates beacon_coordinates;
  char loc_description[MAX_LENGTH_LOC_DESCRIPTION];


}Address_map;

/*
* GLOBAL VARIABLES
*/

/* A global flag which is initially false and is set by main thread to true 
* to tell other threads to shutdown, i.e. clean up and return */
bool system_is_shutting_down;

/*A global flag that is initially false and is set nby main thread to ture 
* when initialization completes Afterward, the flag is used by other threads 
* to inform the main thread the need to shutdown. */
bool ready_to_work;

/* A global flag set to be true by a thread when its inintialiazation failed. */
bool initialization_failed;

/* Initialization of gateway components invole network activates that may take 
* time. The main thread should wait until their initialization is sufficiently 
* compete. These flags enable the modules to inform the main thread when it 
* happens. */
bool NSI_initialization_complete;
bool BHM_initialization_complete;
bool CommUnit_initialization_complete;

/* An array of address maps */
Address_map beacon_address[MAX_NUMBER_NODES];

//current number of beacons
int beacon_count;
// NSI is the only writer of beacon_address; it has many readers.
bool Beacon_address_lock;

bool health_report[MAX_NUMBER_NODES];

/* ZigBee API Variables */
struct xbee *xbee;

struct xbee_con *con;

/* The address stored the destination MAC of xbee */
struct xbee_conAddress address;

/* The setting for xbee */
struct xbee_conSettings settings;

/* A variable to get error code */
xbee_err ret;

/* A variable txRet get Tx return value */
unsigned char txRet;

const char *xbee_mode = "xbeeZB";

char *xbee_device = "/dev/ttyAMA0";

int xbee_baudrate = 9600;

//A 64-bit extended PAN ID for join Network
char *PAN_ID = "0000000000000000";

//0:disable Log, 100:enable Log
int LogLevel = 100;

// A flag to indicate if all part of address are get. 
// 3 send first address 
// 2 send second address 
// 0 success

enum{finish,wait_SL,wait_SH,start};
int get_address = start;

char* Local_Address = "";

pkt_ptr pkt_queue;

bool zigbee_is_ready = false;
/* FUNCTIONS */

/*
*  get_system_time:
*
*  This helper function fetches the current time according to the system
*  clock in terms of the number of milliseconds since January 1, 1970.
*
*  Parameters:
*
*  None
*
*  Return value:
*
*  system_time - system time in milliseconds
*/
long long get_system_time();

/* coordinator initializes the zigbee network:
- if (PAN ID == 0) scan nearby network and chooses a PAN ID;
- channel scan to find a good operating channel;
- ready to access join requests from Lbeacons;
/* Set up Zigbee connection by calling Zigbee_routine in LBeacon_Zigbee.h */
void *NSI_routine();

/*
*  addrss_map_manager:
*
*  This function initializes the table which stores the information
*  of each Lbeacon. Keep monitoring if there's a new beacon send the command to
*  join the gateway. And if there is, call beacon_join_request(). At the
*  meanwhile, it also counts the current number of beacons in this gateway
*  coverage.
*
*  Parameters:
*
*  None
*
*  Return value:
*
*  O
*/
void *address_map_manager();

/*
*  beacon_join_request:
*  This function is executed when a beacon sends command to join the gateway
*  and fills the table with the inputs. And set the network_address according
*  the current number of beacons.
*
*  Parameters:
*
*  ID -
*  Coordinates -
*  Loc_Description[MAX_LENGTH_LOC_DESCRIPTION -
*  Barcode -
*
*  Return value:
*
*  None
*/
void *beacon_join_request(int index, unsigned ID,Coordinates Beacon_Coordinates,
                         char *Loc_Description[MAX_LENGTH_LOC_DESCRIPTION]
                         ,double Barcode);

/*
*  BHM_routine:
*
*  This function initializes the table to record beacon health. After then,
*  keep maintaining the table which stores the beacons health state currently.
*
*  Parameters:
*
*  Node
*
*  Return value:
*
*  None
*/
void *BHM_routine();

/*
*  startThread:
*
*  This function initializes the threads.
*
*  Parameters:
*
*  threads - name of the thread
*  thfunct - the function for thread to do
*  arg - the argument for thread's function
*
*  Return value:
*
*  Error_code: The error code for the corresponding error
*/
Error_code startThread(pthread_t threads, void * (*thfunct)(void*), void *arg);

/*
*  cleanup_exit:
*
*  This function releases all the resources and set the flag.
*
*  Parameters:
*
*  None
*
*  Return value:
*
*  None
*/
void cleanup_exit();

/*
*
*/
void error(char * msg){
    perror(msg);
    exit(0);
}

