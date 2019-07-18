/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and cnditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

  File Name:

     BeDIS.h

  File Description:

     This file contains the definitions and declarations of constants,
     structures, and functions used in both Gateway and LBeacon.

  Version:

     2.0, 20190606

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

     Gary Xiao     , garyh0205@hotmail.com
     Joey Zhou     , joeyzhou5566@gmail.com
     Holly Wang    , hollywang@iis.sinica.edu.tw
     Jake Lee      , jakelee@iis.sinica.edu.tw
     Chun Yu Lai   , chunyu1202@gmail.com

 */

#ifndef BEDIS_H
#define BEDIS_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/file.h>
#include "Mempool.h"
#include "UDP_API.h"
#include "LinkedList.h"
#include "thpool.h"
#include "zlog.h"
#include "global_variable.h"


typedef enum _ErrorCode{

    WORK_SUCCESSFULLY = 0,
    E_MALLOC = 1,
    E_OPEN_FILE = 2,
    E_OPEN_DEVICE = 3,
    E_OPEN_SOCKET = 4,
    E_SEND_OBEXFTP_CLIENT = 5,
    E_SEND_CONNECT_DEVICE = 6,
    E_SEND_PUSH_FILE = 7,
    E_SEND_DISCONNECT_CLIENT = 8,
    E_SCAN_SET_EVENT_MASK = 9,
    E_SCAN_SET_ENABLE = 10,
    E_SCAN_SET_HCI_FILTER = 11,
    E_SCAN_SET_INQUIRY_MODE = 12,
    E_SCAN_START_INQUIRY = 13,
    E_SEND_REQUEST_TIMEOUT = 14,
    E_ADVERTISE_STATUS = 15,
    E_ADVERTISE_MODE = 16,
    E_SET_BLE_PARAMETER = 17,
    E_BLE_ENABLE = 18,
    E_GET_BLE_SOCKET = 19,
    E_START_THREAD = 20,
    E_INIT_THREAD_POOL = 21,
    E_INIT_ZIGBEE = 22,
    E_LOG_INIT = 23,
    E_LOG_GET_CATEGORY = 24,
    E_EMPTY_FILE = 25,
    E_INPUT_PARAMETER = 26,
    E_ADD_WORK_THREAD = 27,
    E_INITIALIZATION_FAIL = 28,
    E_WIFI_INIT_FAIL = 29,
    E_START_COMMUNICAT_ROUTINE_THREAD = 30,
    E_START_BHM_ROUTINE_THREAD = 31,
    E_START_TRACKING_THREAD = 32,
    E_REG_SIG_HANDLER = 33,
    E_JOIN_THREAD = 34,
    E_BUFFER_SIZE = 35,
    E_PREPARE_RESPONSE_BASIC_INFO = 36,
    E_ADD_PACKET_TO_QUEUE = 37,
    E_SQL_PARSE = 38,
    E_SQL_RESULT_EXCEED = 39,
    MAX_ERROR_CODE = 40

} ErrorCode;

typedef struct {
    ErrorCode code;
    char *message;
} errordesc;

typedef enum _HealthReportErrorCode{
    S_NORMAL = 0,
    E_ERROR = 1
} HealthReportErrorCode;

/*  A struct linking network address assigned to a LBeacon to its UUID,
    coordinates, and location description. */
typedef struct {

    char uuid[UUID_LENGTH];

    /* network address of wifi link to the LBeacon*/
    char net_address[NETWORK_ADDR_LENGTH];

    /* The last LBeacon reported datetime */
    int last_lbeacon_datetime;

    /* The last join request time */
    int last_request_time;

} AddressMap;

typedef struct {

    /* A per list lock */
    pthread_mutex_t list_lock;

    /* A Boolean array in which ith element records whether the ith address map
       is in use. */
    bool in_use[MAX_NUMBER_NODES];

    AddressMap address_map_list[MAX_NUMBER_NODES];

} AddressMapArray;

/* The struct of buffers used to store received data and/or data to be send */
typedef struct {

    struct List_Entry buffer_entry;

    /* network address of the source or destination */
    char net_address[NETWORK_ADDR_LENGTH];

    /* pointer to where the data is stored. */
    char content[MAXINUM_WIFI_MESSAGE_LENGTH];

    int content_size;

} BufferNode;

