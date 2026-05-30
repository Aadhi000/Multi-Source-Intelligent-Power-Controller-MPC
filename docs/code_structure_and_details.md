# Code Structure & Firmware Walkthrough

This document provides a technical walkthrough of the firmware logic running on the ESP32 microcontroller (`src/main.cpp`).

---

## 📦 Library Dependencies

The firmware uses the following Arduino core and Adafruit library packages:
1. **`Wire.h`**: Manages I2C master operations on GPIO 21 (SDA) and GPIO 22 (SCL).
2. **`Adafruit_INA219.h`**: Driver for high-side current and voltage monitors. Handles bus voltage, shunt voltage, current, and power calculation registers.
3. **`Adafruit_SSD1306.h` & `Adafruit_GFX.h`**: Manages the SSD1306 0.91" OLED display layout, graphics drawing, and text rendering.

---

## 🏗️ Code Components Walkthrough

### 1. Pin Definitions & Objects
Lines 7–30 map the ESP32 physical pins to logical controls:
* **Relays**:
  ```cpp
  #define RELAY_SOLAR 26
  #define RELAY_WIND  25
  #define RELAY_FUEL  27
  ```
* **User Buttons**:
  ```cpp
  #define BUTTON_SOLAR 33
  #define BUTTON_WIND  23
  #define BUTTON_FUEL  5
  ```
* **I2C Sensor Addressing**:
  ```cpp
  Adafruit_INA219 inaSolar(0x41);
  Adafruit_INA219 inaWind(0x44);
  Adafruit_INA219 inaFuel(0x40);
  ```

### 2. Global Struct: `SourceData`
To keep code clean and modular, a struct stores the voltage, current, power, and availability status of each source:
```cpp
struct SourceData {
  float voltage;
  float current;
  float power;
  bool available;
};
SourceData solar, wind, fuel;
```

---

## 🔄 Core Firmware Functions

### 1. `setup()`
The initialization phase performs the following tasks:
* Configures pin directions (`INPUT_PULLUP` for switches/buttons, `OUTPUT` for relays and status LEDs).
* Ensures all relays are initially turned **OFF** to avoid power surge on booting (`digitalWrite(Pin, HIGH)` since they are active-low).
* Blinks the onboard blue Start LED (GPIO 2) as a status indicator.
* Starts I2C communications (`Wire.begin(21, 22)`).
* Initializes the SSD1306 display and prints `"MPC SYSTEM STARTING"`.
* Connects to the three INA219 chips.

### 2. `loop()`
Executes continuously, coordinating data reading and switching control:
```cpp
void loop() {
  readSensors();           // 1. Fetch current/voltage from INA219s
  updateControlLogic();    // 2. Process Auto/Manual switching states
  updateLEDs();            // 3. Update active source status indicator LEDs
  updateDisplay();         // 4. Draw metrics on SSD1306 OLED Screen
  
  delay(200);              // 5. 200ms loop delay (5Hz polling frequency)
}
```

### 3. `readSensors()`
Queries each sensor over I2C and calculates power data:
```cpp
void readSensors() {
  // Read Solar Values
  solar.voltage = inaSolar.getBusVoltage_V();
  solar.current = inaSolar.getCurrent_mA() / 1000.0f; // Convert mA to A
  solar.power = inaSolar.getPower_mW() / 1000.0f;     // Convert mW to W
  solar.available = (solar.voltage > VOLTAGE_THRESHOLD); // Check if > 5.0V

  // Wind and Fuel readings follow the exact same logic structure...
}
```

### 4. `updateControlLogic()`
Checks the mode switch pin (GPIO 32) and updates the active source selection:
* **Auto Mode (GPIO 32 reads High)**: Evaluates availability in sequence: Solar first, then Wind, then Fuel. If none are above the threshold voltage ($5.0\text{V}$), `activeSource` is set to `0` (OFF).
* **Manual Mode (GPIO 32 reads Low)**: Ignores thresholds. Polls the buttons and updates `activeSource` according to manual button presses.

### 5. `setRelays(int source)`
Triggers the physical switching network based on `activeSource` using active-low writes. This contains built-in physical separation since it writes `HIGH` (OFF) to inactive relays first before pulling the active relay `LOW` (ON):
```cpp
void setRelays(int source) {
  digitalWrite(RELAY_SOLAR, (source == 1) ? LOW : HIGH);
  digitalWrite(RELAY_WIND,  (source == 2) ? LOW : HIGH);
  digitalWrite(RELAY_FUEL,  (source == 3) ? LOW : HIGH);
}
```

### 6. `updateDisplay()`
Calculates which parameters to render and drives the 0.91" screen.
* Line 1 displays: `MODE: AUTO` or `MODE: MANUAL`.
* Line 2 displays: `SRC: SOLAR / WIND / FUEL / OFF`.
* Line 3 shows active readings: `[Voltage]V [Current]A [Power]W`.
* Calls `display.display()` to push the framebuffer to the screen over the I2C lines.
