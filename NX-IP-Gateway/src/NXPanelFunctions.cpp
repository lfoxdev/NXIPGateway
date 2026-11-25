#include "NXPanelFunctions.h"
#include "NXPanel.h"

#define LOG_TAG "NXGAPI"

#define PART_TO_MASK(p) (1 << (p - 1))

requestResult NXPanelFunctions::nxGetSystemStatus()
{
    NXMessage m;
    m.dest = PANEL;
    m.data.msg24.msgnum = 0xA4;
    m.data.msg24.sourceAddr = panel->ownBusAddr;
    m.data.msg24.panelID = 0;
    return panel->makeRequest(&m);
}

requestResult NXPanelFunctions::nxKpdFunction(kpd_function fun, uint8_t partition)
{
    if (partition < 1 || partition > MAX_PARTITION)
        return requestResult::rqBADRQ;
    NXMessage m;
    m.dest = PANEL;
    m.data.msg51.msgnum = 0xD1;
    m.data.msg51.sourceAddr = panel->ownBusAddr;
    m.data.msg51.function = fun;
    m.data.msg51.partitionMask = 1 << (partition - 1);
    requestResult r = panel->makeRequestCheckAck(&m);
    ESP_LOGI(LOG_TAG, "Performed function %d, rc=%d", fun, r);
    return r;
}

requestResult NXPanelFunctions::nxBypass(uint8_t zone)
{
    if (zone < 1 || zone > MAX_ZONE)
        return requestResult::rqBADRQ;
    NXMessage m;
    m.dest = PANEL;
    m.data.msg52.msgnum = 0xD2;
    m.data.msg52.sourceAddr = panel->ownBusAddr;
    m.data.msg52.zone = zone - 1;
    requestResult r = panel->makeRequestCheckAck(&m);
    ESP_LOGI(LOG_TAG, "Toggle bypass, rc=%d", r);
    return r;
}

requestResult NXPanelFunctions::nxSetDateTime(dateTime dt)
{
    NXMessage m;
    m.dest = PANEL;
    m.data.msg60.msgnum = 0xE0;
    m.data.msg60.moduleAddr = panel->ownBusAddr;
    m.data.msg60.locationInfo.segmentSizeOrDataBank = t_segmentSizeOrDataBank::ram;
    m.data.msg60.locationInfo.readWrite = t_readWrite::rw_write;
    m.data.msg60.locationInfo.segmentOffset = t_segmentOffset::none;
    m.data.msg60.locationInfo.locationType = t_locType::physycal;
    m.data.msg60.setLocation(0x70);
    m.data.msg60.dataInfo.numSegments = 3;
    m.data.msg60.dataInfo.dataType = t_dataType::binary;
    m.data.msg60.data[0] = (static_cast<uint8_t>(dt.month) << 4) + static_cast<uint8_t>(dt.weekday);
    m.data.msg60.data[1] = static_cast<uint8_t>(dt.day);
    m.data.msg60.data[2] = static_cast<uint8_t>(dt.year - 2000);
    requestResult r = panel->makeRequestCheckAck(&m);
    ESP_LOGI(LOG_TAG, "Date set, rc=%d", r);
    if (r != rqOK)
        return r;
    m.data.msg60.setLocation(0x73);
    m.data.msg60.dataInfo.numSegments = 2;
    m.data.msg60.data[0] = static_cast<uint8_t>(dt.hour);
    m.data.msg60.data[1] = static_cast<uint8_t>(dt.minute);
    r = panel->makeRequestCheckAck(&m);
    ESP_LOGI(LOG_TAG, "Time set, rc=%d", r);
    return r;
}

requestResult NXPanelFunctions::nxGetLogRecord(uint8_t id, NXLogRecord* record)
{
    NXMessage m, l;
    m.dest = PANEL;
    m.data.msg21.msgnum = 0xA1;
    m.data.msg21.sourceAddr = panel->ownBusAddr;
    m.data.msg21.itemNumber = id;
    requestResult r = panel->makeRequest(&m, &l);
    ESP_LOGI(LOG_TAG, "Get SEL entry id=%d, rc=%d, res=%#X", id, r, l.msgNum());
    if (r != rqOK)
        return (requestResult)r;
    if (l.msgNum() != 0x61)
        return rqNACK;
    record->id = l.data.msg61.logNumber;
    record->event = l.data.msg61.event;
    record->item = l.data.msg61.itemNumber;
    record->reporting = l.data.msg61.reportingevent;
    record->month = l.data.msg61.month();
    record->day = l.data.msg61.day;
    record->hour = l.data.msg61.hour;
    record->minute = l.data.msg61.minute;
    record->partition = l.data.msg61.partition();
    panel->state.logSize = l.data.msg61.logSize;
    m.data.msg22.msgnum = 0xA2;
    m.data.msg21.sourceAddr = panel->ownBusAddr;
    m.data.msg22.itemNumber = id;
    r = panel->makeRequest(&m, &l);
    ESP_LOGI(LOG_TAG, "Get communicator entry id=%d, rc=%d, res=%#X", id, r, l.msgNum());
    if (r != rqOK)
        return (requestResult)r;
    if (l.msgNum() != 0x62)
        return rqNACK;
    record->cid_qualifier = l.data.msg62.CIDEventQualifier;
    record->cid_code = (l.data.msg62.CIDEventCode[0].code % 10) * 100 + (l.data.msg62.CIDEventCode[1].code % 10) * 10 + (l.data.msg62.CIDEventCode[2].code % 10);
    record->sia[0] = sanitize(l.data.msg62.SIACode[0]);
    record->sia[1] = sanitize(l.data.msg62.SIACode[1]);
    return rqOK;
}

checkPinResult NXPanelFunctions::checkPin(pin_t pin, uint8_t partition, checkPinAuthReq requiredAuth)
{
    NXMessage m, l;
    m.dest = PANEL;
    m.data.msg50.msgnum = 0xD0;
    m.data.msg50.sourceAddr = panel->ownBusAddr;
    m.data.msg50.flags.authRequested = true;
    m.data.msg50.partitionMask = PART_TO_MASK(partition);
    m.data.msg50.pin = pin;
    requestResult r = panel->makeRequest(&m, &l);
    if (r != rqOK || l.msgNum() != 0x70) {
        return pinCommErr;
    }
    if (l.data.msg70.userNumber == 0)
        return pinNoUser;
    if (requiredAuth != authProgram && partition != 255 && (l.data.msg70.partitionMask & PART_TO_MASK(partition)) == 0)
        return pinNoAuth;
    if (requiredAuth == authArm)
        return (!l.data.msg70.authorityFlags.normal.mustBeZero && l.data.msg70.authorityFlags.normal.armOnly) ||
               (!l.data.msg70.authorityFlags.normal.mustBeZero && l.data.msg70.authorityFlags.normal.armOnlyClose) ||
               (l.data.msg70.authorityFlags.normal.armDisarm) ? pinOK : pinNoAuth;
    else if (requiredAuth == authDisarm)
        return (l.data.msg70.authorityFlags.normal.armDisarm) ? pinOK : pinNoAuth;
    else if (requiredAuth == authBypass)
        return (l.data.msg70.authorityFlags.normal.bypassEnable) ? pinOK : pinNoAuth;
    else if (requiredAuth == authMaster)
        return (!l.data.msg70.authorityFlags.normal.mustBeZero && l.data.msg70.authorityFlags.normal.masterProgram) ? pinOK : pinNoAuth;
    else if (requiredAuth == authProgram)
        return (l.data.msg70.userNumber == 255) ? pinOK : pinNoAuth;
    else
        return pinOK;
}