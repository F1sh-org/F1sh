/*!
 *  @file F1sh.cpp
 *
 *
 *  @section intro_sec Introduction
 *
 *  This is a library for controlling a robot using a web interface.
 *
 *  The library is designed to work with the ESP32 and ESP8266 microcontrollers
 *
 *  For more infomation on F1sh, please visit: https://github.com/F1sh-org/F1sh.git
 *
 *
 *  @section author Author
 *
 *  Nguyen Minh Thai (@b4iterdev).
 *
 *  @section license License
 *
 *  GNU General Public License version 3.
 *  All text above must be included in any redistribution.
 *
 */

#include "F1sh.h"

PsychicHttpsServer server;
PsychicWebSocketHandler websocketHandler;
PsychicHttpServer *redirectServer = new PsychicHttpServer();

void F1sh::initWiFiAP(const char *ssid,const char *password,const char *hostname, int channel) {
     WiFi.setHostname(hostname);
     WiFi.encryptionType(WIFI_AUTH_WPA2_PSK);
     WiFi.begin(ssid, password);
     WiFi.mode(WIFI_AP);
     WiFi.softAP(ssid,password,(channel >= 1) && (channel <= 13) ? channel : int(random(1, 13)));
     Serial.print("IP address: ");
     Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
 }

void F1sh::start_mdns_service(){
  //initialize mDNS service
  esp_err_t err = mdns_init();
  if (err) {
      Serial.printf("MDNS Init failed: %d\n", err);
      return;
  }

  //set hostname
  mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
  mdns_hostname_set("F1sh");
  mdns_instance_name_set("F1sh Robot Controller");
}

void F1sh::initWebServer() {
    server.config.max_uri_handlers = 20;
    redirectServer->config.ctrl_port = 20420; // just a random port different from the default one
    redirectServer->listen(80);
    redirectServer->onNotFound([](PsychicRequest *request) {
      String url = "https://" + request->host() + request->url();
      return request->redirect(url.c_str());
    });
    String cert = LittleFS.open("/server.crt","r",false).readString();
    String key = LittleFS.open("/server.key","r",false).readString();
    server.listen(443,cert.c_str(),key.c_str());
    websocketHandler.onOpen([](PsychicWebSocketClient *client) {
      Serial.printf("[socket] connection #%u connected from %s\n", client->socket(), client->remoteIP().toString());
    });
    websocketHandler.onClose([](PsychicWebSocketClient *client) {
      Serial.printf("[socket] connection #%u closed from %s\n", client->socket(), client->remoteIP().toString());
    });
    websocketHandler.onFrame([this](PsychicWebSocketRequest *request, httpd_ws_frame *frame) {
      //Serial.printf("[socket] #%d sent: %s\n", request->client()->socket(), (char *)frame->payload);
      if(frame->type == HTTPD_WS_TYPE_TEXT) {
        //Serial.printf("%s",(char *)frame->payload);
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, (char *)frame->payload);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return 0;
        }
        // Extract data
        if(!doc["action"].isNull() && !doc.isNull() && doc.is<JsonObject>()) {
          if (doc["action"] == "gamepad"){
            // Bind gamepad axes to the gamepad object
            for (size_t i = 0; i < doc["data"].size(); i++) {
              copyArray(doc["data"][i]["axes"],F1sh::gamepad[i].axis);
              //Serial.printf("Gamepad %d: %f %f %f %f\n",i,F1sh::gamepad[i].axis[0],F1sh::gamepad[i].axis[1],F1sh::gamepad[i].axis[2],F1sh::gamepad[i].axis[3]);
          }
          // Bind gamepad buttons to the gamepad object
          for (size_t i = 0; i < doc["data"].size(); i++) {
              copyArray(doc["data"][i]["buttons"],F1sh::gamepad[i].button);
          }
          if (gamepadCallback)
          {
            gamepadCallback();
          }
          }
          if (doc["action"] == "reboot") {
            ESP.restart();
          }
          if (doc["action"] == "get") {
            // send available data
            JsonDocument res;
            JsonDocument data;
            data["freeHeap"] = ESP.getFreeHeap();
            data["freePsram"] = ESP.getFreePsram();
            data["cpuFreq"] = ESP.getCpuFreqMHz();
            data["flashChipSize"] = ESP.getFlashChipSize();
            data["flashChipSpeed"] = ESP.getFlashChipSpeed();
            res["data"] = data;
            Serial.println(res.as<String>());
            return request->reply(res.as<String>().c_str());
          }
        }
      }
      return 0;
  });
     server.on("/ws", &websocketHandler);
}

/*!
 *  @brief  Set the callback function for the gamepad
 */
void F1sh::setGamepadCallback(GamepadCallback callback) {
     gamepadCallback = callback;
 }
 
/*!
 *  @brief  Start F1sh in Access Point mode
 */
void F1sh::F1shInitAP(const char *ssid,const char *password,const char *hostname, int channel) {
     Serial.println("Starting F1sh as an Access Point");
   #ifdef ESP32
     LittleFS.begin(true);
   #else
     LittleFS.begin();
   #endif
    initWiFiAP(ssid,password,hostname,channel);
    initWebServer();
    start_mdns_service();
 }

/*!
 *  @brief  map function but used for float instead of Arduino's map function
 */

float F1sh::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

 
