/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and conditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

  File Name:

     Gateway.c

  File Description:

     Each gateway is the root of a star network of LBeacons and a leaf in the
     star network with the server at the root. This file contains programs to
     transmit and receive data to and from LBeacons and the sever through Wi-Fi
     network from and to the Gateway, and programs executed by the network setup 
     and initialization modules, Beacon health monitor and comminication unit.

  Version:

     1.0, 20190902

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

     Holly Wang     , hollywang@iis.sinica.edu.tw
     Jake Lee       , jakelee@iis.sinica.edu.tw
     Gary Xiao      , garyh0205@hotmail.com
     Chun-Yu Lai    , chunyu1202@gmail.com
 */

#include "Gateway.h"


int main(int argc, char **argv){

    int return_value;
    int last_join_request_time = 0;
    int uptime;

    /* The main thread of the communication Unit */
    pthread_t CommUnit_thread;

    /* The thread to listen for messages from Wi-Fi interface */
    pthread_t wifi_listener;

    char *temp_lbeacon_uuid = NULL;   
    struct sigaction sigint_handler;

    /* Initialize zlog */

    if(zlog_init(ZLOG_CONFIG_FILE_NAME) == 0){

        category_health_report = zlog_get_category(LOG_CATEGORY_HEALTH_REPORT);

        if (!category_health_report)
            zlog_fini();

#ifdef debugging
        category_debug = zlog_get_category(LOG_CATEGORY_DEBUG);
        if (!category_debug)
            zlog_fini();
#endif
    }

#ifdef debugging
    zlog_info(category_debug, "Gateway start running");
#endif


    if(get_gateway_config( &config, &common_config, CONFIG_FILE_NAME) 
       != WORK_SUCCESSFULLY){
        zlog_error(category_health_report, "Opening config file Fail");
    #ifdef debugging
        zlog_error(category_debug, "Opening config file Fail");
    #endif
        return E_OPEN_FILE;
    }

    /* Initialize all global flags */
    NSI_initialization_complete      = false;
    CommUnit_initialization_complete = false;
    initialization_failed = false;
    ready_to_work = true;

    /* Initialize the address map*/
    init_Address_Map( &LBeacon_address_map);

    /* Initialize buffer_list_heads and add to the head into the priority list.
     */

    init_buffer( &priority_list_head, (void *) sort_priority_list,
                 common_config.high_priority);

    init_buffer( &command_msg_buffer_list_head,
                (void *) Server_routine, common_config.normal_priority);
    insert_list_tail( &command_msg_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &data_receive_buffer_list_head,
                (void *) LBeacon_routine, common_config.normal_priority);
    insert_list_tail( &data_receive_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &NSI_send_buffer_list_head,
                (void *) process_wifi_send, common_config.high_priority);
    insert_list_tail( &NSI_send_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &NSI_receive_buffer_list_head,
                (void *) NSI_routine, common_config.high_priority);
    insert_list_tail( &NSI_receive_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &BHM_receive_buffer_list_head,
                (void *) BHM_routine, common_config.low_priority);
    insert_list_tail( &BHM_receive_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &BHM_send_buffer_list_head,
                (void *) process_wifi_send, common_config.low_priority);
    insert_list_tail( &BHM_send_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

#ifdef debugging
    zlog_info(category_debug, "Buffers initialize Success");
#endif

    sort_priority_list(&common_config, &priority_list_head);

    /* Create the config from input config file */

    /* Initialize the memory pool */
    if(mp_init( &node_mempool, sizeof(BufferNode), SLOTS_IN_MEM_POOL_BUFFER_NODE)
       != MEMORY_POOL_SUCCESS){
        zlog_error(category_health_report, "Mempool Initialization Fail");
#ifdef debugging
        zlog_error(category_debug, "Mempool Initialization Fail");
#endif
        return E_MALLOC;
    }

    /* Initialize the Wifi connection */
    return_value = Wifi_init();
    if(return_value != WORK_SUCCESSFULLY){
        /* Error handling and return */
        initialization_failed = true;
        zlog_error(category_health_report, "Wi-Fi initialization Fail");
#ifdef debugging
        zlog_error(category_debug, "Wi-Fi initialization Fail");
#endif
        return E_WIFI_INIT_FAIL;
    }

#ifdef debugging
    zlog_info(category_debug, "Wi-Fi initialization Success");
#endif

    /* Create threads for sending and receiving data from and to LBeacons and
       the server. */
    /* Two static threads to listen for messages from LBeacon or Sever */
    return_value = startThread( &wifi_listener, (void *)process_wifi_receive,
                               NULL);

    if(return_value != WORK_SUCCESSFULLY){
        initialization_failed = true;
        zlog_error(category_health_report, "wifi_listener initialization Fail");
#ifdef debugging
        zlog_error(category_debug,  "wifi_listener initialization Fail");
#endif
        return E_WIFI_INIT_FAIL;
    }

#ifdef debugging
    zlog_info(category_debug, "wifi_listener initialization Success");
#endif

    NSI_initialization_complete = true;

    /* Create the main thread of Communication Unit  */
    return_value = startThread( &CommUnit_thread, CommUnit_routine, NULL);

    if(return_value != WORK_SUCCESSFULLY){
        zlog_error(category_health_report, "CommUnit_thread Create Fail");
#ifdef debugging
        zlog_error(category_debug, "CommUnit_thread Create Fail");
#endif
        return return_value;
    }

    /* The while loop waiting for NSI, BHM and CommUnit to be ready */
    while(NSI_initialization_complete == false ||
          CommUnit_initialization_complete == false){

        sleep_t(BUSY_WAITING_TIME_IN_MS);

        if(initialization_failed == true){
            ready_to_work = false;
            zlog_error(category_health_report,
                       "The Network or Buffer initialization Fail.");
#ifdef debugging
            zlog_error(category_debug,
                       "The Network or Buffer initialization Fail.");
#endif
            return E_INITIALIZATION_FAIL;
        }
    }

    /* Register handler function for SIGINT signal */
    sigint_handler.sa_handler = ctrlc_handler;
    sigemptyset(&sigint_handler.sa_mask);
    sigint_handler.sa_flags = 0;

    if (-1 == sigaction(SIGINT, &sigint_handler, NULL)) {
        zlog_error(category_health_report,
                   "Error registering signal handler for SIGINT");
        zlog_error(category_debug,
                   "Error registering signal handler for SIGINT");
    }
    
    server_latest_polling_time = 0;
    last_join_request_time = 0;

    /* The while loop that keeps the program running */
    while(ready_to_work == true){

        uptime = get_clock_time();

        if( ( uptime - server_latest_polling_time > 
              INTERVAL_RECEIVE_MESSAGE_FROM_SERVER_IN_SEC ) && 
            ( uptime - last_join_request_time >
              INTERVAL_FOR_RECONNECT_SERVER_IN_SEC ) ){

           if(WORK_SUCCESSFULLY == send_join_request(true, temp_lbeacon_uuid))
            {
                last_join_request_time = uptime;
            }
        }
        else{
            sleep_t(NORMAL_WAITING_TIME_IN_MS);
        }
    }

    /* The program is going to be ended. Free the connection of Wifi */
    Wifi_free();

    mp_destroy(&node_mempool);

#ifdef debugging
    zlog_info(category_debug, "Gateway exit successfullly");
#endif

    return WORK_SUCCESSFULLY;
}


ErrorCode get_gateway_config(GatewayConfig *config, 
                             CommonConfig *common_config,
                             char *file_name) {

    FILE *file = fopen(file_name, "r");
    char config_message[CONFIG_BUFFER_SIZE];
    
    if (file == NULL) {
        /* Error handling */
        zlog_error(category_health_report, "Open config file fail.");
        return E_OPEN_FILE;
    }
        
    fetch_next_string(file, config_message, sizeof(config_message)); 
    config->is_geofence = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    memcpy(config->IPaddress, config_message, sizeof(config->IPaddress));

    fetch_next_string(file, config_message, sizeof(config_message)); 
    common_config->number_worker_threads = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    common_config->omit_out_of_date_packet_in_sec = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    memcpy(config->server_ip, config_message, sizeof(config->server_ip));

    fetch_next_string(file, config_message, sizeof(config_message)); 
    config->send_port = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    config->recv_port = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    common_config->time_critical_priority = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    common_config->high_priority = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    common_config->normal_priority = atoi(config_message);

    fetch_next_string(file, config_message, sizeof(config_message)); 
    common_config->low_priority = atoi(config_message);

    fclose(file);

    
    return WORK_SUCCESSFULLY;
}

void *NSI_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

    char buf[WIFI_MESSAGE_LENGTH];
    char *saveptr = NULL;

    char *current_uuid = NULL;
    char *datetime_str = NULL;

    char uuid[LENGTH_OF_UUID];
    int LBeacon_datetime;
    JoinStatus join_status = JOIN_UNKNOWN;

    memset(buf, 0, sizeof(buf));
    strcpy(buf, temp->content);

    memset(uuid, 0, sizeof(uuid));
    current_uuid = strtok_save(buf, DELIMITER_SEMICOLON, &saveptr);
    memcpy(uuid, current_uuid, sizeof(uuid));

    datetime_str = strtok_save(NULL, DELIMITER_SEMICOLON, &saveptr);
    sscanf(datetime_str, "%d", &LBeacon_datetime);

    /* Put the address into LBeacon_address_map and set the return pkt type
     */
    if (beacon_join_request(&LBeacon_address_map, current_uuid, temp ->
                            net_address, LBeacon_datetime))
        join_status = JOIN_ACK;
    else
        join_status = JOIN_DENY;

    /* put the pkt type into content */

    zlog_debug(category_debug, "uuid=[%s], " \
                               "LBeacon_datetime=[%d], " \
                               "net_address=[%s], " \
                               "join_result=[%d]",
                               uuid,
                               LBeacon_datetime,
                               temp->net_address,
                               join_status);
  
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d;%d;%s;%s;%d;%s;%d;", from_gateway,
                                          join_response, 
                                          BOT_GATEWAY_API_VERSION_LATEST,
                                          uuid, 
                                          LBeacon_datetime,
                                          temp -> net_address,
                                          join_status);
  
    memset(temp->content, 0, sizeof(temp->content));  
    strcpy(temp->content, buf);
    temp->content_size = strlen(temp-> content);

    pthread_mutex_lock(&NSI_send_buffer_list_head.list_lock);

    insert_list_tail( &temp->buffer_entry,
                      &NSI_send_buffer_list_head.list_head);

    pthread_mutex_unlock( &NSI_send_buffer_list_head.list_lock);
 
    send_join_request(true, uuid);

    return (void *)NULL;
}


