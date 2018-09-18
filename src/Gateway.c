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

void *Initialize_network(){

    //ErrorCode status;

    /* Initialize buffers for sever*/

    init_buffer(buffer_health_list);

    init_buffer(buffer_track_list);

    init_buffer(recieveFromServer);

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

    if(return_value = zigbee_init() != WORK_SUCCESSFULLY){
        /* Error handling and return */
        return E_ZIGBEE_INIT_FAIL;
    }

    ready_to_work = true;

    NSI_initialization_complete = true;

    while( ready_to_work == true ){

    }

}

void *CommUnit_routine(){

    while( ready_to_work == true ){

        //wait for NSI get ready
        while( NSI_initialization_complete != true ){

            sleep(A_LONG_TIME);

        }


        /* Create subthreads for sending and receiving data from or to
           LBeacon and server */
        /* This subthread is created to receive the data form sever via
           wifi */
        pthread_t wifi_receiver_thread;

        return_error_value = startThread(wifi_receiver_thread,
                                         wifi_receiver, NULL);

        if(return_error_value != WORK_SUCCESSFULLY){

            perror(errordesc[E_START_THREAD].message);

        }

        /* This subthread is created to receive the data from LBeacon via
           zigbee */
        pthread_t zigbee_receiver_thread;
        return_error_value = startThread(zigbee_receiver_thread,
                                         zigbee_receiver, NULL);

        if(return_error_value != WORK_SUCCESSFULLY){

            perror(errordesc[E_START_THREAD].message);

        }
        /* This subthread is created to send the data to LBeacon via
           zigbee */
        pthread_t zigbee_sender_thread;
        return_error_value = startThread(zigbee_sender_thread,
                                         zigbee_sender, NULL);

        if(return_error_value != WORK_SUCCESSFULLY){

            perror(errordesc[E_START_THREAD].message);

        }

    }

}


