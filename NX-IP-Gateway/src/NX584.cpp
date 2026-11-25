#include "NX584.h"
#include "Config.h"
#include "NXPanel.h"
#include "NXMessage.h"
#include "Utils.h"
#include "NXPanelFunctions.h"

#define LOG_TAG "NX584"

extern NXPanel panel;

size_t stuffBytes(uint8_t* buf, size_t len) {
    uint8_t stuffed[MAX_PACKET_SIZE * 2];
    stuffed[0] = buf[0];
    size_t ptr = 1;
    for (size_t i = 1; i < len; i++) {
        if (buf[i] == 0x7E) {
            stuffed[ptr++] = 0x7D;
            stuffed[ptr++] = 0x5E;
        }
        else if (buf[i] == 0x7D) {
            stuffed[ptr++] = 0x7D;
            stuffed[ptr++] = 0x5D;
        }
        else
            stuffed[ptr++] = buf[i];
    }
    memcpy(buf, stuffed, ptr);
    return ptr;
}

size_t unstuffBytes(uint8_t* buf, size_t len) {
    uint8_t unstuffed[MAX_PACKET_SIZE * 2];
    unstuffed[0] = buf[0];
    size_t ptr = 1;
    for (size_t i = 1; i < len; i++) {
        if (i < len - 1 && buf[i] == 0x7D) {
            unstuffed[ptr++] = buf[i+1] ^ 0x20;
            i++;
        }
        else
            unstuffed[ptr++] = buf[i];
    }
    memcpy(buf, unstuffed, ptr);
    return ptr;
}

void NX584::receive(uint8_t* buf, size_t len) {
    // special processing (unstuffing/decryption)
    len = preProcessReceive(buf, len);
    if (len == 0)
        return;
    len = unstuffBytes(buf, len);
    // verifying structure
    if (buf[0] != 0x7E) {
        ESP_LOGW(LOG_TAG, "Msg w/incorrect start byte received");
        sendInternal(NX584::NACK);
        return;
    }
    if (buf[1] + 4 != len) { // Msg.len + start + len + s1 + s2
        ESP_LOGW(LOG_TAG, "Msg w/incorrect length received");
        sendInternal(NX584::NACK);
        return;
    }
    uint8_t s1 = 0, s2 = 0;
    for (size_t i = 1; i < len - 2; i++) {
        fletcher(buf[i], s1, s2);
    }
    if (buf[len-2] != s1 || buf[len-1] != s2) {
        ESP_LOGW(LOG_TAG, "Msg w/incorrect checksum received");
        sendInternal(NX584::NACK);
        return;
    }
    //logging
    char logbuf[MAX_PACKET_SIZE * 3 + 10];
    bytesToHexString(logbuf, buf, len);
    ESP_LOGI(LOG_TAG, "Message received: %s", logbuf);
    // processing
    NX584Message msg;
    memcpy(msg.data.bytes, buf + 2, len - 4);
    processNX584Request(&msg);
}

void NX584::send(NX584Message* msg)
{
    // formatting message
    uint8_t buf[MAX_PACKET_SIZE * 2];
    size_t len = msg->msgLen();
    if (len == 0)
        return;
    buf[0] = 0x7E;
    buf[1] = len;
    size_t ptr = 2;
    uint8_t s1 = 0, s2 = 0;
    fletcher(len, s1, s2);
    for (size_t i = 0; i < len; i++) {
        buf[ptr++] = msg->data.bytes[i];
        fletcher(msg->data.bytes[i], s1, s2);
    }
    buf[len + 2] = s1;
    buf[len + 3] = s2;
    len = len + 4;
    // logging
    char logbuf[MAX_PACKET_SIZE * 3 + 10];
    bytesToHexString(logbuf, buf, len);
    ESP_LOGI(LOG_TAG, "Message sent: %s", logbuf);
    // special processing (stuffing/encryption)
    len = stuffBytes(buf, len);
    len = postProcessSend(buf, len);
    if (len == 0)
        return;
    // sending
    client.write(buf, len);
 }

 size_t NX584::preProcessReceive(uint8_t *buf, size_t len)
 {
    return len;
 }

 size_t NX584::postProcessSend(uint8_t *buf, size_t len)
 {
    return len;
 }

 void NX584::onClientDisconnection()
 {
 }

