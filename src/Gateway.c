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
     transmit and receive data to and from LBeacon and the sever through Wi-Fi
     network from and to the Gateway, and programs executed by network setup and
     initialization, Beacon health monitor and comminication unit.

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

     Holly Wang     , hollywang@iis.sinica.edu.tw
     Jake Lee       , jakelee@iis.sinica.edu.tw
     Ray Chao       , raychao5566@gmail.com
     Gary Xiao      , garyh0205@hotmail.com
     Chun Yu Lai    , chunyu1202@gmail.com
 */

/* TODO
     When Gateway not join Server */

#include "Gateway.h"


int main(int argc, char **argv){

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

    int return_value;

    /* The main thread of the communication Unit */
    pthread_t CommUnit_thread;

    /* The thread to listen for messages from Wi-Fi interface */
    pthread_t wifi_listener;

    /* All global flags */
    NSI_initialization_complete      = false;
    CommUnit_initialization_complete = false;

    initialization_failed = false;

    ready_to_work = true;

    /* Create the config from input config file */

    if(get_config( &config, CONFIG_FILE_NAME) != WORK_SUCCESSFULLY){
        zlog_error(category_health_report, "Opening config file Fail");
#ifdef debugging
        zlog_error(category_debug, "Opening config file Fail");
#endif
        return E_OPEN_FILE;
    }

    /* Initialize the memory pool */
    if(mp_init( &node_mempool, sizeof(BufferNode), SLOTS_IN_MEM_POOL)
       != MEMORY_POOL_SUCCESS){
        zlog_error(category_health_report, "Mempool Initialization Fail");
#ifdef debugging
        zlog_error(category_debug, "Mempool Initialization Fail");
#endif
        return E_MALLOC;
    }

    /* Initialize the address map*/
    init_Address_Map( &LBeacon_address_map);

    /* Initialize buffer_list_heads and add to the head in to the priority list.
     */

    init_buffer( &priority_list_head, (void *) sort_priority_list,
                config.high_priority);

    init_buffer( &time_critical_LBeacon_receive_buffer_list_head,
                (void *) LBeacon_routine, config.normal_priority);
    insert_list_tail( &time_critical_LBeacon_receive_buffer_list_head
                     .priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &command_msg_buffer_list_head,
                (void *) Server_routine, config.normal_priority);
    insert_list_tail( &command_msg_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &LBeacon_receive_buffer_list_head,
                (void *) LBeacon_routine, config.normal_priority);
    insert_list_tail( &LBeacon_receive_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &NSI_send_buffer_list_head,
                (void *) process_wifi_send, config.low_priority);
    insert_list_tail( &NSI_send_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &NSI_receive_buffer_list_head,
                (void *) NSI_routine, config.low_priority);
    insert_list_tail( &NSI_receive_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &BHM_receive_buffer_list_head,
                (void *) BHM_routine, config.low_priority);
    insert_list_tail( &BHM_receive_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &BHM_send_buffer_list_head,
                (void *) process_wifi_send, config.low_priority);
    insert_list_tail( &BHM_send_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

#ifdef debugging
    zlog_info(category_debug, "Buffers initialize Success");
#endif

    sort_priority_list(&config, &priority_list_head);

    /* Initialize the Wifi connection */
    if(return_value = Wifi_init(config.IPaddress) != WORK_SUCCESSFULLY){
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

        usleep(BUSY_WAITING_TIME);

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

    int current_time;

    current_time = get_system_time();
    if(config.is_polled_by_server == false){

        /* Start counting down to the time for polling health reports and
           tracking object data */
        last_polling_LBeacon_for_HR_time = current_time;
        last_polling_object_tracking_time = current_time;
    }else{
        last_polling_join_request_time = get_system_time();
    }

	last_polling_time = 0;

    /* The while loop that keeps the program running */
    while(ready_to_work == true){

        current_time = get_system_time();

        if(config.is_polled_by_server == false){

            /* If it is the time to poll LBeacons for tracked object data, get a
               thread to do this work */
            if(current_time - last_polling_object_tracking_time >
               config.period_between_RFTOD){

                /* Poll object tracking object data */
                /* set the pkt content */
                int send_msg_type = ((from_gateway & 0x0f) << 4) +
                                 (tracked_object_data & 0x0f);
                char temp[MINIMUM_WIFI_MESSAGE_LENGTH];
                memset(temp, 0, MINIMUM_WIFI_MESSAGE_LENGTH);

                temp[0] = (char)send_msg_type;

                /* broadcast to LBeacons */
                beacon_broadcast(&LBeacon_address_map, temp,
                                 MINIMUM_WIFI_MESSAGE_LENGTH);

                zlog_info(category_debug,
                          "Polling Tracked Object Data from Gateway");

                /* Update the last_polling_object_tracking_time */
                last_polling_object_tracking_time = current_time;
            }
            else if(current_time - last_polling_LBeacon_for_HR_time >
                    config.period_between_RFHR){

                /* Polling for health reports. */
                /* set the pkt type */
                int send_msg_type = ((from_gateway & 0x0f) << 4) +
                                     (health_report & 0x0f);
                char temp[MINIMUM_WIFI_MESSAGE_LENGTH];
                memset(temp, 0, MINIMUM_WIFI_MESSAGE_LENGTH);

                temp[0] = (char)send_msg_type;

                /* broadcast to LBeacons */
                beacon_broadcast(&LBeacon_address_map, temp,
                                 MINIMUM_WIFI_MESSAGE_LENGTH);

                zlog_info(category_debug, "Polling Health Report from Gateway");


                /* Update the last_polling_LBeacon_for_HR_time */
                last_polling_LBeacon_for_HR_time = get_system_time();
            }
            else{
                usleep(BUSY_WAITING_TIME);
            }
        }


        if(current_time - last_polling_time > JOIN_REQUEST_TIMEOUT){
            /* Join Request */
            /* set the pkt type */
            int send_type = ((from_gateway & 0x0f) << 4) +
                             (request_to_join & 0x0f);

            char content_temp[WIFI_MESSAGE_LENGTH];
            memset(content_temp, 0, WIFI_MESSAGE_LENGTH);

            int content_temp_size = MINIMUM_WIFI_MESSAGE_LENGTH;

            content_temp[0] = (char)send_type;

            content_temp[1] = ';';

            pthread_mutex_lock(&LBeacon_address_map.list_lock);

            char content_LBeacon_status[WIFI_MESSAGE_LENGTH];
            memset(content_LBeacon_status, 0, WIFI_MESSAGE_LENGTH);

            int content_LBeacon_status_size = 0;
            int counter = 0;

            for(int n = 0; n < MAX_NUMBER_NODES; n ++){
                if (LBeacon_address_map.in_use[n] == true){

                    char tmp[WIFI_MESSAGE_LENGTH];
                    memset(tmp, 0, WIFI_MESSAGE_LENGTH);

                    char uuid[UUID_LENGTH + 1];
                    memset(uuid, 0, UUID_LENGTH + 1);

                    memcpy(uuid, LBeacon_address_map.address_map_list[n].
                           uuid, UUID_LENGTH);
                    sprintf(tmp, "%s;%d;", uuid, LBeacon_address_map.
                            address_map_list[n].last_request_time);

                    memcpy(&content_LBeacon_status
                           [content_LBeacon_status_size], tmp, strlen(tmp) *
                           sizeof(char));

                    content_LBeacon_status_size += strlen(tmp);

                    counter ++;
                }
            }

            char tmp[WIFI_MESSAGE_LENGTH];
            memset(tmp, 0, WIFI_MESSAGE_LENGTH);

            sprintf(tmp, "%d;%s;", counter, config.IPaddress);

            memcpy(&content_temp[content_temp_size], tmp, strlen(tmp) *
                                                          sizeof(char));

            content_temp_size += strlen(tmp);

            memcpy(&content_temp[content_temp_size], content_LBeacon_status,
                   content_LBeacon_status_size * sizeof(char));

            content_temp_size += content_LBeacon_status_size;

#ifdef debugging
            zlog_info(category_debug, "Current Joined LBeacon:"
                      "Content_temp [%s]", &content_temp[1]);
#endif
            pthread_mutex_unlock(&LBeacon_address_map.list_lock);

            if(content_temp_size < WIFI_MESSAGE_LENGTH)
                /* broadcast to LBeacons */
                udp_addpkt( &udp_config, config.server_ip, content_temp,
                            content_temp_size);

            last_polling_join_request_time = current_time;
        }else{
            usleep(BUSY_WAITING_TIME);
        }

    }

    /* The program is going to be ended. Free the connection of Wifi */
    Wifi_free();

#ifdef debugging
    zlog_info(category_debug, "Gateway exit successfullly");
#endif

    return WORK_SUCCESSFULLY;
}


ErrorCode get_config(GatewayConfig *config, char *file_name) {

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        /* Error handling */
        zlog_error(category_health_report, "Open config file fail.");
        return E_OPEN_FILE;
    }
    else {

        /* Create spaces for storing the string in the current line being read*/
        char  config_setting[CONFIG_BUFFER_SIZE];
        char *config_message = NULL;
        int config_message_size = 0;

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->is_polled_by_server = atoi(config_message);

        /* Keep reading each line and store into the config struct */
        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        if(config_message[strlen(config_message)-1] == '\n')
            config_message_size = strlen(config_message) - 1;
        else
            config_message_size = strlen(config_message);

        memcpy(config->IPaddress, config_message, config_message_size);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->allowed_number_nodes = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->period_between_RFHR = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->period_between_RFTOD = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->period_between_join_request = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->number_worker_threads = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        if(config_message[strlen(config_message)-1] == '\n')
            config_message_size = strlen(config_message) - 1;
        else
            config_message_size = strlen(config_message);
        memcpy(config->server_ip, config_message, config_message_size);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->send_port = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->recv_port = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->critical_priority = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->high_priority = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->normal_priority = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->low_priority = atoi(config_message);

        fclose(file);

    }
    return WORK_SUCCESSFULLY;
}


void init_buffer(BufferListHead *buffer_list_head, void (*function_p)(void *),
                 int priority_nice){

    init_entry( &(buffer_list_head -> list_head));

    init_entry( &(buffer_list_head -> priority_list_entry));

    pthread_mutex_init( &buffer_list_head->list_lock, 0);

    buffer_list_head -> function = function_p;

    buffer_list_head -> arg = (void *) buffer_list_head;

    buffer_list_head -> priority_nice = priority_nice;
}


void *sort_priority_list(GatewayConfig *config, BufferListHead *list_head){

    List_Entry *list_pointer,
               *next_list_pointer;

    List_Entry critical_priority_entry, high_priority_entry,
               normal_priority_entry, low_priority_entry;

    BufferListHead *current_head, *next_head;

    init_entry( &critical_priority_entry);
    init_entry( &high_priority_entry);
    init_entry( &normal_priority_entry);
    init_entry( &low_priority_entry);

    pthread_mutex_lock( &list_head -> list_lock);

    list_for_each_safe(list_pointer, next_list_pointer,
                       &list_head -> priority_list_entry){

        remove_list_node(list_pointer);

        current_head = ListEntry(list_pointer, BufferListHead,
                                 priority_list_entry);

        if(current_head -> priority_nice == config -> critical_priority)

            insert_list_tail( list_pointer, &critical_priority_entry);

        else if(current_head -> priority_nice == config -> high_priority)

            insert_list_tail( list_pointer, &high_priority_entry);

        else if(current_head -> priority_nice == config -> normal_priority)

            insert_list_tail( list_pointer, &normal_priority_entry);

        else if(current_head -> priority_nice == config -> low_priority)

            insert_list_tail( list_pointer, &low_priority_entry);

    }

    if(is_entry_list_empty(&critical_priority_entry) == false){
        list_pointer = critical_priority_entry.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    if(is_entry_list_empty(&high_priority_entry) == false){
        list_pointer = high_priority_entry.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    if(is_entry_list_empty(&normal_priority_entry) == false){
        list_pointer = normal_priority_entry.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    if(is_entry_list_empty(&low_priority_entry) == false){
        list_pointer = low_priority_entry.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    pthread_mutex_unlock( &list_head -> list_lock);

}


void *CommUnit_routine(){

    int init_time;
    int current_time;
    Threadpool thpool;
    int return_error_value;

    /* wait for NSI get ready */
    while(NSI_initialization_complete == false){
        usleep(BUSY_WAITING_TIME);
        if(initialization_failed == true){
            return (void *)NULL;
        }
    }

    /* Initialize the threadpool with specified number of worker threads
       according to the data stored in the config file. */
    thpool = thpool_init(config.number_worker_threads);

    current_time = get_system_time();

    /* Set the initial time. */
    init_time = current_time;

    /* All the buffers lists have been are initialized and the thread pool
       initialized. Set the flag to true. */
    CommUnit_initialization_complete = true;

    /* When there is no dead thead, do the work. */
    while(ready_to_work == true){

        current_time = get_system_time();

        List_Entry *tmp, *list_entry;
        BufferNode *current_node;
        BufferListHead *current_head;

        /* In the normal situation, the scanning starts from the high priority
           to lower priority. When the timer expired for MAX_STARVATION_TIME,
           reverse the scanning process */
        while(current_time - init_time < MAX_STARVATION_TIME){

            while(thpool -> num_threads_working == thpool -> num_threads_alive){
                usleep(BUSY_WAITING_TIME);
            }

            /* Scan the priority_list to get the buffer list with the highest
               priority among all lists that are not empty. */

            pthread_mutex_lock( &priority_list_head.list_lock);

            list_for_each(tmp, &priority_list_head.priority_list_entry){

                current_head = ListEntry(tmp, BufferListHead,
                                        priority_list_entry);

                pthread_mutex_lock( &current_head -> list_lock);

                if (is_entry_list_empty( &current_head->list_head) == true){

                    pthread_mutex_unlock( &current_head -> list_lock);
                    /* Go to check the next buffer list in the priority list */

                    continue;
                }
                else {

                    list_entry = current_head -> list_head.next;

                    remove_list_node(list_entry);

                    pthread_mutex_unlock( &current_head -> list_lock);

                    current_node = ListEntry(list_entry, BufferNode,
                                             buffer_entry);

                    /* If there is a node in the buffer and the buffer is not be
                       occupied, do the work according to the function pointer
                     */
                    return_error_value = thpool_add_work(thpool,
                                                current_head -> function,
                                                current_node,
                                                current_head -> priority_nice);

                    pthread_mutex_unlock( &current_head -> list_lock);

                    break;
                }
            }

            pthread_mutex_unlock( &priority_list_head.list_lock);

            current_time = get_system_time();

            usleep(BUSY_WAITING_TIME);

        }

        while(thpool -> num_threads_working == thpool -> num_threads_alive){
            usleep(BUSY_WAITING_TIME);
        }
        /* Scan the priority list in reverse order to prevent starving the
           lowest priority buffer list. */

        pthread_mutex_lock( &priority_list_head.list_lock);

        list_for_each_reverse(tmp, &priority_list_head.priority_list_entry){

            current_head= ListEntry(tmp, BufferListHead, priority_list_entry);

            pthread_mutex_lock( &current_head -> list_lock);

            if (is_entry_list_empty( &current_head->list_head) == true){

                pthread_mutex_unlock( &current_head -> list_lock);
                /* Go to check the next buffer list in the priority list */

                continue;
            }
            else {

                list_entry = current_head -> list_head.next;

                remove_list_node(list_entry);

                pthread_mutex_unlock( &current_head -> list_lock);

                current_node = ListEntry(list_entry, BufferNode,
                                         buffer_entry);

                /* If there is a node in the buffer and the buffer is not be
                   occupied, do the work according to the function pointer */
                return_error_value = thpool_add_work(thpool,
                                            current_head -> function,
                                            current_node,
                                            current_head -> priority_nice);

                pthread_mutex_unlock( &current_head -> list_lock);

                break;

            }
        }

        pthread_mutex_unlock( &priority_list_head.list_lock);

        /* Update the init_time */
        init_time = get_system_time();

        usleep(BUSY_WAITING_TIME);

    } /* End while(ready_to_work == true) */


    /* Destroy the thread pool */
    thpool_destroy(thpool);

    return (void *)NULL;
}


void *NSI_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

    char current_uuid[UUID_LENGTH];

    memcpy(current_uuid, &temp->content[1], UUID_LENGTH);

    int send_type = (from_gateway & 0x0f)<<4;

    /* Put the address into LBeacon_address_map and set the return pkt type
     */
    if (beacon_join_request(&LBeacon_address_map, current_uuid, temp ->
                            net_address) == true)
        send_type += join_request_ack & 0x0f;
    else
        send_type += join_request_deny & 0x0f;

    /* put the pkt type to content */
    temp->content[0] = (char)send_type;

    pthread_mutex_lock(&NSI_send_buffer_list_head.list_lock);

    insert_list_tail( &temp->buffer_entry,
                      &NSI_send_buffer_list_head.list_head);

    pthread_mutex_unlock( &NSI_send_buffer_list_head.list_lock);

    return (void *)NULL;
}


void *BHM_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

    /* Add the content of the buffer node to the UDP to be sent to the
       Server */
    udp_addpkt( &udp_config, config.server_ip, temp -> content,
                temp -> content_size);

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}


void *LBeacon_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

    /* Add the content of tje buffer node to the UDP to be sent to the
       Server */
    udp_addpkt( &udp_config, config.server_ip, temp -> content,
                temp -> content_size);

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}


void *Server_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

    zlog_info(category_debug, "Start Broadcast to LBeacon");

    beacon_broadcast(&LBeacon_address_map, temp -> content, temp ->
                     content_size);

    zlog_info(category_debug, "Polling Data from Server");

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}


void init_Address_Map(AddressMapArray *address_map){

    pthread_mutex_init( &address_map -> list_lock, 0);

    memset(address_map -> address_map_list, 0,
           sizeof(address_map -> address_map_list));

    for(int n = 0; n < MAX_NUMBER_NODES; n ++)
        address_map -> in_use[n] = false;
}


int is_in_Address_Map(AddressMapArray *address_map, char *uuid){

    for(int n = 0;n < MAX_NUMBER_NODES;n ++){

        if (address_map -> in_use[n] == true && strncmp(address_map ->
            address_map_list[n].uuid, uuid, UUID_LENGTH) == 0){
                return n;
        }
    }
    return -1;
}


bool beacon_join_request(AddressMapArray *address_map, char *uuid,
                         char *address){

    pthread_mutex_lock( &address_map -> list_lock);
    /* Copy all the necessary information received from the LBeacon to the
       address map. */

    /* Find the first unused address map location and use the location to store
       the new joined LBeacon. */
    int not_in_use = -1;
    int answer;

    if(answer = is_in_Address_Map(address_map, uuid) >=0){
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

    /* If still has space for the LBeacon to register */
    if (not_in_use != -1){

        AddressMap *tmp =  &address_map -> address_map_list[not_in_use];

        address_map -> in_use[not_in_use] = true;

        strncpy(tmp -> uuid, uuid, UUID_LENGTH);
        strncpy(tmp -> net_address, address, NETWORK_ADDR_LENGTH);
        tmp -> last_request_time = get_system_time();
        pthread_mutex_unlock( &address_map -> list_lock);
        return true;
    }
    else{
        pthread_mutex_unlock( &address_map -> list_lock);
        return false;
    }

    return false;
}


void beacon_broadcast(AddressMapArray *address_map, char *msg, int size){

    pthread_mutex_lock( &address_map -> list_lock);

    zlog_info(category_debug, "==Current in Brocast==");

    if (size <= WIFI_MESSAGE_LENGTH){
        for(int n = 0;n < MAX_NUMBER_NODES;n ++){

            if (address_map -> in_use[n] == true){
                zlog_info(category_debug, "Brocast IP: %s", address_map ->
                                          address_map_list[n].net_address);

                /* Add the pkt that to be sent to the server */
                udp_addpkt( &udp_config, address_map -> address_map_list[n]
                            .net_address, msg, size);
            }
        }
    }

    zlog_info(category_debug, "END Brocast");
    pthread_mutex_unlock( &address_map -> list_lock);
}


ErrorCode Wifi_init(char *IPaddress){

    /* Set the address of server */
    array_copy(IPaddress, udp_config.Local_Address, strlen(IPaddress));

    /* Initialize the Wifi cinfig file */
    if(udp_initial( &udp_config, config.send_port, config.recv_port)
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
    udp_addpkt( &udp_config, temp -> net_address, temp->content,
                temp->content_size);

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}


void *process_wifi_receive(){

    while (ready_to_work == true) {

        BufferNode *new_node;

        sPkt temppkt = udp_getrecv( &udp_config);

        if(temppkt.type == UDP){

            /* counting test time for mp_alloc(). */
            int test_times = 0;

            int current_time = get_system_time();

            /* Allocate memory from node_mempool a buffer node for received data
               and copy the data from Wi-Fi receive queue to the node. */
            do{
                if(test_times == TEST_MALLOC_MAX_NUMBER_TIMES)
                    break;
                else if(test_times != 0)
                    usleep(BUSY_WAITING_TIME);

                new_node = mp_alloc( &node_mempool);
                test_times ++;

            }while( new_node == NULL);

            if(new_node == NULL){
                /* Alloc memory failed, error handling. */
            }
            else{

                memset(new_node, 0, sizeof(BufferNode));

                /* Initialize the entry of the buffer node */
                init_entry( &new_node -> buffer_entry);

                /* Copy the content to the buffer_node */
                memcpy(new_node -> content, temppkt.content
                     , temppkt.content_size);

                new_node -> content_size = temppkt.content_size;

                char *tmp_addr = udp_hex_to_address(temppkt.address);

                memcpy(new_node -> net_address, tmp_addr, NETWORK_ADDR_LENGTH);

                free(tmp_addr);

                /* read the pkt direction from higher 4 bits. */
                int pkt_direction = (new_node -> content[0] >> 4) & 0x0f;
                /* read the pkt type from lower lower 4 bits. */
                int pkt_type = new_node -> content[0] & 0x0f;

                /* Insert the node to the specified buffer, and release
                   list_lock. */
                switch (pkt_direction) {
                    case from_server:
                        last_polling_time = get_system_time();
                        switch (pkt_type) {

                            case RFHR_from_server:
                            case health_report:
                                zlog_info(category_debug,
                                         "Get Health Report from the Server");
                                last_polling_LBeacon_for_HR_time = current_time;
                                pthread_mutex_lock(&command_msg_buffer_list_head
                                                   .list_lock);
                                insert_list_tail(&new_node -> buffer_entry,
                                                 &command_msg_buffer_list_head
                                                 .list_head);
                                pthread_mutex_unlock(
                                       &command_msg_buffer_list_head.list_lock);

                                break;

                            case poll_for_tracked_object_data_from_server:
                            case tracked_object_data:
                                zlog_info(category_debug,
                                   "Get Tracked Object Data from the Server");
                                last_polling_object_tracking_time= current_time;
                                pthread_mutex_lock(&command_msg_buffer_list_head
                                                   .list_lock);
                                insert_list_tail( &new_node -> buffer_entry,
                                       &command_msg_buffer_list_head.list_head);
                                pthread_mutex_unlock(
                                       &command_msg_buffer_list_head.list_lock);
                                break;

                            case join_request_ack:
                                zlog_info(category_debug,
                                     "Get Join Request Result from the Server");
                                last_polling_join_request_time =
                                                              get_system_time();
                                mp_free(&node_mempool, new_node);
                                break;
                            case data_for_LBeacon:
                                zlog_info(category_debug,
                                          "Get Data_for_LBeacon");
                                mp_free(&node_mempool, new_node);
                                break;

                            default:
                                mp_free(&node_mempool, new_node);
                                break;
                        }
                        break;

                    case from_beacon:

                        switch (pkt_type) {

                            case request_to_join:
                                zlog_info(category_debug,
                                             "Get Join Request from LBeacon");
                                pthread_mutex_lock(&NSI_receive_buffer_list_head
                                                   .list_lock);
                                insert_list_tail(&new_node -> buffer_entry,
                                                 &NSI_receive_buffer_list_head
                                                 .list_head);
                                pthread_mutex_unlock(
                                       &NSI_receive_buffer_list_head.list_lock);
                                break;

                            case tracked_object_data:
                                zlog_info(category_debug,
                                      "Get Tracked Object Data from LBeacon");
                                pthread_mutex_lock(
                                   &LBeacon_receive_buffer_list_head.list_lock);
                                insert_list_tail( &new_node -> buffer_entry,
                                   &LBeacon_receive_buffer_list_head.list_head);
                                pthread_mutex_unlock(
                                   &LBeacon_receive_buffer_list_head.list_lock);
                                break;

                            case health_report:
                                zlog_info(category_debug,
                                            "Get Health Report from LBeacon");
                                pthread_mutex_lock(&BHM_receive_buffer_list_head
                                                   .list_lock);
                                insert_list_tail( &new_node -> buffer_entry,
                                       &BHM_receive_buffer_list_head.list_head);
                                pthread_mutex_unlock(
                                       &BHM_receive_buffer_list_head.list_lock);
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
            }
        }
        else if(temppkt.type == NONE){
            /* If there is no packet received, sleep a short time */
            usleep(BUSY_WAITING_TIME);
        }
        else {
            /* If there is no packet received, sleep a short time */
            usleep(BUSY_WAITING_TIME);
        }
    }
    return (void *)NULL;
}
