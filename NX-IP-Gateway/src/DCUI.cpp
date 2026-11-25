// 
// 
// 

#include "Config.h"
#include "DCUI.h"
#include "NXPanel.h"
#include "NXMessage.h"
#include "Utils.h"

#define LOG_TAG "DCUI"

extern NXPanel panel;

void onTelnetInput(String line) {
    // Simple cleanup for telnet line
    String clean = "";
    clean.reserve(line.length());
    for (size_t i = 0; i < line.length(); i++) {
        uint8_t ch = (uint8_t)line[i];
        // Apply backspace & delete keys
        if (ch == 0x08 || ch == 0x7F) {
            if (clean.length() > 0)
                clean.remove(clean.length() - 1);
            continue;
        }
        // Ignore ANSI sequences
        if (ch == 0x1B) {
            if (i + 2 < line.length() && line[i + 1] == '[') {
            i += 2;
            while (line[i] != '\0' && (uint8_t)line[i] < 0x40) 
                i++;
            continue;
            }
        }
        clean += (char)ch;
    }
    NXMessage request;
    uint8_t databuf[MAX_PACKET_SIZE];
    int n = hexStringToBytes(databuf, clean.c_str(), clean.length());
    if (n > 1) {
        request.dest = databuf[0];
        memcpy(request.data.bytes, databuf + 1, n - 1);
        request.length = n - 1;
        NXMessage response;
        requestResult r = panel.makeRequest(&request, &response);
        if (r == rqOK) {
            char tx_buf[128];
            tx_buf[0] = '\t';
            tx_buf[1] = '\t';
            bytesToHexString(tx_buf + 2, response.data.bytes, response.msgLen());
            dcui.telnet.println(tx_buf);
        }
        else if (r == rqBADRQ) {
            dcui.telnet.println("\t\tErr: bad request");
        }
        else if (r == rqTOUT) {
            dcui.telnet.println("\t\tErr: panel timeout");
        }
        else if (r == rqBUSY) {
            dcui.telnet.println("\t\tErr: bus is busy");
        }
    }
    else {
        dcui.telnet.println("\t\tErr: malformed string");
    }
}

void onTelnetConnect(String ip) {
    dcui.telnet.println();
    dcui.telnet.println(F("Welcome to NX IP Gateway DCUI"));
    dcui.telnet.println(F("Type a packet as hex sequence, hit enter and it will be sent to NX bus \"as is\". Spaces are ignored."));
    dcui.telnet.print(F("If you want to see replies, use "));
    dcui.telnet.print(String(panel.ownBusAddr, HEX));
    dcui.telnet.println(F(" as source address."));
    dcui.telnet.println(F("================================"));
}

void DCUI::processCommunication(void* arg)
{
    DCUI* self = static_cast<DCUI*>(arg);
    for (;;) {
        self->telnet.loop();
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void DCUI::start()
{
    if (gwCommunicationTask != NULL) {
        ESP_LOGW(LOG_TAG, "DCUI already running");
        return;
    }
    ESP_LOGI(LOG_TAG, "Starting DCUI");
    telnet.onConnect(onTelnetConnect);
    telnet.onInputReceived(onTelnetInput);
    telnet.begin(DIRECT_UI_PORT, false);
    xTaskCreate(processCommunication, "directCommunication", 4096, this, tskIDLE_PRIORITY + 1, &gwCommunicationTask);
}

void DCUI::stop()
{
    if (gwCommunicationTask == NULL) {
        ESP_LOGW(LOG_TAG, "DCUI is not running");
        return;
    }
    vTaskDelete(gwCommunicationTask);
    gwCommunicationTask = NULL;
    telnet.stop();
    ESP_LOGI(LOG_TAG, "DCUI stopped");
}

DCUI dcui;

