/*
MIT License

Copyright (c) 2020 Coyt Barringer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef FIRMWARE_H
#define FIRMWARE_H

/**************************************************************************************************************************/
// External Libraries and Dependencies
/**************************************************************************************************************************/
#include "firmware_config.hpp"
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "time.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESPAsync_WiFiManager.h>
//#include <NTPClient.h>
//#include <WiFiUdp.h>
//#include "DHT.h"

/**************************************************************************************************************************/
// Pin Mapping
/**************************************************************************************************************************/
#define VFD_UART_TXO    4
#define VFD_UART_RXI    33
#define BUTTON_ONE      35
#define BUTTON_TWO      32
#define LIGHT_SENSOR    36
#define BUZZER          25 //34


/**************************************************************************************************************************/
// State Machine Timing Values
/**************************************************************************************************************************/
#define INTERVAL_ONE    100
#define INTERVAL_TWO    1000
#define INTERVAL_THREE  10000
#define INTERVAL_FOUR   900000


/**************************************************************************************************************************/
// Main Firmware Function Declarations
/**************************************************************************************************************************/
void setupVFD();
void setupGPIO(); 
void connectWifi();
void updateDisplaySettings();
void updateDisplayFixedItems();
void updateDisplayFrames();
void updateDisplayBrightness();
void updateDisplayTime();
void buttonOneISR();
void buttonTwoISR();
void getAndParseCryptoPrice();
void getAndParseWeather();
void parseCryptoJson( String json );
void parseWeatherJson( String json );
void scrollWeatherDataDuringFrame();


void setupWiFiConfigManager();
void saveConfigData(void);
void loadConfigData(void);
void check_status(void);
void check_WiFi(void);
void heartBeatPrint(void);
uint8_t connectMultiWiFi(void);
void doWifiManagerNonsense(void);


/**************************************************************************************************************************/
// Custom Type Definitions
/**************************************************************************************************************************/
typedef struct { 
    float temperature;
    float humidity;
    String condition;
    String conditionDetailed;
} weatherData;

typedef struct { 
    float ethereumPrice;
    float bitcoinPrice;
} cryptoData;

typedef struct { 
    unsigned long timerOne;
    unsigned long timerTwo;
    unsigned long timerThree;
    unsigned long timerFour;
} stateMachineTimers;


/**************************************************************************************************************************/
// Settings for ESPAsync_WifiManager
// see Async_ConfigOnSwitch example from khoih-prog on github
// https://github.com/khoih-prog/ESPAsync_WiFiManager/blob/master/examples/Async_ConfigOnSwitch/Async_ConfigOnSwitch.ino
/**************************************************************************************************************************/
  #define USE_SPIFFS      true

#if USE_SPIFFS
    #include <SPIFFS.h>
    FS* filesystem =      &SPIFFS;
    #define FileFS        SPIFFS
    #define FS_Name       "SPIFFS"
#else
    // +Use FFat
    #include <FFat.h>
    FS* filesystem =      &FFat;
    #define FileFS        FFat
    #define FS_Name       "FFat"
#endif
//////

// From v1.1.1
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

//macro
#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

//#define LED_BUILTIN       2
//#define LED_ON            HIGH
//#define LED_OFF           LOW

//set WiFi manager trigger button pin
#define TRIGGER_PIN = BUTTON_ONE;

// SSID and PW for Config Portal
String ssid = "ESP_" + String(ESP_getChipId(), HEX);
const char* password = "your_password";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// From v1.1.1
// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM       true
#define FORMAT_FILESYSTEM         false

#define MIN_AP_PASSWORD_SIZE    8

#define SSID_MAX_LEN            32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
  String wifi_ssid;
  String wifi_pw;
}  WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS      2

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
} WM_Config;

WM_Config         WM_config;

#define  CONFIG_FILENAME              F("/wifi_cred.dat")
//////


// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
  #if defined(USE_DHCP_IP)
    #undef USE_DHCP_IP
  #endif
  #define USE_DHCP_IP     true
#else
  // You can select DHCP or Static IP here
  #define USE_DHCP_IP     true
  //#define USE_DHCP_IP     false
#endif


#if ( USE_DHCP_IP || ( defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP ) )
  // Use DHCP
  #warning Using DHCP IP
  IPAddress stationIP   = IPAddress(0, 0, 0, 0);
  IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#else
  // Use static IP
  #warning Using static IP
  
  #ifdef ESP32
    IPAddress stationIP   = IPAddress(192, 168, 2, 232);
  #else
    IPAddress stationIP   = IPAddress(192, 168, 2, 186);
  #endif
  
  IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

/*
IPAddress stationIP   = IPAddress(192, 168, 2, 186);
IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
*/

IPAddress dns1IP      = gatewayIP;
IPAddress dns2IP      = IPAddress(8, 8, 8, 8);

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

#define HTTP_PORT           80

AsyncWebServer webServer(HTTP_PORT);
DNSServer dnsServer;

//end of document 
#endif /* FIRMWARE_H */