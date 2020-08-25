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


/**************************************************************************************************************************/
// dependencies:
/**************************************************************************************************************************/
#include <Arduino.h>
#include "firmware.hpp"



/**************************************************************************************************************************/
// misc variables:
/**************************************************************************************************************************/
const char* ntpServer = NTP_SERVER;
const long  gmtOffset_sec = GMT_OFFSET_SEC;
const int   daylightOffset_sec = DAYLIGHT_OFFSET_SEC;
unsigned long counter = 0;
bool colonOnLastLoop = false;
volatile bool buttonOnePressed = false;
volatile bool buttonTwoPressed = false;
bool scrollWeatherDataFlag = false;
int frameCycler = 0;
bool firstBoot = false;

stateMachineTimers myStateMachineTimers;
weatherData myWeatherData;
cryptoData myCryptoData;

unsigned long scrollWeatherDataTimer = 0;
unsigned long weatherScrollInterval = 0;
String completeWeatherData = "";
int stringWeatherDataLength = 0;
int weatherDataPosition = 0;

// setting PWM properties
//const int freq = 440;
//const int channel = 0;
//const int resolution = 8;
 

/**************************************************************************************************************************/
// objects:
/**************************************************************************************************************************/
WiFiClient client;


/**************************************************************************************************************************/
// setup runs once:
/**************************************************************************************************************************/
void setup() {

  Serial.begin(9600);                                       //setup serial to the PC
  setupGPIO();                                              //setup miscellaneous GPIO pins if needed
  setupVFD();                                               //setup the VFD
  connectWifi();                                            //make ESP32 connect to wifi
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //init time & connect to NTP server
  firstBoot = true;
}


/**************************************************************************************************************************/
// ISR functions:
/**************************************************************************************************************************/
void buttonOneISR(){
  buttonOnePressed = true;
}


void buttonTwoISR(){
  buttonTwoPressed = true;
}


/**************************************************************************************************************************/
// loop runs continuously and handles main state machine:
/**************************************************************************************************************************/
void loop() {


  if(millis() - myStateMachineTimers.timerOne >= INTERVAL_ONE){
    myStateMachineTimers.timerOne = millis();

    //update display settings
    updateDisplaySettings();
  }

  
  if(millis() - myStateMachineTimers.timerTwo >= INTERVAL_TWO){
    myStateMachineTimers.timerTwo = millis();

    //update display fixed items
    updateDisplayFixedItems();
  }
  

  
  if(millis() - myStateMachineTimers.timerThree >= INTERVAL_THREE){
    myStateMachineTimers.timerThree = millis();
    //update display frames
    updateDisplayFrames();
  }

  
  //update crypto prices
  if(firstBoot || millis() - myStateMachineTimers.timerFour >= INTERVAL_FOUR){
    firstBoot = false;
    myStateMachineTimers.timerFour = millis();
    getAndParseCryptoPrice();
    getAndParseWeather();
  }
  

  //for scroll items - we must handle in main loop to avoid delays
  if(scrollWeatherDataFlag && millis() - scrollWeatherDataTimer >= weatherScrollInterval){
    scrollWeatherDataTimer = millis();
    scrollWeatherDataDuringFrame();
    weatherDataPosition++;
    //scrollWeatherDataFlag = false;
  }


  //hardware test for button one
  if(buttonOnePressed){
    buttonOnePressed = false;

    //clear the VFD
    Serial2.write('\x0C');

    Serial2.println("BUTTON ONE PRESSED");

    delay(2000);
  }


  //hardware test for button two
  if(buttonTwoPressed){
    buttonTwoPressed = false;

    //clear the VFD
    Serial2.write('\x0C');

    Serial2.println("BUTTON TWO PRESSED");

    delay(2000);

  }



}


/**************************************************************************************************************************/
// Setup miscellaneous GPIO if needed
/**************************************************************************************************************************/
void setupGPIO(){

  //button one
  pinMode(BUTTON_ONE, INPUT_PULLDOWN);
  attachInterrupt(BUTTON_ONE, buttonOneISR, RISING);

  //button two
  pinMode(BUTTON_TWO, INPUT_PULLDOWN);
  attachInterrupt(BUTTON_TWO, buttonTwoISR, RISING);

  // configure LED PWM functionalitites
  //ledcSetup(channel, freq, resolution);

  // attach the channel to the GPIO to be controlled
  //ledcAttachPin(BUZZER, channel);

  //actually start playing a tone
  //ledcWrite(channel, 200);


}


/**************************************************************************************************************************/
// Setup the VFD
/**************************************************************************************************************************/
void setupVFD(){

  //setup the VFD
  Serial2.begin(9600, SERIAL_8N1, VFD_UART_TXO, VFD_UART_RXI);

  //initialize the VFD
  Serial2.write(0x1B40);

  //Set initial VFD brightness: commands to change brightness based on CD5220 Command Set
  Serial2.write('\x1B');
  Serial2.write('\x2A');
  Serial2.write('\x02'); //brightness select 1 <= n <= 4 (4 is brightest)

  //clear the VFD
  Serial2.write('\x0C');



}


