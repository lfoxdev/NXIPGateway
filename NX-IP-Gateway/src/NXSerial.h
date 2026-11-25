// NXSerial.h
// NX bus L1 layer
// Software half-duplex UART realization

#ifndef _NXSERIAL_h
#define _NXSERIAL_h

#include "Config.h"
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"

#define BAUD_RATE       3900
#define TIMER_GROUP     TIMER_GROUP_0
#define TIMER           TIMER_0
#define BITS_IN_BYTE    9

#define BIT_TIME            (1000000 / BAUD_RATE)
#define LVL_TO_BIT_RX(b)    (INVERSE_RX ? !b : b)
#define LVL_TO_BIT_TX(b)    (INVERSE_TX ? !b : b)

// Forward declarations
class NXBus;

class NXSerial
{
friend class NXBus;
protected:
    NXBus* bus;
private:
    bool hwInitialized = false;

    enum Mode { SU_WAIT, SU_RX, SU_TX };

    // Internal state
    uint16_t rx_buf[MAX_PACKET_SIZE];
    uint16_t tx_buf[MAX_PACKET_SIZE];
    volatile uint8_t rx_len, tx_len, tx_ptr;
    volatile uint16_t rx_byte, tx_byte;
    volatile int8_t rx_state, tx_state;
    volatile Mode mode;

    // Helpers
    void IRAM_ATTR onBusIdleDetection(BaseType_t* pxHigherPriorityTaskWoken);
    void IRAM_ATTR onByteReceived(BaseType_t* pxHigherPriorityTaskWoken);

    // ISRs
    static void IRAM_ATTR gpioIsrHandler(void* arg);
    static bool IRAM_ATTR timerIsrHandler(void* arg);

public:
    explicit NXSerial(NXBus* b) : bus(b) {}
	void start();
	void stop();
    void IRAM_ATTR resetRxBuf() { rx_len = 0; };
    void IRAM_ATTR send(uint16_t* _tx_buf, uint8_t _tx_len) {
        if (_tx_len > MAX_PACKET_SIZE)
            return;
        memcpy((void*)tx_buf, _tx_buf,  _tx_len * sizeof(_tx_buf[0]));
        tx_len = _tx_len;
    }
};

#endif

