// NX584Message.h
// Structures for storing NX584 messages

#include "Config.h"
#include "NXMessageCommon.h"
#include <cstdint>
#include <cstddef>

#ifndef _NX584MESSAGE_h
#define _NX584MESSAGE_h

#pragma pack(push, 1)

struct nx584msg01_t {
    uint8_t msgnum;        
    char firmware[4];        

    struct { 
        bool reserved00 : 1;
        bool iface_config : 1;
        bool reserved02 : 1;
        bool reserved03 : 1;
        bool zoneStatus : 1;
        bool zonesSnapshot : 1;
        bool partitionStatus : 1;
        bool partitionsSnapshot : 1;
    } transitionFlags1;

    struct { 
        bool systemStatus : 1;
        bool x10Received : 1;
        bool logEvent : 1;
        bool keypadMsg : 1;
        bool reserved0C : 1;
        bool reserved0D : 1;
        bool reserved0E : 1;
        bool reserved0H : 1;
    } transitionFlags2;

    struct { 
        bool reserved20 : 1;
        bool ifaceConfigReq : 1;
        bool reserved22 : 1;
        bool zoneNameReq : 1;
        bool zoneStatusReq : 1;
        bool zonesSnapshotReq : 1;
        bool partitionStatusReq : 1;
        bool partitionsSnapshotReq : 1;
    } requestFlags1;

    struct { 
        bool systemStatusReq : 1;
        bool sendX10 : 1;
        bool logEventReq : 1;
        bool sendKeypadText : 1;
        bool keypadTerminalReq : 1;
        bool reserved2D : 1;
        bool reserved2E : 1;
        bool reserved2F : 1;
    } requestFlags2;

    struct { 
        bool programDataReq : 1;
        bool programDataCmd : 1;
        bool userInfoReqPin : 1;
        bool userInfoReqNoPin : 1;
        bool setUserCodePin : 1;
        bool setUserCodeNoPin : 1;
        bool setUserAuthPin : 1;
        bool setUserAuthNoPin : 1;
    } requestFlags3;

    struct { 
        bool reserved38 : 1;
        bool reserved39 : 1;
        bool storeCommEvent : 1;
        bool setClock : 1;
        bool primaryKeypadPin : 1;
        bool primaryKeypadNoPin : 1;
        bool secondaryKeypad : 1;
        bool zoneBypassToggle : 1;
    } requestFlags4;
};

struct nx584msg03_t {
    uint8_t msgnum;      
    uint8_t zoneNum;  
    char zoneName[16];    
};

struct nx584msg04_t {
    uint8_t msgnum;      
    uint8_t zoneNum;      
    uint8_t partition_mask;    

    struct {
        bool fire : 1; 
        bool hour24 : 1;
        bool keyswitch : 1; 
        bool follower : 1;
        bool entryExit1 : 1;
        bool entryExit2 : 1; 
        bool interior : 1; 
        bool localOnly : 1;
    } zoneTypeFlags1;

    struct {
        bool keypad_sounder : 1; 
        bool yelpingSiren : 1;
        bool steadySiren : 1;
        bool chime : 1;
        bool bypassable : 1;
        bool groupBypassable : 1; 
        bool forceArmable : 1; 
        bool entryGuard : 1;
    } zoneTypeFlags2;

    struct {
        bool fastLoop : 1; 
        bool doubleEOLTamper : 1; 
        bool trouble : 1;
        bool crossZone : 1;
        bool dialerDelay : 1; 
        bool swingerShutdown : 1;
        bool restorable : 1;
        bool listenIn : 1;
    } zoneTypeFlags3;

    struct {
        bool faulted : 1; 
        bool tampered : 1; 
        bool trouble : 1;
        bool bypassed : 1;
        bool inhibited : 1; 
        bool lowBattery : 1; 
        bool lossSupervision : 1;
    } conditionFlags1;

    struct { 
        bool alarmMemory : 1; 
        bool bypassMemory : 1;
    } conditionFlags2;
};

struct two_zones_status_t {
    bool faultedA : 1;
    bool bypassedA : 1;
    bool troubleA : 1;
    bool alarmMemoryA : 1;
    bool faultedB : 1;
    bool bypassedB : 1;
    bool troubleB : 1;
    bool alarmMemoryB : 1;
};

struct nx584msg05_t {
    uint8_t msgnum;       
    uint8_t zoneOffset;       
    two_zones_status_t zoneStatus[8];
};

typedef partitionStatusMessage_t nx584msg06_t;

struct partition_flags_t {
    bool validPartition : 1;
    bool ready : 1;
    bool armed : 1;
    bool stayMode : 1;
    bool chimeMode : 1;
    bool anyEntryDelay : 1;
    bool anyExitDelay : 1;
    bool previousAlarm : 1;
};

struct nx584msg07_t {
    uint8_t msgnum;          // 0x07
    partition_flags_t part[8];
};

struct nx584msg08_t {
    systemStatusMessage_t ss;
    uint8_t commStackPointer;
};

struct nx584msg0A_t {
    uint8_t msgnum;
    uint8_t logNumber;
    uint8_t logSize; // Always 0xB9;
    struct {
        t_event_type event : 7;
        bool reportingevent : 1;
    };
    uint8_t itemNumber;
    uint8_t partition;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute : 6;
};

typedef pgmDataRW_t nx584msg10_t;

typedef userInfoReply_t nx584msg12_t;