void NX584::processCommunication(void* arg)
{
    NX584* self = static_cast<NX584*>(arg);
    uint8_t buf[MAX_PACKET_SIZE * 2];
    size_t len = 0;
    for (;;) {
        self->client = self->tcpServer.accept();
        if (self->client) {
                while (self->client.connected()) {
                if (self->client.available()) {
                    len = self->client.read(buf, sizeof(buf));
                    if (len <= 0)
                        continue;
                    self->receive(buf, len);
                }
            vTaskDelay(100 / portTICK_RATE_MS);
            }
            self->onClientDisconnection();
            self->client.stop();
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

    

void NX584::processNX584Request(NX584Message* msg)
{
    NX584Message res;
    res.data.msg01.msgnum = 0;
    switch (msg->msgNum()) {
    case 0x21:
        res.data.msg01.msgnum = 0x01;
        memcpy(res.data.msg01.firmware, "IPGW", 4);
        res.data.bytes[5] = 0b11100010;
        res.data.bytes[6] = 0b00000101;
        res.data.bytes[7] = 0b11100010;
        res.data.bytes[8] = 0b00000101;
        res.data.bytes[9] = 0b11111111;
        res.data.bytes[10] = 0b11111100;
        break;
    case 0x23:
        // Not realized yet
        sendInternal(NX584::REJECT);
        return;
        break;
    case 0x24:
        // Not realized yet
        sendInternal(NX584::REJECT);
        return;
        break;
    case 0x25: {
        uint8_t offset = msg->data.msg25.zoneOffset;
        if (offset + 16 >= MAX_ZONE) {
            sendInternal(NX584::REJECT);
            return;
        }
        res.data.msg05.msgnum = 0x05;
        res.data.msg05.zoneOffset = offset;
        for (size_t i = offset; i < offset; i += 2) {
            res.data.msg05.zoneStatus[i].faultedA = panel.state.zones[i].fault.get();
            res.data.msg05.zoneStatus[i].bypassedA = panel.state.zones[i].bypass.get() || panel.state.zones[i].inhibit.get();
            res.data.msg05.zoneStatus[i].troubleA = panel.state.zones[i].trouble.get() || panel.state.zones[i].tamper.get() ||
                panel.state.zones[i].lowBattery.get() || panel.state.zones[i].lost.get();
            res.data.msg05.zoneStatus[i].alarmMemoryA = panel.state.zones[i].alarmMemory.get();
            res.data.msg05.zoneStatus[i].faultedB = panel.state.zones[i + 1].fault.get();
            res.data.msg05.zoneStatus[i].bypassedB = panel.state.zones[i + 1].bypass.get() || panel.state.zones[i + 1].inhibit.get();
            res.data.msg05.zoneStatus[i].troubleB = panel.state.zones[i + 1].trouble.get() || panel.state.zones[i + 1].tamper.get() ||
                panel.state.zones[i + 1].lowBattery.get() || panel.state.zones[i + 1].lost.get();
            res.data.msg05.zoneStatus[i].alarmMemoryB = panel.state.zones[i + 1].alarmMemory.get();
        }
        break;
    }
    case 0x26: {
        uint8_t p = msg->data.msg26.partition;
        res.data.msg06.msgnum = 0x06;
        res.data.msg06.partition = p;
        res.data.msg06.flags1.bypassCodeRequired = panel.state.partitions[p].bypassCodeRequired.get();
        res.data.msg06.flags1.fireTrouble        = panel.state.partitions[p].fireTrouble.get();
        res.data.msg06.flags1.fire               = panel.state.partitions[p].fire.get();
        res.data.msg06.flags1.pulsingBuzzer      = panel.state.partitions[p].pulsingBuzzer.get();
        res.data.msg06.flags1.tlmFaultMemory     = panel.state.partitions[p].tlmFaultMemory.get();
        res.data.msg06.flags1.armed              = panel.state.partitions[p].armed.get();
        res.data.msg06.flags1.instant            = panel.state.partitions[p].instant.get();
        res.data.msg06.flags2.previousAlarm      = panel.state.partitions[p].previousAlarm.get();
        res.data.msg06.flags2.sirenOn            = panel.state.partitions[p].sirenOn.get();
        res.data.msg06.flags2.steadySirenOn      = panel.state.partitions[p].steadySirenOn.get();
        res.data.msg06.flags2.alarmMemory        = panel.state.partitions[p].alarmMemory.get();
        res.data.msg06.flags2.tamper             = panel.state.partitions[p].tamper.get();
        res.data.msg06.flags2.cancelCmdEntered   = panel.state.partitions[p].cancelCmdEntered.get();
        res.data.msg06.flags2.codeEntered        = panel.state.partitions[p].codeEntered.get();
        res.data.msg06.flags2.cancelPending      = panel.state.partitions[p].cancelPending.get();
        res.data.msg06.flags3.silentExitEnabled  = panel.state.partitions[p].silentExitEnabled.get();
        res.data.msg06.flags3.entryGuard         = panel.state.partitions[p].entryGuard.get();
        res.data.msg06.flags3.chimeModeOn        = panel.state.partitions[p].chimeModeOn.get();
        res.data.msg06.flags3.entry              = panel.state.partitions[p].entry.get();
        res.data.msg06.flags3.delayExpiration    = panel.state.partitions[p].delayExpiration.get();
        res.data.msg06.flags3.exit1              = panel.state.partitions[p].exit1.get();
        res.data.msg06.flags3.exit2              = panel.state.partitions[p].exit2.get();
        res.data.msg06.flags4.ledExtinguish      = panel.state.partitions[p].ledExtinguish.get();
        res.data.msg06.flags4.crossTiming        = panel.state.partitions[p].crossTiming.get();
        res.data.msg06.flags4.recentClosing      = panel.state.partitions[p].recentClosing.get();
        res.data.msg06.flags4.exitErrorTriggered = panel.state.partitions[p].exitErrorTriggered.get();
        res.data.msg06.flags4.autoHomeInhibited  = panel.state.partitions[p].autoHomeInhibited.get();
        res.data.msg06.flags4.sensorLowBattery   = panel.state.partitions[p].sensorLowBattery.get();
        res.data.msg06.flags4.sensorLostSup      = panel.state.partitions[p].sensorLostSup.get();
        res.data.msg06.flags5.zoneBypassed       = panel.state.partitions[p].zoneBypassed.get();
        res.data.msg06.flags5.forceArmAuto       = panel.state.partitions[p].forceArmAuto.get();
        res.data.msg06.flags5.readyToArm         = panel.state.partitions[p].readyToArm.get();
        res.data.msg06.flags5.readyToForceArm    = panel.state.partitions[p].readyToForceArm.get();
        res.data.msg06.flags5.validPinAccepted   = panel.state.partitions[p].validPinAccepted.get();
        res.data.msg06.flags5.chimeOn            = panel.state.partitions[p].chimeOn.get();
        res.data.msg06.flags5.errorBeep          = panel.state.partitions[p].errorBeep.get();
        res.data.msg06.flags5.toneOn             = panel.state.partitions[p].toneOn.get();
        res.data.msg06.flags6.entry1             = panel.state.partitions[p].entry1.get();
        res.data.msg06.flags6.openPeriod         = panel.state.partitions[p].openPeriod.get();
        res.data.msg06.flags6.alarmPhone1        = panel.state.partitions[p].alarmPhone1.get();
        res.data.msg06.flags6.alarmPhone2        = panel.state.partitions[p].alarmPhone2.get();
        res.data.msg06.flags6.alarmPhone3        = panel.state.partitions[p].alarmPhone3.get();
        res.data.msg06.flags6.cancelReportStack  = panel.state.partitions[p].cancelReportStack.get();
        res.data.msg06.flags6.keyswitchArmed     = panel.state.partitions[p].keyswitchArmed.get();
        res.data.msg06.flags6.delayTrip          = panel.state.partitions[p].delayTrip.get();
        res.data.msg06.lastUserNum               = panel.state.partitions[p].lastUserNum.get();
        break;
     }
    case 0x27: {
        res.data.msg07.msgnum = 0x07;
        for (size_t p = 0; p < 8; p++) {
            res.data.msg07.part[p].validPartition   = panel.state.partitions[p].validPart.get();
            res.data.msg07.part[p].ready            = panel.state.partitions[p].readyToArm.get();
            res.data.msg07.part[p].armed            = panel.state.partitions[p].armed.get();
            res.data.msg07.part[p].stayMode         = panel.state.partitions[p].entryGuard.get();
            res.data.msg07.part[p].chimeMode        = panel.state.partitions[p].chimeModeOn.get();
            res.data.msg07.part[p].anyEntryDelay    = panel.state.partitions[p].entry.get() || panel.state.partitions[p].entry1.get();
            res.data.msg07.part[p].anyExitDelay     = panel.state.partitions[p].exit1.get() || panel.state.partitions[p].exit2.get();
            res.data.msg07.part[p].previousAlarm    = panel.state.partitions[p].previousAlarm.get();
        }
        break;
    }
    case 0x28:{
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg24.msgnum = 0xA4;
        m.data.msg24.sourceAddr = panel.ownBusAddr;
        m.data.msg24.panelID = 0;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x19) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }              
        res.data.msg08.ss = l.data.msg19;
        res.data.msg08.ss.msgnum = 0x08;
        res.data.msg08.ss.panelId = panel.state.panelID;
        res.data.msg08.commStackPointer = panel.state.commStackPointer;
        break;
    }
    case 0x29:
        // Not realized yet
        sendInternal(NX584::REJECT);
        return;
        break;
    case 0x2A: {
        uint8_t id = msg->data.msg2A.eventNum;
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg21.msgnum = 0xA1;
        m.data.msg21.sourceAddr = panel.ownBusAddr;
        m.data.msg21.itemNumber = id;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x61) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        res.data.msg0A.msgnum = 0x0A;
        res.data.msg0A.logNumber        = l.data.msg61.logNumber;
        res.data.msg0A.logSize          = l.data.msg61.logSize;
        res.data.msg0A.event            = l.data.msg61.event;
        res.data.msg0A.reportingevent   = l.data.msg61.reportingevent;
        res.data.msg0A.itemNumber       = l.data.msg61.itemNumber;
        res.data.msg0A.partition        = l.data.msg61.partition();
        res.data.msg0A.month            = l.data.msg61.month();
        res.data.msg0A.day              = l.data.msg61.day;
        res.data.msg0A.hour             = l.data.msg61.hour;
        res.data.msg0A.minute           = l.data.msg61.minute;
        break;
        }
    case 0x2B:
        // Not realized yet
        sendInternal(NX584::REJECT);
        return;
        break;
    case 0x2C:
        // Not realized yet
        sendInternal(NX584::REJECT);
        return;
        break;
    case 0x30: {
        uint16_t loc = msg->data.msg30.getLocation();
        if (msg->data.msg30.moduleAddr == panel.ownBusAddr) {
            ownLocationRead(loc);
            break;
        }
        ESP_LOGI(LOG_TAG, "Reading module %d %s location %d / %#X", 
            msg->data.msg30.moduleAddr, 
            msg->data.msg30.locationInfo.segmentSizeOrDataBank == flash ? "FLASH" : "RAM",
            loc, 
            loc);
        NXMessage m, l;
        m.dest = msg->data.msg30.moduleAddr;
        m.data.msg20 = msg->data.msg30;
        m.data.msg20.msgnum = 0xA0;
        m.data.msg20.moduleAddr = panel.ownBusAddr;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x60) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        res.data.msg10 = l.data.msg60;
        res.data.msg10.msgnum = 0x10;
        break;
    }
    case 0x31: {
        NXMessage m, l;
        m.dest = msg->data.msg31.moduleAddr;
        m.data.msg60 = msg->data.msg31;
        m.data.msg60.msgnum = 0xE0;
        m.data.msg60.moduleAddr = panel.ownBusAddr;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x40) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    case 0x32:
        if (!panel.fun.checkPin(msg->data.msg32.masterPin, 255, authMaster)) {
            sendInternal(NX584::FAIL);
            return;
        }
        [[fallthrough]];
    case 0x33: {
        uint8_t user = msg->msgNum() == 0x32 ? msg->data.msg32.user : msg->data.msg33.user;
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg55.msgnum = 0xD5;
        m.data.msg55.sourceAddr = panel.ownBusAddr;
        m.data.msg55.userNumber = user;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x70) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        res.data.msg12 = l.data.msg70;
        res.data.msg12.msgnum = 0x12;
        break;
    }
    case 0x34:
        if (!panel.fun.checkPin(msg->data.msg34.masterPin, 255, authMaster)) {
            sendInternal(NX584::FAIL);
            return;
        }
        [[fallthrough]];
    case 0x35: {
        uint8_t user = msg->msgNum() == 0x34 ? msg->data.msg34.user : msg->data.msg35.user;
        pin_t newPin = msg->msgNum() == 0x34 ? msg->data.msg34.newPin : msg->data.msg35.newPin;
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg53.msgnum = 0xD3;
        m.data.msg53.sourceAddr = panel.ownBusAddr;
        m.data.msg53.userNumber = user;
        m.data.msg53.pin = newPin;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x40) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    case 0x36:
        if (!panel.fun.checkPin(msg->data.msg36.masterPin, 255, authMaster)) {
            sendInternal(NX584::FAIL);
            return;
        }
        [[fallthrough]];
    case 0x37: {
        userAuth_t auth = msg->msgNum() == 0x36 ? msg->data.msg36.auth : msg->data.msg37.auth;
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg54.msgnum = 0xD4;
        m.data.msg54.sourceAddr = panel.ownBusAddr;
        m.data.msg54.auth = auth;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x40) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    case 0x3A: {
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg5C.msgnum = 0x5C;
        m.data.msg5C.sourceAddr = panel.ownBusAddr;
        m.data.msg5C.partition = msg->data.msg3A.partition;
        m.data.msg5C.reportingevent = msg->data.msg3A.reportingevent;
        m.data.msg5C.event = msg->data.msg3A.event;
        m.data.msg5C.itemNumber = msg->data.msg3A.itemNumber;
        m.data.msg5C.data1 = msg->data.msg3A.data1;
        m.data.msg5C.data2 = msg->data.msg3A.data2;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x40) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    case 0x3B: {
        dateTime dt;
        dt.year = msg->data.msg3B.year;
        dt.month = msg->data.msg3B.month;
        dt.day = msg->data.msg3B.day;
        dt.hour = msg->data.msg3B.hour;
        dt.minute = msg->data.msg3B.minute;
        dt.weekday = msg->data.msg3B.weekDay;
        requestResult r = panel.fun.nxSetDateTime(dt);
        if (r != rqOK) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    case 0x3C:
        if (!panel.fun.checkPin(msg->data.msg36.masterPin, 255, authMaster)) {
            sendInternal(NX584::FAIL);
            return;
        }
       [[fallthrough]];
    case 0x3D: {
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg51.msgnum = 0xD1;
        m.data.msg51.sourceAddr = panel.ownBusAddr;
        m.data.msg51.partitionMask = msg->data.msg3E.partitionMask;
        switch (msg->data.msg3D.function)
        {
            case primaryKpadFunc::sounderOff:       m.data.msg51.function = kpd_function::disarm; break;
            case primaryKpadFunc::disarm:           m.data.msg51.function = kpd_function::disarm; break;
            case primaryKpadFunc::armAway:          m.data.msg51.function = kpd_function::armAway; break;
            case primaryKpadFunc::armStay:          m.data.msg51.function = kpd_function::armStay; break;
            case primaryKpadFunc::cancel:           m.data.msg51.function = kpd_function::cancel; break;
            case primaryKpadFunc::autoArm:          m.data.msg51.function = kpd_function::autoArm; break;
            case primaryKpadFunc::walkTestStart:    m.data.msg51.function = kpd_function::walkTest; break;
            case primaryKpadFunc::walkTestStop:     m.data.msg51.function = kpd_function::walkTest; break;   
            default:
                sendInternal(NX584::REJECT);
                return;
        }
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x40) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    case 0x3E: {
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg51.msgnum = 0xD1;
        m.data.msg51.sourceAddr = panel.ownBusAddr;
        m.data.msg51.partitionMask = msg->data.msg3E.partitionMask;
        switch (msg->data.msg3E.function)
        {
            case secondKpadFunc::Stay:                    m.data.msg51.function = kpd_function::stay; break;
            case secondKpadFunc::Chime:                   m.data.msg51.function = kpd_function::chime; break;
            case secondKpadFunc::Exit:                    m.data.msg51.function = kpd_function::exit; break;
            case secondKpadFunc::BypassInteriors:         m.data.msg51.function = kpd_function::bypassInteriors; break;
            case secondKpadFunc::FirePanic:               m.data.msg51.function = kpd_function::firePanic; break;
            case secondKpadFunc::MedicalPanic:            m.data.msg51.function = kpd_function::medicalPanic; break;
            case secondKpadFunc::PolicePanic:             m.data.msg51.function = kpd_function::policePanic; break;
            case secondKpadFunc::SmokeDetectorReset:      m.data.msg51.function = kpd_function::smokeDetectorReset; break;
            case secondKpadFunc::AutoCallbackDownload:    m.data.msg51.function = kpd_function::callback; break;
            case secondKpadFunc::ManualPickupDownload:    m.data.msg51.function = kpd_function::download; break;
            case secondKpadFunc::SilentExitEnable:        m.data.msg51.function = kpd_function::silentExit; break;
            case secondKpadFunc::PerformTest:             m.data.msg51.function = kpd_function::test; break;
            case secondKpadFunc::GroupBypass:             m.data.msg51.function = kpd_function::groupBypass; break;
            case secondKpadFunc::AuxiliaryFunction1:      m.data.msg51.function = kpd_function::auxillary1; break;
            case secondKpadFunc::AuxiliaryFunction2:      m.data.msg51.function = kpd_function::auxillary2; break;
            case secondKpadFunc::StartKeypadSounder:      m.data.msg51.function = kpd_function::buzzerOn; break;
            default:
                sendInternal(NX584::REJECT);
                return;
        }
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x40) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    case 0x3F: {
        NXMessage m, l;
        m.dest = PANEL;
        m.data.msg52.msgnum = 0xD2;
        m.data.msg52.sourceAddr = panel.ownBusAddr;
        m.data.msg52.zone = msg->data.msg3F.zone;
        requestResult r = panel.makeRequest(&m, &l);
        if (r != rqOK || l.msgNum() != 0x40) {
            ESP_LOGW(LOG_TAG, "Request failed (%d)", r);
            sendInternal(NX584::FAIL);
            return;
        }
        sendInternal(NX584::ACK);
        break;
    }
    default:
        sendInternal(NX584::REJECT);
        return;
    }
    send(&res);
}

