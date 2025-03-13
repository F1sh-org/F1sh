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
 *  For more infomation on F1sh, please visit: https://github.com/b4iterdev/F1sh.git
 *
 */

#ifndef _F1sh_H
#define _F1sh_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "AsyncJson.h"
#include <ArduinoJson.h>


static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

typedef void (*GamepadCallback)(int[][], bool[][]);
static GamepadCallback gamepadCallback = nullptr;

/*!
 *  @brief  Class that stores state and functions for interacting with F1sh.
 */
class F1sh {
    public:
    void F1shInitAP(const char *ssid,const char *password,const char *hostname, int channel);
    void F1shInitSmartAP();
    void F1shLoop();
    void setGamepadCallback(GamepadCallback callback)
    private:
    void initWiFiAP(const char *ssid,const char *password,const char *hostname, int channel);
    void initWiFiSmart();
    void initWebServer();
}
#endif