struct nx584msg1C_t {
    uint8_t msgnum;
};

struct nx584msg1D_t {
    uint8_t msgnum;
};

struct nx584msg1E_t {
    uint8_t msgnum;
};

struct nx584msg1F_t {
    uint8_t msgnum;
};

struct nx584msg21_t {
    uint8_t msgnum;
};

struct nx584msg23_t {
    uint8_t msgnum;
    uint8_t zone;
};

struct nx584msg24_t {
    uint8_t msgnum;
    uint8_t zone;
};

struct nx584msg25_t {
    uint8_t msgnum;
    uint8_t zoneOffset;
};

struct nx584msg26_t {
    uint8_t msgnum;
    uint8_t partition;
};

struct nx584msg27_t {
    uint8_t msgnum;
};

struct nx584msg28_t {
    uint8_t msgnum;
};

struct nx584msg2A_t {
    uint8_t msgnum;
    uint8_t eventNum;
};

typedef pgmDataRequest_t nx584msg30_t;

typedef pgmDataRW_t nx584msg31_t;

struct nx584msg32_t {
    uint8_t msgnum;
    pin_t masterPin;
    uint8_t user;
};

struct nx584msg33_t {
    uint8_t msgnum;
    uint8_t user;
};

struct nx584msg34_t {
    uint8_t msgnum;
    pin_t masterPin;
    uint8_t user;
    pin_t newPin;
};

struct nx584msg35_t {
    uint8_t msgnum;
    uint8_t user;
    pin_t newPin;
};

struct nx584msg36_t {
    uint8_t msgnum;
    pin_t masterPin;
    userAuth_t auth;
};

struct nx584msg37_t {
    uint8_t msgnum;
    userAuth_t auth;
};

struct nx584msg3A_t {
    uint8_t msgnum;
    uint8_t partition;
    struct {
        t_event_type event : 7;
        bool reportingevent : 1;
    };
    uint8_t itemNumber;
    uint8_t data1;
    uint8_t data2;
};

struct nx584msg3B_t {
    uint8_t msgnum;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t weekDay;
};

enum class primaryKpadFunc : uint8_t {
    sounderOff      = 0x00,
    disarm          = 0x01,
    armAway         = 0x02,
    armStay         = 0x03,
    cancel          = 0x04,
    autoArm         = 0x05,
    walkTestStart   = 0x06,
    walkTestStop    = 0x07
};

struct nx584msg3C_t {
    uint8_t msgnum;
    pin_t pin;
    primaryKpadFunc function;
    uint8_t partitionMask;
};

struct nx584msg3D_t {
    uint8_t msgnum;
    primaryKpadFunc function;
    uint8_t partitionMask;
    uint8_t user;
};

enum class secondKpadFunc : uint8_t {
    Stay                 = 0x00, 
    Chime                = 0x01, 
    Exit                 = 0x02, 
    BypassInteriors      = 0x03,
    FirePanic            = 0x04,
    MedicalPanic         = 0x05,
    PolicePanic          = 0x06,
    SmokeDetectorReset   = 0x07,
    AutoCallbackDownload = 0x08,
    ManualPickupDownload = 0x09,
    SilentExitEnable     = 0x0A, 
    PerformTest          = 0x0B,
    GroupBypass          = 0x0C,
    AuxiliaryFunction1   = 0x0D,
    AuxiliaryFunction2   = 0x0E,
    StartKeypadSounder   = 0x0F
};

struct nx584msg3E_t {
    uint8_t msgnum;
    secondKpadFunc function;
    uint8_t partitionMask;
};

struct nx584msg3F_t {
    uint8_t msgnum;
    uint8_t zone;
};


#pragma pack(pop)


class NX584Message
{
 public:
	 union {
		 uint8_t bytes[MAX_PACKET_SIZE];
		 nx584msg01_t msg01;
         nx584msg03_t msg03;
         nx584msg04_t msg04;
         nx584msg05_t msg05;
         nx584msg06_t msg06;
         nx584msg07_t msg07;
         nx584msg08_t msg08;
         nx584msg0A_t msg0A;
         nx584msg10_t msg10;
         nx584msg12_t msg12;
         nx584msg1C_t msg1C;
         nx584msg1D_t msg1D;
         nx584msg1E_t msg1E;
         nx584msg1F_t msg1F;
         nx584msg21_t msg21;
         nx584msg23_t msg23;
         nx584msg24_t msg24;
         nx584msg25_t msg25;
         nx584msg26_t msg26;
         nx584msg27_t msg27;
         nx584msg28_t msg28;
         nx584msg2A_t msg2A;
         nx584msg30_t msg30;
         nx584msg31_t msg31;
         nx584msg32_t msg32;
         nx584msg33_t msg33;
         nx584msg34_t msg34;
         nx584msg35_t msg35;
         nx584msg36_t msg36;
         nx584msg37_t msg37;
         nx584msg3A_t msg3A;
         nx584msg3B_t msg3B;
         nx584msg3C_t msg3C;
         nx584msg3D_t msg3D;
         nx584msg3E_t msg3E;
         nx584msg3F_t msg3F;
	 } data;

	 uint8_t msgNum() const {
		 return (data.msg01.msgnum & 0b01111111);
	 }

	 uint8_t msgLen() {
		 return msgLen(data.msg01.msgnum);
	 }

	 static uint8_t msgLen(uint8_t msg_num);

};


#endif

