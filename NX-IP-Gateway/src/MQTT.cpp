// 
// 
// 
#include "Config.h"
#include "MQTT.h"
#include "NXPanel.h"
#include "NXPanelFunctions.h"

#define LOG_TAG "MQTT"

void MQTTMgr::start(String brokerUri, String username, String password)
{
    ESP_LOGI(LOG_TAG, "Starting MQTT");
	esp_mqtt_client_config_t mqtt_cfg = {};
	mqtt_cfg.broker.address.uri = brokerUri.c_str();
    if (!username.isEmpty()) {
        mqtt_cfg.credentials.username = username.c_str();
        mqtt_cfg.credentials.authentication.password = password.c_str();
    }
    mqtt_cfg.session.last_will = {
        .topic = (Config.mqttID + "/part/0/state").c_str(),
        .msg = "None",
        .qos = 1,
        .retain = 1
    };
	client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == nullptr) {
        ESP_LOGE(LOG_TAG, "MQTT initialization error");
        return;
    }
	esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, eventHandler, this);
	if (esp_mqtt_client_start(client) != ESP_OK)
        ESP_LOGE(LOG_TAG, "MQTT start error");
}

void MQTTMgr::send(String topic, String payload, bool retain, int qos)
{
    if (client) {
        esp_mqtt_client_enqueue(client, (Config.mqttID + "/" + topic).c_str(), payload.c_str(), 0, qos, retain, true);
    }
}

void MQTTMgr::sendErr(String payload)
{
    send ("error", payload, false, 0);
}

bool MQTTMgr::connected() { 
    return mqtt_connected; 
}


void MQTTMgr::eventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    MQTTMgr* self = static_cast<MQTTMgr*>(handler_args);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(LOG_TAG, "MQTT connected");
        self->mqtt_connected = true;
        self->error = "";
        esp_mqtt_client_subscribe(self->client, (Config.mqttID + "/cmd").c_str(), 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(LOG_TAG, "MQTT disconnected");
        self->mqtt_connected = false;
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(LOG_TAG, "MQTT error");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            switch (event->error_handle->connect_return_code) {
            case MQTT_CONNECTION_REFUSE_PROTOCOL:
                self->error = "Unacceptable protocol version";
                break;
            case MQTT_CONNECTION_REFUSE_ID_REJECTED:
                self->error = "Client ID rejected";
                break;
            case MQTT_CONNECTION_REFUSE_SERVER_UNAVAILABLE:
                self->error = "Broker is not ready";
                break;
            case MQTT_CONNECTION_REFUSE_BAD_USERNAME:
                self->error = "Bad username or password";
                break;
            case MQTT_CONNECTION_REFUSE_NOT_AUTHORIZED:
                self->error = "Not authorized";
            default:
                break;
            }
        }
        else if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            self->error = "TCP error";
        }
        break;
    case MQTT_EVENT_DATA: {        
        String currentTopic = String(event->topic, event->topic_len);
        String data(event->data, event->data_len);
        DeserializationError error = deserializeJson(self->currentPayload, data);
        if (error) {
            ESP_LOGE(LOG_TAG, "JSON deserialization error %s", error.c_str());
            break;
        }
        if (currentTopic == "cmd")
            self->onCommandReceived();    
        if (currentTopic == "bypass")
            self->onBypassReceived();     
        break;
    }
    default:
        break;
    }
}

void MQTTMgr::onCommandReceived()
{
    ESP_LOGI(LOG_TAG, "Received CMD via MQTT");

    uint8_t part = 1;
    if (currentPayload["partition"] != "null")
        part = currentPayload["partition"];    
    if (part < 1 || part > MAX_PARTITION) {
        sendErr("Invalid partition number");
        return;
    }

    String pins = currentPayload["pin"];
    pin_t pin;
    if (!pin.set(pins)) {
        sendErr("Incorrect pin");
        return;
    }

    String action = currentPayload["action"];

    checkPinAuthReq authReq;
    if (action == "ARM_AWAY" || action == "ARM_HOME")
        authReq = authArm;
    else if (action == "ARM_CUSTOM_BYPASS")
        authReq = authBypass;
    else if (action == "DISARM")
        authReq = authDisarm;
    else
        authReq = authAny;

    checkPinResult r = panel.fun.checkPin(pin, part, authReq);

    if (r == pinCommErr)
        sendErr("Communication error");
    else if (r == pinNoUser)
        sendErr("Incorrect pin");
    else if (r == pinNoAuth)
        sendErr("Pin not authorized");
    else {
        requestResult r2 = requestResult::rqBADRQ;
        if (action == "ARM_AWAY") {
            r2 = panel.fun.nxKpdFunction(kpd_function::armAway, part);
        }
        else if (action == "ARM_HOME")
            r2 = panel.fun.nxKpdFunction(kpd_function::armStay, part);
        else if (action == "ARM_CUSTOM_BYPASS") {
            r2 = panel.fun.nxKpdFunction(kpd_function::groupBypass, part);
            r2 = panel.fun.nxKpdFunction(kpd_function::armAway, part);
        }
        else if (action == "DISARM")
            r2 = panel.fun.nxKpdFunction(kpd_function::disarm, part);
        else if (action == "TRIGGER")
            r2 = panel.fun.nxKpdFunction(kpd_function::policePanic, part);                
        else
            sendErr("Unknown command");
        if (r2 != rqOK)
            sendErr(requestResultToStr(r2));
    }
}

void MQTTMgr::onBypassReceived()
{
    ESP_LOGI(LOG_TAG, "Received BYPASS via MQTT");

    uint8_t zone = 0;
    if (currentPayload["zone"] != "null")
        zone = currentPayload["zone"];   
    if (zone < 1 || zone > MAX_ZONE) {
        sendErr("Invalid zone number");
        return;
    }
    if (((currentPayload["enable"] == true) && !panel.state.zones[zone-1].bypass.get()) ||   // enable disabled
        ((currentPayload["enable"] == false) && panel.state.zones[zone-1].bypass.get())) {    // disable enabled
            requestResult r = panel.fun.nxBypass(zone);
            if (r != rqOK)
                sendErr(requestResultToStr(r));
        }
}

MQTTMgr MQTT;

