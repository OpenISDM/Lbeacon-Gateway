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
#include "Gateway.h"

void *CommUnit_routine(){

    Buffer sendToBeacon.name = "sendToBeacon";
    Buffer recieveFromBeacon.name = "recieveFromBeacon";
    Buffer sendToServer.name = "sendToServer"; 
    Buffer recieveFromServer.name = "recieveFromServer";
    init_buffer(sendToBeacon);
    init_buffer(recieveFromBeacon);
    init_buffer(sendToServer);
    init_buffer(recieveFromServer);

    //when initialization completes,
    CommUnit_initialization_complete = true;

    pthread_t wifi_reciever_thread;
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
        if(!is_buffer_empty(recieveFromBeacon))
        {
            FILE *item = buffer_dequeue(recieveFromBeacon);
            buffer_enqueue(sendToServer, item);
        }
        if(!is_buffer_empty(recieveFromServer))){
            /* Read the file and execute command */
            /* Depends on command category, it can be broadcast to all 
            beacons or for some specific beacons.
            */
        }

        if() sleep(A_SHORT_TIME);
        

        }
    CommUnit_cleanup_exit();
    return;
 }

void init_buffer(Buffer buffer){
    //clear the buffer by filling null, it might have previously received data
     memset(buffer.content,'\0', BUFFER_SIZE);
    buffer.front = buffer.rear = 0;
    buffer.is_lock = false;
    buffer.is_empty = true;
}



void *buffer_dequeue(Buffer buffer){

    /* Wait for the turn to use the queue */
    while(buffer._is_locked){
        sleep(A_SHORT_TIME);
    }
    buufer.is_locked = true;
    if(buffer.front == buffer.rear){
        printf("%s is empty currently, can not dequeue anymore\n",buffer.name);
        buffer.is_empty = true;
        buffer.is_locked = false;
        return;
    }   
    /* Execute function according the command name */

    /* Free the control*/
    buffer.is_locked = false;
}

void buffer_enqueue(Buffer buffer, FILE *item){
    /* Wait for the turn to use the queue */
    while(buffer.is_locked){
        sllep(A_SHORT_TIME);
    }
    buffer.is_locked = true;
    /* If buffer is full currently then just skip Enqueue till there's
    room for it.P.S. Overflow problem will got solved later */
    if( (buffer.front == buffer.rear + 1) || 
    (buffer.front == 0 && buffer.rear == BUFFER_SIZE-1)){
        printf("Queue is full now\n")
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

void *wifi_reciever(){
    while (system_is_shutting_down == false) {

        /* Recieving and sending have to be splited up into to threads, since
        they are call back functions. It cost too much if they had to wait for 
        each other. And these two threads should not start in a while loop. */
        if (recvfrom(s, buffer, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }
        puts(buffer);
        /* Dequeue buffer */
        if(!is_buffer_empty(recieveFromServer){
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
        //recieve the string from beacon, then pack it into
        //a file.
    }

}

void *zigbee_sender(){
    while (system_is_shutting_down == false) {
        if(!is_buffer_empty(sendToBeacon)){
            //get item from front
            //send item by ZigBee API
            //move front back once
        }

    }

}

void RFHR(){

}