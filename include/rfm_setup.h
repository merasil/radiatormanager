#ifndef RFM_SETUP_H
#define RFM_SETUP_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>

#include <rfm_config.h>

DNSServer dnsServer;

bool isValidWiFiConfig() {
    // Prüft ob SSID und Passwort vorhanden und plausibel sind
    if(strlen(config.ssid) == 0 || strlen(config.ssid) > 32) {
        Serial.println("Invalid or missing SSID");
        return false;
    }
    
    if(strlen(config.password) < 8 || strlen(config.password) > 63) {
        Serial.println("Invalid WiFi password length");
        return false;
    }
    
    return true;
}

bool startNormalMode() {
    isConfigMode = false;

    // WiFi im Station-Modus starten
    WiFi.mode(WIFI_STA);
    
    if(strlen(config.hostname) > 0) {
        WiFi.setHostname(config.hostname);
    }

    // Statische IP-Konfiguration wenn eingestellt
    if(config.staticIP) {
        IPAddress ip, subnet, gateway, dns;
        if(ip.fromString(config.ip) && 
           subnet.fromString(config.subnet) && 
           gateway.fromString(config.gateway) && 
           dns.fromString(config.dns)) {
            WiFi.config(ip, gateway, subnet, dns);
        }
    }

    // Mit WLAN verbinden
    WiFi.begin(config.ssid, config.password);

    // Auf Verbindung warten (mit Timeout)
    int timeout = 0;
    const int maxAttempts = 30;  // 30 Sekunden Timeout
    bool connected = false;
    
    while (timeout < maxAttempts) {
        if(WiFi.status() == WL_CONNECTED) {
            connected = true;
            break;
        }
        
        delay(1000);
        Serial.print(".");
        timeout++;
        
        // Verschiedene Fehlerzustände prüfen
        if(WiFi.status() == WL_NO_SSID_AVAIL) {
            Serial.println("\nConfigured SSID not found");
            break;
        }
        if(WiFi.status() == WL_CONNECT_FAILED) {
            Serial.println("\nConnection failed - possibly wrong password");
            break;
        }
    }

    if(!connected) {
        Serial.println("\nWiFi connection failed");
        WiFi.disconnect();
        return false;
    }

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.println(WiFi.RSSI());
    
    return true;
}

void startConfigMode() {
    isConfigMode = true;

    // Konstanten für den AP-Modus
    const char* AP_SSID = "RadiatorFanManager";
    const char* AP_PASSWORD = "";  // leeres Passwort = offenes WLAN
    const byte DNS_PORT = 53;
    const IPAddress AP_IP(192, 168, 4, 1);
    const IPAddress AP_SUBNET(255, 255, 255, 0);

    // Bestehende Verbindungen trennen
    WiFi.disconnect();
    delay(100);

    // WiFi im AP-Modus starten
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, AP_SUBNET);
    
    // AP starten und Erfolg prüfen
    if(!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        Serial.println("Failed to start Access Point!");
        return;
    }

    // DNS Server starten - leitet alle Anfragen an den ESP um
    dnsServer.start(DNS_PORT, "*", AP_IP);

    Serial.println("Access Point Started");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("IP Address: ");
    Serial.println(AP_IP);
}

#endif