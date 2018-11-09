/*
 Copyright (c) 2016 Academia Sinica, Institute of Information Science

 License:

     GPL 3.0 : The content of this file is subject to the terms and
     cnditions defined in file 'COPYING.txt', which is part of this source
     code package.

 Project Name:

     BeDIPS

 File Description:

     This file contains the program to transmit and receive data to and from
     LBeacon and the sever through Zigbee and Wi-Fi networks, and programs
     executed by network setup and initialization, Beacon health monitor and
     comminication unit. Gateway takes the role of the coordinator in the
     local zigbee network.

 File Name:

     Gateway.c

 Abstract:

     BeDIPS uses LBeacons to deliver 3D coordinates and textual
     descriptions of their locations to users' devices. Basically, a
     LBeacon is an inexpensive, Bluetooth Smart Ready device. The 3D
     coordinates and location description of every LBeacon are retrieved
     from BeDIS (Building/environment Data and Information System) and
     stored locally during deployment and maintenance times. Once
     initialized, each LBeacon broadcasts its coordinates and location
     description to Bluetooth enabled user devices within its coverage
     area.

 Authors:

     Holly Wang     , hollywang@iis.sinica.edu.tw
     Jake Lee     , jakelee@iis.sinica.edu.tw
     Johnson Su   , johnsonsu@iis.sinica.edu.tw
     Hank Kung    , hank910140@gmail.com
     Ray Chao     , raychao5566@gmail.com
     Gary Xiao    , garyh0205@hotmail.com

 */

#include "Gateway.h"

