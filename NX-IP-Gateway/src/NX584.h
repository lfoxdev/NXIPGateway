// NX584.h
// Emulation of NX-584 home automation module

#ifndef _NX584_h
#define _NX584_h

#include "NX584Message.h"
#include <WiFiServer.h>

class NX584
{
 protected:
 	 enum nx584InternalResponse {
		 FAIL, ACK, NACK, REJECT
	 };
	 NetworkServer tcpServer;
	 NetworkClient client;
	 TaskHandle_t gwCommunicationTask;
     void receive(uint8_t *buf, size_t len);
     void send(NX584Message* msg);
	 virtual size_t preProcessReceive(uint8_t *buf, size_t len);
	 virtual size_t postProcessSend(uint8_t *buf, size_t len);
	 virtual void onClientDisconnection();
	 void sendInternal(nx584InternalResponse response);	 
	 void processNX584Request(NX584Message* msg);
	 void ownLocationRead(uint16_t loc);
     static void processCommunication(void *arg);

 public:
	 void start();
	 void stop();
	 NX584() {
		tcpServer = NetworkServer(NX584_PORT);
	 }
	 virtual ~NX584() = default;
};

extern NX584 nx584;

#endif

