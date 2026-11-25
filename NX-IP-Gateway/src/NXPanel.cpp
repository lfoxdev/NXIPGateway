#include "Config.h"
#include "NXPanel.h"
#include "NXPanelState.h"
#include <ArduinoJson.h>
#include "NXPanelFunctions.h"

#define LOG_TAG "NXG"

#define BITEX(byte, n) (((byte) >> (n)) & 0x01)

void NXPanel::handlePanelStateChange(IPanelStateItem* item)
{
    // Nothing needed here
}

requestResult NXPanel::makeRequest(const NXMessage* request, NXMessage* response)
{
    if (request == NULL) {
        ESP_LOGE(LOG_TAG, "Null request");
        return rqBADRQ;
    }
    if (xSemaphoreTake(busSemaphore, REQUEST_TOUT_MS / portTICK_RATE_MS) != pdTRUE) {
        ESP_LOGE(LOG_TAG, "Another request is pending");
        return rqBUSY;
    }
    communicatingTask = xTaskGetCurrentTaskHandle();
    requestResult result;
    for (uint8_t retry = 1; retry <= RQ_RETRIES; retry++) {
        if (xQueueSend(nxSendQueue, request, 0) != pdPASS) {
            ESP_LOGE(LOG_TAG, "Send queue is full");
            result = rqBUSY;
        }
        if (xSemaphoreTake(responseSemaphore, RESPONSE_TOUT_MS / portTICK_RATE_MS) == pdTRUE) {
            if (response)
                *response = currentResponse;
            result = rqOK;
        }
        else {
            char logbuf[MAX_PACKET_SIZE * 3 + 10];
            bytesToHexString(logbuf, request->data.bytes, request->msgLen());
            ESP_LOGW(LOG_TAG, "No answer for request %02X %s, attempt %d", request->dest, logbuf, retry);
            result = rqTOUT;
            bus.counters.send_retries++;
        }
        if (result == rqOK)
            break;
    }
    communicatingTask = NULL;
    xSemaphoreGive(busSemaphore);
    return result;
}

requestResult NXPanel::makeRequestCheckAck(const NXMessage* request)
{
    NXMessage response;
    requestResult res = makeRequest(request, &response);
    if (res != rqOK)
        return res;
    if (response.msgNum() == 0x40)
        return rqOK;
    else {
        char buf[MAX_PACKET_SIZE * 3 + 10];
        bytesToHexString(buf, response.data.bytes, response.msgLen());
        ESP_LOGW(LOG_TAG, "Received n/ack msg: %s", buf);
        return rqNACK;
    }
}

