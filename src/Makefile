# Gateway
#---------------------------------------------------------------------------
CC = gcc
OBJS =  Utilities.o LinkedList.o Mempool.o pkt_Queue.o xbee_Serial.o xbee_API.o CommUnit.o
CFLAGS = -std=gnu99

#---------------------------------------------------------------------------
all: Gateway
Gateway: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) Gateway.h Gateway.c -o Gateway -lxbee -lwiringPi -lrt -lpthread
# -lmulticobex -lbfb -lbluetooth -lobexftp -lopenobex
CommUnit.o: CommUnit.c CommUnit.h
	$(CC) $(CFLAGS) CommUnit.c CommUnit.h -c
Utilities.o: Utilities.c Utilities.h
	$(CC) $(CFLAGS) Utilities.c Utilities.h -c
LinkedList.o: LinkedList.c LinkedList.h
	$(CC) $(CFLAGS) LinkedList.c LinkedList.h -c
Mempool.o: Mempool.c Mempool.h
	$(CC) Mempool.c Mempool.h -c
pkt_Queue.o: pkt_Queue.c pkt_Queue.h
	$(CC) $(CFLAGS) pkt_Queue.c pkt_Queue.h -c
xbee_Serial.o: xbee_Serial.c xbee_Serial.h
	$(CC) $(CFLAGS) xbee_Serial.c xbee_Serial.h -c
xbee_API.o: xbee_API.c xbee_API.h
	$(CC) $(CFLAGS) xbee_API.c xbee_API.h -c
clean:
	rm -f *.o *.out *.h.gch
