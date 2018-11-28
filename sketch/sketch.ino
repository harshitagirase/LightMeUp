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

// Tick and polling configurations.
const unsigned long postingInterval = 3L * 1000L; // delay between polling, in millisecond
const unsigned long tickInterval = 1L; // delay between ticks, in millisecond

// Variables to store webserver configurations.
String mode = "fade";
int led1r = 5;
int led1g = 50;
int led1b = 5;
int led2r = 50;
int led2g = 5;
int led2b = 50;

// Tick and polling related variables.
unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
unsigned long lastTickTime = 0;            // last time tick is updated, in milliseconds
unsigned int tick = 0;
unsigned int fadeIndexPos = 0;

// Variable for state machine for parsing the body of HTTP response
int state = READ_MODE;

// Variables for fade modes.
int colorToShow = 0;

void setup() {
  // Set up LEDs. This also fixes the flickering issue.
  FastLED.setMaxPowerInVoltsAndMilliamps(4,2000);
  FastLED.addLeds<NEOPIXEL, PIN>(leds, NUM_OF_LEDS);

  // Clear out previous settings.
  for (int i = 0; i < NUM_OF_LEDS; i++) {
    leds[i] = CRGB(205, 4, 30);
  }
  FastLED.show();
  
  WiFi.setPins(8,7,4,2); // Enable WiFi module

  // Wait for Serial to connect
  Serial.begin(9600); 
  while (!Serial);

  // Booting: Yellow LEDs
  for (int i = 0; i < NUM_OF_LEDS; i++) {
    leds[i] = CRGB(255, 204, 0);
  }
  FastLED.show();

  // If the WiFi module is missing, blink red and don't do anything else..
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

  // Connecting to a WiFi network: Pink
  for (int i = 0; i < NUM_OF_LEDS; i++) {
    leds[i] = CRGB(255, 71, 117);
  }
  FastLED.show();
  
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  // Attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    // Wit 10 seconds for next attempt
    delay(10000);
  }
  
  printWiFiStatus();
  
  // Connected: show green LEDs
  for (int i = 0; i < NUM_OF_LEDS; i++) {
    leds[i] = CRGB(10, 102, 0);
  }
  FastLED.show();
}
 
void loop() {
  // Update millisecond timestamp for posting.
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
  
  // Update millisecond timestamp for ticks
  if (millis() - lastTickTime > tickInterval) {
    tick = tick > MAX_TICK ? 0 : tick + 1;
    
    // These are used for the fade effect.
    if (tick % 5 == 0) {
        fadeIndexPos = fadeIndexPos > NUM_OF_LEDS ? 0 : fadeIndexPos + 1;
        if (fadeIndexPos == 0) {
          colorToShow = colorToShow == 0 ? 1 : 0;
        }
    }
    
    lastTickTime = millis();
  } 

  // Variables to store the split chunks in.
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
    // Begin the state machine to parse the chunks when we found the body
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
    
    // The body starts with a dollar sign.
    if (c == '$') {
      foundBody = 1;
    }
  }

  // Only apply changes if there are any only.
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

  // Now set the LEDs effects.
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
    // Close any connection before send a new request.
    // This will free the socket on the WiFi shield
    client.stop();
    
    if (client.connect(server, 80)) {
      client.println("GET /data HTTP/1.1");
      client.println("Host: lightmeup.herokuapp.com");
      client.println("Connection: close");
      client.println();
  
      // note the time that the connection was made:
      lastConnectionTime = millis();
      
    } else {
      Serial.println("connection failed");
      // Fail silently here, we don't want to change the LEDs to tell people that its disconnected.
    }
}

// Helper function to print WiFi status.
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
