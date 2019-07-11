/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and cnditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

  File Name:

     BeDIS.c

  File Description:

     This file contains code of functions used in both Gateway and LBeacon.

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

#include "BeDIS.h"


unsigned int *uuid_str_to_data(char *uuid) {
    char conversion[] = "0123456789ABCDEF";
    int uuid_length = strlen(uuid);
    unsigned int *data =
        (unsigned int *)malloc(sizeof(unsigned int) * uuid_length);

    if (data == NULL) {
        /* Error handling */
        perror("Failed to allocate memory");
        return NULL;
    }

    unsigned int *data_pointer = data;
    char *uuid_counter = uuid;

    for (uuid_counter = uuid; uuid_counter < uuid + uuid_length;data_pointer++,
         uuid_counter += 2) {
        *data_pointer =
            ((strchr(conversion, toupper(*uuid_counter)) - conversion) * 16) +
            (strchr(conversion, toupper(*(uuid_counter + 1))) - conversion);
    }

    return data;
}


unsigned int twoc(int in, int t) {

    return (in < 0) ? (in + (2 << (t - 1))) : in;
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


void *CommUnit_routine(){

    int init_time;
    int current_time;
    Threadpool thpool;
    int return_error_value;
    /* The flag is to know if buffer nodes are processed in this while loop */
    bool did_work;

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

        List_Entry *tmp, *list_entry;
        BufferNode *current_node;
        BufferListHead *current_head;

        did_work = false;

        /* Update the init_time */
        init_time = get_system_time();

        current_time = get_system_time();

        /* In the normal situation, the scanning starts from the high priority
           to lower priority. When the timer expired for MAX_STARVATION_TIME,
           reverse the scanning process */
        while(current_time - init_time < MAX_STARVATION_TIME){
            did_work = false;

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

                    did_work = true;

                    break;
                }
            }

            pthread_mutex_unlock( &priority_list_head.list_lock);

            current_time = get_system_time();

            if(did_work == false){
                break;
            }

        }

        /* Scan the priority list in reverse order to prevent starving the
           lowest priority buffer list. */

        pthread_mutex_lock( &priority_list_head.list_lock);

        list_for_each_reverse(tmp, &priority_list_head.priority_list_entry){

            current_head = ListEntry(tmp, BufferListHead, priority_list_entry);

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

                did_work = true;

                break;

            }
        }

        pthread_mutex_unlock( &priority_list_head.list_lock);

        if(did_work == false){
            usleep(BUSY_WAITING_TIME);
        }

    } /* End while(ready_to_work == true) */

    /* Destroy the thread pool */
    thpool_destroy(thpool);

    return (void *)NULL;
}


void trim_string_tail(char *message) {

    int idx = 0;

    /* discard the whitespace, newline, carry-return characters at the end */
    if(strlen(message) > 0){

        idx = strlen(message) - 1;
        while(10 == message[idx] ||
                13 == message[idx] ||
                32 == message[idx]){

           message[idx] = '\0';
           idx--;
        }
    }
}


void ctrlc_handler(int stop) { ready_to_work = false; }


ErrorCode startThread(pthread_t *thread, void *( *start_routine)(void *),
                      void *arg){

    pthread_attr_t attr;

    if ( pthread_attr_init( &attr) != 0 ||
         pthread_create(thread, &attr, start_routine, arg) != 0 ||
         pthread_detach( *thread)){

          printf("Start Thread Error.\n");
          return E_START_THREAD;
    }

    return WORK_SUCCESSFULLY;

}


char *strtok_save(char *str, char *delim, char **saveptr){
    
    char *tmp;

    if(str == NULL){
        tmp = *saveptr;
    }
    else{
        tmp = str;
    }

    if(strncmp(tmp, delim, strlen(delim)) == 0){
        
        *saveptr += strlen(delim) * sizeof(char);
        return NULL;

    }

    return strtok_r(str, delim, saveptr);

}


int get_system_time() {
    /* Return value as a long long type */
    int system_time;

    time_t now = time(NULL);

    /* second ver. */
    system_time = (int)now;

    return system_time;
}
