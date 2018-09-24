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

void init_buffer(BufferListHead buffer){

    init_entry(&(buffer.buffer_entry));
    buffer.is_locked = false;
    buffer.is_empty = true;
}

int zigbee_init(){

    /* Struct for storing necessary objects for zigbee connection */
    extern sxbee_config xbee_config;

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

    xbee_connector(&xbee_config);

    /* Start the chain reaction                                             */
    if((error_indicator = xbee_conValidate(xbee_config.con)) != XBEE_ENONE){

        //perror(errordesc[E_XBEE_VALIDATE].message);

        return E_XBEE_VALIDATE;
    }

    return WORK_SUCCESSFULLY;
}



void zigbee_free(){

    /* Struct for storing necessary objects for zigbee connection */
    extern sxbee_config xbee_config;

    /* Release the xbee elements and close the connection. */
    xbee_release(&xbee_config);

    return;

}
