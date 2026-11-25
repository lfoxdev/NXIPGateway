// NXPanelState.h
// Panel state (system, partitions, zones) storage

#ifndef _NXPANELSTATE_h
#define _NXPANELSTATE_h

#include "NXPanelStateItem.h"
#include "Config.h"
#include "MQTT.h"

// Forward declarations
class NXPanel;

void onStateChangeQ0(IPanelStateItem* item);
void onStateChangeQ1(IPanelStateItem* item);

struct ZoneStatus {
    NXPanelStateItem<bool> fault;
    NXPanelStateItem<bool> tamper;
    NXPanelStateItem<bool> trouble;
    NXPanelStateItem<bool> bypass;
    NXPanelStateItem<bool> alarmMemory;
    NXPanelStateItem<bool> inhibit;
    NXPanelStateItem<bool> lowBattery;
    NXPanelStateItem<bool> lost;
    NXPanelStateItem<bool> bypassMemory;
};

template <std::size_t I>
constexpr ZoneStatus makeZone() {
    return ZoneStatus{
        NXPanelStateItem<bool>("zones", I, "fault", onStateChangeQ0),
        NXPanelStateItem<bool>("zones", I, "tamper", onStateChangeQ0),
        NXPanelStateItem<bool>("zones", I, "trouble", onStateChangeQ0),
        NXPanelStateItem<bool>("zones", I, "bypass", onStateChangeQ0),
        NXPanelStateItem<bool>("zones", I, "alarmMemory"),
        NXPanelStateItem<bool>("zones", I, "inhibit", onStateChangeQ0),
        NXPanelStateItem<bool>("zones", I, "lowBattery", onStateChangeQ1),
        NXPanelStateItem<bool>("zones", I, "lost", onStateChangeQ1),
        NXPanelStateItem<bool>("zones", I, "bypassMemory")
    };
}

template <std::size_t... Is>
constexpr auto makeZones(std::index_sequence<Is...>) {
    return std::array<ZoneStatus, sizeof...(Is)>{ makeZone<Is + 1>()... };
}

struct PartitionStatus {
    NXPanelStateItem<bool> validPart;
    NXPanelStateItem<bool> bypassCodeRequired;
    NXPanelStateItem<bool> fireTrouble;
    NXPanelStateItem<bool> fire;
    NXPanelStateItem<bool> pulsingBuzzer;
    NXPanelStateItem<bool> tlmFaultMemory;
    NXPanelStateItem<bool> armed;
    NXPanelStateItem<bool> instant;
    NXPanelStateItem<bool> previousAlarm;
    NXPanelStateItem<bool> sirenOn;
    NXPanelStateItem<bool> steadySirenOn;
    NXPanelStateItem<bool> alarmMemory;
    NXPanelStateItem<bool> tamper;
    NXPanelStateItem<bool> cancelCmdEntered;
    NXPanelStateItem<bool> codeEntered;
    NXPanelStateItem<bool> cancelPending;
    NXPanelStateItem<bool> silentExitEnabled;
    NXPanelStateItem<bool> entryGuard;
    NXPanelStateItem<bool> chimeModeOn;
    NXPanelStateItem<bool> entry;
    NXPanelStateItem<bool> delayExpiration;
    NXPanelStateItem<bool> exit1;
    NXPanelStateItem<bool> exit2;
    NXPanelStateItem<bool> ledExtinguish;
    NXPanelStateItem<bool> crossTiming;
    NXPanelStateItem<bool> recentClosing;
    NXPanelStateItem<bool> exitErrorTriggered;
    NXPanelStateItem<bool> autoHomeInhibited;
    NXPanelStateItem<bool> sensorLowBattery;
    NXPanelStateItem<bool> sensorLostSup;
    NXPanelStateItem<bool> zoneBypassed;
    NXPanelStateItem<bool> forceArmAuto;
    NXPanelStateItem<bool> readyToArm;
    NXPanelStateItem<bool> readyToForceArm;
    NXPanelStateItem<bool> validPinAccepted;
    NXPanelStateItem<bool> chimeOn;
    NXPanelStateItem<bool> errorBeep;
    NXPanelStateItem<bool> toneOn;
    NXPanelStateItem<bool> entry1;
    NXPanelStateItem<bool> openPeriod;
    NXPanelStateItem<bool> alarmPhone1;
    NXPanelStateItem<bool> alarmPhone2;
    NXPanelStateItem<bool> alarmPhone3;
    NXPanelStateItem<bool> cancelReportStack;
    NXPanelStateItem<bool> keyswitchArmed;
    NXPanelStateItem<bool> delayTrip;
    NXPanelStateItem<int> lastUserNum;
    AggregateState aggregateState;
    void calculateAggregateState();
};

