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

#include "Gateway.h"


int main(int argc, char **argv){

    struct timespec current_time;
    int return_value;

    /* The flag is to know if any routines are processed in this while loop */
    bool did_work;
	
    /* The main thread of the communication Unit */
    pthread_t CommUnit_thread;

    /* The thread to listen for messages from Wi-Fi interface */
    pthread_t wifi_listener;
	
	char *temp_lbeacon_uuid = NULL;

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


    if(get_gateway_config( &config, CONFIG_FILE_NAME) != WORK_SUCCESSFULLY){
        zlog_error(category_health_report, "Opening config file Fail");
    #ifdef debugging
        zlog_error(category_debug, "Opening config file Fail");
    #endif
        return E_OPEN_FILE;
    }

    /* Initialize sll global flags */
    NSI_initialization_complete      = false;
    CommUnit_initialization_complete = false;
    initialization_failed = false;
    ready_to_work = true;

    /* Initialize the address map*/
    init_Address_Map( &LBeacon_address_map);

    /* Initialize buffer_list_heads and add to the head into the priority list.
     */

    init_buffer( &priority_list_head, (void *) sort_priority_list,
                config.high_priority);

    init_buffer( &command_msg_buffer_list_head,
                (void *) Server_routine, config.normal_priority);
    insert_list_tail( &command_msg_buffer_list_head.priority_list_entry,
                      &priority_list_head.priority_list_entry);

    init_buffer( &LBeacon_receive_buffer_list_head,
                (void *) LBeacon_routine, config.normal_priority);
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

    sort_priority_list(&config, &priority_list_head);

    /* Create the config from input config file */

    /* Initialize the memory pool */
    if(mp_init( &node_mempool, sizeof(BufferNode), SLOTS_IN_MEM_POOL)
       != MEMORY_POOL_SUCCESS){
        zlog_error(category_health_report, "Mempool Initialization Fail");
#ifdef debugging
        zlog_error(category_debug, "Mempool Initialization Fail");
#endif
        return E_MALLOC;
    }

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


	server_latest_polling_time = 0;
    last_join_request_time = 0;

    /* The while loop that keeps the program running */
    while(ready_to_work == true){

        did_work = false;

        clock_gettime(CLOCK_MONOTONIC, &current_time);

        if(current_time.tv_sec - server_latest_polling_time > 
		   INTERVAL_RECEIVE_MESSAGE_FROM_SERVER_IN_SEC && 
		   current_time.tv_sec - last_join_request_time >
           INTERVAL_FOR_RECONNECT_SERVER_IN_SEC){
			   
			if(WORK_SUCCESSFULLY == send_join_request(true, temp_lbeacon_uuid))
            {
                last_join_request_time = current_time.tv_sec;
            }
            did_work = true;
        }

        if(did_work == false){
            usleep(BUSY_WAITING_TIME);
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


ErrorCode get_gateway_config(GatewayConfig *config, char *file_name) {

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        /* Error handling */
        zlog_error(category_health_report, "Open config file fail.");
        return E_OPEN_FILE;
    }
    else {

        /* Create spaces for storing the string in the current line being read
         */
        char  config_setting[CONFIG_BUFFER_SIZE];
        char *config_message = NULL;
        int config_message_size = 0;

        fgets(config_setting, sizeof(config_setting), file);
        config_message = strstr((char *)config_setting, DELIMITER);
        config_message = config_message + strlen(DELIMITER);
        trim_string_tail(config_message);
        config->is_geofence = atoi(config_message);
		
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
        config->period_between_join_requests = atoi(config_message);

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


void *sort_priority_list(GatewayConfig *config, BufferListHead *list_head){

    List_Entry *list_pointer,
               *next_list_pointer;

    List_Entry critical_priority_head, high_priority_head,
               normal_priority_head, low_priority_head;

    BufferListHead *current_head, *next_head;

    init_entry( &critical_priority_head);
    init_entry( &high_priority_head);
    init_entry( &normal_priority_head);
    init_entry( &low_priority_head);

    pthread_mutex_lock( &list_head -> list_lock);

    list_for_each_safe(list_pointer, next_list_pointer,
                       &list_head -> priority_list_entry){

        remove_list_node(list_pointer);

        current_head = ListEntry(list_pointer, BufferListHead,
                                 priority_list_entry);

        if(current_head -> priority_nice == config -> critical_priority)

            insert_list_tail( list_pointer, &critical_priority_head);

        else if(current_head -> priority_nice == config -> high_priority)

            insert_list_tail( list_pointer, &high_priority_head);

        else if(current_head -> priority_nice == config -> normal_priority)

            insert_list_tail( list_pointer, &normal_priority_head);

        else if(current_head -> priority_nice == config -> low_priority)

            insert_list_tail( list_pointer, &low_priority_head);

    }

    if(is_entry_list_empty(&critical_priority_head) == false){
        list_pointer = critical_priority_head.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    if(is_entry_list_empty(&high_priority_head) == false){
        list_pointer = high_priority_head.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    if(is_entry_list_empty(&normal_priority_head) == false){
        list_pointer = normal_priority_head.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    if(is_entry_list_empty(&low_priority_head) == false){
        list_pointer = low_priority_head.next;
        remove_list_node(list_pointer -> prev);
        concat_list( &list_head -> priority_list_entry, list_pointer);
    }

    pthread_mutex_unlock( &list_head -> list_lock);

}


void *CommUnit_routine(){

    int init_time;
	struct timespec current_time;
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

    clock_gettime(CLOCK_MONOTONIC, &current_time);

    /* Set the initial time. */
    init_time = current_time.tv_sec;

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

        clock_gettime(CLOCK_MONOTONIC, &current_time);

        /* In the normal situation, the scanning starts from the high priority
           to lower priority. When the timer expired for MAX_STARVATION_TIME,
           reverse the scanning process */
        while(current_time.tv_sec - init_time < MAX_STARVATION_TIME){
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

            clock_gettime(CLOCK_MONOTONIC, &current_time);

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


void *NSI_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

    char *current_uuid = NULL, *datetime_str = NULL, *saveptr = NULL;
	
	char uuid[LENGTH_OF_UUID];

    int LBeacon_datetime;

    int send_type = (from_gateway & 0x0f)<<4;

    memset(uuid, 0, sizeof(uuid));
    
	current_uuid = strtok_save(&temp->content[1], ";", &saveptr);
	
	memcpy(uuid, current_uuid, sizeof(uuid));

    datetime_str = strtok_save(NULL, ";", &saveptr);

    sscanf(datetime_str, "%d", &LBeacon_datetime);

    /* Put the address into LBeacon_address_map and set the return pkt type
     */
    if (beacon_join_request(&LBeacon_address_map, current_uuid, temp ->
                            net_address, LBeacon_datetime) == true)
        send_type += join_request_ack & 0x0f;
    else
        send_type += join_request_deny & 0x0f;

    /* put the pkt type into content */
    temp->content[0] = (char)send_type;

    zlog_debug(category_debug, "current_uuid=[%s], " \
	                           "LBeacon_datetime=[%d], " \
							   "net_address=[%s]",
							   current_uuid,
							   LBeacon_datetime,
							   temp->net_address);
							  
    sprintf(&temp-> content[1], "%s;%d;%s;", current_uuid, LBeacon_datetime,
                                             temp -> net_address);

    temp->content_size = strlen(temp-> content);

    pthread_mutex_lock(&NSI_send_buffer_list_head.list_lock);

    insert_list_tail( &temp->buffer_entry,
                      &NSI_send_buffer_list_head.list_head);

    pthread_mutex_unlock( &NSI_send_buffer_list_head.list_lock);
				  
	// We need to report the registration status of this Beacon to server 
	send_join_request(false, uuid);

    return (void *)NULL;
}


void *BHM_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;

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

    /* If this Gateway is geofence, change the pkt_direction part to be from_gateway */
	if(config.is_geofence)
	{
		zlog_debug(category_debug, "change pkt_direction in LBeacon_routine");
		int send_msg_type = ((from_gateway & 0x0f) << 4) +
                                     (tracked_object_data & 0x0f);
		temp -> content[0] = (char)send_msg_type;
	}
	
    /* Add the content of the buffer node to the UDP to be sent to the
       Server */
    udp_addpkt( &udp_config, config.server_ip, temp -> content,
                temp -> content_size);

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}


void *Server_routine(void *_buffer_node){

    BufferNode *temp = (BufferNode *)_buffer_node;
    int pkt_type;

    pkt_type = temp -> content[0] & 0x0f;

    switch(pkt_type){
        case tracked_object_data:
            zlog_info(category_debug, "Send tracked object data to LBeacon");
            break;
        case health_report:
		    handle_health_report();
            zlog_info(category_debug, "Send health report to LBeacon");
            break;
    }

    zlog_info(category_debug, "Start Broadcast to LBeacon");

    beacon_broadcast(&LBeacon_address_map, temp -> content, temp ->
                     content_size);

    zlog_info(category_debug, "Polling Data from Server");

    mp_free( &node_mempool, temp);

    return (void *)NULL;
}

ErrorCode send_join_request(bool report_all_lbeacons, 
                            char *single_lbeacon_uuid){
	
    char message_buf[MAXINUM_WIFI_MESSAGE_LENGTH];
	char summary_buf[MAXINUM_WIFI_MESSAGE_LENGTH];
	char lbeacons_buf[MAXINUM_WIFI_MESSAGE_LENGTH];
	char one_lbeacon_buf[MAXINUM_WIFI_MESSAGE_LENGTH];
	
	int send_type = 0;
	int count = 0;
	int index = -1;
	int n;
	
	zlog_debug(category_debug, ">>send_join_request");
	
	memset(message_buf, 0, sizeof(message_buf));
	memset(summary_buf, 0, sizeof(summary_buf));
	memset(lbeacons_buf, 0, sizeof(lbeacons_buf));
	memset(one_lbeacon_buf, 0, sizeof(one_lbeacon_buf));
	
	send_type = ((from_gateway & 0x0f) << 4) + (request_to_join & 0x0f);
    message_buf[0] = (char)send_type;
    message_buf[1] = ';';

    if(report_all_lbeacons == true){
		
	    zlog_debug(category_debug, "report_all_lbeacons=[%d]", 
		           report_all_lbeacons);
		pthread_mutex_lock(&LBeacon_address_map.list_lock);
		
		for(n = 0; n < MAX_NUMBER_NODES; n ++){
		    if (LBeacon_address_map.in_use[n] == true){
				
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
	
    udp_addpkt( &udp_config, config.server_ip, message_buf, strlen(message_buf));
	
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
	
    int send_type = ((from_gateway & 0x0f) << 4) + (health_report & 0x0f);

    /* put the pkt type into content */
    new_node->content[0] = (char)send_type;

    sprintf(&new_node-> content[1], "%s;%d;", config.IPaddress, S_NORMAL);
     
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
       the new joined LBeacon. */
    int not_in_use = -1;
    int answer;

    if(answer = is_in_Address_Map(address_map, uuid, 1) >=0){
        strncpy(address_map -> address_map_list[answer].net_address,
                address, NETWORK_ADDR_LENGTH);
        address_map -> address_map_list[answer].last_request_time =
                                                              get_system_time();
        address_map -> address_map_list[answer].last_lbeacon_datetime= datetime;
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

        AddressMap *tmp =  &address_map -> address_map_list[not_in_use];

        address_map -> in_use[not_in_use] = true;

        strncpy(tmp -> uuid, uuid, strlen(uuid));
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

    if (size <= MAXINUM_WIFI_MESSAGE_LENGTH){
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
    char tmp_addr[NETWORK_ADDR_LENGTH];
	struct timespec current_time;

    while (ready_to_work == true) {

        BufferNode *new_node;

        sPkt temppkt = udp_getrecv( &udp_config);

        if(temppkt.type == UDP){

			clock_gettime(CLOCK_MONOTONIC, &current_time);

            /* Allocate memory from node_mempool a buffer node for received data
               and copy the data from Wi-Fi receive queue to the node. */
            
            new_node = mp_alloc( &node_mempool);
               
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
                memset(tmp_addr, 0, sizeof(tmp_addr));
                udp_hex_to_address(temppkt.address, tmp_addr);

                memcpy(new_node -> net_address, tmp_addr, NETWORK_ADDR_LENGTH);

                /* read the pkt direction from higher 4 bits. */
                int pkt_direction = (new_node -> content[0] >> 4) & 0x0f;
                /* read the pkt type from lower lower 4 bits. */
                int pkt_type = new_node -> content[0] & 0x0f;

                /* Insert the node to the specified buffer, and release
                   list_lock. */
                switch (pkt_direction) {
                    case from_server:
					
                        server_latest_polling_time = current_time.tv_sec;
						
                        switch (pkt_type) {

                            case health_report:
                                zlog_info(category_debug,
                                         "Get Health Report from the Server");
                                pthread_mutex_lock(&command_msg_buffer_list_head
                                                   .list_lock);
                                insert_list_tail(&new_node -> buffer_entry,
                                                 &command_msg_buffer_list_head
                                                 .list_head);
                                pthread_mutex_unlock(
                                       &command_msg_buffer_list_head.list_lock);

                                break;

                            case tracked_object_data:
                                zlog_info(category_debug,
                                   "Get Tracked Object Data from the Server");
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
    } /* end of while (ready_to_work == true) */
    return (void *)NULL;
}
