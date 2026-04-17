#include <DHT.h>
#include <math.h>
#include <Wire.h>
#include <MPU6050.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DHT11_PIN 4
#define TRIG_PIN 23
#define ECHO_PIN 19

DHT dht11(DHT11_PIN, DHT11);
MPU6050 mpu;

long duration_us;
float distance_cm;
int16_t ax, ay, az;

BLECharacteristic *sensorCharacteristic;
bool deviceConnected = false;

// ---- distance filtering variables ----
float lastValidDistance = 0.0;
float filteredDistance = 0.0;
const float MIN_DISTANCE_CM = 2.0;
const float MAX_DISTANCE_CM = 400.0;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("Client connected");
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("Client disconnected");
    BLEDevice::startAdvertising();
  }
};

String detectOrientation(int16_t ax, int16_t ay, int16_t az) {
  if (az > 12000 && abs(ax) < 4000 && abs(ay) < 4000) {
    return "FLAT";
  }

  if (ax > 12000) {
    return "RIGHT";
  }

  if (ax < -12000) {
    return "LEFT";
  }

  if (ay > 12000) {
    return "FORWARD";
  }

  if (ay < -12000) {
    return "BACKWARD";
  }

  return "UNKNOWN";
}

// ---- new helper function ----
float readFilteredDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration_us = pulseIn(ECHO_PIN, HIGH, 30000);

  // If no echo came back, keep the last good value
  if (duration_us == 0) {
    return filteredDistance > 0 ? filteredDistance : lastValidDistance;
  }

  float rawDistance = duration_us * 0.0343 / 2.0;

  // Throw away obviously bad readings
  if (rawDistance < MIN_DISTANCE_CM || rawDistance > MAX_DISTANCE_CM) {
    return filteredDistance > 0 ? filteredDistance : lastValidDistance;
  }

  // Save the new good reading
  lastValidDistance = rawDistance;

  // Smooth it so it doesn't jump around as much
  if (filteredDistance == 0.0) {
    filteredDistance = rawDistance;
  } else {
    filteredDistance = (0.7 * filteredDistance) + (0.3 * rawDistance);
  }

  return filteredDistance;
}

void setup() {
  Serial.begin(115200);

  dht11.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Wire.begin(21, 22);
  mpu.initialize();

  Serial.println("Sensors initialized");

  BLEDevice::init("ESP32_Sensor_Node");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *sensorService = pServer->createService("1234");

  sensorCharacteristic = sensorService->createCharacteristic(
    "5678",
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  sensorCharacteristic->addDescriptor(new BLE2902());

  sensorService->start();
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising started");
}

void loop() {
  // ---------- DHT11 ----------
  float humi  = dht11.readHumidity();
  float tempC = dht11.readTemperature();

  // ---------- ULTRASONIC ----------
  distance_cm = readFilteredDistance();

  // ---------- MPU6050 ----------
  mpu.getAcceleration(&ax, &ay, &az);
  String orientation = detectOrientation(ax, ay, az);

  // ---------- OUTPUT ----------
  if (isnan(tempC) || isnan(humi)) {
    Serial.println("DHT11 read failed");
  } else {
    String sensorData = "{";
    sensorData += "\"temperature\":" + String(tempC, 1) + ",";
    sensorData += "\"humidity\":" + String(humi, 1) + ",";
    sensorData += "\"distance_cm\":" + String(distance_cm, 1) + ",";
    sensorData += "\"ax\":" + String(ax) + ",";
    sensorData += "\"ay\":" + String(ay) + ",";
    sensorData += "\"az\":" + String(az) + ",";
    sensorData += "\"orientation\":\"" + orientation + "\",";
    sensorData += "\"timestamp_ms\":" + String(millis());
    sensorData += "}";

    Serial.println(sensorData);

    if (deviceConnected) {
      sensorCharacteristic->setValue(sensorData.c_str());
      sensorCharacteristic->notify();
    }
  }

  delay(500);
}