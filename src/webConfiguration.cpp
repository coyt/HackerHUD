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

extern AsyncWebServer webServer; //the webserver is actually defined in the WiFiManager - DONT TRY TO MAKE A SECOND WEBSERVER HERE

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

/**************************************************************************************************************************/
// Website
/**************************************************************************************************************************/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>

  <!--
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%

  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>

  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  -->

  <h3>HackerHUD WEB Configuration Interface</h3>

  <HR NOSHADE SIZE=10>

  <h4>Set your coinmarketcap.com API key here</h4>
  <input type="text" placeholder="paste your coinmarketcap API key here" id="cryptoApiId">
  <button type="button" onclick="saveCryptoApiId();">Submit</button>
  <h4>Click "Submit" and the api key will be saved.</h4>

  <HR NOSHADE SIZE=10>

  <h4>Set your openweathermap.org API key here</h4>
  <input type="text" placeholder="paste openweathermap.org API key here" id="weatherApiId">
  <button type="button" onclick="saveWeatherApiId();">Submit</button>
  <h4>Click "Submit" and the api key will be saved.</h4>

  <HR NOSHADE SIZE=10>


<script>

  /*

  function toggleCheckbox(element) 
      {
        var xhr = new XMLHttpRequest();
        if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
        else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
        xhr.send();
      }


    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("temperature").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/temperature", true);
      xhttp.send();
    }, 10000 ) ;


    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("humidity").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/humidity", true);
      xhttp.send();
    }, 10000 ) ;

  */

  //ACTUAL FUNCTIONS HERE NOW

    function saveCryptoApiId() {
        // Selecting the input element and get its value 
        let inputVal = document.getElementById("cryptoApiId").value;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/cryptoapi?cryptoApiKey="+inputVal, true);
        xhr.send();
    }

    function saveWeatherApiId() {
        // Selecting the input element and get its value 
        let inputVal = document.getElementById("weatherApiId").value;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/weatherapi?weatherApiKey="+inputVal, true);
        xhr.send();
    }

    
</script>
</body>
</html>
)rawliteral";


/**************************************************************************************************************************/
// Replaces placeholder with button section in your web page
/**************************************************************************************************************************/
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  else if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  return String();

}


/**************************************************************************************************************************/
// outputState helper function
/**************************************************************************************************************************/
String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}


/**************************************************************************************************************************/
// setup web configuration interface
/**************************************************************************************************************************/
void setupWebConfigurationInterface(){


    Serial.println("***** Starting Web Configuration Interface *****");


    // Route for root / web page
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });

    /*

    // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    webServer.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String inputMessage1;
        String inputMessage2;
        // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
        if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
        inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
        }
        else {
        inputMessage1 = "No message sent";
        inputMessage2 = "No message sent";
        }
        Serial.print("GPIO: ");
        Serial.print(inputMessage1);
        Serial.print(" - Set to: ");
        Serial.println(inputMessage2);
        request->send(200, "text/plain", "OK");
    });


    // Send a GET request to <ESP_IP>/temperature
    webServer.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
    });


    // Send a GET request to <ESP_IP>/humidity
    webServer.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", readDHTHumidity().c_str());
    });

    */


    // Send a GET request to <ESP_IP>/
    webServer.on("/cryptoapi", HTTP_GET, [](AsyncWebServerRequest *request){
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
    });


    // Send a GET request to <ESP_IP>/
    webServer.on("/weatherapi", HTTP_GET, [](AsyncWebServerRequest *request){
      String weatherApiKey;
      if (request->hasParam("weatherApiKey")) {
        weatherApiKey = request->getParam("weatherApiKey")->value();
      }
      else {
        //weatherApiKey = OLD_WEATHER_KEY; //failed to update weather api key
        Serial.println("Failed to receive weatherapi key from browser");
      }
      Serial.print("NEW WEATHER API KEY: ");
      Serial.println(weatherApiKey);
      request->send(200, "text/plain", "OK");
    });

    // Start server
    webServer.begin();

}


/**************************************************************************************************************************/
// Kill the web configuration service to allow the wifi manager to take prescedence
/**************************************************************************************************************************/
void  killWebConfiguration(){
  webServer.reset();
}


/**************************************************************************************************************************/
// for testing
/**************************************************************************************************************************/
String readDHTHumidity() {
  return String(80);
}


/**************************************************************************************************************************/
// for testing
/**************************************************************************************************************************/
String readDHTTemperature() {
    return String(72);
}


















