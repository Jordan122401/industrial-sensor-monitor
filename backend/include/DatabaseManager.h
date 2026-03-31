#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "SensorData.h"
#include <sqlite3.h>
#include <string>

class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& dbPath = "sensor_data.db");
    ~DatabaseManager();

    bool initialize();
    const std::string& getDatabasePath() const;
    void insertSensorData(const SensorData& data);
    void insertDecisionEvent(const std::string& status, bool obstacleDetected,
                             bool platformUnstable, double distance,
                             const std::string& orientation);

private:
    sqlite3* db_;
    std::string dbPath_;
};

#endif
