#include "CommonStructures.h"
#include "NXMessageCommon.h"
#include <WString.h>
#include <ArduinoJson.h>

String NXLogRecord::eventStr()
{
    switch (event) {
        case Alarm: return F("Alarm");
        case AlarmRestore: return F("Alarm restore");
        case Bypass: return F("Bypass");
        case BypassRestore: return F("Bypass restore");
        case Tamper: return F("Tamper");
        case TamperRestore: return F("Tamper restore");
        case Trouble: return F("Trouble");
        case TroubleRestore: return F("Trouble restore");
        case TxLowBattery: return F("Transmitter low battery");
        case TxLowBatteryRestore: return F("Transmitter low battery restore");
        case ZoneLost: return F("Zone lost");
        case ZoneLostRestore: return F("Zone lost restore");
        case StartOfCrossTime: return F("Start of cross time");
        case SpecialExpansionEvent: return F("Special expansion event");
        case Duress: return F("Duress");
        case ManualFire: return F("Manual fire");
        case Auxiliary2Panic: return F("Auxiliary 2 panic");
        case Panic: return F("Panic");
        case KeypadTamper: return F("Keypad tamper");
        case ControlBoxTamper: return F("Control box tamper");
        case ControlBoxTamperRestore: return F("Control box tamper restore");
        case AcFail: return F("AC fail");
        case AcFailRestore: return F("AC fail restore");
        case LowBattery: return F("Low battery");
        case LowBatteryRestore: return F("Low battery restore");
        case OverCurrent: return F("Over current");
        case OverCurrentRestore: return F("Over current restore");
        case SirenTamper: return F("Siren tamper");
        case SirenTamperRestore: return F("Siren tamper restore");
        case TelephoneFault: return F("Telephone fault");
        case TelephoneFaultRestore: return F("Telephone fault restore");
        case ExpanderTrouble: return F("Expander trouble");
        case ExpanderTroubleRestore: return F("Expander trouble restore");
        case FailToCommunicate: return F("Fail to communicate");
        case LogFull: return F("Log full");
        case Opening: return F("Opening");
        case Closing: return F("Closing");
        case ExitError: return F("Exit error");
        case RecentClosing: return F("Recent closing");
        case AutoTest: return F("Auto test");
        case StartProgram: return F("Start program");
        case EndProgram: return F("End program");
        case StartDownload: return F("Start download");
        case EndDownload: return F("End download");
        case Cancel: return F("Cancel");
        case GroundFault: return F("Ground fault");
        case GroundFaultRestore: return F("Ground fault restore");
        case ManualTest: return F("Manual test");
        case ClosedWithZonesBypassed: return F("Closed with zones bypassed");
        case StartOfListenIn: return F("Start of listen in");
        case TechnicianOnSite: return F("Technician on site");
        case TechnicianLeft: return F("Technician left");
        case ControlPowerUp: return F("Control power up");
        case EnrolledDevice: return F("Enrolled device");
        case ClockSet: return F("Clock set");
        case FirstToOpen: return F("First to open");
        case LastToClose: return F("Last to close");
        case PinEnteredBit7Set: return F("PIN entered bit 7 set");
        case BeginWalkTest: return F("Begin walk test");
        case EndWalkTest: return F("End walk test");
        case ReExit: return F("Re-exit");
        case OutputTrip: return F("Output trip");
        case DataLost: return F("Data lost");
        default: return "[Event " + String(event) + "]";
    }
}

String NXLogRecord::itemStr()
{
    if (event <= 12)
        return "Zone " + String(item + 1);
    else if ((event >= 40 && event <= 43) || event == 49 || event == 53 || (event >= 120 && event <= 122) || event == 126)
        return "User " + String(item + 1);
    else if ((event >= 24 && event <= 33) || event == 36 || event == 37 || event == 82 || event == 117)
        return "Device " + String(item);
    else if (event == 17)
        return "ID " + String(item);
    else
        return "";
}

String NXLogRecord::toJSON()
{
    StaticJsonDocument<256> doc;
    doc["id"] = id;
    doc["event_num"] = event;
    doc["event"] = eventStr();
    doc["report"] = reporting;
    doc["item"] = item;
    doc["month"] = month;
    doc["day"] = day;
    doc["hour"] = hour;
    doc["minute"] = minute;
    doc["partition"] = partition;
    doc["cid_q"] = cid_qualifier;
    doc["cid_c"] = cid_code;
    char siaBuf[3];
    siaBuf[0] = sia[0];
    siaBuf[1] = sia[1];
    siaBuf[2] = '\0';
    doc["sia"] = siaBuf;
    String output;
    serializeJson(doc, output);
    return output;
}