void *BHM_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;
    char buf[WIFI_MESSAGE_LENGTH];

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d;%d;%s;%s;", from_gateway,
                                 beacon_health_report,  
                                 BOT_SERVER_API_VERSION_LATEST,
                                 temp->content);
    
    strcpy(temp->content, buf);
    temp->content_size = strlen(temp->content);
    strncpy(temp-> net_address, 
            config.server_ip, 
            NETWORK_ADDR_LENGTH);

    pthread_mutex_lock(&BHM_send_buffer_list_head.list_lock);

    insert_list_tail( &temp->buffer_entry,
                      &BHM_send_buffer_list_head.list_head);

    pthread_mutex_unlock( &BHM_send_buffer_list_head.list_lock);

    return (void *)NULL;
}


void *LBeacon_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;
    int pkt_type = temp -> pkt_type;
    float API_version = temp -> API_version;
    char buf[WIFI_MESSAGE_LENGTH];

    if(config.is_geofence)
    {
        pkt_type = time_critical_tracked_object_data;
    }

    memset(buf, 0, sizeof(buf));
    
    // Gateway should support backward compatibility. 
    if(atof(BOT_GATEWAY_API_VERSION_10) == API_version){ 
        sprintf(buf, "%d;%d;%s;%s;", from_gateway,
                                     pkt_type, 
                                     BOT_SERVER_API_VERSION_20,
                                     temp->content);
    }else{
        sprintf(buf, "%d;%d;%s;%s;", from_gateway,
                                     pkt_type, 
                                     BOT_SERVER_API_VERSION_LATEST,
                                     temp->content);
    }
    strcpy(temp->content, buf);	
    temp->content_size = strlen(temp->content);

    /* Add the content of the buffer node to the UDP to be sent to the
       Server */
    udp_addpkt(&udp_config, 
               config.server_ip, 
               config.send_port,
               temp -> content,
               temp -> content_size);

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}


