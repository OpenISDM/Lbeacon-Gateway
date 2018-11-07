/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

      GPL 3.0 : The content of this file is subject to the terms and
      cnditions defined in file 'COPYING.txt', which is part of this source
      code package.

  Project Name:

      BeDIPS

  File Description:

      Communication unit: In the alpha version,the sole function of this
      component is to support the communication of NSI and BHM with location
      beacons and the BeDIS server. (In a later version that contains iGaD,
      this components also receives commands from iGaD in the gateway and
      the BeDIS server and broadcasts the commands tolocal LBeacons.)
      Messages exchange happens in CommUnit. This file contain the
      formats of every kind of message and the buffers which store
      messages.And provide with functions which are executed according
      the messages received.

  File Name:

     CommUnit.c

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

      Holly Wang    , hollywang@iis.sinica.edu.tw
      Ray Chao      , raychao5566@gmail.com
      Gary Xiao     , garyh0205@hotmail.com

 */

#include "CommUnit.h"

void init_buffer(BufferListHead *buffer){

    init_entry( &(buffer -> buffer_entry));

    pthread_mutex_init( &buffer -> list_lock, 0);

    buffer -> num_in_list = 0;
}

int zigbee_init(){

    /* Struct for storing necessary objects for zigbee connection */
    extern sxbee_config xbee_config;

    /* The error indicator returns from the libxbee library */
    int error_indicator;

    xbee_config.xbee_mode = XBEE_MODE;

    xbee_config.xbee_device = XBEE_DEVICE;

    xbee_config.xbee_datastream = -1;

    xbee_config.config_location = XBEE_CONFIG_PATH;


    xbee_Serial_Power_Reset(xbee_Serial_Power_Pin);

    xbee_Serial_init( &xbee_config.xbee_datastream,
                     xbee_config.xbee_device);

    xbee_LoadConfig( &xbee_config);

    close(xbee_config.xbee_datastream);

    xbee_initial( &xbee_config);

    xbee_connector( &xbee_config);

    /* Start the chain reaction                                             */
    if((error_indicator = xbee_conValidate(xbee_config.con)) != XBEE_ENONE){

        return E_XBEE_VALIDATE;
    }

    return WORK_SUCCESSFULLY;
}

