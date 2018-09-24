/*
 * Copyright (c) 2016 Academia Sinica, Institute of Information Science
 *
 * License:
 *
 *     GPL 3.0 : The content of this file is subject to the terms and
 *     cnditions defined in file 'COPYING.txt', which is part of this source
 *     code package.
 *
 * Project Name:
 *
 *     BeDIPS
 *
 * File Description:
 *
 *     This file contains the program to transmit the data or information from
 *     LBeacons through Zigbee or UDP. Main tasks includes network setup and
 *     initialization, Beacon health monitor and comminication unit. Gateway
 *     takes the role as coordinator.
 *
 * File Name:
 *
 *     Gateway.c
 *
 * Abstract:
 *
 *     BeDIPS uses LBeacons to deliver 3D coordinates and textual
 *     descriptions of their locations to users' devices. Basically, a
 *     LBeacon is an inexpensive, Bluetooth Smart Ready device. The 3D
 *     coordinates and location description of every LBeacon are retrieved
 *     from BeDIS (Building/environment Data and Information System) and
 *     stored locally during deployment and maintenance times. Once
 *     initialized, each LBeacon broadcasts its coordinates and location
 *     description to Bluetooth enabled user devices within its coverage
 *     area.
 *
 * Authors:
 *
 *     Han Wang     , hollywang@iis.sinica.edu.tw
 *     Jake Lee     , jakelee@iis.sinica.edu.tw
 *     Johnson Su   , johnsonsu@iis.sinica.edu.tw
 *     Hank Kung    , hank910140@gmail.com
 *     Ray Chao     , raychao5566@gmail.com
 *     Gary Xiao    , garyh0205@hotmail.com
 */

#include "Gateway.h"


GatewayConfig get_config(char *file_name) {

    /* Return value is a struct containing all config information */
    GatewayConfig config;

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {

        /* Error handling */
        perror(errordesc[E_OPEN_FILE].message);
        zlog_info(category_health_report, 
                  errordesc[E_OPEN_FILE].message);
        cleanup_exit();
        return;

    }
    else {
    /* Create spaces for storing the string of the current line being read */
    char config_setting[CONFIG_BUFFER_SIZE];
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

    /* Initialize buffers for sever*/

    //init_buffer(buffer_health_list);

    //init_buffer(buffer_track_list);

    //init_buffer(recieveFromServer);

    /* set up WIFI connection */
    /* open temporary wpa_supplicant.conf file to setup wifi environment*/
    /*
    FILE *cfgfile = fopen("/etc/wpa_supplicant/wpa_supplicant.conf ","w");
    fwrite(*cfgfile,"ctrl_interface=DIR=/var/run/wpa_supplicant\nGROUP=netdev\n\
    update_config=1\ncountry=CN\nnetwork={\nssid=\"Wifi_ssid\"\npsk=\"\
    passphrasehere\"\npriority=5\n}");
    */
    //TODO check whether wpa_supplicant file content is correct.

    //fclose(*cfgfile);

    /* check if WiFi is connected .... */
    //TODO need to find another way to check connection status.
    //
    //status = system("ping google.com");
    //if (status = E_WIFI_CONNECT_FAIL){
    //    printf("Wifi is not connected....\n")
    //    int count =5;
    //    while(count != 0){
    //      printf("retry count %d", 6 - count);
    //      status = system("ping google.com");
    //    }
    //    system("sudo reboot");
    //        return;
    //
    //    }

    //return E_WIFI_CONNECT_FAIL;

    printf("Start Init Zigbee\n");

    if(return_value = zigbee_init() != WORK_SUCCESSFULLY){
        /* Error handling and return */

        //perror(errordesc[E_ZIGBEE_INIT_FAIL].message);

        printf("Initialize Zigbee Fail.\n");

        pthread_exit(0);

    }

    printf("Initialize Zigbee Success.\n");

    ready_to_work = true;

    NSI_initialization_complete = true;

    while( ready_to_work == true ){
        //printf("Trans to false\n");
        //ready_to_work = false;
    }

    zigbee_free();

}

ErrorCode startThread(pthread_t* threads ,void * (*thfunct)(void*), void *arg){

    pthread_attr_t attr;

    if ( pthread_attr_init(&attr) != 0
      || pthread_create(threads, &attr, thfunct, arg) != 0
      || pthread_attr_destroy(&attr) != 0
  /* || pthread_detach(*threads) != 0 */){

          printf("Start Thread Error.\n");
          return E_START_THREAD;

    }

    printf("Start Thread Success.\n");
    return WORK_SUCCESSFULLY;

}

int main(int argc, char **argv){

    int return_value;

    NSI_initialization_complete = false;

    ready_to_work = false;

    /* Network Setup and Initialization for Zigbee and Wifi */

    pthread_t NSI_routine_thrad;

    return_value = startThread(&NSI_routine_thrad, Initialize_network, NULL);

    if(return_value != WORK_SUCCESSFULLY){

      //perror(errordesc[return_value].message);

      return return_value;

    }

    perror("NSI_SUCCESS");

    return_value = pthread_join(NSI_routine_thrad, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        //perror(strerror(errno));

        return return_value;

    }

    return WORK_SUCCESSFULLY;

}
