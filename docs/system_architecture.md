# System Architecture & Mathematical Modeling

This document describes the high-level architecture, subsystem configurations, and mathematical representations of the **Multi-Source Intelligent Power Controller (MPC)** and **Hybrid Renewable Microgrid**.

---

## 🏛️ System Overview

The system is designed around a **Common DC Bus Architecture** that integrates four energy sources: Solar PV, Wind, PEM Fuel Cell, and Battery Storage. This centralized bus approach reduces conversion stages, simplifies control logic, and stabilizes power transfer.

![Overall System Architecture](../images/Overall%20System%20Architecture%20of%20the%20Proposed%20Hybrid%20Renewable%20Energy.png)
*Figure 3.1: Overall System Architecture of the Proposed Hybrid Renewable Energy System*

---

## 🔌 Subsystem Descriptions

### 1. Solar Photovoltaic System
The Solar Photovoltaic (PV) array converts solar irradiance directly into DC power. In a full-scale microgrid, modules are configured in series-parallel combinations to match the bus voltage.

### 2. Wind Energy System
The wind energy subsystem utilizes a wind turbine and electrical generator to convert wind's kinetic energy into electrical power. The AC output from the generator is rectified into DC and boosted to match the common DC bus voltage.

### 3. Proton Exchange Membrane Fuel Cell (PEMFC)
The Proton Exchange Membrane Fuel Cell acts as an auxiliary zero-emission power source. It converts the chemical energy of hydrogen and oxygen into electricity, serving as a long-duration backup when both solar and wind resources are depleted.

### 4. Battery Energy Storage System (BESS)
The BESS stores excess energy during peak renewable generation periods. It acts as a bidirectional buffer, discharging to cover short-term dips in renewable output and absorbing excess power to prevent overvoltage.

### 5. Common DC Bus
All energy conversion stages terminate at the DC Bus. The bus acts as the primary power balancing node where input power is shared and distributed:
$$\Sigma P_{\text{sources}} \pm P_{\text{battery}} = P_{\text{load}} + P_{\text{losses}}$$

---

## 📈 Subsystem Mathematical Modeling

### ☀️ Solar PV System Modeling
For an array of photovoltaic modules:
* **Array Output Voltage ($V_{\text{PV}}$)**:
  $$V_{\text{PV}} = N_s \times V_{\text{oc}}$$
  *Where $N_s$ is the number of series-connected modules, and $V_{\text{oc}}$ is the open-circuit voltage of a single module.*
  *For $N_s = 18$ and $V_{\text{oc}} = 36.84\text{ V}$, $V_{\text{PV}} \approx 663\text{ V}$.*
* **Array Output Current ($I_{\text{PV}}$)**:
  $$I_{\text{PV}} = N_p \times I_{\text{sc}}$$
  *Where $N_p$ is the number of parallel strings, and $I_{\text{sc}}$ is the short-circuit current ($8.32\text{ A}$).*
  *For $N_p = 5$, $I_{\text{PV}} \approx 41.6\text{ A}$.*
* **Array Output Power ($P_{\text{PV}}$)**:
  $$P_{\text{PV}} = V_{\text{PV}} \times I_{\text{PV}} \approx 27.59\text{ kW}$$

### 🌀 Wind Energy System Modeling
The mechanical power extracted from the wind by a wind turbine is given by:
$$P_w = \frac{1}{2} \rho A v^3 C_p(\lambda, \beta)$$
where:
* $\rho$ represents the air density ($\approx 1.225\text{ kg/m}^3$).
* $A$ is the turbine swept area ($A = \pi R^2$).
* $v$ is the wind velocity ($\text{m/s}$).
* $C_p$ is the power coefficient (turbine efficiency), which is a function of the tip-speed ratio ($\lambda$) and blade pitch angle ($\beta$).

### 🔋 Battery Energy Storage System (BESS)
The State of Charge ($SoC$) of the battery is modeled as:
$$SoC(t) = SoC(t_0) - \int \frac{\eta \cdot I_{\text{bat}}(t)}{C_b} dt$$
where:
* $I_{\text{bat}}$ is the battery current (positive during discharge, negative during charge).
* $C_b$ is the nominal battery capacity (Ampere-hours).
* $\eta$ is the round-trip coulombic efficiency.

### 🔬 PEM Fuel Cell Polarization Equation
The output stack voltage ($V_{\text{FC}}$) of the PEM Fuel Cell is defined by:
$$V_{\text{FC}} = E_{\text{Nernst}} - V_{\text{act}} - V_{\text{ohmic}} - V_{\text{con}}$$
where:
* $E_{\text{Nernst}}$ is the thermodynamic Nernst potential.
* $V_{\text{act}}$ represents the activation voltage loss (due to slow electrochemical kinetics at the electrodes).
* $V_{\text{ohmic}}$ represents the ohmic voltage loss (due to resistance in the polymer membrane and electrodes).
* $V_{\text{con}}$ represents the concentration voltage loss (due to mass transport limitations of reactants).

---

## 🔄 Power Flow Coordination & Scenarios

The system dynamically shifts between the following operating scenarios to ensure load continuity:

| Scenario | Active Source(s) | Battery Status | Relay / Switch State |
| :--- | :--- | :--- | :--- |
| **1. Solar Dominant** | Solar PV | Charging (excess power) | Solar Relay active |
| **2. Wind Dominant** | Wind Turbine | Charging (excess power) | Wind Relay active |
| **3. Combined Renewables** | Solar + Wind | Charging | Solar/Wind Relays active |
| **4. Battery Support** | Solar/Wind + Battery | Discharging (dips covered) | Battery Switch active |
| **5. Fuel Cell Backup** | PEMFC | Standby / Discharging | Fuel Cell Relay active |
