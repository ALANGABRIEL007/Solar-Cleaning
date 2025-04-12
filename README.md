# ESP32 IoT Project with ESP-NOW and Blynk Integration SOLAR PANEL SURFACE CONDITIONING

Overview
This repository contains four ESP32-based codes designed to work together using ESP-NOW protocol for wireless communication and **Blynk IoT platform** for remote monitoring and control. The system is optimized for real-time data transmission, automated scheduling, and sensor-based actuation.
I made this project to clean the surface of solar panels to maintain efficiency of the output power 

## Features
- **ESP-NOW Communication**: Low-power, peer-to-peer wireless data transfer between ESP32 devices.
- **Blynk Integration**: Remote monitoring and control via the Blynk mobile app.
- **NTP Time Synchronization**: Ensures time-based automation with accurate timestamps.
- **UART Communication**: Serial data transfer between ESP32 devices.
- **Scheduled and Threshold-Based Actions**: Automatically triggers devices based on preset conditions.

## Components Used
- **ESP32 (Multiple units for Master and Slaves)**
- **Blynk IoT Platform**
- **NTP Server for Time Synchronization**
- **Various Sensors (optional, e.g., temperature, motion sensors)**
- **Relays/Motors (optional, for actuation)**

## Code Structure
This project consists of four main ESP32 programs:

### 1. Master Node Code 
   - Connects to **Wi-Fi** and syncs time using **NTP**.
   - Communicates with **Blynk** for remote control.
   - Uses **ESP-NOW** to send control signals to slave nodes.
   - Handles scheduling logic for automated actions.
   - Supports manual triggering via **Blynk Virtual Pins**.

### 2. Slave Node Code 
   - Listens for commands from the **Master** via **ESP-NOW**.
   - Controls connected peripherals (motors, LEDs, relays, etc.).
   - Sends sensor data back to the Master for logging or further processing.

### 3. UART Communication Handler 
   - Manages serial communication between ESP32 units.
   - Transmits sensor data and receives control commands.
   - Used for temperature data retrieval and relay activation.

### 4. Blynk-Controlled Node 
   - Receives commands from the **Blynk app**.
   - Sends control signals to the **Master** for execution.
   - Handles remote data logging and device status updates.

## How It Works
1. The **Master ESP32** connects to Wi-Fi and Blynk, synchronizes time, and waits for input.
2. Scheduled tasks or **Blynk commands** trigger events.
3. The **Master** sends commands to **Slave ESP32 nodes** using **ESP-NOW**.
4. **Slaves** execute actions like turning on motors or collecting sensor data.
5. If needed, data is sent back to the **Master** via **ESP-NOW** or **UART**.
6. The **Master** updates Blynk with the latest status and sensor readings.

## Setup Instructions
1. Install the necessary **Arduino libraries**:
   - `WiFi.h`
   - `BlynkSimpleEsp32.h`
   - `ESP-NOW`
   - `NTPClient.h`
2. Flash the **Master Code** to one ESP32 unit.
3. Flash the **Slave Code** to other ESP32 units.
4. Set up a **Blynk project** and link it with your ESP32.
5. Power up the devices and monitor data in **Blynk**.

## Troubleshooting
- **ESP-NOW Communication Failure?** Ensure all ESP32 nodes have the same Wi-Fi channel.
- **No Data on Blynk?** Double-check your **Blynk Auth Token** and Wi-Fi credentials.
- **Timing Issues?** Verify that NTP synchronization is working correctly.

## Future Improvements
- Add **MQTT** for cloud-based data logging.
- Implement **Deep Sleep Mode** for power efficiency.
- Enhance security with **AES encryption** for ESP-NOW messages.

---
### Contributors
- Alan Gabriel, alangabriel071@gmail.com, https://www.linkedin.com/in/alan-gabriel-2021b1219/