/**************************************************************************************************************************/
// Connect to WiFi
/**************************************************************************************************************************/
void connectWifi(){

  Serial2.write('\x0C'); //clear display
  Serial2.println("Trying to connect to WIFI");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial2.write('\x0C'); //clear display
  Serial2.println("WiFi connected.");
  Serial2.print("IP address: ");
  Serial2.print(WiFi.localIP());

  //clear the VFD
  Serial2.write('\x0C'); 
}


/**************************************************************************************************************************/
// Upate Display Settings
/**************************************************************************************************************************/
void updateDisplayFrames(){

  //reset flags if necessary
  if(frameCycler == 3){
    scrollWeatherDataFlag = false;
  }

  if(frameCycler == 0){
    //home VFD cursor 
    Serial2.write('\x0B');

    //set overwrite mode
    Serial2.write('\x1B');
    Serial2.write('\x11');

    //print pitcoin price
    Serial2.println("BITCOIN       ");

    //clear cursor line and clear string mode
    Serial2.write('\x18');

    Serial2.print(myCryptoData.bitcoinPrice);
    Serial2.print(" USD        ");

    frameCycler = 1;
  }
  else if(frameCycler == 1){
    //home VFD cursor 
    Serial2.write('\x0B');

    //set overwrite mode
    Serial2.write('\x1B');
    Serial2.write('\x11');

    //print pitcoin price
    Serial2.println("ETHEREUM      ");

    //clear cursor line and clear string mode
    Serial2.write('\x18');

    Serial2.print(myCryptoData.ethereumPrice);
    Serial2.print(" USD        ");
    frameCycler = 2;
  }
  else if(frameCycler == 2){
    //home VFD cursor 
    Serial2.write('\x0B');

    //set overwrite mode
    Serial2.write('\x1B');
    Serial2.write('\x11');

    Serial2.println("Tampa Weather  ");

    //clear cursor line and clear string mode
    Serial2.write('\x18');

    //concatenate to build full string:
    completeWeatherData = myWeatherData.condition + " : " + myWeatherData.conditionDetailed;
    stringWeatherDataLength = completeWeatherData.length();

    //check if we're going to overflow the second line - setup and perform scroll if we do. 
    if(stringWeatherDataLength >= 20){

      Serial.println("We've met conditions to scroll on bottom line");
      
      //we need to begin scroll mode for this function - set global flag and timer so we can go back and scroll in main state machine!
      scrollWeatherDataFlag = true;
      weatherScrollInterval = INTERVAL_THREE / stringWeatherDataLength;
      weatherDataPosition = 0;

    }
    else{
      Serial2.print(completeWeatherData);
    }
    frameCycler = 3;
  }
  else if(frameCycler == 3){
    //home VFD cursor 
    Serial2.write('\x0B');

    //set overwrite mode
    Serial2.write('\x1B');
    Serial2.write('\x11');

    //print pitcoin price
    Serial2.println("Tampa Weather ");

    //clear cursor line and clear string mode
    Serial2.write('\x18');

    Serial2.print(myWeatherData.temperature);
    Serial2.write(0xF8);
    Serial2.print("F  ");
    Serial2.print(myWeatherData.humidity);
    Serial2.print("%    ");
    frameCycler = 0;
  }
  else{
    //do nothing
  }

}


/**************************************************************************************************************************/
// Scroll Weather Data during the appropriate frame
/**************************************************************************************************************************/
void scrollWeatherDataDuringFrame(){
  
  //set cursor to position 1, line 2 (X,Y)
  Serial2.write('\x1B');
  Serial2.write('\x6C');
  Serial2.write('\x01');
  Serial2.write('\x02');

  //set horizontal scroll mode
  Serial2.write('\x1B');
  Serial2.write('\x13');

  Serial2.print(completeWeatherData[weatherDataPosition]);
}


/**************************************************************************************************************************/
// Upate Display Settings
/**************************************************************************************************************************/
void updateDisplaySettings(){

  //update display brightness based on ambient light
  updateDisplayBrightness();

}


