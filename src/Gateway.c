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

      Han Wang     , hollywang@iis.sinica.edu.tw
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

        config.allowed_number_of_nodes = atoi(config_message[0]);

        fgets(config_setting, sizeof(config_setting), file);
        config_message[1] = strstr((char *)config_setting, DELIMITER);
        config_message[1] = config_message[1] + strlen(DELIMITER);


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

    }

    zigbee_free();

}

void *CommUnit_routine(){

    int return_error_value;

    /* This thread is created to send the data to the Server via wifi. */
    pthread_t wifi_sender_thread;
    /* This thread is created to receive the data from the sever via wifi. */
    pthread_t wifi_receiver_thread;
    /* This thread is created to send the data to LBeacon via zigbee. */
    pthread_t zigbee_sender_thread;
    /* This thread is created to receive the data from LBeacon via zigbee. */
    pthread_t zigbee_receiver_thread;

    //wait for NSI get ready
    while( NSI_initialization_complete == false ){

        sleep(A_LONG_TIME);

    }

    /*
        Create threads for sending and receiving data from and to LBeacon and
        server.
     */

    return_error_value = startThread(wifi_receive_thread, wifi_receive, NULL);

    if(return_error_value != WORK_SUCCESSFULLY){

        return return_error_value;

    }

    return_error_value = startThread(zigbee_receive_thread, zigbee_receive, NULL);

    if(return_error_value != WORK_SUCCESSFULLY){

        return return_error_value;

    }

    return_error_value = startThread(wifi_send_thread, wifi_send, NULL);

    if(return_error_value != WORK_SUCCESSFULLY){

        return return_error_value;

    }

    return_error_value = startThread(zigbee_send_thread, zigbee_send, NULL);

    if(return_error_value != WORK_SUCCESSFULLY){

        return return_error_value;

    }

    while(ready_to_work == True){

    }

    return_error_value = pthread_join(zigbee_send_thread, NULL);

    if (return_error_value != WORK_SUCCESSFULLY) {

        return return_error_value;

    }

    return_error_value = pthread_join(wifi_send_thread, NULL);

    if (return_error_value != WORK_SUCCESSFULLY) {

        return return_error_value;

    }

    return_error_value = pthread_join(zigbee_receive_thread, NULL);

    if (return_error_value != WORK_SUCCESSFULLY) {

        return return_error_value;

    }

    return_error_value = pthread_join(wifi_receive_thread, NULL);

    if (return_error_value != WORK_SUCCESSFULLY) {

        return return_error_value;

    }

}

void *BHM_routine(){

    while( ready_to_work == true){

        File *track_file;
        struct List_Entry *list_pointers,

        /* data that will be sent to the server, received by LBeacon */

        /*
            Create a new file for integrating the tracked data from each
            LBeacon. Each line represents each beacon's data
         */
        track_file = fopen("track.txt", "a+");

        if(track_file == NULL){

            track_file = fopen("track.txt", "wt");

        }

        /* Go through the track_buffer to get all the received data that
           is ready to sent to the sever */
        list_for_each(list_pointers, &buffer_health_list.buffer_entry){


        }
        /* Write the data to the file which is to be sent to the sever */
        fwrite(...);
        /* Call the function in Communit.c to execute the steps for
           transmitting.*/
        send_via_wifi("track.txt");

    }

}

ErrorCode startThread(pthread_t *threads ,void *( *thfunct)(void *), void *arg){

    pthread_attr_t attr;

    if ( pthread_attr_init( &attr) != 0
      || pthread_create(threads, &attr, thfunct, arg) != 0
      || pthread_attr_destroy( &attr) != 0){

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
    pthread_t BHM_thread;
    pthread_t CommUnit_thread;

    NSI_initialization_complete      = false;
    BHM_initialization_complete      = false;
    CommUnit_initialization_complete = false;

    ready_to_work = false;

    config = get_config(Gateway_CONFIG_PATH);

    /* Initialize the buffer_list_heads */
    init_buffer(LBeacon_receive_buffer_list_head);
    init_buffer(LBeacon_send_buffer_list_head);

    init_buffer(NSI_receive_buffer_list_head);
    init_buffer(NSI_send_buffer_list_head);

    init_buffer(BHM_receive_buffer_list_head);
    init_buffer(BHM_send_buffer_list_head);

    init_buffer(Command_msg_buffer_list_head);

    /* Network Setup and Initialization for Zigbee and Wifi */
    return_value = startThread(&NSI_thread, Initialize_network, NULL);

    if(return_value != WORK_SUCCESSFULLY){

       return return_value;

    }

    perror("NSI_SUCCESS");

    /* Create threads for Beacon Health Monitor  */
    return_value = startThread(&BHM_thread, BHM_routine, NULL);

    if(return_value != WORK_SUCCESSFULLY){

        return return_value;

    }

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

    return_value = pthread_join(BHM_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        return return_value;

    }

    return_value = pthread_join(NSI_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        return return_value;

    }

    return 0;

}
