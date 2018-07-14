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
 *   	This file contains the header of  function declarations and variable
 *      used in Serial.c
 *
 * File Name:
 *
 *      xbee_Serial.h
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

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>         //Used for Serial
 #include <fcntl.h>          //Used for Serial
 #include <termios.h>        //Used for Serial
 #include <string.h>

#ifndef xbee_Serial_H
#define xbee_Serial_H

 /*
  * A flag to record does the command returned value.
  * Remain : the command we send haven't return value.
  * Ended  : the command we send had return value.
  */
 enum{ Remain , Ended };

 //-------------------------
 //----- SETUP Serial -----
 //-------------------------

 //Initialize xbee Serial connection and get the pointer of the xbee
 int xbee_Serial_init(int *xbee_datastream, char *xbee_device );

 //Send Serial command to xbee
 int xbee_Serial_Tx(int *xbee_datastream, int xbee_Serial_buffer, char* Data);

 //Receive Serial command from xbee
 int xbee_Serial_Rx(int *xbee_datastream, int xbee_Serial_buffer, char* Data);

 //Receive Serial command from xbee and return it.
 char* xbee_Serial_Return(int *xbee_datastream, int xbee_Serial_buffer);

 //Send AT command to xbee
 int xbee_Send_Command(int *xbee_datastream, int xbee_Serial_buffer, char* Command, char* Command_Result);

 //Send AT command to xbee
 char* xbee_Send_Command_result(int *xbee_datastream, int xbee_Serial_buffer, char* Command);

#endif