/**************************************************************************************************************************/
// Upate Display Brightness based on ambient light
/**************************************************************************************************************************/
void updateDisplayBrightness(){

  int lightReading = analogRead(LIGHT_SENSOR);
  if(lightReading > LIGHT_THRESHOLD_ONE){
    //brightest option - lvl 4 brightness
    Serial2.write('\x1B');
    Serial2.write('\x2A');
    Serial2.write('\x04');
  }
  else if(lightReading > LIGHT_THRESHOLD_TWO){
    //lvl 3 brightness
    Serial2.write('\x1B');
    Serial2.write('\x2A');
    Serial2.write('\x03');
  }
  else if(lightReading > LIGHT_THRESHOLD_THREE){
    //lvl 2 brightness
    Serial2.write('\x1B');
    Serial2.write('\x2A');
    Serial2.write('\x02'); 
  }
  else{
    //dimmest option - lvl 1 brightness
    Serial2.write('\x1B');
    Serial2.write('\x2A');
    Serial2.write('\x01');
  }
}


/**************************************************************************************************************************/
// Upate Fixed Items on the VFD display
/**************************************************************************************************************************/
void updateDisplayFixedItems(){

  updateDisplayTime();

}


/**************************************************************************************************************************/
// Upate Time on the VFD Display
/**************************************************************************************************************************/
void updateDisplayTime(){

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  //move cursor to location where we want time to be displayed
  //set cursor to 16,1 (X,Y)
  Serial2.write('\x1B');
  Serial2.write('\x6C');
  Serial2.write('\x10'); 
  Serial2.write('\x01');

  //set overwrite mode
  Serial2.write('\x1B');
  Serial2.write('\x11');

  //grab time from background time structure and convert from 24 to 12 hr time. 
  char buffer[2];
  int myHour = timeinfo.tm_hour;
  if(myHour > 12){
    myHour = myHour - 12;
  }

  //if the hour is a single digit, we need to shift curson one to right to keep time justified in top right corner
  if(myHour < 10){
    Serial2.print(" ");
  }

  //take hour, convert it to a string, and remove leading zero if present
  std::string myHourString = itoa(myHour,buffer,10);
  myHourString.erase(0, min(myHourString.find_first_not_of('0'), myHourString.size()-1));

  //blink colon and print time 
  if(colonOnLastLoop){
    colonOnLastLoop = false;
    Serial2.print(myHourString.c_str());
    Serial2.print(" ");
    Serial2.print(&timeinfo, "%M");
  }
  else{
    colonOnLastLoop = true;
    Serial2.print(myHourString.c_str());
    Serial2.print(":");
    Serial2.print(&timeinfo, "%M");
  }
}


/**************************************************************************************************************************/
// get the ethereum price from the internet and parse the JSON 
/**************************************************************************************************************************/
void getAndParseCryptoPrice(){

  String apiKey = CRYPTO_API_KEY;

  //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

    String serverPath = CRYPTO_SERVER_PATH ;

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Specify content-type header
    http.addHeader("X-CMC_PRO_API_KEY", apiKey);
    // Specify content-type header
    http.addHeader("Accepts", "application/json");
      
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    //print results
    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        //Serial.println(payload); //for debug
        //Serial.print("Parsing JSON: ");
        parseCryptoJson(payload);

        //for debug
        Serial.print("ETH Value: ");
        Serial.println(myCryptoData.ethereumPrice);
        Serial.print("BTC Value: ");
        Serial.println(myCryptoData.bitcoinPrice);

      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}


/**************************************************************************************************************************/
// parse the JSON response from the ETHEREUM API
/**************************************************************************************************************************/
void parseCryptoJson( String json ) {
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, json);
  if (error){
    Serial.println(error.c_str());
  }
  myCryptoData.ethereumPrice = doc["data"]["ETH"]["quote"]["USD"]["price"];
  myCryptoData.bitcoinPrice = doc["data"]["BTC"]["quote"]["USD"]["price"];
}


/**************************************************************************************************************************/
// get the weather from the internet and parse the JSON 
/**************************************************************************************************************************/
void getAndParseWeather(){

  //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

    String serverPath = WEATHER_SERVER_PATH ;

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
      
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    //print results
    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        //Serial.println(payload); //for debug
        //Serial.print("Parsing JSON: ");
        parseWeatherJson(payload);

        //for debug
        Serial.print("Temperature: ");
        Serial.println(myWeatherData.temperature);
        Serial.print("Humidity: ");
        Serial.println(myWeatherData.humidity);
        Serial.print("Conditions: ");
        Serial.println(myWeatherData.condition);
        Serial.print("Conditions detailed: ");
        Serial.println(myWeatherData.conditionDetailed);

      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}


/**************************************************************************************************************************/
// parse the JSON response from the WEATHER API
/**************************************************************************************************************************/
void parseWeatherJson( String json ) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  if (error){
    Serial.println(error.c_str());
  }

  float localTemp = doc["main"]["temp"];
  myWeatherData.temperature = (1.8 * (localTemp - 273.15)) + 32;

  myWeatherData.humidity = doc["main"]["humidity"];
  myWeatherData.condition = (const char*)doc["weather"][0]["main"];
  myWeatherData.conditionDetailed = (const char*)doc["weather"][0]["description"];

}

