// Nano_ESP32_Sensor_Final.ino  
// Final firmware for the Sensor Unit (Arduino Nano ESP32).  
// This version is cleaned up for deployment.

#include <ArduinoBLE.h>  
#include <SPI.h>  
#include <Wire.h>  
#include <Adafruit_GFX.h>  
#include <Adafruit_SSD1306.h>

// -- Configuration --  
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_RESET    -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int soilSensorPin = A0;  
const int buttonPin = 2;

// BLE Service and Characteristics  
BLEService plantService("a70a845a-115f-4e39-9628-654a3e532168");  
BLEUnsignedIntCharacteristic moistureCharacteristic("3c8737b8-386d-49a3-832f-3b136423d20d", BLERead | BLENotify);  
BLECharCharacteristic commandCharacteristic("8e88457c-2101-44a6-b452-166258f33333", BLEWrite);

enum Mode { NORMAL, CONTINUOUS };  
Mode currentMode = NORMAL;

unsigned long previousMillis = 0;  
const long normalInterval = 300000; // 5 minutes  
const long continuousInterval = 5000;  // 5 seconds

bool initialReadingSent = false;

void setup() {  
  Serial.begin(115200);

  // -- Initialize OLED Display --  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
    Serial.println(F("SSD1306 allocation failed"));  
    for(;;);  
  }  
  display.display();  
  delay(1000);  
  display.clearDisplay();  
  display.setTextSize(1);  
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0,0);  
  display.println("Sensor Unit Starting...");  
  display.display();

  // -- Initialize Button --  
  pinMode(buttonPin, INPUT_PULLUP);

  // -- Initialize BLE --  
  if (!BLE.begin()) {  
    while (1);  
  }

  BLE.setLocalName("NanoPlantSensor");  
  BLE.setAdvertisedService(plantService);  
  plantService.addCharacteristic(moistureCharacteristic);  
  plantService.addCharacteristic(commandCharacteristic);  
  BLE.addService(plantService);

  moistureCharacteristic.writeValue(0);  
  commandCharacteristic.writeValue('N');

  BLE.advertise();  
  display.clearDisplay();  
  display.println("Advertising...");  
  display.display();  
}

void loop() {  
  BLEDevice central = BLE.central();

  if (central) {  
    display.clearDisplay();  
    display.setCursor(0,0);  
    display.println("Connected!");  
    display.display();

    initialReadingSent = false;

    while (central.connected()) {  
      if (!initialReadingSent) {  
        if (moistureCharacteristic.subscribed()) {  
          updateAndSendMoisture();  
          previousMillis = millis();  
          initialReadingSent = true;  
        }  
      }

      if (initialReadingSent) {  
          unsigned long currentMillis = millis();  
          long interval = (currentMode == CONTINUOUS) ? continuousInterval : normalInterval;

          if (currentMillis - previousMillis >= interval) {  
            previousMillis = currentMillis;  
            updateAndSendMoisture();  
          }  
      }

      if (commandCharacteristic.written()) {  
        char cmd = commandCharacteristic.value();  
        if (cmd == 'C') {  
          currentMode = CONTINUOUS;  
        } else if (cmd == 'N') {  
          currentMode = NORMAL;  
        }  
      }  
        
      delay(10);   
    }

    display.clearDisplay();  
    display.setCursor(0,0);  
    display.println("Disconnected.");  
    display.println("Advertising...");  
    display.display();  
  }

  if (digitalRead(buttonPin) == LOW) {  
      delay(50);  
      if (digitalRead(buttonPin) == LOW) {  
         updateAndSendMoisture();  
         delay(2000);  
         display.clearDisplay();  
         display.setCursor(0,0);  
         display.println(central.connected() ? "Connected" : "Advertising...");  
         display.display();  
      }  
  }  
}

void updateAndSendMoisture() {  
  // Calibrated values for the sensor.  
  const int dryValue = 3347;  
  const int wetValue = 1408;

  int sensorValue = analogRead(soilSensorPin);  
  int moisturePercent = map(sensorValue, dryValue, wetValue, 0, 100);  
  moisturePercent = constrain(moisturePercent, 0, 100);

  if (BLE.central()) {  
    moistureCharacteristic.writeValue(moisturePercent);  
  }

  display.clearDisplay();  
  display.setCursor(0,0);  
  display.println(BLE.central() ? "Connected" : "Advertising...");  
  display.setTextSize(2);  
  display.setCursor(20, 20);  
  display.print(moisturePercent);  
  display.print("%");  
  display.setTextSize(1);  
  display.setCursor(20, 45);  
  display.print(currentMode == CONTINUOUS ? "MODE: CONTINUOUS" : "MODE: NORMAL");  
  display.display();  
}