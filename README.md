# Energy Theft and Meter Bypass Detection System

## Project Overview
This project addresses the critical challenge of non-technical losses in power distribution networks, specifically focusing on illegal meter bypassing and energy theft. Using a combination of **IoT-based real-time monitoring** and **predictive modeling**, the system distinguishes between legitimate load fluctuations and fraudulent activity.

## Key Features
* **Real-time Detection:** Identifies discrepancies between the energy sent from the distribution point and the energy recorded by individual smart meters.
* **Simulation-Driven Insights:** Modeled using **MATLAB/Simulink** to simulate various theft scenarios and grid stability impacts.
* **IoT Integration:** Utilizes the **ThingSpeak API** for cloud-based data logging and real-time alerts.
* **Data Analytics:** Python-based processing of current and voltage signatures to identify tampering patterns.

## Technical Stack
* **Simulation:** MATLAB/Simulink (Simscape for physical power modeling).
* **Hardware Design:** Proteus/KiCad for PCB layout and circuit simulation.
* **IoT:** ESP32/Arduino IDE and ThingSpeak.

## Simulation Results
One of the core objectives of this project was generating high-resolution figures to explain system behavior during a bypass event.

| Scenario | Observation | Result |
| :--- | :--- | :--- |
| **Normal Load** | Input Power $\approx$ Output Power | No Alert |
| **Meter Bypass** | Output Power $>$ Recorded Power | **Theft Alert Triggered** |

## How to Use
1. **Simulink Model:** Open `Energy_Theft_Model.slx` in MATLAB to view the physical system simulation.
2. **Data Analysis:** Run `detection_logic.py` to see the Python implementation of the theft detection algorithm.
3. **Hardware:** The `.pdsprj` file contains the circuit schematic for the physical prototype.

---

## About the Author
**Muhammad-Tariq Abdulraheem**
* **Chair**, IEEE Student Branch, FUT Minna
* **B.Eng. Mechatronics Engineering** 
* **Junior Research Assistant** at AEIRG, FUT Minna
