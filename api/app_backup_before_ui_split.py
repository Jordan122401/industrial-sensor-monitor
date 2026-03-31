from flask import Flask, jsonify
import sqlite3
import os

HOME_TEMPLATE = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Industrial Sensor Monitor</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        html {
            scroll-behavior: smooth;
        }

        body {
            font-family: Arial, sans-serif;
            background: #d97706;
            color: white;
        }

        .hero {
            min-height: 100vh;
            background: linear-gradient(180deg, #d97706 0%, #c2410c 100%);
            padding: 50px 20px 70px;
            text-align: center;
        }

        .hero h1 {
            font-family: Georgia, "Times New Roman", serif;
            font-size: 3.2rem;
            margin-bottom: 16px;
        }

        .hero-subtext {
            max-width: 760px;
            margin: 0 auto 24px;
            font-size: 1.05rem;
            line-height: 1.6;
            color: #ffedd5;
        }

        .hero-button {
            display: inline-block;
            padding: 14px 28px;
            border: 2px solid white;
            color: white;
            text-decoration: none;
            margin-bottom: 50px;
            font-size: 1rem;
            transition: 0.2s ease;
        }

        .hero-button:hover {
            background: white;
            color: #c2410c;
        }

        .menu-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
            gap: 34px;
            max-width: 1200px;
            margin: 0 auto;
            align-items: start;
        }

        .menu-card {
            text-align: center;
        }

        .plate {
            width: 240px;
            height: 240px;
            border-radius: 50%;
            background: radial-gradient(circle at 35% 30%, #ffffff 0%, #fff7ed 58%, #fed7aa 100%);
            margin: 0 auto 26px;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 18px 28px rgba(0, 0, 0, 0.28);
            border: 6px solid rgba(255, 255, 255, 0.45);
        }

        .plate-inner {
            width: 168px;
            height: 168px;
            border-radius: 50%;
            background: linear-gradient(135deg, #f97316 0%, #fb923c 100%);
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 3.4rem;
            box-shadow: inset 0 8px 14px rgba(255,255,255,0.2);
        }

        .menu-card h3 {
            font-family: Georgia, "Times New Roman", serif;
            font-size: 1.6rem;
            margin-bottom: 12px;
        }

        .menu-card p {
            font-size: 1rem;
            line-height: 1.6;
            max-width: 280px;
            margin: 0 auto 12px;
            color: #ffedd5;
        }

        .metric-value {
            font-family: Georgia, "Times New Roman", serif;
            font-size: 2.1rem;
            margin-top: 12px;
        }

        .live-section {
            background: #fff7ed;
            color: #431407;
            padding: 50px 20px 70px;
        }

        .live-wrap {
            max-width: 1200px;
            margin: 0 auto;
        }

        .section-title {
            font-family: Georgia, "Times New Roman", serif;
            font-size: 2.2rem;
            margin-bottom: 12px;
            text-align: center;
        }

        .section-subtitle {
            text-align: center;
            margin-bottom: 32px;
            color: #7c2d12;
        }

        .status-row {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
            gap: 16px;
            margin-bottom: 26px;
        }

        .status-box {
            background: white;
            border-radius: 18px;
            padding: 20px;
            box-shadow: 0 8px 18px rgba(0,0,0,0.08);
            border: 1px solid #fdba74;
        }

        .status-label {
            font-size: 0.95rem;
            color: #9a3412;
            margin-bottom: 10px;
        }

        .status-value {
            font-size: 1.35rem;
            font-weight: bold;
            color: #431407;
            word-break: break-word;
        }

        .panel {
            background: white;
            border-radius: 20px;
            padding: 22px;
            box-shadow: 0 8px 18px rgba(0,0,0,0.08);
            border: 1px solid #fdba74;
            margin-bottom: 22px;
            overflow-x: auto;
        }

        .panel h3 {
            margin-bottom: 14px;
            color: #7c2d12;
        }

        pre {
            background: #fff7ed;
            color: #431407;
            padding: 16px;
            border-radius: 14px;
            overflow-x: auto;
            white-space: pre-wrap;
            word-wrap: break-word;
            border: 1px solid #fdba74;
        }

        table {
            width: 100%;
            border-collapse: collapse;
        }

        th, td {
            padding: 12px;
            border-bottom: 1px solid #fed7aa;
            text-align: left;
        }

        th {
            color: #9a3412;
            background: #fff7ed;
        }

        .error-box {
            display: none;
            background: #7f1d1d;
            color: #fee2e2;
            padding: 14px 16px;
            border-radius: 14px;
            margin-bottom: 20px;
        }

        .footer-note {
            text-align: center;
            color: #7c2d12;
            margin-top: 10px;
            font-size: 0.95rem;
        }

        @media (max-width: 700px) {
            .hero h1 {
                font-size: 2.4rem;
            }

            .plate {
                width: 210px;
                height: 210px;
            }

            .plate-inner {
                width: 150px;
                height: 150px;
                font-size: 3rem;
            }
        }
    </style>
</head>
<body>
    <section class="hero">
        <h1>Our Sensor Menu</h1>
        <p class="hero-subtext">
            This page shows live ESP32 sensor readings in a menu-style layout.
            The three big circles are your main live values: temperature, humidity, and distance.
        </p>

        <a class="hero-button" href="#live-data">View More</a>

        <div class="menu-grid">
            <div class="menu-card">
                <div class="plate">
                    <div class="plate-inner">🌡️</div>
                </div>
                <h3>Temperature</h3>
                <p>Live temperature coming from your ESP32 sensor system.</p>
                <div class="metric-value" id="temperatureValue">-- °C</div>
            </div>

            <div class="menu-card">
                <div class="plate">
                    <div class="plate-inner">💧</div>
                </div>
                <h3>Humidity</h3>
                <p>Live humidity reading coming from the DHT sensor.</p>
                <div class="metric-value" id="humidityValue">-- %</div>
            </div>

            <div class="menu-card">
                <div class="plate">
                    <div class="plate-inner">📏</div>
                </div>
                <h3>Distance</h3>
                <p>Live distance reading from the ultrasonic sensor.</p>
                <div class="metric-value" id="distanceValue">-- cm</div>
            </div>
        </div>
    </section>

    <section class="live-section" id="live-data">
        <div class="live-wrap">
            <h2 class="section-title">Live Data Dashboard</h2>
            <p class="section-subtitle">
                This section auto-refreshes every 2 seconds.
            </p>

            <div id="errorBox" class="error-box"></div>

            <div class="status-row">
                <div class="status-box">
                    <div class="status-label">System Status</div>
                    <div class="status-value" id="systemStatus">Loading...</div>
                </div>

                <div class="status-box">
                    <div class="status-label">Database</div>
                    <div class="status-value" id="databaseStatus">Checking...</div>
                </div>

                <div class="status-box">
                    <div class="status-label">Tables Ready</div>
                    <div class="status-value" id="tablesStatus">Checking...</div>
                </div>

                <div class="status-box">
                    <div class="status-label">Last Update</div>
                    <div class="status-value" id="lastUpdate">Waiting...</div>
                </div>
            </div>

            <div class="panel">
                <h3>Latest Sensor JSON</h3>
                <pre id="latestJson">Waiting for data...</pre>
            </div>

            <div class="panel">
                <h3>Recent Readings</h3>
                <table>
                    <thead id="tableHead"></thead>
                    <tbody id="tableBody"></tbody>
                </table>
            </div>

            <p class="footer-note">
                If these numbers do not move, your ESP32 is not writing new data into the database yet.
            </p>
        </div>
    </section>

    <script>
        function setText(id, value) {
            document.getElementById(id).textContent = value;
        }

        function showError(message) {
            const box = document.getElementById("errorBox");
            box.style.display = "block";
            box.textContent = message;
        }

        function clearError() {
            const box = document.getElementById("errorBox");
            box.style.display = "none";
            box.textContent = "";
        }

        async function getJson(url) {
            const response = await fetch(url, { cache: "no-store" });
            const data = await response.json();

            if (!response.ok) {
                throw new Error(data.message || "Request failed");
            }

            return data;
        }

        async function tryGetJson(url, fallbackValue) {
            try {
                return await getJson(url);
            } catch (error) {
                return fallbackValue;
            }
        }

        function formatValue(value, suffix) {
            if (value === null || value === undefined || value === "") {
                return "-- " + suffix;
            }
            return value + " " + suffix;
        }

        function buildTable(rows) {
            const head = document.getElementById("tableHead");
            const body = document.getElementById("tableBody");

            head.innerHTML = "";
            body.innerHTML = "";

            if (!rows || rows.length === 0) {
                body.innerHTML = "<tr><td>No data yet.</td></tr>";
                return;
            }

            const columns = Object.keys(rows[0]);

            let headerHtml = "<tr>";
            for (const col of columns) {
                headerHtml += `<th>${col}</th>`;
            }
            headerHtml += "</tr>";
            head.innerHTML = headerHtml;

            for (const row of rows.slice(0, 10)) {
                let rowHtml = "<tr>";
                for (const col of columns) {
                    rowHtml += `<td>${row[col]}</td>`;
                }
                rowHtml += "</tr>";
                body.innerHTML += rowHtml;
            }
        }

        async function refreshDashboard() {
            try {
                clearError();

                const health = await getJson("/api/health");
                const latest = await tryGetJson("/api/sensor-data/latest", {});
                const status = await tryGetJson("/api/status", {"status": "UNKNOWN", "timestamp": null});
                const rows = await tryGetJson("/api/sensor-data", []);

                setText("databaseStatus", health.db_exists ? "Connected" : "Missing");
                setText("tablesStatus", health.required_tables_present ? "Ready" : "Missing");
                setText("systemStatus", status.status || "UNKNOWN");
                setText("lastUpdate", latest.timestamp || status.timestamp || "No timestamp yet");

                setText("temperatureValue", formatValue(latest.temperature, "°C"));
                setText("humidityValue", formatValue(latest.humidity, "%"));
                setText("distanceValue", formatValue(latest.distance_cm, "cm"));

                document.getElementById("latestJson").textContent =
                    JSON.stringify(latest, null, 2);

                buildTable(rows);

            } catch (error) {
                showError(error.message);
                setText("systemStatus", "ERROR");
                setText("databaseStatus", "CHECK FAILED");
                setText("tablesStatus", "CHECK FAILED");
            }
        }

        refreshDashboard();
        setInterval(refreshDashboard, 2000);
    </script>
</body>
</html>
"""


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
        return HOME_TEMPLATE

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