void *Server_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;
    int pkt_type = temp->pkt_type;
    
    switch(pkt_type){
        case tracked_object_data:
        
            zlog_info(category_debug, "Send tracked data request to LBeacon");

            zlog_info(category_debug, "Start Broadcast to LBeacon");

            broadcast_to_beacons(&LBeacon_address_map, pkt_type, 
                                 temp -> content, 
                                 temp -> content_size);


            break;

        case gateway_health_report:
        
            handle_health_report();
            
            // Use beacon_health_report as pkt_type to ask LBeacons to report
            // health report status
            zlog_info(category_debug, "Send health report request to LBeacon");
            
            // Use beacon_health_report as pkt_type to ask LBeacons to report
            // health report status
            pkt_type = beacon_health_report;
            
            zlog_info(category_debug, "Start Broadcast to LBeacon");

            broadcast_to_beacons(&LBeacon_address_map, pkt_type, 
                                 temp -> content, 
                                 temp -> content_size);
          
            break;
            
        case send_notification_alarm:
        
            zlog_info(category_debug, "Send notification alarm request to Agent");

            zlog_info(category_debug, "Start Broadcast to Agent");
            
            send_notification_alarm_to_agents(temp -> content, 
                                              temp -> content_size);
            
            break;
            
        default:
            break;
    }

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}

