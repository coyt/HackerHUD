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
  setupWiFiConfigManager();                                 //setup the wifi config manager
  //connectWifi();                                            //make ESP32 connect to wifi
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

  doWifiManagerNonsense();


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


  /*
  //hardware test for button one
  if(buttonOnePressed){
    buttonOnePressed = false;

    //clear the VFD
    Serial2.write('\x0C');

    Serial2.println("BUTTON ONE PRESSED");

    delay(2000);
  }
  */


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
// do wifi manager nonsense from main loop
/**************************************************************************************************************************/
void doWifiManagerNonsense(){

  // is configuration portal requested?
  //if ((digitalRead(TRIGGER_PIN) == LOW) || (digitalRead(TRIGGER_PIN2) == LOW))
  if (buttonOnePressed)
  {
    buttonOnePressed = false; //reset button one

    Serial.println("\nConfiguration portal requested.");
    //digitalWrite(LED_BUILTIN, LED_ON); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

    //Local intialization. Once its business is done, there is no need to keep it around
    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "ConfigOnSwitch");

    ESPAsync_wifiManager.setMinimumSignalQuality(-1);

    // From v1.0.10 only
    // Set config portal channel, default = 1. Use 0 => random channel from 1-13
    ESPAsync_wifiManager.setConfigPortalChannel(0);
    //////

    //set custom ip for portal
    //ESPAsync_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));

#if !USE_DHCP_IP    
  #if USE_CONFIGURABLE_DNS  
    // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
    ESPAsync_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask, dns1IP, dns2IP);  
  #else
    // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
    ESPAsync_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask);
  #endif 
#endif       

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

    //Check if there is stored WiFi router/password credentials.
    //If not found, device will remain in configuration mode until switched off via webserver.
    Serial.print("Opening configuration portal. ");
    Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
    Router_Pass = ESPAsync_wifiManager.WiFi_Pass();
    
    // From v1.1.0, Don't permit NULL password
    if ( (Router_SSID != "") && (Router_Pass != "") )
    {
      ESPAsync_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
      Serial.println("Got stored Credentials. Timeout 120s");
    }
    else
      Serial.println("No stored Credentials. No timeout");

    //Starts an access point
    //and goes into a blocking loop awaiting configuration
    if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password))
    {
      Serial.println("Not connected to WiFi but continuing anyway.");
    }
    else
    {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
      Serial.print("Local IP: ");
      Serial.println(WiFi.localIP());
    }

    // Only clear then save data if CP entered and with new valid Credentials
    // No CP => stored getSSID() = ""
    if ( String(ESPAsync_wifiManager.getSSID(0)) != "" && String(ESPAsync_wifiManager.getSSID(1)) != "" )
    {
      // Stored  for later usage, from v1.1.0, but clear first
      memset(&WM_config, 0, sizeof(WM_config));
      
      for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
      {
        String tempSSID = ESPAsync_wifiManager.getSSID(i);
        String tempPW   = ESPAsync_wifiManager.getPW(i);
    
        if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
          strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
        else
          strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);
    
        if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
          strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
        else
          strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  
    
        // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
        if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
        {
          LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
          wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
        }
      }
    
      saveConfigData();
    }

    //digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.
  }

  check_status();

}


/**************************************************************************************************************************/
// Setup the WiFi Config Manager
/**************************************************************************************************************************/
void setupWiFiConfigManager(){

  //format FileFS if not yet
  if (FORMAT_FILESYSTEM) {
    FileFS.format();
  }

  // test FileFS
  if (!FileFS.begin(true)){
    Serial.print(FS_Name);
    Serial.println(F(" failed to mount filesystem!"));
  }

  unsigned long startedAt = millis();

  //Local intialization. Once its business is done, there is no need to keep it around
  // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  //ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer);
  // Use this to personalize DHCP hostname (RFC952 conformed)
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "ConfigOnSwitch");

  // Use only to erase stored WiFi Credentials
  //resetSettings();
  //ESPAsync_wifiManager.resetSettings();

  //set custom ip for portal
  //ESPAsync_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));

  ESPAsync_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESPAsync_wifiManager.setConfigPortalChannel(0);
  //////

  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  ESPAsync_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask);

  // New from v1.1.1
  #if USING_CORS_FEATURE
    ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
  #endif

  // We can't use WiFi.SSID() in ESP32as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS for this purpose
  Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
  Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

  //Remove this line if you do not want to see WiFi password printed
  Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  // SSID to uppercase
  ssid.toUpperCase();

  // From v1.1.0, Don't permit NULL password
  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
    
    ESPAsync_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
    Serial.println("Got stored Credentials. Timeout 120s for Config Portal");
  }
  else
  {
    Serial.println("Open Config Portal without Timeout: No stored Credentials.");
    
    initialConfig = true;
  }


  if (initialConfig)
  {
    Serial.println("Starting configuration portal.");
    //digitalWrite(LED_BUILTIN, LED_ON); // Turn led on as we are in configuration mode.

    //sets timeout in seconds until configuration portal gets turned off.
    //If not specified device will remain in configuration mode until
    //switched off via webserver or device is restarted.
    //ESPAsync_wifiManager.setConfigPortalTimeout(600);

    // Starts an access point
    if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password))
      Serial.println("Not connected to WiFi but continuing anyway.");
    else
    {
      Serial.println("WiFi connected...yeey :)");
    }

    // Stored  for later usage, from v1.1.0, but clear first
    memset(&WM_config, 0, sizeof(WM_config));
    
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      String tempSSID = ESPAsync_wifiManager.getSSID(i);
      String tempPW   = ESPAsync_wifiManager.getPW(i);
  
      if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

      if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  

      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    saveConfigData();
  }

  startedAt = millis();

  if (!initialConfig)
  {
    // Load stored data, the addAP ready for MultiWiFi reconnection
    loadConfigData();

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    if ( WiFi.status() != WL_CONNECTED ) 
    {
      Serial.println("ConnectMultiWiFi in setup");
     
      connectMultiWiFi();
    }
  }

  startedAt = millis();

  Serial.print("After waiting ");
  Serial.print((float) (millis() - startedAt) / 1000L);
  Serial.print(" secs more in setup(), connection result is ");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("connected. Local IP: ");
    Serial.println(WiFi.localIP());
  }
  else
    Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));


}