void *BHM_routine(){

    while( ready_to_work == true){

        File *track_file;
        struct List_Entry *list_pointers,

        /* data that will be sent to the server, received by LBeacon */
        /* Create a new file for integrating the tracked data from each
           LBeacon. Each line represents each beacon's data */
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


void *Manage_track_buffer(){

    while( ready_to_work == true){

        File *track_file;

        struct List_Entry *list_pointers,

        /* data that will be sent to the server, received by LBeacon */
        /* Create a new file for integrating the tracked data from each
           LBeacon. Each line represents each beacon's data */
        track_file = fopen("track.txt", "a+");

        if(track_file == NULL){

            track_file = fopen("track.txt", "wt");

        }

        /* Go through the track_buffer to get all the received data that
           is ready to sent to the sever */
        list_for_each(list_pointers, &buffer_track_list.buffer_entry){


        }
        /* Write the data to the file which is to be sent to the sever */
        fwrite(...);
        /* Call the function in Communit.c to execute the steps for
           transmitting.*/
        send_via_wifi("track.txt");

    }

}

void* zigbee_receiver(){

        /* Check the connection of call back is enable */
        if(xbee_check_CallBack(&xbee_config, false)){

            /* Error handling TODO */

        }

        /* Get the packet in the receive queue received from the LBeacon */
        pPkt temppkt = get_pkt(&xbee_config.Received_Queue);

        if(temppkt != NULL){

            /* When the first element of the content in the packet is T, it
               indicates that it is a message for tracking data. */
            if(temppkt -> content[0] == 'T'){

            /* Add the message to the buffer_track_list that is going to send
               to the sever */

            /* Delete the packet and return the indicator back. */
            delpkt(&xbee_config.Received_Queue);


            /* When the first element of the content in the packet is H, it
               indicates that it is a message for health report. */
            }else if(temppkt -> content[0] == 'H'){

            /* Add the message to the buffer_health_list that is going to send
               to the sever */

            /* Delete the packet and return the indicator back. */
               delpkt(&xbee_config.Received_Queue);


            /* When the first element of the content in the packet is R, it
               indicates that it is a message for registration at the very
               first time. */
            }else if(temppkt -> content[0] == 'R'){

            /* Get the content of the message to match to the address map,
               e.g., mac_address for xbee, UUID, and so on. */
                beacon_join_request(zigbee_macaddr, zigbee_macaddr,
                                    gateway_coordinates,
                                    gateway_loc_description);

            /* Delete the packet and return the indicator back. */
            delpkt(&xbee_config.Received_Queue);

            /* If data[0] == '@', callback will be end. */
            }else if(temppkt -> content[0] == '@'){

                xbee_conCallbackSet(xbee_config.con, NULL, NULL);

            }

            delpkt(&xbee_config.Received_Queue);

        }

}


void* zigbee_sender(){

        /* There is no any received command from the sever, sleep for a
           while. Or the timer for counting down the time to ask for the
           data is still counting down, go to sleep */
        while(recieveFromServer.is_empty == true || TIMEOUT){

        sleep(A_SHORT_TIME);

        }

        /* Dequeue the "recieveFromServer" buffer to get the command or
           request to ask for track object data or health repot */

        /* Send the command or message to the LBeacons via zigbee */
        for(int beacon_number = 0; beacon_number < MAX_NUMBER_NODES;
            beacon_number++){

            /* Add the content that to be sent to the gateway to the packet
               queue */
            addpkt(&xbee_config.pkt_Queue, Data,
                   beacon_address[beacon_number], zig_message);

            /* If there are remain some packet need to send in the Queue,
               send the packet */
            xbee_send_pkt(&xbee_config);

        }

        xbee_connector(&xbee_config);

        usleep(XBEE_TIMEOUT);

   return;

}

ErrorCode startThread(pthread_t threads ,void * (*thfunct)(void*), void *arg){

    pthread_attr_t attr;

    if ( pthread_attr_init(&attr) != 0
      || pthread_create(&threads, &attr, thfunct, arg) != 0
      || pthread_attr_destroy(&attr) != 0
      || pthread_detach(threads) != 0) {

          return E_START_THREAD;
    }

  return WORK_SUCCESSFULLY;
  
}

int main(int argc, char **argv){

    int return_value;

    NSI_initialization_complete = false;

    ready_to_work = false;

    /* Network Setup and Initialization for Zigbee and Wifi */

    pthread_t NSI_routine_thrad;

    return_value = startThread(NSI_routine_thrad, Initialize_network, NULL);

    if(return_value != WORK_SUCCESSFULLY){

      perror(errordesc[E_START_COMMUNIT_ROUTINE_THREAD].message);

      return return_value;

    }

    perror("NSI_SUCCESS");

    /* Create threads for Beacon Health Monitor  */

    pthread_t BHM_routine_thread;

    return_value = startThread(BHM_routine_thread, BHM_routine, NULL);

    if(return_value != WORK_SUCCESSFULLY){

        perror(errordesc[E_START_THREAD].message);

        return return_value;

    }

    /* Create threads for Communication Unit  */

    pthread_t CommUnit_routine_thread;

    return_value = startThread(CommUnit_routine_thread, CommUnit_routine, NULL);

    if(return_value != WORK_SUCCESSFULLY){

        perror(errordesc[E_START_THREAD].message);

        return return_value;

    }

    /* Create threads for Beacon track data  */

    pthread_t Tracking_thread;

    return_value = startThread(Tracking_thread, Manage_track_buffer, NULL);

    if(return_value != WORK_SUCCESSFULLY){

        perror(errordesc[E_START_THREAD].message);

        return return_value;

    }

    /* Join threads */

    return_value = pthread_join(Tracking_thread, NULL);

    if(return_value != WORK_SUCCESSFULLY){

        perror(strerror(errno));

        return return_value;

    }


    return_value = pthread_join(CommUnit_routine_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        perror(strerror(errno));

        return return_value;

    }

    return_value = pthread_join(BHM_routine_thread, NULL);

    if (return_value != WORK_SUCCESSFULLY) {

        perror(strerror(errno));

        return return_value;

    }

     return_value = pthread_join(NSI_routine_thrad, NULL);

      if (return_value != WORK_SUCCESSFULLY) {

          perror(strerror(errno));

          return return_value;

      }

    return WORK_SUCCESSFULLY;

}