ErrorCode send_join_request(bool report_all_lbeacons, 
                            char *single_lbeacon_uuid){

    char message_buf[WIFI_MESSAGE_LENGTH];
    char summary_buf[WIFI_MESSAGE_LENGTH];
    char lbeacons_buf[WIFI_MESSAGE_LENGTH];
    char one_lbeacon_buf[WIFI_MESSAGE_LENGTH];

    int send_type = 0;
    int count = 0;
    int index = -1;
    int n;

    zlog_debug(category_debug, ">>send_join_request");

    memset(message_buf, 0, sizeof(message_buf));
    memset(summary_buf, 0, sizeof(summary_buf));
    memset(lbeacons_buf, 0, sizeof(lbeacons_buf));
    memset(one_lbeacon_buf, 0, sizeof(one_lbeacon_buf));

    snprintf(message_buf, sizeof(message_buf), "%d;%d;%s;", 
             from_gateway, request_to_join, BOT_SERVER_API_VERSION_LATEST);

    if(report_all_lbeacons == true){

        zlog_debug(category_debug, "report_all_lbeacons=[%d]", 
                   report_all_lbeacons);
        pthread_mutex_lock(&LBeacon_address_map.list_lock);

        for(n = 0; n < MAX_NUMBER_NODES; n ++){
            if (LBeacon_address_map.in_use[n] == true){

                count++;
                memset(one_lbeacon_buf, 0, sizeof(one_lbeacon_buf));
                sprintf(one_lbeacon_buf, "%s;%d;%s;", 
                        LBeacon_address_map.address_map_list[n].uuid, 
                        LBeacon_address_map.address_map_list[n].last_request_time,
                        LBeacon_address_map.address_map_list[n].net_address);

                if(sizeof(lbeacons_buf) <= 
                   (strlen(lbeacons_buf) + strlen(one_lbeacon_buf))){
   
                    zlog_error(category_debug, 
                               "lbeacons_buf is not big enough to " \
                               "include one_lbeacon_buf");
                    pthread_mutex_unlock(&LBeacon_address_map.list_lock);
                    return E_BUFFER_SIZE;
                }
                strcat(lbeacons_buf, one_lbeacon_buf);
            }
        }

        pthread_mutex_unlock(&LBeacon_address_map.list_lock);
    }
    else if(report_all_lbeacons == false && single_lbeacon_uuid != NULL)
    {
        zlog_debug(category_debug, 
                   "report_all_lbeacons=[%d], " \
                   "single_lbeacon_uuid=[%s]", 
                   report_all_lbeacons,
                   single_lbeacon_uuid);

        pthread_mutex_lock(&LBeacon_address_map.list_lock);

        index = is_in_Address_Map(&LBeacon_address_map, single_lbeacon_uuid, 1);
        if(index >= 0){
            count = 1;

            memset(lbeacons_buf, 0, sizeof(lbeacons_buf));

            sprintf(lbeacons_buf, "%s;%d;%s;",  
                    LBeacon_address_map.address_map_list[index].uuid, 
                    LBeacon_address_map.address_map_list[index].last_request_time,
                    LBeacon_address_map.address_map_list[index].net_address);

            zlog_debug(category_debug, 
                       "lbeacons_buf=[%s]",
                       lbeacons_buf);
        }

        pthread_mutex_unlock(&LBeacon_address_map.list_lock);
    }

    sprintf(summary_buf, "%d;%s;", count, config.IPaddress);
    

    if(sizeof(message_buf) <= strlen(message_buf) + strlen(summary_buf)){
        zlog_error(category_debug, "message_buf is not big enough to " \
                                   "include summary_buf");
        return E_BUFFER_SIZE;
    }

    strcat(message_buf, summary_buf);
    
    if(sizeof(message_buf) <= strlen(message_buf) + strlen(lbeacons_buf)){
        zlog_error(category_debug, "message_buf is not big enough to " \
                                   "include lbeacons_buf");
        return E_BUFFER_SIZE;
    }

    strcat(message_buf, lbeacons_buf);

    udp_addpkt(&udp_config, 
               config.server_ip, 
               config.send_port,
               message_buf, 
               strlen(message_buf));

    zlog_debug(category_debug, "<<send_join_request");

    return WORK_SUCCESSFULLY;
}