/* A Head of a list of msg buffers */
typedef struct {

    /* A per list lock */
    pthread_mutex_t list_lock;


    struct List_Entry list_head;

    struct List_Entry priority_list_entry;

    /* nice relative to normal priority (i.e. nice = 0) */
    int priority_nice;

    /* The pointer point to the function to be called to process buffer nodes in
       the list. */
    void (*function)(void *arg);

    /* function's argument */
    void *arg;

} BufferListHead;


typedef struct coordinates{

    char X_coordinates[COORDINATE_LENGTH];
    char Y_coordinates[COORDINATE_LENGTH];
    char Z_coordinates[COORDINATE_LENGTH];

} Coordinates;


typedef enum pkt_types {
    /* Unknown type of pkt type */
    undefined = 0,

    /* For Join Request */

    /* Request join */
    request_to_join = 1,
    /* Accept join request */
    join_request_ack = 2,
    /* Deny join request */
    join_request_deny = 3,

    /* For LBeacon pkt type */

    /* A pkt containing tracked object data */
    tracked_object_data = 4,
    /* A pkt containing health report */
    health_report = 5,
    /* A pkt for LBeacon */
    data_for_LBeacon = 6,

    /* For server */

    /* For the Gateway polling tracked object data from LBeacons */
    poll_for_tracked_object_data_from_server = 9,
    /* A polling request for health report from server */
    RFHR_from_server = 10

} PktType;


typedef enum pkt_direction {
    /* pkt from gateway */
    from_gateway = 10,
    /* pkt from server */
    from_server = 8,
    /* pkt from beacon */
    from_beacon = 0

} PktDirection;


/* Type of device to be tracked. */
typedef enum DeviceType {

    BR_EDR = 0,
    BLE = 1,
    max_type = 2

} DeviceType;


/* The pointer to the category of the log file */
zlog_category_t *category_health_report, *category_debug;


/* Struct for storing necessary objects for Wifi connection */
sudp_config udp_config;

/* mempool from which buffer node structure are allocated */
Memory_Pool node_mempool;

/* The head of a list of buffers of data from LBeacons to be send to the Server
 */
BufferListHead LBeacon_receive_buffer_list_head;

/* The head of a list of the return message for LBeacon join requests */
BufferListHead NSI_send_buffer_list_head;

/* The head of a list of buffers for return join request status */
BufferListHead NSI_receive_buffer_list_head;

/* The head of a list of buffers holding health reports to be processed and sent
   to the Server */
BufferListHead BHM_send_buffer_list_head;

/* The head of a list of buffers holding health reports from LBeacons */
BufferListHead BHM_receive_buffer_list_head;

/* Head of a list of buffer list head in priority order. */
BufferListHead priority_list_head;



/* Flags */

/*
  Initialization of gateway components involves network activaties that may take
  time. These flags enable each module to inform the main thread when its
  initialization completes.
 */
bool NSI_initialization_complete;
bool CommUnit_initialization_complete;

/* The flag is to identify whether any component fail to initialize */
bool initialization_failed;

/* A global flag that is initially set to true by the main thread. It is set
   to false by any thread when the thread encounters a fatal error,
   indicating that it is about to exit. In addition, if user presses Ctrl+C,
   the ready_to_work flag will be set as false to stop all threads. */
bool ready_to_work;


/* FUNCTIONS */


/*
  twoc:

  @todo

  Parameters:

     in - @todo
     t  -  @todo

  Return value:

     data - @todo
 */
unsigned int twoc(int in, int t);


/*
  sort_priority_list:

     The function arrange entries in the priority list in nonincreasing
     order of Priority_nice.

  Parameters:

     config - The pointer points to the structure which stored config for
              gateway.
     list_head - The pointer of the priority list head.

  Return value:

     None
 */
void *sort_priority_list(GatewayConfig *config, BufferListHead *list_head);


/*
  CommUnit_routine:

     The function is executed by the main thread of the communication unit that
     is responsible for sending and receiving packets to and from the sever and
     LBeacons after the NSI module has initialized WiFi networks. It creates
     threads to carry out the communication process.

  Parameters:

     None

  Return value:

     None

 */
void *CommUnit_routine();