void NXPanel::processNXDebug(void* arg)
{
	NXPanel* self = static_cast<NXPanel*>(arg);
	RAWMessage_t msg;
    for (;;) {
        NetworkClient client = self->debugTcpServer.accept();
        if (client) {
            while (client.connected()) {
                if ((self->rawMsgQueue != NULL) && xQueueReceive(self->rawMsgQueue, &msg, 50 / portTICK_RATE_MS) == pdPASS) {
                    char buf[MAX_PACKET_SIZE * 3 + 10];
                    bytesToHexString(buf, msg.buf, msg.len);
                    client.write(buf);
                }
            vTaskDelay(100 / portTICK_RATE_MS);
            }
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void NXPanel::processNXMessage(void* arg) {
    NXPanel* self = static_cast<NXPanel*>(arg);
    NXMessage msg;
    for (;;) {
        if (xQueueReceive(self->nxRecvQueue, &msg, portMAX_DELAY) == pdPASS) {
            if (msg.dest <= 0x08)
                processBroadcastMessage(self, msg);
            else if (msg.dest == self->ownBusAddr)
                processSinglecastMessage(self, msg);
        }
    }
}

void NXPanel::processBroadcastMessage(NXPanel* self, NXMessage msg)
{
    switch (msg.msgNum()) {
    case 0x00: {
        self->state.lineSeizure = msg.data.msg00.flags.flags1.lineSeizure;
        self->state.offHook = msg.data.msg00.flags.flags1.offHook;
        self->state.handshakeReceived = msg.data.msg00.flags.flags1.handshakeReceived;
        self->state.downloadInProgress = msg.data.msg00.flags.flags1.downloadInProgress;
        self->state.dialerDelay = msg.data.msg00.flags.flags1.dialerDelay;
        self->state.usingBackupPhone = msg.data.msg00.flags.flags1.usingBackupPhone;
        self->state.listenInActive = msg.data.msg00.flags.flags1.listenInActive;
        self->state.twoWayLockout = msg.data.msg00.flags.flags1.twoWayLockout;
        self->state.groundFault = msg.data.msg00.flags.flags2.groundFault;
        self->state.phoneFault = msg.data.msg00.flags.flags2.phoneFault;
        self->state.failToCommunicate = msg.data.msg00.flags.flags2.failToCommunicate;
        self->state.fuseFault = msg.data.msg00.flags.flags2.fuseFault;
        self->state.boxTamper = msg.data.msg00.flags.flags2.boxTamper;
        self->state.sirenTamper = msg.data.msg00.flags.flags2.sirenTamper;
        self->state.lowBattery = msg.data.msg00.flags.flags2.lowBattery;
        self->state.acFail = msg.data.msg00.flags.flags2.acFail;
        self->state.expBoxTamper = msg.data.msg00.flags.flags3.expBoxTamper;
        self->state.expAcFailure = msg.data.msg00.flags.flags3.expAcFailure;
        self->state.expLowBattery = msg.data.msg00.flags.flags3.expLowBattery;
        self->state.expLossSup = msg.data.msg00.flags.flags3.expLossSup;
        self->state.expAuxOverCurrent = msg.data.msg00.flags.flags3.expAuxOverCurrent;
        self->state.auxCommFail = msg.data.msg00.flags.flags3.auxCommFail;
        self->state.expBellFault = msg.data.msg00.flags.flags3.expBellFault;
        self->state.pin6DigitEnabled = msg.data.msg00.flags.flags4.pin6DigitEnabled;
        self->state.tokenInUse = msg.data.msg00.flags.flags4.tokenInUse;
        self->state.pinRequiredLocal = msg.data.msg00.flags.flags4.pinRequiredLocal;
        self->state.globalBuzzer = msg.data.msg00.flags.flags4.globalBuzzer;
        self->state.globalSirenOn = msg.data.msg00.flags.flags4.globalSirenOn;
        self->state.globalSteadySiren = msg.data.msg00.flags.flags4.globalSteadySiren;
        self->state.busLineSeized = msg.data.msg00.flags.flags4.busLineSeized;
        self->state.busSniffReq = msg.data.msg00.flags.flags4.busSniffReq;
        self->state.dynBatteryTest = msg.data.msg00.flags.flags5.dynBatteryTest;
        self->state.acPowerOn = msg.data.msg00.flags.flags5.acPowerOn;
        self->state.lowBatteryMem = msg.data.msg00.flags.flags5.lowBatteryMem;
        self->state.groundFaultMem = msg.data.msg00.flags.flags5.groundFaultMem;
        self->state.fireAlarmVerify = msg.data.msg00.flags.flags5.fireAlarmVerify;
        self->state.smokePowerReset = msg.data.msg00.flags.flags5.smokePowerReset;
        self->state.line50HzDetected = msg.data.msg00.flags.flags5.line50HzDetected;
        self->state.hvBatteryCharge = msg.data.msg00.flags.flags5.hvBatteryCharge;
        self->state.commSinceAutotest = msg.data.msg00.flags.flags6.commSinceAutotest;
        self->state.powerUpDelay = msg.data.msg00.flags.flags6.powerUpDelay;
        self->state.walkTestMode = msg.data.msg00.flags.flags6.walkTestMode;
        self->state.lossSystemTime = msg.data.msg00.flags.flags6.lossSystemTime;
        self->state.enrollRequested = msg.data.msg00.flags.flags6.enrollRequested;
        self->state.testFixtureMode = msg.data.msg00.flags.flags6.testFixtureMode;
        self->state.ctrlShutdownMode = msg.data.msg00.flags.flags6.ctrlShutdownMode;
        self->state.cancelWindow = msg.data.msg00.flags.flags6.cancelWindow;
        break;
    }
    case 0x01: {
        self->state.panelID = msg.data.msg01.panelID;
        self->state.second = msg.data.msg01.second;
        self->state.minute = msg.data.msg01.minute;
        self->state.hour = msg.data.msg01.hour;
        self->state.day = msg.data.msg01.day;
        self->state.month = msg.data.msg01.month;
        self->state.weekday = msg.data.msg01.weekday;
        uint8_t newloghead = msg.data.msg01.logsize;
        uint8_t loghead = self->state.logHead;
        if (newloghead != loghead && !self->state.logReadFirstTime) {
            ESP_LOGI(LOG_TAG, "There are new SEL entries (%d)", newloghead - loghead);
            do {
                loghead = (loghead == self->state.logSize) ? 0 : loghead + 1;
                xQueueSend(self->logMonitorQueue, &loghead, 0);
            } while (newloghead != loghead);            
        }
        self->state.logHead = newloghead;
        self->state.logReadFirstTime = false;
        break;
    }
    case 0x04:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].fault = BITEX(msg.data.msg04.zoneFault[i / 8], i % 8);
        break;
    case 0x05:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].tamper = BITEX(msg.data.msg05.zoneTamper[i / 8], i % 8);
        break;
    case 0x06:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].trouble = BITEX(msg.data.msg06.zoneTrouble[i / 8], i % 8);
        break;
    case 0x07:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].bypass = BITEX(msg.data.msg07.zoneBypass[i / 8], i % 8);
        break;
    case 0x08:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].alarmMemory = BITEX(msg.data.msg08.zoneAlarmMem[i / 8], i % 8);
        break;
    case 0x09:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].inhibit = BITEX(msg.data.msg09.zoneInhibit[i / 8], i % 8);
        break;
    case 0x0A:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].lowBattery = BITEX(msg.data.msg0A.zoneLowBat[i / 8], i % 8);
        break;
    case 0x0B:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].lost = BITEX(msg.data.msg0B.zoneLost[i / 8], i % 8);
        break;
    case 0x0C:
        for (size_t i = 0; i < 48; i++)
            self->state.zones[i].bypassMemory = BITEX(msg.data.msg0C.zoneBypassMem[i / 8], i % 8);
        break;
    case 0x16:
        self->state.commStackPointer = msg.data.msg16.commStackPointer;
        break;
    case 0x18: {
        uint8_t p = msg.data.msg18.partition;
        if (p >= MAX_PARTITION)
            break;
        self->state.partitions[p].bypassCodeRequired = msg.data.msg18.flags1.bypassCodeRequired;
        self->state.partitions[p].fireTrouble = msg.data.msg18.flags1.fireTrouble;
        self->state.partitions[p].fire = msg.data.msg18.flags1.fire;
        self->state.partitions[p].pulsingBuzzer = msg.data.msg18.flags1.pulsingBuzzer;
        self->state.partitions[p].tlmFaultMemory = msg.data.msg18.flags1.tlmFaultMemory;
        self->state.partitions[p].armed = msg.data.msg18.flags1.armed;
        self->state.partitions[p].instant = msg.data.msg18.flags1.instant;
        self->state.partitions[p].previousAlarm = msg.data.msg18.flags2.previousAlarm;
        self->state.partitions[p].sirenOn = msg.data.msg18.flags2.sirenOn;
        self->state.partitions[p].steadySirenOn = msg.data.msg18.flags2.steadySirenOn;
        self->state.partitions[p].alarmMemory = msg.data.msg18.flags2.alarmMemory;
        self->state.partitions[p].tamper = msg.data.msg18.flags2.tamper;
        self->state.partitions[p].cancelCmdEntered = msg.data.msg18.flags2.cancelCmdEntered;
        self->state.partitions[p].codeEntered = msg.data.msg18.flags2.codeEntered;
        self->state.partitions[p].cancelPending = msg.data.msg18.flags2.cancelPending;
        self->state.partitions[p].silentExitEnabled = msg.data.msg18.flags3.silentExitEnabled;
        self->state.partitions[p].entryGuard = msg.data.msg18.flags3.entryGuard;
        self->state.partitions[p].chimeModeOn = msg.data.msg18.flags3.chimeModeOn;
        self->state.partitions[p].entry = msg.data.msg18.flags3.entry;
        self->state.partitions[p].delayExpiration = msg.data.msg18.flags3.delayExpiration;
        self->state.partitions[p].exit1 = msg.data.msg18.flags3.exit1;
        self->state.partitions[p].exit2 = msg.data.msg18.flags3.exit2;
        self->state.partitions[p].ledExtinguish = msg.data.msg18.flags4.ledExtinguish;
        self->state.partitions[p].crossTiming = msg.data.msg18.flags4.crossTiming;
        self->state.partitions[p].recentClosing = msg.data.msg18.flags4.recentClosing;
        self->state.partitions[p].exitErrorTriggered = msg.data.msg18.flags4.exitErrorTriggered;
        self->state.partitions[p].autoHomeInhibited = msg.data.msg18.flags4.autoHomeInhibited;
        self->state.partitions[p].sensorLowBattery = msg.data.msg18.flags4.sensorLowBattery;
        self->state.partitions[p].sensorLostSup = msg.data.msg18.flags4.sensorLostSup;
        self->state.partitions[p].zoneBypassed = msg.data.msg18.flags5.zoneBypassed;
        self->state.partitions[p].forceArmAuto = msg.data.msg18.flags5.forceArmAuto;
        self->state.partitions[p].readyToArm = msg.data.msg18.flags5.readyToArm;
        self->state.partitions[p].readyToForceArm = msg.data.msg18.flags5.readyToForceArm;
        self->state.partitions[p].validPinAccepted = msg.data.msg18.flags5.validPinAccepted;
        self->state.partitions[p].chimeOn = msg.data.msg18.flags5.chimeOn;
        self->state.partitions[p].errorBeep = msg.data.msg18.flags5.errorBeep;
        self->state.partitions[p].toneOn = msg.data.msg18.flags5.toneOn;
        self->state.partitions[p].entry1 = msg.data.msg18.flags6.entry1;
        self->state.partitions[p].openPeriod = msg.data.msg18.flags6.openPeriod;
        self->state.partitions[p].alarmPhone1 = msg.data.msg18.flags6.alarmPhone1;
        self->state.partitions[p].alarmPhone2 = msg.data.msg18.flags6.alarmPhone2;
        self->state.partitions[p].alarmPhone3 = msg.data.msg18.flags6.alarmPhone3;
        self->state.partitions[p].cancelReportStack = msg.data.msg18.flags6.cancelReportStack;
        self->state.partitions[p].keyswitchArmed = msg.data.msg18.flags6.keyswitchArmed;
        self->state.partitions[p].delayTrip = msg.data.msg18.flags6.delayTrip;
        self->state.partitions[p].lastUserNum = msg.data.msg18.lastUserNum;
        self->state.partitions[p].calculateAggregateState();
        break;
    }
    case 0x19: {
        if (msg.data.msg19.panelId != 0x00)
            break;
        self->state.callBackInProgress = msg.data.msg19.flags9.callBackInProgress;
        self->state.phoneLineFaulted = msg.data.msg19.flags10.phoneLineFaulted;
        self->state.voltageInterrupt = msg.data.msg19.flags10.voltageInterrupt;
        self->state.housePhoneOffHook = msg.data.msg19.flags10.housePhoneOffHook;
        self->state.phoneMonitorEnable = msg.data.msg19.flags10.phoneMonitorEnable;
        self->state.sniffing = msg.data.msg19.flags10.sniffing;
        self->state.lastReadOffHook = msg.data.msg19.flags10.lastReadOffHook;
        self->state.listenInRequested = msg.data.msg19.flags10.listenInRequested;
        self->state.listenInTrigger = msg.data.msg19.flags10.listenInTrigger;
        for (size_t p = 0; p < 8; p++) {
            self->state.partitions[p].validPart = BITEX(msg.data.msg19.validPartMask, p);
        }
        break;
    }
    default:
        break;
    }
}