ErrorCode handle_health_report(){
    BufferNode *new_node = NULL;

    new_node = mp_alloc( &node_mempool);
    if(new_node == NULL){
        zlog_error(category_debug, "Cannot malloc memory by mp_alloc");
        return E_MALLOC;
    }
	
    memset(new_node, 0, sizeof(BufferNode));

    /* Initialize the entry of the buffer node */
    init_entry( &new_node -> buffer_entry);
        
    new_node->uptime_at_receive = get_clock_time();
    new_node->pkt_direction = from_gateway;
    new_node->pkt_type = gateway_health_report;
    new_node->API_version = atof(BOT_SERVER_API_VERSION_LATEST);

    sprintf(new_node->content, "%d;%d;%s;%s;%d;", from_gateway, 
                                                  gateway_health_report, 
                                                  BOT_SERVER_API_VERSION_LATEST, 
                                                  config.IPaddress, 
                                                  S_NORMAL_STATUS);
     
    new_node->content_size = strlen(new_node-> content);

    strncpy(new_node-> net_address, 
            config.server_ip, 
            NETWORK_ADDR_LENGTH);

    pthread_mutex_lock(&BHM_send_buffer_list_head.list_lock);

    insert_list_tail( &new_node->buffer_entry,
                      &BHM_send_buffer_list_head.list_head);

    pthread_mutex_unlock( &BHM_send_buffer_list_head.list_lock);

    return WORK_SUCCESSFULLY;
}


