#ifndef RFM_CONFIG_H
#define RFM_CONFIG_H

// Get current time from compiler
String getBuildTimestamp() {
   String date = __DATE__;
   String time = __TIME__;
   
   // Parse month
   String month = date.substring(0, 3);
   String MM;
   if (month == "Jan") MM = "01";
   else if (month == "Feb") MM = "02";
   else if (month == "Mar") MM = "03";
   else if (month == "Apr") MM = "04";
   else if (month == "May") MM = "05";
   else if (month == "Jun") MM = "06";
   else if (month == "Jul") MM = "07";
   else if (month == "Aug") MM = "08";
   else if (month == "Sep") MM = "09";
   else if (month == "Oct") MM = "10";
   else if (month == "Nov") MM = "11";
   else if (month == "Dec") MM = "12";
   
   // Parse day and year
   String DD = date.substring(4, 6);
   DD.trim(); // Remove spaces
   if (DD.length() == 1) DD = "0" + DD;
   
   String YY = date.substring(date.length()-2);
   
   // Parse time
   String HH = time.substring(0, 2);
   String mm = time.substring(3, 5);
   
   return YY + MM + DD + "_" + HH + mm;
}

#define FIRMWARE_VERSION "1.2.0"
#define BUILD_TIMESTAMP getBuildTimestamp()


#include <Preferences.h>

bool isConfigMode = false;

// Struktur für die Konfigurationsdaten
struct Config {
    char ssid[32];
    char password[64];
    char hostname[32];
    bool staticIP;
    char ip[16];
    char subnet[16];
    char gateway[16];
    char dns[16];
    bool mqttEnabled;
    char mqttServer[64];
    int mqttPort;
    char mqttUser[32];
    char mqttPassword[32];
};

Config config;
Preferences pref;

// Funktion zum Laden der Konfiguration
void loadConfiguration() {
    pref.begin("settings", false);  // false = read/write mode
    
    // WLAN Einstellungen
    pref.getString("ssid", config.ssid, sizeof(config.ssid));
    pref.getString("password", config.password, sizeof(config.password));
    pref.getString("hostname", config.hostname, sizeof(config.hostname));
    
    // IP Einstellungen
    config.staticIP = pref.getBool("staticIP", false);
    if (config.staticIP) {
        pref.getString("ip", config.ip, sizeof(config.ip));
        pref.getString("subnet", config.subnet, sizeof(config.subnet));
        pref.getString("gateway", config.gateway, sizeof(config.gateway));
        pref.getString("dns", config.dns, sizeof(config.dns));
    }
    
    // MQTT Einstellungen
    config.mqttEnabled = pref.getBool("mqttEnabled", false);
    if (config.mqttEnabled) {
        pref.getString("mqttServer", config.mqttServer, sizeof(config.mqttServer));
        config.mqttPort = pref.getInt("mqttPort", 1883);
        pref.getString("mqttUser", config.mqttUser, sizeof(config.mqttUser));
        pref.getString("mqttPassword", config.mqttPassword, sizeof(config.mqttPassword));
    }
    
    pref.end();
}

void saveConfiguration() {
    pref.begin("settings", false);
    
    // WLAN Einstellungen
    pref.putString("ssid", config.ssid);
    pref.putString("password", config.password);
    pref.putString("hostname", config.hostname);
    
    // IP Einstellungen
    pref.putBool("staticIP", config.staticIP);
    if (config.staticIP) {
        pref.putString("ip", config.ip);
        pref.putString("subnet", config.subnet);
        pref.putString("gateway", config.gateway);
        pref.putString("dns", config.dns);
    }
    
    // MQTT Einstellungen
    pref.putBool("mqttEnabled", config.mqttEnabled);
    if (config.mqttEnabled) {
        pref.putString("mqttServer", config.mqttServer);
        pref.putInt("mqttPort", config.mqttPort);
        pref.putString("mqttUser", config.mqttUser);
        pref.putString("mqttPassword", config.mqttPassword);
    }
    
    pref.end();
}
#endif