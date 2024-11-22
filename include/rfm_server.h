#ifndef RFM_SERVER_H
#define RFM_SERVER_H

#include <Arduino.h>
#include <Update.h>
#include <WebServer.h>
#include <FanManager.h>

#include <rfm_config.h>

WebServer server(80);

// Funktion zum Escapen von HTML-Sonderzeichen
String escapeHTML(const String& text) {
    String escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#039;");
    return escaped;
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    server.send(404, "text/plain", message);
}

void handleIndex(FanManager& fm) {
    const int UPDATE_INTERVAL = 2000;
    String html = F("<!DOCTYPE html>"
    "<html lang=\"de\">"
    "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>Radiator Fan Manager</title>"
        "<style>");

    html += F("body {"
    "font-family: Arial, sans-serif;"
    "max-width: 800px;"
    "margin: 0 auto;"
    "padding: 20px;"
    "background-color: #f4f4f4;"
    "}"
    ".nav-container {"
    "background-color: white;"
    "padding: 15px;"
    "border-radius: 8px;"
    "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
    "margin-bottom: 20px;"
    "}"
    ".nav-path {"
    "display: flex;"
    "align-items: center;"
    "gap: 10px;"
    "color: #666;"
    "}"
    ".nav-current {"
    "color: #333;"
    "font-weight: bold;"
    "}"
    ".container {"
    "background-color: white;"
    "padding: 30px;"
    "border-radius: 8px;"
    "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
    "}"
    "h1 {"
    "color: #333;"
    "margin-bottom: 30px;"
    "text-align: center;"
    "}"
    ".menu-container {"
    "display: flex;"
    "justify-content: center;"
    "gap: 15px;"
    "flex-wrap: wrap;"
    "margin-bottom: 30px;"
    "}"
    ".menu-button {"
    "padding: 12px 25px;"
    "color: white;"
    "text-decoration: none;"
    "border-radius: 5px;"
    "text-align: center;"
    "transition: background-color 0.3s ease;"
    "min-width: 160px;"
    "}"
    ".menu-button.config { background-color: #2196F3; }"
    ".menu-button.config:hover { background-color: #1E88E5; }"
    ".menu-button.erase { background-color: #f44336; }"
    ".menu-button.erase:hover { background-color: #d32f2f; }"
    ".menu-button.firmware { background-color: #FF9800; }"
    ".menu-button.firmware:hover { background-color: #F57C00; }"
    ".device-info {"
    "background-color: #f1f1f1;"
    "padding: 15px;"
    "border-radius: 5px;"
    "margin-bottom: 30px;"
    "}"
    ".device-info p {"
    "margin: 10px 0;"
    "display: flex;"
    "justify-content: space-between;"
    "}"
    ".fans-container {"
    "display: grid;"
    "grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));"
    "gap: 20px;"
    "}"
    ".fan-card {"
    "background-color: #f8f9fa;"
    "border-radius: 8px;"
    "padding: 20px;"
    "border: 1px solid #dee2e6;"
    "}"
    ".fan-header {"
    "font-weight: bold;"
    "margin-bottom: 15px;"
    "color: #2196F3;"
    "}"
    ".fan-info {"
    "margin-bottom: 15px;"
    "}"
    ".fan-info p {"
    "margin: 5px 0;"
    "display: flex;"
    "justify-content: space-between;"
    "}"
    ".pwm-control {"
    "display: flex;"
    "align-items: center;"
    "gap: 10px;"
    "}"
    ".pwm-slider {"
    "flex-grow: 1;"
    "height: 8px;"
    "-webkit-appearance: none;"
    "background: #ddd;"
    "border-radius: 4px;"
    "outline: none;"
    "}"
    ".pwm-slider::-webkit-slider-thumb {"
    "-webkit-appearance: none;"
    "width: 20px;"
    "height: 20px;"
    "background: #2196F3;"
    "border-radius: 50%;"
    "cursor: pointer;"
    "}"
    ".pwm-value {"
    "min-width: 48px;"
    "text-align: right;"
    "}");

    html += F("</style>"
    "</head>"
    "<body>"
        "<div class=\"nav-container\">"
            "<div class=\"nav-path\">"
                "<span class=\"nav-current\">Startseite</span>"
            "</div>"
        "</div>"
        "<div class=\"container\">"
            "<h1>Radiator Fan Manager</h1>"
            "<div class=\"menu-container\">"
                "<a href=\"/config\" class=\"menu-button config\">"
                    "Konfiguration ändern"
                "</a>"
                "<a href=\"/configerase\" class=\"menu-button erase\">"
                    "Konfiguration löschen"
                "</a>"
                "<a href=\"/firmware\" class=\"menu-button firmware\">"
                    "Firmware aktualisieren"
                "</a>"
            "</div>"
            "<div class=\"device-info\">"
                "<strong>Geräte-Informationen:</strong>");

    html += F("<p><span>Hostname:</span><span>");
    html += WiFi.getHostname();
    html += F("</span></p>"
            "<p><span>IP-Adresse:</span><span>");
    html += WiFi.localIP().toString();
    html += F("</span></p>"
            "<p><span>Firmware-Version:</span><span>");
    html += String(FIRMWARE_VERSION) + "_" + String(BUILD_TIMESTAMP);
    html += F("</span></p>"
            "</div>"
            "<div class=\"fans-container\">");

    // Lüfterkarten dynamisch generieren
    for(int i = 0; i < fm.getFanCount(); i++) {
        html += F("<div class=\"fan-card\">"
                    "<div class=\"fan-header\">Lüfter ");
        html += String(i + 1);
        html += F("</div>"
                    "<div class=\"fan-info\">"
                    "<p><span>Drehzahl:</span><span>");
        html += String(fm.getRPM(i));
        html += F(" RPM</span></p>"
                    "<p><span>PWM:</span><span>");
        html += String(fm.getPWM(i));
        html += F("%</span></p>"
                    "</div>");
        
        // PWM-Slider nur anzeigen wenn MQTT deaktiviert ist
        if (!config.mqttEnabled) {
            html += F("<div class=\"pwm-control\">"
                        "<input type=\"range\" class=\"pwm-slider\" min=\"0\" max=\"100\" value=\"");
            html += String(fm.getPWM(i));
            html += F("\" onchange=\"updatePWM(");
            html += String(i);
            html += F(", this.value)\">"
                        "<span class=\"pwm-value\">");
            html += String(fm.getPWM(i));
            html += F("%</span>"
                        "</div>");
        }
        html += F("</div>");
    }

    html += F("</div>"
        "</div>");

    // JavaScript nur einfügen wenn MQTT deaktiviert ist
    if (!config.mqttEnabled) {
        html += F("<script>");
        
        // PWM Update Funktion
        html += F("function updatePWM(fanId, value) {"
                    "  fetch('/setpwm?fan=' + fanId + '&value=' + value)"
                    "    .then(response => response.text())"
                    "    .then(data => {"
                    "      document.querySelectorAll('.pwm-value')[fanId].textContent = value + '%';"
                    "    });"
                    "}"
                    
                    // Event Listener für Slider
                    "document.querySelectorAll('.pwm-slider').forEach((slider, index) => {"
                    "  const valueDisplay = slider.nextElementSibling;"
                    "  slider.addEventListener('input', function() {"
                    "    valueDisplay.textContent = this.value + '%';"
                    "  });"
                    "});");
                    
        html += F("</script>");
    }

    // Automatische Aktualisierung (immer aktiv)
    html += F("<script>"
                "function updateFanData() {"
                "  fetch('/getfandata')"
                "    .then(response => response.json())"
                "    .then(data => {"
                "      Object.keys(data).forEach(fanId => {"
                "        const index = parseInt(fanId.replace('fan', ''));"
                "        const fan = data[fanId];"
                "        const card = document.querySelectorAll('.fan-card')[index];"
                "        card.querySelector('.fan-info p:nth-child(1) span:last-child').textContent = fan.rpm + ' RPM';"
                "        card.querySelector('.fan-info p:nth-child(2) span:last-child').textContent = fan.pwm + '%';"
                "        if(!");

    // Slider-Update nur wenn MQTT deaktiviert ist
    if (!config.mqttEnabled) {
        html += F("document.querySelector('.pwm-slider').matches(':active')");
    } else {
        html += F("false");
    }

    html += F(") {"
                "          const slider = card.querySelector('.pwm-slider');"
                "          if(slider) {"  // Prüfen ob Slider existiert
                "            slider.value = fan.pwm;"
                "            slider.nextElementSibling.textContent = fan.pwm + '%';"
                "          }"
                "        }"
                "      });"
                "    })"
                "    .catch(error => console.error('Error:', error));"
                "}"
                "setInterval(updateFanData, ");
    html += String(UPDATE_INTERVAL);
    html += F(");"
                "</script>"
                "</body>"
                "</html>");

    server.send(200, "text/html", html);
}