/*
  init_buffer:

     The function fills the attributes of a specified buffer to be called by
     another threads to process the buffer content, including the function, the
     argument of the function and the priority level which the function is to be
     executed.

  Parameters:

     buffer - A pointer of the buffer to be modified.
     buff_id - The index of the buffer for the priority array
     function - A function pointer to be assigned to the buffer
     priority - The priority level of the buffer

  Return value:

     None
 */
void init_buffer(BufferListHead *buffer_list_head, void (*function_p)(void *),
                 int priority_nice);


/*
  init_Address_Map:

     This function initialize the head of the AddressMap.

  Parameters:

     address_map - The head of the AddressMap.

  Return value:

     None
 */
void init_Address_Map(AddressMapArray *address_map);


/*
  is_in_Address_Map:

     This function check whether the uuid is in LBeacon_address_map.

  Parameters:

     address_map - The head of the AddressMap.
     uuid - the uuid we decide to compare.

  Return value:

     bool: If return true means in the address map, else false.
 */
int is_in_Address_Map(AddressMapArray *address_map, char *uuid);


/*
  trim_string_tail:

     This function trims whitespace, newline and carry-return at the end of
     the string when reading config messages.

  Parameters:

     message - The pointer points to the character array containing the input
               string

  Return value:

     None
 */
void trim_string_tail(char *message);


/*
  ctrlc_handler:

     When the user presses CTRL-C, the function sets the global variable
     ready_to_work to false, and throw a signal to stop running the program.

  Parameters:

     stop - A interger signal triggered by ctrl-c.

  Return value:

     None

 */
void ctrlc_handler(int stop);


/*
  startThread:

     This function initializes the specified thread. Threads initialized by
     this function will be create in detached mode.

  Parameters:

     thread        - The pointer of the thread.
     start_routine - routine to be executed by the thread.
     arg           - the argument for the start_routine.

  Return value:

     Error_code: The error code for the corresponding error
 */
ErrorCode startThread(pthread_t *thread, void *( *start_routine)(void *),
                      void *arg);


/*
  get_system_time:

     This helper function fetches the current time according to the system
     clock in terms of the number of seconds since January 1, 1970.

  Parameters:

     None

  Return value:

     system_time - system time in seconds
*/
int get_system_time();


/*
  strtok_save:
     
     This function breaks string str into a series of tokens using the delimiter delim.

     Linux uses strtok_r()

  Parameters:

     str - The contents of this string are modified and broken into smaller    
           strings (tokens).
     delim - This is the C string containing the delimiters. These may vary 
             from one call to another.

     saveptr - The pointer points to the next char of the searched char.

  Return value:

      Return a pointer to the next token, or NULL if there are no more tokens.

 */
char *strtok_save(char *str, char *delim, char **saveptr);


/*
  memset:

      This function is called to fill a block of memory with specified value.

  Parameters:

     ptr    - the pointer to the block memory to fill
     value  - The value in int type: The function will fill the memory with this
              value
     number - number of bytes in the memory area starting from ptr to be
               set to value

  Return value:

     void * - a pointer points to the memory area
*/
extern void * memset(void * ptr, int value, size_t number);


/*
  pthread_attr_init:

      This function is called to initialize thread attributes object pointed
      to by attr with default attribute values

  Parameters:

      attr - pointer to the thread attributes object to be initialized

  Return value:

      0 for success. error number for error.
*/
extern int pthread_attr_init(pthread_attr_t *attr);


/*
  pthread_attr_destroy:

      This function is called to destroy the thread attributes object
      pointed to by attr

  Parameters:

      attr - the thread attributes object to be destroyed

  Return value:

      0 for success. error number for error.
*/
extern int pthread_attr_destroy(pthread_attr_t *attr);


/*
  pthread_create:

      This function is called to start a new thread in the calling process.
      The new thread starts execution by invoking start_routine.

  Parameters:

      thread - a pointer to the new thread
      attr - set thread properties
      start_routine - routine to be executed by the new thread
      arg - the parameters of the start_routine.

  Return value:

      0 for success. error number for error and the contents of *thread are
      undefined.
*/
extern int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);


/*
  pthread_detach:

      This function is called to mark the thread identified by thread as
      detached. When a detached thread returns, its resources are
      automatically released back to the system.

  Parameters:

      thread - a thread to be detached

  Return value:

      0 for success. error number for error.
*/
extern int pthread_detach(pthread_t thread);


#endif