/**************************************************************************************************************************/
// saveConfigData function
/**************************************************************************************************************************/
void saveConfigData(void)
{
  File file = FileFS.open(CONFIG_FILENAME, "w");
  LOGERROR(F("SaveWiFiCfgFile "));

  if (file)
  {
    file.write((uint8_t*) &WM_config, sizeof(WM_config));
    file.close();
    LOGERROR(F("OK"));
  }
  else
  {
    LOGERROR(F("failed"));
  }
}


/**************************************************************************************************************************/
// loadConfigData function
/**************************************************************************************************************************/
void loadConfigData(void)
{
  File file = FileFS.open(CONFIG_FILENAME, "r");
  LOGERROR(F("LoadWiFiCfgFile "));

  if (file)
  {
    file.readBytes((char *) &WM_config, sizeof(WM_config));
    file.close();
    LOGERROR(F("OK"));
  }
  else
  {
    LOGERROR(F("failed"));
  }
}


/**************************************************************************************************************************/
// checkStatus function
/**************************************************************************************************************************/
void check_status(void)
{
  static ulong checkstatus_timeout  = 0;
  static ulong LEDstatus_timeout    = 0;
  static ulong checkwifi_timeout    = 0;

  static ulong current_millis;

#define WIFICHECK_INTERVAL    1000L
#define LED_INTERVAL          2000L
#define HEARTBEAT_INTERVAL    10000L

  current_millis = millis();
  
  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  if ((current_millis > LEDstatus_timeout) || (LEDstatus_timeout == 0))
  {
    // Toggle LED at LED_INTERVAL = 2s
    //toggleLED();
    LEDstatus_timeout = current_millis + LED_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }
}


/**************************************************************************************************************************/
// checkWifi function
/**************************************************************************************************************************/
void check_WiFi(void)
{
  if ( (WiFi.status() != WL_CONNECTED) )
  {
    Serial.println("\nWiFi lost. Call connectMultiWiFi in loop");
    connectMultiWiFi();
  }
}  


/**************************************************************************************************************************/
// heartbeat function
/**************************************************************************************************************************/
void heartBeatPrint(void)
{
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print("H");        // H means connected to WiFi
  else
    Serial.print("F");        // F means not connected to WiFi

  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(" ");
  }
}


/**************************************************************************************************************************/
// connectMultiWifi function
/**************************************************************************************************************************/
uint8_t connectMultiWiFi(void)
{
#if ESP32
  // For ESP32, this better be 0 to shorten the connect time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS       0
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS       2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS           100L
  
  uint8_t status;

  LOGERROR(F("ConnectMultiWiFi with :"));
  
  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
    {
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
    }
  }
  
  LOGERROR(F("Connecting MultiWifi..."));

  WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP    
  #if USE_CONFIGURABLE_DNS  
    // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
    WiFi.config(stationIP, gatewayIP, netMask, dns1IP, dns2IP);  
  #else
    // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
    WiFi.config(stationIP, gatewayIP, netMask);
  #endif 
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ( ( i++ < 20 ) && ( status != WL_CONNECTED ) )
  {
    status = wifiMulti.run();

    if ( status == WL_CONNECTED )
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if ( status == WL_CONNECTED )
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
  }
  else
    LOGERROR(F("WiFi not connected"));

  return status;
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
/*
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
*/


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

