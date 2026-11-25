// NXPanelFunctions.h
// Common functions of NX panel

#ifndef _NXPANELFUNCTIONS_h
#define _NXPANELFUNCTIONS_h

#include "Config.h"
#include "NXMessage.h"
#include "NXMessageCommon.h"
#include "CommonStructures.h"

//Forward declaration
class NXPanel;
enum requestResult : uint8_t;
enum requestResult : uint8_t;

enum checkPinAuthReq {authAny, authArm, authDisarm, authBypass, authMaster, authProgram};
enum checkPinResult {pinOK, pinNoUser, pinNoAuth, pinCommErr};

class NXPanelFunctions
{
 friend class NXPanel;
 protected:
    NXPanel* panel;

 public:
    constexpr explicit NXPanelFunctions(NXPanel* _p) : 
         panel(_p) {};
 	requestResult nxGetSystemStatus();
	requestResult nxKpdFunction(kpd_function fun, uint8_t partition);
	requestResult nxBypass(uint8_t zone);
	requestResult nxSetDateTime(dateTime dt);
	requestResult nxGetLogRecord(uint8_t id, NXLogRecord* record);
	checkPinResult checkPin(pin_t pin, uint8_t partition, checkPinAuthReq requiredAuth);
};

extern NXPanel panel;

#endif