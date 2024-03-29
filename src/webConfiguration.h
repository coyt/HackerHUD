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

#ifndef WEBCONFIGURATION_H
#define WEBCONFIGURATION_H


/**************************************************************************************************************************/
// External Libraries and Dependencies
/**************************************************************************************************************************/
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <secrets.hpp>


/**************************************************************************************************************************/
// Function Declarations
/**************************************************************************************************************************/
void setupWebConfigurationInterface();
void  killWebConfiguration();
String processor(const String& var);
void saveConfigDataWebConfig(void);
void loadConfigDataWebConfig(void);
void setupTimeAndTimezoneRoutes(void);
void setupWeatherRoutes(void);
void setupCryptoRoutes(void);


#define DEFAULT_DURATION 1000
#define API_KEY_MAX_LEN 100



/**************************************************************************************************************************/
// NTP Server & Time Info defaults - replace with yours:
/**************************************************************************************************************************/
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      -18000 //for EST
#define DAYLIGHT_OFFSET_SEC 3600



/**************************************************************************************************************************/
// Main Configuration Structs
/**************************************************************************************************************************/
typedef struct
{
  char* ntpServer = NTP_SERVER;         
  long gmtOffset_sec = GMT_OFFSET_SEC;               
  int enabled = true;                                      //time on or off
  int daylightOffset_sec = DAYLIGHT_OFFSET_SEC;  
}  time_settings;




typedef struct
{
  char apiKey[API_KEY_MAX_LEN] = WEATHER_API_KEY;         //API key for weather data
  char location[100];                   //city you reside in
  int enabled = true;                  //Weather Frame on or off
  int duration = DEFAULT_DURATION;      //Duration of weather frame
}  weather_frame_settings;




typedef struct
{
  char apiKey[API_KEY_MAX_LEN];         //API key for staking rig status
  bool enabled = false;                 //Frame on or off
  int duration = DEFAULT_DURATION;      //Duration of frame
}  staking_frame_settings;




typedef struct
{ 
  //defines one cryptocurrency frame
  bool enabled = false;                     
  int duration = DEFAULT_DURATION;
  char cryptocurrency_type[40];
}  crypto_frame_settings;

#define DEFAULT_NUM_CRYPTO_FRAMES 6
//defines global crypto frame settings and how many crypto frames exist
typedef struct
{
  char apiKey[API_KEY_MAX_LEN] = CRYPTO_API_KEY;
  char currency_type[40];
  crypto_frame_settings  cryptoFrames [DEFAULT_NUM_CRYPTO_FRAMES];
} Crypto_Config;




//defines what is contained in the stock frame itself
typedef struct
{
  //defines one stock price frame
  bool enabled = false;                     
  int duration = DEFAULT_DURATION;
  char stock_type[40];
}  stock_frame_settings;

#define DEFAULT_NUM_STOCK_FRAMES 6
//defines global stock frame settings and how many stock frames exist
typedef struct
{
  char apiKey[API_KEY_MAX_LEN];
  char currency_type[40];
  stock_frame_settings  stockFrames [DEFAULT_NUM_STOCK_FRAMES];
} Stock_Config;


#define  SETTINGS_FILENAME              F("/settings_cfg.dat")


//end of document 
#endif /* WEBCONFIGURATION_H */