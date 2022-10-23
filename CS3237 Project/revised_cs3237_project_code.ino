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
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "Anurag's Galaxy S21 FE 5G"; // To fill out
const char* password = "belr4158"; // To fill out
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
  
  pinMode(D4, OUTPUT); //Red LED to detect student is lying or not 
  pinMode(D5, OUTPUT); //Green LED to detect student is telling the truth 
  pinMode(A0, INPUT); //Heartbeat/Pulse Sensor to detect the physiological response of the student to find out if student is lying or not 
  pinMode(D0, INPUT); //Touch Sensor to detect if student presses the sensor or not to check if the student is lying or not 
  
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

const int i = 4000;

void loop() {
  server.handleClient();

  //Print current status of Touch Sensor 
  int state = digitalRead(D0);
  Serial.println(state);
  delay(250);

  
  //Print out current pulse of person
  float pulse;
  int sum = 0;
  for (int i = 0; i < 20; i++)
    sum += analogRead(A0);
  pulse = sum / 20.00;
  Serial.println(pulse);
  delay(100);
  
  // Delay between measurements.
  delay(delayMS);
  
  // Get temperature event and print its value. 
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temperature = event.temperature;
   
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    //Serial.print(F("Temperature: "));
    Serial.print(event.temperature); 
    //Serial.println(F("°C"));
  }
  
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;
  
  if ((humidity > 85) || (state == HIGH) || (pulse > 750)) {
    Serial.println(F("====================="));
    Serial.print(F("Humidity (%): "));
    Serial.println(humidity);
    Serial.println(F("---------------------"));
    Serial.print(F("Temperature (°C): ")); 
    Serial.println(temperature);
    Serial.println(F("---------------------"));
    Serial.println(state);
    Serial.println("Touch Sensor Is Activated"); 
    Serial.println(F("---------------------"));
    Serial.print(F("Pulse Rate (bpm): "));
    Serial.println(pulse);
    Serial.println("High Pulse Rate!");
    Serial.println(F("---------------------"));
    Serial.println(F("The Person Seems To Be Lying!"));
    digitalWrite(D4, HIGH);
    digitalWrite(D5, LOW);
  }
  else {
    Serial.println(F("====================="));
    Serial.print(F("Humidity (%): "));
    Serial.println(humidity);
    Serial.println(F("---------------------"));
    Serial.print(F("Temperature (°C): "));
    Serial.println(temperature);
    Serial.println(F("---------------------"));
    Serial.println(state);
    Serial.println("Touch Sensor Is Activated");
    Serial.println(F("---------------------"));
    Serial.print(F("Pulse Rate (bpm): "));
    Serial.println(pulse);
    Serial.println("Erratic Heartbeats are not present");
    Serial.println(F("---------------------"));
    Serial.println(F("The Person Does Not Seems To Be Lying - Seems Normal Out There"));
    digitalWrite(D4, LOW);
    digitalWrite(D5, HIGH);
  }
}
