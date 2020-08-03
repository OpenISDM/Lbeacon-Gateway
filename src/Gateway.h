/*
  Copyright (c) BiDaE Technology Inc. All rights reserved.

  License:

    BiDaE SHAREWARE LICENSE
    Version 1.0, 31 July 2020

    Copyright (c) BiDaE Technology Inc. All rights reserved.
    The SOFTWARE implemented in the product is copyrighted and protected by 
    all applicable intellectual property laws worldwide. Any duplication of 
    the SOFTWARE other than for archiving or resetting purposes on the same 
    product without the written agreement from BiDaE Technology could be a 
    violation of law. For the avoidance of doubt, redistribution of this 
    SOFTWARE in source or binary form is prohibited. You agree to prevent 
    any unauthorized copying and redistribution of the SOFTWARE. 

    BiDaE Technology Inc. is the license steward. No one other than the 
    license steward has the right to modify or publish new versions of this 
    License. However, You may distribute the SOFTWARE under the terms of the 
    version of the License under which You originally received the Covered 
    Software, or under the terms of any subsequent version published by the 
    license steward.

    LIMITED WARRANTY:

    BiDaE Technology Inc. or its distributors, depending on which party sold 
    the SOFTWARE, warrants that the media on which the SOFTWARE is installed 
    will be free from defects in materials under normal and purposed use.

    BiDaE Technology Inc. or its distributor warrants, for your benefit alone, 
    that during the Warranty Period the SOFTWARE, shall operate substantially 
    in accordance with the functional specifications in the User's Manual. If, 
    during the Warranty Period, a defect in the SOFTWARE appears, You may 
    obtain a replacement of the SOFTWARE. Any replacement SOFTWARE will be 
    warranted for the remainder of the Warranty Period attached to the product.

    WHEN THE WARRANTY PERIOD HAS BEEN EXPIRED, THIS SOFTWARE IS PROVIDED 
    ''AS IS,'' AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
    PARTICULAR PURPOSE ARE DISCLAIMED. HENCEFORTH, IN NO EVENT SHALL BiDaE 
    TECHNOLOGY INC. OR ITS COLLABORATOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Project Name:

     BeDIS

  File Name:

     Gateway.h

  File Description:

     This is the header file containing the declarations of functions and
     variables used in the Gateway.c file.

  Version:

     1.0, 20190306

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
     Gary Xiao    , garyh0205@hotmail.com
     Chun-Yu Lai  , chunyu1202@gmail.com
     Jia Ying Shi , littlestone1225@yahoo.com.tw 

 */

#ifndef GATEWAY_H
#define GATEWAY_H

#define _GNU_SOURCE

#include "BeDIS.h"

/* Enable debugging mode. */
#define debugging

/* Gateway config file location and the config file definition. */

/* File path of the config file of the gateway */
#define CONFIG_FILE_NAME "../config/gateway.conf"

/* File path of the config file of the zlog */
#define ZLOG_CONFIG_FILE_NAME "../config/zlog.conf"

/* File path of the temporary file for active Lbeacons */
#define ACTIVE_LBEACON_FILE_NAME "../log/active_lbeacon_list"

/* File path of the temporary file for abnormal Lbeacons */
#define ABNORMAL_LBEACON_FILE_NAME "../log/abnormal_lbeacon_list"

/* The file for LBeacon self-check result */
#define SELF_CHECK_RESULT_FILE_NAME "../log/self_check_result"

/* The file for LBeacon version */
#define VERSION_FILE_NAME "../log/version"

/* Time interval in seconds for idle status of the Wifi connection between the
gateway and server. Usually, the Wifi connection being idle for longer than
the specified time interval is impossible in BeDIS Object tracker solution. So
we treat the condition as a network connection failure. When this happens,
gateway sends UDP join_request to the server again.
*/
#define INTERVAL_RECEIVE_MESSAGE_FROM_SERVER_IN_SEC 30

/* Time interval in seconds for reconnect to server */
#define INTERVAL_FOR_RECONNECT_SERVER_IN_SEC 30

/* Time interval in seconds for dumping active Lbeacons ip addresses */
#define INTERVAL_FOR_DUMP_ACTIVE_LBEACONS_IN_SEC 60

/* The number of slots in the memory pool for buffer nodes */
#define SLOTS_IN_MEM_POOL_BUFFER_NODE 2048

/* Global variables */

/* The configuration file structure */
typedef struct {

    /* A flag indicating whether this gateway is responsible for geofence feature.*/
    bool is_geofence;

    /* The IP address of the server for WiFi network connection. */
    char IPaddress[NETWORK_ADDR_LENGTH];

    /* The IP address of the server */
    char server_ip[NETWORK_ADDR_LENGTH];

    /* A port that LBeacons and the server are listening on and for gateway to
       send to. */
    int send_port;

    /* A port for gateway to send on */
    int recv_port;
    
    /* The valid time duration for entries in Lbeacon AddressMap */
    int address_map_time_duration_in_sec;
    
} GatewayConfig;

/* A gateway config struct for storing config parameters from the config file */
GatewayConfig config;

/* An array of address maps */
AddressMapArray LBeacon_address_map;

/* The head of a list of buffers for polling messages and commands */
BufferListHead command_msg_buffer_list_head;