bool beacon_join_request(AddressMapArray *address_map, char *uuid,
                         char *address, int datetime){

    pthread_mutex_lock( &address_map -> list_lock);
    /* Copy all the necessary information received from the LBeacon to the
       address map. */

    /* Find the first unused address map location and use the location to store
       address of the newly joined LBeacon. */
    int not_in_use = -1;
    int answer = -1;

    answer = is_in_Address_Map(address_map, uuid, 1);
    if(answer >=0)
    {
        /* Need to update both ip address and last request time for each 
           LBeacon */
        memset(address_map -> address_map_list[answer].net_address, 0,
               NETWORK_ADDR_LENGTH);
        strncpy(address_map -> address_map_list[answer].net_address,
                address, strlen(address));
        address_map -> address_map_list[answer].last_request_time =
            get_system_time();
            
        pthread_mutex_unlock( &address_map -> list_lock);
        return true;
    }

    for(int n = 0 ; n < MAX_NUMBER_NODES ; n ++){
        if(address_map -> in_use[n] == false && not_in_use == -1){
            not_in_use = n;
            break;
        }
    }

    /* If here still has space for the LBeacon to register */
    if (not_in_use != -1){

        address_map -> in_use[not_in_use] = true;
        
        memset(address_map->address_map_list[not_in_use].net_address, 0,
               NETWORK_ADDR_LENGTH);
        strncpy(address_map->address_map_list[not_in_use].net_address, 
                address, strlen(address));
                
        memset(address_map->address_map_list[not_in_use].uuid, 0, 
               LENGTH_OF_UUID);
        strncpy(address_map->address_map_list[not_in_use].uuid, 
                uuid, strlen(uuid));
                
        address_map->address_map_list[not_in_use].last_request_time
            = get_system_time();
            
        pthread_mutex_unlock( &address_map -> list_lock);
        return true;
    }
    else{
        pthread_mutex_unlock( &address_map -> list_lock);
        return false;
    }

    return false;
}


void broadcast_to_beacons(AddressMapArray *address_map,
                          int pkt_type, 
                          char *msg, 
                          int size){
  
    char buf[WIFI_MESSAGE_LENGTH];

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d;%d;%s;%s;", from_gateway,
                                 pkt_type, 
                                 BOT_GATEWAY_API_VERSION_LATEST,
                                 msg);
                                             
    pthread_mutex_lock( &address_map -> list_lock);

    zlog_info(category_debug, "==Current in Brocast==");

    if (size <= WIFI_MESSAGE_LENGTH){
        for(int n = 0; n < MAX_NUMBER_NODES; n++){

            if (address_map -> in_use[n] == true){
                zlog_info(category_debug, "Brocast IP: [%s] UUID [%s]", 
                                          address_map ->
                                          address_map_list[n].net_address,
                                          address_map ->
                                          address_map_list[n].uuid);

                /* Add the pkt that to be sent to the server */
                udp_addpkt(&udp_config, 
                           address_map -> address_map_list[n].net_address, 
                           config.send_port,
                           buf, 
                           strlen(buf));
                            
            }
        }
    }

    zlog_info(category_debug, "END Broadcast");
    pthread_mutex_unlock( &address_map -> list_lock);
}

void send_notification_alarm_to_agents(char *message, int size){
  
    char buf[WIFI_MESSAGE_LENGTH];
    char *saveptr = NULL;
    
    char *alarm_type = NULL;
    char *alarm_duration_in_sec = NULL;
    char *agents = NULL;
    char *number_of_agents = NULL;
    int number_agents = 0;
    int i = 0;
    char *agent_ip = NULL;
    char *agent_port = NULL;
    int port = 0;
    
    char message_to_send[WIFI_MESSAGE_LENGTH];

    
    memset(buf, 0, sizeof(buf));
    strcpy(buf, message);
    
    alarm_type = strtok_save(buf, DELIMITER_SEMICOLON, &saveptr);
    
    alarm_duration_in_sec = strtok_save(NULL, DELIMITER_SEMICOLON, &saveptr);
    
    agents = strtok_save(NULL, DELIMITER_SEMICOLON, &saveptr);
    
    number_of_agents = strtok_save(agents, DELIMITER_COMMA, &saveptr);
    
    number_agents = atoi(number_of_agents);
   
    for(i = 0 ; i < number_agents ; i ++){
        agent_ip = strtok_save(NULL, DELIMITER_COLON, &saveptr);
        
        agent_port = strtok_save(NULL, DELIMITER_COMMA, &saveptr);
        if(agent_port == NULL){
            zlog_debug(category_debug, 
                       "agent_port is incorrect, abort the action");
            continue;
        }
        port = atoi(agent_port);
        
        memset(message_to_send, 0, sizeof(message_to_send));
        sprintf(message_to_send, "%d;%d;%s;%s;%s;", 
                from_gateway,
                send_notification_alarm, 
                BOT_AGENT_API_VERSION_LATEST,
                alarm_type,
                alarm_duration_in_sec);
        
        zlog_debug(category_debug, 
                   "send notification alarm [%s] to agent [%s:%d]", 
                   message_to_send,
                   agent_ip,
                   port);
                       
        udp_addpkt(&udp_config, 
                   agent_ip, 
                   port,
                   message_to_send, 
                   strlen(message_to_send));
                   

                       
    }
                 
}

