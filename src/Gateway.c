/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and conditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIPS

  File Description:

     This file contains the program to transmit and receive data to and from
     LBeacon and the sever through Zigbee and Wi-Fi networks, and programs
     executed by network setup and initialization, Beacon health monitor and
     comminication unit. Each gateway is the root of a star network of LBeacons.

  Version:

     1.0, 20181130

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

    pthread_t NSI_thread;
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
    init_buffer(&LBeacon_receive_buffer_list_head,
                (void *) Process_message, HIGH_PRIORITY);
    insert_list_first(&LBeacon_receive_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&Server_send_buffer_list_head,
                (void *) wifi_send, HIGH_PRIORITY);
    insert_list_first(&Server_send_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&LBeacon_send_buffer_list_head,
                (void *) wifi_send, HIGH_PRIORITY);
    insert_list_first(&LBeacon_send_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&Command_msg_buffer_list_head,
                (void *) Process_message, NORMAL_PRIORITY);
    insert_list_first(&Command_msg_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&LBeacon_send_buffer_list_head,
                (void *) wifi_send, HIGH_PRIORITY);
    insert_list_first(&LBeacon_send_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&BHM_receive_buffer_list_head,
                (void *) Process_message, LOW_PRIORITY);
    insert_list_first(&BHM_receive_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    init_buffer(&BHM_send_buffer_list_head,
                (void *) wifi_send, LOW_PRIORITY);
    insert_list_first(&BHM_send_buffer_list_head.priority_entry
                    , &Priority_buffer_list_head);

    /* Network Setup and Initialization for Wi-Fi */
    return_value = startThread(&NSI_thread, Initialize_network, NULL);

    if(return_value != WORK_SUCCESSFULLY) return return_value;

    /* Create threads for the main thread of Communication Unit  */
    //return_value = startThread(&CommUnit_thread, CommUnit_routine, NULL);

    //if(return_value != WORK_SUCCESSFULLY) return return_value;


    while(NSI_initialization_complete == false ||
          CommUnit_initialization_complete == false ||
          BHM_initialization_complete == false){
        printf("In While\n");
        sleep(A_SHORT_TIME);

        if(initialization_failed == true){

            ready_to_work = false;
            return E_INITIALIZATION_FAIL;
        }
    }

    while(ready_to_work == true){
        // Do bookkeeping work
    }

    return_value = pthread_join(CommUnit_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) return return_value;

    return_value = pthread_join(NSI_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) return return_value;

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
        char *config_message[CONFIG_FILE_LENGTH];
        int config_message_size = 0;

        /* Keep reading each line and store into the config struct */
        fgets(config_setting, sizeof(config_setting), file);
        config_message[0] = strstr((char *)config_setting, DELIMITER);
        config_message[0] = config_message[0] + strlen(DELIMITER);
        if(config_message[0][strlen(config_message[0])-1] == '\n')
            config_message_size = strlen(config_message[0])-1;
        else
            config_message_size = strlen(config_message[0]);
        memcpy(config->IPaddress, config_message[0], config_message_size);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[1] = strstr((char *)config_setting, DELIMITER);
        config_message[1] = config_message[1] + strlen(DELIMITER);
        config->allowed_number_nodes = atoi(config_message[1]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[2] = strstr((char *)config_setting, DELIMITER);
        config_message[2] = config_message[2] + strlen(DELIMITER);
        config->Period_between_RFHR = atoi(config_message[2]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[3] = strstr((char *)config_setting, DELIMITER);
        config_message[3] = config_message[3] + strlen(DELIMITER);
        config->Number_worker_threads = atoi(config_message[3]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[4] = strstr((char *)config_setting, DELIMITER);
        config_message[4] = config_message[4] + strlen(DELIMITER);
        config->Number_priority_levels = atoi(config_message[4]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[5] = strstr((char *)config_setting, DELIMITER);
        config_message[5] = config_message[5] + strlen(DELIMITER);
        if(config_message[5][strlen(config_message[5])-1] == '\n')
            config_message_size = strlen(config_message[5])-1;
        else
            config_message_size = strlen(config_message[5]);
        memcpy(config->WiFi_SSID, config_message[5], config_message_size);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[6] = strstr((char *)config_setting, DELIMITER);
        config_message[6] = config_message[6] + strlen(DELIMITER);
        if(config_message[6][strlen(config_message[6])-1] == '\n')
            config_message_size = strlen(config_message[6])-1;
        else
            config_message_size = strlen(config_message[6]);
        memcpy(config->WiFi_PASS, config_message[6], config_message_size);

        fclose(file);

    }
    return WORK_SUCCESSFULLY;
}


void init_buffer(BufferListHead *buffer, void (*function_p)(void*),
								                            int priority_boast){

    init_entry( &(buffer->buffer_entry));

    init_entry( &(buffer->priority_entry));

    pthread_mutex_init( &buffer->list_lock, 0);

    buffer->num_in_list = 0;

    buffer->function = function_p;

    buffer->arg = (void *) buffer;

    buffer->priority_boast = priority_boast;
}

void init_Address_map(Address_map_head *LBeacon_map){

    pthread_mutex_init( &LBeacon_map->list_lock, 0);

    LBeacon_map->num_in_list = 0;

    memset(LBeacon_map->Address_map_list, 0
         , sizeof(LBeacon_map->Address_map_list));
}


void *Initialize_network(){

    int return_value;

    pthread_t Lbeacon_listener;
    pthread_t Sever_listener;

    /* set up WIFI connection */
    /* open temporary wpa_supplicant.conf file to setup wifi environment*/
    FILE *cfgfile = fopen("/etc/wpa_supplicant/wpa_supplicant.conf","w");
    fprintf(cfgfile, "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\
\nupdate_config=1\ncountry=TW\n\nnetwork={\n    ssid=\"%s\"\n    psk=\"%s\"\n}"
    , config.WiFi_SSID, config.WiFi_PASS);

    fclose(cfgfile);

    /* Initialize the Wifi connection */
    if(return_value = Wifi_init(config.IPaddress) != WORK_SUCCESSFULLY){

        /* Error handling and return */

        initialization_failed = true;
        printf("Initialize Wifi Fail.\n");

        pthread_exit(0);

    }

    /* Create threads for sending and receiving data from and to LBeacon and
       server. */
    /* Two static threads for listening the data from LBeacon or Sever */
    return_value = startThread(&Lbeacon_listener, (void *)wifi_receive,
                               &Priority_buffer_list_head);

    if(return_value != WORK_SUCCESSFULLY){

        initialization_failed = true;
        //return return_value;

    }

    //pthread_setschedprio(&Lbeacon_listener, HIGH_PRIORITY);

    return_value = startThread(&Sever_listener, (void *)wifi_receive,
                               (void *)&Command_msg_buffer_list_head);

    if(return_value != WORK_SUCCESSFULLY){

        initialization_failed = true;
        //return return_value;
    }

    //pthread_setschedprio(&Sever_listener, NORMAL_PRIORITY);

    NSI_initialization_complete = true;
    while( ready_to_work == true ){

        /* Nothing to do, go to sleep. */
        sleep(A_LONG_TIME);
    }

    /* Waitting for the system is down, pthread_join and return. */
    return_value = pthread_join(Lbeacon_listener, NULL);

    if (return_value != WORK_SUCCESSFULLY){

        initialization_failed = true;
        //return return_value;
    }

    return_value = pthread_join(Sever_listener, NULL);

    if (return_value != WORK_SUCCESSFULLY){

        initialization_failed = true;
        //return return_value;
    }
    /* The thread is going to be ended. Free the connection of Wifi */
    Wifi_free();
    //return;
}


void *CommUnit_routine(){

    Threadpool thpool;
    int return_error_value;
    bool is_reverse = false;

    /* Set the initial time. */
    init_time = get_system_time();

    //wait for NSI get ready
    while( NSI_initialization_complete == false || ready_to_work == false){
        sleep(A_LONG_TIME);
        if(initialization_failed == true){
            pthread_exit(NULL);
        }
    }

    /* Initialize the threadpool with assigned number of worker threads
       according to the data stored in the config file. */
    thpool = thpool_init(config.Number_worker_threads);

    /* Start counting down the time for polling the tracking data */
    poll_LBeacon_for_HR_time = get_system_time();

    /* After all the buffer are initialized and the static threads are created,
    set the flag to true. */
    CommUnit_initialization_complete = true;

    /* When there is no dead thead, do the work. */
    while(thpool->num_threads_alive > 0){

        /* There is still idle worker thread is waiting for the work */
        if(thpool->num_threads_working < thpool->num_threads_alive){

            /* If it is the time to poll the tracking data from LBeacon, Make a
            thread to do this work */
            if(get_system_time() - poll_LBeacon_for_HR_time > MAX_POLLING_TIME){

                /* Reset the poll_LBeacon_time */
                poll_LBeacon_for_HR_time = get_system_time();

            }

            /* In the normal situation, the scanning starts from the high
            priority to lower priority. If the timer expired for
            MAX_STARVATION_TIME, reverse the scanning process. */
            if(get_system_time() - init_time < MAX_STARVATION_TIME){
                /* Scan the priority_array to get the corresponding work fro the
                   worker thread */
                List_Entry *tmp;

                list_for_each(tmp, &Priority_buffer_list_head){

                    BufferListHead *current_head = ListEntry(tmp, BufferListHead
                                                           , priority_entry);
                    pthread_mutex_lock(&current_head->list_lock);

                    if (current_head->num_in_list > 0){
                        /* If there is a node in the buffer and the buffer is
                           not be occupied, do the work according to the
                           function pointer
                         */

                        return_error_value = thpool_add_work(thpool
                                           , current_head -> function
                                           , current_head
                                           , current_head -> priority_boast);

                        if(return_error_value != WORK_SUCCESSFULLY){
                            //return return_error_value;
                        }
                        pthread_mutex_unlock(&current_head->list_lock);
                    }
                } // End list for each
            }
            else{
                /* Scan the priority_array to get the corresponding work fro the
                   worker thread */
                List_Entry *tmp;

                list_for_each_reverse(tmp, &Priority_buffer_list_head){

                    BufferListHead *current_head = ListEntry(tmp, BufferListHead
                                                           , priority_entry);
                    pthread_mutex_lock(&current_head->list_lock);

                    if (current_head->num_in_list > 0){
                        /* If there is a node in the buffer and the buffer is
                           not be occupied, do the work according to the
                           function pointer
                         */

                        return_error_value = thpool_add_work(thpool
                                           , current_head -> function
                                           , current_head
                                           , current_head -> priority_boast);

                        if(return_error_value != WORK_SUCCESSFULLY){
                            //return return_error_value;
                        }
                        pthread_mutex_unlock(&current_head->list_lock);
                    }
                }
                /* Reset the inital time */
                init_time = get_system_time();
            }
        } // End if
    } //End while

    /* Destroy the thread pool */
    thpool_destroy(thpool);

    //return;
}


void *Process_message(void *buffer_head){

    BufferListHead *buffer = (BufferListHead *)buffer_head;

    /* Create a temporary node and set as the head */
    struct List_Entry *list_pointers, *save_list_pointers;

    BufferNode *temp;

    pthread_mutex_lock(&buffer->list_lock);

    list_for_each_safe(list_pointers, save_list_pointers
                                    , &buffer->buffer_entry){

        temp = ListEntry(list_pointers, BufferNode, buffer_entry);
        /* Remove the node from the orignal buffer list. */
        remove_list_node(list_pointers);

        /* According to the different buffer, the node is inserting to different
           buffer  */
        if ( buffer == &LBeacon_receive_buffer_list_head){
            pthread_mutex_lock(&Server_send_buffer_list_head.list_lock);
            insert_list_tail(&temp->buffer_entry
                           , &Server_send_buffer_list_head.buffer_entry);
            Server_send_buffer_list_head.num_in_list =
                                   Server_send_buffer_list_head.num_in_list + 1;
            pthread_mutex_unlock(&Server_send_buffer_list_head.list_lock);
        }
        else if ( buffer == &Command_msg_buffer_list_head){
            pthread_mutex_lock(&LBeacon_send_buffer_list_head.list_lock);
            insert_list_tail(&temp->buffer_entry
                           , &LBeacon_send_buffer_list_head.buffer_entry);
            LBeacon_send_buffer_list_head.num_in_list =
                                  LBeacon_send_buffer_list_head.num_in_list + 1;
            pthread_mutex_unlock(&LBeacon_send_buffer_list_head.list_lock);
        }
        else if(buffer == &BHM_receive_buffer_list_head){
            pthread_mutex_lock(&BHM_send_buffer_list_head.list_lock);
            insert_list_tail(&temp->buffer_entry
                           , &BHM_send_buffer_list_head.buffer_entry);
            BHM_send_buffer_list_head.num_in_list =
                                      BHM_send_buffer_list_head.num_in_list + 1;
            pthread_mutex_unlock(&BHM_send_buffer_list_head.list_lock);
        }
        else{

        }
        buffer->num_in_list = buffer->num_in_list - 1;
    }

    pthread_mutex_unlock(&buffer->list_lock);

    //return;
}


void beacon_join_request(char *ID, char *address,
                         Coordinates Beacon_Coordinates){
    pthread_mutex_lock(&LBeacon_Address_Map.list_lock);
    /* Copy all the necessary information received from the LBeacon to the
       address map. */
    LBeacon_Address_Map.num_in_list += 1;
    Address_map *tmp = &LBeacon_Address_Map
                       .Address_map_list[LBeacon_Address_Map.num_in_list];
    strcpy(tmp -> beacon_uuid, ID);
    strcpy(tmp -> net_address, address);
    strcpy(tmp -> beacon_coordinates.X_coordinates
         , Beacon_Coordinates.X_coordinates);
    strcpy(tmp -> beacon_coordinates.Y_coordinates
         , Beacon_Coordinates.Y_coordinates);
    strcpy(tmp -> beacon_coordinates.Z_coordinates
         , Beacon_Coordinates.Z_coordinates);

    pthread_mutex_unlock(&LBeacon_Address_Map.list_lock);

    //return;
}


int Wifi_init(char *IPaddress){

    /* Set the address of server */
    array_copy(IPaddress, udp_config.Local_Address, strlen(IPaddress));

    /* Initialize the Wifi cinfig file */
    if(udp_initial(&udp_config) != WORK_SUCCESSFULLY){

        /* Error handling TODO */
        return E_WIFI_INIT_FAIL;
    }
    return WORK_SUCCESSFULLY;
}


void Wifi_free(){

    /* Release the Wifi elements and close the connection. */
    udp_release(&udp_config);
    return;
}


void *wifi_send(void *buffer_head){

    BufferListHead *buffer = (BufferListHead *)buffer_head;

    struct List_Entry *list_pointers, *save_list_pointers;
    BufferNode *temp;

    pthread_mutex_lock(&buffer -> list_lock);

    list_for_each_safe(list_pointers,
                       save_list_pointers,
                       &buffer -> buffer_entry){

        temp = ListEntry(list_pointers, BufferNode, buffer_entry);

        /* Add the content that to be sent to the server */
        udp_addpkt( &udp_config.pkt_Queue, temp -> net_address, temp -> content
                  , temp -> content_size);

        /* Remove the node from the orignal buffer list and free the memory. */
        remove_list_node(list_pointers);

        mp_free(&node_mempool, temp);

        buffer -> num_in_list = buffer -> num_in_list - 1;
    }

    pthread_mutex_unlock(&buffer -> list_lock);

    //return;
}


void *wifi_receive(void *buffer_head){

    BufferListHead *buffer = (BufferListHead *)buffer_head;

    while (ready_to_work == true) {

        struct BufferNode *new_node;

        pPkt temppkt = get_pkt( &udp_config.Received_Queue);

        if(temppkt != NULL){

            /* Allocate form zigbee packet memory pool a buffer for received
               data and copy the data from Xbee receive queue to the buffer. */
            new_node = mp_alloc( &node_mempool);

            if(new_node == NULL){
                /* Alloc mempry failed, error handling. */
                perror("E_MALLOC");

            }
            else{

                /* Initialize the entry of the buffer node */
                init_entry(&new_node->buffer_entry);

                /* Copy the content to the buffer_node */
                memcpy(new_node->content, temppkt->content,
                       sizeof(temppkt->content));

                char *tmp_addr = udp_hex_to_address(temppkt->address);

                /* Get the zigbee network address from the content and look up
                   from Lbeacon_address_map the UUID of the LBeacon, and the
                   buffer_index. */
                memcpy(new_node -> net_address, tmp_addr, sizeof(tmp_addr));

                pthread_mutex_lock(&buffer -> list_lock);
                /* Insert the node to the input buffer, and release
                   list_lock. */

                insert_list_tail( &new_node -> buffer_entry
                                , &buffer -> buffer_entry);

                buffer -> num_in_list = buffer -> num_in_list + 1;

                pthread_mutex_unlock(&buffer -> list_lock);

                delpkt(&udp_config.Received_Queue);

            }
        }
        else{

            /* If there is no packet received, sleep a short time */
            sleep(A_SHORT_TIME);
        }
    }

    //return;
}
