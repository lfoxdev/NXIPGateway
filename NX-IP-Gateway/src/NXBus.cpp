#include "NXBus.h"
#include "NXPanel.h"
#include "NXMessage.h"
#include "Utils.h"

#define LOG_TAG "BUS"

// Convert to uint8_t by cutting-off parity byte
static inline uint8_t dp(uint16_t x) { return static_cast<uint8_t>(x & 0xFF); };

void NXBus::processPacketFromISR(uint16_t* rx_buf, size_t rx_len, BaseType_t* pxHigherPriorityTaskWoken)
{
	nxserial.resetRxBuf();
	counters.received++;
	// Place raw message to debug queue
	if (debugEnabled) {
		RAWMessage_t rawMsg;
		rawMsg.len = rx_len;
		memcpy(rawMsg.buf, rx_buf, rx_len * sizeof(rx_buf[0]));
		xQueueSendFromISR(panel->rawMsgQueue, &rawMsg, pxHigherPriorityTaskWoken);
	}
	// Drop too short messages
	if (rx_len < 4) {
		counters.too_short++;
		return;
	}
	// Drop too long messages
	if (rx_len >= MAX_PACKET_SIZE) {
		counters.buffer_overflows++;
		return;
	}
	// Validate checksum
	uint8_t s1 = 0, s2 = 0;
	for (size_t i = 0; i < rx_len - 2; i++) {
		fletcher(dp(rx_buf[i]), s1, s2);
	}
	if (s1 != dp(rx_buf[rx_len - 2]) || s2 != dp(rx_buf[rx_len - 1])) {
		counters.incorrect_checksums++;
		return;
	}
	// Check if broadcast cycles 0-7 go continiously
	uint8_t cycle = dp(rx_buf[0]);
	if (cycle <= 0x07) {
		if (cycle - last_cycle != 1 && last_cycle - cycle != 7 && last_cycle != 0xFF)
			counters.lost_cycles++;
		last_cycle = cycle;
	}
	// Create NXMessage from buffer and place to queue
	NXMessage msg = NXMessage();
	msg.dest = dp(rx_buf[0]);
	for (size_t i = 1; i < rx_len - 2; i++) {
		msg.data.bytes[i - 1] = dp(rx_buf[i]);
	}
	msg.length = rx_len - 3;
	if (xQueueSendFromISR(panel->nxRecvQueue, &msg, pxHigherPriorityTaskWoken) != pdPASS)
		counters.queue_overflows++;
	// Check if we are allowed to send anything after received packet
	if (msg.dest == own_address || (msg.dest <= 0x07 && msg.dest == own_address % 8)) {
		if (registered && xQueueReceiveFromISR(panel->nxSendQueue, &msg, pxHigherPriorityTaskWoken) == pdPASS) {
			// Send message from queue if we have it
			uint16_t tx_buf[MAX_PACKET_SIZE];
			size_t tx_len = makePacket(&msg, tx_buf);
			if (tx_len == 0)
				return;
			// Place own message to debug queue
			if (debugEnabled) {
				RAWMessage_t rawMsg;
				rawMsg.len = tx_len;
				memcpy(rawMsg.buf, tx_buf, tx_len * sizeof(tx_buf[0]));
				xQueueSendFromISR(panel->rawMsgQueue, &rawMsg, pxHigherPriorityTaskWoken);
			}
			nxserial.send(tx_buf, tx_len);
			counters.sent++;
		}
		else if (msg.dest == own_address && msg.msgNum() == 0x10) {
			// Reply ping messages addressed to us
			nxserial.send(pong_buf, pong_len);
			registered = true;
			counters.sent++;
			counters.last_poll_ms = esp_timer_get_time() / 1000;
			// Place own message to debug queue
			if (debugEnabled) {
				RAWMessage_t rawMsg;
				rawMsg.len = pong_len;
				memcpy(rawMsg.buf, pong_buf, pong_len * sizeof(pong_buf[0]));
				xQueueSendFromISR(panel->rawMsgQueue, &rawMsg, pxHigherPriorityTaskWoken);
			}
		}
	}
}

size_t NXBus::makePacket(NXMessage* msg, uint16_t* buf)
{
	size_t len = msg->msgLen();
	if (len == 0)
		return 0;
	buf[0] = msg->dest | 0b100000000; // First byte has 9th bit set to 1
	uint8_t s1 = 0, s2 = 0;
	fletcher(msg->dest, s1, s2);
	for (size_t i = 0; i < len; i++) {
		buf[i + 1] = msg->data.bytes[i];
		fletcher(msg->data.bytes[i], s1, s2);
	}
	buf[len + 1] = s1;
	buf[len + 2] = s2;
	return len + 3;
}

void NXBus::onBusIdleDetection(uint16_t* rx_buf, size_t rx_len, BaseType_t* pxHigherPriorityTaskWoken)
{
	processPacketFromISR(rx_buf, rx_len, pxHigherPriorityTaskWoken);
}

void NXBus::onByteReceived(uint16_t* rx_buf, size_t rx_len, BaseType_t* pxHigherPriorityTaskWoken)
{
	if (!DISABLE_MSG_END_DETECTION_BY_LEN && rx_len >= 4 && rx_len == NXMessage::msgLen(dp(rx_buf[1])) + 3) {
		processPacketFromISR(rx_buf, rx_len, pxHigherPriorityTaskWoken);
	}
}

void NXBus::start(uint8_t bus_address)
{
	ESP_LOGI(LOG_TAG, "Starting NX bus communicator");
	if (!pong_len) {
		ESP_LOGE(LOG_TAG, "Pong message should be set prior to communication start");
		return;
	}
	own_address = bus_address;
	nxserial.start();
}

void NXBus::stop()
{
	nxserial.stop();
	ESP_LOGI(LOG_TAG, "NX bus communicator stopped");
}

void NXBus::setPongMessage(NXMessage msg)
{
	pong_len = makePacket(&msg, pong_buf);
}