GatewayConfig get_config(char *file_name) {

    /* Return value is a struct containing all config information */
    GatewayConfig config;

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {

        /* Error handling */
        zlog_info(category_health_report, errordesc[E_OPEN_FILE].message);

        return NULL;

    }
    else {

        /* Create spaces for storing the string in the current line being read*/
        char  config_setting[CONFIG_BUFFER_SIZE];
        char *config_message[CONFIG_FILE_LENGTH];

         /* Keep reading each line and store into the config struct */
         fgets(config_setting, sizeof(config_setting), file);
         config_message[0] = strstr((char *)config_setting, DELIMITER);
         config_message[0] = config_message[0] + strlen(DELIMITER);
         memcpy(config.IPaddress, config_message[0],
           strlen(config_message[0]));
         config.address_length = strlen(config_message[0]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[1] = strstr((char *)config_setting, DELIMITER);
        config_message[1] = config_message[1] + strlen(DELIMITER);
        config.allowed_number_of_nodes = atoi(config_message[1]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[2] = strstr((char *)config_setting, DELIMITER);
        config_message[2] = config_message[2] + strlen(DELIMITER);
        config.period_between_RFHR = atoi(config_message[2]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[3] = strstr((char *)config_setting, DELIMITER);
        config_message[3] = config_message[3] + strlen(DELIMITER);
        config.number_worker_thread = atoi(config_message[3]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[4] = strstr((char *)config_setting, DELIMITER);
        config_message[4] = config_message[4] + strlen(DELIMITER);
        config.number_priority_levels = atoi(config_message[4]);

        fclose(file);
    }

    return config;
}


long long get_system_time() {
    /* A struct that stores the time */
    struct timeb t;

    /* Return value as a long long type */
    long long system_time;

    /* Convert time from Epoch to time in milliseconds of a long long type */
    ftime(&t);
    system_time = 1000 * t.time + t.millitm;

    return system_time;
}


void *Initialize_network(){

    printf("Enter Initialize_network\n");

    int return_value;

    //ErrorCode status;

    /* set up WIFI connection */
    /* open temporary wpa_supplicant.conf file to setup wifi environment*/

    FILE *cfgfile = fopen("/etc/wpa_supplicant/wpa_supplicant.conf ","w");
    fwrite(*cfgfile,"ctrl_interface=DIR=/var/run/wpa_supplicant\nGROUP=netdev\n\
    update_config=1\ncountry=CN\nnetwork={\nssid=\"Wifi_ssid\"\npsk=\"\
    passphrasehere\"\npriority=5\n}");

    //TODO check whether wpa_supplicant file content is correct.

    fclose(*cfgfile);

    /* check if WiFi is connected .... */
    //TODO need to find another way to check connection status.

    printf("Start Init Zigbee\n");

    if(return_value = zigbee_init() != WORK_SUCCESSFULLY){
        /* Error handling and return */

        printf("Initialize Zigbee Fail.\n");

        pthread_exit(0);

    }

    printf("Initialize Zigbee Success.\n");

    NSI_initialization_complete = true;

    while( ready_to_work == true ){
          /* Nothing to do, go to sleep. */
          sleep(A_LONG_TIME);
    }

    zigbee_free();

    return;

}

void *CommUnit_routine(){

    Threadpool thpool;
    pthread_t Lbeacon_listener;
    pthread_t Sever_listener;
    int return_error_value;

    /* Initialize the buffer_list_heads and add to the buffer array in the
    order of priority */
    init_buffer(LBeacon_receive_buffer_list_head, LBeacon_receive_buffer);
    priority_array[LBeacon_receive_buffer] = &LBeacon_receive_buffer_list_head;
    init_buffer(Server_send_buffer_list_head, Server_send_buffer);
    priority_array[Server_send_buffer] = &Server_send_buffer_list_head;
    init_buffer(LBeacon_send_buffer_list_head, LBeacon_send_buffer);
    priority_array[LBeacon_send_buffer] = &LBeacon_send_buffer_list_head;
    init_buffer(Command_msg_buffer_list_head, Command_msg_buffer);
    priority_array[Command_msg_buffer] = &Command_msg_buffer_list_head;
    init_buffer(BHM_receive_buffer_list_head, BHM_receive_buffer);
    priority_array[BHM_receive_buffer] = &BHM_receive_buffer_list_head;
    init_buffer(BHM_send_buffer_list_head, BHM_send_buffer);
    priority_array[BHM_send_buffer] = &BHM_send_buffer_list_head;


    //wait for NSI get ready
    while( NSI_initialization_complete == false || ready_to_work == false ){

        sleep(A_LONG_TIME);

    }

    /* Initialize the threadpool with assigned number of worker threads
    according to the data stored in the config file. */
    thpool = thpool_init(config.Number_worker_threads);

    /* Create threads for sending and receiving data from and to LBeacon and
    server. */
    /* Two static threads for listening the data from LBeacon or Sever */
    return_value = startThread(&Lbeacon_listener, zigbee_receive, buffer_array);

    if(return_value != WORK_SUCCESSFULLY){

       return return_value;

    }
    pthread_setschedprio(&Lbeacon_listener, HIGH_PRIORITY);

    return_value = startThread(&Sever_listener, wifi_receive,
                               &Command_msg_buffer_list_head);

    if(return_value != WORK_SUCCESSFULLY){

       return return_value;

    }
    pthread_setschedprio(&Sever_listener, NORMAL_PRIORITY);

    /* When there is no dead thead, do the work. */
    while(thpool.num_threads_alive != 0){

      /* There is still idle worker thread is waiting for the work */
      if(thpool.num_threads_working < thpool.num_threads_alive){

        int buff_id;
        /* Scan the priority_list_head to get the corresponding work fro the
        worker thread */
        for(buff_id = LBeacon_receive_buffer;
            buff_id <= BHM_send_buffer;
            buff_id++){

              /* If there is a node in the buffer and the buffer is not be
              occupied, do the work according to the function pointer */
              if(priority_array[buff_id]->num_in_list != 0 &&
                 priority_array[buff_id].buffer_is_busy == false){

                /* Add the work to the worker thread with its priority */
                return_error_value = thpool_add_work(thpool,
                                    (void *)priority_array->function,
                                    priority_array[buff_id],
                                    priority_array[buff_id]->priority_boast);

                if(return_error_value != WORK_SUCCESSFULLY){

                    return return_error_value;

                }

              }

        }

      }

    }
    /* Waitting for the system is down, pthread_join and return. */
    return_error_value = pthread_join(Lbeacon_listener, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        return return_value;

    }

    return_error_value = pthread_join(Sever_listener, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        return return_value;

    }

    return;

}

void *Process_message(BufferListHead *buffer){

  buffer->buffer_is_busy == true;

  /* Create a temporary node and set as the head */
  struct List_Entry *list_pointers, *save_list_pointers;
  BufferNode *temp;

  pthread_mutex_lock(buffer->list_lock);

  list_for_each_safe(list_pointers,
                   save_list_pointers,
                   &buffer->list_entry){

      temp = ListEntry(list_pointers, BufferNode, buffer_entry);
      /* Remove the node from the orignal buffer list. */
      remove_list_node(list_pointers);

      /* According to the different buffer, the node is inserting to different
      buffer  */
      switch (buffer->buff_id) {

        case LBeacon_receive_buffer:

          insert_list_first(temp->buffer_entry, &Server_send_buffer_list_head);
          Server_send_buffer_list_head.num_in_list =
            Server_send_buffer_list_head.num_in_list + 1;
          break;

        case Command_msg_buffer:

          insert_list_first(temp->buffer_entry, &LBeacon_send_buffer_list_head);
          LBeacon_send_buffer_list_head.num_in_list =
            LBeacon_send_buffer_list_head.num_in_list + 1;

          break;

        case BHM_receive_buffer:

         insert_list_first(temp->buffer_entry, &BHM_send_buffer_list_head);
         BHM_send_buffer_list_head.num_in_list =
          BHM_send_buffer_list_head.num_in_list + 1;

         break;

         default:
          break;

      }

      buffer->num_in_list = buffer->num_in_list - 1;

  }

  pthread_mutex_unlock(buffer->list_lock);

  buffer->buffer_is_busy = false;


}

ErrorCode startThread(pthread_t *threads ,void *( *thfunct)(void *), void *arg){

    pthread_attr_t attr;

    if ( pthread_attr_init( &attr) != 0
      || pthread_create(threads, &attr, thfunct, arg) != 0
      ){

          printf("Start Thread Error.\n");
          return E_START_THREAD;

    }

    printf("Start Thread Success.\n");
    return WORK_SUCCESSFULLY;

}

int main(int argc, char **argv){

    int return_value;
    GatewayConfig config;

    pthread_t NSI_thread;
    pthread_t CommUnit_thread;

    NSI_initialization_complete      = false;
    BHM_initialization_complete      = false;
    CommUnit_initialization_complete = false;

    ready_to_work = false;

    config = get_config(CONFIG_FILE_NAME);



    /* Initialize the memory pool */
    if(mp_init(&node_mempool, sizeof(struct BufferNode), SLOTS_IN_MEM_POOL)
        == NULL){
          /* Error handling */
          perror(E_MALLOC);
          return E_MALLOC;

}


    /* Network Setup and Initialization for Zigbee and Wifi */
    return_value = startThread(&NSI_thread, Initialize_network, NULL);

    if(return_value != WORK_SUCCESSFULLY){

       return return_value;

    }

    perror("NSI_SUCCESS");


    /* Create threads for the main thread of Communication Unit  */
    return_value = startThread(&CommUnit_thread, CommUnit_routine, NULL);

    if(return_value != WORK_SUCCESSFULLY){

        return return_value;

    }


    while(NSI_initialization_complete == false ||
          BHM_initialization_complete == false ||
          CommUnit_initialization_complete == false){

        sleep(A_SHORT_TIME);

        if(initialization_failed == true){

            ready_to_work = false;

            return;

        }

    }

    // initalization completed
    ready_to_work = true;

    while(ready_to_work == true){
        // Do bookkeeping work
    }

    return_value = pthread_join(CommUnit_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        return return_value;

    }


    return_value = pthread_join(NSI_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        return return_value;

    }

    return 0;

}
