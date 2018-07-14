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
 *   	This file demonstrate how to use Serial.c and Serial.h to setup xbee
 *      S2C in AT mode.
 *
 * File Name:
 *
 *      xbee_Serial_Demo.c
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

#include "../src/xbee_Serial.h"

int main(){
    int xbee_datastream = -1;

    char* xbee_device = "/dev/ttyAMA0";

    int xbee_Serial_buffer = 50;

    xbee_Serial_init(&xbee_datastream, xbee_device);

    printf("ATID 55\n");
    xbee_Send_Command(&xbee_datastream, xbee_Serial_buffer, "ATID 55\r", "OK");
    getchar();
    printf("ATSH\n");
    xbee_Send_Command_result(&xbee_datastream, xbee_Serial_buffer, "ATSH\r");
    getchar();
    printf("ATWR\n");
    xbee_Send_Command(&xbee_datastream, xbee_Serial_buffer, "ATWR\r", "OK");
    getchar();
    //----- CLOSE THE SERIAL -----
    close(xbee_datastream);

    return 0;
}
