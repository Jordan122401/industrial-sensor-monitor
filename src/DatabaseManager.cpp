#include "DatabaseManager.h"
#include "Logger.h"

DatabaseManager::DatabaseManager()
    : db_(nullptr), dbPath_("sensor_data.db") {}

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

    const char* createTableSql =
        "CREATE TABLE IF NOT EXISTS sensor_data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "temperature REAL,"
        "humidity REAL,"
        "distance REAL,"
        "motion REAL"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, createTableSql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        Logger::error("Failed to create table: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }

    Logger::info("Database table ready");
    return true;
}

void DatabaseManager::insertSensorData(const SensorData& data) {
    if (db_ == nullptr) {
        Logger::error("Database is not initialized");
        return;
    }

    const char* insertSql =
        "INSERT INTO sensor_data (temperature, humidity, distance, motion) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, insertSql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        Logger::error("Failed to prepare INSERT statement: " + std::string(sqlite3_errmsg(db_)));
        return;
    }

    sqlite3_bind_double(stmt, 1, data.temperature);
    sqlite3_bind_double(stmt, 2, data.humidity);
    sqlite3_bind_double(stmt, 3, data.distance);
    sqlite3_bind_double(stmt, 4, data.motion);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        Logger::error("Failed to execute INSERT statement: " + std::string(sqlite3_errmsg(db_)));
    } else {
        Logger::info("Sensor reading inserted into database");
    }

    sqlite3_finalize(stmt);
}
