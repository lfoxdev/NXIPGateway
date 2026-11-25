// Config.h
// Constants and runtime configuration

#pragma once

#include <IPAddress.h>
#include <WString.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#define VERSION "1.1.0"

// Panel address on NX bus
#define PANEL 0x08

// Zone and partition count
#define MAX_PARTITION 8
#define MAX_ZONE 48

// Maximum raw NX message length, bytes
#define MAX_PACKET_SIZE 25

// NX bus queue length
#define NX_MSG_QUEUE_LEN 30
#define NX_DBG_QUEUE_LEN 20

// How long to wait for free space in send queue
#define REQUEST_TOUT_MS 1000
// How long to wait for panel response
#define RESPONSE_TOUT_MS 1000

// How many retries would be done if panel not answers
#define RQ_RETRIES 3

// How often quick stats (serial, time...) would be updated on web page
#define WEBUI_UPD_PERIOD_STAT 1000
// How often full panel status would be updated on web page
#define WEBUI_UPD_PERIOD_FULL 10000

// TCP ports
#define DIRECT_UI_PORT 23
#define NX584_PORT 1584
#define DL900_PORT 9998
#define DEBUG_PORT 3333

// NX bus serial configuration
#define RX_PIN          GPIO_NUM_14  
#define TX_PIN          GPIO_NUM_15  
// #define DBG_PIN         GPIO_NUM_2	// If defined would strob on each serial I/O interrupt - connect logic analyzer here
#define INVERSE_RX      false
#define INVERSE_TX      true

// Pull this pin down on boot to reset configuration to default
#define CFGCLR_PIN		GPIO_NUM_4

// Turn this to true if panel sends unknown messages or messages with length different from defined in NXMessage.h
#define DISABLE_MSG_END_DETECTION_BY_LEN	true

class Config_t {
public:
	uint8_t busAddress;
	IPAddress ip;
	IPAddress gateway;
	IPAddress subnet;
	IPAddress dns;
	String mqttServer;
	String mqttUsername;
	String mqttPassword;
	String mqttID;
	bool nx584Enabled;
	bool dl900Enabled;
	bool dcuiEnabled;
	bool dbgEnabled;
	String password;
	void load();
};

extern Config_t Config;
