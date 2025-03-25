/*!
 *  @file F1sh.h
 *
 *  This is the header file for F1sh, a library for controlling a robot using a web interface.
 *
 *  The library is designed to work with the ESP32 and ESP8266 microcontrollers.
 *
 *  Released under the Creative Commons Attribution-NonCommercial 4.0 International License.
 *
 *  Created by B4iter (@b4iterdev).
 *  Stemist Club logo and all associated assets are property of Stemist Club. All rights reserved.
 *  For more infomation on F1sh, please visit: https://github.com/F1sh-org/F1sh.git
 *  All text above must be included in any redistribution.
 */

 #ifndef _F1sh_H
 #define _F1sh_H
 
 #include <Arduino.h>
 #include <WiFi.h>
 #include <LittleFS.h>
 #include <ArduinoJson.h>
 #include <PsychicHttp.h>
 #include <PsychicHttpsServer.h>
 
 extern PsychicHttpsServer server;
 extern PsychicWebSocketHandler websocketHandler;
 
 typedef void (*GamepadCallback)();
 static GamepadCallback gamepadCallback = nullptr;
 
 /*!
  *  @brief  Class that stores state and functions for interacting with F1sh.
  */
 class F1sh {
    private:
     struct Gamepad
     {
        float axis[4];
        float button[17];
     };
     void initWiFiAP(const char *ssid,const char *password,const char *hostname, int channel);
     void initWiFiSmart();
     void initWebServer();
     public:
     F1sh() {     // Constructor

        }
     void F1shInitAP(const char *ssid,const char *password,const char *hostname, int channel);
     void F1shInitSmartAP();
     void F1shLoop();
     void setGamepadCallback(GamepadCallback callback);
     float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
     Gamepad gamepad[4];
 };
 #endif