// Handler für die Konfig-Seite
void handleConfig() {
    String html = F("<!DOCTYPE html>"
        "<html lang=\"de\">"
        "<head>"
            "<meta charset=\"UTF-8\">"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<title>Konfiguration</title>"
            "<style>");

    // CSS
    html += F("body {"
        "font-family: Arial, sans-serif;"
        "max-width: 600px;"
        "margin: 0 auto;"
        "padding: 20px;"
        "background-color: #f4f4f4;"
        "}"
        ".nav-container {"
        "background-color: white;"
        "padding: 15px;"
        "border-radius: 8px;"
        "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
        "margin-bottom: 20px;"
        "}"
        ".nav-path {"
        "display: flex;"
        "align-items: center;"
        "gap: 10px;"
        "color: #666;"
        "}"
        ".nav-separator {"
        "color: #999;"
        "}"
        ".nav-link {"
        "text-decoration: none;"
        "color: #2196F3;"
        "transition: color 0.3s ease;"
        "}"
        ".nav-current {"
        "color: #333;"
        "font-weight: bold;"
        "}"
        ".home-icon {"
        "color: #2196F3;"
        "font-size: 18px;"
        "text-decoration: none;"
        "}"
        ".container {"
        "background-color: white;"
        "padding: 30px;"
        "border-radius: 8px;"
        "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
        "}"
        "h1 {"
        "text-align: center;"
        "color: #333;"
        "margin-bottom: 20px;"
        "}"
        ".form-group {"
        "margin-bottom: 15px;"
        "}"
        "label {"
        "display: block;"
        "margin-bottom: 5px;"
        "font-weight: bold;"
        "}"
        "input[type=\"text\"], input[type=\"password\"], input[type=\"number\"] {"
        "width: 100%;"
        "padding: 8px;"
        "border: 1px solid #ddd;"
        "border-radius: 4px;"
        "box-sizing: border-box;"
        "}"
        ".toggle-section {"
        "background-color: #f9f9f9;"
        "padding: 15px;"
        "border-radius: 4px;"
        "margin-top: 10px;"
        "display: none;"
        "}"
        ".btn {"
        "display: block;"
        "width: 100%;"
        "padding: 10px;"
        "background-color: #4CAF50;"
        "color: white;"
        "border: none;"
        "border-radius: 4px;"
        "cursor: pointer;"
        "margin-top: 20px;"
        "}"
        ".radio-group {"
        "display: flex;"
        "gap: 15px;"
        "margin-bottom: 10px;"
        "}"
        ".radio-group label {"
        "display: flex;"
        "align-items: center;"
        "gap: 5px;"
        "font-weight: normal;"
        "}");

    html += F("</style>"
        "</head>"
        "<body>"
            "<div class=\"nav-container\">"
                "<div class=\"nav-path\">"
                    "<a href=\"/\" class=\"home-icon\" title=\"Zur Startseite\">🏠</a>"
                    "<span class=\"nav-separator\">/</span>"
                    "<span class=\"nav-current\">Konfiguration</span>"
                "</div>"
            "</div>"
            "<div class=\"container\">"
                "<h1>Konfiguration</h1>"
                "<form id=\"configForm\" method=\"POST\" action=\"/config-confirm\">");

    // WLAN Einstellungen
    html += F("<div class=\"form-group\">"
        "<label for=\"ssid\">WLAN SSID</label>"
        "<input type=\"text\" id=\"ssid\" name=\"ssid\" required value=\"");
    html += escapeHTML(String(config.ssid));
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"psk\">WLAN Passwort</label>"
            "<input type=\"password\" id=\"psk\" name=\"psk\" required value=\"");
    html += escapeHTML(String(config.password));
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"hostname\">Hostname</label>"
            "<input type=\"text\" id=\"hostname\" name=\"hostname\" required "
            "pattern=\"[a-zA-Z0-9\\-]{3,32}\" "
            "title=\"3-32 Zeichen, nur Buchstaben, Zahlen und Bindestriche\" value=\"");
    html += escapeHTML(String(config.hostname));
    html += F("\"></div>");

    // IP Konfiguration
    html += F("<div class=\"form-group\">"
        "<label>IP-Adresse</label>"
        "<div class=\"radio-group\">"
            "<label>"
                "<input type=\"radio\" name=\"ip_type\" value=\"dhcp\"");
    html += !config.staticIP ? F(" checked") : F("");
    html += F(" onclick=\"toggleIPConfig()\">"
                "Automatisch (DHCP)"
            "</label>"
            "<label>"
                "<input type=\"radio\" name=\"ip_type\" value=\"static\"");
    html += config.staticIP ? F(" checked") : F("");
    html += F(" onclick=\"toggleIPConfig()\">"
                "Statisch"
            "</label>"
        "</div>");

    html += F("<div id=\"staticIPSection\" class=\"toggle-section\">"
        "<div class=\"form-group\">"
            "<label for=\"ip\">IP-Adresse</label>"
            "<input type=\"text\" id=\"ip\" name=\"ip\" "
            "pattern=\"^(\\d{1,3}\\.){3}\\d{1,3}$\" "
            "title=\"Gültige IP-Adresse erforderlich\" value=\"");
    html += config.staticIP ? escapeHTML(String(config.ip)) : F("");
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"subnet\">Subnetzmaske</label>"
            "<input type=\"text\" id=\"subnet\" name=\"subnet\" "
            "pattern=\"^(\\d{1,3}\\.){3}\\d{1,3}$\" "
            "title=\"Gültige Subnetzmaske erforderlich\" value=\"");
    html += config.staticIP ? escapeHTML(String(config.subnet)) : F("");
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"gateway\">Gateway</label>"
            "<input type=\"text\" id=\"gateway\" name=\"gateway\" "
            "pattern=\"^(\\d{1,3}\\.){3}\\d{1,3}$\" "
            "title=\"Gültige Gateway-IP erforderlich\" value=\"");
    html += config.staticIP ? escapeHTML(String(config.gateway)) : F("");
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"dns\">DNS Server</label>"
            "<input type=\"text\" id=\"dns\" name=\"dns\" "
            "pattern=\"^(\\d{1,3}\\.){3}\\d{1,3}$\" "
            "title=\"Gültige DNS-IP erforderlich\" value=\"");
    html += config.staticIP ? escapeHTML(String(config.dns)) : F("");
    html += F("\"></div></div></div>");

    // MQTT Konfiguration
    html += F("<div class=\"form-group\">"
        "<label>MQTT Steuerung</label>"
        "<div class=\"radio-group\">"
            "<label>"
                "<input type=\"radio\" name=\"mqtt_enabled\" value=\"no\"");
    html += !config.mqttEnabled ? F(" checked") : F("");
    html += F(" onclick=\"toggleMQTTConfig()\">"
                "Deaktiviert"
            "</label>"
            "<label>"
                "<input type=\"radio\" name=\"mqtt_enabled\" value=\"yes\"");
    html += config.mqttEnabled ? F(" checked") : F("");
    html += F(" onclick=\"toggleMQTTConfig()\">"
                "Aktiviert"
            "</label>"
        "</div>");

    html += F("<div id=\"mqttSection\" class=\"toggle-section\">"
        "<div class=\"form-group\">"
            "<label for=\"mqtt_server\">MQTT Server</label>"
            "<input type=\"text\" id=\"mqtt_server\" name=\"mqtt_server\" "
            "placeholder=\"Hostname oder IP\" value=\"");
    html += config.mqttEnabled ? escapeHTML(String(config.mqttServer)) : F("");
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"mqtt_port\">MQTT Port</label>"
            "<input type=\"number\" id=\"mqtt_port\" name=\"mqtt_port\" "
            "min=\"1\" max=\"65535\" value=\"");
    html += String(config.mqttPort);
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"mqtt_user\">Benutzername</label>"
            "<input type=\"text\" id=\"mqtt_user\" name=\"mqtt_user\" value=\"");
    html += config.mqttEnabled ? escapeHTML(String(config.mqttUser)) : F("");
    html += F("\"></div>"
        "<div class=\"form-group\">"
            "<label for=\"mqtt_pass\">Passwort</label>"
            "<input type=\"password\" id=\"mqtt_pass\" name=\"mqtt_pass\" value=\"");
    html += config.mqttEnabled ? escapeHTML(String(config.mqttPassword)) : F("");
    html += F("\"></div></div></div>");

    // Submit Button und JavaScript
    html += F("<button type=\"submit\" class=\"btn\">Konfiguration speichern</button>"
        "</form></div>"
        "<script>");

    html += F("function toggleIPConfig() {"
        "const staticIPSection = document.getElementById('staticIPSection');"
        "const isStatic = document.querySelector('input[name=\"ip_type\"]:checked').value === 'static';"
        "staticIPSection.style.display = isStatic ? 'block' : 'none';"
        "const requiredFields = staticIPSection.querySelectorAll('input');"
        "requiredFields.forEach(field => {"
            "field.required = isStatic;"
        "});"
    "}"
    "function toggleMQTTConfig() {"
        "const mqttSection = document.getElementById('mqttSection');"
        "const isMQTTEnabled = document.querySelector('input[name=\"mqtt_enabled\"]:checked').value === 'yes';"
        "mqttSection.style.display = isMQTTEnabled ? 'block' : 'none';"
        "const requiredFields = mqttSection.querySelectorAll('input[id=\"mqtt_server\"], input[id=\"mqtt_user\"], input[id=\"mqtt_pass\"]');"
        "requiredFields.forEach(field => {"
            "field.required = isMQTTEnabled;"
        "});"
    "}"
    "toggleIPConfig();"
    "toggleMQTTConfig();");

    html += F("</script></body></html>");

    server.send(200, "text/html", html);
}

