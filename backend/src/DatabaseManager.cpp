#include "DatabaseManager.h"
#include "Logger.h"

DatabaseManager::DatabaseManager(const std::string& dbPath)
    : db_(nullptr), dbPath_(dbPath) {}

DatabaseManager::~DatabaseManager() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::initialize() {
    int rc = sqlite3_open(dbPath_.c_str(), &db_);

    if (rc != SQLITE_OK) {
        Logger::error("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }

    Logger::info("Opened SQLite database: " + dbPath_);

    const char* createSensorTableSql =
        "CREATE TABLE IF NOT EXISTS sensor_readings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "temperature REAL,"
        "humidity REAL,"
        "distance REAL,"
        "accel_x INTEGER,"
        "accel_y INTEGER,"
        "accel_z INTEGER,"
        "orientation TEXT"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, createSensorTableSql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        Logger::error("Failed to create sensor_readings table: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }

    const char* createDecisionTableSql =
        "CREATE TABLE IF NOT EXISTS decision_events ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "status TEXT,"
        "obstacle_detected INTEGER,"
        "platform_unstable INTEGER,"
        "distance_at_decision REAL,"
        "orientation_at_decision TEXT"
        ");";

    rc = sqlite3_exec(db_, createDecisionTableSql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        Logger::error("Failed to create decision_events table: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }

    Logger::info("Database tables ready");
    return true;
}

const std::string& DatabaseManager::getDatabasePath() const {
    return dbPath_;
}

void DatabaseManager::insertSensorData(const SensorData& data) {
    if (db_ == nullptr) {
        Logger::error("Database is not initialized");
        return;
    }

    const char* insertSql =
        "INSERT INTO sensor_readings (temperature, humidity, distance, accel_x, accel_y, accel_z, orientation) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, insertSql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        Logger::error("Failed to prepare sensor INSERT: " + std::string(sqlite3_errmsg(db_)));
        return;
    }

    sqlite3_bind_double(stmt, 1, data.temperature);
    sqlite3_bind_double(stmt, 2, data.humidity);
    sqlite3_bind_double(stmt, 3, data.distance);
    sqlite3_bind_int(stmt, 4, data.accelX);
    sqlite3_bind_int(stmt, 5, data.accelY);
    sqlite3_bind_int(stmt, 6, data.accelZ);
    sqlite3_bind_text(stmt, 7, data.orientation.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        Logger::error("Failed to insert sensor reading: " + std::string(sqlite3_errmsg(db_)));
    } else {
        Logger::info("Sensor reading inserted into database");
    }

    sqlite3_finalize(stmt);
}

void DatabaseManager::insertDecisionEvent(const std::string& status, bool obstacleDetected,
                                           bool platformUnstable, double distance,
                                           const std::string& orientation) {
    if (db_ == nullptr) {
        Logger::error("Database is not initialized");
        return;
    }

    const char* insertSql =
        "INSERT INTO decision_events (status, obstacle_detected, platform_unstable, "
        "distance_at_decision, orientation_at_decision) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, insertSql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        Logger::error("Failed to prepare decision INSERT: " + std::string(sqlite3_errmsg(db_)));
        return;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, obstacleDetected ? 1 : 0);
    sqlite3_bind_int(stmt, 3, platformUnstable ? 1 : 0);
    sqlite3_bind_double(stmt, 4, distance);
    sqlite3_bind_text(stmt, 5, orientation.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        Logger::error("Failed to insert decision event: " + std::string(sqlite3_errmsg(db_)));
    } else {
        Logger::info("Decision event logged: " + status);
    }

    sqlite3_finalize(stmt);
}
