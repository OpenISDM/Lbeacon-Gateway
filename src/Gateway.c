/*
 Copyright (c) 2016 Academia Sinica, Institute of Information Science

 License:

      GPL 3.0 : The content of this file is subject to the terms and
      cnditions defined in file 'COPYING.txt', which is part of this source
      code package.

 Project Name:

      BeDIPS

 File Description:

      This file contains the program to transmit the data or information from
      LBeacons through Zigbee or UDP. Main tasks includes network setup and 
      initialization, Beacon health monitor and comminication unit. Gateway 
      takes the role as coordinator.

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

      Han Wang, hollywang@iis.sinica.edu.tw
      Jake Lee, jakelee@iis.sinica.edu.tw
      Johnson Su, johnsonsu@iis.sinica.edu.tw
      Hank Kung, hank910140@gmail.com
      Ray Chao, raychao5566@gmail.com
      
*/


#include "Gateway.h"



/* coordinator initializes the zigbee network:
- if (PAN ID == 0) scan nearby network and chooses a PAN ID;
- channel scan to find a good operating channel;
- ready to access join requests from Lbeacons;
- Set up Zigbee connection by calling Zigbee_routine in LBeacon_Zigbee.h */
void initialize_network(){


    int status; 

    /* Initialize two buffer for sever*/
    init_buffer(sendToServer);
    init_buffer(recieveFromServer);

    /* Initialize two buffers for LBeacon */
    init_buffer(sendToBeacon);
    init_buffer(recieveFromBeacon);
    

    /* set up WIFI connection */
    /* open temporary wpa_supplicant.conf file to setup wifi environment*/
    FILE *cfgfile = fopen("/etc/wpa_supplicant/wpa_supplicant.conf ","w");
    fwrite(*cfgfile,"ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
                update_config=1
                country=CN
                network={
                ssid="Wifi_ssid"
                psk="passphrasehere"
                priority=5
                }");
    
    fclose(*cfgfile);
    /* check if WiFi is connected .... */
    status = system("ping google.com");
    if (-1 != status){
        printf("Wifi is not connected....\n")
        system("sudo reboot");
            return;
    }

    /* wifi connection is completed, set the flag wifi_is_ready to true */
    wifi_is_ready = true;

    /* Initialize Zigbee, after finishing, set the flag zigbee_is_ready to 
       true */   
    zigbee_is_ready = true;

    /* ZigBee connection done */


    // finish phase 2 initialization (in ways TBD)
    NSI_initialization_complete = true;
    
    // wait for other components to complete initialization
    while ( (system_is_shutting_down == false) &&
    (ready_to_work == false))
    {
         sleep(A_SHORT_TIME);
    }
    
    /* Ready to work, check for system shutdown flag periodically */
    while (system_is_shutting_down == false) {
        //do a chunk of work and/or sleep for a short time
        sleep(A_SHORT_TIME);
    }

    close(s);
    /* Upon fatal failure, set ready_to_work = false and
    then call NSIcleanupExit( )*/
    ready_to_work = false;
    //NSIcleanupExit();
    // wait for all threads to have exited then returns
    
}

void *address_map_manager(){ 

    beacon_count = 1;
    //gateway info
    char *zigbee_macaddr;
    Coordinates gateway_coordinates;
    char * gateway_loc_description;
    char *gateway_barcode;
    
    //Fill the gateway information into the address table
    //Gateway's index is always 0
    beacon_join_request(0, zigbee_macaddr, zigbee_macaddr, gateway_coordinates,
                        gateway_loc_description, gateway_barcode);
    while(system_is_shutting_down == false){
        
        //if a new join request && (beacon_count>=32)
        //startthread(beacon_join_request());
    }
}

void *CommUnit_routine(){


    /* When initialization completes */
    Network_initialization_complete = true;

    //wait for NSI get ready
    while(!zigbee_is_ready){
        sleep(A_LONG_TIME);
    }
    
    
    /* Create four subthreads for sending and receiving data from or to 
       LBeacon and server */
    /* This subthread is created to receive the data form sever via wifi */    
    pthread_t wifi_receiver_thread;

    return_error_value = startThread(wifi_receiver_thread, 
                                     wifi_receiver, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
    }
    
    /* This subthread is created to send the data to sever via wifi */
    pthread_t wifi_sender_thread;
    return_error_value = startThread(wifi_sender_thread, 
                                     wifi_sender, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
    }

    /* This subthread is created to receive the data from LBeacon via 
       zigbee */
    pthread_t zigbee_receiver_thread;
    return_error_value = startThread(zigbee_receiver_thread, 
                                     zigbee_receiver, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);

    }
    /* This subthread is created to send the data to LBeacon via 
       zigbee */
    pthread_t zigbee_sender_thread;
    return_error_value = startThread(zigbee_sender_thread, 
                                     zigbee_sender, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
    }

    while (system_is_shutting_down == false) {

        /* If both Zigbee queue and UDP queue are empty then sleep 
        a short time*/

        if(1) sleep(A_SHORT_TIME);
        
        }
 }


void *BHM_routine(){

    for (int i = 0; i<beacon_count; i++) {
        /* Default value is true; If beacon is failed, then set to false */
        health_report[i] = true;
    }
    // when initialization completes,
    BHM_initialization_complete = true;
     while (system_is_shutting_down == false) {
    //    do a chunk of work and/or sleep for a short time
         //RFHR(); //might return a boolean array
         //broadcast
         sleep(PERIOD_TO_MONITOR);
    }
    ready_to_work = false;
    //BHM_cleanup_exit();
}


ErrorCode startThread(pthread_t threads ,void * (*thfunct)(void*), void *arg){

    pthread_attr_t attr;

    if ( pthread_attr_init(&attr) != 0
      || pthread_create(&threads, &attr, thfunct, arg) != 0
      || pthread_attr_destroy(&attr) != 0
      || pthread_detach(threads) != 0) {

    return E_START_THREAD;
  }

  return WORK_SCUCESSFULLY;

}



void cleanup_exit(){

    ready_to_work = false;

    return;

}

int main(int argc, char **argv)
{
    
    /* Define and initialize all importent golbal variables including */
    system_is_shutting_down = false;
    ready_to_work = false;
    initialization_failed = false;
    NSI_initialization_complete = false;
    BHM_initialization_complete = false;
    CommUnit_initialization_complete = false;
    wifi_is_ready = false;
    zigbee_is_ready = false;

    int return_value;

    Address_map beacon_address [MAX_NUMBER_NODES];

    pthread_t NSI_routine_thread;

    return_value = startThread(NSI_routine_thread, NSI_routine, NULL);

    if(return_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
        cleanup_exit();
    }

    pthread_t BHM_routine_thread;

    return_value = startThread(BHM_routine_thread, BHM_routine, NULL);

    if(return_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
        cleanup_exit();
    }

    pthread_t CommUnit_routine_thread;

    return_value = startThread(CommUnit_routine_thread, CommUnit_routine, NULL);

    if(return_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
        cleanup_exit();
    }

    while(1){
        sleep(A_LONG_TIME);
    }

    cleanup_exit();

}

