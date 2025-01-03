#include <Arduino.h>
#include <WiFi.h>
#include <nvs_flash.h>
#include <Update.h>

#include <rfm_config.h>
#include <rfm_server.h>
#include <rfm_setup.h>
#include <rfm_mqtt.h>

#include <FanManager.h>

FanManager fm;
MQTTManager mm{fm};

unsigned long lastWiFiCheck = 0;

void setup() {
  Serial.begin(115200);

  // FanManager konfigurieren
  fm.addFan(0, 5, true, true);
  fm.addFan(1, 6, true, true);
  fm.addFan(3, 7, true, true);
  fm.addFan(4, 10, true, true);

  // Konfiguration laden
  Serial.println("Loading Configuration...");
  loadConfiguration();

  // Prüfen ob eine gültige WLAN-Konfiguration vorhanden ist
  if(!isValidWiFiConfig()) {
    Serial.println("Invalid or missing WiFi configuration. Starting config mode...");
    startConfigMode();
  } else {
    Serial.println("Configuration found. Trying to connect to WiFi...");
    if(!startNormalMode()) {
      Serial.println("Could not connect to WiFi. Starting config mode...");
      startConfigMode();
    }
  }

  // Einrichten des Webservers
  Serial.println("Starting HTTP Server...");
  setupWebServer(fm);

  // MQTT einrichten
  if (config.mqttEnabled) {
    Serial.println("Enabling MQTT...");
    if (mm.setup(config.mqttServer, config.mqttPort, config.mqttUser, config.mqttPassword)) {
        Serial.println("...connected!");
    } else {
        Serial.println("...failed!");
    }
  }
  Serial.println("Setup finished!");
}

void loop() {
  if(isConfigMode)
    dnsServer.processNextRequest();
  if (config.mqttEnabled) {
    mm.loop();
  }
  if (millis() - lastWiFiCheck >= 30000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Reconnecting...");
      
      WiFi.disconnect();
      delay(1000);
            
      if (config.staticIP) {
          IPAddress ip, subnet, gateway, dns;
          ip.fromString(config.ip);
          subnet.fromString(config.subnet);
          gateway.fromString(config.gateway);
          dns.fromString(config.dns);
          WiFi.config(ip, gateway, subnet, dns);
      }
      
      WiFi.begin(config.ssid, config.password);

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi reconnected");
        Serial.println(WiFi.localIP());
      }
    }
    lastWiFiCheck = millis();
  }
  fm.handle();
  server.handleClient();
}