template <std::size_t I>
constexpr PartitionStatus makePartition() {
    return PartitionStatus{
        NXPanelStateItem<bool>("sys",  I, "validPart"),
        NXPanelStateItem<bool>("part", I, "bypassCodeRequired"),
        NXPanelStateItem<bool>("part", I, "fireTrouble", onStateChangeQ1),
        NXPanelStateItem<bool>("part", I, "fire", onStateChangeQ1),
        NXPanelStateItem<bool>("part", I, "pulsingBuzzer"),
        NXPanelStateItem<bool>("part", I, "tlmFaultMemory"),
        NXPanelStateItem<bool>("part", I, "armed", onStateChangeQ1),
        NXPanelStateItem<bool>("part", I, "instant", onStateChangeQ1),
        NXPanelStateItem<bool>("part", I, "previousAlarm"),
        NXPanelStateItem<bool>("part", I, "sirenOn"),
        NXPanelStateItem<bool>("part", I, "steadySirenOn"),
        NXPanelStateItem<bool>("part", I, "alarmMemory"),
        NXPanelStateItem<bool>("part", I, "tamper"),
        NXPanelStateItem<bool>("part", I, "cancelCmdEntered"),
        NXPanelStateItem<bool>("part", I, "codeEntered"),
        NXPanelStateItem<bool>("part", I, "cancelPending"),
        NXPanelStateItem<bool>("part", I, "silentExitEnabled"),
        NXPanelStateItem<bool>("part", I, "entryGuard"),
        NXPanelStateItem<bool>("part", I, "chimeModeOn", onStateChangeQ1),
        NXPanelStateItem<bool>("part", I, "entry"),
        NXPanelStateItem<bool>("part", I, "delayExpiration"),
        NXPanelStateItem<bool>("part", I, "exit1"),
        NXPanelStateItem<bool>("part", I, "exit2"),
        NXPanelStateItem<bool>("part", I, "ledExtinguish"),
        NXPanelStateItem<bool>("part", I, "crossTiming"),
        NXPanelStateItem<bool>("part", I, "recentClosing"),
        NXPanelStateItem<bool>("part", I, "exitErrorTriggered"),
        NXPanelStateItem<bool>("part", I, "autoHomeInhibited"),
        NXPanelStateItem<bool>("part", I, "sensorLowBattery"),
        NXPanelStateItem<bool>("part", I, "sensorLostSup"),
        NXPanelStateItem<bool>("part", I, "zoneBypassed"),
        NXPanelStateItem<bool>("part", I, "forceArmAuto"),
        NXPanelStateItem<bool>("part", I, "readyToArm", onStateChangeQ1),
        NXPanelStateItem<bool>("part", I, "readyToForceArm"),
        NXPanelStateItem<bool>("part", I, "validPinAccepted"),
        NXPanelStateItem<bool>("part", I, "chimeOn", onStateChangeQ1),
        NXPanelStateItem<bool>("part", I, "errorBeep"),
        NXPanelStateItem<bool>("part", I, "toneOn"),
        NXPanelStateItem<bool>("part", I, "entry1"),
        NXPanelStateItem<bool>("part", I, "openPeriod"),
        NXPanelStateItem<bool>("part", I, "alarmPhone1"),
        NXPanelStateItem<bool>("part", I, "alarmPhone2"),
        NXPanelStateItem<bool>("part", I, "alarmPhone3"),
        NXPanelStateItem<bool>("part", I, "cancelReportStack"),
        NXPanelStateItem<bool>("part", I, "keyswitchArmed"),
        NXPanelStateItem<bool>("part", I, "delayTrip"),
        NXPanelStateItem<int> ("part", I, "lastUserNum"),
        AggregateState("part", I, "state", onStateChangeQ1)
    };
}

