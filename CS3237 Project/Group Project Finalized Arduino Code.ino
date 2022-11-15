// DHT Temperature & Humidity Sensor
// Unified Sensor Library Example
// Written by Tony DiCola for Adafruit Industries
// Released under an MIT license.

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Panku"; // To fill out
const char* password = "abcd1234"; // To fill out
String ip = "http://192.168.25.46:5000/"; // To fill out

bool ledstate = LOW;
ESP8266WebServer server(80);

void handleNotFound(){
  String message = "File Not Found\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "n";
  for (uint8_t i = 0; i < server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "n";
  }
  server.send(404, "text/plain", message);
}

#define DHTPIN D6     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT11     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);
// constants won't change. They're used here to set pin numbers:
uint32_t delayMS;
bool newQuestion=true;
volatile unsigned long prev =0;
volatile unsigned long prevLoop =0;
volatile int buttonPressed=0;
int elapsedTime=0;
bool answerWasTruth=true;

void PERSONSTATUS() {
  ledstate != ledstate;
  digitalWrite(D4, ledstate);
  
  if (ledstate == LOW) {
    server.send(200, "text/plain", "Person's State: Lying");
  }
  else {
    server.send(200, "text/plain", "Person's State: NOT Lying ");
  }
  //String LEDSTATUS = ledstate == LOW ? "The Person Is NOT Lying!" : "The Person Is Lying!";
  server.send(200, "text/plain", "Person's State: " + ledstate);
}
//this function sends data and this will be the "normal" values because these are all truth
void sendTruthData(float temp, float hum, float pulse){
  if (WiFi.status() == WL_CONNECTED) {             //Check WiFi connection status
    Serial.println("Sending FIRST datas to server");
    DynamicJsonDocument doc(2048);
    doc["temperature"] = temp;
    doc["humidity"] = hum;
    doc["pulse"] = pulse;
    // Serialize JSON document
    String json;
    serializeJson(doc, json);

    WiFiClient client;
    HTTPClient http;                               //Declare object of class HTTPClient
 
    http.begin(client, ip + "collectTruthData");      //Specify request destination
    http.addHeader("Content-Type", "application/json");           //Specify content-type header
 
    int httpCode = http.POST(json);                               //Send the request
    String payload = http.getString();                            //Get the response payload
 
    Serial.println(httpCode);                                     //Print HTTP return code
    Serial.println(payload);                                      //Print request response payload
 
    http.end();  //Close connection
  }
}
//this function sends data while the answer lasts
void sendAnswerData(float temp, float hum, float pulse){
  if (WiFi.status() == WL_CONNECTED) {             //Check WiFi connection status
    Serial.println("Sending ANSWER data to server");
    DynamicJsonDocument doc(2048);
    doc["temperature"] = temp;
    doc["humidity"] = hum;
    doc["pulse"] = pulse;
    // Serialize JSON document
    String json;
    serializeJson(doc, json);
    WiFiClient client;
    HTTPClient http;                               //Declare object of class HTTPClient
    http.begin(client, ip + "collectAnswerData");      //Specify request destination
    http.addHeader("Content-Type", "application/json");           //Specify content-type header
 
    int httpCode = http.POST(json);                               //Send the request
    String payload = http.getString();                            //Get the response payload
 
    Serial.println(httpCode);                                     //Print HTTP return code
    Serial.println(payload);                                      //Print request response payload
    http.end();  //Close connection
  }
}
//this function calls the prediction on the server
bool answerEnded(float temp, float hum, float pulse){
  if (WiFi.status() == WL_CONNECTED) {             //Check WiFi connection status
    Serial.println("Getting deception data from server");

    WiFiClient client; 
    HTTPClient http;                               //Declare object of class HTTPClient
    http.begin(client, ip + "getTruth");      //Specify request destination
    http.addHeader("Content-Type", "application/json");  
    int httpCode = http.GET();                               //Send the request
    String payload = http.getString();       //Get the response payload
    Serial.println(httpCode); 
    Serial.println(payload);      
    http.end();  //Close connection
    Serial.println("THIS IS THE PAYLOAD-----------------------------" +payload);
    if(payload=="true"){
      return true;
      }         
      else{
        return false;
      }                          
  }
  return false;
 }

