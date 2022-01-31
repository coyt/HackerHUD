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

#include "webConfiguration.h"
#include <SPIFFS.h>

extern AsyncWebServer webServer; //the webserver is actually defined in the WiFiManager - DONT TRY TO MAKE A SECOND WEBSERVER HERE

extern FS* filesystem;
#define FileFS        SPIFFS
#define FS_Name       "SPIFFS"

weather_frame_settings weatherFrameSettings;
staking_frame_settings stakingFrameSettings;
Crypto_Config myCryptoConfig;
Stock_Config myStockConfig;
time_settings myTimeConfig;


/**************************************************************************************************************************/
// setup web configuration interface
/**************************************************************************************************************************/
void setupWebConfigurationInterface(){

    Serial.println("***** Starting Web Configuration Interface *****");

    loadConfigDataWebConfig(); //check flash memory for data and update all frame settings and data

    //Route for root / web page
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/indexConfig.html", String(), false);
    });

    // Route to load main css file
    webServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/milligram.css", "text/css");
    });

    // Routes to time and timezone settings
    setupTimeAndTimezoneRoutes();

    // Routes to weather data settings
    setupWeatherRoutes();

    // Routes to crypto settings
    setupCryptoRoutes();

    // Start server
    webServer.begin();

    Serial.println("***** Finishing Web Configuration Interface from webConfiguration.cpp *****");

}


/**************************************************************************************************************************/
// setup the webserver routes / callback functions for Time and TimeZone data
/**************************************************************************************************************************/
void  setupTimeAndTimezoneRoutes(){

    // Browser sends bool value to ESP32 indicating checkbox change
    webServer.on("/timeBool", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String inputMessage;
      if (request->hasParam("timeEnable")) {
        inputMessage = request->getParam("timeEnable")->value();
        myTimeConfig.enabled = inputMessage.toInt();
        saveConfigDataWebConfig();
      }
      else {
        Serial.println("Failed to receive timeBool");
      }
      request->send(200, "text/plain", "OK");
    });


    // Send a GET request to ESP32 to update timezone offset
    webServer.on("/timezoneoffsetRX", HTTP_GET, [](AsyncWebServerRequest *request){
      String localBuffer;
      if (request->hasParam("timezoneOffset")) {
        localBuffer = request->getParam("timezoneOffset")->value();
        Serial.print("*** just got new timezone offset val: ");
      }
      else {
        Serial.println("Failed to receive weatherapi key from browser");


      }

      myTimeConfig.gmtOffset_sec = localBuffer.toInt();
      Serial.print(myTimeConfig.gmtOffset_sec);
      Serial.println(" ***");

      /*
      //copy the arduino "String" into the byte array
      if (strlen(localBuffer.c_str()) < sizeof(myTimeConfig.gmtOffset_sec) - 1)
        strcpy(myTimeConfig.gmtOffset_sec, localBuffer.c_str());
      else
        //fill buffer as much as it can
        strncpy(myTimeConfig.gmtOffset_sec, localBuffer.c_str(), sizeof(myTimeConfig.gmtOffset_sec) - 1);
      */

      //Update Saved / Persistent Flash Data
      saveConfigDataWebConfig(); 

      //Update key on webpage so state is reflected
      request->send_P(200, "text/plain", String(myTimeConfig.gmtOffset_sec).c_str());
    });


    // Send a GET request to ESP32 to update timezone offset
    webServer.on("/ntpServerRX", HTTP_GET, [](AsyncWebServerRequest *request){
      String localBuffer;
      if (request->hasParam("ntpServer")) {
        localBuffer = request->getParam("ntpServer")->value();
      }
      else {
        Serial.println("Failed to receive weatherapi key from browser");
      }

      //copy the arduino "String" into the byte array
      if (strlen(localBuffer.c_str()) < sizeof(myTimeConfig.ntpServer) - 1)
        strcpy(myTimeConfig.ntpServer, localBuffer.c_str());
      else
        //fill buffer as much as it can
        strncpy(myTimeConfig.ntpServer, localBuffer.c_str(), sizeof(myTimeConfig.ntpServer) - 1);
      
      //Update Saved / Persistent Flash Data
      saveConfigDataWebConfig(); 

      //Update key on webpage so state is reflected
      request->send_P(200, "text/plain", String(myTimeConfig.ntpServer).c_str());
    });


    // Send a GET request to ESP32 to update timezone offset
    webServer.on("/daylightOffsetSecondsRX", HTTP_GET, [](AsyncWebServerRequest *request){
      String localBuffer;
      if (request->hasParam("daylightOffsetSeconds")) {
        localBuffer = request->getParam("daylightOffsetSeconds")->value();
        Serial.print("*** just got new daylight offset: ");
      }
      else {
        Serial.println("Failed to receive weatherapi key from browser");
      }

      myTimeConfig.daylightOffset_sec = localBuffer.toInt();
      Serial.print(myTimeConfig.daylightOffset_sec);
      Serial.println(" ***");

      /*
      //copy the arduino "String" into the byte array
      if (strlen(localBuffer.c_str()) < sizeof(myTimeConfig.daylightOffset_sec) - 1)
        strcpy(myTimeConfig.daylightOffset_sec, localBuffer.c_str());
      else
        //fill buffer as much as it can
        strncpy(myTimeConfig.daylightOffset_sec, localBuffer.c_str(), sizeof(myTimeConfig.daylightOffset_sec) - 1);
      */

      //Update Saved / Persistent Flash Data
      saveConfigDataWebConfig(); 

      //Update key on webpage so state is reflected
      request->send_P(200, "text/plain", String(myTimeConfig.daylightOffset_sec).c_str());
    });


    //send current value to browser
    webServer.on("/timezoneOffsetSync", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myTimeConfig.gmtOffset_sec).c_str());
    });

    //send current value to browser
    webServer.on("/ntpServerSync", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myTimeConfig.ntpServer).c_str());
    });

    //send current value to browser
    webServer.on("/daylightOffsetSync", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myTimeConfig.daylightOffset_sec).c_str());
    });

    //send current value to browser
    webServer.on("/timeEnableSync", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myTimeConfig.enabled).c_str());
    });
}


