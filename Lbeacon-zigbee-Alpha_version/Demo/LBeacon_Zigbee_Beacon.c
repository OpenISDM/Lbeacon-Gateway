/*
 * Copyright (c) 2016 Academia Sinica, Institute of Information Science
 *
 * License:
 *
 *      GPL 3.0 : The content of this file is subject to the terms and
 *      cnditions defined in file 'COPYING.txt', which is part of this
 *      source code package.
 *
 * Project Name:
 *
 *      BeDIPS
 *
 * File Description:
 *
 *   	This file contains the program to set up a star network by XBEE S2C
 *      module in order to deal with NSI(Network Setup and Initialization)
 *      and Data transmission between Gateway and LBeacon. And This file is
 *		for LBeacon.
 *
 * File Name:
 *
 *      LBeacon_Zigbee_LBeacon.c
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
 *      Gary Xiao		, garyh0205@hotmail.com
 */

#include "../src/xbee_API.h"

int main(void) {

    char* xbee_mode = "xbeeZB";

    char* xbee_device = "/dev/ttyAMA0";

    int xbee_baudrate = 9600;

    int LogLevel = 100;

    struct xbee *xbee;

    struct xbee_con *con;

    pkt_ptr pkt_Queue = malloc(sizeof(spkt_ptr));

    xbee_initial(xbee_mode, xbee_device, xbee_baudrate
                            , LogLevel, &xbee, pkt_Queue);

    printf("Start establishing Connection to xbee\n");


    /*--------------Configuration for connection in Data mode----------------*/
    /* In this mode we aim to get Data.                                      */
    /*-----------------------------------------------------------------------*/

    printf("Establishing Connection...\n");

    xbee_connector(&xbee, &con, pkt_Queue);

    printf("Connection Successfully Established\n");

    /* Start the chain reaction!                                             */

    if((ret = xbee_conValidate(con)) != XBEE_ENONE){
        xbee_log(xbee, 1, "con unvalidate ret : %d", ret);
        return ret;
    }

    while(1) {
        /* Pointer point_to_CallBack will store the callback function.       */
        /* If pointer point_to_CallBack is NULL, break the Loop              */
        void *point_to_CallBack;

        if ((ret = xbee_conCallbackGet(con, (xbee_t_conCallback*)
            &point_to_CallBack))!= XBEE_ENONE) {
            xbee_log(xbee, -1, "xbee_conCallbackGet() returned: %d", ret);
            return ret;
        }

        if (point_to_CallBack == NULL){
            printf("Stop Xbee...\n");
            break;
        }


    	addpkt(pkt_Queue, Data, Gateway, "AAAAA");

        /* If there are remain some packet need to send in the Queue,        */
        /* send the packet                                                   */
        if(pkt_Queue->front->next != NULL){

            xbee_conTx(con, NULL, pkt_Queue->front->next->content);

            delpkt(pkt_Queue);
        }
        else{
            xbee_log(xbee, -1, "xbee packet Queue is NULL.");
        }
        usleep(2000000);
    }

    printf("Jump out while\n");

    Free_Packet_Queue(pkt_Queue);

    /* Close connection                                                      */
    if ((ret = xbee_conEnd(con)) != XBEE_ENONE) {
        xbee_log(xbee, 10, "xbee_conEnd() returned: %d", ret);
        return ret;
    }

    printf("Stop connection Succeeded\n");

    /* Close xbee                                                            */
    xbee_shutdown(xbee);
    printf("Shutdown Xbee Succeeded\n");

    return 0;
}
