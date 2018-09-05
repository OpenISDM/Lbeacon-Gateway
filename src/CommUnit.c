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



void init_buffer(Buffer buffer){

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
        
        while (wifi_is_ready == true){

        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;

        /* creates binary representation of server name
        /* and stores it as sin_addr*/
        inet_pton(AF_INET, server_name, &server_address.sin_addr);

        /* port in network order format */
        server_address.sin_port = htons(server_port);

        /* open socket */
        int sock;
        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            
            printf("could not create socket\n");
            
            return 1;
        
        }
        /* Recieving and sending have to be splited up into to threads, since
        they are call back functions. It cost too much if they had to wait for
        each other. And these two threads should not start in a while loop. */
        if (recvfrom(sock, receive_from_server_buffer, MAXLINE, 0, NULL, NULL) == -1)
        {
            /* error in recieving the file */
        }
        
        /* get message from the server */
        char *content;
        char *type;
        
        /* generate corresponding commands */
        generate_command(content, type);
        
        /* broadcast to LBeacons */
        addpkt(&pkt_send_queue, Data, Gateway, content);

        /* clear buffer */
        clearBuf(receive_from_server_buffer);
        }
    }
    

}

void *wifi_sender(){
   
    while (system_is_shutting_down == false) {
        
        /* Check if the WiFi is connected */
        while (wifi_is_ready == true){
            /* set the destination server IP */
            struct sockaddr_in server_address;
            memset(&server_address, 0, sizeof(server_address));
            server_address.sin_family = AF_INET;

            /* creates binary representation of server name
            /* and stores it as sin_addr*/
            inet_pton(AF_INET, server_name, &server_address.sin_addr);

            /* port in network order format */
            server_address.sin_port = htons(server_port);

            /* open socket */
            int sock;
            
            if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
                    
                    printf("could not create socket\n");
                    return 1;

            }

            /* data that will be sent to the server, received by LBeacon */
            /* Create a new file with tracked_object_list's data*/
            track_file = fopen("track.txt", "a+");

            if(track_file == NULL){

                track_file = fopen("track.txt", "wt");

            }
            if(track_file == NULL){

                perror(errordesc[E_OPEN_FILE].message);
                return false;

            } 
    
            fwrite(send_to_server_buffer,1,sizeof(send_to_server_buffer),track_file);
            clearBuf(send_to_server_buffer);
            
            /* send data */
            sendto(sock, track_file, strlen(track_file), MAXLINE,
                   (struct sockaddr*)&server_address, sizeof(server_address));

            /* received echoed data back */
            recvfrom(sock, receive_from_server_buffer, MAXLINE, 0, NULL, NULL);

            receive_from_server_buffer[len] = '\0';
            printf("received: '%s'\n", receive_from_server_buffer);

            /* close the socket */
            close(sock);
        
        }
    
    }


}

/* This function for thread that receives the track object data, health 
   report or registeration from the LBeacon. Check the first element in 
   the received message to learn the type. */
void *zigbee_reciever(){    

     /* Check the connection of call back is enable */ 
    if(xbee_check_CallBack(&xbee_config, false)){
      
      perror(error_xbee[E_CALL_BACK].message);
      zlog_info(category_health_report, 
                error_xbee[E_CALL_BACK].message);
      
      return NOT_YET_POLLED;
    
    };

    /* Get the packet in the receive queue received from the LBeacon */
    pPkt temppkt = get_pkt(&xbee_config.Received_Queue);
        
    if(temppkt != NULL){

        /* When the first element of the content in the packet is T, it 
           indicates that it is a message for tracking data. */ 
        if(temppkt -> content[0] == 'T'){

          /* Delete the packet and return the indicator back. */
          delpkt(&xbee_config.Received_Queue);
         

         /* When the first element of the content in the packet is H, it 
           indicates that it is a message for health report. */  
        }else if(temppkt -> content[0] == 'H'){

          /* Delete the packet and return the indicator back. */
          delpkt(&xbee_config.Received_Queue);
          

        /* When the first element of the content in the packet is R, it 
           indicates that it is a message for registration at the very 
           first time. */    
        }else if(temppkt -> content[0] == 'R'){

          /* Get the content of the message to match to the address map, 
             e.g., mac_address for xbee, UUID, and so on. */  


          /* Delete the packet and return the indicator back. */
          delpkt(&xbee_config.Received_Queue);

         /* If data[0] == '@', callback will be end. */
        }else if(temppkt -> content[0] == '@'){

            xbee_conCallbackSet(xbee_config.con, NULL, NULL);

        }

         delpkt(&xbee_config.Received_Queue);   

    }
    

}