void NX584::ownLocationRead(uint16_t loc)
{
    // Read by DL900, purpose is unknown, we would send all 0s
    NX584Message res;
    if (loc == 0xFFF) {
        res.data.msg10.msgnum = 0x10;
        res.data.msg10.moduleAddr = panel.ownBusAddr;
        res.data.msg10.setLocation(loc);
        res.data.msg10.locationInfo.locationType = t_locType::physycal;
        res.data.msg10.locationInfo.segmentSizeOrDataBank = t_segmentSizeOrDataBank::flash;
        res.data.msg10.locationInfo.readWrite = t_readWrite::rw_read;
        res.data.msg10.locationInfo.segmentOffset = t_segmentOffset::none;
        res.data.msg10.dataInfo.numSegments = 4;
        memset(res.data.msg10.data, 0, 8);
        send (&res);
    } else
        sendInternal(NX584::REJECT);

}

void NX584::start()
{
    if (gwCommunicationTask != NULL) {
        ESP_LOGW(LOG_TAG, "NX584 emul already running");
        return;
    }
    ESP_LOGI(LOG_TAG, "Starting NX584 emul");
    tcpServer.begin();
    xTaskCreate(processCommunication, "NX584", 4096, this, tskIDLE_PRIORITY + 1, &gwCommunicationTask);
}

void NX584::stop()
{
    if (gwCommunicationTask == NULL) {
        ESP_LOGW(LOG_TAG, "NX584 emul is not running");
        return;
    }
    vTaskDelete(gwCommunicationTask);
    gwCommunicationTask = NULL;
    tcpServer.stop();
    ESP_LOGI(LOG_TAG, "NX584 emul stopped");
}

void NX584::sendInternal(nx584InternalResponse response)
{
    NX584Message msg;
    switch (response)
    {
    case NX584::FAIL:
        msg.data.msg1C.msgnum = 0x1C;
        break;
    case NX584::ACK:
        msg.data.msg1D.msgnum = 0x1D;
        break;
    case NX584::NACK:
        msg.data.msg1E.msgnum = 0x1E;
        break;
    case NX584::REJECT:
        msg.data.msg1F.msgnum = 0x1F;
        break;
    default:
        return;
    }
    send(&msg);
}


NX584 nx584;

