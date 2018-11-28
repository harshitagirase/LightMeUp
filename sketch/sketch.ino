#include <SPI.h>
#include <WiFi101.h>
#include <FastLED.h>

// Configuration for connected LED Strip.
#define NUM_OF_LEDS 300
#define PIN 11

// The maximum number of counting ticks used for LED modes.
// The lower the faster.
#define MAX_TICK 100

// Enums used for state machine. Do not modify these.
#define READ_DONE 0
#define READ_MODE 1
#define READ_LED_1_RED 2
#define READ_LED_1_GREEN 3
#define READ_LED_1_BLUE 4
#define READ_LED_2_RED 5
#define READ_LED_2_GREEN 6
#define READ_LED_2_BLUE 7
// --- Seriously.

// WiFi Configuration.
char ssid[] = "Snakes and Flakes";
char pass[] = "anguswhatsthewifi";

// Server to connect to.
char server[] = "lightmeup.herokuapp.com";

// WiFi client variables.
WiFiClient client;
int status = WL_IDLE_STATUS;


// LED setup
CRGB leds[NUM_OF_LEDS];

int state = READ_MODE;

String mode = "fade";
int led1r = 5;
int led1g = 50;
int led1b = 5;
int led2r = 50;
int led2g = 5;
int led2b = 50;

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
unsigned long lastTickTime = 0;            // last time tick is updated, in milliseconds
unsigned int tick = 0;
unsigned int fadeIndexPos = 0;
const unsigned long postingInterval = 10L * 1000L; // delay between polling, in millisecond
const unsigned long tickInterval = 1L; // delay between ticks, in millisecond

// Variables for fade mode
int colorToShow = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  FastLED.setMaxPowerInVoltsAndMilliamps(4,2000); 
  FastLED.addLeds<NEOPIXEL, PIN>(leds, NUM_OF_LEDS);
  
  for (int i = 0; i < NUM_OF_LEDS; i++) {
    leds[i] = CRGB(205, 4, 30);
  }
  FastLED.show();
  
  WiFi.setPins(8,7,4,2);
  Serial.begin(9600);
  while (!Serial);

  // Booting: Yellow
  for (int i = 0; i < NUM_OF_LEDS; i++) {
    leds[i] = CRGB(255, 204, 0);
  }
  FastLED.show();

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");

    // WiFi shield not working: Red
    while (true) {
      for (int i = 0; i < NUM_OF_LEDS; i++) {
        leds[i] = CRGB(204, 0, 51);
      }
      
      FastLED.show();
      delay(1000);
      
      for (int i = 0; i < NUM_OF_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
      
      FastLED.show();
      delay(1000);
    }
  }

  // Connecting: Pink
  for (int i = 0; i < NUM_OF_LEDS; i++) {
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
  for (int i = 0; i < NUM_OF_LEDS; i++) {
    leds[i] = CRGB(73, 102, 100);
  }
  FastLED.show();
}
 
// the loop function runs over and over again forever
void loop() {
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

  
  if (millis() - lastTickTime > tickInterval) {
    tick = tick > MAX_TICK ? 0 : tick + 1;

    if (tick % 5 == 0) {
        fadeIndexPos = fadeIndexPos > NUM_OF_LEDS ? 0 : fadeIndexPos + 1;
        if (fadeIndexPos == 0) {
          colorToShow = colorToShow == 0 ? 1 : 0;
        }
    }
    
    lastTickTime = millis();
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
            newLed1r = int(data.toInt());
            data = "";
            state = READ_LED_1_GREEN;
          }
          break;
        case READ_LED_1_GREEN:
          if (c != ',') {
            data.concat(c);
          } else {
            newLed1g = int(data.toInt());
            data = "";
            state = READ_LED_1_BLUE;
          }
          break;
        case READ_LED_1_BLUE:
          if (c != ':') {
            data.concat(c);
          } else {
            newLed1b = int(data.toInt());
            data = "";
            state = READ_LED_2_RED;
          }
          break;
        case READ_LED_2_RED:
          if (c != ',') {
            data.concat(c);
          } else {
            newLed2r = int(data.toInt());
            data = "";
            state = READ_LED_2_GREEN;
          }
          break;
        case READ_LED_2_GREEN:
          if (c != ',') {
            data.concat(c);
          } else {
            newLed2g = int(data.toInt());
            data = "";
            state = READ_LED_2_BLUE;
          }
          break;
        case READ_LED_2_BLUE:
          if (c != ':') {
            data.concat(c);
          } else {
            newLed2b = int(data.toInt());
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
    if (
        mode != newMode ||
        led1r != newLed1r ||
        led1g != newLed1g ||
        led1b != newLed1b ||
        led2r != newLed2r ||
        led2g != newLed2g ||
        led2b != newLed2b
      ) {
      Serial.println("change detected");
      Serial.print(mode);
      Serial.print(" -> ");
      Serial.println(newMode);
      
      Serial.print(led1r, DEC);
      Serial.print(" -> ");
      Serial.println(newLed1r, DEC);

      Serial.print(led1r, DEC);
      Serial.print(" -> ");
      Serial.println(newLed1r, DEC);

      Serial.print(led1g, DEC);
      Serial.print(" -> ");
      Serial.println(newLed1g, DEC);

      Serial.print(led1b, DEC);
      Serial.print(" -> ");
      Serial.println(newLed1b, DEC);

      Serial.print(led2r, DEC);
      Serial.print(" -> ");
      Serial.println(newLed2r, DEC);

      Serial.print(led2g, DEC);
      Serial.print(" -> ");
      Serial.println(newLed2g, DEC);

      Serial.print(led2b, DEC);
      Serial.print(" -> ");
      Serial.println(newLed2b, DEC);
      
      mode = newMode;
      led1r = newLed1r;
      led1g = newLed1g;
      led1b = newLed1b;
      led2r = newLed2r;
      led2g = newLed2g;
      led2b = newLed2b;
    }
  }

  // Now set the leds.
  if (mode == "fade") {
    if (colorToShow) {
      leds[fadeIndexPos] = CRGB(led1r, led1g, led1b);  
    } else {
      leds[fadeIndexPos] = CRGB(led2r, led2g, led2b); 
    }
    
  } else if (mode == "blink") {
    for (int i = 0; i < NUM_OF_LEDS; i++) {
      if (tick < (MAX_TICK / 2)) {
        leds[i] = CRGB(led1r, led1g, led1b);
      } else {
        leds[i] = CRGB(led2r, led2g, led2b);
      }
    }
    
  } else {
    // Default is a solid color
    for (int i = 0; i < NUM_OF_LEDS; i++) {
      leds[i] = CRGB(led1r, led1g, led1b);
    }
  }
  
  FastLED.show();
}

void httpRequest() {
  // if there's a successful connection:
//  while (true) {
    // close any connection before send a new request.
    // This will free the socket on the WiFi shield
    client.stop();
    
    if (client.connect(server, 80)) {
      client.println("GET /data HTTP/1.1");
      client.println("Host: lightmeup.herokuapp.com");
      client.println("Connection: close");
      client.println();
  
      // note the time that the connection was made:
      lastConnectionTime = millis();
//      break;
      
    } else {
      // if you couldn't make a connection:
      Serial.println("connection failed");
    }
//  }
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
