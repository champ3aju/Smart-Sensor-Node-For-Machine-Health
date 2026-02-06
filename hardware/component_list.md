# Components List  
## Smart Sensor Node for Machine Health

---

## 1. Microcontroller & Processing

| Component | Specification | Purpose |
|---------|---------------|---------|
| STM32 Development Board | STM32F4 series (ARM Cortex-M4) | Sensor interfacing, signal processing, ML inference |
| ESP32 Dev Module | Dual-core MCU with Wi-Fi | Wireless data transmission and dashboard communication |

---

## 2. Sensors

| Component | Model | Purpose |
|---------|-------|---------|
| Current Sensor | ACS712 | Measures machine current for electrical health monitoring |
| Vibration Sensor Module | ADXL345 | Measures vibration magnitude and mechanical faults |

---

## 3. Communication Interfaces

| Component | Interface | Purpose |
|---------|-----------|---------|
| SPI Bus | STM32 ↔ ADXL345 | High-speed vibration data transfer |
| UART | STM32 ↔ ESP32 | Health data communication |
| I²C (Optional) | Sensor expansion | Additional sensor support |

---

## 4. Power Supply Components

| Component | Rating | Purpose |
|---------|--------|---------|
| Voltage Regulator 7805 | 12V → 5V | Supplies stable voltage to ESP32 and sensors |
| DC Power Adapter | 12V | External power source |
| Decoupling Capacitors | 0.1 µF, 10 µF | Noise filtering and voltage stability |

---

## 5. Analog Signal Conditioning

| Component | Value | Purpose |
|---------|-------|---------|
| Resistors | 10K, 2.2K, 1K Ohm | Voltage divider, biasing |
| Capacitors | Various | Noise filtering for ADC signals |

---

## 6. Display & Indicators (Optional)

| Component | Specification | Purpose |
|---------|---------------|---------|
| OLED Display | 128×64, I²C | Local machine health display |

---

## 7. Development & Debugging

| Component | Purpose |
|---------|---------|
| ST-Link Programmer | STM32 flashing and debugging |
| USB Cable | Power and programming |
| Breadboard / PCB | Prototyping and final assembly |

---

## 8. Software & Tools

| Tool | Purpose |
|-----|---------|
| STM32CubeIDE | Firmware development |
| Arduino IDE / PlatformIO | ESP32 programming |
| Python (TensorFlow, NumPy) | ML model training |
| Google Colab | Model development and evaluation |
| GitHub | Version control and documentation |

---

## 9. Miscellaneous

| Component | Purpose |
|---------|---------|
| Jumper Wires | Interconnections |
| Headers & Connectors | Modular sensor connections |
| Enclosure | Physical protection |

---

## 10. Summary

This components list includes all hardware and software elements required to implement the Smart Sensor Node for Machine Health. The selected components ensure low cost, real-time performance, and suitability for embedded machine learning applications.