void NXPanel::processSinglecastMessage(NXPanel* self, NXMessage msg)
{
    if (msg.msgNum() == 0x10)
        return; // Ignore ping messages
    if (msg.msgNum() == 0x19)
        processBroadcastMessage(self, msg); // If System status message was received by request 0xA4
    if (msg.dest == self->ownBusAddr && (msg.msgNum() == 0x20 || msg.msgNum() == 0x60)) {
        ESP_LOGI(LOG_TAG, "Gateway programming from KB not realised yet");
        // TODO: self programming
    }

    ESP_LOGD(LOG_TAG, "Got message %#X for thread %d", msg.msgNum(), self->communicatingTask);
    self->currentResponse = msg;
    if (self->communicatingTask != NULL)
        xSemaphoreGive(self->responseSemaphore);
}

void NXPanel::monitorLogEntries(void* arg)
{
    NXPanel* self = static_cast<NXPanel*>(arg);
    uint8_t id;
    while (true) {
        if (xQueueReceive(self->logMonitorQueue, &id, portMAX_DELAY)) {
            NXLogRecord r;
            for (uint8_t retry = 0; retry < 3; retry++) {
                ESP_LOGI(LOG_TAG, "Fetching SEL entry 0x%X", id);
                if (panel.fun.nxGetLogRecord(id, &r) == rqOK) {
                    MQTT.send("event", r.toJSON(), false, 1);
                    break;
                }
            }
        }
    }
}

