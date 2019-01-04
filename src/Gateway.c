/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and conditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

  File Description:

     This file contains the program to transmit and receive data to and from
     LBeacon and the sever through Zigbee and Wi-Fi networks, and programs
     executed by network setup and initialization, Beacon health monitor and
     comminication unit. Each gateway is the root of a star network of LBeacons.

  Version:

     1.0, 201901041100

  File Name:

     Gateway.c

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

 */


#include "Gateway.h"


int main(int argc, char **argv){

    int return_value;

    pthread_t CommUnit_thread;

    NSI_initialization_complete      = false;
    CommUnit_initialization_complete = false;
    BHM_initialization_complete      = false;

    initialization_failed = false;

    ready_to_work = true;

    return_value = get_config(&config, CONFIG_FILE_NAME);

    if(return_value != WORK_SUCCESSFULLY){
        return E_OPEN_FILE;
    }

    /* Initialize the memory pool */
    if(mp_init(&node_mempool, sizeof(struct BufferNode), SLOTS_IN_MEM_POOL)
       != MEMORY_POOL_SUCCESS){
        /* Error handling */
        perror("E_MALLOC");
        return E_MALLOC;
    }

    init_entry(&Priority_buffer_list_head);
    init_Address_map(&LBeacon_Address_Map);

    /* Initialize the buffer_list_heads and add to the buffer array in the
    order of priority. Each buffer has the corresponding function pointer. */

    init_buffer(&Time_critical_LBeacon_receive_buffer_list_head,
                (void *) LBeacon_Process, config.HIGH_PRIORITY);
    insert_list_tail(&Time_critical_LBeacon_receive_buffer_list_head
                     .priority_entry, &Priority_buffer_list_head);

    init_buffer(&NSI_send_buffer_list_head,
                (void *) wifi_send, config.NORMAL_PRIORITY);
    insert_list_tail(&NSI_send_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&NSI_receive_buffer_list_head,
                (void *) NSI_Process, config.NORMAL_PRIORITY);
    insert_list_tail(&NSI_receive_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&LBeacon_receive_buffer_list_head,
                (void *) LBeacon_Process, config.HIGH_PRIORITY);
    insert_list_tail(&LBeacon_receive_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&Command_msg_buffer_list_head,
                (void *) Server_Process, config.NORMAL_PRIORITY);
    insert_list_tail(&Command_msg_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&BHM_receive_buffer_list_head,
                (void *) BHM_Process, config.LOW_PRIORITY);
    insert_list_tail(&BHM_receive_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&BHM_send_buffer_list_head,
                (void *) wifi_send, config.LOW_PRIORITY);
    insert_list_tail(&BHM_send_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    /* Network Setup and Initialization for Wi-Fi */
    return_value = NSI_routine();

    if(return_value != WORK_SUCCESSFULLY) return return_value;

    /* Create threads for the main thread of Communication Unit  */
    return_value = startThread(&CommUnit_thread, CommUnit_routine, NULL);

    if(return_value != WORK_SUCCESSFULLY) return return_value;

    while(NSI_initialization_complete == false ||
          CommUnit_initialization_complete == false ||
          BHM_initialization_complete == false){

        sleep(WAITING_TIME);

        if(initialization_failed == true){
            ready_to_work = false;
            return E_INITIALIZATION_FAIL;
        }
    }
    while(ready_to_work == true){
        sleep(WAITING_TIME);
    }

    /* The thread is going to be ended. Free the connection of Wifi */
    Wifi_free();
    return 0;
}


ErrorCode get_config(GatewayConfig *config, char *file_name) {

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        /* Error handling */
        //zlog_info(category_health_report, errordesc[E_OPEN_FILE].message);
        return E_OPEN_FILE;
    }
    else {

        /* Create spaces for storing the string in the current line being read*/
        char  config_setting[CONFIG_BUFFER_SIZE];
        char *config_message = NULL;
        int config_message_size = 0;

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
        config->Isolated_Mode = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->allowed_number_nodes = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->Period_between_RFHR = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->Period_between_RFOT = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->Number_worker_threads = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        if(config_message[strlen(config_message)-1] == '\n')
            config_message_size = strlen(config_message)-1;
        else
            config_message_size = strlen(config_message);
        memcpy(config->WiFi_SSID, config_message, config_message_size);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        if(config_message[strlen(config_message)-1] == '\n')
            config_message_size = strlen(config_message)-1;
        else
            config_message_size = strlen(config_message);
        memcpy(config->WiFi_PASS, config_message, config_message_size);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        if(config_message[strlen(config_message)-1] == '\n')
            config_message_size = strlen(config_message) - 1;
        else
            config_message_size = strlen(config_message);
        memcpy(config->SERVER_IP, config_message, config_message_size);

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
        config->CRITICAL_PRIORITY = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->HIGH_PRIORITY = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->NORMAL_PRIORITY = atoi(config_message);

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->LOW_PRIORITY = atoi(config_message);

        fclose(file);

    }
    return WORK_SUCCESSFULLY;
}


