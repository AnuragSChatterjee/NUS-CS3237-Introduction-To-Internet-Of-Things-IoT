#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "Anurag's Galaxy S21 FE 5G"; // To fill out
const char* password = "belr4158"; // To fill out
bool ledState =   LOW;

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "WeMOS on the Web!");
}

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

void toggleLED() {
  ledState != ledState;
  digitalWrite(D4, ledState);
  String LED_STATUS = ledState == LOW ? "LED Is On!" : "LED Is Off!";
  server.send(200, "text/plain", "LED Status: " + LED_STATUS);
}

void setup(void){
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  
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

  server.on("/", handleRoot);

  server.on("/inline", [](){
  server.send(200, "text/plain", "You can define a handler like this as well!");
  });
 
  server.on("/toggleLED", toggleLED);
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
