#ifndef RFM_MQTT_H
#define RFM_MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <FanManager.h>

#include <rfm_config.h>

class MQTTManager {
private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    FanManager& fm;
    String baseTopic;
    unsigned long lastPublish = 0;
    const unsigned long PUBLISH_INTERVAL = 5000;

public:
    MQTTManager(FanManager& fm) : mqttClient(espClient), fm(fm) {}

    bool setup(const char* server, int port, const char* username, const char* password) {
        baseTopic = "radiatorfanmanager/";
        baseTopic += String(config.hostname);
        mqttClient.setServer(server, port);
        mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
            this->callback(topic, payload, length);
        });

        return connect(username, password);
    }

    bool connect(const char* username, const char* password) {
        String clientName = String(config.hostname);
        clientName.toLowerCase();
        String clientId = "rfm_"+clientName;

        if (mqttClient.connect(clientId.c_str(), username, password)) {
            // Subscribe für alle Fan-PWM-Set Topics
            for (int i = 0; i < fm.getFanCount(); i++) {
                String topic = baseTopic + "/fan" + String(i) + "/pwm/set";
                mqttClient.subscribe(topic.c_str());
            }
            return true;
        }
        return false;
    }

    void loop() {
        if (!mqttClient.connected()) {
            reconnect();
        }
        mqttClient.loop();

        // Regelmäßiges Publishing der Werte
        unsigned long now = millis();
        if (now - lastPublish >= PUBLISH_INTERVAL) {
            publishValues();
            lastPublish = now;
        }
    }

    // PWM und RPM Werte publishen
    void publishValues() {
        if (!mqttClient.connected()) return;

        for (int i = 0; i < fm.getFanCount(); i++) {
            String pwmTopic = baseTopic + "/fan" + String(i) + "/pwm";
            String rpmTopic = baseTopic + "/fan" + String(i) + "/rpm";

             // IP-Adresse publishen
            String ipTopic = baseTopic + "/ip";

            // Hier die aktuellen Werte von Ihrer Fan-Steuerung holen
            int currentPWM = fm.getPWM(i);
            int currentRPM = fm.getRPM(i);

            mqttClient.publish(pwmTopic.c_str(), String(currentPWM).c_str());
            mqttClient.publish(rpmTopic.c_str(), String(currentRPM).c_str());
            mqttClient.publish(ipTopic.c_str(), WiFi.localIP().toString().c_str());
        }
    }

    // Callback für empfangene MQTT Nachrichten
    void callback(char* topic, byte* payload, unsigned int length) {
        String topicStr = String(topic);
        
        // Payload in String umwandeln
        char payloadStr[length + 1];
        memcpy(payloadStr, payload, length);
        payloadStr[length] = '\0';
        
        // Fan-Nummer aus Topic extrahieren
        int fanNumber = -1;
        for (int i = 0; i < fm.getFanCount(); i++) {
            String checkTopic = baseTopic + "/fan" + String(i) + "/pwm/set";
            if (topicStr == checkTopic) {
                fanNumber = i;
                break;
            }
        }

        if (fanNumber >= 0) {
            // PWM-Wert parsen und speichern
            int pwmValue = atoi(payloadStr);
            if(pwmValue > 100)
                pwmValue = 100;
            fm.setPWM(fanNumber, pwmValue);
        }
    }

private:
    void reconnect() {
        // Reconnect-Versuch alle 5 Sekunden
        static unsigned long lastReconnectAttempt = 0;
        unsigned long now = millis();
        
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            if (connect(config.mqttUser, config.mqttPassword)) {
                lastReconnectAttempt = 0;
            }
        }
    }
};

#endif