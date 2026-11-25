// WebUI.h
// Web user interface based on CRMui3 library which also provides configuration storage

#include "CRMui3.h"
#include "Config.h"

#ifndef _WEBUI_h
#define _WEBUI_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

namespace WebUI
{

	void crmLoop(void* arg);
	void cfgLoad();
	void cfgDelete();
	void start();
	void onInterface();
	void onVarChange();
	void updateFull();
	void updateStat();
	void notify(const String& type, const String& text, long tout = 5, bool x = true);
	void btnResponse(const char* name);
	String getID();
	String configKeyLoad(const String& name, const String& default_val);
};


#endif

