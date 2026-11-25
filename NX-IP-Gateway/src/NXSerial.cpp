#include "NXSerial.h"
#include "NXBus.h"

#include "driver/gpio.h"
#include "driver/timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// Helpers
inline void NXSerial::onBusIdleDetection(BaseType_t* pxHigherPriorityTaskWoken) {
    if (rx_len != 0)
        bus->onBusIdleDetection((uint16_t*)rx_buf, rx_len, pxHigherPriorityTaskWoken);
}

inline void NXSerial::onByteReceived(BaseType_t* pxHigherPriorityTaskWoken) {
    bus->onByteReceived((uint16_t*)rx_buf, rx_len, pxHigherPriorityTaskWoken);
}

void NXSerial::gpioIsrHandler(void* arg)
{
#ifdef DBG_PIN
    gpio_set_level(DBG_PIN, 1);
#endif
    auto* self = static_cast<NXSerial*>(arg);
    if (self->mode == SU_RX || self->mode == SU_WAIT) {
        self->mode = SU_RX;
        // Disable edge detection, start timer
        gpio_intr_disable(RX_PIN);
        timer_pause(TIMER_GROUP, TIMER);
        timer_group_set_alarm_value_in_isr(TIMER_GROUP, TIMER, BIT_TIME / 2);
        timer_set_counter_value(TIMER_GROUP, TIMER, 0);
        timer_start(TIMER_GROUP, TIMER);
        self->rx_byte = 0;
        self->rx_state = -1;
    }
#ifdef DBG_PIN
    gpio_set_level(DBG_PIN, 0);
#endif
}

bool NXSerial::timerIsrHandler(void* arg)
{
#ifdef DBG_PIN
    gpio_set_level(DBG_PIN, 1);
#endif
    auto* self = static_cast<NXSerial*>(arg);
    timer_group_clr_intr_status_in_isr(TIMER_GROUP, TIMER);
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;;

    if (self->mode == SU_RX) {
        // Receive
        uint8_t bit = LVL_TO_BIT_RX(gpio_get_level(RX_PIN));
        switch (++self->rx_state) {
        case 0:
            // Start bit
            timer_group_set_alarm_value_in_isr(TIMER_GROUP, TIMER, BIT_TIME);
            if (bit != 0) {
                // No start bit - stop timer, enable edge detection
                timer_pause(TIMER_GROUP, TIMER);
                gpio_intr_enable(RX_PIN);
            }
            break;
        case (BITS_IN_BYTE + 1):
            // Stop bit
            if (bit == 1 && self->rx_len < MAX_PACKET_SIZE) {
                // Byte is correct - process
                self->rx_buf[self->rx_len++] = self->rx_byte;
                self->onByteReceived(&pxHigherPriorityTaskWoken);
            }
            self->mode = SU_WAIT;
            gpio_intr_enable(RX_PIN);
            break;
        default:
            // Data bits
            self->rx_byte >>= 1;
            if (bit == 1)
                self->rx_byte |= (1 << (BITS_IN_BYTE - 1));
            break;
        }
    }
    else if (self->mode == SU_TX) {
        // Transmit
        switch (++self->tx_state) {
        /*case 0:
            // Start bit
            gpio_set_level(TX_PIN, LVL_TO_BIT_TX(0));
            break;*/
        case (BITS_IN_BYTE + 1):
            // Stop bit
            gpio_set_level(TX_PIN, LVL_TO_BIT_TX(1));
            if (++self->tx_ptr < self->tx_len) {
                // Next byte
                self->tx_byte = self->tx_buf[self->tx_ptr];
                self->tx_state = -1;
            }
            else {
                // Start receiving - stop timer, enable edge detection
                self->tx_len = 0;
                self->mode = SU_RX;
                gpio_intr_enable(RX_PIN);
                timer_pause(TIMER_GROUP, TIMER);
            }
            break;
        default:
            // Data bits
            uint8_t bit = (self->tx_byte >> (self->tx_state - 1)) & 1;
            gpio_set_level(TX_PIN, LVL_TO_BIT_TX(bit));
            break;
        }
    }
    else { //self->mode == SU_WAIT
        // Bus idle detected
        self->onBusIdleDetection(&pxHigherPriorityTaskWoken);
        if (self->tx_len > 0 && LVL_TO_BIT_RX(gpio_get_level(RX_PIN)) != 0) {
            // Start transmitting if have anything and bus is still free
            self->mode = SU_TX;
            gpio_intr_disable(RX_PIN);
            self->tx_ptr = 0;
            self->tx_byte = self->tx_buf[0];
            // Set start bit
            gpio_set_level(TX_PIN, LVL_TO_BIT_TX(0));
            self->tx_state = 0;
        }
        else {
            // Waiting for edge detection
            timer_pause(TIMER_GROUP, TIMER);
        }
    }
#ifdef DBG_PIN
    gpio_set_level(DBG_PIN, 0);
#endif
    return (pxHigherPriorityTaskWoken == pdTRUE);
}

void NXSerial::start()
{
    if (!hwInitialized) {
        // RX GPIO configuration
        ESP_ERROR_CHECK(gpio_reset_pin(RX_PIN));
        ESP_ERROR_CHECK(gpio_set_direction(RX_PIN, GPIO_MODE_INPUT));
        ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3));
        ESP_ERROR_CHECK(gpio_set_intr_type(RX_PIN, (INVERSE_RX ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE)));
        ESP_ERROR_CHECK(gpio_isr_handler_add(RX_PIN, gpioIsrHandler, this));
        // TX GPIO configuration
        ESP_ERROR_CHECK(gpio_reset_pin(TX_PIN));
        ESP_ERROR_CHECK(gpio_set_direction(TX_PIN, GPIO_MODE_OUTPUT));
        gpio_set_level(TX_PIN, LVL_TO_BIT_TX(1));
        // Hardware timer configuration
        timer_config_t config = {
            .alarm_en = TIMER_ALARM_EN,
            .counter_en = TIMER_PAUSE,
            .counter_dir = TIMER_COUNT_UP,
            .auto_reload = TIMER_AUTORELOAD_EN,
            .divider = 80
        };
        ESP_ERROR_CHECK(timer_init(TIMER_GROUP, TIMER, &config));
        ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP, TIMER, 0));
        ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP, TIMER));
        ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP, TIMER, timerIsrHandler, this, 0));
#ifdef DBG_PIN
        ESP_ERROR_CHECK(gpio_set_direction(DBG_PIN, GPIO_MODE_OUTPUT));
        gpio_set_level(DBG_PIN, 0);
#endif
    }
    ESP_ERROR_CHECK(gpio_intr_enable(RX_PIN));
    mode = SU_RX;
 }

void NXSerial::stop()
{
    gpio_set_level(TX_PIN, LVL_TO_BIT_TX(1));
    gpio_intr_disable(RX_PIN);
    timer_pause(TIMER_GROUP, TIMER);
#ifdef DBG_PIN
    gpio_set_level(DBG_PIN, 0);
#endif
}
