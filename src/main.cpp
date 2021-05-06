#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <arduino-timer.h>

#include <DallasTemperature.h>


// Timer stuff
auto timer = timer_create_default();

// WiFi stuff
String header;
WiFiServer server(80);

// Valves
const int beerPin = 5; // D1
const int vodkaPin = 4; // D2
String beerState = "CLOSED";
String vodkaState = "CLOSED";

// Temp sensor
OneWire oneWire(2); // D4 aka GPIO2
DallasTemperature tempSensor(&oneWire);
DeviceAddress probeID;

void startBeer() {
  digitalWrite(beerPin, HIGH);
  beerState = "OPEN";
}

bool stopBeer(void *) {
  digitalWrite(beerPin, LOW);
  beerState = "CLOSED";
  return true;
}

void startVodka() {
  digitalWrite(vodkaPin, HIGH);
  vodkaState = "OPEN";
}

bool stopVodka(void *) {
  digitalWrite(vodkaPin, LOW);
  vodkaState = "CLOSED";
  return true;
}

void setup() {
  Serial.begin(115200);
  WiFi.hostname("sensorbackend");
  // Initialize pins
  pinMode(beerPin, OUTPUT);
  digitalWrite(beerPin, LOW);
  pinMode(vodkaPin, OUTPUT);
  digitalWrite(vodkaPin, LOW);
  // Initialize temp sensor
  tempSensor.begin();
  oneWire.reset_search();
  oneWire.search(probeID);
  WiFiManager wifiManager;
  // Uncomment and run it once, if you want to erase all the stored information
  // wifiManager.resetSettings();
  wifiManager.autoConnect("getBeer_WiFi_configuration");
  // wifi_station_set_hostname("sensorbackend");
  server.begin();
}

void loop() {
  timer.tick();
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client!");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println("Access-Control-Allow-Origin: *");
            client.println();

            // CONTROL VALVES
            if (header.indexOf("GET /api/dispenseBeer HTTP/1.1") >= 0) {
              startBeer();
              timer.in(5000, stopBeer);
              // Ends pour after 5s, adjust time for specific volume
              client.println("BEER SUCCESSFUL.");
            }
            else if (header.indexOf("GET /api/dispenseVodka HTTP/1.1") >= 0) {
              startVodka();
              timer.in(2000, stopVodka);
              // Ends pour after 2s, adjust time for specific volume
              client.println("VODKA SUCCESSFUL.");
            }
            else if (header.indexOf("GET /api/temp HTTP/1.1") >= 0) {
              float temp = tempSensor.getTempC(probeID);
              client.println(temp);
            }
            else if (header.indexOf("GET /api/pints HTTP/1.1") >= 0) {
              int pints = 9999;
              client.println(pints);
            }
            else if (header.indexOf("GET /api/killswitch HTTP/1.1") >= 0) {
              digitalWrite(beerPin, LOW);
              digitalWrite(vodkaPin, LOW);
            }
            break;
          }
          else {
            // If you get a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // Clear header
    header = "";
    // Close connection
    client.stop();
  }
}

