/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

       GPL 3.0 : The content of this file is subject to the terms and
       cnditions defined in file 'COPYING.txt', which is part of this
       source code package.

  Project Name:

       BeDIPS

  File Description:

       This file contains the program to connect to Wi-Fi and in
       the project, we use it for data transmission most.

  File Name:

       UDP_API.c

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
       Gary Xiao		, garyh0205@hotmail.com
 */
#include "UDP_API.h"


int udp_initial(pudp_config udp_config){

    int ret;

    // zero out the structure
    memset((char *) &udp_config -> si_server, 0, sizeof(udp_config -> si_server));

    if(ret = init_Packet_Queue( &udp_config -> pkt_Queue) != pkt_Queue_SUCCESS)

        return ret;

    if(ret = init_Packet_Queue( &udp_config -> Received_Queue) != pkt_Queue_SUCCESS)

        return ret;

    //create a send UDP socket
    if ((udp_config -> send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        perror("socket errror.\n");
    }

    //create a recv UDP socket
    if ((udp_config -> recv_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        perror("socket errror.\n");
    }

    udp_config -> si_server.sin_family = AF_INET;
    udp_config -> si_server.sin_port = htons(UDP_LISTEN_PORT);
    udp_config -> si_server.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind recv socket to port
    if( bind(udp_config -> recv_socket , (struct sockaddr *)&udp_config -> si_server, sizeof(udp_config -> si_server) ) == -1)
    {
        perror("bind error.\n");
    }


    pthread_create(&udp_config -> udp_send, NULL, udp_send_pkt, (void*) udp_config);

    usleep(10);

    pthread_create(&udp_config -> udp_receive, NULL, udp_receive_pkt, (void*) udp_config);

    return 0;

}

int udp_addpkt(pkt_ptr pkt_queue, char *raw_addr, char *content, int size){

    printf("udp_addpkt\n");

    int UDP = 3;

    char identification[identification_length];

    memset(&identification, 0, sizeof(char) * identification_length);

    generate_identification(identification, identification_length);

    unsigned int offset = 0;

    unsigned int  Data_fragmentation = 0;

    char tmp_content[MAX_DATA_LENGTH];

    char address[Address_length];

    memset(address, 0, Address_length);

    int address_loc = 0;

    printf(" adj address\n");

    for(int n = 0; n < 4; n++){

        printf("n = %d\n", n);

        int count = 0;
        unsigned char tmp[3];
        memset(&tmp, 0, sizeof(char) * 3);

        while(count != 3){


            if (raw_addr[address_loc] == '.'){

                address_loc ++;

                break;

            }
            else{
                tmp[count] = raw_addr[address_loc];

                count ++;

                address_loc ++;

                if(address_loc >= strlen(raw_addr))
                    break;
            }

        }

        for(int lo = 0; lo < 3;lo ++){

            if ((3 - count) > lo )
                address[n * 3 + lo] = '0';
            else{
                address[n * 3 + lo] = tmp[lo - (3 - count)];
            }

        }
        if (count == 3)
            address_loc ++;
    }

    printf("Address :");

    print_content(address, Address_length);


    memset(&tmp_content, 0, sizeof(char) * MAX_DATA_LENGTH);

    for(int loc = 0;loc < MAX_DATA_LENGTH;loc ++){

        tmp_content[loc] = content[loc];

        if((loc  + 1) == size)

            break;

    }

    addpkt(pkt_queue, UDP, identification
         , Data_fragmentation, offset, address, tmp_content, MAX_DATA_LENGTH);

    return 0;
}

void *udp_send_pkt(void *udpconfig){

    pudp_config udp_config = (pudp_config) udpconfig;

    struct sockaddr_in si_send;

    int socketaddr_len = sizeof(si_send);

    char dest_address[17];

    printf("send pkt.\n");

    while(!(udp_config -> shutdown)){

        if(!(is_null( &udp_config -> pkt_Queue))){

            memset(&dest_address, 0, sizeof(char) * 17);

            char *tmp_address = hex_to_char(udp_config -> pkt_Queue.Queue[udp_config
                  -> pkt_Queue.front].address, 12);

            array_copy(tmp_address, dest_address, 3);

            dest_address[3] = '.';

            array_copy(&tmp_address[3], &dest_address[4], 3);

            dest_address[7] = '.';

            array_copy(&tmp_address[6], &dest_address[8], 3);

            dest_address[11] = '.';

            array_copy(&tmp_address[9], &dest_address[12], 3);

            printf("Dest Address : %s\n", dest_address );

            memset(&si_send, 0, sizeof(si_send));
            si_send.sin_family = AF_INET;
            si_send.sin_port   = htons(UDP_LISTEN_PORT);

            if (inet_aton(dest_address, &si_send.sin_addr) == 0){

                perror("inet_aton error.\n");

            }

            if (sendto(udp_config -> send_socket, udp_config -> pkt_Queue.Queue[udp_config
                  -> pkt_Queue.front].content, MAX_DATA_LENGTH,0 , (struct sockaddr *) &si_send, socketaddr_len) == -1){

                perror("recvfrom error.\n");

            }

            delpkt( &udp_config -> pkt_Queue);

        }

    }

    printf("Exit Send.\n");

}

void *udp_receive_pkt(void *udpconfig){

    pudp_config udp_config = (pudp_config) udpconfig;

    int recv_len;

    char recv_buf[MAX_DATA_LENGTH];

    struct sockaddr_in si_recv;

    int socketaddr_len = sizeof(si_recv);

    //keep listening for data
    while(!(udp_config -> shutdown)){

        memset(&si_recv, 0, sizeof(si_recv));

        printf("recv pkt.\n");

        printf("Waiting for data...");
        fflush(stdout);

        printf("MAX_DATA_LENGH : %d\n", MAX_DATA_LENGTH);

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(udp_config -> recv_socket, recv_buf, MAX_DATA_LENGTH, 0, (struct sockaddr *) &si_recv, &socketaddr_len)) == -1){

            perror("recvfrom error.\n");

        }

        if(udp_addpkt(&udp_config -> Received_Queue, inet_ntoa(si_recv.sin_addr), recv_buf, recv_len) == -1){

            perror("udp_addpkt error.\n");

        }

        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_recv.sin_addr), ntohs(si_recv.sin_port));
        printf("Data: %s\n" , recv_buf);



    }
    printf("Exit Receive.\n");
}

int udp_release(pudp_config udp_config){

    pthread_join(udp_config -> udp_send, NULL);

    pthread_join(udp_config -> udp_receive, NULL);

    close(udp_config -> send_socket);

    close(udp_config -> recv_socket);

    Free_Packet_Queue( &udp_config -> pkt_Queue);

    Free_Packet_Queue( &udp_config -> Received_Queue);

    return 0;
}
