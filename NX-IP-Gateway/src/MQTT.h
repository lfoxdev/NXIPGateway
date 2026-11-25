// MQTT.h
// MQTT telemetry and commands

#ifndef _MQTT_h
#define _MQTT_h

#include "mqtt_client.h"
#include <ArduinoJson.h>
#include "WString.h"

class MQTTMgr
{
 private:
	 esp_mqtt_client_handle_t client;
	 static void eventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
	 bool mqtt_connected = false;
	 StaticJsonDocument<256> currentPayload;
	 void onCommandReceived();
	 void onBypassReceived();
 public:
	void start(String brokerUri, String username, String password);
	void send(String topic, String payload, bool retain, int qos);
	void sendErr(String payload);
	bool connected();
	String error;

};

extern MQTTMgr MQTT;

#endif

