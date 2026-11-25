// NXMessage.h
// Structures for storing NX bus messages

#include "Config.h"
#include "NXMessageCommon.h"
#include <cstdint>
#include <cstddef>
#include "esp_attr.h"

#ifndef _NXMESSAGE_h
#define _NXMESSAGE_h

#pragma pack(push, 1)

struct msg00_t {
    uint8_t msgnum;
    systemStatusFlags_t flags;
};

struct msg01_t {
	uint8_t msgnum;
	uint8_t panelID;
	uint8_t logsize;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t weekday : 4;
	uint8_t month : 4;
};

struct msg02_t {
	uint8_t msgnum;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
};

struct msg04_t {
	uint8_t msgnum;
	uint8_t zoneFault[6];
};

struct msg05_t {
	uint8_t msgnum;
	uint8_t zoneTamper[6];
};

struct msg06_t {
	uint8_t msgnum;
	uint8_t zoneTrouble[6];
};

struct msg07_t {
	uint8_t msgnum;
	uint8_t zoneBypass[6];
};

struct msg08_t {
	uint8_t msgnum;
	uint8_t zoneAlarmMem[6];
};

struct msg09_t {
	uint8_t msgnum;
	uint8_t zoneInhibit[6];
};

struct msg0A_t {
	uint8_t msgnum;
	uint8_t zoneLowBat[6];
};

struct msg0B_t {
	uint8_t msgnum;
	uint8_t zoneLost[6];
};

struct msg0C_t {
	uint8_t msgnum;
	uint8_t zoneBypassMem[6];
};

struct msg0D_t {
	uint8_t msgnum;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;

};
struct msg10_t {
	uint8_t msgnum;
};

struct msg16_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t unk1;
    uint8_t unk2;
    uint8_t unk3;
    uint8_t commStackPointer;
};

typedef partitionStatusMessage_t msg18_t;

typedef systemStatusMessage_t msg19_t;

struct msg1B_t {
    uint8_t msgnum;
    uint8_t unk[4];
};

typedef pgmDataRequest_t msg20_t;

struct msg21_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t itemNumber;
};

struct msg22_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t itemNumber;
};

struct msg23_t {
    uint8_t msgnum;
    uint8_t id1;
    uint8_t id2;
};

struct msg24_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t panelID;
};

struct msg40_t {
    uint8_t msgnum;
};

struct msg41_t {
    uint8_t msgnum;
};

struct msg44_t {
    uint8_t msgnum;
    uint8_t nul;
};

struct msg45_t {
    uint8_t msgnum;
    uint8_t moduleAddress;
    uint8_t unk1;
    uint8_t unk2;
};

struct msg50_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    struct {
        bool authRequested : 1;
        uint8_t unk : 7;
    } flags;
    uint8_t partitionMask;
    pin_t pin;
};

enum class kpd_function : uint8_t {
    stay               = 0x00, 
    chime              = 0x01,
    exit               = 0x02, 
    bypassInteriors    = 0x03,
    cancel             = 0x04,
    firePanic          = 0x05,
    policePanic        = 0x06,
    medicalPanic       = 0x07,
    smokeDetectorReset = 0x08,
    callback           = 0x09,
    download           = 0x0A, 
    enterProgramMode   = 0x0B,
    exitProgramMode    = 0x0C,
    silentExit         = 0x0D,
    test               = 0x0E,
    groupBypass        = 0x0F,
    disarm             = 0x10,
    armAway            = 0x11,
    armStay            = 0x12,
    auxillary1         = 0x13,
    auxillary2         = 0x14,
    pinEntered         = 0x15,
    autoArm            = 0x16,
    buzzerOn           = 0x17,
    walkTest           = 0x18,
    autoBypassCancel   = 0x19
};

struct msg51_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    kpd_function function;
    uint8_t partitionMask;
};

struct msg52_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t zone;
};

struct msg53_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t userNumber;
    uint8_t unk; // FF
    pin_t pin;
};

struct msg54_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    userAuth_t auth;
};

struct msg55_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t userNumber;
};

struct msg5C_t {
    uint8_t msgnum;
    uint8_t sourceAddr;
    uint8_t partition;
    struct {
        t_event_type event : 7;
        bool reportingevent : 1;
    };
    uint8_t itemNumber;
    uint8_t data1;
    uint8_t data2;
};

typedef pgmDataRW_t msg60_t;

struct msg61_t {
    uint8_t msgnum;
    uint8_t logNumber;
    uint8_t logSize; // Always 0xB9;
    struct {
        t_event_type event : 7;
        bool reportingevent : 1;
    };
    uint8_t itemNumber;
    uint8_t hour : 5;
    uint8_t month_low : 3;
    uint8_t month_high : 1;
    uint8_t day : 5;
    uint8_t partition_high : 2;
    uint8_t minute : 6;
    uint8_t partition_low : 2;

    uint8_t month() const { return month_high << 3 | month_low; }
    uint8_t partition() const { return partition_high << 2 | partition_low; }
};

struct msg62_t {
    uint8_t msgnum;
    uint8_t CIDEventQualifier : 4;
    struct {
        uint8_t code : 4;
    } CIDEventCode[3];
    uint8_t SIACode[2];
    uint8_t unk1;
    uint8_t zone;
    uint8_t unk2;
};

struct msg63_t {
    uint8_t msgnum;
    uint8_t id1;
    uint8_t id2;
    uint8_t unk[5];
};

typedef userInfoReply_t msg70_t;

#pragma pack(pop)

class NXMessage
{
 public:
	 union {
		 uint8_t bytes[MAX_PACKET_SIZE];
		 msg00_t msg00;
		 msg01_t msg01;
		 msg02_t msg02;
		 msg04_t msg04;
		 msg05_t msg05;
		 msg06_t msg06;
		 msg07_t msg07;
		 msg08_t msg08;
		 msg09_t msg09;
		 msg0A_t msg0A;
		 msg0B_t msg0B;
		 msg0C_t msg0C;
		 msg0D_t msg0D;
		 msg10_t msg10;
         msg16_t msg16;
		 msg18_t msg18;
		 msg19_t msg19;
         msg1B_t msg1B;
         msg20_t msg20;
         msg21_t msg21;
         msg22_t msg22;
         msg23_t msg23;
         msg24_t msg24;
         msg40_t msg40;
         msg41_t msg41;
         msg44_t msg44;
		 msg45_t msg45;
         msg50_t msg50;
         msg51_t msg51;
         msg52_t msg52;
         msg53_t msg53;
         msg54_t msg54;
         msg55_t msg55;
         msg5C_t msg5C;
         msg60_t msg60;
         msg61_t msg61;
         msg62_t msg62;
         msg63_t msg63;
         msg70_t msg70;
	 } data;

	 uint8_t dest;

     uint8_t length = 0;

	 uint8_t msgNum() const {
		 return (data.msg00.msgnum & 0b01111111);
	 }

	 uint8_t IRAM_ATTR msgLen() const {
        return length > 0 ? length : msgLen(data.msg00.msgnum);
	 }

	 static uint8_t IRAM_ATTR msgLen(uint8_t msg_num);
};

#endif
