from flask import Flask, jsonify, render_template
import sqlite3
import os


def create_app():
    app = Flask(__name__)

    base_dir = os.path.dirname(os.path.abspath(__file__))
    default_db_path = os.path.join(base_dir, "sensor_data.db")
    app.config["DB_PATH"] = os.environ.get("SENSOR_DB_PATH", default_db_path)

    REQUIRED_TABLES = {"sensor_readings", "decision_events"}

    def db_path():
        return app.config["DB_PATH"]

    def db_exists():
        return os.path.exists(db_path())

    def get_db():
        conn = sqlite3.connect(db_path())
        conn.row_factory = sqlite3.Row
        return conn

    def get_existing_tables():
        if not db_exists():
            return set()

        conn = get_db()
        try:
            rows = conn.execute(
                "SELECT name FROM sqlite_master WHERE type='table'"
            ).fetchall()
            return {row["name"] for row in rows}
        finally:
            conn.close()

    def missing_tables():
        return sorted(REQUIRED_TABLES - get_existing_tables())

    def json_error(message, status_code=500, extra=None):
        payload = {
            "status": "error",
            "message": message,
            "db_path": db_path(),
        }
        if extra:
            payload.update(extra)
        return jsonify(payload), status_code

    def fetch_all(query):
        if not db_exists():
            return None, json_error("Database file does not exist.", 500)

        missing = missing_tables()
        if missing:
            return None, json_error(
                "Required table(s) missing.",
                500,
                {"missing_tables": missing},
            )

        try:
            conn = get_db()
            try:
                rows = conn.execute(query).fetchall()
                return rows, None
            finally:
                conn.close()
        except sqlite3.Error as e:
            return None, json_error(
                "SQLite query failed.",
                500,
                {"sqlite_error": str(e)},
            )

    def fetch_one(query):
        if not db_exists():
            return None, json_error("Database file does not exist.", 500)

        missing = missing_tables()
        if missing:
            return None, json_error(
                "Required table(s) missing.",
                500,
                {"missing_tables": missing},
            )

        try:
            conn = get_db()
            try:
                row = conn.execute(query).fetchone()
                return row, None
            finally:
                conn.close()
        except sqlite3.Error as e:
            return None, json_error(
                "SQLite query failed.",
                500,
                {"sqlite_error": str(e)},
            )

    @app.route("/", methods=["GET"])
    def home():
        return render_template("index.html")

    @app.route("/api/health", methods=["GET"])
    def health():
        existing_tables = sorted(get_existing_tables()) if db_exists() else []
        return jsonify(
            {
                "status": "ok",
                "db_path": db_path(),
                "db_exists": db_exists(),
                "required_tables_present": len(missing_tables()) == 0 if db_exists() else False,
                "existing_tables": existing_tables,
                "missing_tables": missing_tables() if db_exists() else sorted(REQUIRED_TABLES),
            }
        )

    @app.route("/api/sensor-data", methods=["GET"])
    def get_sensor_data():
        rows, error = fetch_all(
            "SELECT * FROM sensor_readings ORDER BY id DESC LIMIT 50"
        )
        if error:
            return error
        return jsonify([dict(row) for row in rows])

    @app.route("/api/sensor-data/latest", methods=["GET"])
    def get_latest_sensor_data():
        row, error = fetch_one(
            "SELECT * FROM sensor_readings ORDER BY id DESC LIMIT 1"
        )
        if error:
            return error
        if row is None:
            return jsonify({"status": "error", "message": "No sensor data found."}), 404
        return jsonify(dict(row))

    @app.route("/api/decision-events", methods=["GET"])
    def get_decision_events():
        rows, error = fetch_all(
            "SELECT * FROM decision_events ORDER BY id DESC LIMIT 50"
        )
        if error:
            return error
        return jsonify([dict(row) for row in rows])

    @app.route("/api/decision-events/latest", methods=["GET"])
    def get_latest_decision():
        row, error = fetch_one(
            "SELECT * FROM decision_events ORDER BY id DESC LIMIT 1"
        )
        if error:
            return error
        if row is None:
            return jsonify({"status": "error", "message": "No decision events found."}), 404
        return jsonify(dict(row))

    @app.route("/api/status", methods=["GET"])
    def get_status():
        row, error = fetch_one(
            "SELECT status, timestamp FROM decision_events ORDER BY id DESC LIMIT 1"
        )
        if error:
            return error
        if row is None:
            return jsonify({"status": "UNKNOWN", "timestamp": None})
        return jsonify(dict(row))

    return app


app = create_app()

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)