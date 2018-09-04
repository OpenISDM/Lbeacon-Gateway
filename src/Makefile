# Gateway
#---------------------------------------------------------------------------
CC = gcc
OBJS = Gateway.o Utilities.o LinkedList.o
CFLAGS = -g
LIB = -L/usr/local/lib

#---------------------------------------------------------------------------
all: Gateway
Gateway: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o Gateway $(LIB) -lrt -lpthread -lmulticobex -lbfb -lbluetooth -lobexftp -lopenobex
Gateway.o: Gateway.c Gateway.h
	$(CC) Gateway.c $(CFLAGS) $(LIB) -c
Utilities.o: Utilities.c Utilities.h
	$(CC) Utilities.c $(CFLAGS) $(LIB) -c
LinkedList.o: LinkedList.c LinkedList.h
	$(CC) LinkedList.c $(CFLAGS) $(LIB) -c
pkt_Queue.o: pkt_Queue.c pkt_Queue.h
	$(CC) pkt_Queue.c pkt_Queue.h -lxbee -c
xbee_Serial.o: xbee_Serial.c xbee_Serial.h
	$(CC) xbee_Serial.c xbee_Serial.h -c
xbee_API.o: xbee_API.c xbee_API.h
	$(CC) xbee_API.c xbee_API.h  -lxbee -c
clean:
	@rm -rf *.o