// Handler für das Speichern der Konfiguration
void handleConfigSet() {
    // WLAN-Einstellungen
    strlcpy(config.ssid, server.arg("ssid").c_str(), sizeof(config.ssid));
    strlcpy(config.password, server.arg("psk").c_str(), sizeof(config.password));
    strlcpy(config.hostname, server.arg("hostname").c_str(), sizeof(config.hostname));
    
    // IP-Einstellungen
    config.staticIP = (server.arg("ip_type") == "static");
    if (config.staticIP) {
        strlcpy(config.ip, server.arg("ip").c_str(), sizeof(config.ip));
        strlcpy(config.subnet, server.arg("subnet").c_str(), sizeof(config.subnet));
        strlcpy(config.gateway, server.arg("gateway").c_str(), sizeof(config.gateway));
        strlcpy(config.dns, server.arg("dns").c_str(), sizeof(config.dns));
    }
    
    // MQTT-Einstellungen
    config.mqttEnabled = (server.arg("mqtt_enabled") == "yes");
    if (config.mqttEnabled) {
        strlcpy(config.mqttServer, server.arg("mqtt_server").c_str(), sizeof(config.mqttServer));
        config.mqttPort = server.arg("mqtt_port").toInt();
        strlcpy(config.mqttUser, server.arg("mqtt_user").c_str(), sizeof(config.mqttUser));
        strlcpy(config.mqttPassword, server.arg("mqtt_pass").c_str(), sizeof(config.mqttPassword));
    }
    
    // Konfiguration speichern
    saveConfiguration();
    
    // Bestätigung senden und neu starten
    server.send(200, "text/html", "<html><body>"
        "<h1>Konfiguration gespeichert</h1>"
        "<p>Die Einstellungen wurden gespeichert. Das Gerät wird neu gestartet...</p>"
        "<script>setTimeout(function(){ window.location.href = '/'; }, 5000);</script>"
        "</body></html>");
        
    // Verzögerter Neustart
    delay(1000);
    ESP.restart();
}

