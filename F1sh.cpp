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
 *  For more infomation on F1sh, please visit: https://github.com/stemistclub/F1sh.git
 *
 *
 *  @section author Author
 *
 *  Nguyen Minh Thai (@b4iterdev).
 *
 *  @section license License
 *
 *  Creative Commons Attribution-NonCommercial 4.0 International License.
 */

#include "F1sh.h"


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
     ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
       (void)len;
   
       if (type == WS_EVT_CONNECT) {
         ws.textAll("new client connected");
         Serial.println("ws connect");
         client->setCloseClientOnQueueFull(false);
         client->ping();
   
       } else if (type == WS_EVT_DISCONNECT) {
         ws.textAll("client disconnected");
         Serial.println("ws disconnect");
   
       } else if (type == WS_EVT_ERROR) {
         Serial.println("ws error");
   
       } else if (type == WS_EVT_PONG) {
         Serial.println("ws pong");
   
       } else if (type == WS_EVT_DATA) {
         AwsFrameInfo *info = (AwsFrameInfo *)arg;
         // Serial.printf("index: %" PRIu64 ", len: %" PRIu64 ", final: %" PRIu8 ", opcode: %" PRIu8 "\n", info->index, info->len, info->final, info->opcode);
         // String msg = "";
         if (info->final && info->index == 0 && info->len == len) {
           if (info->opcode == WS_TEXT) {
             data[len] = 0;
             Serial.printf("ws text: %s\n", (char *)data);
   
             // Parse the JSON message
             JsonDocument doc;
             DeserializationError error = deserializeJson(doc, data);
             if (error) {
               Serial.print(F("deserializeJson() failed: "));
               Serial.println(error.f_str());
               return;
             }
   
             // Extract data
             if (!doc["action"].isNull()) {
               const char* action = doc["action"];
               Serial.println(action);
               if (doc["action"] == "gamepad")
               {
                 const int gamepad = doc["gamepad"];
                 Serial.println(gamepad);
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
     });
     server.rewrite("/config", "/index.html");
     server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
     server.addHandler(&ws);
     server.begin();
}
 
void F1sh::setGamepadCallback(GamepadCallback callback) {
     gamepadCallback = callback;
 }
 
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
 
void F1sh::F1shLoop() {
     ws.cleanupClients();
 }
 