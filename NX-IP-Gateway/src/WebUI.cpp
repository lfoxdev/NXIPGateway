// 
// 
// 

#include "Config.h"
#include "WebUI.h"
#include "NXPanel.h"
#include "MQTT.h"
#include "DCUI.h"
#include "NX584.h"
#include "Utils.h"
#include "NXPanelFunctions.h"
#include "DL900.h"

#define LOG_TAG "UI"

#define NDEF F("?")
#define NCLR F("#f66")
#define YCLR F("#1c1")

extern NXPanel panel;

CRMui3 crm;

TaskHandle_t logLoadTask = NULL;

// T = element type (e.g. PartitionStatus)
// N = array size
// Member = pointer-to-member of type T
template <typename T, std::size_t N, typename Member>
String partStateToString(const std::array<T, N>& arr, Member member)
{
	String result;
	result.reserve(N*2);

	for (size_t i = 0; i < N; ++i) {
		const auto& item = arr[i].*member;
		if (item.get())
			result += String(i + 1) + " ";
		else
			result += "- ";
	}
	return result;
}

String bToS(bool val) {
	return val ? F("<c style='color:#1c1'>YES</c>") : F("NO");
}

void selLoadTask(void* pvParameters) {
	String sel;
	int count = (int)pvParameters;
	uint8_t start, end; 
	if (count == 0) {
		start = 0;
		end = panel.state.logSize;
	}
	else {
		start = max(0, (int)(panel.state.logHead) - count + 1);
		end = panel.state.logHead;
	}
	ESP_LOGI(LOG_TAG, "Reading log from %d to %d", end, start);		
	for (uint8_t id = 0; id <= panel.state.logSize; id++) {
		crm.webUpdate("sel" + String(id), "");
		vTaskDelay(1 / portTICK_RATE_MS);
	}
	uint8_t row = 0;
	for (uint8_t id = start; id <= end; id++) {
		//ESP_LOGI(LOG_TAG, "Reading SEL entry %d", id);
		NXLogRecord r;
		if (panel.fun.nxGetLogRecord(id, &r) == rqOK) {
			char buf[256];
			sprintf(buf, "<pre style=\\\"font-family:monospace\\\">[%03d] %02d.%02d %02d:%02d P%d %s%s %s (cid %d %03d sia %c%c)</pre>", id, r.day, r.month, r.hour, r.minute, r.partition + 1, (r.reporting ? "*" : " "), r.eventStr().c_str(), r.itemStr().c_str(), r.cid_qualifier, r.cid_code, sanitize(r.sia[0]), sanitize(r.sia[1]));
			sel = String(buf);
		}
		else
			sel = F("<pre style=\\\"font-family:monospace\\\">Error fetching log item</pre>");
		ESP_LOGI(LOG_TAG, "SEL entry %s", sel.c_str());
		crm.webUpdate("sel" + String(row++), sel);
		vTaskDelay(10 / portTICK_RATE_MS);
	}
	crm.webNotif("i", F("Reading log completed"), 20, 1);
	logLoadTask = NULL;
	vTaskDelete(NULL);
}


void WebUI::crmLoop(void* arg)
{
	crm.run();
}

void WebUI::cfgLoad()
{
	crm.cfgLoad();
}

void WebUI::cfgDelete()
{
	crm.cfgDelete();
}

void WebUI::start()
{
	//crm.useArduinoOta();
	crm.btnCallback(btnResponse);
	crm.begin("NetworX IP Gateway", onInterface, onVarChange);
	crm.contacts("", "", "https://github.com/lfoxdev/NXIPGateway");
	crm.version(VERSION);
	if (Config.password != "")
		crm.setWebAuth("admin", Config.password);
}

void WebUI::onVarChange()
{
	bool dbge = (crm.var("c_dbg") == "1");
	bool dcuie = (crm.var("c_dcui") == "1");
	bool nx584e = (crm.var("c_nx584") == "1");
	bool dl900e = (crm.var("c_dl900") == "1");
	if (dbge && !Config.dbgEnabled)
		panel.enableDebug();
	else if (!dbge && Config.dbgEnabled)
		panel.disableDebug();
	if (dcuie && !Config.dcuiEnabled)
		dcui.start();
	else if (!dcuie && Config.dcuiEnabled)
		dcui.stop();
	if (nx584e && !Config.nx584Enabled)
		nx584.start();
	else if (!nx584e && Config.nx584Enabled)
		nx584.stop();
	if (dl900e && !Config.dl900Enabled)
		dl900.start();
	else if (!dl900e && Config.dl900Enabled)
		dl900.stop();
	Config.dbgEnabled = dbge;
	Config.dcuiEnabled = dcuie;
	Config.nx584Enabled = nx584e;
	Config.dl900Enabled = dl900e;
}