void handleConfigErase() {
   String html = F("<!DOCTYPE html>"
       "<html lang=\"de\">"
       "<head>"
           "<meta charset=\"UTF-8\">"
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
           "<title>Konfiguration löschen</title>"
           "<style>");

   // CSS
   html += F("body {"
       "font-family: Arial, sans-serif;"
       "max-width: 600px;"
       "margin: 0 auto;"
       "padding: 20px;"
       "background-color: #f4f4f4;"
       "text-align: center;"
       "}"
       ".nav-container {"
       "background-color: white;"
       "padding: 15px;"
       "border-radius: 8px;"
       "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
       "margin-bottom: 20px;"
       "text-align: left;"
       "}"
       ".nav-path {"
       "display: flex;"
       "align-items: center;"
       "gap: 10px;"
       "color: #666;"
       "}"
       ".nav-separator {"
       "color: #999;"
       "}"
       ".nav-link {"
       "text-decoration: none;"
       "color: #2196F3;"
       "transition: color 0.3s ease;"
       "}"
       ".home-icon {"
       "color: #2196F3;"
       "font-size: 18px;"
       "text-decoration: none;"
       "}"
       ".nav-current {"
       "color: #333;"
       "font-weight: bold;"
       "}"
       ".container {"
       "background-color: white;"
       "padding: 30px;"
       "border-radius: 8px;"
       "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
       "}"
       ".warning-icon {"
       "color: #ff9800;"
       "font-size: 80px;"
       "margin-bottom: 20px;"
       "}"
       "h1 {"
       "color: #f44336;"
       "margin-bottom: 20px;"
       "}"
       ".warning-text {"
       "color: #333;"
       "margin-bottom: 30px;"
       "line-height: 1.6;"
       "}"
       ".button-container {"
       "display: flex;"
       "justify-content: center;"
       "gap: 20px;"
       "}"
       ".btn {"
       "display: inline-block;"
       "padding: 12px 25px;"
       "border-radius: 5px;"
       "text-decoration: none;"
       "font-weight: bold;"
       "transition: background-color 0.3s ease;"
       "}"
       ".btn-danger {"
       "background-color: #f44336;"
       "color: white;"
       "}"
       ".btn-danger:hover {"
       "background-color: #d32f2f;"
       "}"
       ".btn-cancel {"
       "background-color: #e0e0e0;"
       "color: #333;"
       "}"
       ".btn-cancel:hover {"
       "background-color: #d5d5d5;"
       "}");

   html += F("</style>"
       "</head>"
       "<body>"
           "<div class=\"nav-container\">"
               "<div class=\"nav-path\">"
                   "<a href=\"/\" class=\"home-icon\" title=\"Zur Startseite\">🏠</a>"
                   "<span class=\"nav-separator\">/</span>"
                   "<span class=\"nav-current\">Konfiguration löschen</span>"
               "</div>"
           "</div>"
           "<div class=\"container\">"
               "<div class=\"warning-icon\">"
                   "⚠️"
               "</div>"
               "<h1>Konfiguration löschen</h1>"
               "<div class=\"warning-text\">"
                   "<p>"
                       "<strong>Achtung!</strong> Wenn Sie die Konfiguration löschen, werden alle "
                       "aktuellen Einstellungen unwiderruflich gelöscht. Der Radiator Fan Manager wird "
                       "in den Werkszustand zurückgesetzt."
                   "</p>"
                   "<p>"
                       "Nach dem Löschen müssen Sie alle Netzwerk- und "
                       "Verbindungseinstellungen neu konfigurieren."
                   "</p>"
               "</div>"
               "<div class=\"button-container\">"
                   "<a href=\"/configerase-confirm\" class=\"btn btn-danger\">"
                       "Konfiguration unwiderruflich löschen"
                   "</a>"
                   "<a href=\"/\" class=\"btn btn-cancel\">"
                       "Abbrechen"
                   "</a>"
               "</div>"
           "</div>"
       "</body>"
       "</html>");

   server.send(200, "text/html", html);
}

