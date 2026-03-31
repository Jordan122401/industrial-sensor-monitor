import importlib.util
import os
import sqlite3
from pathlib import Path
import shutil
import uuid

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]
API_MODULE_PATH = REPO_ROOT / "api" / "app.py"
TEST_RUN_ROOT = REPO_ROOT / "data" / "test_runs"


def load_app(db_path: Path):
    os.environ["SENSOR_DB_PATH"] = str(db_path)

    spec = importlib.util.spec_from_file_location("sensor_api_app", API_MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module.app


def seed_database(db_path: Path):
    db_path.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(db_path)
    conn.execute(
        """
        CREATE TABLE sensor_readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            temperature REAL,
            humidity REAL,
            distance REAL,
            accel_x INTEGER,
            accel_y INTEGER,
            accel_z INTEGER,
            orientation TEXT
        )
        """
    )
    conn.execute(
        """
        CREATE TABLE decision_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            status TEXT,
            obstacle_detected INTEGER,
            platform_unstable INTEGER,
            distance_at_decision REAL,
            orientation_at_decision TEXT
        )
        """
    )
    conn.execute(
        """
        INSERT INTO sensor_readings
        (temperature, humidity, distance, accel_x, accel_y, accel_z, orientation)
        VALUES (22.9, 62.5, 7.6, -1440, 648, 17756, 'FLAT')
        """
    )
    conn.execute(
        """
        INSERT INTO decision_events
        (status, obstacle_detected, platform_unstable, distance_at_decision, orientation_at_decision)
        VALUES ('OBSTACLE_NEAR', 1, 0, 7.6, 'FLAT')
        """
    )
    conn.commit()
    conn.close()


@pytest.fixture()
def test_db_path():
    run_dir = TEST_RUN_ROOT / f"api_test_{uuid.uuid4().hex}"
    run_dir.mkdir(parents=True, exist_ok=True)
    db_path = run_dir / "sensor_data.db"
    try:
        yield db_path
    finally:
        shutil.rmtree(run_dir, ignore_errors=True)


def test_health_reports_shared_db_path(test_db_path):
    seed_database(test_db_path)
    app = load_app(test_db_path)
    client = app.test_client()

    response = client.get("/api/health")

    assert response.status_code == 200
    payload = response.get_json()
    assert payload["status"] == "ok"
    assert payload["db_path"] == str(test_db_path)


def test_latest_sensor_and_status_endpoints(test_db_path):
    seed_database(test_db_path)
    app = load_app(test_db_path)
    client = app.test_client()

    latest_sensor = client.get("/api/sensor-data/latest")
    assert latest_sensor.status_code == 200
    latest_sensor_payload = latest_sensor.get_json()
    assert latest_sensor_payload["temperature"] == 22.9
    assert latest_sensor_payload["orientation"] == "FLAT"

    status = client.get("/api/status")
    assert status.status_code == 200
    status_payload = status.get_json()
    assert status_payload["status"] == "OBSTACLE_NEAR"
