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

#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

#include <secrets.hpp>

/**************************************************************************************************************************/
// Wifi Network Credentials - replace with yours:
/**************************************************************************************************************************/
//const char* ssid     = "xxxxxxxx";
//const char* password = "xxxxxxxx";


/**************************************************************************************************************************/
// NTP Server & Time Info - replace with yours:
/**************************************************************************************************************************/
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      -18000 //for EST
#define DAYLIGHT_OFFSET_SEC 3600


/**************************************************************************************************************************/
// Light Sensor Settings & Thresholds:
/**************************************************************************************************************************/
#define LIGHT_THRESHOLD_ONE     3100
#define LIGHT_THRESHOLD_TWO     2000
#define LIGHT_THRESHOLD_THREE   150


/**************************************************************************************************************************/
// API info for BITCOIN cryptocurrency price frame:
/**************************************************************************************************************************/
//#define BITCOIN_SERVER_PATH "https://api.coindesk.com/v1/bpi/currentprice.json"
//#define BITCOIN_CURRENCY "USD"


/**************************************************************************************************************************/
// API info for cryptocurrency price frames:
/**************************************************************************************************************************/
//#define CRYPTO_API_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"


/**************************************************************************************************************************/
// API info for BITCOIN cryptocurrency price frame:
/**************************************************************************************************************************/
//#define WEATHER_SERVER_PATH "http://api.openweathermap.org/data/2.5/weather?q=Tampa,US&APPID=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
//#define WEATHER_API_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                        


#endif /* FIRMWARE_CONFIG_H */