void init_buffer(BufferListHead *buffer, void (*function_p)(void*)
               , int priority_boast){

    init_entry( &(buffer->buffer_entry));

    init_entry( &(buffer->priority_entry));

    pthread_mutex_init( &buffer->list_lock, 0);

    buffer->function = function_p;

    buffer->arg = (void *) buffer;

    buffer->priority_boast = priority_boast;
}


void init_Address_map(Address_map_head *LBeacon_map){

    pthread_mutex_init( &LBeacon_map->list_lock, 0);

    memset(LBeacon_map->Address_map_list, 0
         , sizeof(LBeacon_map->Address_map_list));

    for(int n=0; n < MAX_NUMBER_NODES; n++){
        LBeacon_map->Address_map_list[n].in_use = reset_bit;
    }
}


bool is_in_Address_map(char *address){

    for(int n=0;n<MAX_NUMBER_NODES;n++){
        Address_map *curent_address = &LBeacon_Address_Map.Address_map_list[n];
        if (curent_address->in_use == set_bit){
            if(strcmp(curent_address->net_address, address) == 0){
                return true;
            }
        }
    }
    return false;
}


ErrorCode NSI_routine(){

    int return_value;

    pthread_t wifi_listener;

    /* Initialize the Wifi connection */
    if(return_value = Wifi_init(config.IPaddress) != WORK_SUCCESSFULLY){
        /* Error handling and return */
        initialization_failed = true;
        return E_WIFI_INIT_FAIL;
    }

    /* Create threads for sending and receiving data from and to LBeacon and
       server. */
    /* Two static threads for listening the data from LBeacon or Sever */
    return_value = startThread(&wifi_listener, (void *)wifi_receive, NULL);

    if(return_value != WORK_SUCCESSFULLY){
        initialization_failed = true;
        return E_WIFI_INIT_FAIL;
    }

    NSI_initialization_complete = true;

    return WORK_SUCCESSFULLY;
}


