// MKR1010_Plant_Monitor_Standalone.ino
// Complete IoT plant monitoring system - no cloud dependencies
// Educational version focusing on core IoT concepts
// WiFi functionality removed for faster startup and standalone operation

#include <SPI.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -- Configuration --
// -- Pump Logic Configuration --
const int MOISTURE_LOW_THRESHOLD = 30;   // Start pump when moisture < 30%
const int MOISTURE_HIGH_THRESHOLD = 60;  // Stop pump when moisture > 60%
const unsigned long PUMP_TIMEOUT_SECONDS = 300; // Max pump runtime: 5 minutes

// -- Pin & Display Configuration --
const int relayPin = 1;
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BLE Service and Characteristics UUIDs
const char* serviceUuid = "b81b956b-226f-5f4a-a739-765b4f643279";
const char* moistureCharUuid = "4d9848c9-497e-5ab4-943f-4c247534e31e";
const char* commandCharUuid = "9f99568d-3212-55b7-c563-277369044444";

// BLE objects
BLECharacteristic moistureCharacteristic;
BLECharacteristic commandCharacteristic;

// Global state
bool isPumping = false;
unsigned long pumpStartTime = 0;
int lastMoisture = -1;
int totalWateringCycles = 0;
unsigned long systemStartTime = 0;

// Data logging arrays (last 10 readings)
int moistureHistory[10] = {0};
bool pumpHistory[10] = {false};
unsigned long timeHistory[10] = {0};
int historyIndex = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("====================================");
  Serial.println("🌱 SMART PLANT MONITOR SYSTEM 🌱");
  Serial.println("====================================");
  Serial.println("Features:");
  Serial.println("✓ Wireless moisture sensing (BLE)");
  Serial.println("✓ Automatic pump control");
  Serial.println("✓ OLED status display");
  Serial.println("✓ Data logging to Serial");
  Serial.println("✓ Historical data tracking");
  Serial.println("✓ Standalone operation (no WiFi)");
  Serial.println("====================================\n");

  systemStartTime = millis();

  // -- Initialize OLED --
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ ERROR: OLED initialization failed!");
    for(;;);
  }
  updateDisplay("🌱 Plant Monitor", "Starting...", "System Init");
  Serial.println("✓ OLED Display initialized");

  // -- Initialize Relay --
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  Serial.println("✓ Pump relay initialized");

  // -- Initialize BLE --
  if (!BLE.begin()) {
    Serial.println("❌ ERROR: BLE initialization failed!");
    updateDisplay("❌ BLE Error", "Check wiring", "Reset device");
    while (1);
  }
  Serial.println("✓ BLE initialized successfully");

  startScan();
  
  Serial.println("\n🔍 Searching for plant sensor...");
  printSystemInfo();
}

void loop() {
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    BLE.stopScan();
    controlPeripheral(peripheral);
    startScan();
  }
  
  // Print periodic status updates
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 30000) { // Every 30 seconds
    lastStatusUpdate = millis();
    printSystemStatus();
  }
}

void controlPeripheral(BLEDevice peripheral) {
  Serial.println("\n📡 Found plant sensor, attempting connection...");
  
  if (!peripheral.connect()) {
    Serial.println("❌ Failed to connect to sensor");
    return;
  }
  Serial.println("✓ Connected to plant sensor successfully!");

  if (!peripheral.discoverAttributes()) {
    Serial.println("❌ Failed to discover sensor attributes");
    peripheral.disconnect();
    return;
  }
   
  BLEService plantService = peripheral.service(serviceUuid);
  if (!plantService) {
    Serial.println("❌ Plant service not found");
    peripheral.disconnect();
    return;
  }

  moistureCharacteristic = plantService.characteristic(moistureCharUuid);
  commandCharacteristic = plantService.characteristic(commandCharUuid);

  if (!moistureCharacteristic || !commandCharacteristic) {
    Serial.println("❌ Required characteristics not found");
    peripheral.disconnect();
    return;
  }

  if (moistureCharacteristic.canSubscribe()) {
    if(!moistureCharacteristic.subscribe()){
      Serial.println("❌ Failed to subscribe to moisture updates");
      peripheral.disconnect();
      return;
    }
    Serial.println("✓ Subscribed to moisture updates");
    Serial.println("📊 Starting continuous monitoring...\n");
    delay(1000);
  } else {
    Serial.println("❌ Cannot subscribe to moisture characteristic");
    peripheral.disconnect();
    return;
  }

  unsigned long lastDataLog = 0;
  const long dataLogInterval = 10000; // Log data every 10 seconds

  while (peripheral.connected()) {
    BLE.poll();
     
    if (moistureCharacteristic.valueUpdated()) {
       uint32_t moistureValue;
       moistureCharacteristic.readValue(moistureValue);
       lastMoisture = moistureValue;
       
       logDataPoint(lastMoisture, isPumping);
       pumpControlLogic();
       updateDisplay("🌱 Connected", 
                    isPumping ? "💧 Pump: ON" : "💧 Pump: OFF", 
                    "Moisture: " + String(lastMoisture) + "%");
    }
     
    if (millis() - lastDataLog > dataLogInterval) {
      lastDataLog = millis();
      if (lastMoisture != -1) {
        printDataLog();
      }
    }

    // Pump safety timeout
    if(isPumping && (millis() - pumpStartTime > (PUMP_TIMEOUT_SECONDS * 1000))) {
        Serial.println("⚠️  SAFETY: Pump timeout reached, stopping pump");
        setPumpState(false);
    }
  }
  
  Serial.println("\n📡 Sensor disconnected, resuming scan...");
  updateDisplay("🔍 Scanning", "Sensor lost", "Reconnecting...");
}

