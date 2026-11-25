// CaddxIPGateway.cpp
// Initialization and main loop

#include <Arduino.h>
#include "NX584Message.h"
#include "Config.h"
#include "MQTT.h"
#include "DCUI.h"
#include "NX584.h"
#include "DL900.h"
#include "NXPanel.h"
#include "NXPanelStateItem.h"
#include "WebUI.h"
#include <ETH.h>
#include <ESP32Ping.h>
#include "esp_ota_ops.h"

#if ETH_ENABLED == 1
    #include <ETH.h>
#endif

#define LOG_TAG "MAIN"

extern NXPanel panel;

uint32_t last_ota_time = 0;

void setup() {

	Serial.begin(19200);
    Serial.println("Booting...");

    delay(1000);

    gpio_reset_pin(CFGCLR_PIN);
    gpio_set_direction(CFGCLR_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(CFGCLR_PIN);
    gpio_pulldown_dis(CFGCLR_PIN);
    
    esp_log_level_set("*", ESP_LOG_INFO);

    if (gpio_get_level(CFGCLR_PIN) == 0) {
        Serial.println("Config reset to defaults");
        WebUI::cfgDelete();
    }
    WebUI::cfgLoad();
    Config.load();

#if ETH_ENABLED == 1
    ETH.begin();
    ETH.config(Config.ip, Config.gateway, Config.subnet, Config.dns);
    Serial.printf("Server started on %s/%s, default gateway on %s, DNS server %s\r\n", Config.ip.toString().c_str(), Config.subnet.toString().c_str(), Config.gateway.toString().c_str(), Config.dns.toString().c_str());
#endif

    ArduinoOTA
        .onStart([]() {
        panel.stop();
        if (ArduinoOTA.getCommand() == U_FLASH) {
            WebUI::notify("orange", F("Start updating firmware"));
            ESP_LOGI(LOG_TAG, "Start updating firmware\r\n");
        }
        else {
            WebUI::notify("orange", F("Start updating filesystem"));
            ESP_LOGI(LOG_TAG, "Start updating filesystem\r\n");
        }
        })
        .onEnd([]() {
            WebUI::notify("orange", F("Update complete. Rebooting..."));
            ESP_LOGI(LOG_TAG, "Update complete. Rebooting...");
            })
        .onProgress([](unsigned int progress, unsigned int total) {
        if (millis() - last_ota_time > 500) {
            WebUI::notify("orange", "Updating " + String(progress / (total / 100)) + "%");
            ESP_LOGI(LOG_TAG, "Progress: %u%%\r\n", (progress / (total / 100)));
            last_ota_time = millis();
        }
            })
        .onError([](ota_error_t error) {
        ESP_LOGI(LOG_TAG, "Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            WebUI::notify("orange", F("Update error: Auth Failed"));
            ESP_LOGI(LOG_TAG, "Auth Failed\r\n");
        }
        else if (error == OTA_BEGIN_ERROR) {
            WebUI::notify("orange", F("Update error: Begin Failed"));
            ESP_LOGI(LOG_TAG, "Begin Failed\r\n");
        }
        else if (error == OTA_CONNECT_ERROR) {
            WebUI::notify("orange", F("Update error: Connect Failed"));
            ESP_LOGI(LOG_TAG, "Connect Failed\r\n");
        }
        else if (error == OTA_RECEIVE_ERROR) {
            WebUI::notify("orange", F("Update error: Receive Failed"));
            ESP_LOGI(LOG_TAG, "Receive Failed\r\n");
        }
        else if (error == OTA_END_ERROR) {
            WebUI::notify("orange", F("Update error: End Failed"));
            ESP_LOGI(LOG_TAG, "End Failed\r\n");
        }
            });

    ArduinoOTA.setPassword(Config.password.c_str());
    ArduinoOTA.begin();

    esp_timer_create_args_t ota_timer_args = {
        .callback = [](void*){
        ArduinoOTA.handle();
        },
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "ota_timer"
    };
    esp_timer_handle_t ota_timer;
    esp_timer_create(&ota_timer_args, &ota_timer);
    esp_timer_start_periodic(ota_timer, 100000); 

    panel.start(Config.busAddress);
    if (Config.dbgEnabled)
        panel.enableDebug();
    if (Config.dcuiEnabled)
        dcui.start();
    if (Config.nx584Enabled)
        nx584.start();
    if (Config.dl900Enabled)
        dl900.start();

    WebUI::start();

    if (!Config.mqttServer.isEmpty()) {
        MQTT.start(Config.mqttServer, Config.mqttUsername, Config.mqttPassword);
        Serial.printf("MQTT broker is set to %s with username %s\r\n", Config.mqttServer.c_str(), Config.mqttUsername.c_str());
    } else
        Serial.println("MQTT disabled");

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;
    if (esp_ota_get_state_partition(running, &state) != ESP_OK) {
       Serial.println("OTA is not used");
    }
    else if (state == ESP_OTA_IMG_PENDING_VERIFY) {
        Serial.println("Firmware is pending verification!");
        bool selfTestPassed = false;
        selfTestPassed = Ping.ping(Config.gateway);

        if (selfTestPassed) {
            Serial.println("Self-test passed, confirming new firmware.");
            esp_ota_mark_app_valid_cancel_rollback();
        } else {
            Serial.println("Self-test failed, rolling back...");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }
    }
    else {
        Serial.printf("Firmware state %d\r\n", state);
    }

}



void loop() {
    static uint32_t last_stat_update = 0;
    static uint32_t last_full_update = 0;
    if (millis() - last_stat_update > WEBUI_UPD_PERIOD_STAT) {
        last_stat_update = millis();
        WebUI::updateStat();
    }
    if (millis() - last_full_update > WEBUI_UPD_PERIOD_FULL) {
        last_full_update = millis();
        WebUI::updateFull();
    }
    WebUI::crmLoop(NULL);
}