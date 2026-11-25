#include "DL900.h"
#include "Utils.h"

#define LOG_TAG "DL900"

const char* masterKeys[] = { "UZ\252-02INTERLOGIX-DEFAULT", "U\252Caddx Controls DL900Z\245" };

void DL900::decodeKey(uint8_t *encodedKey, uint8_t masterKeySelector)
{
    uint8_t key[24];
    // initialize linear feedback shift register (LFSR) with master key
    uint8_t lfsr[24];
    for (int i = 0; i < 24; i++) {
        lfsr[i] = masterKeys[masterKeySelector][23 - i];
    }

    for (int i = 0; i < 24; i++) {
        // decode key byte
        key[i] = lfsr[23] ^ encodedKey[i];
        // rotate LFSR to 9 bits right
        uint16_t right_bits = ((static_cast<uint16_t>(lfsr[22]) & 0x01) << 8) | lfsr[23];
        for (int i = 23; i > 0; i--) {
            uint8_t high_part = lfsr[i - 1];
            uint8_t low_part = (i >= 2) ? lfsr[i - 2] : 0;
            lfsr[i] = (high_part >> 1) | (low_part << 7);
        }
        lfsr[0] = 0;
        lfsr[0] |= (right_bits >> 1) & 0xFF;
        lfsr[1] |= (right_bits & 0x01) << 7;
    }

    three_des_key_setup(key, encSchedule, DES_ENCRYPT);
    three_des_key_setup(key, decSchedule, DES_DECRYPT);

    char logbuf[24 * 3 + 10];
    bytesToHexString(logbuf, key, 24);
    ESP_LOGI(LOG_TAG, "Received encryption key : %s", logbuf);
    hasKey = true;   
}

void DL900::encrypt(uint8_t *buf, size_t len)
{
    for (size_t b = 0; b < len / 8; b++) {
        three_des_crypt(&buf[b * 8], &buf[b * 8], encSchedule);
    }
}

void DL900::decrypt(uint8_t *buf, size_t len)
{
    for (size_t b = 0; b < len / 8; b++) {
        three_des_crypt(&buf[b * 8], &buf[b * 8], decSchedule);
    }
}

size_t DL900::preProcessReceive(uint8_t *buf, size_t len)
{
    if (buf[0] == 0x7E && len == 25) {
        decodeKey(buf + 1);        
        return 0;
    }
    else if (hasKey)
        decrypt(buf, len);
    return len;
}

size_t DL900::postProcessSend(uint8_t *buf, size_t len)
{
    if (hasKey)
        encrypt(buf, len);
    return len;
}

void DL900::onClientDisconnection()
{
    hasKey = false;
}

DL900 dl900;