void pumpControlLogic() {
  if (!isPumping && lastMoisture < MOISTURE_LOW_THRESHOLD) {
    Serial.println("🚨 LOW MOISTURE DETECTED!");
    Serial.println("💧 Starting automatic watering...");
    Serial.println("   Moisture: " + String(lastMoisture) + "% (threshold: " + String(MOISTURE_LOW_THRESHOLD) + "%)");
    setPumpState(true);
  } else if (isPumping && lastMoisture > MOISTURE_HIGH_THRESHOLD) {
    Serial.println("✅ ADEQUATE MOISTURE REACHED!");
    Serial.println("💧 Stopping watering system...");
    Serial.println("   Moisture: " + String(lastMoisture) + "% (threshold: " + String(MOISTURE_HIGH_THRESHOLD) + "%)");
    setPumpState(false);
  }
}

void setPumpState(bool turnOn) {
  if (turnOn) {
    isPumping = true;
    digitalWrite(relayPin, HIGH);
    pumpStartTime = millis();
    commandCharacteristic.writeValue((uint8_t)'C'); // Set sensor to continuous mode
    totalWateringCycles++;
    Serial.println("   ⚡ Pump relay: ON");
    Serial.println("   📡 Sensor mode: Continuous");
    Serial.println("   🔢 Watering cycle #" + String(totalWateringCycles));
  } else {
    isPumping = false;
    digitalWrite(relayPin, LOW);
    commandCharacteristic.writeValue((uint8_t)'N'); // Set sensor to normal mode
    
    if (pumpStartTime > 0) {
      unsigned long runtime = (millis() - pumpStartTime) / 1000;
      Serial.println("   ⚡ Pump relay: OFF");
      Serial.println("   📡 Sensor mode: Normal");
      Serial.println("   ⏱️  Pump runtime: " + String(runtime) + " seconds");
    }
  }
}

void logDataPoint(int moisture, bool pumping) {
  // Store in circular buffer
  moistureHistory[historyIndex] = moisture;
  pumpHistory[historyIndex] = pumping;
  timeHistory[historyIndex] = millis();
  
  historyIndex = (historyIndex + 1) % 10;
}

void printDataLog() {
  static int logCounter = 0;
  logCounter++;
  
  unsigned long uptime = (millis() - systemStartTime) / 1000;
  
  Serial.println("📊 DATA LOG #" + String(logCounter) + " [Uptime: " + String(uptime) + "s]");
  Serial.println("   💧 Moisture: " + String(lastMoisture) + "%");
  Serial.println("   🚰 Pump: " + String(isPumping ? "RUNNING" : "STOPPED"));
  Serial.println("   🔄 Total watering cycles: " + String(totalWateringCycles));
  
  // Calculate averages from last 10 readings
  int avgMoisture = 0;
  int validReadings = 0;
  for (int i = 0; i < 10; i++) {
    if (moistureHistory[i] > 0) {
      avgMoisture += moistureHistory[i];
      validReadings++;
    }
  }
  if (validReadings > 0) {
    avgMoisture /= validReadings;
    Serial.println("   📈 10-reading average: " + String(avgMoisture) + "%");
  }
  Serial.println();
}

void printSystemStatus() {
  Serial.println("🖥️  SYSTEM STATUS REPORT");
  Serial.println("========================");
  
  unsigned long uptime = (millis() - systemStartTime) / 1000;
  int hours = uptime / 3600;
  int minutes = (uptime % 3600) / 60;
  int seconds = uptime % 60;
  
  Serial.println("⏰ System uptime: " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s");
  Serial.println("📡 Operation mode: Standalone (no WiFi)");
  Serial.println("💧 Current moisture: " + String(lastMoisture) + "%");
  Serial.println("🚰 Pump status: " + String(isPumping ? "RUNNING" : "STOPPED"));
  Serial.println("🔄 Total watering cycles: " + String(totalWateringCycles));
  
  // Print recent history
  Serial.println("📜 Recent history (last 5 readings):");
  for (int i = 0; i < 5; i++) {
    int idx = (historyIndex - 1 - i + 10) % 10;
    if (moistureHistory[idx] > 0) {
      unsigned long timeAgo = (millis() - timeHistory[idx]) / 1000;
      Serial.println("   " + String(timeAgo) + "s ago: " + String(moistureHistory[idx]) + "% " + 
                    (pumpHistory[idx] ? "💧" : "⏸️"));
    }
  }
  Serial.println();
}

void startScan() {
  updateDisplay("🔍 Scanning", "Looking for", "plant sensor...");
  BLE.scanForUuid(serviceUuid);
}

void updateDisplay(const String& line1, const String& line2, const String& line3) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 16);
  display.println(line2);
  display.setCursor(0, 32);
  display.println(line3);
  
  // Add status indicators
  display.setCursor(0, 48);
  unsigned long uptime = (millis() - systemStartTime) / 60000; // minutes
  display.print("Up:" + String(uptime) + "m Cycles:" + String(totalWateringCycles));
  
  display.display();
}

void printSystemInfo() {
  Serial.println("⚙️  SYSTEM CONFIGURATION");
  Serial.println("========================");
  Serial.println("💧 Pump start threshold: < " + String(MOISTURE_LOW_THRESHOLD) + "%");
  Serial.println("💧 Pump stop threshold: > " + String(MOISTURE_HIGH_THRESHOLD) + "%");
  Serial.println("⏱️  Max pump runtime: " + String(PUMP_TIMEOUT_SECONDS) + " seconds");
  Serial.println("📊 Data logging interval: 10 seconds");
  Serial.println("📈 History buffer: 10 readings");
  Serial.println("🌐 Network mode: Standalone (no WiFi)");
  Serial.println("========================\n");
}
