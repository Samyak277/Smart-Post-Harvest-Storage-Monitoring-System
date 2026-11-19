# Post-Harvest Storage Monitoring & Loss Prevention System (ESP32)

![Platform](https://img.shields.io/badge/Platform-ESP32%20DevKit%20V1-blue)
![Framework](https://img.shields.io/badge/IDE-Arduino%20IDE-00979D)
![Language](https://img.shields.io/badge/Language-C%2B%2B-orange)
![Domain](https://img.shields.io/badge/Domain-AgriTech%20%7C%20Embedded%20Systems-green)

An automated embedded system built with the **ESP32 DevKit V1** to prevent post-harvest spoilage of stored grains, onions, potatoes, and other agricultural produce. The system continuously monitors ambient storage temperature, relative humidity, and internal bulk crop moisture, displays real-time status on a local LCD, and autonomously activates forced ventilation and an audible alarm whenever conditions exceed safe storage thresholds.

---

## Table of Contents

- [Problem Statement](#problem-statement)
- [Key Features](#key-features)
- [Hardware Components](#hardware-components)
- [Pinout & Wiring Guide](#pinout--wiring-guide)
- [Circuit Diagram & Prototype](#circuit-diagram--prototype)
- [How It Works](#how-it-works)
- [Software Setup & Installation](#software-setup--installation)
- [Calibration & Threshold Customization](#calibration--threshold-customization)
- [Recommended Repository Structure](#recommended-repository-structure)
- [Future Scope](#future-scope)
- [License](#license)

---

## Problem Statement

A significant portion of agricultural produce is lost after harvest due to improper storage conditions. Trapped heat, excess humidity, and hidden internal dampness inside grain sacks or storage bins promote mold growth, bacterial rot, insect infestation, and premature sprouting. Manual periodic inspection often detects these issues too late.

This project provides an affordable, standalone, 24/7 automated climate guardian that detects unfavorable microclimate shifts early and immediately ventilates the storage space.

---

## Key Features

- **Dual-Layer Environmental Sensing:** Tracks ambient air temperature and humidity via a **DHT22** sensor alongside internal produce dampness using a **resistive analog moisture probe**.
- **Real-Time Local Display:** Outputs live temperature, humidity, moisture percentage, and system state (`SAFE` or `FAN ON!`) on a backlit **I2C 16x2 LCD**.
- **Autonomous Ventilation Control:** Automatically triggers a **5V Single-Channel Relay** to power an external **5V/12V DC Exhaust Fan** whenever safety limits are breached.
- **Instant Local Alerting:** Drives a pulsing **Active Buzzer** during high-risk storage conditions until the environment stabilizes.
- **Sensor Fault Detection:** Detects disconnected or failing DHT22 reads and immediately displays `DHT SENSOR ERROR` on the LCD and Serial Monitor.
- **Mixed-Voltage Safe Architecture:** Isolates 3.3V sensors, 5V logic peripherals, and the external DC exhaust fan power supply.

---

## Hardware Components

| # | Component | Operating Voltage | Interface / Pin | Role |
| :-: | :--- | :--- | :--- | :--- |
| 1 | **ESP32 DevKit V1** | 5V via USB / VIN (3.3V logic) | GPIO, I2C, 12-bit ADC | Main microcontroller |
| 2 | **DHT22 (AM2302) Sensor** | 3.3V DC | Single-wire Digital (`GPIO 4`) | Ambient temperature & humidity sensing |
| 3 | **Analog Moisture Sensor + LM393 Module** | 3.3V DC | Analog `A0` (`GPIO 34`) | Bulk crop internal moisture detection |
| 4 | **I2C 16x2 LCD Display** | 5V DC | I2C (`GPIO 21`, `GPIO 22`) | Real-time local telemetry display |
| 5 | **5V Single-Channel Relay Module** | 5V DC coil | Digital (`GPIO 5`) | Electrically isolated switch for exhaust fan |
| 6 | **Active Buzzer** | 3.3V - 5V DC | Digital (`GPIO 18`) | Pulsing local audible alarm |
| 7 | **5V or 12V DC Exhaust Fan** | External 5V / 12V DC | Relay `COM` & `NO` | Active forced ventilation |
| 8 | **Breadboard, Jumper Wires & $10\text{ k}\Omega$ Resistor** | — | Passive | Prototyping & DHT22 pull-up (if using a bare sensor) |

---

## Pinout & Wiring Guide

### 1. Breadboard Power Distribution
- **ESP32 `3V3`** -> Breadboard **3.3V Rail (Side A)** *(Sensors)*
- **ESP32 `VIN`** -> Breadboard **5V Rail (Side B)** *(LCD & Relay)*
- **ESP32 `GND`** -> Breadboard **Common Ground Rail** *(Bridge both negative rails together)*

### 2. Sensor & Peripheral Connections

| Module | Module Pin | Connect To | Notes |
| :--- | :--- | :--- | :--- |
| **DHT22 Sensor** | `VCC` | 3.3V Rail | Add a $10\text{ k}\Omega$ pull-up resistor between `VCC` and `DATA` if using a bare sensor |
| | `GND` | GND Rail | Common ground |
| | `DATA` | `GPIO 4` | Digital sensor output |
| **Moisture Sensor** | `VCC` | 3.3V Rail | **Do not use 5V** (ESP32 ADC limit is 3.3V) |
| *(LM393 Blue Module)* | `GND` | GND Rail | Connect the two probe prongs to the 2-pin side of the blue module (polarity independent) |
| | `A0` | `GPIO 34` | Input-only ADC1 pin |
| | `D0` | *Not Connected* | Unused (software handles thresholds) |
| **I2C 16x2 LCD** | `VCC` | 5V Rail (`VIN`) | Required for full backlight brightness |
| | `GND` | GND Rail | Common ground |
| | `SDA` | `GPIO 21` | Default ESP32 I2C Data |
| | `SCL` | `GPIO 22` | Default ESP32 I2C Clock |
| **Active Buzzer** | `+` (Long Leg) | `GPIO 18` | Triggered HIGH in pulses during alert |
| | `-` (Short Leg) | GND Rail | Common ground |
| **5V Relay Module** | `VCC` / `DC+` | 5V Rail (`VIN`) | Relay coil supply |
| *(Control Side)* | `GND` / `DC-` | GND Rail | Common ground |
| | `IN` | `GPIO 5` | Control signal |

### 3. DC Exhaust Fan Wiring (Relay Load Side)

> **Electrical Safety Warning:** Never connect a DC exhaust fan directly to the ESP32 pins. Always power the fan with a dedicated external 5V or 12V DC power adapter routed through the relay screw terminals.

- **External Power Supply Negative (`-`)** -> **Fan Black Wire (`-`)** *(Direct connection)*
- **External Power Supply Positive (`+`)** -> **Relay `COM`** *(Center screw terminal)*
- **Fan Red Wire (`+`)** -> **Relay `NO`** *(Normally Open screw terminal)*
- **Relay `NC`** -> *Leave disconnected*

---

## Circuit Diagram & Prototype

*(Replace the image paths below after uploading your circuit diagram and working prototype photos into an `assets/` folder in your repository.)*

```markdown
![Circuit Diagram](assets/circuit_diagram.png)
![Working Prototype](assets/working_prototype.jpg)
```

---

## How It Works

The firmware runs a continuous **Sense -> Evaluate -> Act** loop every 2 seconds:

1. **Sense:** Reads ambient temperature and humidity from the DHT22 (`GPIO 4`) and samples the 12-bit analog voltage from the moisture probe (`GPIO 34`).
2. **Convert & Display:** Maps the raw ADC value (`0` to `4095`) into a constrained `0%` to `100%` moisture reading and refreshes the 16x2 I2C LCD along with Serial Monitor logs (`115200` baud).
3. **Evaluate:** Compares live metrics against the safety thresholds:
   - Temperature $> 30.0^\circ\text{C}$
   - Relative Humidity $> 70.0\%$
   - Crop Moisture $> 60\%$
4. **Act:**
   - **Danger State:** Energizes the relay (`GPIO 5`) to switch ON the DC exhaust fan, displays `FAN ON!` on the LCD, and pulses the active buzzer (`GPIO 18`).
   - **Safe State:** Keeps the relay and buzzer OFF and displays `SAFE` on the LCD. Once ventilation lowers readings below the thresholds, the system automatically returns to standby monitoring.

---

## Software Setup & Installation

### 1. Install ESP32 Board Support in Arduino IDE
1. Open the **Arduino IDE** and go to **File -> Preferences**.
2. Paste the following URL into **Additional Boards Manager URLs**:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools -> Board -> Boards Manager**, search for **`esp32`** by **Espressif Systems**, and click **Install**.

### 2. Install Required Libraries
Navigate to **Sketch -> Include Library -> Manage Libraries** and install:
- **DHT sensor library** by *Adafruit* (accept prompt to install **Adafruit Unified Sensor**)
- **LiquidCrystal I2C** by *Frank de Brabander*

### 3. Upload the Code
1. Clone this repository and open `src/post_harvest_monitor/post_harvest_monitor.ino` in the Arduino IDE.
2. Select **Tools -> Board -> esp32 -> DOIT ESP32 DEVKIT V1**.
3. Select the corresponding **Port** under **Tools -> Port**.
4. Click **Upload**. *(If the console pauses at `Connecting...`, hold the physical `BOOT` button on the ESP32 until upload progress starts.)*
5. Open **Tools -> Serial Monitor** at **`115200` baud** to view live debug readings.

---

## Calibration & Threshold Customization

You can easily adapt the system to specific crops (e.g., grains, onions, potatoes, pulses) by modifying the constants at the top of the sketch:

```cpp
// Spoilage Prevention Thresholds
const float TEMP_HIGH_THRESHOLD = 30.0;    // Celsius
const float HUMID_HIGH_THRESHOLD = 70.0;   // Percent RH
const int MOISTURE_HIGH_THRESHOLD = 60;    // Percent Moisture

// 12-bit ADC Calibration (0 - 4095)
const int DRY_VALUE = 3500;                // Reading in dry air
const int WET_VALUE = 1500;                // Reading in damp medium
```

The moisture percentage is computed linearly using:

$$\text{Moisture (\%)} = \text{constrain}\left(\frac{\text{Raw}_{\text{ADC}} - \text{DRY\_VALUE}}{\text{WET\_VALUE} - \text{DRY\_VALUE}} \times 100,\ 0,\ 100\right)$$

**Troubleshooting Tips:**
- **Active-Low Relay Modules:** If your relay turns ON when it should be OFF, invert `HIGH` and `LOW` for `RELAY_PIN` in `setup()` and `loop()`.
- **Blank LCD Screen:** Check the contrast potentiometer on the blue I2C backpack and verify whether your LCD uses I2C address `0x27` or `0x3F`.

---

## Recommended Repository Structure

```text
post-harvest-storage-monitor-esp32/
├── assets/
│   ├── circuit_diagram.png
│   └── working_prototype.jpg
├── docs/
│   └── project_report.pdf
├── src/
│   └── post_harvest_monitor/
│       └── post_harvest_monitor.ino
└── README.md
```

---

## Future Scope

- **Wi-Fi Cloud Telemetry & Mobile Alerts:** Use the ESP32's built-in Wi-Fi to stream sensor logs to ThingSpeak, Blynk, or Firebase and push automated alerts to farmers' phones.
- **Spoilage Gas Sensing:** Integrate an **MQ-135** gas sensor to detect early ammonia, ethylene, and $\text{CO}_2$ emissions from rotting produce.
- **Capacitive Moisture Probe:** Upgrade from a resistive probe to a corrosion-resistant capacitive probe for long-term continuous deployment.
- **Solar & Battery Backup:** Add a solar charge controller and Li-ion battery pack for off-grid rural warehouses.

---

## License

This project is open-source and available under the [MIT License](LICENSE).