ErrorCode Wifi_init(){

    /* Initialize the Wifi cinfig file */
    if(udp_initial( &udp_config, config.recv_port)
                   != WORK_SUCCESSFULLY){

        /* Error handling TODO */
        return E_WIFI_INIT_FAIL;
    }
    return WORK_SUCCESSFULLY;
}


void Wifi_free(){

    /* Release the Wifi elements and close the connection. */
    udp_release( &udp_config);
    return (void)NULL;
}


void *process_wifi_send(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

    /* Add the content that to be sent to the server */
    udp_addpkt(&udp_config, 
               temp -> net_address, 
               config.send_port, 
               temp->content, 
               temp->content_size);

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}


void *process_wifi_receive(){
    int last_join_request_time;
    int uptime;

    char buf[WIFI_MESSAGE_LENGTH];
    char *saveptr = NULL;
    char *remain_string = NULL;

    char *from_direction = NULL;
    char *request_type = NULL;
    char *API_version = NULL;
    
    while (ready_to_work == true) {

        BufferNode *new_node;

        sPkt temppkt = udp_getrecv( &udp_config);

        if(temppkt.is_null == true){
            /* If there is no packet received, sleep a short time */
            sleep_t(BUSY_WAITING_TIME_IN_MS);
            continue;
        }
        
        uptime = get_clock_time();
        /* Allocate memory from node_mempool a buffer node for received data
           and copy the data from Wi-Fi receive queue to the node. */
        
        new_node = mp_alloc( &node_mempool);
               
        if(new_node == NULL){
            zlog_debug(category_debug, 
                       "process_wifi_receive (new_node) mp_alloc " \
                       "failed, abort this data");
            continue;
        }
        
        memset(new_node, 0, sizeof(BufferNode));

        /* Initialize the entry of the buffer node */
        init_entry( &new_node -> buffer_entry);
        
        new_node->uptime_at_receive = get_clock_time();

        memset(buf, 0, sizeof(buf));
        strcpy(buf, temppkt.content);

        remain_string = buf;
 
        from_direction = strtok_save(buf, DELIMITER_SEMICOLON, &saveptr);
        if(from_direction == NULL){
            mp_free( &node_mempool, new_node);
            continue;
        }
        remain_string = remain_string + strlen(from_direction) + 
                        strlen(DELIMITER_SEMICOLON);
        sscanf(from_direction, "%d", &new_node -> pkt_direction);

        request_type = strtok_save(NULL, DELIMITER_SEMICOLON, &saveptr);
        if(request_type == NULL){
            mp_free( &node_mempool, new_node);
            continue;
        }
        remain_string = remain_string + strlen(request_type) + 
                        strlen(DELIMITER_SEMICOLON);
        sscanf(request_type, "%d", &new_node -> pkt_type);

        API_version = strtok_save(NULL, DELIMITER_SEMICOLON, &saveptr);
        if(API_version == NULL){
            mp_free( &node_mempool, new_node);
            continue;
        }
        remain_string = remain_string + strlen(API_version) + 
                        strlen(DELIMITER_SEMICOLON);
        sscanf(API_version, "%f", &new_node -> API_version);

        /* Copy the content to the buffer_node */
        strcpy(new_node -> content, remain_string);

        new_node -> content_size = strlen(new_node -> content);

        zlog_info(category_debug, "pkt_direction=[%d], " \
                  "pkt_type=[%d] API_version=[%f] " \
                  "new_node -> content=[%s]",   
                  new_node->pkt_direction, 
                  new_node->pkt_type,
                  new_node->API_version,
                  new_node -> content);

        memcpy(new_node -> net_address, temppkt.address, 
               NETWORK_ADDR_LENGTH);

        /* Insert the node to the specified buffer, and release
           list_lock. */
        switch (new_node -> pkt_direction) {
            
            case from_server:

                server_latest_polling_time = uptime;

                switch (new_node -> pkt_type) {

                    case join_response:
                    
                        zlog_info(category_debug,
                                  "Get Join Request Result from the Server");
                        mp_free(&node_mempool, new_node);
                        
                        break;
                        
                    case gateway_health_report:
       
                        zlog_info(category_debug,
                                  "Get Health Report from the Server");
                        pthread_mutex_lock(&command_msg_buffer_list_head
                                           .list_lock);
                        insert_list_tail(&new_node -> buffer_entry,
                                         &command_msg_buffer_list_head
                                         .list_head);
                        pthread_mutex_unlock(&command_msg_buffer_list_head
                                             .list_lock);

                        break;

                    case tracked_object_data:
                
                        zlog_info(category_debug,
                                  "Get Tracked Object Data from the Server");
                        pthread_mutex_lock(&command_msg_buffer_list_head
                                           .list_lock);
                        insert_list_tail(&new_node -> buffer_entry,
                                         &command_msg_buffer_list_head
                                         .list_head);
                        pthread_mutex_unlock(&command_msg_buffer_list_head
                                             .list_lock);
                        break;
                     
                    case send_notification_alarm:
                
                        zlog_info(category_debug,
                                  "Get Send Notification Alarm from the Server");
                        pthread_mutex_lock(&command_msg_buffer_list_head
                                           .list_lock);
                        insert_list_tail(&new_node -> buffer_entry,
                                         &command_msg_buffer_list_head
                                         .list_head);
                        pthread_mutex_unlock(&command_msg_buffer_list_head
                                             .list_lock);
                        break;
                                            
                    default:
                 
                        mp_free(&node_mempool, new_node);
                        break;
                }
                
                break;

            case from_beacon:

                switch (new_node -> pkt_type) {

                    case request_to_join:
                    
                        zlog_info(category_debug,
                                  "Get Join Request from LBeacon");
                        pthread_mutex_lock(&NSI_receive_buffer_list_head
                                           .list_lock);
                        insert_list_tail(&new_node -> buffer_entry,
                                         &NSI_receive_buffer_list_head
                                         .list_head);
                        pthread_mutex_unlock(&NSI_receive_buffer_list_head
                                             .list_lock);
                        break;

                    case tracked_object_data:
                   
                        zlog_info(category_debug,
                                  "Get Tracked Object Data from LBeacon");
                        pthread_mutex_lock(&data_receive_buffer_list_head
                                           .list_lock);
                        insert_list_tail(&new_node -> buffer_entry,
                                         &data_receive_buffer_list_head
                                         .list_head);
                        pthread_mutex_unlock(&data_receive_buffer_list_head
                                             .list_lock);
                        break;

                    case beacon_health_report:
                    
                        zlog_info(category_debug,
                                  "Get Health Report from LBeacon");
                        pthread_mutex_lock(&BHM_receive_buffer_list_head
                                           .list_lock);
                        insert_list_tail(&new_node -> buffer_entry,
                                         &BHM_receive_buffer_list_head
                                         .list_head);
                        pthread_mutex_unlock(&BHM_receive_buffer_list_head
                                             .list_lock);
                        break;

                    default:
                        mp_free( &node_mempool, new_node);
                        break;
                }
                
                break;

            default:
                mp_free( &node_mempool, new_node);
                break;
        }    
    } /* end of while (ready_to_work == true) */
    return (void *)NULL;
}
