# Industrial Sensor Monitoring System

This project is a real-time embedded-to-web monitoring system that uses an ESP32 sensor node and a backend written in Python. It collects data about the environment and movement, sends it over Bluetooth Low Energy (BLE), stores it in a database, and shows it on a live dashboard.

---

## System Overview

ESP32 → BLE → Python Logger → SQLite → Flask Dashboard


---

## What the System Does

The ESP32 gathers data from sensors in the real world and sends it over BLE at one packet per second. The backend gets, processes, and stores that data, and the frontend shows it in real time.

### Sensor Data Collected

- Temperature (°C)
- Humidity (%)
- Distance (cm) using ultrasonic sensor
- Acceleration (X, Y, Z axes)

### Orientation Detection

Using accelerometer data, the system determines the device orientation:

- Flat (normal resting position)
- Tilted left / right
- Tilted forward / backward
- Upright / vertical states

This adds basic **state awareness**, making the system closer to a real industrial monitoring setup instead of just raw data collection.

---

## Features

- Real-time BLE telemetry from ESP32
- Continuous data streaming and ingestion
- SQLite database for persistent local storage
- Flask backend serving structured API endpoints
- Live dashboard with:
  - sensor cards (temperature, humidity, distance)
  - system status indicators
  - recent telemetry table
  - raw JSON payload viewer
  - real-time distance chart
- Orientation detection using accelerometer data
- Basic alert logic based on distance threshold

---

## Tech Stack

### Embedded / Hardware
- ESP32
- Arduino C++
- DHT11 (temperature + humidity)
- Ultrasonic sensor (distance)
- MPU6050 (accelerometer)

### Backend
- Python
- Flask
- SQLite
- Bleak (BLE communication)

### Frontend
- HTML
- Tailwind CSS
- JavaScript
- Chart.js

---

## How It Works (Step-by-Step)

1. The ESP32 reads sensor values (temperature, humidity, distance, acceleration).
2. Data is formatted into JSON and sent via BLE notifications.
3. A Python script (`ble_logger.py`) listens for BLE data.
4. Incoming data is parsed and inserted into SQLite.
5. Flask (`app.py`) exposes API endpoints to retrieve this data.
6. The frontend dashboard fetches data every few seconds.
7. The UI updates:
   - live sensor values
   - recent readings
   - system status
   - distance trend chart

---

## How to Run

### 1. Setup environment

```bash
cd api
python3 -m venv ../venv
source ../venv/bin/activate
pip install -r requirements.txt

---

##2. Start BLE Logger 
python ble_logger.py