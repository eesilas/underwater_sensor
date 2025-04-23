#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

unsigned char buffer_RTT[4] = {0};
uint8_t CS;
#define COM 0x55
int Distance = -1, lastStableDistance = -1;
unsigned long stableStartTime = 0;

// Use SoftwareSerial with pins that don't conflict with hardware serial
SoftwareSerial mySerial(2, 3); // RX, TX (avoid pins 0,1 as they're used for USB)

// For the SSD1306 OLED display Module
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Initializing..."));
  display.display();
  delay(1000);
}

void loop() {
  mySerial.write(COM); // Request distance measurement
  delay(100); // Give sensor time to respond

  // Debug output to display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("Request sent..."));
  display.display();

  // Wait for data with timeout
  unsigned long startTime = millis();
  while (mySerial.available() < 4 && millis() - startTime < 200) {
    // Wait for data or timeout after 200ms
  }

  if (mySerial.available() >= 4) {
    // Debug output
    display.setCursor(0, 15);
    display.print(F("Data received"));
    display.display();

    if (mySerial.read() == 0xff) {
      buffer_RTT[0] = 0xff;
      for (int i = 1; i < 4; i++) {
        buffer_RTT[i] = mySerial.read();
      }
      
      CS = buffer_RTT[0] + buffer_RTT[1] + buffer_RTT[2];

      if (buffer_RTT[3] == CS) {
        int newDistance = (buffer_RTT[1] << 8) + buffer_RTT[2];
        
        // Debug output
        display.setCursor(0, 30);
        display.print(F("Valid data"));
        display.display();
        delay(500); // Show debug message briefly

        if (Distance == -1) {  
          // First valid reading
          Distance = newDistance;
          lastStableDistance = newDistance;
          stableStartTime = millis();
          updateDisplay(lastStableDistance);
        } 
        else if (newDistance == Distance) {
          // Check if stable for 500ms
          if (millis() - stableStartTime >= 500 && newDistance != lastStableDistance) {
            lastStableDistance = newDistance;
            updateDisplay(lastStableDistance);
          }
        } 
        else {
          // Reset stability timer on change
          Distance = newDistance;
          stableStartTime = millis();
        }

        Serial.print(F("Distance: "));
        Serial.print(Distance);
        Serial.println(F(" mm"));
      }
      else {
        Serial.println(F("Checksum error"));
        display.setCursor(0, 45);
        display.print(F("Checksum error"));
        display.display();
      }
    }
  }
  else {
    Serial.println(F("No data or incomplete data"));
    display.setCursor(0, 45);
    display.print(F("No data received"));
    display.display();
    delay(1000);
  }
}

void updateDisplay(int value) {
  display.clearDisplay();
  display.setCursor(0, 45);
  //display.setTextSize(2);
  display.setTextSize(1);
  display.print(F("Distance:"));
  
  display.setCursor(0, 48);
  display.setTextSize(1);
  display.print(value);
  //display.setTextSize(1);
  //display.print(F(" mm"));
  
  display.display();
}