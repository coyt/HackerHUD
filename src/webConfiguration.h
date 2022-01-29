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


/**************************************************************************************************************************/
// Function Declarations
/**************************************************************************************************************************/
void setupWebConfigurationInterface();
void  killWebConfiguration();
String outputState(int output);
String processor(const String& var);
String readDHTHumidity();
String readDHTTemperature();
void saveConfigDataWebConfig(void);
void loadConfigDataWebConfig(void);


#define DEFAULT_DURATION 1000
#define API_KEY_MAX_LEN 100


/**************************************************************************************************************************/
// Main Configuration Structs
/**************************************************************************************************************************/
typedef struct
{
  char apiKey[API_KEY_MAX_LEN];         //API key for weather data
  char location[100];                   //city you reside in
  int enabled = false;                  //Weather Frame on or off
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
  char apiKey[API_KEY_MAX_LEN];
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