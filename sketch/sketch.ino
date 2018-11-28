#include <SPI.h>
#include <WiFi101.h>
#include <FastLED.h>

#define NUMS_OF_LEDS 300
#define PIN 11

char ssid[] = "Honor 6X";
char pass[] = "twowordsallcaps";
int status = WL_IDLE_STATUS;

char server[] = "www.google.com";
WiFiClient client;

CRGB leds[NUMS_OF_LEDS];

// the setup function runs once when you press reset or power the board
void setup() {
  FastLED.setMaxPowerInVoltsAndMilliamps(4,2000); 
  FastLED.addLeds<NEOPIXEL, PIN>(leds, NUMS_OF_LEDS);
  
  for (int i = 0; i < NUMS_OF_LEDS; i++) {
    leds[i] = CRGB::White;
  }
  FastLED.show();
  
  WiFi.setPins(8,7,4,2);
  Serial.begin(9600);
  while (!Serial);

  // Booting: Yellow
  for (int i = 0; i < NUMS_OF_LEDS; i++) {
    leds[i] = CRGB(255, 204, 0);
  }
  FastLED.show();

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");

    // WiFi shield not working: Red
    for (int i = 0; i < NUMS_OF_LEDS; i++) {
      leds[i] = CRGB(204, 0, 51);
    }
    
    FastLED.show();
    while (true);
  }

  // Connecting: Pink
  for (int i = 0; i < NUMS_OF_LEDS; i++) {
    leds[i] = CRGB(255, 71, 117);
  }
  FastLED.show();

  
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid);
//    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    // wait 10 seconds for connection:
    delay(10000);
  }

  
  printWiFiStatus();
  
  // Connected: Green
  for (int i = 0; i < NUMS_OF_LEDS; i++) {
    leds[i] = CRGB(73, 102, 100);
  }
  FastLED.show();

  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  }
}
 
// the loop function runs over and over again forever
void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}
//  for (int i = 0; i < NUMS_OF_LEDS; i++) {
//    leds[i] = CRGB::White;
//    FastLED.show();
//    delay(1);
//    leds[i] = CRGB::Green;
//  }
//
//  for (int i = 0; i < NUMS_OF_LEDS; i++) {
//    leds[i] = CRGB::Red;
//    FastLED.show();
//    delay(1);
//    leds[i] = CRGB::Blue;
//  }
//
//  for (int i = 0; i < NUMS_OF_LEDS; i++) {
//    leds[i] = CRGB::Black;
//    FastLED.show();
//    delay(1);
//    leds[i] = CRGB::Red;
//  }
//
//  for (int i = 0; i < NUMS_OF_LEDS; i++) {
//    leds[i] = CRGB::Black;
//    FastLED.show();
//    delay(1);
//    leds[i] = CRGB::White;
//  }

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
