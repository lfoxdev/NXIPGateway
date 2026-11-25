#include "Config.h"
#include "WebUI.h"

void Config_t::load()
{
	Config.busAddress = WebUI::configKeyLoad("c_busaddr", "79").toInt();
	Config.ip.fromString(WebUI::configKeyLoad("c_ip", "192.168.1.200"));
	Config.gateway.fromString(WebUI::configKeyLoad("c_gateway", "192.168.1.1"));
	Config.subnet.fromString(WebUI::configKeyLoad("c_subnet", "255.255.255.0"));
	Config.dns.fromString(WebUI::configKeyLoad("c_dns", "192.168.1.1"));
	Config.mqttServer = WebUI::configKeyLoad("c_mqttserver", "mqtt://example.com");
	Config.mqttUsername = WebUI::configKeyLoad("c_mqttuser", "");
	Config.mqttPassword = WebUI::configKeyLoad("c_mqttpassword", "");
	Config.mqttID = WebUI::configKeyLoad("c_mqttid", "nxipgw/" + WebUI::getID());
	Config.dl900Enabled = (WebUI::configKeyLoad("c_dl900", "1") == "1");
	Config.nx584Enabled = (WebUI::configKeyLoad("c_nx584", "0") == "1");
	Config.dcuiEnabled = (WebUI::configKeyLoad("c_dcui", "0") == "1");
	Config.dbgEnabled = (WebUI::configKeyLoad("c_dbg", "0") == "1");
	Config.password = WebUI::configKeyLoad("c_password", "caddx");
}

Config_t Config;