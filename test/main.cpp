#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <FanManager.h>
#include <Update.h>
#include <ServerHandles/ServerHandles.h>
#include <nvs_flash.h>
#include <Preferences.h>

#define TOPIC_MAXLEN 60

WebServer server(80);
WiFiClient wificlient;
PubSubClient mqttclient(wificlient);

FanManager fm;

Preferences config;
char ssid[63];
char psk[63];
char hostname[20];
IPAddress ip;
IPAddress subnet;
IPAddress gw;
IPAddress dns;
char mqttsrv[20];
char mqttuser[20];
char mqttpass[20];
char mqtttopic[TOPIC_MAXLEN];

bool setupMode = false;
bool staticIPMode = false;
bool mqttMode = false;
uint64_t t1;

bool checkValue(char* value)
{
  if(value[0] == '\0')
    return false;
  return true;
}

void writeConfig()
{
  char ssid[63];
  char psk[63];
  char hostname[20];
  char ip[17];
  char subnet[17];
  char gw[17];
  char dns[17];
  char mqttsrv[20];
  char mqttuser[20];
  char mqttpass[30];
  server.arg("ssid").toCharArray(ssid, sizeof(ssid));
  server.arg("psk").toCharArray(psk, sizeof(psk));
  server.arg("hostname").toCharArray(hostname, sizeof(hostname));
  server.arg("ip").toCharArray(ip, sizeof(ip));
  server.arg("subnet").toCharArray(subnet, sizeof(subnet));
  server.arg("gw").toCharArray(gw, sizeof(gw));
  server.arg("dns").toCharArray(dns, sizeof(dns));
  server.arg("mqttsrv").toCharArray(mqttsrv, sizeof(mqttsrv));
  server.arg("mqttuser").toCharArray(mqttuser, sizeof(mqttuser));
  server.arg("mqttpass").toCharArray(mqttpass, sizeof(mqttpass));
  if(checkValue(ssid))
    config.putString("ssid", ssid);
  if(checkValue(psk))
    config.putString("psk", psk);
  if(checkValue(hostname))
    config.putString("hostname", hostname);
  if(checkValue(ip))
    config.putString("ip", ip);  
  if(checkValue(subnet))
    config.putString("subnet", subnet);
  if(checkValue(gw))
    config.putString("gw", gw);  
  if(checkValue(dns))
    config.putString("dns", dns);
  if(checkValue(mqttsrv))
    config.putString("mqttsrv", mqttsrv);
  if(checkValue(mqttuser))
    config.putString("mqttuser", mqttuser);
  if(checkValue(mqttpass))
    config.putString("mqttpass", mqttpass);
}

void mqttCallback(const char* topic, uint8_t* payload, uint length)
{
  char tmptopic[TOPIC_MAXLEN + 50];
  for(uint8_t i = 0; i < fm.getFanCount(); ++i)
  {
    sprintf(tmptopic, "%s%s%d%s", mqtttopic, "/fan", i, "/pwm/set");
    if(strcmp(topic, tmptopic) == 0)
    {
      char buffer[length+1];
      uint8_t pwm;
      memcpy(buffer, payload, length);
      buffer[length] = '\0';
      pwm = atoi(buffer);
      if(pwm > 100)
        pwm = 100;
      fm.setPWM(i, pwm);
      break;
    }
  }
}

void mqttPublish()
{
  for(uint8_t i = 0; i < fm.getFanCount(); ++i)
  {
    char tmptopic[TOPIC_MAXLEN + 50];
    char tmpvalue[20];
    uint16_t rpm = fm.getRPM(i);
    uint8_t pwm = fm.getPWM(i);
    sprintf(tmpvalue, "%d", rpm);
    sprintf(tmptopic, "%s%s%d%s", mqtttopic, "/fan", i, "/rpm");
    mqttclient.publish(tmptopic, tmpvalue);
    sprintf(tmpvalue, "%d", pwm);
    sprintf(tmptopic, "%s%s%d%s", mqtttopic, "/fan", i, "/pwm");
    mqttclient.publish(tmptopic, tmpvalue);
  }
}

void mqttSubscribe()
{
  for(uint8_t i = 0; i < fm.getFanCount(); ++i)
  {
    char tmptopic[TOPIC_MAXLEN + 50];
    sprintf(tmptopic, "%s%s%d%s", mqtttopic, "/fan", i, "/pwm/set");
    mqttclient.subscribe(tmptopic);
  }
}

