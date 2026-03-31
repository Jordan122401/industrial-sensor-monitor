import asyncio
import json
import sqlite3
import os
from datetime import datetime
from bleak import BleakScanner, BleakClient

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = "../data/sensor_data.db"

ESP32_NAME = "ESP32_Sensor_Node"
CHAR_UUID = "5678"


def init_db():
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()

    cur.execute("""
        CREATE TABLE IF NOT EXISTS sensor_readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            temperature REAL,
            humidity REAL,
            distance_cm REAL,
            ax REAL,
            ay REAL,
            az REAL,
            orientation TEXT,
            timestamp_ms INTEGER,
            timestamp TEXT
        )
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS decision_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            status TEXT,
            timestamp TEXT
        )
    """)

    conn.commit()
    conn.close()


def parse_payload(text):
    text = text.strip()

    if not text:
        raise ValueError("Empty BLE message")

    if text.startswith("{"):
        data = json.loads(text)
        return {
            "temperature": data.get("temperature"),
            "humidity": data.get("humidity"),
            "distance_cm": data.get("distance_cm"),
            "ax": data.get("ax"),
            "ay": data.get("ay"),
            "az": data.get("az"),
            "orientation": data.get("orientation"),
            "timestamp_ms": data.get("timestamp_ms"),
            "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        }

    parts = text.split(",")

    if len(parts) >= 6:
        return {
            "temperature": float(parts[0]),
            "humidity": float(parts[1]),
            "distance_cm": float(parts[2]),
            "ax": float(parts[3]),
            "ay": float(parts[4]),
            "az": float(parts[5]),
            "orientation": None,
            "timestamp_ms": None,
            "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        }

    raise ValueError("BLE message format not recognized")


def save_reading(data):
    timestamp = data.get("timestamp") or datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()

    cur.execute("""
        INSERT INTO sensor_readings (
            temperature,
            humidity,
            distance_cm,
            ax,
            ay,
            az,
            orientation,
            timestamp_ms,
            timestamp
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        data.get("temperature"),
        data.get("humidity"),
        data.get("distance_cm"),
        data.get("ax"),
        data.get("ay"),
        data.get("az"),
        data.get("orientation"),
        data.get("timestamp_ms"),
        timestamp
    ))

    status = "NORMAL"
    try:
        distance = data.get("distance_cm")
        if distance is not None and float(distance) < 20:
            status = "ALERT"
    except Exception:
        pass

    cur.execute("""
        INSERT INTO decision_events (status, timestamp)
        VALUES (?, ?)
    """, (status, timestamp))

    conn.commit()
    conn.close()

    print("Saved:", data, "| Status:", status)


def notification_handler(sender, raw_data):
    try:
        text = raw_data.decode("utf-8").strip()
        print("BLE raw:", text)
        data = parse_payload(text)
        save_reading(data)
    except Exception as e:
        print("BLE parse/save error:", e)


async def main():
    init_db()

    print("Scanning for ESP32...")
    device = await BleakScanner.find_device_by_name(ESP32_NAME, timeout=15.0)

    if device is None:
        print("ESP32 not found.")
        return

    print("Found device:", device)

    async with BleakClient(device) as client:
        print("Connected:", client.is_connected)

        await client.start_notify(CHAR_UUID, notification_handler)
        print("Listening for notifications... Press Ctrl+C to stop.")

        while True:
            await asyncio.sleep(1)


if __name__ == "__main__":
    asyncio.run(main())
