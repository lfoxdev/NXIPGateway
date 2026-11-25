// NXMessageCommon.h
// Structures for storing common NX bus and NX-584 messages

#pragma once

#include "Config.h"

struct univ_msg_t {
    uint8_t byte[MAX_PACKET_SIZE];
};

struct partitionStatusMessage_t {
    uint8_t msgnum;
    uint8_t partition;
    struct {
        uint8_t bypassCodeRequired : 1;
        uint8_t fireTrouble : 1;
        uint8_t fire : 1;
        uint8_t pulsingBuzzer : 1;
        uint8_t tlmFaultMemory : 1;
        uint8_t reserved : 1;
        uint8_t armed : 1;
        uint8_t instant : 1;
    } flags1;
    struct {
        uint8_t previousAlarm : 1;
        uint8_t sirenOn : 1;
        uint8_t steadySirenOn : 1;
        uint8_t alarmMemory : 1;
        uint8_t tamper : 1;
        uint8_t cancelCmdEntered : 1;
        uint8_t codeEntered : 1;
        uint8_t cancelPending : 1;
    } flags2;
    struct {
        uint8_t reserved : 1;
        uint8_t silentExitEnabled : 1;
        uint8_t entryGuard : 1;
        uint8_t chimeModeOn : 1;
        uint8_t entry : 1;
        uint8_t delayExpiration : 1;
        uint8_t exit1 : 1;
        uint8_t exit2 : 1;
    } flags3;
    struct {
        uint8_t ledExtinguish : 1;
        uint8_t crossTiming : 1;
        uint8_t recentClosing : 1;
        uint8_t reserved : 1;
        uint8_t exitErrorTriggered : 1;
        uint8_t autoHomeInhibited : 1;
        uint8_t sensorLowBattery : 1;
        uint8_t sensorLostSup : 1;
    } flags4;
    uint8_t lastUserNum;
    struct {
        uint8_t zoneBypassed : 1;
        uint8_t forceArmAuto : 1;
        uint8_t readyToArm : 1;
        uint8_t readyToForceArm : 1;
        uint8_t validPinAccepted : 1;
        uint8_t chimeOn : 1;
        uint8_t errorBeep : 1;
        uint8_t toneOn : 1;
    } flags5;
    struct {
        uint8_t entry1 : 1;
        uint8_t openPeriod : 1;
        uint8_t alarmPhone1 : 1;
        uint8_t alarmPhone2 : 1;
        uint8_t alarmPhone3 : 1;
        uint8_t cancelReportStack : 1;
        uint8_t keyswitchArmed : 1;
        uint8_t delayTrip : 1;
    } flags6;
};

struct systemStatusFlags_t {
    struct {
        bool lineSeizure : 1;
        bool offHook : 1;
        bool handshakeReceived : 1;
        bool downloadInProgress : 1;
        bool dialerDelay : 1;
        bool usingBackupPhone : 1;
        bool listenInActive : 1;
        bool twoWayLockout : 1;
    } flags1;
    struct {
        bool groundFault : 1;
        bool phoneFault : 1;
        bool failToCommunicate : 1;
        bool fuseFault : 1;
        bool boxTamper : 1;
        bool sirenTamper : 1;
        bool lowBattery : 1;
        bool acFail : 1;
    } flags2;
    struct {
        bool expBoxTamper : 1;
        bool expAcFailure : 1;
        bool expLowBattery : 1;
        bool expLossSup : 1;
        bool expAuxOverCurrent : 1;
        bool auxCommFail : 1;
        bool expBellFault : 1;
        bool reserved : 1;
    } flags3;
    struct {
        bool pin6DigitEnabled : 1;
        bool tokenInUse : 1;
        bool pinRequiredLocal : 1;
        bool globalBuzzer : 1;
        bool globalSirenOn : 1;
        bool globalSteadySiren : 1;
        bool busLineSeized : 1;
        bool busSniffReq : 1;
    } flags4;
    struct {
        bool dynBatteryTest : 1;
        bool acPowerOn : 1;
        bool lowBatteryMem : 1;
        bool groundFaultMem : 1;
        bool fireAlarmVerify : 1;
        bool smokePowerReset : 1;
        bool line50HzDetected : 1;
        bool hvBatteryCharge : 1;
    } flags5;
    struct {
        bool commSinceAutotest : 1;
        bool powerUpDelay : 1;
        bool walkTestMode : 1;
        bool lossSystemTime : 1;
        bool enrollRequested : 1;
        bool testFixtureMode : 1;
        bool ctrlShutdownMode : 1;
        bool cancelWindow : 1;
    } flags6;
};

