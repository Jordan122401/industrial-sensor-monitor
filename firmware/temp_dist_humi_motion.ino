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
  float humi = dht11.readHumidity();
  float tempC = dht11.readTemperature();

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration_us = pulseIn(ECHO_PIN, HIGH, 30000);
  distance_cm = duration_us * 0.034 / 2.0;

  mpu.getAcceleration(&ax, &ay, &az);
  float motion = sqrt((float)ax * ax + (float)ay * ay + (float)az * az) / 16384.0;

  if (isnan(tempC) || isnan(humi)) {
    Serial.println("DHT11 read failed");
  } else {
    String sensorData =
      "TEMP=" + String(tempC, 1) +
      ",HUM=" + String(humi, 1) +
      ",DIST=" + String(distance_cm, 1) +
      ",MOTION=" + String(motion, 2);

    Serial.println(sensorData);

    if (deviceConnected) {
      sensorCharacteristic->setValue(sensorData.c_str());
      sensorCharacteristic->notify();
    }
  }

  delay(2000);
}