/* The last polling times in second*/
int server_latest_polling_time;



/*
  get_gateway_config:

     This function reads the specified config file line by line until the
     end of file and copies the data in each line into an element of the
     GatewayConfig struct global variable.

  Parameters:
     config - gateway related configration settings
     common_config - Common configuration settings among gateway and server
     file_name - The name of the config file that stores the server data

  Return value:

     config - GatewayConfig struct
 */

ErrorCode get_gateway_config(GatewayConfig *config, 
                             CommonConfig *common_config, 
                             char *file_name);

/*
  NSI_routine:

     This function is executed by worker threads when they process the buffer
     nodes in NSI receive buffer list.

  Parameters:

     _buffer_list_head - A pointer to the head of the buffer list containing 
                         buffers to be processed.

  Return value:

     None

 */
void *NSI_routine(void *_buffer_node);

/*
  BHM_routine:

     This function is executed by worker threads when they process the buffer
     nodes in BHM_receive_buffer_list.

  Parameters:

     _buffer_list_head - A pointer of the head of the list containing buffers 
                         to be processed.

  Return value:

     None

 */
void *BHM_routine(void *_buffer_node);


/*
  LBeacon_routine:

     This function is executed by worker threads when they remove buffer nodes
     from LBeacon_receive_buffer_list and send them to the server.

  Parameters:

     _buffer_list_head - A pointer to the head of the list containing buffers 
                         to be processed.

  Return value:

     None

 */
void *LBeacon_routine(void *_buffer_node);


/*
  Server_routine:

     This function is executed by worker threads when they process the buffer
     nodes in Command_msg_buffer_list and broadcast messages in them to 
     LBeacons.

  Parameters:

     _buffer_list_head - A pointer to the head of the list containing buffers 
                         of command messages 

  Return value:

     None

 */
void *Server_routine(void *_buffer_node);

/*
  send_join_request:

      This function sends join_request of a gateway to the server when there 
      is no packets from the server for a specified long time or when there is 
      a new LBeacon requesting to join to this gateway.

  Parameters:

      report_all_lbeacons - a Bool flag indicating the need to report all 
                            registered lbeacons to server
      single_lbeacon_uuid - the uuid of LBeacon which needs to be reported to 
                            the server 

  Return value:

      ErrorCode - The error code for the corresponding error if the function
                  fails or WORK SUCCESSFULLY otherwise
*/

ErrorCode send_join_request(bool report_all_lbeacons, 
                            char *single_lbeacon_uuid);

/*
  handle_health_report:

      This function sends health status to server.

  Parameters:

      None

  Return value:

      ErrorCode - The error code for the corresponding error if the function
                  fails or WORK SUCCESSFULLY otherwise
*/
ErrorCode handle_health_report();


/*
  beacon_join_request:

     This function is executed when a beacon sends a request to join the 
     gateway. When executed, it fills the AddressMap with the inputs and sets the
     network_address if the number of beacons already joined the gateway does
     not excceed allowed_number_of_nodes.

  Parameters:

     address_map - The starting address of the AddressMap
     uuid - The UUID of the LBeacon
     address - The mac address of the LBeacon
     API_version - API version used by LBeacon

  Return value:

     bool - true  : Join success.
            false : Fail to join

 */
bool beacon_join_request(AddressMapArray *address_map, 
                         char *uuid, 
                         char *address, 
                         char *API_version);


/*
  broadcast_to_beacons:

     This function is executed when a command is to be broadcast to LBeacons.
     When called, this function sends the message pointered to by an input 
     parameter to all LBeacons registered in the LBeacon_address_map.

  Parameters:
     address_map - The starting address of the AddressMap.
     pkt_type - The type of this request command
     msg - A pointer to the message to be send to beacons.
     size - The size of the message.

  Return value:

     None

 */
void broadcast_to_beacons(AddressMapArray *address_map, 
                          int pkt_type, 
                          char *msg, 
                          int size);
                          

/*
  send_notification_alarm_to_agents:

     This function is executed when a notification alarm needs to be sent 
     to agents. When called, this function sends notification alarms to the
     agents specified in packet content.

  Parameters:
     message - A pointer to the content of packet.
     size - The size of the message.

  Return value:

     None

*/
void send_notification_alarm_to_agents(char *message, int size);

/*
  Wifi_init:

     This function initializes the Wifi objects.

  Parameters:

     None

  Return value:

      ErrorCode - The error code for the corresponding error or successful

 */
ErrorCode Wifi_init();


/*
  Wifi_free:

     When called, this function frees the queue of the Wi-Fi pkts and sockets.

  Parameters:

     None

  Return value:

     None

 */
void Wifi_free();


/*
  process_wifi_send:

     This function sends the message in the specified buffer list to the 
     server via Wi-Fi.

  Parameters:

     _buffer_list_head - A pointer to the buffer list head.

  Return value:

     None
 */
void *process_wifi_send(void *_buffer_node);


/*
  process_wifi_receive:

     This function listens for messages or commands received from the server or
     beacons and after getting the message, puts the data in the message into a
     buffer.

  Parameters:

     None

  Return value:

     None
 */
void *process_wifi_receive();

#endif
