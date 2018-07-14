CC = gcc -std=gnu99

pkt_Queue.o: pkt_Queue.c pkt_Queue.h
	$(CC) pkt_Queue.c pkt_Queue.h -c
xbee_API.o: xbee_API.c xbee_API.h
	$(CC) xbee_API.c xbee_API.h -c
xbee_Serial.o: xbee_Serial.c xbee_Serial.h
	$(CC) xbee_Serial.c xbee_Serial.h -c
clean:
	rm -f *.o *.out *.h.gch 
