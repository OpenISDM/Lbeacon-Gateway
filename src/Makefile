# Gateway
#---------------------------------------------------------------------------
CC = gcc
OBJS =  Utilities.o LinkedList.o Mempool.o thpool.o pkt_Queue.o
CFLAGS = -std=gnu99 -lrt -lpthread

#---------------------------------------------------------------------------
all: Gateway.out
Gateway.out: $(OBJS) BeDIS.h Gateway.h Gateway.c
	$(CC) $(CFLAGS) $(OBJS) BeDIS.h Gateway.h Gateway.c -o Gateway.out
# -lmulticobex -lbfb -lbluetooth -lobexftp -lopenobex
Utilities.o: Utilities.c Utilities.h BeDIS.h
	$(CC) $(CFLAGS) Utilities.h Utilities.c BeDIS.h -c
LinkedList.o: LinkedList.c LinkedList.h
	$(CC) $(CFLAGS) LinkedList.h LinkedList.c -c
thpool.o: thpool.c thpool.h
	$(CC) $(CFLAGS) thpool.h thpool.c -c
Mempool.o: Mempool.c Mempool.h
	$(CC) $(CFLAGS) Mempool.h Mempool.c -c
pkt_Queue.o: pkt_Queue.c pkt_Queue.h
	$(CC) $(CFLAGS) pkt_Queue.h pkt_Queue.c -c
clean:
	rm -f *.o *.out *.h.gch