void beacon_join_request(char *ID, char *mac, 
                         Coordinates Beacon_Coordinates,
                         char *Loc_Description){
    
    /* Copy all the necessary information received from the LBeacon to the 
       address map. */
    strcpy(beacon_address[index].beacon_uuid, ID);
    strcpy(beacon_address[index].mac_addr, mac);
    strcpy(beacon_address[index].loc_description, Loc_Description);
    strcpy(beacon_address[index].beacon_coordinates.X_coordinates, 
                                Beacon_Coordinates.X_coordinates);
    strcpy(beacon_address[index].beacon_coordinates.Y_coordinates, 
                                Beacon_Coordinates.Y_coordinates);
    strcpy(beacon_address[index].beacon_coordinates.Z_coordinates, 
                                Beacon_Coordinates.Z_coordinates);



}


void *zigbee_sender(){

    /* There is no any received command from the sever, sleep for a while */
    while(recieveFromServer.is_empty == true){

        sleep(A_SHORT_TIME);

    }

    /* Dequeue the "recieveFromServer" buffer to get the command or request 
       to ask for track object data or health repot */

    /* Send the command or message to the LBeacons via zigbee */
    for(int beacon_number = 0; beacon_number < MAX_NUMBER_NODES; 
        beacon_number++){

        /* Add the content that to be sent to the gateway to the packet 
           queue */
        addpkt(&xbee_config.pkt_Queue, Data, beacon_address[], zig_message);

        /* If there are remain some packet need to send in the Queue,send the 
        packet */                                      
        xbee_send_pkt(&xbee_config);

    }
       
    xbee_connector(&xbee_config);

    usleep(XBEE_TIMEOUT);
        

   return;
    
}

ErrorCode_Xbee zigbee_init(){

    /* The error indicator returns from the libxbee library */
    int error_indicator;

    xbee_config.xbee_mode = XBEE_MODE;

    xbee_config.xbee_device = XBEE_DEVICE;

    xbee_config.xbee_datastream = XBEE_DATASTREAM;

    xbee_config.config_location = XBEE_CONFIG_PATH;

    
    xbee_Serial_Power_Reset(xbee_Serial_Power_Pin);

    xbee_Serial_init(&xbee_config.xbee_datastream, 
                     xbee_config.xbee_device);

    xbee_LoadConfig(&xbee_config);

    close(xbee_config.xbee_datastream);

    xbee_initial(&xbee_config);
   
#ifdef Debugging 

    zlog_debug(category_debug, "Establishing Connection...");
      
#endif

    xbee_connector(&xbee_config);

    
#ifdef Debugging 
   
    zlog_debug(category_debug, 
               "Zigbee Connection Successfully Established");
      
#endif

    zlog_info(category_health_report, 
              "Zigbee Connection Successfully Established");

    /* Start the chain reaction                                             */
    if((error_indicator = xbee_conValidate(xbee_config.con)) != XBEE_ENONE){

#ifdef Debugging 

        zlog_debug(category_debug, "con unvalidate ret : %d", 
                   error_indicator);
      
#endif      
        
        perror(error_xbee[E_XBEE_VALIDATE].message);
        zlog_info(category_health_report, 
                  error_xbee[E_XBEE_VALIDATE].message);
        
        return E_XBEE_VALIDATE;
    }

    return XBEE_SUCCESSFULLY;
}



void zigbee_free(){


    /* Release the xbee elements and close the connection. */
    xbee_release(&xbee_config);
   
    zlog_info(category_health_report, 
              "Stop Xbee connection Succeeded\n");

    return;

}

