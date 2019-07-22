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

     2.0, 20190617

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


unsigned int twoc(int in, int t) 
{
    return (in < 0) ? (in + (2 << (t - 1))) : in;
}


void init_buffer(BufferListHead *buffer_list_head, void (*function_p)(void *),
                 int priority_nice)
{
    init_entry( &(buffer_list_head -> list_head));

    init_entry( &(buffer_list_head -> priority_list_entry));

    pthread_mutex_init( &buffer_list_head -> list_lock, 0);

    buffer_list_head -> function = function_p;

    buffer_list_head -> arg = (void *) buffer_list_head;

    buffer_list_head -> priority_nice = priority_nice;
}


void init_Address_Map(AddressMapArray *address_map)
{
    int n;

    pthread_mutex_init( &address_map -> list_lock, 0);

    memset(address_map -> address_map_list, 0,
           sizeof(address_map -> address_map_list));

    for(n = 0; n < MAX_NUMBER_NODES; n ++)
        address_map -> in_use[n] = false;
}


int is_in_Address_Map(AddressMapArray *address_map, char *find, int flag)
{
    int n;
    if (flag==0)
    {
 
        for(n = 0;n < MAX_NUMBER_NODES;n ++)
        {
            if (address_map -> in_use[n] == true && 
                strncmp(address_map -> address_map_list[n].net_address, 
                find, NETWORK_ADDR_LENGTH)== 0)
            {
                    return n;
            }
        }
    }
    else if(flag ==1)
    {
        for(n = 0;n < MAX_NUMBER_NODES; n ++){
            if (address_map -> in_use[n] == true && 
                strncmp(address_map -> address_map_list[n].uuid, 
                find, LENGTH_OF_UUID) == 0)
            {
                zlog_debug(category_debug,
                            "uuid matached n=%d [%s] [%s] [%d]\n", 
                            n, 
                            address_map->address_map_list[n].uuid, 
                            find, 
                            LENGTH_OF_UUID);
                    return n;
                   
            }
        }
    }
    return -1;
}

void *CommUnit_routine()
{
    /* The last reset time */
    int init_time;

    int uptime;

    Threadpool thpool;

    int return_error_value;

    /* A flag to indicate whether any buffer nodes were processed in 
    this iteration of the while loop */
    bool did_work;

    /* The pointer point to the current priority buffer list entry */
    List_Entry *current_entry, *list_entry;

    BufferNode *current_node;

    /* The pointer point to the current buffer list head */
    BufferListHead *current_head;

    /* wait for NSI get ready */
    while(NSI_initialization_complete == false)
    {
        Sleep(BUSY_WAITING_TIME_IN_MS);
        if(initialization_failed == true)
        {
            return (void *)NULL;
        }
    }
#ifdef debugging
    zlog_info(category_debug,"[CommUnit] thread pool Initializing");
#endif
    /* Initialize the threadpool with specified number of worker threads
       according to the data stored in the serverconfig file. */
    thpool = thpool_init(config.number_worker_threads);

#ifdef debugging
    zlog_info(category_debug, "[CommUnit] thread pool Initialized");
#endif

    uptime = clock_gettime();

    /* Set the initial time. */
    init_time = uptime;

    /* All the buffers lists have been are initialized and the thread pool
       initialized. Set the flag to true. */
    CommUnit_initialization_complete = true;

    /* When there is no dead thead, do the work. */
    while(ready_to_work == true)
    {
        did_work = false;

        uptime = clock_gettime();

        /* In the normal situation, the scanning starts from the high priority
           to lower priority. When the timer expired for MAX_STARVATION_TIME,
           reverse the scanning process */
        while(uptime - init_time < MAX_STARVATION_TIME)
        {
            /* Scan the priority_list to get the buffer list with the highest
               priority among all lists that are not empty. */

            pthread_mutex_lock( &priority_list_head.list_lock);

            list_for_each(current_entry,
                          &priority_list_head.priority_list_entry)
            {
                current_head = ListEntry(current_entry, BufferListHead,
                                         priority_list_entry);

                pthread_mutex_lock( &current_head -> list_lock);

                if (is_entry_list_empty( &current_head->list_head) == true)
                {
                    pthread_mutex_unlock( &current_head -> list_lock);
                    /* Go to check the next buffer list in the priority list */

                    continue;
                }
                else 
                {
                    list_entry = current_head -> list_head.next;

                    remove_list_node(list_entry);

                    pthread_mutex_unlock( &current_head -> list_lock);

                    current_node = ListEntry(list_entry, BufferNode,
                                             buffer_entry);

                    /* Call the function specified by the function pointer to 
                       the work */
                    return_error_value = thpool_add_work(thpool,
                                                     current_head -> function,
                                                     current_node,
                                                     current_head ->
                                                     priority_nice);
                    did_work = true;
                    break;
                }
            }
            uptime = clock_gettime();
            pthread_mutex_unlock( &priority_list_head.list_lock);
        }

        /* Scan the priority list in reverse order to prevent starving the
           lowest priority buffer list. */

        pthread_mutex_lock( &priority_list_head.list_lock);

        
        // In starvation scenario, we still need to process time-critial buffer 
        // lists first. We first process Geo_fence_alert_buffer_list_head, 
        // then Geo_fence_receive_buffer_list_head, and finally reversely 
        // traverse the priority list.
        list_for_each(current_entry, &priority_list_head.priority_list_entry)
        {
            current_head = ListEntry(current_entry, BufferListHead,
                                     priority_list_entry);

            if(current_head -> priority_nice == 
                config.time_critical_priority){

                pthread_mutex_lock( &current_head -> list_lock);

                if (is_entry_list_empty( &current_head->list_head) == true)
                {
                    pthread_mutex_unlock( &current_head -> list_lock);

                    continue;
                }
                else 
                {
                    list_entry = current_head -> list_head.next;

                    remove_list_node(list_entry);

                    pthread_mutex_unlock( &current_head -> list_lock);

                    current_node = ListEntry(list_entry, BufferNode,
                                             buffer_entry);

                    return_error_value = thpool_add_work(thpool,
                                                     current_head -> function,
                                                     current_node,
                                                     current_head ->
                                                     priority_nice);
                    did_work = true;
                    break;
                }
            }
        }

        // reversly traverse the priority list
        list_for_each_reverse(current_entry,
                              &priority_list_head.priority_list_entry)
        {
            current_head = ListEntry(current_entry, BufferListHead,
                                     priority_list_entry);

            pthread_mutex_lock( &current_head -> list_lock);

            if (is_entry_list_empty( &current_head->list_head) == true)
            {
                pthread_mutex_unlock( &current_head -> list_lock);

                continue;
            }
            else 
            {
                list_entry = current_head -> list_head.next;

                remove_list_node(list_entry);

                pthread_mutex_unlock( &current_head -> list_lock);

                current_node = ListEntry(list_entry, BufferNode,
                                         buffer_entry);

                /* Call the function pointed to by the function pointer to do 
                   the work */
                return_error_value = thpool_add_work(thpool,
                                                     current_head -> function,
                                                     current_node,
                                                     current_head ->
                                                     priority_nice);
                did_work = true;
                break;
            }
        }

        /* Update the init_time */
        init_time = clock_gettime();

        pthread_mutex_unlock( &priority_list_head.list_lock);

        /* If during this iteration of while loop no work were done, 
           sleep before starting the next iteration */
        if(did_work == false)
        {
            Sleep(BUSY_WAITING_TIME_IN_MS);
        }

    } /* End while(ready_to_work == true) */

    
    /* Destroy the thread pool */
    thpool_destroy(thpool);

    return (void *)NULL;
}