struct systemStatusMessage_t {
    uint8_t msgnum;
    uint8_t panelId;
    systemStatusFlags_t flags;
        struct {
        bool reserved : 7;
        bool callBackInProgress : 1;
    } flags9;
    struct {
        bool phoneLineFaulted : 1;
        bool voltageInterrupt : 1;
        bool housePhoneOffHook : 1;
        bool phoneMonitorEnable : 1;
        bool sniffing : 1;
        bool lastReadOffHook : 1;
        bool listenInRequested : 1;
        bool listenInTrigger : 1;
    } flags10;
    uint8_t validPartMask;
};

enum t_event_type : uint8_t {
    Alarm = 0,
    AlarmRestore = 1,
    Bypass = 2,
    BypassRestore = 3,
    Tamper = 4,
    TamperRestore = 5,
    Trouble = 6,
    TroubleRestore = 7,
    TxLowBattery = 8,
    TxLowBatteryRestore = 9,
    ZoneLost = 10,
    ZoneLostRestore = 11,
    StartOfCrossTime = 12,
    // 13-16 not used
    SpecialExpansionEvent = 17,
    Duress = 18,
    ManualFire = 19,
    Auxiliary2Panic = 20,
    // 21 not used
    Panic = 22,
    KeypadTamper = 23,
    ControlBoxTamper = 24,
    ControlBoxTamperRestore = 25,
    AcFail = 26,
    AcFailRestore = 27,
    LowBattery = 28,
    LowBatteryRestore = 29,
    OverCurrent = 30,
    OverCurrentRestore = 31,
    SirenTamper = 32,
    SirenTamperRestore = 33,
    TelephoneFault = 34,
    TelephoneFaultRestore = 35,
    ExpanderTrouble = 36,
    ExpanderTroubleRestore = 37,
    FailToCommunicate = 38,
    LogFull = 39,
    Opening = 40,
    Closing = 41,
    ExitError = 42,
    RecentClosing = 43,
    AutoTest = 44,
    StartProgram = 45,
    EndProgram = 46,
    StartDownload = 47,
    EndDownload = 48,
    Cancel = 49,
    GroundFault = 50,
    GroundFaultRestore = 51,
    ManualTest = 52,
    ClosedWithZonesBypassed = 53,
    StartOfListenIn = 54,
    TechnicianOnSite = 55,
    TechnicianLeft = 56,
    ControlPowerUp = 57,
    // 58�119 not used
    EnrolledDevice = 117, // Non-documented
    ClockSet = 119, // Non-documented
    FirstToOpen = 120,
    LastToClose = 121,
    PinEnteredBit7Set = 122,
    BeginWalkTest = 123,
    EndWalkTest = 124,
    ReExit = 125,
    OutputTrip = 126,
    DataLost = 127
};

enum t_segmentSizeOrDataBank : uint8_t { ss_byte = 0, ss_nibble = 1, flash = 0, ram = 1 };
enum t_readWrite : uint8_t { rw_read = 0, rw_write = 1 };
enum t_segmentOffset : uint8_t { none = 0, eightbytes = 1 };
enum t_dataType : uint8_t { binary = 0, decimal = 1, hex = 2, ascii = 3 };
enum t_locType : uint8_t { logical = 0, physycal = 1};

struct pgmDataRequest_t {
    uint8_t msgnum;
    uint8_t moduleAddr;
    struct {
        uint8_t locationBits8_11 : 4; // Upper logical location
        t_segmentSizeOrDataBank segmentSizeOrDataBank : 1; // 0=byte, 1=nibble
        t_readWrite readWrite : 1; // 0=read, 1=write
        t_segmentOffset segmentOffset : 1; // 0=none, 1=8 bytes
        t_locType locationType : 1;
    } locationInfo;
    uint8_t locationBits0_7;  // lower 8 bits of logical location

