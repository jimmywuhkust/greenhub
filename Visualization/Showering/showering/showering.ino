/****************
 * HEADER FILES *
 ****************/

#include <Wire.h>
#include <DNSServer.h>
#include <WiFiClient.h>

//#if defined(ESP8266)
//// you are using an ESP8266 use these libraries
//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
//ESP8266WebServer webServer(80);
//#include <ESP8266mDNS.h>
//
//#else

// otherwise use the ESP32 libraries
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

WebServer webServer(80);
// #endif

// in general we need these libraries
#include <WebSocketsServer.h>
#include <Hash.h>

// include the HTML and p5.js files we shall host
#include "index_html.h"
#include "sketch_js.h"

/*************
 * VARIABLES *
 *************/

// pin where the photoresistor is plugged in
const int ldrPin = 36;
const int ledPin = 13;

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;
const long interval = 50;   

// Logins for WiFi
const char* ssid = "GreenHub_2.4"; // name of WiFi network
const char* password = "greenhub"; // the password

// create a WebSocket on port 81
WebSocketsServer webSocket = WebSocketsServer(81);

/*************
 * FUNCTIONS *
 *************/

// Handles all the webSocket activities
void webSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", client_num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(client_num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", client_num, ip[0], ip[1], ip[2], ip[3], payload);
        
                // send message to client
                webSocket.sendTXT(client_num, "Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] %s\n", client_num, payload);

            // send message to client
            // webSocket.sendTXT(client_num, "Received");

            // using strncmp to compare payload (an byte array) to a string
            if (!strncmp((char *)payload, "on", length)) {
              digitalWrite(ledPin, HIGH);
            } else {
              digitalWrite(ledPin, LOW);
            }
            
            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", client_num, length);
            // hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(client_num, payload, length);
            break;
        case WStype_ERROR:{}
        case WStype_FRAGMENT_TEXT_START:{}
        case WStype_FRAGMENT_BIN_START:{}
        case WStype_FRAGMENT:{}
        case WStype_FRAGMENT_FIN:{}
        case WStype_PING:{}
        case WStype_PONG:{}
          break;
    }

}

// For the webServer_setup()
void handleRoot() {
  webServer.send_P(200, "text/html", INDEX_HTML); // serves the index.html file
}

void handleSketch() {
  webServer.send_P(200, "text/javascript", SKETCH_JS); // serve the sketch_js.h file - send_P send the second and third values as PGM_P types (see WebServer.h at line 123: https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/src/WebServer.h)
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i=0; i<webServer.args(); i++){
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);
}

// Arduino setup function automatically called at startup
void webServer_setup() {
  webServer.on("/", handleRoot);
  webServer.on("/sketch.js", handleSketch);

  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("HTTP server started");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Connect to wifi
  WiFi.begin(ssid, password);
  
  Serial.println("");
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  webServer_setup(); 
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  webSocket.loop();
  webServer.handleClient();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    String value = String(analogRead(ldrPin));
    // Serial.println(value);

    webSocket.broadcastTXT(value);
  }
}