/**************************************************************************************************************************/
// setup the webserver routes / callback functions for Weather Data
/**************************************************************************************************************************/
void  setupWeatherRoutes(){


    // Browser sends bool value to ESP32 indicating checkbox change
    webServer.on("/weatherBool", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String inputMessage;
      if (request->hasParam("weatherEnable")) {
        inputMessage = request->getParam("weatherEnable")->value();
        weatherFrameSettings.enabled = inputMessage.toInt();
        saveConfigDataWebConfig();
      }
      else {
        Serial.println("Failed to receive weatherBool");
      }
      request->send(200, "text/plain", "OK");
    });


    // Send a GET request to ESP32 to update weather API key
    webServer.on("/weatherapi", HTTP_GET, [](AsyncWebServerRequest *request){
      String weatherApiKey;
      if (request->hasParam("weatherApiKey")) {
        weatherApiKey = request->getParam("weatherApiKey")->value();
      }
      else {
        Serial.println("Failed to receive weatherapi key from browser");
      }

      //copy the arduino "String" into the byte array
      if (strlen(weatherApiKey.c_str()) < sizeof(weatherFrameSettings.apiKey) - 1)
        strcpy(weatherFrameSettings.apiKey, weatherApiKey.c_str());
      else
        //fill buffer as much as it can
        strncpy(weatherFrameSettings.apiKey, weatherApiKey.c_str(), sizeof(weatherFrameSettings.apiKey) - 1);
      
      //Update Saved / Persistent Flash Data
      saveConfigDataWebConfig(); 

      //Update key on webpage so state is reflected
      request->send_P(200, "text/plain", String(weatherFrameSettings.apiKey).c_str());
    });


    //send current value to browser
    webServer.on("/weatherApiSync", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(weatherFrameSettings.apiKey).c_str());
    });

    //send current value to browser
    webServer.on("/weatherEnableSync", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(weatherFrameSettings.enabled).c_str());
    });

}


/**************************************************************************************************************************/
// setup the webserver routes / callback functions for Time and TimeZone data
/**************************************************************************************************************************/
void setupCryptoRoutes(){

    // Send a GET request to <ESP_IP>/
    webServer.on("/cryptoapi", HTTP_GET, [](AsyncWebServerRequest *request){
      String cryptoApiKey;
      if (request->hasParam("cryptoApiKey")) {
        cryptoApiKey = request->getParam("cryptoApiKey")->value();
      }
      else {
        Serial.println("Failed to receive cryptoapi key from browser");
      }
      request->send_P(200, "text/plain", String(myCryptoConfig.apiKey).c_str());

      //UPDATE SAVED DATA IN FLASH
      //UPDATE SETTINGS ON WEBPAGE?
    });


    //send current value to browser
    webServer.on("/cryptoApiKey", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myCryptoConfig.apiKey).c_str());
    });

}

/**************************************************************************************************************************/
// Kill the web configuration service to allow the wifi manager to take prescedence
/**************************************************************************************************************************/
void  killWebConfiguration(){
  webServer.reset();
}


/**************************************************************************************************************************/
// saveConfigData function
/**************************************************************************************************************************/
void saveConfigDataWebConfig(void)
{
  File file = FileFS.open(SETTINGS_FILENAME, "w");

  if (file)
  {
    file.write((uint8_t*) &weatherFrameSettings, sizeof(weatherFrameSettings));
    //file.write((uint8_t*) &stakingFrameSettings, sizeof(stakingFrameSettings));
    //file.write((uint8_t*) &myCryptoConfig, sizeof(myCryptoConfig));
    //file.write((uint8_t*) &myStockConfig, sizeof(myStockConfig));
    file.write((uint8_t*) &myTimeConfig, sizeof(myTimeConfig));
    file.close();
    //ok!
  }
  else
  {
    //error
  }
}



/**************************************************************************************************************************/
// loadConfigData function
/**************************************************************************************************************************/
void loadConfigDataWebConfig(void)
{
  File file = FileFS.open(SETTINGS_FILENAME, "r");

  if (file)
  {
    file.readBytes((char *) &weatherFrameSettings, sizeof(weatherFrameSettings));
    //file.readBytes((char *) &stakingFrameSettings, sizeof(stakingFrameSettings));
    //file.readBytes((char *) &myCryptoConfig, sizeof(myCryptoConfig));
    //file.readBytes((char *) &myStockConfig, sizeof(myStockConfig));
    file.readBytes((char *) &myTimeConfig, sizeof(myTimeConfig));
    file.close();
    //ok!
  }
  else
  {
    //error
  }
}














