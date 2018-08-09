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
 *   	This file contains the header of function declarations and variable
 *      used in pkt_Queue.h
 *
 * File Name:
 *
 *      pkt_Queue.h
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
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define Gateway   "0000000000000000"
#define Broadcast "000000000000FFFF"
#define BUFFER_SIZE 256

enum {Data, Local_AT};

/* packet format in the Queue */
struct pkt {

    //"Data"
	int type;

	// Brocast:     000000000000FFFF;
	// Coordinator: 0000000000000000
	unsigned char address[8];

	// Data
	char *content;

    struct pkt *next;
  //turn the buffer to static
};

typedef struct pkt sPkt;
typedef sPkt *pPkt;



typedef enum {Lock_Queue,unLock_Queue} Locker;

struct pkt_header {

    // front point to the first of thr Pkt Queue
    // rear  point to the end of the Pkt Queue
    sPkt front;
    sPkt rear;
    //sPkt buffer[BUFFER_SIZE]; 

    unsigned char address[8];
    bool locker;
};

typedef struct pkt_header spkt_ptr;
typedef spkt_ptr *pkt_ptr;

/* init_Packet_Queue
 *  Initialize Queue for packets
 * Parameter:
 *  pkt_queue : A struct stored pointers of the first and the last of packet.
 * Return Value:
 *  None
 */
void init_Packet_Queue(pkt_ptr pkt_queue);

/*
 * Free_Packet_Queue
 *     Release all the packets in the packet queue, the header and
 *     the tail of the packet queue and release the struct stored the pointer of
 *      the packet queue.
 * Parameter:
 *     pkt_queue : A struct stored the first and the last of the packet queue.
 * Return Value:
 *     None
 */
void Free_Packet_Queue(pkt_ptr pkt_queue);

/*
 * addpkt
 *     Add new packet into the packet queue we assigned.
 * Parameter:
 *     pkt_queue : A struct stored the first and the last of the packet queue.
 *     type      : Record the type of packets working environment.
 *     raw_addr  : The destnation address of the packet.
 *     content   : The content we decided to send.
 * Return Value:
 *     None
 */
void addpkt(pkt_ptr pkt_queue, int type, char *raw_addr, char *content);

/*
 * delpkt
 *     delete the first of the packet queue we assigned.
 * Parameter:
 *     pkt_queue : A struct stored the first and the last of the packet queue.
 * Return Value:
 *     None
 */
void delpkt(pkt_ptr pkt_queue);

void delallpkt(pkt_ptr pkt_queue);

/*
 * type_to_str
 *     TO convert type to it's original type name.
 * Parameter:
 *     type: A variable stored packet needed send type.
 * Return Value:
 *     Return a char pointer which is it's type name.
 */
char *type_to_str(int type);

/*
 * print_address
 *     Convert hex type address to char type address.
 * Parameter:
 *     address: A address stored in Hex.
 * Return Value:
 *     char_addr: A address stored in char convert from address.
 */
char *print_address(unsigned char* address);

/* display_pkt
 *     display the packet we decide to see.
 * Parameter:
 *     content: The title we want to show in front of the packet content.
 *     pkt: The packet we want to see it's content.
 * Return Value:
 *     None
 */
void display_pkt(char* content, pPkt pkt);

/*
 * Fill_address
 *     Convert the address from raw(char) to addr(Hex).
 * Parameter:
 *     raw: The original char type address.
 *     addr: The destnation variable to store the converted result.
 * Return Value:
 *     None
 */
void Fill_Address(char *raw, unsigned char* addr);

/*
 * address_compare
 *      Compare the address whether is the same.
 * Parameter:
 *      addr1: the address we want to compare.
 *      addr2: the address we want to compare.
 * Return Value:
 *      bool: if true, the same.
 */
bool address_compare(unsigned char* addr1,unsigned char* addr2);
/*
 * address_compare
 *      Compare the address whether is the same.
 * Parameter:
 *      addr1: the src address we copy from.
 *      addr2: the dest address we copy to.
 * Return Value:
 *      None
 */
void address_copy(unsigned char* src_addr, unsigned char* dest_addr);

/*
 * is_null
 *      check if pkt_Queue is null.
 * Parameter:
 *      pkt_Queue: the pkt we stored in the Queue.
 * Return Value:
 *      None
 */
bool is_null(pkt_ptr pkt_Queue);

/*
 * queue-len
 *      Count the length of the queue.
 * Parameter:
 *      pkt_Queue: The queue we want to count.
 * Return Value:
 *      int: the length of the Queue.
 */
int queue_len(pkt_ptr pkt_queue);

