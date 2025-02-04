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

// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------

#define greenLedPin 26
#define BTN_PIN 23
#define LED_BUILTIN 2

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

const uint8_t DEBOUNCE_DELAY = 10; // in milliseconds

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
Led greenLed(greenLedPin, false);
Led onboardLed (LED_BUILTIN, false);
Button button = { BTN_PIN, HIGH, 0, 0 };
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
        Serial.println(F("OLED Screen Mounted Successfully!"));
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 10);
        display.println("Hello!");
        display.display();
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
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    button.read();

    if (button.pressed()) greenLed.on = !greenLed.on;

    greenLed.update();
}