# Software Architecture & Control Logic

This document details the firmware control loops, protective algorithms, state flow, and academic case studies comparing rule-based switching against MPPT-based control.

---

## ⚙️ Control Modes

The ESP32 firmware executes a high-frequency polling loop (200ms intervals) that monitors input source parameters and handles source switching. The toggle switch (GPIO 32) directs the controller to execute one of two operational modes:

### 1. Automatic Priority Mode (Default)
In Auto Mode, the controller reads the bus voltage of each input source via the I2C sensors and assigns power drawing priority. Renewable sources are prioritized to minimize non-renewable fuel utilization:

```
                  +--------------------+
                  | Read Source Volts  |
                  +--------------------+
                            |
                            v
                 /----------------------\
                |   Is Solar > 5.0V?     |
                 \----------------------/
                   /                  \
             YES  /                    \ NO
                 v                      v
        +-------------------+  /----------------------\
        | Active = SOLAR    | |    Is Wind > 5.0V?     |
        +-------------------+  \----------------------/
                                 /                  \
                           YES  /                    \ NO
                               v                      v
                      +------------------+  /----------------------\
                      | Active = WIND    | |    Is Fuel > 5.0V?     |
                      +------------------+  \----------------------/
                                              /                  \
                                        YES  /                    \ NO
                                            v                      v
                                   +------------------+  +------------------+
                                   | Active = FUEL    |  | Active = OFF     |
                                   +------------------+  +------------------+
```

### 2. Manual Override Mode
In Manual Mode, the automated source validation logic is bypassed. The ESP32 polls three dedicated tactical push buttons:
* **Button 1 (GPIO 33)** -> Forces switching to **Solar**.
* **Button 2 (GPIO 23)** -> Forces switching to **Wind**.
* **Button 3 (GPIO 5)**  -> Forces switching to **Fuel**.
* *Note: If no buttons are pressed or a conflict occurs, the controller defaults to turning the relays OFF (Active High output state) to protect the load.*

---

## 🛡️ Software Protection Algorithms

### 1. Break-Before-Make Switching
When transitioning from an active source to a new source, there is a risk of contact overlap in electromagnetic relays. If two relays are closed simultaneously, current can back-feed from a higher voltage source into a lower voltage source.
To prevent this, the firmware enforces a physical **Break-Before-Make delay**:
1. Disable current relay outputs (Write `HIGH` to turn OFF).
2. Execute a small, non-blocking delay (typically 20ms - 50ms) allowing the mechanical relay contacts to fully isolate.
3. Enable the target source relay (Write `LOW` to turn ON).

### 2. Anti-Chatter (Hysteresis & Sampling Delay)
When a source voltage sits right at the $5.0\text{V}$ threshold (e.g., during sunrise/sunset or intermittent wind gusts), the source state might fluctuate between available and unavailable. This causes the relays to cycle rapidly—a behavior known as **relay chatter**, which leads to contact wear and load instability.
* **Hysteresis**: The software uses a voltage threshold gap. A source must rise above $5.0\text{V}$ to be marked available, but must fall below $4.6\text{V}$ to be deactivated.
* **Software Filtering**: The loop delay is set to $200\text{ms}$ to buffer immediate transients.

---

## 📊 Case Study: MPC vs. MPPT Configurations

An academic case study evaluated the performance of three different control designs:
1. **Rule-Based Switching System (MPC)**: Priority selection with zero active battery management.
2. **MPPT-Based Power System**: Focuses on maximum power point tracking from a single renewable source.
3. **Enhanced Rule-Based System with Battery Storage (BESS)**: Switches inputs intelligently while utilizing a battery buffer to absorb transients.

### Comparative Evaluation Matrix

| Feature Parameter | Standard Rule-Based | MPPT-Based | Enhanced BESS (This Project) |
| :--- | :--- | :--- | :--- |
| **Primary Focus** | Source logic / priority | Power maximization | Combined logic + energy buffer |
| **Continuous Supply** | Low (drops on source shift) | Medium (source-dependent)| **High (seamless battery backup)** |
| **Dynamic Response** | Slow ($\approx 100\text{ms}$) | Medium | **Fast ($< 10\text{ms}$ transient)** |
| **Relay Chatter** | Occurs without filter | N/A (no relay) | **Eliminated via hysteresis** |
| **Circuit Complexity** | Simple | Very High | **Balanced / Modular** |
| **System Efficiency** | $\approx 82\%$ | $\approx 92\%$ (peak source) | **$\approx 88\%$ (average load delivery)** |

---

## 📈 Experimental Validation

During hardware trials:
* **Solar PV Voltage Profile**: Showed stable operation above $12\text{V}$ under light load, dropping when current demand exceeded module rating.
* **Wind Turbine Profile**: Depicted voltage spikes corresponding to simulated wind gusts, which were normalized by the boost regulator.
* **Fuel Cell Transition**: When both solar and wind drops were simulated, the ESP32 detected the drop and successfully switched to the Fuel Cell backup within $45\text{ms}$, ensuring the load did not reset.
