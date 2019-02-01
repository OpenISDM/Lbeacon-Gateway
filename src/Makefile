# Gateway
#---------------------------------------------------------------------------
CC = gcc
OBJS =  LinkedList.o Mempool.o thpool.o pkt_Queue.o UDP_API.o
CFLAGS = -std=gnu99 -lrt -lpthread -lzlog
LIB = -L /usr/local/lib
#---------------------------------------------------------------------------
all: Gateway.out
Gateway.out: $(OBJS) BeDIS.h BeDIS.c Gateway.h Gateway.c
	$(CC) $(CFLAGS) $(OBJS) $(LIB) BeDIS.c Gateway.c -o ../bin/Gateway.out
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
