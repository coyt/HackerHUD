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

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

//FIX MEEEEEE!! we likely need to reference the instance of FS from wifimanager

extern FS* filesystem;
#define FileFS        SPIFFS
#define FS_Name       "SPIFFS"


weather_frame_settings weatherFrameSettings;
staking_frame_settings stakingFrameSettings;
Crypto_Config myCryptoConfig;
Stock_Config myStockConfig;
time_settings myTimeConfig;


/**************************************************************************************************************************/
// Website
/**************************************************************************************************************************/
/*
const char index_html[] PROGMEM = R"rawliteral(
  //WEBSITE WAS HERE ORIGINALLY
)rawliteral";
*/



/**************************************************************************************************************************/
// Replaces placeholder with button section in your web page
/**************************************************************************************************************************/
String processor(const String& var){

 return String();

}


/**************************************************************************************************************************/
// setup web configuration interface
/**************************************************************************************************************************/
void setupWebConfigurationInterface(){


    Serial.println("***** Starting Web Configuration Interface *****");

    loadConfigDataWebConfig(); //check flash memory for data and update all frame settings and data

    //HERE WE NEED TO INJECT CURRENTLY SET SETTINGS INTO THE WEBPAGE!!! WRITE A NEW FUNCTION TO MODIFY THE WEBPAGE HERE!
    //After settings have been loaded to webpage - any webserver callbacks require updating of the saved parameters.

    // Route for root / web page
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //request->send_P(200, "text/html", index_html, processor);
        request->send(SPIFFS, "/indexConfig.html", String(), false, processor);
        //UPDATE SAVED DATA IN FLASH
        //UPDATE SETTINGS ON WEBPAGE?
    });

    // Route to load style.css file
    webServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/milligram.css", "text/css");
    });

    // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    webServer.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String inputMessage1;

        // GET weatherFrameEnable
        if (request->hasParam("timeEnable")) {
          inputMessage1 = request->getParam("timeEnable")->value();
          myTimeConfig.enabled = inputMessage1.toInt();
          Serial.print("timeEnable = ");
          Serial.println(myTimeConfig.enabled);
          saveConfigDataWebConfig();
        }
        if (request->hasParam("weatherFrameEnable")) {
          inputMessage1 = request->getParam("weatherFrameEnable")->value();
          weatherFrameSettings.enabled = inputMessage1.toInt();
          Serial.print("weatherFrameEnable = ");
          Serial.println(weatherFrameSettings.enabled);
          saveConfigDataWebConfig();
        }
        else if(request->hasParam("cryptoOneFrameEnable")){
          inputMessage1 = request->getParam("cryptoOneFrameEnable")->value();\
          myCryptoConfig.cryptoFrames[1].enabled = inputMessage1.toInt();
          Serial.print("cyptoOneFrameEnable = ");
          Serial.println(myCryptoConfig.cryptoFrames[1].enabled);
          saveConfigDataWebConfig();
        }
        else {
        inputMessage1 = "No message sent";
        }

        request->send(200, "text/plain", "OK");

        //UPDATE SAVED DATA IN FLASH
        //UPDATE SETTINGS ON WEBPAGE?
    });


    // Send a GET request to <ESP_IP>/
    webServer.on("/cryptoapi", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.print("arrived here! ");
      String cryptoApiKey;
      // GET cryptoApiKey on <ESP_IP>/cryptoapi?cryptoApiKey=<inputVal>
      if (request->hasParam("cryptoApiKey")) {
        cryptoApiKey = request->getParam("cryptoApiKey")->value();
      }
      else {
        //cryptoApiKey = OLD_CRYPTO_KEY; //failed to update crypto api key
        Serial.println("Failed to receive cryptoapi key from browser");
      }
      Serial.print("NEW CRYPTO API KEY: ");
      Serial.println(cryptoApiKey);
      request->send(200, "text/plain", "OK");

      //UPDATE SAVED DATA IN FLASH
      //UPDATE SETTINGS ON WEBPAGE?
    });


    // Send a GET request to ESP32 to update timezone NTP Server
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

    //Browser Request for info - just provide data - don't save
    webServer.on("/timezoneOffset", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myTimeConfig.gmtOffset_sec).c_str());
    });

    //Browser Request for info - just provide data - don't save
    webServer.on("/ntpServer", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myTimeConfig.ntpServer).c_str());
    });

    //Browser Request for info - just provide data - don't save
    webServer.on("/daylightOffset", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myTimeConfig.daylightOffset_sec).c_str());
    });

    //Browser Request for info - just provide data - don't save
    webServer.on("/weatherApiKey", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(weatherFrameSettings.apiKey).c_str());
    });

    //Browser Request for info - just provide data - don't save
    webServer.on("/cryptoApiKey", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(myCryptoConfig.apiKey).c_str());
    });

    // Start server
    webServer.begin();

    Serial.println("***** Finishing Web Configuration Interface from webConfiguration.cpp *****");

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














