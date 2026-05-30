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

## 📈 Simulation Scenarios & Waveforms

The complete microgrid operation was simulated in MATLAB/Simulink.

![Simulink Microgrid Model](../images/Complete%20Integrated%20Simulink%20Model%20of%20the%20Proposed%20Hybrid%20Renewable.png)

*Figure 4.1: Integrated Simulink Model of the Proposed Hybrid Renewable Microgrid*

### 💻 Simulation Waveforms

#### 1. Solar Photovoltaic System Supplying the Load
![Solar PV Supplying Load](../images/Solar%20Photovoltaic%20System%20Supplying%20the%20Load.png)
*(Figure 4.2: Solar Photovoltaic array supplying the microgrid load)*

#### 2. Wind Energy Source Supplying the Load
![Wind System Supplying Load](../images/Wind%20Energy%20Source%20Supplying%20the%20Load.png)
*(Figure 4.3: Wind Energy generator supplying the microgrid load)*

#### 3. Coordinated Solar and Wind Energy Source Operation
![Coordinated Solar and Wind](../images/Coordinated%20Solar%20and%20Wind%20Energy%20Source%20Operation.png)
*(Figure 4.4: Active power balancing during coordinated Solar and Wind supply)*

#### 4. Transition from Renewable Sources to PEM Fuel Cell Operation
![Transition to Fuel Cell](../images/Transition%20from%20Renewable%20Sources%20to%20PEM%20Fuel%20Cell%20Operation.png)
*(Figure 4.5: Transient wave during transition from Solar/Wind to PEMFC backup)*

#### 5. Battery Backup Operation During Fuel Cell Disconnection
![Battery Backup Operation](../images/Battery%20Backup%20Operation%20During%20Fuel%20Cell%20Disconnection.png)
*(Figure 4.6: BESS discharging phase during Fuel Cell isolation)*

---

## ⚡ Experimental Hardware Validation

Below are the actual voltage and current profiles recorded from the hardware sensors under varying conditions.

### 🔬 Experimental Voltage & Current Profiles

#### 1. Solar PV Output Characteristics
![Solar PV Output Characteristics](../images/Solar%20PV%20Output%20Characteristics.jpg)
*(Figure 5.12: Solar PV measured output voltage profile)*

#### 2. Wind Generator Output Voltage Characteristics
![Wind Generator Characteristics](../images/Wind%20Generator%20Output%20Voltage%20Characteristics.jpg)
*(Figure 5.13: Wind generator voltage spikes and regulated output profile)*

#### 3. Fuel Cell Output Voltage Characteristics
![Fuel Cell Backup Performance](../images/Fuel%20Cell%20Output%20Voltage%20Characteristics.jpg)
*(Figure 5.14: PEM Fuel Cell activation and load voltage stability profile)*

#### 4. RULE-Based Hybrid System Voltage Characteristics
![RULE-Based Hybrid Characteristics](../images/RULE-Based%20Hybrid%20System%20Voltage%20Characteristics.png)
*(Figure 6.1: Rule-based hybrid microgrid switching transients)*

#### 5. MPPT-Based Hybrid System Voltage Characteristics
![MPPT-Based Hybrid Characteristics](../images/MPPT-Based%20Hybrid%20System%20Voltage%20Characteristics.png)
*(Figure 6.2: Output voltage using MPPT controller)*

#### 6. Comparative System Voltage Output (Rule-Based vs MPPT-Based)
![Comparative Voltage Output](../images/Comparative%20System%20Voltage%20Output%20Rule-Based%20vs%20MPPT-Based.png)
*(Comparative overlay: Rule-Based vs MPPT-Based microgrid output)*

#### 7. Comparative Performance Evaluation Chart
![Comparison between MPPT-Based and Enhanced Rule-Based Systems](../images/Comparison%20between%20MPPT-Based%20and%20Enhanced%20Rule-Based%20Systems.png)
*(Table 6.1: Tabular comparison evaluation graph)*
