// Utils.h
// Some common functions

#pragma once

#include <cstdint>
#include <cctype>
#include <cstddef>
#include <esp_attr.h>

inline void IRAM_ATTR fletcher(uint8_t b, uint8_t& sum1, uint8_t& sum2)
{
    if (255 - sum1 < b)
        sum1++;
    sum1 += b;
    if (sum1 == 255)
        sum1 = 0;
    if (255 - sum2 < sum1)
        sum2++;
    sum2 += sum1;
    if (sum2 == 255)
        sum2 = 0;
};

template<typename T>
size_t bytesToHexString(char* out, const T* bytes, size_t length)
{
    static const char hexDigits[] = "0123456789ABCDEF";
    size_t pos = 0;

    for (size_t i = 0; i < length; ++i) {
        uint8_t b = static_cast<uint8_t>(bytes[i] & 0xFF);

        // First nibble
        out[pos++] = hexDigits[b >> 4];
        // Second nibble
        out[pos++] = hexDigits[b & 0x0F];
        // Space
        out[pos++] = ' ';
    }

    if (pos > 0)
        out[pos - 1] = '\0';
    else
        out[0] = '\0';
        
    return pos; // number of characters written (excluding '\0')
}


inline static int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1; // invalid
};

inline int hexStringToBytes(uint8_t* bytes, const char* string, size_t length)
{
    size_t count = 0;
    int highNibble = -1;

    for (size_t i = 0; i < length; ++i) {
        char c = string[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            continue; // ignore spaces
        }

        int val = hexDigit(c);
        if (val < 0) {
            return -1; // invalid character
        }

        if (highNibble < 0) {
            highNibble = val; // store first digit
        }
        else {
            // combine two nibbles into one byte
            uint8_t byte = (highNibble << 4) | val;
            bytes[count++] = byte;
            highNibble = -1;
        }
    }

    // If we ended with a dangling high nibble, it's malformed
    if (highNibble >= 0) {
        return -1;
    }

    return count;
}

inline uint8_t sanitize(uint8_t c) {
    return isprint(c) ? c : '?';
}