void *CommUnit_routine(){

    int init_time;

    Threadpool thpool;
    int return_error_value;

    //wait for NSI get ready
    while(NSI_initialization_complete == false){
        sleep(WAITING_TIME);
        if(initialization_failed == true){
            return (void *)NULL;
        }
    }

    /* Initialize the threadpool with assigned number of worker threads
       according to the data stored in the config file. */
    thpool = thpool_init(config.Number_worker_threads);

    /* Set the initial time. */
    init_time = get_system_time();

    /* Start counting down the time for polling the tracking data */
    poll_LBeacon_for_HR_time = get_system_time();

    polling_object_tracking_time = get_system_time();

    /* After all the buffer are initialized and the thread pool initialized,
    set the flag to true. */
    CommUnit_initialization_complete = true;

    /* When there is no dead thead, do the work. */
    while(ready_to_work == true){

        /* If it is the time to poll the tracking data from LBeacon, Make a
        thread to do this work */
        if((config.Isolated_Mode == true) && (get_system_time() -
           poll_LBeacon_for_HR_time > config.Period_between_RFHR)){

            // Polling Tracked Object Data
            // set the pkt type
            int send_type = ((from_gateway & 0x0f)<<4) +
                             (tracked_object_data & 0x0f);
            char temp[WIFI_MESSAGE_LENGTH];
            memset(temp, 0, WIFI_MESSAGE_LENGTH);

            temp[0] = (char)send_type;

            // broadcast to LBeacons
            beacon_broadcast(temp, 2);

            poll_LBeacon_for_HR_time = get_system_time();
        }

        if((config.Isolated_Mode == true) && (get_system_time() -
           polling_object_tracking_time > config.Period_between_RFOT)){

            // Pulling Health Report
            // set the pkt type
            int send_type = ((from_gateway & 0x0f)<<4) +
                             (health_report & 0x0f);
            char temp[WIFI_MESSAGE_LENGTH];
            memset(temp, 0, WIFI_MESSAGE_LENGTH);

            temp[0] = (char)send_type;

            // broadcast to LBeacons
            beacon_broadcast(temp, 2);

            /* Reset the poll_LBeacon_time */
            polling_object_tracking_time = get_system_time();
        }

        /* In the normal situation, the scanning starts from the high
           priority to lower priority. If the timer expired for
           MAX_STARVATION_TIME, reverse the scanning process */
        if(get_system_time() - init_time < MAX_STARVATION_TIME){

            /* Scan the priority_array to get the corresponding work for the
               worker thread */
            List_Entry *tmp;

            list_for_each(tmp, &Priority_buffer_list_head){

                BufferListHead *current_head = ListEntry(tmp, BufferListHead
                                                       , priority_entry);

                pthread_mutex_lock( &current_head -> list_lock);

                /* There is still idle worker thread is waiting for the work */
                if ((thpool -> num_threads_working < thpool -> num_threads_alive
                    ) && (is_entry_list_empty( &current_head->buffer_entry)
                    == false)){

                    /* If there is a node in the buffer and the buffer
                       is not be occupied, do the work according to the
                       function pointer */
                    return_error_value = thpool_add_work(thpool
                                       , current_head -> function
                                       , current_head
                                       , current_head -> priority_boast);
                    if(return_error_value != WORK_SUCCESSFULLY){
                        //return return_error_value;
                    }
                }

                pthread_mutex_unlock( &current_head -> list_lock);
            } // End list for each
        }
        else{
            /* Scan the priority_array to get the corresponding work for the
               worker thread */

            List_Entry *tmp;

            int count = 0;

            list_for_each_reverse(tmp, &Priority_buffer_list_head){

                BufferListHead *current_head = ListEntry(tmp, BufferListHead
                                                       , priority_entry);
                pthread_mutex_lock(&current_head->list_lock);

                /* There is still idle worker thread is waiting for the work */
                if ((thpool -> num_threads_working < thpool -> num_threads_alive
                    ) && (is_entry_list_empty( &current_head->buffer_entry)
                    == false)){

                    /* If there is a node in the buffer and the buffer
                       is not be occupied, do the work according to the
                       function pointer */
                    return_error_value = thpool_add_work(thpool
                                       , current_head -> function
                                       , current_head
                                       , current_head -> priority_boast);
                    if(return_error_value != WORK_SUCCESSFULLY){
                        //return return_error_value;
                    }
                }

                pthread_mutex_unlock(&current_head->list_lock);
            }

            /* Reset the inital time */
            init_time = get_system_time();
        }

        sleep(WAITING_TIME);

    } //End while

    /* Destroy the thread pool */
    thpool_destroy(thpool);

    return (void *)NULL;
}


void *NSI_Process(void *buffer_head){

    BufferListHead *buffer = (BufferListHead *)buffer_head;

    pthread_mutex_lock(&buffer -> list_lock);

    if(is_entry_list_empty( &buffer -> buffer_entry) == false){
        /* Create a temporary node and set as the head */
        struct List_Entry *list_pointers, *save_list_pointers;
        list_for_each_safe(list_pointers, save_list_pointers
                         , &buffer -> buffer_entry){
            remove_list_node(list_pointers);

            BufferNode *temp = ListEntry(list_pointers, BufferNode
                                       , buffer_entry);

            char current_uuid[UUID_LENGTH];

            memcpy(current_uuid, &temp->content[1], UUID_LENGTH);

            int send_type = (from_gateway & 0x0f)<<4;

            /* Put the address into LBeacon_Address_Map and set the return
               pkt type */
            if (beacon_join_request(current_uuid, temp -> net_address)
                == true)
                send_type += join_request_ack & 0x0f;
            else
                send_type += join_request_deny & 0x0f;

            // put the pkt type to content
            temp->content[0] = (char)send_type;

            pthread_mutex_lock(&NSI_send_buffer_list_head.list_lock);

            insert_list_tail(&temp->buffer_entry
                           , &NSI_send_buffer_list_head.buffer_entry);

            pthread_mutex_unlock(&NSI_send_buffer_list_head.list_lock);

            mp_free(&node_mempool, temp);
        }
        pthread_mutex_unlock(&buffer -> list_lock);
    }
    else{
        pthread_mutex_unlock(&buffer -> list_lock);
        sleep(WAITING_TIME);
    }

    return (void *)NULL;
}