void WebUI::updateFull()
{
	if (!crm.webConnStatus())
		return;
	crm.webUpdate("mqttok", MQTT.connected() ? "MQTT status: <c style='color:green'>connected</c>" : "MQTT broker: <c style='color:red'>disconnected</c> (" + MQTT.error + ")");

	crm.webUpdate("logHead", String(panel.state.logHead) + " (0x" + String(panel.state.logHead, HEX) + ")");

	crm.webUpdate("lineSeizure", bToS(panel.state.lineSeizure.get()));
	crm.webUpdate("offHook", bToS(panel.state.offHook.get()));
	crm.webUpdate("handshakeReceived", bToS(panel.state.handshakeReceived.get()));
	crm.webUpdate("downloadInProgress", bToS(panel.state.downloadInProgress.get()));
	crm.webUpdate("dialerDelay", bToS(panel.state.dialerDelay.get()));
	crm.webUpdate("usingBackupPhone", bToS(panel.state.usingBackupPhone.get()));
	crm.webUpdate("listenInActive", bToS(panel.state.listenInActive.get()));
	crm.webUpdate("twoWayLockout", bToS(panel.state.twoWayLockout.get()));
	crm.webUpdate("groundFault", bToS(panel.state.groundFault.get()));
	crm.webUpdate("phoneFault", bToS(panel.state.phoneFault.get()));
	crm.webUpdate("failToCommunicate", bToS(panel.state.failToCommunicate.get()));
	crm.webUpdate("fuseFault", bToS(panel.state.fuseFault.get()));
	crm.webUpdate("boxTamper", bToS(panel.state.boxTamper.get()));
	crm.webUpdate("sirenTamper", bToS(panel.state.sirenTamper.get()));
	crm.webUpdate("lowBattery", bToS(panel.state.lowBattery.get()));
	crm.webUpdate("acFail", bToS(panel.state.acFail.get()));
	crm.webUpdate("expBoxTamper", bToS(panel.state.expBoxTamper.get()));
	crm.webUpdate("expAcFailure", bToS(panel.state.expAcFailure.get()));
	crm.webUpdate("expLowBattery", bToS(panel.state.expLowBattery.get()));
	crm.webUpdate("expLossSup", bToS(panel.state.expLossSup.get()));
	crm.webUpdate("expAuxOverCurrent", bToS(panel.state.expAuxOverCurrent.get()));
	crm.webUpdate("auxCommFail", bToS(panel.state.auxCommFail.get()));
	crm.webUpdate("expBellFault", bToS(panel.state.expBellFault.get()));
	crm.webUpdate("pin6DigitEnabled", bToS(panel.state.pin6DigitEnabled.get()));
	crm.webUpdate("tokenInUse", bToS(panel.state.tokenInUse.get()));
	crm.webUpdate("pinRequiredLocal", bToS(panel.state.pinRequiredLocal.get()));
	crm.webUpdate("globalBuzzer", bToS(panel.state.globalBuzzer.get()));
	crm.webUpdate("globalSirenOn", bToS(panel.state.globalSirenOn.get()));
	crm.webUpdate("globalSteadySiren", bToS(panel.state.globalSteadySiren.get()));
	crm.webUpdate("busLineSeized", bToS(panel.state.busLineSeized.get()));
	crm.webUpdate("busSniffReq", bToS(panel.state.busSniffReq.get()));
	crm.webUpdate("dynBatteryTest", bToS(panel.state.dynBatteryTest.get()));
	crm.webUpdate("acPowerOn", bToS(panel.state.acPowerOn.get()));
	crm.webUpdate("lowBatteryMem", bToS(panel.state.lowBatteryMem.get()));
	crm.webUpdate("groundFaultMem", bToS(panel.state.groundFaultMem.get()));
	crm.webUpdate("fireAlarmVerify", bToS(panel.state.fireAlarmVerify.get()));
	crm.webUpdate("smokePowerReset", bToS(panel.state.smokePowerReset.get()));
	crm.webUpdate("line50HzDetected", bToS(panel.state.line50HzDetected.get()));
	crm.webUpdate("hvBatteryCharge", bToS(panel.state.hvBatteryCharge.get()));
	crm.webUpdate("commSinceAutotest", bToS(panel.state.commSinceAutotest.get()));
	crm.webUpdate("powerUpDelay", bToS(panel.state.powerUpDelay.get()));
	crm.webUpdate("walkTestMode", bToS(panel.state.walkTestMode.get()));
	crm.webUpdate("lossSystemTime", bToS(panel.state.lossSystemTime.get()));
	crm.webUpdate("enrollRequested", bToS(panel.state.enrollRequested.get()));
	crm.webUpdate("testFixtureMode", bToS(panel.state.testFixtureMode.get()));
	crm.webUpdate("ctrlShutdownMode", bToS(panel.state.ctrlShutdownMode.get()));
	crm.webUpdate("cancelWindow", bToS(panel.state.cancelWindow.get()));
	crm.webUpdate("callBackInProgress", bToS(panel.state.callBackInProgress.get()));
	crm.webUpdate("phoneLineFaulted", bToS(panel.state.phoneLineFaulted.get()));
	crm.webUpdate("voltageInterrupt", bToS(panel.state.voltageInterrupt.get()));
	crm.webUpdate("housePhoneOffHook", bToS(panel.state.housePhoneOffHook.get()));
	crm.webUpdate("phoneMonitorEnable", bToS(panel.state.phoneMonitorEnable.get()));
	crm.webUpdate("sniffing", bToS(panel.state.sniffing.get()));
	crm.webUpdate("lastReadOffHook", bToS(panel.state.lastReadOffHook.get()));
	crm.webUpdate("listenInRequested", bToS(panel.state.listenInRequested.get()));
	crm.webUpdate("listenInTrigger", bToS(panel.state.listenInTrigger.get()));

	crm.webUpdate("partValid", partStateToString(panel.state.partitions, &PartitionStatus::validPart));
	crm.webUpdate("bypassCodeRequired", partStateToString(panel.state.partitions, &PartitionStatus::bypassCodeRequired));
	crm.webUpdate("fireTrouble", partStateToString(panel.state.partitions, &PartitionStatus::fireTrouble));
	crm.webUpdate("fire", partStateToString(panel.state.partitions, &PartitionStatus::fire));
	crm.webUpdate("pulsingBuzzer", partStateToString(panel.state.partitions, &PartitionStatus::pulsingBuzzer));
	crm.webUpdate("tlmFaultMemory", partStateToString(panel.state.partitions, &PartitionStatus::tlmFaultMemory));
	crm.webUpdate("armed", partStateToString(panel.state.partitions, &PartitionStatus::armed));
	crm.webUpdate("instant", partStateToString(panel.state.partitions, &PartitionStatus::instant));
	crm.webUpdate("previousAlarm", partStateToString(panel.state.partitions, &PartitionStatus::previousAlarm));
	crm.webUpdate("sirenOn", partStateToString(panel.state.partitions, &PartitionStatus::sirenOn));
	crm.webUpdate("steadySirenOn", partStateToString(panel.state.partitions, &PartitionStatus::steadySirenOn));
	crm.webUpdate("alarmMemory", partStateToString(panel.state.partitions, &PartitionStatus::alarmMemory));
	crm.webUpdate("tamper", partStateToString(panel.state.partitions, &PartitionStatus::tamper));
	crm.webUpdate("cancelCmdEntered", partStateToString(panel.state.partitions, &PartitionStatus::cancelCmdEntered));
	crm.webUpdate("codeEntered", partStateToString(panel.state.partitions, &PartitionStatus::codeEntered));
	crm.webUpdate("cancelPending", partStateToString(panel.state.partitions, &PartitionStatus::cancelPending));
	crm.webUpdate("silentExitEnabled", partStateToString(panel.state.partitions, &PartitionStatus::silentExitEnabled));
	crm.webUpdate("entryGuard", partStateToString(panel.state.partitions, &PartitionStatus::entryGuard));
	crm.webUpdate("chimeModeOn", partStateToString(panel.state.partitions, &PartitionStatus::chimeModeOn));
	crm.webUpdate("entry", partStateToString(panel.state.partitions, &PartitionStatus::entry));
	crm.webUpdate("delayExpiration", partStateToString(panel.state.partitions, &PartitionStatus::delayExpiration));
	crm.webUpdate("exit1", partStateToString(panel.state.partitions, &PartitionStatus::exit1));
	crm.webUpdate("exit2", partStateToString(panel.state.partitions, &PartitionStatus::exit2));
	crm.webUpdate("ledExtinguish", partStateToString(panel.state.partitions, &PartitionStatus::ledExtinguish));
	crm.webUpdate("crossTiming", partStateToString(panel.state.partitions, &PartitionStatus::crossTiming));
	crm.webUpdate("recentClosing", partStateToString(panel.state.partitions, &PartitionStatus::recentClosing));
	crm.webUpdate("exitErrorTriggered", partStateToString(panel.state.partitions, &PartitionStatus::exitErrorTriggered));
	crm.webUpdate("autoHomeInhibited", partStateToString(panel.state.partitions, &PartitionStatus::autoHomeInhibited));
	crm.webUpdate("sensorLowBattery", partStateToString(panel.state.partitions, &PartitionStatus::sensorLowBattery));
	crm.webUpdate("sensorLostSup", partStateToString(panel.state.partitions, &PartitionStatus::sensorLostSup));
	crm.webUpdate("zoneBypassed", partStateToString(panel.state.partitions, &PartitionStatus::zoneBypassed));
	crm.webUpdate("forceArmAuto", partStateToString(panel.state.partitions, &PartitionStatus::forceArmAuto));
	crm.webUpdate("readyToArm", partStateToString(panel.state.partitions, &PartitionStatus::readyToArm));
	crm.webUpdate("readyToForceArm", partStateToString(panel.state.partitions, &PartitionStatus::readyToForceArm));
	crm.webUpdate("validPinAccepted", partStateToString(panel.state.partitions, &PartitionStatus::validPinAccepted));
	crm.webUpdate("chimeOn", partStateToString(panel.state.partitions, &PartitionStatus::chimeOn));
	crm.webUpdate("errorBeep", partStateToString(panel.state.partitions, &PartitionStatus::errorBeep));
	crm.webUpdate("toneOn", partStateToString(panel.state.partitions, &PartitionStatus::toneOn));
	crm.webUpdate("entry1", partStateToString(panel.state.partitions, &PartitionStatus::entry1));
	crm.webUpdate("openPeriod", partStateToString(panel.state.partitions, &PartitionStatus::openPeriod));
	crm.webUpdate("alarmPhone1", partStateToString(panel.state.partitions, &PartitionStatus::alarmPhone1));
	crm.webUpdate("alarmPhone2", partStateToString(panel.state.partitions, &PartitionStatus::alarmPhone2));
	crm.webUpdate("alarmPhone3", partStateToString(panel.state.partitions, &PartitionStatus::alarmPhone3));
	crm.webUpdate("cancelReportStack", partStateToString(panel.state.partitions, &PartitionStatus::cancelReportStack));
	crm.webUpdate("keyswitchArmed", partStateToString(panel.state.partitions, &PartitionStatus::keyswitchArmed));
	crm.webUpdate("delayTrip", partStateToString(panel.state.partitions, &PartitionStatus::delayTrip));
	String lastUserNum;
	lastUserNum.reserve(40);
	for (size_t i = 0; i < MAX_PARTITION; i++) {
		lastUserNum += String(panel.state.partitions[i].lastUserNum.toString()) + ", ";
	}
	lastUserNum.remove(lastUserNum.length() - 2, 2);
	crm.webUpdate("lastUserNum", lastUserNum);
	for (size_t i = 0; i < MAX_ZONE; i++) {
		String result;
		result.reserve(22);
		result += panel.state.zones[i].fault.get()        ? "F  " : "-  ";
		result += panel.state.zones[i].tamper.get()       ? "Ta " : "-  ";
		result += panel.state.zones[i].trouble.get()      ? "Tr " : "-  ";
		result += panel.state.zones[i].bypass.get()       ? "B  " : "-  ";
		result += panel.state.zones[i].alarmMemory.get()  ? "Am " : "-  ";
		result += panel.state.zones[i].inhibit.get()      ? "I  " : "-  ";
		result += panel.state.zones[i].lowBattery.get()   ? "Lb " : "-  ";
		result += panel.state.zones[i].lost.get()         ? "L  " : "-  ";
		result += panel.state.zones[i].bypassMemory.get() ? "Bm " : "-  ";
		crm.webUpdate(String("zone") + String(i + 1), result);
	}

}

