#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <SI7021.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#define SDA 0 // GPIO0 on ESP-01 module
#define SCL 2 // GPIO2 on ESP-01 module
#define FIVEMIN (1000UL * 60 * 5) // Update Frequency
#define AP_SSID   "xxxxx" // Access Point SSID
#define AP_PSK    "xxxxx" // Access Point PSK
#define TS_APIKEY "xxxxx" // ThingSpeak API Key

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
SI7021 sensor;
HTTPClient http;

unsigned long rolltime = millis() + FIVEMIN;

// Root Handle
void handle_root() {
  int temperature = sensor.getCelsiusHundredths();
  int humidity = sensor.getHumidityPercent();
  float temp = temperature / 100.0;
  server.send(200, "text/plain", String() +   F("{\"temperature\": ") + String(temp,1) +  F(", \"humidity\": ") + String(humidity) + F("}"));
}

// 404 Handler
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  Serial.begin (9600);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(AP_SSID, AP_PSK);

  Serial.print("Connecting Wifi..");

  // Try to connect to WIFI
  while(wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  // Show AP connection details
  if(wifiMulti.run() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  }

  // Start MDNS responder
  if(MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
  }

  // Set Root Handler
  server.on("/", handle_root);

  // Inline Handler
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  // 404 Handler
  server.onNotFound(handleNotFound);

  // Start Server
  server.begin();

  // Start SI7021
  sensor.begin(SDA,SCL);
}


void loop() {

  // Handle HTTP Server
  if (wifiMulti.run() == WL_CONNECTED) {
    server.handleClient();
  }

  // Check to see if we need to send an ThingSpeak update
  if((long)(millis() - rolltime) >= 0) {

    int temperature = sensor.getCelsiusHundredths();
    int humidity = sensor.getHumidityPercent();
    float temp = temperature / 100.0;

    Serial.println("Sending Update To ThingSpeak ");


    if (wifiMulti.run() == WL_CONNECTED) {
      http.begin("https://api.thingspeak.com/update.json");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpCode = http.POST("api_key=" + String(TS_APIKEY) + "&field1=" + String(temp,1) + "&field2=" + humidity);

      if(httpCode > 0) {
        // We got a header
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // We got a 200!
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    }
    // Roll on timer
    rolltime += FIVEMIN;
  }
}
