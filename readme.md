# 🌱 IoT Plant Monitoring System - Student Lab Guide

## Learning Objectives

By completing this lab, you will:
- **Understand IoT fundamentals**: sensors, actuators, wireless communication, and control systems
- **Build a complete IoT device** from hardware assembly to software deployment
- **Learn wireless communication** using Bluetooth Low Energy (BLE)
- **Implement automated control logic** based on sensor feedback
- **Practice real-world troubleshooting** and system debugging
- **Explore data logging and monitoring** techniques

---

## 🎯 Project Overview

### What You're Building
A **smart plant watering system** that:
- **Monitors soil moisture** wirelessly using a capacitive sensor
- **Automatically waters plants** when soil becomes too dry
- **Displays real-time status** on an OLED screen
- **Logs all data** for analysis and learning
- **Operates independently** without constant computer connection

### Real-World Applications
This project demonstrates concepts used in:
- **Smart agriculture** and precision farming
- **Home automation** systems
- **Environmental monitoring** networks
- **Industrial IoT** applications
- **Greenhouse management** systems

---

## 🧩 System Architecture

```
[Sensor Unit]          [Control Unit]           [Actuator]
Nano ESP32       BLE    MKR WiFi 1010     →      12V Pump
├─ Moisture     ←→      ├─ WiFi Module           ├─ Water Flow
├─ OLED Display         ├─ Relay Control         └─ Plant Watering
├─ Button               ├─ OLED Display
└─ BLE Radio            └─ Data Processing
```

### Communication Flow
1. **Sensor reads** soil moisture every 5 minutes
2. **BLE transmits** data to control unit
3. **Control unit analyzes** moisture level
4. **Pump activates** if moisture < 30%
5. **System logs** all events and data
6. **Display updates** show current status

---

## 🔧 Hardware Components

### 🛒 **Complete Shopping List**