void handleConfigEraseConfirm() {
    pref.begin("settings", false);
    pref.clear();
    delay(5000);
    pref.end();
    
    server.send(200, "text/html", "<html><body>"
        "<h1>Konfiguration gelöscht</h1>"
        "<p>Alle Einstellungen wurden zurückgesetzt. Das Gerät wird neu gestartet...</p>"
        "<script>setTimeout(function(){ window.location.href = '/'; }, 5000);</script>"
        "</body></html>");
        
    delay(1000);
    ESP.restart();
}

void handleFirmware() {
    String html = F("<!DOCTYPE html>"
        "<html lang=\"de\">"
        "<head>"
            "<meta charset=\"UTF-8\">"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<title>Firmware-Aktualisierung</title>"
            "<style>");

    // CSS
    html += F("body {"
        "font-family: Arial, sans-serif;"
        "max-width: 600px;"
        "margin: 0 auto;"
        "padding: 20px;"
        "background-color: #f4f4f4;"
        "text-align: center;"
        "}"
        ".nav-container {"
        "background-color: white;"
        "padding: 15px;"
        "border-radius: 8px;"
        "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
        "margin-bottom: 20px;"
        "text-align: left;"
        "}"
        ".nav-path {"
        "display: flex;"
        "align-items: center;"
        "gap: 10px;"
        "color: #666;"
        "}"
        ".nav-separator {"
        "color: #999;"
        "}"
        ".nav-link {"
        "text-decoration: none;"
        "color: #2196F3;"
        "transition: color 0.3s ease;"
        "}"
        ".home-icon {"
        "color: #2196F3;"
        "font-size: 18px;"
        "text-decoration: none;"
        "}"
        ".nav-current {"
        "color: #333;"
        "font-weight: bold;"
        "}"
        ".container {"
        "background-color: white;"
        "padding: 30px;"
        "border-radius: 8px;"
        "box-shadow: 0 4px 6px rgba(0,0,0,0.1);"
        "}"
        ".warning-icon {"
        "color: #ff9800;"
        "font-size: 80px;"
        "margin-bottom: 20px;"
        "}"
        "h1 {"
        "color: #333;"
        "margin-bottom: 20px;"
        "}"
        ".warning-text {"
        "color: #333;"
        "margin-bottom: 30px;"
        "line-height: 1.6;"
        "background-color: #fff3e0;"
        "padding: 15px;"
        "border-radius: 5px;"
        "border-left: 5px solid #ff9800;"
        "text-align: left;"
        "}"
        ".file-upload {"
        "margin-bottom: 30px;"
        "}"
        ".file-upload input[type=\"file\"] {"
        "display: none;"
        "}"
        ".file-upload label {"
        "display: inline-block;"
        "padding: 12px 25px;"
        "background-color: #4CAF50;"
        "color: white;"
        "border-radius: 5px;"
        "cursor: pointer;"
        "transition: background-color 0.3s ease;"
        "}"
        ".file-upload label:hover {"
        "background-color: #45a049;"
        "}"
        "#fileName {"
        "margin-top: 10px;"
        "color: #666;"
        "}"
        ".btn {"
        "display: inline-block;"
        "padding: 12px 25px;"
        "border-radius: 5px;"
        "text-decoration: none;"
        "font-weight: bold;"
        "transition: background-color 0.3s ease;"
        "margin-top: 20px;"
        "}"
        ".btn-primary {"
        "background-color: #2196F3;"
        "color: white;"
        "}"
        ".btn-primary:disabled {"
        "background-color: #b0b0b0;"
        "cursor: not-allowed;"
        "}"
        ".progress-container {"
        "margin-top: 20px;"
        "display: none;"
        "}"
        ".progress-bar {"
        "width: 100%;"
        "background-color: #e0e0e0;"
        "border-radius: 5px;"
        "overflow: hidden;"
        "}"
        ".progress-bar-fill {"
        "width: 0%;"
        "height: 20px;"
        "background-color: #4CAF50;"
        "transition: width 0.5s ease;"
        "}"
        ".progress-text {"
        "margin-top: 10px;"
        "color: #666;"
        "}");

    html += F("</style>"
        "</head>"
        "<body>"
            "<div class=\"nav-container\">"
                "<div class=\"nav-path\">"
                    "<a href=\"/\" class=\"home-icon\" title=\"Zur Startseite\">🏠</a>"
                    "<span class=\"nav-separator\">/</span>"
                    "<span class=\"nav-current\">Firmware aktualisieren</span>"
                "</div>"
            "</div>"
            "<div class=\"container\">"
                "<div class=\"warning-icon\">"
                    "⚠️"
                "</div>"
                "<h1>Firmware-Aktualisierung</h1>"
                "<div class=\"warning-text\">"
                    "<p><strong>Wichtige Warnhinweise:</strong></p>"
                    "<ul>"
                        "<li>Schalten Sie das Gerät während der Aktualisierung NICHT aus.</li>"
                        "<li>Stellen Sie sicher, dass Sie die richtige Firmware-Datei verwenden.</li>"
                        "<li>Der Aktualisierungsvorgang kann einige Minuten dauern.</li>"
                        "<li>Eine Unterbrechung kann das Gerät unbrauchbar machen.</li>"
                    "</ul>"
                "</div>"
                "<div class=\"file-upload\">"
                    "<input type=\"file\" id=\"firmwareFile\" accept=\".bin\" required>"
                    "<label for=\"firmwareFile\">Firmware-Datei auswählen</label>"
                    "<div id=\"fileName\"></div>"
                "</div>"
                "<button id=\"updateBtn\" class=\"btn btn-primary\" disabled>"
                    "Firmware aktualisieren"
                "</button>"
                "<div class=\"progress-container\" id=\"progressContainer\">"
                    "<div class=\"progress-bar\">"
                        "<div class=\"progress-bar-fill\" id=\"progressBarFill\"></div>"
                    "</div>"
                    "<div class=\"progress-text\" id=\"progressText\">"
                        "Aktualisierung: 0%"
                    "</div>"
                "</div>"
            "</div>");

    html += F("<script>"
        "const fileInput = document.getElementById('firmwareFile');"
        "const fileNameDisplay = document.getElementById('fileName');"
        "const updateBtn = document.getElementById('updateBtn');"
        "const progressContainer = document.getElementById('progressContainer');"
        "const progressBarFill = document.getElementById('progressBarFill');"
        "const progressText = document.getElementById('progressText');"

        "fileInput.addEventListener('change', function() {"
            "if (this.files && this.files.length > 0) {"
                "fileNameDisplay.textContent = `Ausgewählte Datei: ${this.files[0].name}`;"
                "updateBtn.disabled = false;"
            "}"
        "});"

        "updateBtn.addEventListener('click', function() {"
            "if (!fileInput.files.length) return;"
            
            "const file = fileInput.files[0];"
            "const formData = new FormData();"
            "formData.append('firmware', file);"
            
            "updateBtn.disabled = true;"
            "progressContainer.style.display = 'block';"
            
            "const xhr = new XMLHttpRequest();"
            "xhr.upload.addEventListener('progress', function(e) {"
                "if (e.lengthComputable) {"
                    "const percent = Math.round((e.loaded / e.total) * 100);"
                    "progressBarFill.style.width = percent + '%';"
                    "progressText.textContent = `Aktualisierung: ${percent}%`;"
                "}"
            "});"
            
            "xhr.onreadystatechange = function() {"
                "if (xhr.readyState === 4) {"
                    "if (xhr.status === 200) {"
                        "progressText.textContent = 'Aktualisierung erfolgreich!';"
                        "setTimeout(() => {"
                            "window.location.href = '/';"
                        "}, 3000);"
                    "} else {"
                        "progressText.textContent = 'Fehler bei der Aktualisierung';"
                        "updateBtn.disabled = false;"
                    "}"
                "}"
            "};"
            
            "xhr.open('POST', '/firmware-update', true);"
            "xhr.send(formData);"
        "});"
        "</script>"
        "</body>"
        "</html>");

    server.send(200, "text/html", html);
}

