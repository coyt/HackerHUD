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
#include <WiFi.h>
#include "time.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
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


#endif /* FIRMWARE_H */