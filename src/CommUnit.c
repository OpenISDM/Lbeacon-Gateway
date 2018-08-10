/*
* Copyright (c) 2016 Academia Sinica, Institute of Information Science
*
* License:
*
*      GPL 3.0 : The content of this file is subject to the terms and
*      cnditions defined in file 'COPYING.txt', which is part of this source
*      code package.
*
* Project Name:
*
*      BeDIPS
*
* File Description:
*
*       Communication unit: In the alpha version,the sole function of this 
*       component is to support the communication of NSI and BHM with location 
*       beacons and the BeDIS server. (In a later version that contains iGaD,
*       this components also receives commands from iGaD in the gateway and 
*       the BeDIS server and broadcasts the commands tolocal LBeacons.) 
*       Messages exchange happens in CommUnit. This file contain the 
*       formats of every kind of message and the buffers which store 
*       messages.And provide with functions which are executed according 
*       the messages received.     
*
* File Name:
*
*     CommUnit.c
*
* Abstract:
*
*      BeDIPS uses LBeacons to deliver 3D coordinates and textual
*      descriptions of their locations to users' devices. Basically, a
*      LBeacon is an inexpensive, Bluetooth Smart Ready device. The 3D
*      coordinates and location description of every LBeacon are retrieved
*      from BeDIS (Building/environment Data and Information System) and
*      stored locally during deployment and maintenance times. Once
*      initialized, each LBeacon broadcasts its coordinates and location
*      description to Bluetooth enabled user devices within its coverage
*      area.
*
* Authors:
*
*      Hank Kung, hank910140@gmail.com
*      
*/

#include "CommUnit.h"

void *CommUnit_routine(){

    //Buffer sendToBeacon.name = "sendToBeacon";
    //Buffer recieveFromBeacon.name = "recieveFromBeacon";
    //Buffer sendToServer.name = "sendToServer"; 
    //Buffer recieveFromServer.name = "recieveFromServer";

    /* When initialization completes */
    CommUnit_initialization_complete = true;

    //wait for NSI get ready
    while(!zigbee_is_ready){
        sleep(A_LONG_TIME);
    }
    
    

    pthread_t wifi_receiver_thread;
    /* Rename it to prevent from getting confused with the one in
    main thread */
    return_error_value = startThread(wifi_receiver_thread, wifi_receiver, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
    }

    pthread_t wifi_sender_thread;
    return_error_value = startThread(wifi_sender_thread, wifi_sender, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
    }

    pthread_t zigbee_receiver_thread;
    return_error_value = startThread(zigbee_receiver_thread, zigbee_receiver, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);

    }
    pthread_t zigbee_sender_thread;
    return_error_value = startThread(zigbee_sender_thread, zigbee_sender, NULL);

    if(return_error_value != WORK_SCUCESSFULLY){

        perror(errordesc[E_START_THREAD].message);
    }

    while (system_is_shutting_down == false) {
        //   do a chunk of work and/or sleep for a short time

        /* If both Zigbee queue and UDP queue are empty then sleep 
        a short time*/

        if(1) sleep(A_SHORT_TIME);
        
        }
 }

void init_buffer(Buffer buffer){
    //clear the buffer by filling null, it might have previously received data
    memset(buffer.content,'\0', BUFFER_SIZE);
    buffer.front = buffer.rear = 0;
    buffer.is_locked = false;
    buffer.is_empty = true;
}



void *buffer_dequeue(Buffer buffer){

    /* Wait for the turn to use the queue */
    while(buffer.is_locked){
        sleep(A_SHORT_TIME);
    }
    buffer.is_locked = true;
    if(buffer.front == buffer.rear){
        printf("%s is empty currently, can not dequeue anymore\n",buffer.name);
        buffer.is_empty = true;
        buffer.is_locked = false;
        //return;
    }   
    /* Execute function according the command name */

    /* Free the control*/
    buffer.is_locked = false;
}

