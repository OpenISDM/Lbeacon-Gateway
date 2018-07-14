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
 *      This file demonstrate how the pkt_Queue work and how to use it.
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

#include "../src/pkt_Queue.h"

int main(){

  pkt_ptr pkt_queue = malloc(sizeof(spkt_ptr));

  /* Initialize Queue for packets                                          */
  init_Packet_Queue(pkt_queue);

  if(pkt_queue->front->next == NULL && pkt_queue->rear->next == NULL){
      printf("Queue is null\n");
  }

    addpkt(pkt_queue, Data, "0123456789ABCDEF", "1");
    delpkt(pkt_queue);
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "2");
    delpkt(pkt_queue);
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "3");
    delpkt(pkt_queue);
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "4");
    delpkt(pkt_queue);
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "5");
    delpkt(pkt_queue);
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "6");
    delpkt(pkt_queue);
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "7");
    delpkt(pkt_queue);
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "8");
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "9");
    delpkt(pkt_queue);

    delpkt(pkt_queue);

    addpkt(pkt_queue, Data, "0123456789ABCDEF", "10");
    addpkt(pkt_queue, Data, "0123456789ABCDEF", "11");

    delallpkt(pkt_queue);

    Free_Packet_Queue(pkt_queue);

    return 0;
}
