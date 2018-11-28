#include <SPI.h>
#include <WiFi101.h>
#include <FastLED.h>

#define NUMS_OF_LEDS 300
#define PIN 11

#define READ_DONE 0
#define READ_MODE 1
#define READ_LED_1_RED 2
#define READ_LED_1_GREEN 3
#define READ_LED_1_BLUE 4
#define READ_LED_2_RED 5
#define READ_LED_2_GREEN 6
#define READ_LED_2_BLUE 7

// WiFi configuration
char ssid[] = "Snakes and Flakes";
char pass[] = "anguswhatsthewifi";
int status = WL_IDLE_STATUS;

// Server to connect to
char server[] = "0beb9d75.ngrok.io";
WiFiClient client;

// LED setup
CRGB leds[NUMS_OF_LEDS];

int state = READ_MODE;

String mode = "default";
int led1r = 255;
int led1g = 255;
int led1b = 255;
int led2r = 0;
int led2g = 0;
int led2b = 0;

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between updates, in millisecond

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
    while (true) {
      for (int i = 0; i < NUMS_OF_LEDS; i++) {
        leds[i] = CRGB(204, 0, 51);
      }
      
      FastLED.show();
      delay(1000);
      
      for (int i = 0; i < NUMS_OF_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
      
      FastLED.show();
      delay(1000);
    }
  }

  // Connecting: Pink
  for (int i = 0; i < NUMS_OF_LEDS; i++) {
    leds[i] = CRGB(255, 71, 117);
  }
  FastLED.show();
  
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    // wait 10 seconds for retry:
    delay(10000);
  }
  
  printWiFiStatus();
  
  // Connected: Green
  for (int i = 0; i < NUMS_OF_LEDS; i++) {
    leds[i] = CRGB(73, 102, 100);
  }
  FastLED.show();
}
 
// the loop function runs over and over again forever
void loop() {
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  } 

  String data = "";
  String newMode = "";
  int newLed1r = 0;
  int newLed1g = 0;
  int newLed1b = 255;
  int newLed2r = 255;
  int newLed2g = 0;
  int newLed2b = 0;

  // This boolean determines when the body starts.
  int foundBody = 0;
  while (client.available()) {
    char c = client.read();
    if (foundBody) {
      switch (state) {
        case READ_MODE:
          if (c != '@') {
            data.concat(c);
          } else {
            newMode = String(data);
            data = "";
            state = READ_LED_1_RED;
          }
          break;
        case READ_LED_1_RED:
          if (c != ',') {
            data.concat(c);
          } else {
            newLed1r = data.toInt();
            data = "";
            state = READ_LED_1_GREEN;
          }
          break;
        case READ_LED_1_GREEN:
          if (c != ',') {
            data.concat(c);
          } else {
            newLed1g = data.toInt();
            data = "";
            state = READ_LED_1_BLUE;
          }
          break;
        case READ_LED_1_BLUE:
          if (c != ':') {
            data.concat(c);
          } else {
            newLed1b = data.toInt();
            data = "";
            state = READ_LED_2_RED;
          }
          break;
        case READ_LED_2_RED:
          if (c != ',') {
            data.concat(c);
          } else {
            newLed2r = data.toInt();
            data = "";
            state = READ_LED_2_GREEN;
          }
          break;
        case READ_LED_2_GREEN:
          if (c != ',') {
            data.concat(c);
          } else {
            newLed2g = data.toInt();
            data = "";
            state = READ_LED_2_BLUE;
          }
          break;
        case READ_LED_2_BLUE:
          if (c != ':') {
            data.concat(c);
          } else {
            newLed2b = data.toInt();
            data = "";
            state = READ_MODE;
          }
          break;
        default:
          break;
      }
    }
    
    if (c == '$') {
      foundBody = 1;
    }
  }

  if (foundBody) {
    if (mode != newMode || led1r != newLed1r || led1g != newLed1g || led1b != newLed1b || led2r != newLed2r || led2g != newLed2g || led2b != newLed2b) {
      mode = newMode;
      led1r = newLed1r;
      led1g = newLed1g;
      led1b = newLed1b;
      led2r = newLed2r;
      led2g = newLed2g;
      led2b = newLed2b;
      
      Serial.println(mode);
      Serial.println(led1r);
      Serial.println(led1g);
      Serial.println(led1b);
      Serial.println(led2r);
      Serial.println(led2g);
      Serial.println(led2b);
    }
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

void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    client.println("GET /data HTTP/1.1");
    client.println("Host: 0beb9d75.ngrok.io");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

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
