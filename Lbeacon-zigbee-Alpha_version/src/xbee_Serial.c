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
 *   	This file contains the program to connect to xbee and send AT command
 *      to xbee.
 *
 * File Name:
 *
 *      xbee_Serial.c
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

#include "xbee_Serial.h"

int xbee_Serial_init(int *xbee_datastream, char *xbee_device ){
    //  Open the Serial
    //  The flags (defined in fcntl.h):
    //  Access modes (use 1 of these):
    //      O_RDONLY - Open for reading only.
    //      O_RDWR - Open for reading and writing.
    //      O_WRONLY - Open for writing only.
    //
    //  O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode.
    //      When set read requests on the file can return immediately with a
    //      failure status. If there is no input immediately available
    //      (instead of blocking). Likewise, write requests can also return
    //      immediately with a failure status. If the output can't be written
    //      immediately.
    //
    //  O_NOCTTY - When set and path identifies a terminal device,
    //             open() shall not cause the terminal device to become
    //             the controlling terminal for the process.

    //Open in non blocking read/write mode  // | O_NOCTTY | O_NDELAY
    if ((*xbee_datastream = open(xbee_device, O_RDWR )) == -1){
        //ERROR  CAN'T OPEN SERIAL PORT
        printf("Error - Unable to open Serial.  Ensure it is not in use by \
        another application\n");
        return -1;
    }

    //CONFIGURE THE SERIAL
    //The flags (defined in /usr/include/termios.h ---
    //see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
    //  Baud rate: B1200, B2400, B4800, B9600, B19200, B38400,
    //             B57600, B115200, B230400, B460800, B500000,
    //             B576000, B921600, B1000000, B1152000, B1500000,
    //             B2000000, B2500000, B3000000, B3500000, B4000000
    //  CSIZE  : CS5, CS6, CS7, CS8
    //  CLOCAL : Ignore modem status lines
    //  CREAD  : Enable receiver
    //  IGNPAR : Ignore characters with parity errors
    //  ICRNL  : Map CR to NL on input
    //  (Use for ASCII comms where you want to auto correct end of line
    //  characters don't use for bianry comms!)
    //  PARENB : Parity enable
    //  PARODD : Odd parity (else even)

    // set new parameters to the serial device
    struct termios newtio;

    fcntl(*xbee_datastream, F_SETFL, 0);
    // set everything to 0
    bzero(&newtio, sizeof(newtio));

    // again set everything to 0
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag |= B9600; // Set Baudrate first time
    newtio.c_cflag |= CLOCAL; // Local line - do not change "owner" of port
    newtio.c_cflag |= CREAD; // Enable receiver

    newtio.c_cflag &= ~ECHO; // Disable echoing of input characters
    newtio.c_cflag &= ~ECHOE;

    // set to 8N1
    newtio.c_cflag &= ~PARENB; // no parentybyte
    newtio.c_cflag &= ~CSTOPB; // 1 stop bit
    newtio.c_cflag &= ~CSIZE; // Mask the character size bits
    newtio.c_cflag |= CS8; // 8 data bits

    // output mode to
    newtio.c_oflag = 0;

    // Set teh baudrate for sure
    cfsetispeed(&newtio, B9600);
    cfsetospeed(&newtio, B9600);

    newtio.c_cc[VTIME] = 10; /* inter-character timer  */
    newtio.c_cc[VMIN] = 0; /* blocking read until  */

    tcflush(*xbee_datastream, TCIFLUSH); // flush pending data

    // set the new defined settings
    if (tcsetattr(*xbee_datastream, TCSANOW, &newtio)) {
        perror("could not set the serial settings!");
        return -99;
    }


    return 0;
 }

 int xbee_Serial_Tx(int* xbee_datastream, int xbee_Serial_buffer, char* Data){

    //----- TX BYTES -----
    unsigned char tx_buffer[xbee_Serial_buffer];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];

    for(int i = 0; i < strlen(Data) ; i++)
        *p_tx_buffer++ = Data[i];

    if (*xbee_datastream != -1){
        printf("Start Write\n");
        int count = write(*xbee_datastream, &tx_buffer[0],
                            (p_tx_buffer - &tx_buffer[0]));
        //Datastream, bytes to write, number of bytes to write
        if (count < 0)
            printf("Serial TX error\n");
        printf("Count : %d\n",count);
    }

    return 0;
}

