// DCUI.h
// Telnet interface for debugging purposes, exposing raw NX bus for user requests

#ifndef _DIRECTIFACE_h
#define _DIRECTIFACE_h

#include "ESPTelnet.h"

class DCUI
{
 protected:
	 TaskHandle_t gwCommunicationTask;
	 static void processCommunication(void* arg);

 public:
	ESPTelnet telnet;
	void start();
	void stop();
};

extern DCUI dcui;

#endif