void *BHM_Process(void *buffer_head){

    BufferListHead *buffer = (BufferListHead *)buffer_head;

    /* Create a temporary node and set as the head */
    struct List_Entry *list_pointers, *save_list_pointers;

    BufferNode *temp;

    pthread_mutex_lock(&buffer->list_lock);

    list_for_each_safe(list_pointers, save_list_pointers
                                    , &buffer->buffer_entry){

        /* Remove the node from the orignal buffer list. And directly send to
           the server. */
        remove_list_node(list_pointers);

        temp = ListEntry(list_pointers, BufferNode, buffer_entry);

        //TODO Make a buffer to merge all the HR pkt and wait for polling.

        insert_list_tail( &temp -> buffer_entry
                        , &BHM_send_buffer_list_head.buffer_entry);

    }

    pthread_mutex_unlock(&buffer->list_lock);

    return (void *)NULL;
}


void *LBeacon_Process(void *buffer_head){

    BufferListHead *buffer = (BufferListHead *)buffer_head;

    /* Create a temporary node and set as the head */
    struct List_Entry *list_pointers, *save_list_pointers;

    BufferNode *temp;

    pthread_mutex_lock(&buffer->list_lock);

    list_for_each_safe(list_pointers, save_list_pointers
                                    , &buffer->buffer_entry){

        /* Remove the node from the orignal buffer list. */
        remove_list_node(list_pointers);

        temp = ListEntry(list_pointers, BufferNode, buffer_entry);

        /* Add the content that to be sent to the server */
        udp_addpkt( &udp_config, config.SERVER_IP, temp->content
                  , temp->content_size);

        mp_free(&node_mempool, temp);
    }

    pthread_mutex_unlock(&buffer->list_lock);

    return (void *)NULL;
}


void *Server_Process(void *buffer_head){
    BufferListHead *buffer = (BufferListHead *)buffer_head;

    /* Create a temporary node and set as the head */
    struct List_Entry *list_pointers, *save_list_pointers;

    BufferNode *temp;

    pthread_mutex_lock(&buffer->list_lock);

    list_for_each_safe(list_pointers, save_list_pointers
                                    , &buffer->buffer_entry){

        /* Remove the node from the orignal buffer list. */
        remove_list_node(list_pointers);

        temp = ListEntry(list_pointers, BufferNode, buffer_entry);

        beacon_broadcast(temp -> content, temp -> content_size);

        mp_free(&node_mempool, temp);
    }
}


bool beacon_join_request(char *ID, char *address){

    pthread_mutex_lock(&LBeacon_Address_Map.list_lock);
    /* Copy all the necessary information received from the LBeacon to the
       address map. */

    /* Record the first unused address map location in order to store the new
       joined LBeacon. */
    int not_in_use = -1;

    for(int n = 0 ; n < MAX_NUMBER_NODES ; n ++){
        Address_map *current_addrmap = &LBeacon_Address_Map.Address_map_list[n];

        if(current_addrmap->in_use == set_bit && strcmp(address
         , current_addrmap->net_address) == 0){
            pthread_mutex_unlock(&LBeacon_Address_Map.list_lock);
            return true;
        }
        else if(current_addrmap->in_use == reset_bit && not_in_use == -1){
            not_in_use = n;
        }
    }

    /* If still has space for the LBeacon to register */
    if (not_in_use != -1){
        Address_map *tmp = &LBeacon_Address_Map
                           .Address_map_list[not_in_use];

        tmp->in_use = set_bit;

        strncpy(tmp -> beacon_uuid, ID, UUID_LENGTH);
        strncpy(tmp -> net_address, address, NETWORK_ADDR_LENGTH);
        pthread_mutex_unlock(&LBeacon_Address_Map.list_lock);
        return true;
    }
    else{
        pthread_mutex_unlock(&LBeacon_Address_Map.list_lock);
        return false;
    }

    return false;
}


void beacon_broadcast(char *msg, int size){

    pthread_mutex_lock(&LBeacon_Address_Map.list_lock);

    if (size <= WIFI_MESSAGE_LENGTH){
        for(int n = 0;n<MAX_NUMBER_NODES;n++){
            Address_map *current_addrmap =
                                       &LBeacon_Address_Map.Address_map_list[n];
            if (current_addrmap->in_use == set_bit){

                /* Add the pkt that to be sent to the server */
                udp_addpkt( &udp_config, current_addrmap -> net_address, msg
                          , size);
            }
        }
    }

    pthread_mutex_unlock(&LBeacon_Address_Map.list_lock);
}



int Wifi_init(char *IPaddress){

    /* Set the address of server */
    array_copy(IPaddress, udp_config.Local_Address, strlen(IPaddress));

    /* Initialize the Wifi cinfig file */
    if(udp_initial(&udp_config, config.send_port, config.recv_port)
                   != WORK_SUCCESSFULLY){

        /* Error handling TODO */
        return E_WIFI_INIT_FAIL;
    }
    return WORK_SUCCESSFULLY;
}