void handleFirmwareUpdateResponse() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    delay(1000);
    ESP.restart();
}

void handleFirmwareUpdate() {
    static size_t lastProgress = 0;
    HTTPUpload& upload = server.upload();
    
    switch (upload.status) {
        case UPLOAD_FILE_START: {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            
            if (!upload.filename.endsWith(".bin")) {
                Serial.println("Only .bin Files are allowed!");
                Update.end(true);
                return;
            }

            // WiFi-Timeout während Update erhöhen
            WiFi.setSleep(false);
            
            // Flash vollständig löschen vor Update
            if(!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH, LED_BUILTIN, HIGH)) {
                Serial.printf("Not enough free Space: %u\n", upload.totalSize);
                Update.printError(Serial);
                return;
            }
            
            Serial.println("Starting Update...");
            lastProgress = 0;
            break;
        }
        
        case UPLOAD_FILE_WRITE: {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
                return;
            }
            
            // Fortschritt nur ausgeben wenn sich was geändert hat
            size_t progress = (Update.progress() * 100) / Update.size();
            if (progress != lastProgress) {
                lastProgress = progress;
                Serial.printf("Progress: %u%%\r", progress);
            }
            break;
        }
        
        case UPLOAD_FILE_END: {
            if (Update.end(true)) {
                Serial.printf("\nUpdate successful: %u bytes\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
            WiFi.setSleep(true);  // WiFi-Einstellungen zurücksetzen
            break;
        }
        
        case UPLOAD_FILE_ABORTED: {
            Serial.println("Update aborted!");
            Update.end();
            WiFi.setSleep(true);  // WiFi-Einstellungen zurücksetzen
            break;
        }
    }
    
    // Watchdog zurücksetzen
    yield();
}