void setup(void) {
  Serial.begin(115200);

  /* Read Config */
  Serial.println(F("Reading Settings..."));
  config.begin("settings");
  config.getString("ssid", ssid, sizeof(ssid));
  config.getString("psk", psk, sizeof(psk));
  config.getString("hostname", hostname, sizeof(hostname));
  if(ip.fromString(config.getString("ip", "")) && gw.fromString(config.getString("gw", "")) && subnet.fromString(config.getString("subnet", "")) && dns.fromString(config.getString("dns", "")))
    staticIPMode = true;
  config.getString("mqttsrv", mqttsrv, sizeof(mqttsrv));
  config.getString("mqttuser", mqttuser, sizeof(mqttuser));
  config.getString("mqttpass", mqttpass, sizeof(mqttpass));

  if(ssid[0] == '\0' || psk[0] == '\0')
  {
    Serial.println(F("... not found!"));
    setupMode = true;
    goto SETUPMODE;                           // JUMP
  }

  /* Setting up Wifi */
  Serial.println(F("Starting Wifi..."));
  t1 = millis();
  setupMode = true;
  if(staticIPMode)
  {
    Serial.println(F("Static IP Mode!"));
    WiFi.config(ip, gw, subnet, dns, dns);
    if(hostname != "")
    {
      WiFi.setHostname(hostname);
    }
  }
  WiFi.begin(ssid, psk);
  while(millis() - t1 < 30000)
  {
    if(WiFi.status() == WL_CONNECTED)
    {
      Serial.println(F("... completed!"));
      setupMode = false;
      break;
    }
    Serial.print(".");
    delay(1000);
  }
  if(setupMode)
    goto SETUPMODE;                               // JUMP
  Serial.print(F("Connected to "));
  Serial.println(ssid);
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  /* Setting up Server Handles */
  /* Index */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /* Config */
  server.on("/config", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverConfig);
  });
  /* Config Set */
  server.on("/configset", HTTP_POST, []() {
    writeConfig();
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverConfigSend);
  });
  /* Config Erase */
  server.on("/configerase", HTTP_GET, []() {
    config.clear();
    delay(5000);
    config.begin("settings");
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverConfigSend);
  });
  /* Firmware */
  server.on("/firmware", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverFirmwareUpload);
  });
  /* Firmware Uploade */
  server.on("/firmwareupdate", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  /* Setting up FanManager */
  fm.addFan(0, 5, true, true);
  fm.addFan(1, 6, true, true);
  fm.addFan(3, 7, true, true);
  fm.addFan(4, 10, true, true);

  /* Setting up MQTT */
  if(mqttsrv[0] == '\0' || mqttuser[0] == '\0' || mqttpass[0] == '\0' || hostname[0] == '\0')
  {
    Serial.println(F("MQTT Error: Server, User, Pass or Hostname is empty!"));
    goto FINISH;
  }
  else
  {
    mqttclient.setServer(mqttsrv, 1883);
    mqttclient.setCallback(mqttCallback);
    sprintf(mqtttopic, "%s%s", "radiatorfanmanager/", hostname);
    if(!mqttclient.connected())
    {
      t1 = millis();
      Serial.print(F("Trying to connect to MQTT Server... "));
      while(millis() - t1 < 30000)
      {
        if(mqttclient.connect(hostname, mqttuser, mqttpass))
        {
          mqttMode = true;
          Serial.println(F("completed!"));
          break;
        }
      }
      if(!mqttMode)
      {
        Serial.println(F("failed!"));
        goto FINISH;                              // JUMP
      }
    }
    /* Publish Topics */
    char tmpip[50];
    sprintf(tmpip, "%s%s%s", mqtttopic, "/ip/", WiFi.localIP());
    mqttclient.publish(tmpip, "0");
    for(uint8_t i = 0; i < fm.getFanCount(); ++i)
    {
      char tmptopic[TOPIC_MAXLEN + 50];
      sprintf(tmptopic, "%s%s%d%s", mqtttopic, "/fan", i, "/rpm");
      mqttclient.publish(tmptopic, "0");
      sprintf(tmptopic, "%s%s%d%s", mqtttopic, "/fan", i, "/pwm");
      mqttclient.publish(tmptopic, "0");
      sprintf(tmptopic, "%s%s%d%s", mqtttopic, "/fan", i, "/pwm/set");
      mqttclient.publish(tmptopic, "0");
    }
    mqttSubscribe();
  }

  goto FINISH;
  
  /* Setup Mode */
  SETUPMODE:
    WiFi.mode(WIFI_AP);
    WiFi.softAP("RadiatorFanManager", NULL);
    Serial.print("IP: "); Serial.println(WiFi.softAPIP());
    
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverConfig);
    });
    server.on("/configset", HTTP_POST, []() {
      writeConfig();
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverConfigSend);
    });
    server.begin();
    while(true)
    {
      server.handleClient();
      delay(1);
    }
  
  FINISH:
    Serial.println(F("Setup finished!"));
    server.begin();
    t1 = millis();
}

void loop(void) {
  server.handleClient();
  fm.handle();
  if(millis() - t1 > 1000)
  {
    if(!mqttclient.connected())
    {
      if(mqttclient.connect(mqttsrv, mqttuser, mqttpass))
        mqttSubscribe();
    }
    else
    {
      mqttclient.loop();
      mqttPublish();
    }
    t1 = millis();
  }
  delay(1);
}