void Wifi_free(){

    /* Release the Wifi elements and close the connection. */
    udp_release(&udp_config);
    return (void)NULL;
}


void *wifi_send(void *buffer_head){

    BufferListHead *buffer = (BufferListHead *)buffer_head;

    struct List_Entry *list_pointers, *save_list_pointers;
    BufferNode *temp;

    pthread_mutex_lock(&buffer -> list_lock);

    list_for_each_safe(list_pointers,
                       save_list_pointers,
                       &buffer -> buffer_entry){

        /* Remove the node from the orignal buffer list and free the memory. */
        remove_list_node(list_pointers);

        temp = ListEntry(list_pointers, BufferNode, buffer_entry);

        /* Add the content that to be sent to the server */
        udp_addpkt( &udp_config, temp -> net_address, temp->content
                  , temp->content_size);

        mp_free(&node_mempool, temp);
    }

    pthread_mutex_unlock(&buffer -> list_lock);

    return (void *)NULL;
}


void *wifi_receive(){

    while (ready_to_work == true) {

        struct BufferNode *new_node;

        sPkt temppkt = udp_getrecv( &udp_config);

        if(temppkt.type == UDP){

            /* Allocate form zigbee packet memory pool a buffer for received
               data and copy the data from Xbee receive queue to the buffer. */
            new_node = mp_alloc( &node_mempool);

            if(new_node == NULL){
                /* Alloc mempry failed, error handling. */
                printf("E_MALLOC");
            }
            else{
                /* Initialize the entry of the buffer node */
                init_entry(&new_node -> buffer_entry);

                /* Copy the content to the buffer_node */
                memcpy(new_node->content, temppkt.content
                     , temppkt.content_size);

                new_node->content_size = temppkt.content_size;

                char *tmp_addr = udp_hex_to_address(temppkt.address);

                memcpy(new_node -> net_address, tmp_addr, NETWORK_ADDR_LENGTH);

                // read the pkt direction
                int type_H = (new_node -> content[0] >> 4) & 0x0f;
                // read the pkt type
                int type_L = new_node -> content[0] & 0x0f;

                /* Insert the node to the specified buffer, and release
                   list_lock. */
                switch (type_H) {
                    case from_server:

                        switch (type_L) {

                            case RFHR_from_server:
                                poll_LBeacon_for_HR_time = get_system_time();
                                pthread_mutex_lock(&Command_msg_buffer_list_head
                                                 .list_lock);
                                insert_list_tail(&new_node -> buffer_entry
                                               , &Command_msg_buffer_list_head
                                               .buffer_entry);
                                pthread_mutex_unlock(
                                       &Command_msg_buffer_list_head.list_lock);

                                break;

                            case poll_for_tracked_object_data_from_server:
                                polling_object_tracking_time =
                                                              get_system_time();
                                pthread_mutex_lock(&Command_msg_buffer_list_head
                                                 .list_lock);
                                insert_list_tail( &new_node -> buffer_entry
                                  , &Command_msg_buffer_list_head.buffer_entry);
                                pthread_mutex_unlock(
                                       &Command_msg_buffer_list_head.list_lock);
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

                        switch (type_L) {

                            case request_to_join:
                                pthread_mutex_lock(&NSI_receive_buffer_list_head
                                                 .list_lock);
                                insert_list_tail(&new_node -> buffer_entry
                                               , &NSI_receive_buffer_list_head
                                               .buffer_entry);
                                pthread_mutex_unlock(
                                       &NSI_receive_buffer_list_head.list_lock);
                                break;

                            case tracked_object_data:
                                pthread_mutex_lock(
                                   &LBeacon_receive_buffer_list_head.list_lock);
                                insert_list_tail( &new_node -> buffer_entry
                              , &LBeacon_receive_buffer_list_head.buffer_entry);
                                pthread_mutex_unlock(
                                   &LBeacon_receive_buffer_list_head.list_lock);
                                break;

                            case health_report:
                                pthread_mutex_lock(&BHM_receive_buffer_list_head
                                                   .list_lock);
                                insert_list_tail( &new_node -> buffer_entry
                                  , &BHM_receive_buffer_list_head.buffer_entry);
                                pthread_mutex_unlock(
                                       &BHM_receive_buffer_list_head.list_lock);
                                break;

                            default:
                                mp_free(&node_mempool, new_node);
                                break;
                        }
                        break;

                    default:
                        mp_free(&node_mempool, new_node);
                        break;
                }
            }
        }
        else{
            /* If there is no packet received, sleep a short time */
            sleep(WAITING_TIME);
        }
    }
    return (void *)NULL;
}
