# Gateway
#---------------------------------------------------------------------------
CC = gcc
OBJS =  BeDIS.o LinkedList.o Mempool.o thpool.o pkt_Queue.o UDP_API.o
CFLAGS = -std=gnu99 -lrt -lpthread

#---------------------------------------------------------------------------
all: Gateway.out
Gateway.out: $(OBJS) BeDIS.h Gateway.h Gateway.c
	$(CC) $(CFLAGS) $(OBJS) Gateway.c -o Gateway.out
BeDIS.o: BeDIS.c BeDIS.h
	$(CC) $(CFLAGS) BeDIS.c -c
LinkedList.o: LinkedList.c LinkedList.h
	$(CC) $(CFLAGS) LinkedList.c -c
thpool.o: thpool.c thpool.h
	$(CC) $(CFLAGS) thpool.c -c
Mempool.o: Mempool.c Mempool.h
	$(CC) $(CFLAGS) Mempool.c -c
UDP_API.o: UDP_API.c UDP_API.h pkt_Queue.o
	$(CC) $(CFLAGS) UDP_API.c -c
pkt_Queue.o: pkt_Queue.c pkt_Queue.h
	$(CC) $(CFLAGS) pkt_Queue.c -c
clean:
	rm -f *.o *.out *.h.gch