void buffer_enqueue(Buffer buffer, FILE *item){
    /* Wait for the turn to use the queue */
    while(buffer.is_locked){
        sleep(A_SHORT_TIME);
    }
    buffer.is_locked = true;
    /* If buffer is full currently then just skip Enqueue till there's
    room for it.P.S. Overflow problem will got solved later */
    if( (buffer.front == buffer.rear + 1) || 
    (buffer.front == 0 && buffer.rear == BUFFER_SIZE-1)){
        printf("Queue is full now\n");
        return;
    }
    /* *front is -1 when the buffer is empty */
    if(buffer.front == -1) buffer.front = 0;
    buffer.rear = (buffer.rear + 1) % BUFFER_SIZE;
    buffer.content[buffer.rear] = *item;
    /* Set flag true anyway */
    buffer.is_locked = false;
}

bool is_buffer_empty(Buffer buffer){
    return buffer.is_empty;
}

void *wifi_reciever(Buffer buf){
    while (system_is_shutting_down == false) {

        /* Recieving and sending have to be splited up into to threads, since
        they are call back functions. It cost too much if they had to wait for 
        each other. And these two threads should not start in a while loop. */
        if (recvfrom(s, recieveFromServer.content, BUFFER_SIZE, 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            //die("recvfrom()");
        }
        /* get string from the file sent by server */ 
        char *content;
        char *type;
        generate_command(content, type);
        addpkt(&pkt_send_queue, Data, Gateway, content);

        /* Dequeue buffer */
        if(!is_buffer_empty(recieveFromServer)){
            FILE *item = buffer_dequeue(recieveFromServer);
            /* Read the file dequeued from buffer, then execute command */
        }
    }
}

void *wifi_sender(){
    while (system_is_shutting_down == false) {

    }
}

void *zigbee_reciever(){
    while (system_is_shutting_down == false) {
        /* If the recieving queue is not empty. */
        if(!is_null(pkt_recv_queue)){
            //
        }
        else
            sleep(A_SHORT_TIME);

    }
    printf("Stop xbee ...\n");
    Free_Packet_Queue(&pkt_recv_queue);

    /* Close connection                                                      */
    if ((ret = xbee_conEnd(con)) != XBEE_ENONE) {
        xbee_log(xbee, 10, "xbee_conEnd() returned: %d", ret);
        return ret;
    }
    Free_Packet_Queue(&pkt_recv_queue);
    printf("Stop connection Succeeded\n");

    /* Close xbee                                                            */
    xbee_shutdown(xbee);
    printf("Shutdown Xbee Succeeded\n");
}

void *zigbee_sender(){
    while (system_is_shutting_down == false) {
        /* If the sneding queue is not empty. */
        if(is_null(pkt_send_queue)){
            //send item by ZigBee API
            xbee_send_pkt(con, &pkt_send_queue);
            xbee_connector(&xbee, &con, &pkt_send_queue);
        }
        else
            sleep(A_SHORT_TIME);

    }
    printf("Stop xbee ...\n");
    Free_Packet_Queue(&pkt_send_queue);

    /* Close connection                                                      */
    if ((ret = xbee_conEnd(con)) != XBEE_ENONE) {
        xbee_log(xbee, 10, "xbee_conEnd() returned: %d", ret);
        return ret;
    }
    Free_Packet_Queue(&pkt_send_queue);
    printf("Stop connection Succeeded\n");

    /* Close xbee                                                            */
    xbee_shutdown(xbee);
    printf("Shutdown Xbee Succeeded\n");
}

void generate_command(const char *command, const char *type){
    /* If it's a pulling message */
    char *transaction;
    if(type == "0000"){
        transaction = Broadcast;
        strcat(transaction,command);
        printf("command content: %s", transaction);
        addpkt(pkt_send_queue, Data, Broadcast, transaction);
    }
    else{
        for(int i =0; i<=MAX_ERROR_CODE; i++){
            if(beacon_address[i].network_address == type){
                char *mac = beacon_address[i].mac_addr;
                transaction = mac;
                strcat(transaction,command);
                printf("command content: %s", transaction);
                addpkt(pkt_send_queue, Data, mac, transaction);
                break;
            }
        }
    }
}