int xbee_Serial_Rx(int *xbee_datastream, int xbee_Serial_buffer, char* Data){
    int Waiting;
    if(strlen(Data) > 0){
        Waiting = Remain;
    }
    else{
        Waiting = Ended;
    }

    int Received = 0;
    //----- CHECK FOR ANY RX BYTES -----
    if (*xbee_datastream != -1)
    {
        // Read up to xbee_Serial_buffer characters from the port
        // if they are there
        unsigned char rx_buffer[xbee_Serial_buffer];
        //Datastream, buffer to store in, number of bytes to read (max)
        printf("Start Read\n");
        int rx_length = 0;
        do{
            rx_length = read(*xbee_datastream, (void*)rx_buffer
                                            , xbee_Serial_buffer);
            rx_buffer[rx_length] = '\0';
            if(rx_length == 0){
                printf("No Data Received\n");
            }
 	        else{
                printf("%d bytes read : %s\n", rx_length - 1, rx_buffer);
                if((strlen(Data) - Received) > (rx_length - 1)){
                    printf("OverFlow\n");
                    return -1;
                }
                for(int i = 0 ; i < rx_length -1; i++){
                    printf("%c and %c is the same?\n", rx_buffer[i]
                                            , Data[i + Received]);
                    if(rx_buffer[i] != Data[i + Received]){
                        printf("Data Receive Not Matched\n");
                        return -1;
                    }
                }
                Received += rx_length - 1;
                if(strlen(Data) == Received)
                    Waiting = Ended;
            }
         }while(Waiting != Ended);
     }
     else{
         printf("xbee_datastream Error.\n");
         return -1;
     }

     return 0;
 }

char* xbee_Serial_Return(int *xbee_datastream, int xbee_Serial_buffer){

    unsigned char rx_buffer[xbee_Serial_buffer];
    int rx_length;
   //----- CHECK FOR ANY RX BYTES -----
   if (*xbee_datastream != -1)
   {
       // Read up to xbee_Serial_buffer characters from the port if they are there
       printf("Start Read\n");
       rx_length = 0;
       //Datastream, buffer to store in, number of bytes to read (max)
       rx_length = read(*xbee_datastream, (void*)rx_buffer, xbee_Serial_buffer);
       //Bytes received
       rx_buffer[rx_length] = '\0';
       if(rx_length == 0){
           printf("No Data Received\n");
       }
       else{
           printf("%d bytes read : %s\n", rx_length - 1, rx_buffer);
       }
    }
    else{
        printf("xbee_datastream Error.\n");
        return "NULL";
    }

    char* return_received = malloc(sizeof(char)*rx_length);

    for(int n = 0; n < rx_length; n++ ){
        return_received[n] = rx_buffer[n];
    }

    return return_received;
}

int  xbee_Send_Command(int *xbee_datastream, int xbee_Serial_buffer
                            , char *Command, char *Command_Result){
    printf("Send Command\n");
    if(xbee_Serial_Tx(xbee_datastream, xbee_Serial_buffer, "+++") != 0){
        return -1;
    }

    usleep(100000);

    printf("Receive Result\n");
    if(xbee_Serial_Rx(xbee_datastream, xbee_Serial_buffer, "OK") != 0){
        return -1;
    }

    printf("Send Command\n");
    if(xbee_Serial_Tx(xbee_datastream, xbee_Serial_buffer,Command) != 0){
        return -1;
    }

    usleep(100000);

    printf("Receive Result\n");
    if(xbee_Serial_Rx(xbee_datastream, xbee_Serial_buffer,Command_Result) != 0){
        return -1;
    }

    printf("Sended\n");

    return 0;
}

char* xbee_Send_Command_result(int *xbee_datastream, int xbee_Serial_buffer
                                                            , char *Command){
    printf("Send Command\n");
    if((xbee_Send_Command(xbee_datastream, xbee_Serial_buffer, "+++", "OK"))
                                                                    != 0){
        return "NULL";
    }

    if(xbee_Serial_Tx(xbee_datastream, xbee_Serial_buffer,Command) != 0){
        return "NULL";
    }

    usleep(100000);

    char* result =  xbee_Serial_Return(xbee_datastream, xbee_Serial_buffer);

    return result;
}