void NXPanel::start(uint8_t busAddress)
{
    rawMsgQueue = xQueueCreate(NX_DBG_QUEUE_LEN, sizeof(RAWMessage_t));
	nxRecvQueue = xQueueCreate(NX_MSG_QUEUE_LEN, sizeof(NXMessage));
	nxSendQueue = xQueueCreate(NX_MSG_QUEUE_LEN, sizeof(NXMessage));
    logMonitorQueue = xQueueCreate(16, sizeof(uint8_t));
    busSemaphore = xSemaphoreCreateMutex();
    responseSemaphore = xSemaphoreCreateBinary();
    ESP_LOGI(LOG_TAG, "Starting gateway on 0x%X", busAddress);
    if (busAddress < 9) {
        ESP_LOGE(LOG_TAG, "Incorrect module address 0x%X", busAddress);
        return;
    }
	xTaskCreate(processNXMessage, "processNXMessage", 2048, this, 10, &nxMsgProcessTask);
    xTaskCreate(monitorLogEntries, "logMonitorTask", 2048, this, tskIDLE_PRIORITY + 1, &logMonitorTask);
	ownBusAddr = busAddress;
	NXMessage pong;
	pong.dest = 0x08;
	pong.data.msg45.msgnum = 0xC5;
	pong.data.msg45.moduleAddress = busAddress;
	pong.data.msg45.unk1 = 0;
	pong.data.msg45.unk2 = 0;
	bus.setPongMessage(pong);
	bus.start(busAddress);
    panel.fun.nxGetSystemStatus();
}

