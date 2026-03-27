# Autonomous Sensor Integration and Test Platform

An embedded systems integration project that collects real-time sensor data from an ESP32 microcontroller, processes it on a Raspberry Pi 5, runs decision logic, logs events to a database, and exposes data through TCP and REST interfaces. Designed to reflect the workflow of a software integration and test engineer in aerospace/defense environments.

## Architecture

```
ESP32 Sensor Node (BLE)
        |
        v
Raspberry Pi 5 Integration Host
  ├── BluetoothManager    — receives BLE notifications via BlueZ/D-Bus
  ├── SensorParser         — parses key=value payload into structured data
  ├── DecisionEngine       — evaluates system status (SAFE / OBSTACLE_NEAR / UNSTABLE_PLATFORM / AVOIDANCE_TRIGGER)
  ├── DatabaseManager      — logs sensor readings and decision events to SQLite
  ├── SocketServer         — serves live data over TCP (port 9000)
  └── FakeSensorSource     — generates simulated data for testing without hardware
        |
        v
  Python REST API (Flask, port 5000)
  Docker / docker-compose
```

## Hardware

| Component | Role |
|-----------|------|
| ESP32 NodeMCU-32S | Sensor node — collects and transmits via BLE |
| Raspberry Pi 5 | Integration host — processes, decides, logs, serves |
| MPU6050 | 3-axis accelerometer for orientation detection |
| HC-SR04 | Ultrasonic distance sensor |
| DHT11 | Temperature and humidity sensor |

## Sensor Payload Format

The ESP32 transmits this string over BLE every 2 seconds:

```
TEMP=22.9,HUM=62.5,DIST=7.6,ACCEL_X=-1440,ACCEL_Y=648,ACCEL_Z=17756,ORIENT=FLAT
```

Orientation labels: `FLAT`, `LEFT`, `RIGHT`, `FORWARD`, `BACKWARD`, `UNKNOWN`

## Decision Engine

The DecisionEngine evaluates each sensor reading and produces a system status:

| Status | Condition |
|--------|-----------|
| `SAFE` | Distance >= threshold AND orientation == FLAT |
| `OBSTACLE_NEAR` | Distance < threshold (default 15 cm) |
| `UNSTABLE_PLATFORM` | Orientation != FLAT |
| `AVOIDANCE_TRIGGER` | Both obstacle detected AND platform unstable |

Each decision event is logged to SQLite with timestamp, status, flags, and sensor snapshot values.

## Project Structure

```
├── firmware/                  # ESP32 Arduino firmware
│   └── sensor_node.ino
├── backend/                   # C++ integration host (Raspberry Pi)
│   ├── include/               # Header files
│   ├── src/                   # Implementation files
│   ├── tests/                 # Test files
│   ├── CMakeLists.txt
│   └── Dockerfile
├── api/                       # Python REST API
│   ├── app.py
│   ├── requirements.txt
│   └── Dockerfile
├── test_tools/                # Test utilities and scripts
├── docs/                      # Documentation
├── data/                      # Runtime data (DB files, gitignored)
├── docker-compose.yml
└── README.md
```

## Building and Running

### C++ Backend (on Raspberry Pi)

```bash
cd backend
mkdir -p build && cd build
cmake .. && make
```

Run with real BLE hardware:
```bash
./sensor_platform
```

Run with fake/simulated sensor data (no ESP32 needed):
```bash
./sensor_platform --fake
```

Run a short fake-data capture to populate a shared database for testing:
```bash
SENSOR_DB_PATH=../data/sensor_data.db ./sensor_platform --fake --iterations 5
```

### REST API

```bash
cd api
pip install -r requirements.txt
python app.py
```

### Docker

```bash
docker-compose up --build
```

Both services use the same SQLite path inside Docker:
`/app/data/sensor_data.db`

## TCP Interface

Connect to port 9000 and send a command:

```bash
# Get full sensor data + system status
echo "GET_DATA" | nc localhost 9000
# Response: TEMP=22.9,HUM=62.5,DIST=7.6,ACCEL_X=-1440,ACCEL_Y=648,ACCEL_Z=17756,ORIENT=FLAT,STATUS=SAFE

# Get system status only
echo "GET_STATUS" | nc localhost 9000
# Response: STATUS=SAFE
```

## REST API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/health` | Health check |
| GET | `/api/sensor-data` | Last 50 sensor readings |
| GET | `/api/sensor-data/latest` | Most recent sensor reading |
| GET | `/api/decision-events` | Last 50 decision events |
| GET | `/api/decision-events/latest` | Most recent decision event |
| GET | `/api/status` | Current system status |

## Test Workflow

### 1. Backend unit tests

```bash
cd backend
mkdir -p build && cd build
cmake .. -DBUILD_TESTING=ON
cmake --build .
ctest --output-on-failure
```

### 2. API tests

```bash
pip install -r api/requirements.txt
pytest test_tools/test_api_integration.py
```

### 3. End-to-end fake-mode check

1. Build the backend on the Raspberry Pi.
2. Run `./sensor_platform --fake --db-path ../data/sensor_data.db --iterations 5`.
3. Start the API with `SENSOR_DB_PATH=../data/sensor_data.db python api/app.py`.
4. Query `GET /api/sensor-data/latest` and `GET /api/status`.
5. Confirm the API returns recent sensor data and a non-`UNKNOWN` status.

## Database Schema

**sensor_readings** — raw sensor data log

| Column | Type |
|--------|------|
| id | INTEGER PRIMARY KEY |
| timestamp | DATETIME |
| temperature | REAL |
| humidity | REAL |
| distance | REAL |
| accel_x | INTEGER |
| accel_y | INTEGER |
| accel_z | INTEGER |
| orientation | TEXT |

**decision_events** — decision engine output log

| Column | Type |
|--------|------|
| id | INTEGER PRIMARY KEY |
| timestamp | DATETIME |
| status | TEXT |
| obstacle_detected | INTEGER |
| platform_unstable | INTEGER |
| distance_at_decision | REAL |
| orientation_at_decision | TEXT |

## Technologies

- **Firmware**: C++ / Arduino, BLE (ESP32)
- **Backend**: C++17, CMake, BlueZ/D-Bus, SQLite, POSIX sockets
- **API**: Python 3, Flask
- **Infrastructure**: Docker, docker-compose
- **Hardware**: ESP32, Raspberry Pi 5, MPU6050, HC-SR04, DHT11
