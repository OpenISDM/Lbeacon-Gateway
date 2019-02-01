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

     This file contains programs to transmit and receive data to and from
     LBeacon and the sever through Wi-Fi network from and to the Gateway, and
     programs executed by network setup and initialization, Beacon health
     monitor and comminication unit. Each gateway is the root of a star network
     of LBeacons.

  Version:

     1.0, 20190117

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

    /* The main thread do the communication Unit */
    pthread_t CommUnit_thread;

    /* The thread to listen for messages from Wi-Fi */
    pthread_t wifi_listener;

    /* Reset all flags */
    NSI_initialization_complete      = false;
    CommUnit_initialization_complete = false;
    BHM_initialization_complete      = false;

    initialization_failed = false;

    ready_to_work = true;


    /* Reading the config */

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

    /* Initialize buffer_list_heads and add to the head in to the priority list.
     */

    init_Address_Map( &LBeacon_address_map);

    init_buffer( &priority_list_head, (void *) sort_priority,
                config.high_priority);

    init_buffer( &time_critical_LBeacon_receive_buffer_list_head,
                (void *) LBeacon_routine, config.high_priority);
    insert_list_tail( &time_critical_LBeacon_receive_buffer_list_head
                     .priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &command_msg_buffer_list_head,
                (void *) Server_routine, config.normal_priority);
    insert_list_tail( &command_msg_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &LBeacon_receive_buffer_list_head,
                (void *) LBeacon_routine, config.high_priority);
    insert_list_tail( &LBeacon_receive_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &NSI_send_buffer_list_head,
                (void *) process_wifi_send, config.normal_priority);
    insert_list_tail( &NSI_send_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &NSI_receive_buffer_list_head,
                (void *) NSI_routine, config.normal_priority);
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

    sort_priority( &priority_list_head);

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

    /* Create the thread of Communication Unit  */
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
          CommUnit_initialization_complete == false ||
          BHM_initialization_complete == false){

        sleep(WAITING_TIME);

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

        /* Start counting down the time for polling the health reports and
           tracking object data */
        last_polling_LBeacon_for_HR_time = current_time;
        last_polling_object_tracking_time = current_time;
    }

    /* The while loop that keeps the program running */
    while(ready_to_work == true){

        if(config.is_polled_by_server == false){

            current_time = get_system_time();

            /* If it is the time to poll health reports from LBeacons, get a
               thread to do this work */
            if(current_time - last_polling_object_tracking_time >
               config.period_between_RFTOD){

                /* Pull object tracking object data */
                /* set the pkt type */
                int send_type = ((from_gateway & 0x0f) << 4) +
                                 (tracked_object_data & 0x0f);
                char temp[MINIMUM_WIFI_MESSAGE_LENGTH];
                memset(temp, 0, MINIMUM_WIFI_MESSAGE_LENGTH);

                temp[0] = (char)send_type;

                /* broadcast to LBeacons */
                beacon_broadcast(&LBeacon_address_map, temp,
                                 MINIMUM_WIFI_MESSAGE_LENGTH);

                /* Update the last_polling_object_tracking_time */
                last_polling_object_tracking_time = current_time;
            }
            else if(current_time - last_polling_LBeacon_for_HR_time >
                    config.period_between_RFHR){

                /* Polling for health reports. */
                /* set the pkt type */
                int send_type = ((from_gateway & 0x0f) << 4) +
                                 (health_report & 0x0f);
                char temp[MINIMUM_WIFI_MESSAGE_LENGTH];
                memset(temp, 0, MINIMUM_WIFI_MESSAGE_LENGTH);

                temp[0] = (char)send_type;

                /* broadcast to LBeacons */
                beacon_broadcast(&LBeacon_address_map, temp,
                                 MINIMUM_WIFI_MESSAGE_LENGTH);

                /* Update the last_polling_LBeacon_for_HR_time */
                last_polling_LBeacon_for_HR_time = get_system_time();
            }
        }

        sleep(WAITING_TIME);

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


void *sort_priority(BufferListHead *list_head){

    List_Entry *list_pointers,
               *list_pointers_in_bubble,
               *save_list_pointers_in_bubble,
               tmp;

    BufferListHead *current_head, *next_head;

    pthread_mutex_lock( &list_head -> list_lock);

    list_for_each(list_pointers,  &list_head -> priority_list_entry){

        list_for_each_safe_reverse(list_pointers_in_bubble,
                                   save_list_pointers_in_bubble,
                                    &list_head -> priority_list_entry){

            current_head = ListEntry(list_pointers_in_bubble, BufferListHead,
                                     priority_list_entry);
            next_head = ListEntry(save_list_pointers_in_bubble, BufferListHead,
                                  priority_list_entry);

            /* save_list_pointers_in_bubble <-> list_pointers_in_bubble <-> head
             */

            if(current_head -> priority_nice < next_head -> priority_nice){

                tmp.prev = save_list_pointers_in_bubble -> prev;
                tmp.next = list_pointers_in_bubble -> next;

                list_pointers_in_bubble -> prev = tmp.prev;
                list_pointers_in_bubble -> next = save_list_pointers_in_bubble;

                save_list_pointers_in_bubble -> next = tmp.next;
                save_list_pointers_in_bubble -> prev = list_pointers_in_bubble;

                list_pointers_in_bubble -> prev -> next =
                                                        list_pointers_in_bubble;
                save_list_pointers_in_bubble -> next -> prev =
                                                   save_list_pointers_in_bubble;

                save_list_pointers_in_bubble = list_pointers_in_bubble;
            }

            if(list_pointers_in_bubble == list_pointers){
                break;
            }
        }
    }

    pthread_mutex_unlock( &list_head -> list_lock);
}


void* CommUnit_routine(){

    int init_time;
    int current_time;
    Threadpool thpool;
    int return_error_value;

    /* wait for NSI get ready */
    while(NSI_initialization_complete == false){
        sleep(WAITING_TIME);
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

    /* After all the buffers are initialized and the thread pool initialized,
       set the flag to true. */
    CommUnit_initialization_complete = true;

    /* When there is no dead thead, do the work. */
    while(ready_to_work == true){

        current_time = get_system_time();

        List_Entry *tmp;
        BufferListHead *current_head;
        bool is_empty_list_head;


        /* In the normal situation, the scanning starts from the high priority
           to lower priority. If the timer expired for MAX_STARVATION_TIME,
           reverse the scanning process */
        while(current_time - init_time < MAX_STARVATION_TIME){

            while(thpool -> num_threads_working == thpool -> num_threads_alive){
                sleep(WAITING_TIME);
            }

            /* Scan the priority_list to get the buffer list with the highest
               priority among all lists that are not empty. */

            pthread_mutex_lock( &priority_list_head.list_lock);

            list_for_each(tmp, &priority_list_head.priority_list_entry){

                is_empty_list_head = true;

                current_head= ListEntry(tmp, BufferListHead,
                                        priority_list_entry);

                pthread_mutex_lock( &current_head -> list_lock);

                if (is_entry_list_empty( &current_head->list_head) == true){

                    pthread_mutex_unlock( &current_head -> list_lock);
                    /* Go to check the next buffer list in the priority list */
                    continue;
                }
                else {

                    is_empty_list_head = false;

                    /* Currently, a work thread is processing this buffer list.
                     */
                    pthread_mutex_unlock( &current_head -> list_lock);
                    /* Go to check the next buffer list in the priority
                        list */
                    break;
                }
            }

            pthread_mutex_unlock( &priority_list_head.list_lock);

            if (is_empty_list_head == false){

                /* If there is a node in the buffer and the buffer is not be
                   occupied, do the work according to the function pointer */
                return_error_value = thpool_add_work(thpool,
                                                     current_head -> function,
                                                     current_head,
                                                     current_head ->
                                                     priority_nice);

            }

            current_time = get_system_time();
        }

        while(thpool -> num_threads_working == thpool -> num_threads_alive){
            sleep(WAITING_TIME);
        }

        /* Scan the priority list in reverse order to prevent starving the
           lowest priority buffer list. */

        pthread_mutex_lock( &priority_list_head.list_lock);

        list_for_each_reverse(tmp, &priority_list_head.priority_list_entry){

            is_empty_list_head = true;

            current_head= ListEntry(tmp, BufferListHead, priority_list_entry);

            pthread_mutex_lock( &current_head -> list_lock);

            if (is_entry_list_empty( &current_head->list_head) == true){

                pthread_mutex_unlock( &current_head -> list_lock);
                /* Go to check the next buffer list in the priority list */
                continue;
            }
            else {

                is_empty_list_head = false;

                /* Currently, a work thread is processing this buffer list. */
                pthread_mutex_unlock( &current_head -> list_lock);
                /* Go to check the next buffer list in the priority list */
                break;
            }
        }

        pthread_mutex_unlock( &priority_list_head.list_lock);

        if (is_empty_list_head == false){
            /* If there is a node in the buffer and the buffer is not be
               occupied, do the work according to the function pointer */
            return_error_value = thpool_add_work(thpool,
                                                 current_head -> function,
                                                 current_head,
                                                 current_head ->
                                                 priority_nice);
        }

        /* Update the init_time */
        init_time = get_system_time();
    } /* End while(ready_to_work == true) */


    /* Destroy the thread pool */
    thpool_destroy(thpool);

    return (void *)NULL;
}


void *NSI_routine(void *_buffer_list_head){

    BufferListHead *buffer_list_head = (BufferListHead *)_buffer_list_head;

    struct List_Entry *temp_list_entry_pointers;

    BufferNode *temp;

    pthread_mutex_lock( &buffer_list_head -> list_lock);

    if(is_entry_list_empty( &buffer_list_head -> list_head) == false){

        temp_list_entry_pointers = buffer_list_head -> list_head.next;

        remove_list_node(temp_list_entry_pointers);

        pthread_mutex_unlock( &buffer_list_head -> list_lock);

        temp = ListEntry(temp_list_entry_pointers, BufferNode, buffer_entry);

        char current_uuid[UUID_LENGTH];

        memcpy(current_uuid, &temp->content[2], UUID_LENGTH);

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
    }
    else
        pthread_mutex_unlock( &buffer_list_head -> list_lock);

    return (void *)NULL;
}


void *BHM_routine(void *_buffer_list_head){

    BufferListHead *buffer_list_head = (BufferListHead *)_buffer_list_head;

    /* Create a temporary node and set as the head */
    struct List_Entry *temp_list_entry_pointers;

    BufferNode *temp;

    pthread_mutex_lock( &buffer_list_head->list_lock);

    if(is_entry_list_empty( &buffer_list_head -> list_head) == false){

        temp_list_entry_pointers = buffer_list_head -> list_head.next;

        remove_list_node(temp_list_entry_pointers);

        pthread_mutex_unlock( &buffer_list_head -> list_lock);

        temp = ListEntry(temp_list_entry_pointers, BufferNode, buffer_entry);

        /* TODO Make a buffer to merge all the HR pkt and wait for polling. */

        insert_list_tail( &temp -> buffer_entry,
                          &BHM_send_buffer_list_head.list_head);
    }
    else
        pthread_mutex_unlock( &buffer_list_head -> list_lock);

    return (void *)NULL;
}


void *LBeacon_routine(void *_buffer_list_head){

    BufferListHead *buffer_list_head = (BufferListHead *)_buffer_list_head;

    /* Create a temporary node and set as the head */
    struct List_Entry *temp_list_entry_pointers;

    BufferNode *temp;

    pthread_mutex_lock( &buffer_list_head -> list_lock);

    if(is_entry_list_empty( &buffer_list_head -> list_head) == false){

        temp_list_entry_pointers = buffer_list_head -> list_head.next;

        remove_list_node(temp_list_entry_pointers);

        pthread_mutex_unlock( &buffer_list_head -> list_lock);

        temp = ListEntry(temp_list_entry_pointers, BufferNode, buffer_entry);

        /* Add the content that to be sent to the server */
        udp_addpkt( &udp_config, config.server_ip, temp -> content,
                    temp -> content_size);

        mp_free( &node_mempool, temp);
    }
    else
        pthread_mutex_unlock( &buffer_list_head -> list_lock);

    return (void* )NULL;
}


void *Server_routine(void *_buffer_list_head){

    BufferListHead *buffer_list_head = (BufferListHead *)_buffer_list_head;

    /* Create a temporary node and set as the head */
    struct List_Entry *temp_list_entry_pointers;

    BufferNode *temp;

    pthread_mutex_lock( &buffer_list_head -> list_lock);

    if(is_entry_list_empty( &buffer_list_head -> list_head) == false){

        temp_list_entry_pointers = buffer_list_head -> list_head.next;

        remove_list_node(temp_list_entry_pointers);

        pthread_mutex_unlock( &buffer_list_head -> list_lock);

        temp = ListEntry(temp_list_entry_pointers, BufferNode, buffer_entry);

        beacon_broadcast(&LBeacon_address_map, temp -> content, temp ->
                         content_size);

        mp_free( &node_mempool, temp);
    }
    else
        pthread_mutex_unlock( &buffer_list_head -> list_lock);

    return (void* )NULL;
}


void init_Address_Map(AddressMapArray *address_map){

    pthread_mutex_init( &address_map -> list_lock, 0);

    memset(address_map -> address_map_list, 0,
           sizeof(address_map -> address_map_list));

    for(int n = 0; n < MAX_NUMBER_NODES; n ++)
        address_map -> in_use[n] = false;
}


bool is_in_Address_Map(AddressMapArray *address_map, char *uuid){

    for(int n = 0;n < MAX_NUMBER_NODES;n ++){

        if (address_map -> in_use[n] == true && strncmp(address_map ->
            address_map_list[n].uuid, uuid, UUID_LENGTH) == 0){
                return true;
        }
    }
    return false;
}


bool beacon_join_request(AddressMapArray *address_map, char *uuid,
                         char *address){

    pthread_mutex_lock( &address_map -> list_lock);
    /* Copy all the necessary information received from the LBeacon to the
       address map. */

    /* Record the first unused address map location in order to store the new
       joined LBeacon. */
    int not_in_use = -1;

    if(is_in_Address_Map(address_map, uuid) == true){
        for(int n = 0 ; n < MAX_NUMBER_NODES ; n ++){
            if(address_map -> in_use[n] == true && strncmp(address_map ->
               address_map_list[n].uuid, uuid, UUID_LENGTH) == 0){
                address_map -> address_map_list[n].last_request_time =
                                                              get_system_time();
                break;
            }
        }
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

    if (size <= WIFI_MESSAGE_LENGTH){
        for(int n = 0;n < MAX_NUMBER_NODES;n ++){

            if (address_map -> in_use[n] == true){
                /* Add the pkt that to be sent to the server */
                udp_addpkt( &udp_config, address_map -> address_map_list[n]
                            .net_address, msg, size);
            }
        }
    }

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


void *process_wifi_send(void *_buffer_list_head){

    BufferListHead *buffer_list_head = (BufferListHead *)_buffer_list_head;

    struct List_Entry *temp_list_entry_pointers;

    BufferNode *temp;

    pthread_mutex_lock( &buffer_list_head -> list_lock);

    if(is_entry_list_empty( &buffer_list_head -> list_head) == false){

        temp_list_entry_pointers = buffer_list_head -> list_head.next;

        remove_list_node(temp_list_entry_pointers);

        pthread_mutex_unlock( &buffer_list_head -> list_lock);

        temp = ListEntry(temp_list_entry_pointers, BufferNode, buffer_entry);

        /* Add the content that to be sent to the server */
        udp_addpkt( &udp_config, temp -> net_address, temp->content,
                   temp->content_size);

        mp_free( &node_mempool, temp);
    }
    else
        pthread_mutex_unlock( &buffer_list_head -> list_lock);

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
                if(test_times == test_malloc_max_number_times)
                    break;
                else if(test_times != 0)
                    sleep(1);

                new_node = mp_alloc( &node_mempool);
                test_times ++;

            }while( new_node == NULL);

            if(new_node == NULL){
                /* Alloc memory failed, error handling. */
            }
            else{
                /* Initialize the entry of the buffer node */
                init_entry( &new_node -> buffer_entry);

                /* Copy the content to the buffer_node */
                memcpy(new_node -> content, temppkt.content
                     , temppkt.content_size);

                new_node -> content_size = temppkt.content_size;

                char *tmp_addr = udp_hex_to_address(temppkt.address);

                memcpy(new_node -> net_address, tmp_addr, NETWORK_ADDR_LENGTH);

                /* read the pkt direction from higher 4 bits. */
                int pkt_direction = (new_node -> content[0] >> 4) & 0x0f;
                /* read the pkt type from lower lower 4 bits. */
                int pkt_type = new_node -> content[0] & 0x0f;

                /* Insert the node to the specified buffer, and release
                   list_lock. */
                switch (pkt_direction) {
                    case from_server:

                        switch (pkt_type) {

                            case RFHR_from_server:
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
                                last_polling_object_tracking_time= current_time;
                                pthread_mutex_lock(&command_msg_buffer_list_head
                                                   .list_lock);
                                insert_list_tail( &new_node -> buffer_entry,
                                       &command_msg_buffer_list_head.list_head);
                                pthread_mutex_unlock(
                                       &command_msg_buffer_list_head.list_lock);
                                break;

                            case data_for_LBeacon:
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
                                pthread_mutex_lock(&NSI_receive_buffer_list_head
                                                   .list_lock);
                                insert_list_tail(&new_node -> buffer_entry,
                                                 &NSI_receive_buffer_list_head
                                                 .list_head);
                                pthread_mutex_unlock(
                                       &NSI_receive_buffer_list_head.list_lock);
                                break;

                            case tracked_object_data:
                                pthread_mutex_lock(
                                   &LBeacon_receive_buffer_list_head.list_lock);
                                insert_list_tail( &new_node -> buffer_entry,
                                   &LBeacon_receive_buffer_list_head.list_head);
                                pthread_mutex_unlock(
                                   &LBeacon_receive_buffer_list_head.list_lock);
                                break;

                            case health_report:
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
            sleep(WAITING_TIME);
        }
    }
    return (void *)NULL;
}