**➤ [Project Parts List *Amazon* - Complete Kit](https://github.com/MikeMester/MoistureSensor/blob/main/parts.md)**  
*All required components in one convenient list for easy ordering*

### Sensor Unit (Nano ESP32)
| Component | Purpose | Interface | Approx Cost |
|-----------|---------|-----------|-------------|
| **Arduino Nano ESP32** | Main controller with BLE | USB Programming | $25 |
| **Capacitive Moisture Sensor** | Measures soil moisture | Analog (A0) | $8 |
| **0.96" OLED Display** | Shows sensor status | I2C (SDA/SCL) | $6 |
| **Push Button** | Manual readings | Digital (D2) | $2 |
| **Breadboard & Wires** | Connections | Various | $8 |

### Control Unit (MKR WiFi 1010)
| Component | Purpose | Interface | Approx Cost |
|-----------|---------|-----------|-------------|
| **Arduino MKR WiFi 1010** | Main controller with WiFi/BLE | USB Programming | $45 |
| **MKR Relay Proto Shield** | Controls high-power devices | Stacked on MKR | $25 |
| **0.96" OLED Display** | Shows system status | I2C (SDA/SCL) | $6 |
| **12V Peristaltic Pump** | Water delivery | Relay-controlled | $18 |
| **12V Power Adapter** | Pump power supply | AC/DC adapter | $12 |
| **JST Connector Cables** | Pump connections (18 AWG) | Screw terminals | $5 |

### 🎓 **For Instructors: Kit Preparation**

#### Bulk Ordering Recommendations:
- **Order 2-3 weeks ahead** of lab schedule
- **Test one complete kit** before ordering multiples  
- **Consider 10% extra components** for backup/replacements
- **Group orders** for better pricing (25+ kits get educational discounts)

#### Pre-Lab Setup:
1. **Verify all components** against wishlist
2. **Test pump/relay operation** with one kit
3. **Prepare connection diagrams** for students
4. **Have multimeter available** for troubleshooting
5. **Pre-load Arduino libraries** on lab computers

#### Student Pairing:
- **2 students per kit** works well for collaboration
- **One focuses on sensor unit**, other on control unit
- **Swap roles** for second build or troubleshooting
- **Share final system** for testing and demonstration

### **Total Kit Cost: ~$160 per student pair**

### Why These Components?
- **Capacitive sensor**: Doesn't corrode like resistive sensors
- **BLE communication**: Low power, good range for IoT
- **Peristaltic pump**: Precise, food-safe water delivery
- **OLED displays**: Clear status indication
- **WiFi capability**: Future cloud connectivity
- **18 AWG wiring**: Safe current capacity for 12V pump circuit

---

## 💻 Software Architecture

### Key Programming Concepts

#### 1. **State Machines**
```cpp
enum SensorMode { NORMAL, CONTINUOUS };
enum PumpState { IDLE, PUMPING, TIMEOUT };
```
- **Why**: Manages complex system behavior
- **Learning**: Essential for robust embedded systems

#### 2. **Non-blocking Code**
```cpp
if (millis() - lastReading > interval) {
    // Take reading without stopping other tasks
}
```
- **Why**: Allows multiple operations simultaneously
- **Learning**: Critical for responsive IoT devices

#### 3. **Sensor Calibration**
```cpp
int moisturePercent = map(sensorValue, dryValue, wetValue, 0, 100);
```
- **Why**: Converts raw readings to meaningful data
- **Learning**: Essential for accurate measurements

#### 5. **Control Logic**
```cpp
if (!isPumping && moisture < LOW_THRESHOLD) {
    startPump();
}
```
- **Why**: Automates system response to conditions
- **Learning**: Core of intelligent IoT systems

---

## 🔨 Assembly Instructions

### Part 1: Sensor Unit Assembly

#### Step 1: Prepare Breadboard
1. Insert **Nano ESP32** into breadboard
2. Verify **power connections** (3.3V, GND)
3. Test with **simple blink sketch**

#### Step 2: Connect Moisture Sensor
```
Sensor Pin → Nano ESP32 Pin
VCC       → 3V3
GND       → GND
AOUT      → A0
```

#### Step 3: Connect OLED Display
```
OLED Pin → Nano ESP32 Pin
VCC      → 3V3
GND      → GND
SDA      → A4
SCL      → A5
```

#### Step 4: Add Push Button
```
Button → Nano ESP32
Leg 1  → D2
Leg 2  → GND (with internal pullup)
```

### Part 2: Control Unit Assembly

#### Step 1: Stack Relay Shield
1. **Carefully align** MKR Relay Proto Shield with MKR WiFi 1010
2. **Press firmly** to ensure good connection
3. **Verify alignment** of all pins

#### Step 2: Connect OLED Display
```
OLED Pin → MKR Pin
VCC      → 5V
GND      → GND
SDA      → SDA (Pin 11)
SCL      → SCL (Pin 12)
```

#### Step 3: Wire Pump Circuit
```
Power Supply Connections:
12V Adapter (+) → COM1 (relay terminal)
Pump (+)        → NO1 (relay terminal) 
Pump (-)        → 12V Adapter (-) (direct connection)
```

**🔌 Wire Specifications:**
- **Wire Gauge**: 18 AWG (recommended for 12V pump circuit)
- **Current Rating**: 2-3A continuous (pump uses ~1A)
- **Connection Type**: JST connectors to screw terminals
- **Wire Colors**: Red (+), Black (-) for safety

**⚠️ Safety Notes**: 
- Always disconnect power when wiring!
- Verify polarity before connecting
- Use pre-made cables from the Amazon wishlist to avoid cutting/stripping
- Test connections with multimeter if available

**🔧 Connection Sequence:**
1. **First**: Connect 12V adapter (+) to COM1 terminal
2. **Second**: Connect pump (+) to NO1 terminal using JST adapter cable
3. **Third**: Connect pump (-) directly to 12V adapter (-) 
4. **Finally**: Verify all screw terminals are tight

---

## 📝 Programming Steps

### Step 1: Install Required Libraries
```
Tools → Manage Libraries → Install:
- ArduinoBLE
- Adafruit GFX Library
- Adafruit SSD1306
```

### Step 2: Configure Board Settings
- **Sensor Unit**: Arduino Nano ESP32
- **Control Unit**: Arduino MKR WiFi 1010
- **Port**: Select correct COM port
- **Programmer**: Default

### Step 3: Upload Sensor Code
1. **Open** sensor unit code
2. **Select** Nano ESP32 board
3. **Upload** and verify operation
4. **Check** Serial Monitor for debug output

### Step 2: Upload Control Code
1. **Open** control unit code
2. **Save file** (Ctrl+S on each tab)
3. **Select** MKR WiFi 1010 board
4. **Upload** and verify BLE connection

**📝 Note**: Use `"quotes"` for local files, `<brackets>` for libraries!

---

## 🧪 Testing Procedures

### Phase 1: Individual Component Testing

#### Test Moisture Sensor
1. **Insert sensor** in dry soil → expect low reading (0-30%)
2. **Dip in water** → expect high reading (70-100%)
3. **Check Serial output** for consistent values

#### Test BLE Communication
1. **Power both units**
2. **Watch for "Connected" on displays**
3. **Verify moisture data** appears on control unit
4. **Check update frequency** (every 5 minutes normal, 5 seconds when pumping)

#### Test Pump Control
1. **Force low moisture** condition (modify threshold or sensor)
2. **Verify pump starts** automatically
3. **Check pump stops** when moisture rises
4. **Test safety timeout** (5-minute maximum runtime)

### Phase 2: System Integration Testing

#### 24-Hour Stress Test
1. **Monitor system** continuously for 24 hours
2. **Log all events** and moisture readings
3. **Verify reliable operation** without human intervention
4. **Check for memory leaks** or system crashes

#### Real Plant Testing
1. **Install in actual plant** setup
2. **Monitor plant health** over several days
3. **Adjust thresholds** based on plant needs
4. **Document watering cycles** and plant response

---

## 📊 Data Analysis & Learning

### Understanding the Data

#### Moisture Readings
- **0-30%**: Dry soil, pump should activate
- **30-60%**: Adequate moisture, no action needed  
- **60-100%**: Wet soil, pump should not run

#### System Behavior
- **Normal mode**: Readings every 5 minutes
- **Continuous mode**: Readings every 5 seconds (during pumping)
- **Pump cycles**: Automatic start/stop based on thresholds

### Key Metrics to Track
1. **Average moisture levels** throughout day
2. **Pump runtime** per activation cycle
3. **Time between waterings** 
4. **System uptime** and reliability
5. **BLE connection stability**

### Questions to Explore
- How does **room temperature** affect moisture readings?
- What's the **optimal watering schedule** for different plants?
- How does **sensor placement** impact accuracy?
- What **environmental factors** influence system performance?

---

## 🔧 Troubleshooting Guide

### Common Issues & Solutions

#### "No BLE Connection"
**Symptoms**: Displays show "Scanning..." indefinitely
**Solutions**:
1. Check both units are powered and running
2. Verify UUIDs match in both code files
3. Ensure units are within 10 meters
4. Reset both units and try again

#### "Pump Won't Start"
**Symptoms**: High moisture but pump never activates
**Solutions**:
1. Check current moisture reading (pump only starts < 30%)
2. Verify relay wiring to Pin 1
3. Test 12V power supply
4. Use pump test code to isolate issue

#### "Moisture Readings Inconsistent"
**Symptoms**: Values jumping erratically
**Solutions**:
1. Ensure sensor is properly inserted in soil
2. Check for loose wiring connections
3. Calibrate sensor with known dry/wet conditions
4. Consider sensor placement and soil type

#### "System Crashes/Resets"
**Symptoms**: Units restart unexpectedly
**Solutions**:
1. Check power supply stability
2. Verify sufficient current capacity
3. Look for memory leaks in code
4. Add delays to prevent overloading

---

## 🚀 Extensions & Challenges

### Beginner Extensions
1. **Add LED indicators** for system status
2. **Implement buzzer alerts** for low moisture
3. **Create pump run-time logging** 
4. **Add manual override button**

### Intermediate Challenges  
1. **WiFi data logging** to cloud service
2. **Multiple sensor support** (temperature, humidity)
3. **Mobile app interface** for remote monitoring
4. **Solar power system** for outdoor use

### Advanced Projects
1. **Machine learning** for predictive watering
2. **Weather API integration** for rain detection
3. **Multi-plant monitoring network**
4. **Professional enclosure design**

### Research Questions
- How does **plant type** affect optimal moisture thresholds?
- What's the relationship between **watering frequency** and plant growth?
- How can **sensor fusion** improve accuracy?
- What **machine learning models** work best for plant care?

---

## 📚 Learning Reflection

### Key Concepts Mastered
After completing this lab, you should understand:

#### IoT Architecture
- **Sensor-to-cloud** data pipeline
- **Edge computing** vs cloud processing
- **Device communication** protocols
- **System reliability** and fault tolerance

#### Embedded Programming
- **Real-time constraints** and timing
- **Resource management** (memory, power)
- **Hardware abstraction** layers
- **Debugging** embedded systems

#### Control Systems
- **Feedback loops** and automation
- **Threshold-based** decision making
- **Safety systems** and fail-safes
- **System optimization** and tuning

#### Professional Skills
- **Project documentation** and reporting
- **Systematic troubleshooting** methodology
- **Version control** and code management
- **Collaboration** and peer review

### Real-World Connections
This project demonstrates principles used in:
- **Smart city** infrastructure
- **Industrial automation** systems
- **Environmental monitoring** networks
- **Agricultural technology** solutions
- **Home automation** platforms

---

## 📋 Project Deliverables

### Required Documentation
1. **System diagram** showing all connections
2. **Test results** for each phase
3. **24-hour data log** with analysis
4. **Troubleshooting report** of issues encountered
5. **Reflection essay** on lessons learned

### Code Requirements
1. **Well-commented** source code for both units
2. **Version control** using Git
3. **Test cases** for major functions
4. **User manual** for operation

### Demonstration
1. **Live system** showing automatic operation
2. **Explanation** of design decisions
3. **Troubleshooting** demonstration
4. **Extension** or improvement implemented

---

## ✅ Assessment Criteria

### Technical Implementation (40%)
- **Hardware assembly** correct and secure
- **Software functionality** meets all requirements
- **System reliability** operates without intervention
- **Data accuracy** sensors provide meaningful readings

### Problem Solving (30%)
- **Troubleshooting approach** systematic and logical
- **Documentation quality** clear and comprehensive
- **Testing methodology** thorough and appropriate
- **Solution optimization** improvements beyond basic requirements

### Understanding (20%)
- **Concept explanation** demonstrates deep understanding
- **Design rationale** justifies component and approach choices
- **Real-world connections** relates project to broader applications
- **Learning reflection** identifies key insights and growth

### Professional Skills (10%)
- **Code quality** well-structured and commented
- **Documentation** professional and complete
- **Presentation** clear and engaging
- **Collaboration** effective teamwork and communication

---

---

*"The best way to learn IoT is to build real systems that solve real problems."*

**Congratulations on building your first complete IoT system!** 🎉

---

## 📁 Appendix A: Sensor Unit Code (Nano ESP32)


```

### Key Features:
- **BLE communication** with unique UUIDs
- **Automatic moisture readings** every 5 minutes (normal) or 5 seconds (continuous)
- **Manual button readings** for testing
- **OLED status display** showing connection and moisture
- **Calibrated sensor values** for accurate percentage readings

---
```
## 📁 Appendix B: Control Unit Code (MKR WiFi 1010)

### Key Features:
- **Complete standalone operation** - no cloud dependencies
- **Automatic pump control** based on moisture thresholds  
- **Professional data logging** with historical analysis
- **Safety features** including pump timeout protection
- **Real-time status display** on OLED and Serial Monitor
- **WiFi connectivity** for future expansion (optional)

### Before Uploading:
1. **Update WiFi credentials** in lines 15-16 (or leave as-is for offline operation)
2. **Verify UUIDs match** the sensor unit code exactly
3. **Check pin assignments** match your hardware setup
4. **Install all required libraries** listed in the main guide

---

## 🔧 Quick Setup Instructions

### For Sensor Unit (Nano ESP32):
1. **Select Board**: Arduino Nano ESP32
2. **Upload Code**: Copy Plant_Sensor code exactly
3. **Monitor**: Open Serial Monitor at 115200 baud
4. **Verify**: Should show "Advertising..." on OLED

### For Control Unit (MKR WiFi 1010): 
1. **Select Board**: Arduino MKR WiFi 1010
2. **Update WiFi**: Edit ssid/pass in code (optional)
3. **Upload Code**: Copy Plant_Central code exactly  
4. **Monitor**: Open Serial Monitor at 115200 baud
5. **Verify**: Should show system startup messages

### Testing Communication:
1. **Power both units**
2. **Watch for "Connected!" on sensor OLED**
3. **Watch for "Connected to plant sensor successfully!" in control unit Serial Monitor**
4. **Verify moisture readings** appear every 5 minutes

---