void handleSetPWM(FanManager& fm) {
   if(!server.hasArg("fan") || !server.hasArg("value")) {
       server.send(400, "text/plain", "Missing parameters");
       return;
   }

   int fanId = server.arg("fan").toInt();
   int value = server.arg("value").toInt();

   if(fanId >= 0 && fanId < fm.getFanCount() && value >= 0 && value <= 100) {
       fm.setPWM(fanId, value);
       server.send(200, "text/plain", "OK");
   } else {
       server.send(400, "text/plain", "Invalid parameters");
   }
}

void handleGetFanData(FanManager& fm) {
    String json = "{";
    for(int i = 0; i < fm.getFanCount(); i++) {
        if(i > 0) json += ",";
        json += "\"fan" + String(i) + "\":{";
        json += "\"rpm\":" + String(fm.getRPM(i)) + ",";
        json += "\"pwm\":" + String(fm.getPWM(i));
        json += "}";
    }
    json += "}";
    server.send(200, "application/json", json);
}

void setupWebServer(FanManager& fm)
{
    // Root-Umleitung (muss als erstes kommen)
    server.on("/", HTTP_GET, [&fm]() {
        if(isConfigMode) {
            server.sendHeader("Location", "/config", true);
            server.send(302, "text/plain", "");
        } else {
            handleIndex(fm);  // Direct call to index handler
        }
    });
    /* PWM Control */
    server.on("/setpwm", HTTP_GET, [&fm]() {
        handleSetPWM(fm);
    });
    /* Fan Data */
    server.on("/getfandata", HTTP_GET, [&fm]() {
        handleGetFanData(fm);
    });

    /* Config */
    server.on("/config", HTTP_GET, handleConfig);
    /* Config Set */
    server.on("/config-confirm", HTTP_POST, handleConfigSet);
    /* Config Erase */
    server.on("/configerase", HTTP_GET, handleConfigErase);
    /* Config Erase Confirm */
    server.on("/configerase-confirm", HTTP_GET, handleConfigEraseConfirm);
    /* Firmware*/
    server.on("/firmware", HTTP_GET, handleFirmware);
    /* Firmware Upload */
    server.on("/firmware-update", HTTP_POST, handleFirmwareUpdateResponse, handleFirmwareUpdate);

    // Im Konfig-Modus: Umleitung aller Anfragen zur Konfigurationsseite
    server.onNotFound([]() {
        if(isConfigMode) {
            // Im Konfig-Modus alle unbekannten Anfragen auf die Konfig-Seite umleiten
            server.sendHeader("Location", "/config", true);
            server.send(302, "text/plain", "");
        } else {
            // Im normalen Modus: Standard 404
            server.send(404, "text/plain", "File Not Found");
        }
    });

    server.begin();
}

#endif