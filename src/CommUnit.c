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

      Holly Wang, hollywang@iis.sinica.edu.tw
      Ray Chao, raychao5566@gmail.com

*/

#include "CommUnit.h"



void init_buffer(BufferHead buffer){

    init_entry(buffer.buffer_entry);
    buffer.is_locked = false;
    buffer.is_empty = true;
}


void *wifi_reciever(){

    while (ready_to_work == true) {

        /* Set up the network */
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
        if (recvfrom(...) == -1)
        {
            /* error in recieving the file */
        }
        /* Get the command from the sever, add the message or command to the
           buffer */
        Add_to_buffer(recieveFromServer);


    }


}

void sned_via_wifi(char *file_name){

    while (system_is_shutting_down == false) {

        /* If two buffers are all empty, sleep for a while */
        while(buffer_track_list.is_empty == true &&
              buffer_health_list.is_empty == true){

            sleep(A_SHORT_TIME);

        }


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
                    return;

        }



        /* send data via sendto() function to send data to sever */
        sendto(...);

        /* received echoed data back for hand shacking with sever*/
        recvfrom(...);



        /* close the socket */
        close(sock);



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



int zigbee_init(){

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

    return WOEK_SUCCESSFULLY;
}



void zigbee_free(){


    /* Release the xbee elements and close the connection. */
    xbee_release(&xbee_config);

    zlog_info(category_health_report,
              "Stop Xbee connection Succeeded\n");

    return;

}
