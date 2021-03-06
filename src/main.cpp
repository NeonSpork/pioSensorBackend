#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <DallasTemperature.h>
#include <HX711.h>

// WiFi stuff
String header;
WiFiServer server(80);

// Valves
const int beerPin = 5; // D1
const int vodkaPin = 4; // D2
bool beerState = false;
bool vodkaState = false;

// Temp sensor
OneWire oneWire(2); // D4 aka GPIO2
DallasTemperature tempSensor(&oneWire);
float temp = 99;
int pints = 99;

// Load sensor
const int loadDout = 14; // D5
const int loadSdk = 12; // D6
const int offset = 8234508;
const int scale = -20.9993;
HX711 hx;
void updatePints() {
  float grams = hx.get_units(1);
  pints = int((grams-4250)*0.002);
}

void setup() {
  Serial.begin(115200);
  // Initialize pins
  pinMode(beerPin, OUTPUT);
  digitalWrite(beerPin, LOW);
  pinMode(vodkaPin, OUTPUT);
  digitalWrite(vodkaPin, LOW);
  // Initialize temp sensor
  tempSensor.begin();
  delay(1000); // Give the OneWire time to fire up
  tempSensor.requestTemperatures();
  temp = tempSensor.getTempCByIndex(0);
  // Initialize loadsensor
  hx.begin(loadDout, loadSdk);
  hx.set_scale(scale);
  hx.set_offset(offset);
  // Initialize WiFi stuff
  WiFiManager wifiManager;
  // Uncomment and run it once, if you want to erase all the stored information
  // wifiManager.resetSettings();
  wifiManager.autoConnect("getBeer_WiFi_configuration");
  server.begin();
}

void loop() {
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
            client.println("Access-Control-Allow-Methods: GET, PUT");
            client.println();

            // CONTROL VALVES
            if (header.indexOf("GET /api/openBeer HTTP/1.1") >= 0) {
              beerState = true;
              client.println(beerState);
            }
            else if (header.indexOf("GET /api/closeBeer HTTP/1.1") >= 0) {
              beerState = false;
              client.println(beerState);
            }
            else if (header.indexOf("GET /api/openVodka HTTP/1.1") >= 0) {
              vodkaState = true;
              client.println(vodkaState);
            }
            else if (header.indexOf("GET /api/closeVodka HTTP/1.1") >= 0) {
              vodkaState = false;
              client.println(vodkaState);
            }
            else if (header.indexOf("GET /api/temp HTTP/1.1") >= 0) {
              tempSensor.requestTemperatures();
              temp = tempSensor.getTempCByIndex(0);
              client.println(temp);
            }
            else if (header.indexOf("GET /api/pints HTTP/1.1") >= 0) {
              updatePints();
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
  digitalWrite(beerPin, beerState);
  digitalWrite(vodkaPin, vodkaState);
}