void zigbee_free(){

    /* Struct for storing necessary objects for zigbee connection */
    extern sxbee_config xbee_config;

    /* Release the xbee elements and close the connection. */
    xbee_release( &xbee_config);

    return;

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

void *wifi_send(BufferListHead *buffer_array[]){

    while (ready_to_work == true) {

        /* If two buffers are all empty, sleep for a while */
        while(buffer_track_list.num_in_list == 0 &&
              buffer_health_list.num_in_list == 0){

            sleep(A_SHORT_TIME);

        }

        /* set the destination server IP */
        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;

        /* creates binary representation of server name
        /* and stores it as sin_addr*/
        inet_pton(AF_INET, /* addr to send */, &server_address.sin_addr);

        /* port in network order format */
        server_address.sin_port = htons(server_port);

        /* open socket */
        int sock;

        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {

                    printf("could not create socket\n");

                    return;

        }

        /* send data via sendto() function to send data to sever */
        sendto(sock, /* buf */, /* size */, 0, &server_address
             , sizeof(server_address) );

        /* close the socket */
        close(sock);

    }

}

void *wifi_receieve(BufferListHead *buffer){

    while (ready_to_work == true) {

        /* Set up the network */
        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;

        /* creates binary representation of server name
        /* and stores it as sin_addr*/
        inet_pton(AF_INET, /* addr to rcv */, &server_address.sin_addr);

        /* port in network order format */
        server_address.sin_port = htons(server_port);

        /* open socket */
        int sock;
        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {

            printf("could not create socket\n");

        }
        /* Recieving and sending have to be splited up into to threads, since
        they are call back functions. It cost too much if they had to wait for
        each other. And these two threads should not start in a while loop. */
        if (recvfrom(sock, /* buf */, /* size */,  0, &server_address
          , sizeof(server_address)) == -1)
        {
            /* error in recieving the file */
        }
        /* Get the command from the sever, add the message or command to the
           buffer */
        Add_to_buffer(&buffer);

    }

}

void *zigbee_send(BufferListHead *buffer){

    time_t start_time = time(NULL);

    while(ready_to_work == true) {

      /* There is a routine time for a period to send the request for tracking
      data from LBeacon. The request also be sent when there is a request from
      the server which means the input buffer is no longer empty. */
      if(start_time - time(NULL) >= A_SHORT_TIME || buffer->num_in_list != 0){

        /* Send the command or message to the LBeacons via zigbee */
        for(int beacon_number = 0; beacon_number < MAX_NUMBER_NODES;
            beacon_number++){

            /* Add the content that to be sent to the gateway to the packet
               queue */
            addpkt( &xbee_config.pkt_Queue, Data,
                   beacon_address[beacon_number], zig_message);

            /* If there are remain some packet need to send in the Queue,
               send the packet */
            xbee_send_pkt(&xbee_config);

        }

        xbee_connector(&xbee_config);

        usleep(XBEE_TIMEOUT);

        start_time = start_time + A_SHORT_TIME;


      }

    }
}

void *zigbee_receive(BufferListHead *buffer_array[]){

    while(ready_to_work == true){

        struct BufferNode *new_node;

        /* Check the connection of call back is enable */
        if(xbee_check_CallBack(&xbee_config, false)){

            /* Error handling TODO */

        }

        /* Get the packet in the receive queue received from the LBeacon */
        pPkt temppkt = get_pkt( &xbee_config.Received_Queue);

        if(temppkt != NULL){

            /* Allocate form zigbee packet memory pool a buffer for received
            data and copy the data from Xbee receive queue to the buffer. */
            new_node = mp_alloc(&node_mempool);

            /* Initialize the entry of the buffer node */
            init_entry(new_node->buffer_entry);

            if(new_node == NULL){

                    ready_to_work = false;
                    return;

            }else{

              /* Copy the content to the buffer_node */
              memcpy(new_node->content, temppkt->content,
                     sizeof(temppkt->content));

              /* Get the zigbee network address from the content and look up
              from Lbeacon_address_map the UUID of the LBeacon, and the
              buffer_index. */
              memcpy(new_node->net_address, temppkt->address,
                     sizeof(temppkt->address));

            }
            /* According to different packet type, insert the node into
            the corresponding buffer list with the buffer_index */
            switch(buffer_ptr -> content.pkt_header.pkt_types){

              case "health_report":

                pthread_mutex_lock(BHM_receive_buffer_list_head.list_lock);

                /* Acquire BHM_recieve_buffer_list_head.list_lock, Insert
                the buffer_node in BHM_receive_buffer_list, release
                list_lock. */
                insert_list_first(&new_node->buffer_entry,
                                  &BHM_receive_buffer_list_head);

                pthread_mutex_unlock(BHM_receive_buffer_list_head.list_lock);
                /* Delete the packet and return the indicator back. */
                delpkt(&xbee_config.Received_Queue);

                break;
                
              case "tracked_object_data":

                  pthread_mutex_lock(LBeacon_receive_buffer_list_head.list_lock);

                  /* Insert the node in LBeacon_receive_buffer_list, and release
                  list_lock. */
                  insert_list_first(&new_node->buffer_entry,
                                      &LBeacon_receive_buffer_list_head);

                  pthread_mutex_unlock(LBeacon_receive_buffer_list_head.list_lock);

                  /* Delete the packet and return the indicator back. */
                  delpkt(&xbee_config.Received_Queue);

                  break;

              case "request_to_join":
                  /* The packet is a request for registration. Acquire
                  NSI_send_buffer_list_head.list_lock, insert
                  the buffer_node in the list, increament
                  num_in_list element by 1 and release list_lock. */

                  /* Get the content of the message to match to the address map,
                     e.g., mac_address for xbee, UUID, and so on. */
                     beacon_join_request(zigbee_macaddr, zigbee_macaddr,
                                          gateway_coordinates,
                                          gateway_loc_description);

                  /* Delete the packet and return the indicator back. */
                      delpkt(&xbee_config.Received_Queue);

                    break;

              default:

                break;

            }

        }
    }
}