int udp_sendpkt(pudp_config udp_config, BufferNode *buffer_node)
{
    int pkt_type;

    char content[WIFI_MESSAGE_LENGTH];

    pkt_type = ((buffer_node->pkt_direction << 4) & 0xf0) + 
               (buffer_node->pkt_type & 0x0f);

    memset(content, 0, WIFI_MESSAGE_LENGTH);

    sprintf(content, "%c%s", (char)pkt_type, buffer_node ->content);

    buffer_node -> content_size =  buffer_node -> content_size + 1;
  
    /* Add the content of the buffer node to the UDP to be sent to the 
       destination */
    udp_addpkt(udp_config, 
               buffer_node -> net_address, 
               buffer_node -> port,
               content,
               buffer_node -> content_size);

    return WORK_SUCCESSFULLY;
}


void trim_string_tail(char *message) 
{
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

int strncmp_caseinsensitive(char const *str_a, char const *str_b, size_t len)
{
    int index = 0;
    int diff = 0;

    for(index = 0; index < len; index++)
    {
        diff = tolower((unsigned char)str_a[index]) - 
               tolower((unsigned char)str_b[index]);
        if(0 != diff)
        {
            return -1;
        }
    }
    return 0;
}

ErrorCode startThread(pthread_t *thread, void *( *start_routine)(void *),
                      void *arg)
{
    pthread_attr_t attr;

    if ( pthread_attr_init( &attr) != 0 ||
         pthread_create(thread, &attr, start_routine, arg) != 0 ||
         pthread_detach( *thread))
    {
          zlog_info(category_debug, "Start Thread Error.");
          return E_START_THREAD;
    }

    return WORK_SUCCESSFULLY;

}


int get_system_time()
{
    /* Return value as a long long type */
    int system_time;

    time_t now = time(NULL);

    /* second ver. */
    system_time = (int)now;

    return system_time;
}

int clock_gettime()
{
#ifdef _WIN32
    return GetTickCount() / 1000;
#elif __unix__
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    return current_time.tv_sec;
#endif
}


char *strtok_save(char *str, char *delim, char **saveptr)
{
    char *tmp;

    if(str == NULL)
    {
        tmp = *saveptr;
    }
    else
    {
        tmp = str;
    }

    if(strncmp(tmp, delim, strlen(delim)) == 0)
    {
        *saveptr += strlen(delim) * sizeof(char);
        return NULL;
    }

    return strtok_s(str, delim, saveptr);
}


int display_time(void)
{
    // variables to store date and time components
    //int hours, minutes, seconds, day, month, year;

    // time_t is arithmetic time type
    time_t now;

    // Obtain current time
    // time() returns the current time of the system as a time_t value
    time(&now);

    // Convert to local time format and print to stdout
    zlog_debug(category_debug, "%s", ctime(&now));
    
    return 0;
}