// Nano_ESP32_Sensor_Final.ino  
// Final firmware for the Sensor Unit (Arduino Nano ESP32).  
// Improved version with immediate display updates on mode changes

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
BLEService plantService("fc5fd9af-66af-9f8e-eb7d-ba9f82087613");  
BLEUnsignedIntCharacteristic moistureCharacteristic("81dc8cfd-8dbf-9ef8-d873-8068b978a752", BLERead | BLENotify);  
BLECharCharacteristic commandCharacteristic("d3dd9acf-7656-99fb-0907-6bb7a3488888", BLEWrite);

enum Mode { NORMAL, CONTINUOUS };  
Mode currentMode = NORMAL;
Mode previousMode = NORMAL; // Track mode changes

unsigned long previousMillis = 0;  
const long normalInterval = 300000; // 5 minutes  
const long continuousInterval = 5000;  // 5 seconds

bool initialReadingSent = false;
int lastMoistureReading = 0; // Store last reading for display

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
  display.println("🌱 Sensor Unit");  
  display.setCursor(0,16);  
  display.println("Starting...");  
  display.display();

  // -- Initialize Button --  
  pinMode(buttonPin, INPUT_PULLUP);

  // -- Initialize BLE --  
  if (!BLE.begin()) {  
    Serial.println("BLE initialization failed!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("❌ BLE Error");
    display.display();
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
  updateDisplayStatus("Advertising...", 0, NORMAL);
  
  Serial.println("✓ Sensor unit initialized");
  Serial.println("🔍 Waiting for connection...");
}

void loop() {  
  BLEDevice central = BLE.central();

  if (central) {  
    Serial.println("✓ Control unit connected!");
    updateDisplayStatus("Connected!", lastMoistureReading, currentMode);

    initialReadingSent = false;

    while (central.connected()) {  
      // Send initial reading immediately when subscribed
      if (!initialReadingSent) {  
        if (moistureCharacteristic.subscribed()) {  
          updateAndSendMoisture();  
          previousMillis = millis();  
          initialReadingSent = true;
          Serial.println("📊 Initial moisture reading sent");
        }  
      }

      // Send periodic readings based on current mode
      if (initialReadingSent) {  
          unsigned long currentMillis = millis();  
          long interval = (currentMode == CONTINUOUS) ? continuousInterval : normalInterval;

          if (currentMillis - previousMillis >= interval) {  
            previousMillis = currentMillis;  
            updateAndSendMoisture();  
          }  
      }

      // Check for mode change commands from control unit
      if (commandCharacteristic.written()) {  
        char cmd = commandCharacteristic.value();  
        Mode newMode = currentMode;
        
        if (cmd == 'C') {  
          newMode = CONTINUOUS;  
        } else if (cmd == 'N') {  
          newMode = NORMAL;  
        }
        
        // If mode changed, update immediately
        if (newMode != currentMode) {
          currentMode = newMode;
          Serial.println("🔄 Mode changed to: " + String(currentMode == CONTINUOUS ? "CONTINUOUS" : "NORMAL"));
          
          // Immediate display update when mode changes
          updateDisplayStatus("Connected!", lastMoistureReading, currentMode);
          
          // Also send immediate moisture reading when switching to continuous mode
          if (currentMode == CONTINUOUS) {
            Serial.println("💧 Pump starting - switching to fast readings");
            updateAndSendMoisture();
            previousMillis = millis(); // Reset timer
          } else {
            Serial.println("⏸️ Pump stopped - returning to normal readings");
          }
        }
      }  
        
      delay(10);   
    }

    Serial.println("📡 Control unit disconnected");
    updateDisplayStatus("Disconnected", lastMoistureReading, currentMode);
    delay(1000);
    updateDisplayStatus("Advertising...", lastMoistureReading, currentMode);
  }

  // Manual button reading (works even when disconnected)
  if (digitalRead(buttonPin) == LOW) {  
      delay(50); // Debounce
      if (digitalRead(buttonPin) == LOW) {  
         Serial.println("🔘 Manual reading requested");
         updateAndSendMoisture();  
         
         // Show "Manual Read" briefly
         display.clearDisplay();
         display.setTextSize(1);
         display.setCursor(0,0);
         display.println("Manual Reading...");
         display.setTextSize(2);
         display.setCursor(20, 20);
         display.print(lastMoistureReading);
         display.print("%");
         display.display();
         
         delay(2000);  
         updateDisplayStatus(central.connected() ? "Connected!" : "Advertising...", lastMoistureReading, currentMode);
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
  
  lastMoistureReading = moisturePercent; // Store for display

  // Send via BLE if connected
  if (BLE.central()) {  
    moistureCharacteristic.writeValue(moisturePercent);  
    Serial.println("📊 Sent: " + String(moisturePercent) + "% [" + 
                  String(currentMode == CONTINUOUS ? "CONTINUOUS" : "NORMAL") + "]");
  }

  // Update display with new reading
  updateDisplayStatus(BLE.central() ? "Connected!" : "Advertising...", moisturePercent, currentMode);
}

void updateDisplayStatus(String connectionStatus, int moisture, Mode mode) {
  display.clearDisplay();  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Connection status
  display.setCursor(0,0);  
  display.println(connectionStatus);  
  
  // Large moisture reading
  display.setTextSize(2);  
  display.setCursor(20, 16);  
  display.print(moisture);  
  display.print("%");  
  
  // Mode indicator with visual distinction
  display.setTextSize(1);  
  display.setCursor(0, 40);  
  if (mode == CONTINUOUS) {
    display.println("MODE: 💧 PUMPING");
    // Add blinking indicator for pump mode
    static bool blink = false;
    if (blink) {
      display.setCursor(110, 40);
      display.print("●");
    }
    blink = !blink;
  } else {
    display.println("MODE: NORMAL");
  }
  
  // Additional status info
  display.setCursor(0, 52);
  unsigned long uptime = millis() / 60000; // minutes
  display.print("Up: " + String(uptime) + "m");
  
  if (BLE.central()) {
    display.setCursor(60, 52);
    display.print("RSSI: " + String(BLE.central().rssi()));
  }
  
  display.display();  
}