    uint16_t getLocation() { return ((uint16_t)locationInfo.locationBits8_11) << 8 | locationBits0_7; };
    void setLocation(uint16_t l) { locationInfo.locationBits8_11 = l >> 8; locationBits0_7 = (uint8_t)(l & 0xFF); };
};

struct pgmDataRW_t {
    uint8_t msgnum;
    uint8_t moduleAddr;
    struct {
        uint8_t locationBits8_11 : 4; // Upper logical location
        t_segmentSizeOrDataBank segmentSizeOrDataBank : 1; // 0=byte, 1=nibble
        t_readWrite readWrite : 1; // 0=read, 1=write
        t_segmentOffset segmentOffset : 1; // 0=none, 1=8 bytes
        t_locType locationType : 1;
    } locationInfo;
    uint8_t locationBits0_7;  // lower 8 bits of logical location
    struct {
        uint8_t numSegments : 5;
        t_dataType dataType : 3; // 0=Binary, 1=Decimal, 2=Hex, 3=ASCII
    } dataInfo;
    uint8_t data[8];

    uint16_t getLocation() const { return ((uint16_t)locationInfo.locationBits8_11) << 8 | locationBits0_7; };
    void setLocation(uint16_t l) { locationInfo.locationBits8_11 = l >> 8; locationBits0_7 = (uint8_t)(l & 0xFF); };
};

struct pin_t {
    struct {
        uint8_t pin1 : 4;
        uint8_t pin2 : 4;
    } pin12;
    struct {
        uint8_t pin3 : 4;
        uint8_t pin4 : 4;
    } pin34;
    struct {
        uint8_t pin5 : 4;
        uint8_t pin6 : 4;
    } pin56;

    void set(uint8_t pin[6]) {
        pin12.pin1 = pin[0];
        pin12.pin2 = pin[1];
        pin34.pin3 = pin[2];
        pin34.pin4 = pin[3];
        pin56.pin5 = pin[4];
        pin56.pin6 = pin[5];
    }

    bool set(String pin) {
        if (pin.length() < 4 || pin.length() > 6)
            return false;
        pin12.pin1 = pin[0] - '0';
        pin12.pin2 = pin[1] - '0';
        pin34.pin3 = pin[2] - '0';
        pin34.pin4 = pin[3] - '0';
        pin56.pin5 = pin[4] - '0';
        pin56.pin6 = pin[5] - '0';
        return true;
    }
};

struct userInfoReply_t {
    uint8_t msgnum;
    uint8_t userNumber;
    pin_t pin;
    union {
        struct {
            bool reserved : 1;
            bool armOnly : 1;
            bool armOnlyClose : 1;
            bool masterProgram : 1;
            bool armDisarm : 1;
            bool bypassEnable : 1;
            bool openCloseReport : 1;
            bool mustBeZero : 1;
        } normal;
        struct {
            bool output1Enable : 1;
            bool output2Enable : 1;
            bool output3Enable : 1;
            bool output4Enable : 1;
            bool armDisarm : 1;
            bool bypassEnable : 1;
            bool openCloseReport : 1;
            bool mustBeOne : 1;
        } output;
    } authorityFlags;
    uint8_t partitionMask;
};

struct userAuth_t {
    uint8_t userNumber;
    union {
        struct {
            bool : 1;
            bool armOnly : 1;
            bool armClose : 1;
            bool masterProgram : 1;
            bool armDisarm : 1;
            bool bypass : 1;
            bool reportEnable : 1;
            bool sentinel : 1;
        } authFlags;
        struct {
            bool output1 : 1;
            bool output2 : 1;
            bool output3 : 1;
            bool output4 : 1;
            bool armDisarm : 1;
            bool bypass : 1;
            bool reportEnable : 1;
            bool sentinel : 1;
        } outputFlags;
    } flags;
    uint8_t partitionMask;
};