void NXPanel::stop()
{
	bus.stop();
    ESP_LOGI(LOG_TAG, "Gateway stopped");
}

void NXPanel::enableDebug()
{
    if (nxDebugProcessTask != NULL) {
        ESP_LOGI(LOG_TAG, "Debug is already running");
        return;
    }
    ESP_LOGI(LOG_TAG, "Starting debug");
    debugTcpServer.begin();
    xTaskCreate(processNXDebug, "processNXDebug", 2048, this, 5, &nxDebugProcessTask);
    bus.debugEnabled = true;
}

void NXPanel::disableDebug()
{
    if (nxDebugProcessTask == NULL) {
        ESP_LOGI(LOG_TAG, "Debug is not running");
        return;
    }
    bus.debugEnabled = false;
    vTaskDelete(nxDebugProcessTask);
    nxDebugProcessTask = NULL;
    debugTcpServer.stop();
    ESP_LOGI(LOG_TAG, "Debug stopped");
}

String requestResultToStr(requestResult r)
 {
	switch (r)
	{
	case rqOK: 		return "Request successed";
	case rqBADRQ: 	return "Bad request";
	case rqTOUT: 	return "Request timed out";
	case rqBUSY: 	return "Another request is pending. Try later.";
	case rqNACK: 	return "Request declined";
    default:        return "";
	}
}

NXPanel panel;