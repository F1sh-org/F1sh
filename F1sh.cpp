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
 *  Creative Commons Attribution-NonCommercial 4.0 International License.
 *  All text above must be included in any redistribution.
 */

#include "F1sh.h"

PsychicHttpsServer server;
PsychicWebSocketHandler websocketHandler;


void F1sh::initWiFiAP(const char *ssid,const char *password,const char *hostname, int channel) {
     WiFi.setHostname(hostname);
     WiFi.encryptionType(WIFI_AUTH_WPA2_PSK);
     WiFi.begin(ssid, password);
     WiFi.mode(WIFI_AP);
     WiFi.softAP(ssid,password,(channel >= 1) && (channel <= 13) ? channel : int(random(1, 13)));
     Serial.print("IP address: ");
     Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
 }
 
void F1sh::initWiFiSmart() {
     WiFi.mode(WIFI_STA);
     WiFi.beginSmartConfig();
     //Wait for SmartConfig packet from mobile
     Serial.println("Waiting for SmartConfig.");
     while (!WiFi.smartConfigDone()) {
         delay(500);
         Serial.print(".");
     }
     Serial.println("");
     Serial.println("SmartConfig received.");
     //Wait for WiFi to connect to AP
     Serial.println("Waiting for WiFi");
     while (WiFi.status() != WL_CONNECTED) {
         delay(500);
         Serial.print(".");
     }
     Serial.println("WiFi Connected.");
     Serial.print("IP address: ");
     Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
 }
 
void F1sh::initWebServer() {
    Serial.println("Webserver are being created");
    server.config.max_uri_handlers = 20;
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
            for (size_t i = 0; i < doc["gamepad"].size(); i++) {
              copyArray(doc["gamepad"][i]["axes"],F1sh::gamepad[i].axis);
              //Serial.printf("Gamepad %d: %f %f %f %f\n",i,F1sh::gamepad[i].axis[0],F1sh::gamepad[i].axis[1],F1sh::gamepad[i].axis[2],F1sh::gamepad[i].axis[3]);
          }
          // Bind gamepad buttons to the gamepad object
          for (size_t i = 0; i < doc["gamepad"].size(); i++) {
              copyArray(doc["gamepad"][i]["buttons"],F1sh::gamepad[i].button);
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
            res["action"] = "get";
            res["data"] = "ok";
            return request->reply(res.as<String>().c_str());
          }
        }
      }
      return 0;
  });
  /*
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
       (void)len;
      if (type == WS_EVT_DATA) {
         AwsFrameInfo *info = (AwsFrameInfo *)arg;
         // Serial.printf("index: %" PRIu64 ", len: %" PRIu64 ", final: %" PRIu8 ", opcode: %" PRIu8 "\n", info->index, info->len, info->final, info->opcode);
         // String msg = "";
         if (info->final && info->index == 0 && info->len == len) {
           if (info->opcode == WS_TEXT) {
             data[len] = 0;
             //Serial.printf("ws text: %s\n", (char *)data);
   
             // Parse the JSON message
             JsonDocument doc;
             DeserializationError error = deserializeJson(doc, data);
             if (error) {
               Serial.print(F("deserializeJson() failed: "));
               Serial.println(error.f_str());
               return;
             }
   
             // Extract data
             if(!doc.isNull() && doc.is<JsonObject>()) {
             if (!doc["action"].isNull()) {
               if (doc["action"] == "gamepad")
               {
                 // Bind gamepad axes to the gamepad object
                 for (size_t i = 0; i < doc["gamepad"].size(); i++) {
                    copyArray(doc["gamepad"][i]["axes"],F1sh::gamepad[i].axis);
                    //Serial.printf("Gamepad %d: %f %f %f %f\n",i,F1sh::gamepad[i].axis[0],F1sh::gamepad[i].axis[1],F1sh::gamepad[i].axis[2],F1sh::gamepad[i].axis[3]);
                }
                // Bind gamepad buttons to the gamepad object
                for (size_t i = 0; i < doc["gamepad"].size(); i++) {
                    copyArray(doc["gamepad"][i]["buttons"],F1sh::gamepad[i].button);
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
                 res["action"] = "get";
                 res["data"] = "ok";
                 ws.text(client->id(), res.as<String>());
               }
             }
            }
           }
         }
       }
     });
     */
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
 }
 
/*!
 *  @brief  Start F1sh in SmartConfig mode
 */
void F1sh::F1shInitSmartAP(){
     Serial.println("Starting F1sh in SmartConfig mode");
   #ifdef ESP32
       LittleFS.begin(true);
   #else
       LittleFS.begin();
   #endif
       initWiFiSmart();
       initWebServer();
 }

/*!
 *  @brief  map function but used for float instead of Arduino's map function
 */

float F1sh::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void F1sh::F1shLoop() {
     //ws.cleanupClients();
 }
 