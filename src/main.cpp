/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include "LittleFS.h"
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------

//Pin Defines
#define GREEN_LED_PIN 26
#define BTN_PIN 23
#define LED_BUILTIN 2

//OLED Defines
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

//Webserver Defines
#define HTTP_PORT 80

// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

const uint8_t DEBOUNCE_DELAY = 10; // in milliseconds

const char* SSID = "SamberNet";
const char* PASSWORD = "Amber1664";

// ----------------------------------------------------------------------------
// Definition of the LED component will leave as a struct as basically everything will be public.  I need to access the on variable in the loop function.
// ----------------------------------------------------------------------------

struct Led {
    // state variables
    uint8_t pin;
    bool    on;

    Led(uint8_t pin, bool on=false) : pin(pin), on(on) {
      pinMode(pin, OUTPUT);
      update();
    }

    // methods
    void update() {
        digitalWrite(pin, on ? HIGH : LOW);
    }
};

// ----------------------------------------------------------------------------
// Definition of the Button component
// ----------------------------------------------------------------------------

struct Button {
    // state variables
    uint8_t  pin;
    bool     lastReading;
    uint32_t lastDebounceTime;
    uint16_t state;

    // methods determining the logical state of the button
    bool pressed()                { return state == 1; }
    bool released()               { return state == 0xffff; }
    bool held(uint16_t count = 0) { return state > 1 + count && state < 0xffff; }

    // method for reading the physical state of the button
    void read() {
        // reads the voltage on the pin connected to the button
        bool reading = digitalRead(pin);

        // if the logic level has changed since the last reading,
        // we reset the timer which counts down the necessary time
        // beyond which we can consider that the bouncing effect
        // has passed.
        if (reading != lastReading) {
            lastDebounceTime = millis();
        }

        // from the moment we're out of the bouncing phase
        // the actual status of the button can be determined
        if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
            // don't forget that the read pin is pulled-up
            bool pressed = reading == LOW;
            if (pressed) {
                     if (state  < 0xfffe) state++;
                else if (state == 0xfffe) state = 2;
            } else if (state) {
                state = state == 0xffff ? 0 : 0xffff;
            }
        }

        // finally, each new reading is saved
        lastReading = reading;
    }
};

// ----------------------------------------------------------------------------
// Definition of global variables
// ----------------------------------------------------------------------------
Led greenLed(GREEN_LED_PIN, false);
Led onboardLed (LED_BUILTIN, false);
Button button = { BTN_PIN, HIGH, 0, 0 };
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer webServer{HTTP_PORT};

// ----------------------------------------------------------------------------
// Initialization of LittleFS
// ----------------------------------------------------------------------------

void initLittleFS(){
    if (!LittleFS.begin()) {
        Serial.println("Cannot mount LittleFS Volume"); 
        while(1) {
            onboardLed.on = millis() %  200 <50;
            onboardLed.update();
        } 
    } 
    else {
        Serial.println(F("LittleFS Mounted Successfully!"));
        }
}
// ----------------------------------------------------------------------------
// Initialization of OLED
// ----------------------------------------------------------------------------

// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
void initOLED() {
    delay(500);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    } 
    else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.display();
    }
}

// ----------------------------------------------------------------------------
// Initialization of Wifi - prints error messages to the OLED if it falls over.
// ----------------------------------------------------------------------------


void connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 10) { 
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Connecting");
        
        for (int i = 0; i <= attempt % 3; i++) {
            display.print(".");
        }

        display.display();
        Serial.print("Attempt ");
        Serial.println(attempt + 1);
        
        delay(1000);
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Connected!");
        display.display();
        Serial.println("Connected!");
        delay(2000);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("IP: ");
        display.println(WiFi.localIP());
        display.print("MAC:");
        display.println(WiFi.macAddress());
        display.display();

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC Address: ");
        Serial.println(WiFi.macAddress());
    } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Connection Failed!");
        display.display();
        Serial.println("Failed to connect to WiFi");
    }
}

// ----------------------------------------------------------------------------
// Initialization of WebServer
// ----------------------------------------------------------------------------

//String Processor (convert %STATE% to 'on' or 'off' depending on the led.on state)
String processor(const String &var) {
    return String(var == "STATE" && greenLed.on ? "on" : "off");
}

void onRootRequest(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/index.html", "text/html", false, processor);
}

void initWebServer() {
    webServer.on("/", onRootRequest);
    webServer.serveStatic("/", LittleFS, "/");
    webServer.begin();

    // Small delay to allow the server to start properly
    delay(100);

    // Check if the server is listening on port 80
    WiFiClient client;
    if (client.connect(WiFi.localIP(), 80)) {
        display.println("Webserver Available!");
        display.display();
        Serial.println(F("WebServer Available!"));
        
        client.stop();  // Close the test connection
    } else {
        display.println("Webserver Failed to Start!");
        display.display();
        Serial.println(F("WebServer Failed to initialised"));
    }

}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    pinMode(button.pin, INPUT);
    Serial.begin(115200);delay(500);

    initLittleFS();
    initOLED();
    connectToWiFi();
    initWebServer();
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    button.read();

    if (button.pressed()) greenLed.on = !greenLed.on;

    greenLed.update();
}