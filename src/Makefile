# Gateway
#---------------------------------------------------------------------------
CC = gcc
OBJS =  Utilities.o LinkedList.o Mempool.o thpool.o pkt_Queue.o
CFLAGS = -std=gnu99 -lrt -lpthread

#---------------------------------------------------------------------------
all: Gateway.out
Gateway.out: $(OBJS) BeDIS.h Gateway.h Gateway.c
	$(CC) $(CFLAGS) $(OBJS) Gateway.c -o Gateway.out
# -lmulticobex -lbfb -lbluetooth -lobexftp -lopenobex
Utilities.o: Utilities.c Utilities.h BeDIS.h
	$(CC) $(CFLAGS) Utilities.c -c
LinkedList.o: LinkedList.c LinkedList.h
	$(CC) $(CFLAGS) LinkedList.c -c
thpool.o: thpool.c thpool.h
	$(CC) $(CFLAGS) thpool.c -c
Mempool.o: Mempool.c Mempool.h
	$(CC) $(CFLAGS) Mempool.c -c
pkt_Queue.o: pkt_Queue.c pkt_Queue.h
	$(CC) $(CFLAGS) pkt_Queue.c -c
clean:
	rm -f *.o *.out *.h.gch