void WebUI::updateStat()
{
	if (!crm.webConnStatus())
		return;
	NXBusCounters counters = panel.getBusCounters();
	uint32_t last_poll = millis() - counters.last_poll_ms;
	crm.webUpdate("enrolled", bToS(last_poll < 10000));
	crm.webUpdate("received", String(counters.received));
	crm.webUpdate("tooShort", String(counters.too_short));
	crm.webUpdate("bufferOverflows", String(counters.buffer_overflows));
	crm.webUpdate("incorrectChecksums", String(counters.incorrect_checksums));
	crm.webUpdate("lostCycles", String(counters.lost_cycles));
	crm.webUpdate("queueOverflows", String(counters.queue_overflows));
	crm.webUpdate("sent", String(counters.sent));
	crm.webUpdate("retries", String(counters.send_retries));
	crm.webUpdate("lastPoll", String(last_poll));

	const char* weekdayNames[] = { "", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
	char buf[25];
	sprintf(buf, "%s %02d.%02d %02d:%02d:%02d", weekdayNames[panel.state.weekday % 8], panel.state.day, panel.state.month, panel.state.hour, panel.state.minute, panel.state.second);
	crm.webUpdate("time", String(buf));
}

void WebUI::notify(const String& type, const String& text, long tout, bool x)
{
	crm.webNotif(type, text, tout, x);
}

void WebUI::btnResponse(const char* name)
{
	if (strcmp(name, "savecfg") == 0) {
		crm.webNotif("i", F("Rebooting..."), 5, 1);
		crm.espReboot();
	} else
	// Setting date & time
	if (strcmp(name, "sd_set") == 0) {
		String dateStr = String(crm.var("sd_date").c_str());
		String timeStr = String(crm.var("sd_time").c_str());
		String weekdayStr = String(crm.var("sd_weekday").c_str());		
		dateTime dt;
		if (sscanf(dateStr.c_str(), "%4d-%2d-%2d", &dt.year, &dt.month, &dt.day) == 3 &&
			sscanf(timeStr.c_str(), "%2d:%2d:%2d", &dt.hour, &dt.minute, &dt.second) == 3 &&
			sscanf(weekdayStr.c_str(), "%d", &dt.weekday) == 1) {
			crm.webNotif("i", requestResultToStr(panel.fun.nxSetDateTime(dt)), 5, 1);
		}
		else {
			crm.webNotif("e", F("Incorrect date or time"), 5, 1);
		}
		crm.webUpdate("sd_date", "01.01.2000");
		crm.webUpdate("sd_time", "00:00:00");
		crm.webUpdate("sd_weekday", "1");
	} else
	// Reading event log
	if (strstr(name, "readsel") != 0) {
		int count = 0;
		if (strcmp(name, "readsel10") == 0)
			count = 10;
		if (strcmp(name, "readsel50") == 0)
			count = 50;
		if (logLoadTask == NULL) {
			crm.webNotif("i", F("Reading log, please wait..."), 5, 1);
			xTaskCreate(selLoadTask, "selLoadTask", 4096, (void*)count, tskIDLE_PRIORITY + 1, &logLoadTask);
		}
		else
			crm.webNotif("e", F("Task is already running"), 5, 1);
	}
	// Partition actions
	else if (strcmp(name, "a_armaway") == 0)
		crm.webNotif("i", requestResultToStr(panel.fun.nxKpdFunction(kpd_function::armAway, crm.var("a_part").toInt())), 5, 1);
	else if (strcmp(name, "a_armstay") == 0)
		crm.webNotif("i", requestResultToStr(panel.fun.nxKpdFunction(kpd_function::armStay, crm.var("a_part").toInt())), 5, 1);
	else if (strcmp(name, "a_disarm") == 0)
		crm.webNotif("i", requestResultToStr(panel.fun.nxKpdFunction(kpd_function::disarm, crm.var("a_part").toInt())), 5, 1);
	else if (strcmp(name, "a_cancel") == 0)
		crm.webNotif("i", requestResultToStr(panel.fun.nxKpdFunction(kpd_function::cancel, crm.var("a_part").toInt())), 5, 1);
	else if (strcmp(name, "a_chime") == 0)
		crm.webNotif("i", requestResultToStr(panel.fun.nxKpdFunction(kpd_function::chime, crm.var("a_part").toInt())), 5, 1);
	else if (strcmp(name, "a_bypass") == 0)
		crm.webNotif("i", requestResultToStr(panel.fun.nxBypass(crm.var("a_zone").toInt())), 5, 1);
	updateFull();
}

String WebUI::getID()
{
	return crm.getID();
}

String WebUI::configKeyLoad(const String& name, const String& default_val)
{
	String val = crm.var(name);
	if (val == F("null"))
		return default_val;
	else
		return val;
}

void WebUI::onInterface()
{
	crm.page("&#xe802; System status");
	crm.output({ OUTPUT_TABL, F("time"), F("Date & time"), NDEF });
	crm.group(G_START);
	crm.input({ INPUT_DATE, "sd_date", "Date" });
	crm.input({ INPUT_TIME, "sd_time", "Time" });
	crm.selOpt({ "SUN", "1" });
	crm.selOpt({ "MON", "2" });
	crm.selOpt({ "TUE", "3" });
	crm.selOpt({ "WED", "4" });
	crm.selOpt({ "THU", "5" });
	crm.selOpt({ "FRI", "6" });
	crm.selOpt({ "SAT", "7" });
	crm.select({ "sd_weekday", "Week day", "1", false });
	crm.input({ INPUT_BUTTON, "sd_set", "Set" });
	crm.group(G_END);
	crm.output({ OUTPUT_TABL, F("lineSeizure"), F("Line seizure"), NDEF, NCLR});
	crm.output({ OUTPUT_TABL, F("offHook"), F("Off hook"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("handshakeReceived"), F("Initial handshake received"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("downloadInProgress"), F("Download in progress"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("dialerDelay"), F("Dialer delay in progress"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("usingBackupPhone"), F("Using backup phone"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("listenInActive"), F("Listen in active"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("twoWayLockout"), F("Two way lockout"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("groundFault"), F("Ground fault"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("phoneFault"), F("Phone fault"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("failToCommunicate"), F("Fail to communicate"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("fuseFault"), F("Fuse fault"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("boxTamper"), F("Box tamper"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("sirenTamper"), F("Siren tamper / trouble"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("lowBattery"), F("Low Battery"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("acFail"), F("AC fail"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("expBoxTamper"), F("Expander box tamper"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("expAcFailure"), F("Expander AC failure"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("expLowBattery"), F("Expander low battery"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("expLossSup"), F("Expander loss of supervision"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("expAuxOverCurrent"), F("Expander auxiliary output over current"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("auxCommFail"), F("Auxiliary communication channel failure"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("expBellFault"), F("Expander bell fault"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("pin6DigitEnabled"), F("6 digit PIN enabled"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("tokenInUse"), F("Programming token in use"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("pinRequiredLocal"), F("PIN required for local download"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("globalBuzzer"), F("Global pulsing buzzer"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("globalSirenOn"), F("Global Siren on"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("globalSteadySiren"), F("Global steady siren"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("busLineSeized"), F("Bus device has line seized"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("busSniffReq"), F("Bus device has requested sniff mode"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("dynBatteryTest"), F("Dynamic battery test"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("acPowerOn"), F("AC power on"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("lowBatteryMem"), F("Low battery memory"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("groundFaultMem"), F("Ground fault memory"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("fireAlarmVerify"), F("Fire alarm verification being timed"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("smokePowerReset"), F("Smoke power reset"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("line50HzDetected"), F("50 Hz line power detected"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("hvBatteryCharge"), F("Timing a high voltage battery charge"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("commSinceAutotest"), F("Communication since last autotest"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("powerUpDelay"), F("Power up delay in progress"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("walkTestMode"), F("Walk test mode"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("lossSystemTime"), F("Loss of system time"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("enrollRequested"), F("Enroll requested"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("testFixtureMode"), F("Test fixture mode"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("ctrlShutdownMode"), F("Control shutdown mode"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("cancelWindow"), F("Timing a cancel window"), NDEF, NCLR });
	crm.output({OUTPUT_HR, "1px", ""});
	crm.output({ OUTPUT_TABL, F("callBackInProgress"), F("Call back in progress"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("phoneLineFaulted"), F("Phone line faulted"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("voltageInterrupt"), F("Voltage present interrupt active"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("housePhoneOffHook"), F("House phone off hook"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("phoneMonitorEnable"), F("Phone line monitor enabled"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("sniffing"), F("Sniffing"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("lastReadOffHook"), F("Last read was off hook"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("listenInRequested"), F("Listen in requested"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("listenInTrigger"), F("Listen in trigger"), NDEF, NCLR });

	crm.page("&#xf0e8; Partitions");
	crm.group(G_START);
	crm.input({ INPUT_NUMBER, "a_part", "Partition", "1" });
	crm.input({ INPUT_BUTTON, "a_armaway", F("Arm away") });
	crm.input({ INPUT_BUTTON, "a_armstay", F("Arm stay") });
	crm.input({ INPUT_BUTTON, "a_disarm", F("Disarm") });
	crm.input({ INPUT_BUTTON, "a_cancel", F("Cancel") });
	crm.input({ INPUT_BUTTON, "a_chime", F("Chime") });
	crm.group(G_END);
	//State
	crm.output({ OUTPUT_TABL, F("partValid"), F("Valid partition"), NDEF });
	crm.output({ OUTPUT_TABL, F("bypassCodeRequired"), F("Bypass code required"), NDEF });
	crm.output({ OUTPUT_TABL, F("fireTrouble"), F("Fire trouble"), NDEF });
	crm.output({ OUTPUT_TABL, F("fire"), F("Fire"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("pulsingBuzzer"), F("Pulsing Buzzer"), NDEF });
	crm.output({ OUTPUT_TABL, F("tlmFaultMemory"), F("TLM fault memory"), NDEF });
	crm.output({ OUTPUT_TABL, F("armed"), F("Armed"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("instant"), F("Instant"), NDEF });
	crm.output({ OUTPUT_TABL, F("previousAlarm"), F("Previous Alarm"), NDEF });
	crm.output({ OUTPUT_TABL, F("sirenOn"), F("Siren on"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("steadySirenOn"), F("Steady siren on"), NDEF });
	crm.output({ OUTPUT_TABL, F("alarmMemory"), F("Alarm memory"), NDEF });
	crm.output({ OUTPUT_TABL, F("tamper"), F("Tamper"), NDEF });
	crm.output({ OUTPUT_TABL, F("cancelCmdEntered"), F("Cancel command entered"), NDEF });
	crm.output({ OUTPUT_TABL, F("codeEntered"), F("Code entered"), NDEF });
	crm.output({ OUTPUT_TABL, F("cancelPending"), F("Cancel pending"), NDEF });
	crm.output({ OUTPUT_TABL, F("silentExitEnabled"), F("Silent exit enabled"), NDEF });
	crm.output({ OUTPUT_TABL, F("entryGuard"), F("Entryguard (stay mode)"), NDEF });
	crm.output({ OUTPUT_TABL, F("chimeModeOn"), F("Chime mode on"), NDEF, YCLR });
	crm.output({ OUTPUT_TABL, F("entry"), F("Entry"), NDEF });
	crm.output({ OUTPUT_TABL, F("delayExpiration"), F("Delay expiration warning"), NDEF });
	crm.output({ OUTPUT_TABL, F("exit1"), F("Exit1"), NDEF });
	crm.output({ OUTPUT_TABL, F("exit2"), F("Exit2"), NDEF });
	crm.output({ OUTPUT_TABL, F("ledExtinguish"), F("LED extinguish"), NDEF });
	crm.output({ OUTPUT_TABL, F("crossTiming"), F("Cross timing"), NDEF });
	crm.output({ OUTPUT_TABL, F("recentClosing"), F("Recent closing being timed"), NDEF });
	crm.output({ OUTPUT_TABL, F("exitErrorTriggered"), F("Exit error triggered"), NDEF });
	crm.output({ OUTPUT_TABL, F("autoHomeInhibited"), F("Auto home inhibited"), NDEF });
	crm.output({ OUTPUT_TABL, F("sensorLowBattery"), F("Sensor low battery"), NDEF });
	crm.output({ OUTPUT_TABL, F("sensorLostSup"), F("Sensor lost supervision"), NDEF });
	crm.output({ OUTPUT_TABL, F("zoneBypassed"), F("Zone bypassed"), NDEF });
	crm.output({ OUTPUT_TABL, F("forceArmAuto"), F("Force arm triggered by auto arm"), NDEF });
	crm.output({ OUTPUT_TABL, F("readyToArm"), F("Ready to arm"), NDEF, YCLR });
	crm.output({ OUTPUT_TABL, F("readyToForceArm"), F("Ready to force arm"), NDEF, YCLR });
	crm.output({ OUTPUT_TABL, F("validPinAccepted"), F("Valid PIN accepted"), NDEF });
	crm.output({ OUTPUT_TABL, F("chimeOn"), F("Chime on (sounding)"), NDEF });
	crm.output({ OUTPUT_TABL, F("errorBeep"), F("Error beep (triple beep)"), NDEF });
	crm.output({ OUTPUT_TABL, F("toneOn"), F("Tone on (activation tone)"), NDEF });
	crm.output({ OUTPUT_TABL, F("entry1"), F("Entry 1"), NDEF });
	crm.output({ OUTPUT_TABL, F("openPeriod"), F("Open period"), NDEF });
	crm.output({ OUTPUT_TABL, F("alarmPhone1"), F("Alarm sent using phone number 1"), NDEF });
	crm.output({ OUTPUT_TABL, F("alarmPhone2"), F("Alarm sent using phone number 2"), NDEF });
	crm.output({ OUTPUT_TABL, F("alarmPhone3"), F("Alarm sent using phone number 3"), NDEF });
	crm.output({ OUTPUT_TABL, F("cancelReportStack"), F("Cancel report is in the stack"), NDEF });
	crm.output({ OUTPUT_TABL, F("keyswitchArmed"), F("Keyswitch armed"), NDEF });
	crm.output({ OUTPUT_TABL, F("delayTrip"), F("Delay Trip in progress (common zone)"), NDEF });
	crm.output({ OUTPUT_TABL, F("lastUserNum"), F("Last user number"), NDEF });

	crm.page("&#xe806; Zones");
	crm.group(G_START);
	crm.input({ INPUT_NUMBER, "a_zone", "Zone", "1" });
	crm.input({ INPUT_BUTTON, "a_bypass", F("Toggle bypass") });
	crm.group(G_END);
	crm.output({ OUTPUT_TEXT, "t1", F("Zone status"), F("<u>F</u>ault, <u>Ta</u>mper, <u>Tr</u>ouble, <u>B</u>ypass, <u>A</u>larm <u>m</u>emory, <u>I</u>nhibit, <u>L</u>ow <u>b</u>attery, <u>L</u>ost, <u>B</u>ypass <u>m</u>emory"), "#888888" });
	for (size_t i = 1; i <= MAX_ZONE; i++) {
		crm.output({ OUTPUT_TABL, "zone" + String(i), "Zone " + String(i), NDEF});
	}

	crm.page("&#xe80b; Event log");
	crm.output({ OUTPUT_TABL, F("logHead"), F("Log head"), NDEF });
	crm.group(G_START);
	crm.input({ INPUT_BUTTON, "readselfull", F("Read all events") });
	crm.input({ INPUT_BUTTON, "readsel10", F("Read top 10 events") });
	crm.input({ INPUT_BUTTON, "readsel50", F("Read top 50 events") });
	crm.group(G_END);
	for (size_t i = 0; i <= panel.state.logSize; i++) {
		crm.output({ OUTPUT_TABL, "sel" + String(i), "#" + String(i) });
	}

	crm.page("&#xe82c; Serial");
	crm.output({ OUTPUT_TABL, F("enrolled"), F("Module enrolled"), NDEF, NCLR });
	crm.output({ OUTPUT_TABL, F("lastPoll"), F("Last poll, ms"), NDEF });
	crm.group(G_START);
	crm.input({ INPUT_NUMBER, "c_busaddr", "Module address", String(Config.busAddress) });
	crm.input({ INPUT_BUTTON, "savecfg", "Save & reboot" });
	crm.group(G_END);
	crm.output({ OUTPUT_TABL, F("received"), F("Received messages"), NDEF });
	crm.output({ OUTPUT_TABL, F("tooShort"), F("Too short messages"), NDEF });
	crm.output({ OUTPUT_TABL, F("bufferOverflows"), F("Too long messages"), NDEF });
	crm.output({ OUTPUT_TABL, F("incorrectChecksums"), F("Incorrect checksums"), NDEF });
	crm.output({ OUTPUT_TABL, F("lostCycles"), F("Lost cycles"), NDEF });
	crm.output({ OUTPUT_TABL, F("queueOverflows"), F("Message queue overflows"), NDEF });
	crm.output({ OUTPUT_TABL, F("sent"), F("Sent messages"), NDEF });
	crm.output({ OUTPUT_TABL, F("retries"), F("Requests retries"), NDEF });

	crm.page("&#xf1de; Features");
	crm.input({ INPUT_CHECKBOX, "c_dl900", "Enable DL900 TCP/IP download (port " + String(DL900_PORT) + ")", "0", "0" });
	crm.input({ INPUT_CHECKBOX, "c_nx584", "Enable NX584 binary commands (port " + String(NX584_PORT) + ")", "0", "0" });
	crm.input({ INPUT_CHECKBOX, "c_dcui", "Enable direct shell to NX bus (port " + String(DIRECT_UI_PORT) + ")", "0", "0" });
	crm.input({ INPUT_CHECKBOX, "c_dbg", "Enable raw NX bus messages streaming (port " + String(DEBUG_PORT) + ")", "0", "0" });

	crm.page("&#xe808; Network");
#if ETH_ENABLED == 1
	crm.output({ OUTPUT_LABEL, "", "Ethernet", "left", "#0f0", "20" });
	crm.group(G_START);
	crm.input({ INPUT_TEXT, "c_ip",					"IP",			Config.ip.toString(),		F("[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}") });
	crm.input({ INPUT_TEXT, "c_subnet",				"Mask",			Config.subnet.toString(),	F("[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}") });
	crm.group(G_END);
	crm.input({ INPUT_TEXT, "c_gateway",			"Gateway",		Config.gateway.toString(),	F("[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}") });
	crm.input({ INPUT_TEXT, "c_dns",				"DNS server",	Config.dns.toString(),		F("[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}\\\\.[0-9]{1,3}") });
	crm.output({ OUTPUT_LABEL, "", "WiFi", "left", "#0f0", "20" });
	crm.input({ INPUT_CHECKBOX, "_uw", "Enable WiFi", "0", "1" });
	if (crm.var("_uw") == "1")
#endif
		crm.wifiForm(WIFI_AP, "NX Gateway");
	
	crm.output({ OUTPUT_LABEL, "", "MQTT", "left", "#0f0", "20" });
	crm.input({ INPUT_TEXT, "c_mqttserver",			"MQTT server",	Config.mqttServer });
	crm.group(G_START);
	crm.input({ INPUT_TEXT, "c_mqttuser",			"MQTT user",	 Config.mqttUsername });
	crm.input({ INPUT_PASSWORD, "c_mqttpassword",   "MQTT password", Config.mqttPassword });
	crm.group(G_END);
	crm.input({ INPUT_TEXT, "c_mqttid",   "MQTT device ID", Config.mqttID, F("^[a-zA-Z0-9_\\\\-]+(?:\\\\/[a-zA-Z0-9_\\\\-]+)*$") });
	crm.output({ OUTPUT_LABEL, "mqttok", "", "left", "#bbb"});
	crm.output({ OUTPUT_LABEL, "", "Security", "left", "#0f0", "20" });
	crm.input({ INPUT_PASSWORD, "c_password", "WebUI & OTA update password", Config.password });
	crm.input({ INPUT_BUTTON, "savecfg", "Save & reboot" });
}