template <std::size_t... Is>
constexpr auto makePartitions(std::index_sequence<Is...>) {
    return std::array<PartitionStatus, sizeof...(Is)>{ makePartition<Is + 1>()... };
}

class NXPanelState
{
 friend class NXPanel;
 protected:
    NXPanel* panel;

 public:
     constexpr explicit NXPanelState(NXPanel* _p) : 
         panel(_p),
         zones(makeZones(std::make_index_sequence<MAX_ZONE>{})),
         partitions(makePartitions(std::make_index_sequence<MAX_PARTITION>{}))
            {}
     
     uint8_t month = 1, day = 1, hour = 0, minute = 0, second = 0, weekday = 0;
     uint8_t panelID = 0;
     uint8_t logSize = 0xB9; // Absolute maximum log size, 185 by default
     uint8_t logHead = 0; // Number of last log item
     uint8_t commStackPointer = 0;
     bool logReadFirstTime = true;
 
    NXPanelStateItem<bool> lineSeizure      { "sys", 0, "lineSeizure" };
    NXPanelStateItem<bool> offHook          { "sys", 0, "offHook" };
    NXPanelStateItem<bool> handshakeReceived{ "sys", 0, "handshakeReceived" };
    NXPanelStateItem<bool> downloadInProgress{ "sys", 0, "downloadInProgress" };
    NXPanelStateItem<bool> dialerDelay      { "sys", 0, "dialerDelay" };
    NXPanelStateItem<bool> usingBackupPhone { "sys", 0, "usingBackupPhone" };
    NXPanelStateItem<bool> listenInActive   { "sys", 0, "listenInActive" };
    NXPanelStateItem<bool> twoWayLockout    { "sys", 0, "twoWayLockout" };
    NXPanelStateItem<bool> groundFault      { "sys", 0, "groundFault" };
    NXPanelStateItem<bool> phoneFault       { "sys", 0, "phoneFault", onStateChangeQ1 };
    NXPanelStateItem<bool> failToCommunicate{ "sys", 0, "failToCommunicate" };
    NXPanelStateItem<bool> fuseFault        { "sys", 0, "fuseFault", onStateChangeQ1  };
    NXPanelStateItem<bool> boxTamper        { "sys", 0, "boxTamper", onStateChangeQ1  };
    NXPanelStateItem<bool> sirenTamper      { "sys", 0, "sirenTamper", onStateChangeQ1  };
    NXPanelStateItem<bool> lowBattery       { "sys", 0, "lowBattery", onStateChangeQ1 };
    NXPanelStateItem<bool> acFail           { "sys", 0, "acFail", onStateChangeQ1 };
    NXPanelStateItem<bool> expBoxTamper     { "sys", 0, "expBoxTamper", onStateChangeQ1  };
    NXPanelStateItem<bool> expAcFailure     { "sys", 0, "expAcFailure", onStateChangeQ1  };
    NXPanelStateItem<bool> expLowBattery    { "sys", 0, "expLowBattery", onStateChangeQ1  };
    NXPanelStateItem<bool> expLossSup       { "sys", 0, "expLossSup", onStateChangeQ1  };
    NXPanelStateItem<bool> expAuxOverCurrent{ "sys", 0, "expAuxOverCurrent" };
    NXPanelStateItem<bool> auxCommFail      { "sys", 0, "auxCommFail" };
    NXPanelStateItem<bool> expBellFault     { "sys", 0, "expBellFault", onStateChangeQ1  };
    NXPanelStateItem<bool> pin6DigitEnabled { "sys", 0, "pin6DigitEnabled" };
    NXPanelStateItem<bool> tokenInUse       { "sys", 0, "tokenInUse" };
    NXPanelStateItem<bool> pinRequiredLocal { "sys", 0, "pinRequiredLocal" };
    NXPanelStateItem<bool> globalBuzzer     { "sys", 0, "globalBuzzer" };
    NXPanelStateItem<bool> globalSirenOn    { "sys", 0, "globalSirenOn" };
    NXPanelStateItem<bool> globalSteadySiren{ "sys", 0, "globalSteadySiren" };
    NXPanelStateItem<bool> busLineSeized    { "sys", 0, "busLineSeized" };
    NXPanelStateItem<bool> busSniffReq      { "sys", 0, "busSniffReq" };
    NXPanelStateItem<bool> dynBatteryTest   { "sys", 0, "dynBatteryTest" };
    NXPanelStateItem<bool> acPowerOn        { "sys", 0, "acPowerOn"  };
    NXPanelStateItem<bool> lowBatteryMem    { "sys", 0, "lowBatteryMem" };
    NXPanelStateItem<bool> groundFaultMem   { "sys", 0, "groundFaultMem" };
    NXPanelStateItem<bool> fireAlarmVerify  { "sys", 0, "fireAlarmVerify" };
    NXPanelStateItem<bool> smokePowerReset  { "sys", 0, "smokePowerReset" };
    NXPanelStateItem<bool> line50HzDetected { "sys", 0, "line50HzDetected" };
    NXPanelStateItem<bool> hvBatteryCharge  { "sys", 0, "hvBatteryCharge" };
    NXPanelStateItem<bool> commSinceAutotest{ "sys", 0, "commSinceAutotest" };
    NXPanelStateItem<bool> powerUpDelay     { "sys", 0, "powerUpDelay" };
    NXPanelStateItem<bool> walkTestMode     { "sys", 0, "walkTestMode" };
    NXPanelStateItem<bool> lossSystemTime   { "sys", 0, "lossSystemTime", onStateChangeQ1  };
    NXPanelStateItem<bool> enrollRequested  { "sys", 0, "enrollRequested" };
    NXPanelStateItem<bool> testFixtureMode  { "sys", 0, "testFixtureMode" };
    NXPanelStateItem<bool> ctrlShutdownMode { "sys", 0, "ctrlShutdownMode" };
    NXPanelStateItem<bool> cancelWindow     { "sys", 0, "cancelWindow" };
    NXPanelStateItem<bool> callBackInProgress{ "sys", 0, "callBackInProgress" };
    NXPanelStateItem<bool> phoneLineFaulted { "sys", 0, "phoneLineFaulted", onStateChangeQ1  };
    NXPanelStateItem<bool> voltageInterrupt { "sys", 0, "voltageInterrupt" };
    NXPanelStateItem<bool> housePhoneOffHook{ "sys", 0, "housePhoneOffHook" };
    NXPanelStateItem<bool> phoneMonitorEnable{ "sys", 0, "phoneMonitorEnable" };
    NXPanelStateItem<bool> sniffing         { "sys", 0, "sniffing" };
    NXPanelStateItem<bool> lastReadOffHook  { "sys", 0, "lastReadOffHook" };
    NXPanelStateItem<bool> listenInRequested{ "sys", 0, "listenInRequested" };
    NXPanelStateItem<bool> listenInTrigger  { "sys", 0, "listenInTrigger" };

    std::array<ZoneStatus, MAX_ZONE> zones;
    std::array<PartitionStatus, MAX_PARTITION> partitions;

};



#endif