void setup() {
  // Initialize device.
  WiFi.begin(ssid, password);

    
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  
  //pinMode(D4, OUTPUT); //Red LED to detect student is lying or not 
  //pinMode(D5, OUTPUT); //Green LED to detect student is telling the truth 
  pinMode(A0, INPUT); //Heartbeat/Pulse Sensor to detect the physiological response of the student to find out if student is lying or not 
  //pinMode(D0, INPUT); //Touch Sensor to detect if student presses the sensor or not to check if the student is lying or not 
  pinMode(D7, INPUT_PULLUP); //Button to control the lying experiment intervals  
  prev = millis();
  prevLoop = millis();

  attachInterrupt(digitalPinToInterrupt(D7),interruptButton, RISING); //Converts D7 where button is attached into an interrupt number 

  Serial.begin(9600);

  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------")); 
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 2000;
  
  server.on("/personstatus", PERSONSTATUS);
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

IRAM_ATTR void interruptButton(){
  if((millis()- prev)>1000 && elapsedTime>=3 && digitalRead(D7)==HIGH){
    buttonPressed+=1;
    prev= millis();
    elapsedTime=0;
    }
}
  
void loop() {

  if((millis()- prevLoop)>250){
    elapsedTime+=1;
  server.handleClient();

  //Print current status of Touch Sensor 
  //int state = digitalRead(D0); to be deleted
  //Serial.println(state);

  
  //Print out current pulse of person
  float pulse;
  int sum = 0;
  for (int i = 0; i < 20; i++)
    sum += analogRead(A0);
  pulse = sum / 20.00;
  //Serial.println(pulse);
  

  
  // Get temperature event and print its value. 
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temperature = event.temperature;
    
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;

  //send data to server
  //VARY  WITHT HE CALL OF THESE 3 DEPENDING ON BUTTON PUSH, 1st button push enables sendtruthData, 2nd stops it (1st,2nd button push)
     //after that every second button push call enables this (3th,5th,7th etc)
     
  if(pulse<1500){
    if(buttonPressed==1){
      sendTruthData(temperature,humidity,pulse);
      Serial.println("Sending all truth data now");
      }

    // 3rd,5,7,9,11,13,15,17 etc
    else if(buttonPressed%2==1){ 
      digitalWrite(D4,LOW);
      digitalWrite(D5,LOW);
      newQuestion=true;
      sendAnswerData(temperature,humidity,pulse);
      Serial.println("Sending all answers now");
      }
    //this returns the answer after every other second button push (4th,6th,8th, etc)
    
    // 4,6,8,10,12,14,16,18 etc button
    else if(buttonPressed!=0 && buttonPressed!=2 && newQuestion==true) {
      newQuestion=false;
      answerWasTruth=answerEnded(temperature,humidity,pulse);
      if(answerWasTruth){
        Serial.println("Answer is truth");
        digitalWrite(D4,LOW);
        digitalWrite(D5,HIGH);
        }
      else{
        Serial.println("Answer was lie.");
        digitalWrite(D5,LOW);
        digitalWrite(D4,HIGH);
        }
      Serial.println("Interview ended");
   }
  } 
  Serial.print("\n");
  Serial.print(F("Humidity (%): "));
  Serial.println(humidity);
  
  Serial.print(F("Temperature (°C): ")); 
  Serial.println(temperature);
  
  Serial.print(F("Pulse Rate (bpm): "));
  Serial.println(pulse);

  Serial.print(F("Person State (Seems): ")); 
  Serial.println(F("Lying!"));
 
  Serial.print("--------------------------");   
    

  prevLoop=millis();
  }
}
