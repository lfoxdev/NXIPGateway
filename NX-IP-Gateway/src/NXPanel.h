// NXPanel.h
// NX bus logical layer
// Processing of incoming messages to panel state, request-response system and system log monitoring

#ifndef _NXGATEWAY_h
#define _NXGATEWAY_h

#include "Config.h"
#include "NXBus.h"
#include "NXPanelState.h"
#include "NXPanelStateItem.h"
#include "NXPanelFunctions.h"
#include "NXMessage.h"
#include "CommonStructures.h"
#include "Utils.h"

#include "freertos/task.h" 
#include "freertos/queue.h"
#include <NetworkServer.h>

enum requestResult : uint8_t {
	rqOK, 		// Request is acknowleged by panel
	rqBADRQ, 	// Request contains invalid parameters
	rqTOUT, 	// There was no answer from panel
	rqBUSY, 	// Request queue is full
	rqNACK		// Request not acknowleged by panel
};

String requestResultToStr(requestResult r);

class NXPanel
{
friend class NXBus;
private:
	TaskHandle_t nxDebugProcessTask;
	TaskHandle_t nxMsgProcessTask;
	TaskHandle_t logMonitorTask;

	// Received messages processing
	static void processNXDebug(void* arg);
	static void processNXMessage(void* arg);
	static void processBroadcastMessage(NXPanel* self, NXMessage msg);
	static void processSinglecastMessage(NXPanel* self, NXMessage msg);
	// System log monitoring task
	static void monitorLogEntries(void* arg);

 protected:
	NXBus bus;
	QueueHandle_t rawMsgQueue;
	QueueHandle_t nxRecvQueue;
	QueueHandle_t nxSendQueue;	 
	TaskHandle_t communicatingTask = NULL;
	NXMessage currentResponse;
	SemaphoreHandle_t busSemaphore;
	SemaphoreHandle_t responseSemaphore;
	QueueHandle_t logMonitorQueue;

 public:
	NXPanelState state;
	NXPanelFunctions fun;
	uint8_t ownBusAddr;
	NetworkServer debugTcpServer = NetworkServer(DEBUG_PORT);
	NXPanel() : bus(this), state(this), fun(this) {}
	void start(uint8_t busAddress);
	void stop();
	void enableDebug();
	void disableDebug();
	void handlePanelStateChange(IPanelStateItem* item);
	NXBusCounters getBusCounters() { return bus.counters; };

	// Request-response system
	requestResult makeRequest(const NXMessage* request, NXMessage* response = nullptr);
	requestResult makeRequestCheckAck(const NXMessage* request);

};

extern NXPanel panel;

#endif

