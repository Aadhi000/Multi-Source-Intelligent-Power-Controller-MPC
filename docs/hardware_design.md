# Hardware Design & Circuit blue-prints

This document details the physical hardware, pin configurations, component specifications, and circuit protection mechanisms implemented in the Multi-Source Intelligent Power Controller (MPC).

---

## 🛠️ Bill of Materials (BOM) & Specifications

### 1. ESP32 Dev Board (30-Pin NodeMCU ESP-WROOM-32)
* **Processor**: Tensilica Xtensa Dual-core 32-bit LX6 running at 240 MHz.
* **Interfaces**: Dedicated I2C bus (GPIO 21 SDA, GPIO 22 SCL) configured with internal pull-ups for multi-device communication.
* **Operating Voltage**: 3.3V logic levels; 5V supply input via VIN.

### 2. INA219 High-Side Current & Power Sensor (x3)
* **Protocol**: I2C bus interface.
* **Shunt Resistor**: $0.1\ \Omega$ standard shunt resistor onboard.
* **Range**: Measures voltages up to 26V and currents up to 3.2A with high accuracy ($\pm 1\%$).
* **I2C Addresses**: Set via address jumpers:
  * **Solar Sensor**: `0x41` (A0 connected to VCC)
  * **Wind Sensor**: `0x44` (A1 connected to VCC)
  * **Fuel Cell/Battery Sensor**: `0x40` (Default, A0 and A1 connected to GND)

### 3. Relay Module (Active-Low SPDT)
* **Type**: 5V Single-Pole Double-Throw (SPDT) electromagnetic relay module.
* **Current Rating**: Supports AC loads up to 10A (250V) or DC loads up to 10A (30V).
* **Control logic**: Driven by low-level trigger signals (Active-Low configuration) to protect the microgrid control lines from default high states.

### 4. Schottky Diodes (SS14 / SS34 / 1N5819)
* **Purpose**: Placed in series with the positive terminals of all input sources.
* **Features**: Very low forward voltage drop ($V_F \approx 0.3\text{V} - 0.4\text{V}$) to minimize thermal losses while providing absolute blocking of reverse leakage currents.

### 5. MP1584 Buck Converter & Boost Converter
* **Buck Converter**: Steps down high source voltage to stable 5V for powering the ESP32 and relay coils.
* **Boost Converter**: Placed on the Wind energy source line to regulate lower wind speeds (8V-10V) up to a stable 12V output.

---

## 📌 Connection & Pinout Table

### I2C Bus Pins
| Device Pin | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| **SDA** | GPIO 21 | Serial Data line shared by 3x INA219 and SSD1306 OLED |
| **SCL** | GPIO 22 | Serial Clock line shared by 3x INA219 and SSD1306 OLED |
| **VCC** | 3.3V | Logic Power for sensors and OLED display |
| **GND** | Common GND | Central Ground tie-point |

### Control & Switch Pins
| ESP32 GPIO | Connected To | Direction | Logic Level |
| :--- | :--- | :--- | :--- |
| **GPIO 26** | Solar Relay Control | Output | Low = Relay ON, High = Relay OFF |
| **GPIO 25** | Wind Relay Control | Output | Low = Relay ON, High = Relay OFF |
| **GPIO 27** | Fuel Relay Control | Output | Low = Relay ON, High = Relay OFF |
| **GPIO 32** | Auto/Manual Select | Input | Pull-Up (GND = Manual, Open/High = Auto) |
| **GPIO 33** | Solar Select Button | Input | Pull-Up (GND when pressed) |
| **GPIO 23** | Wind Select Button | Input | Pull-Up (GND when pressed) |
| **GPIO 5** | Fuel Select Button | Input | Pull-Up (GND when pressed) |

### Status Indicators
| ESP32 GPIO | Color | Purpose |
| :--- | :--- | :--- |
| **GPIO 2** | Blue / White | System Starting / Normal Heartbeat |
| **GPIO 4** | Yellow / Green | Solar Source Active |
| **GPIO 18** | Blue | Wind Source Active |
| **GPIO 19** | Red | Fuel Cell Source Active |

## 📸 Hardware Component Gallery

````carousel
![ESP32 Development Board](../images/ESP32%20Development%20Board.jpg)
<!-- slide -->
![INA219 Current Sensor](../images/INA219%20Current%20Sensor.jpg)
<!-- slide -->
![IRLZ44N MOSFET](../images/IRLZ44N%20MOSFET.jpg)
<!-- slide -->
![0.91-Inch OLED Display](../images/0.91-Inch%20OLED%20Display.jpg)
<!-- slide -->
![MP1584 DC-DC Buck Converter](../images/MP1584%20DC-DC%20Buck%20Converter.jpg)
<!-- slide -->
![Schottky Diode](../images/Schottky%20Diod.jpg)
<!-- slide -->
![DC-DC Boost Converter](../images/DC-DC%20Boost%20Converter.jpg)
<!-- slide -->
![Fuse](../images/Fuse.png)
<!-- slide -->
![Toggle Switch](../images/Toggle%20Switch.jpg)
````

---

## ⚡ Circuit Schematic & Prototype Setup

![Circuit Diagram of the Sytem](../images/Circuit%20Diagram%20of%20the%20Sytem.png)
*Figure 5.10: Complete Circuit Diagram of the Microgrid System*

![Prototype Setup](../images/Prototype%20Setup.jpg)
*Figure 5.11: Integrated Microgrid Hardware Prototype Setup*

> [!CAUTION]
> **Relay Isolation**: Always tie the grounds of the inputs together (`Common GND`) to get accurate voltage readings on the INA219 current sensors. Do NOT bridge the positive terminals of the sources directly. Always keep them physically isolated behind the diodes and the relays.
