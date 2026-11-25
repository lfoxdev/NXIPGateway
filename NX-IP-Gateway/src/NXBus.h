// NXBus.h
// NX bus L2 layer
// Converting raw bytes to logical messages and vice versa, checksums, ping-pong, placement of outgoing messages in flow

#ifndef _NXBUS_h
#define _NXBUS_h

#include "Config.h"
#include "NXSerial.h"
#include "NXMessage.h"

#include <cstdint>
#include <cstddef>
#include "freertos/FreeRTOS.h"

struct NXBusCounters {
	uint32_t received;
	uint16_t too_short;
	uint16_t buffer_overflows;
	uint16_t incorrect_checksums;
	uint16_t lost_cycles;
	uint16_t queue_overflows;
	uint32_t sent;
	uint16_t send_retries;
	uint32_t last_poll_ms;
};

struct RAWMessage_t {
	uint16_t buf[MAX_PACKET_SIZE];
	uint8_t len;
};

// Forward declarations
class NXPanel;

class NXBus
{
 friend class NXPanel;
 friend class NXSerial;
 private:
	void IRAM_ATTR processPacketFromISR(uint16_t* rx_buf, size_t rx_len, BaseType_t* pxHigherPriorityTaskWoken);
	size_t IRAM_ATTR makePacket(NXMessage* msg, uint16_t* buf);
	uint16_t pong_buf[MAX_PACKET_SIZE];
	uint8_t pong_len = 0;
	uint8_t own_address;
	volatile uint8_t last_cycle = 0xFF;
	bool registered = false;

 protected:
	NXPanel* panel;
	NXSerial nxserial;
	void IRAM_ATTR onBusIdleDetection(uint16_t* rx_buf, size_t rx_len, BaseType_t* pxHigherPriorityTaskWoken);
	void IRAM_ATTR onByteReceived(uint16_t* rx_buf, size_t rx_len, BaseType_t* pxHigherPriorityTaskWoken);
	NXBusCounters counters;
	bool debugEnabled = false;

 public:
	 explicit NXBus(NXPanel* _p) : panel(_p), nxserial(this) {}
	 void start(uint8_t bus_address);
	 void stop();
	 void setPongMessage(NXMessage msg);
};

#endif

