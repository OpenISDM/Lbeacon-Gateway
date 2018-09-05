
/*
 Copyright (c) 2016 Academia Sinica, Institute of Information Science

 License:

      GPL 3.0 : The content of this file is subject to the terms and
      conditions defined in file 'COPYING.txt', which is part of this source
      code package.

 Project Name:

      BeDIPS

 File Description:

      This is the header file containing the function declarations and
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

      Han Wang, hollywang@iis.sinica.edu.tw
      Hank Kung, hank910140@gmail.com
      Ray Chao, raychao5566@gmail.com

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


/* The gernal timeout for waiting */
#define TIMEOUT 3000
#define A_SHORT_TIME 5000

/* Maximum number of nodes (LBeacons) per star network */
#define MAX_NUMBER_NODES 32

/*Length of the beacon's UUID*/
#define UUID_LENGTH 32

/*Length of the address of the network */
#define NETWORK_ADD_LENGTH 16

/* Maximum number of characters in location description*/
#define MAX_LENGTH_LOC_DESCRIPTION  64

#define COORDINATE_LENGTH 64
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

/* Flag to state the connection of wifi */
bool wifi_is_ready;
/* Flag to state the connection of xbee */
bool zigbee_is_ready;

//current number of beacons
int beacon_count;





/*
* TYPEDEF STRUCTS
*/

typedef struct{

  char X_coordinates[COORDINATE_LENGTH];
  char Y_coordinates[COORDINATE_LENGTH];
  char Z_coordinates[COORDINATE_LENGTH];

}Coordinates;


/* A struct linking network address assigned to a LBeacon to its UUID, 
   corrnidate , and location description. */
typedef struct{

  char beacon_uuid[UUID_LENGTH];
  char *mac_addr;
  Coordinates beacon_coordinates;
  char loc_description[MAX_LENGTH_LOC_DESCRIPTION];



}Address_map;

/* An array of address maps */
Address_map beacon_address[MAX_NUMBER_NODES];



/* FUNCTIONS */



/*
  Initialize_network:

  Initialize and set up all the necessary component for the zigee 
  and wifi network.

  Parameters:

    None

  Return value:

    None
*/
void Initialize_network();



/*
  CommUnit_routine:

  The function held all packets sent and recieved from server and beacon
  after NSI module initializes UDP and ZigBee network setup. And, create 
  subthreads for supervise the process of communication. 

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

/*
  Manage_track_buffer:

  This function integrates the tracked data collected from all the LBeacons 
  and write them in the file. After that send this file to the sever via
  comminication unit.

  Parameters:

    None

  Return value:

    None

*/
void *Manage_track_buffer()


/*
  zigbee_receiver:
  
    This function for thread that receives the track object data, health 
    report or registeration from the LBeacon. Check the first element in 
    the received message to learn the type. According to different kind
    of message, different actions would be taken. 

  Parameters:

    None

  Return value:

    None

*/
void *zigbee_receiver();

/*
  zigbee_sender:

    This functions send the command or message to LBeacon according to the 
    buffer which is receiving message from sever. Or, after a timeout, 
    send the request to LBeacon to ask track data or heal report.

  Parameters:

    None

  Return value:

    None
*/
void *zigbee_sender();

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
ErrorCode startThread(pthread_t threads, void * (*thfunct)(void*), void *arg);


/*
  cleanup_exit:

  This function releases all the resources and set the flag.

  Parameters:

  None

  Return value:

  None
*/
void cleanup_exit();



