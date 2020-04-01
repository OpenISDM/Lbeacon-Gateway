# Gateway
#---------------------------------------------------------------------------
CC = gcc
OBJS =  LinkedList.o Mempool.o thpool.o pkt_Queue.o UDP_API.o BeDIS.o
CFLAGS = -std=gnu99 -lrt -lpthread -lzlog -O3
LIB = -L /usr/local/lib
INC = -I ../import
#---------------------------------------------------------------------------
all: Gateway.out
Gateway.out: $(OBJS) Gateway.h Gateway.c
	$(CC) $(CFLAGS) $(OBJS) $(INC) $(LIB) Gateway.c -o ../bin/Gateway.out
LinkedList.o:
	$(CC) $(CFLAGS) ../import/LinkedList.c -c
thpool.o: 
	$(CC) $(CFLAGS) ../import/thpool.c -c
Mempool.o: 
	$(CC) $(CFLAGS) ../import/Mempool.c -c
UDP_API.o: pkt_Queue.o
	$(CC) $(CFLAGS) ../import/UDP_API.c -c
pkt_Queue.o: 
	$(CC) $(CFLAGS) ../import/pkt_Queue.c -c
BeDIS.o: 
	$(CC) $(CFLAGS) ../import/BeDIS.c -c
clean:
	rm -f *.o *.out *.h.gch
