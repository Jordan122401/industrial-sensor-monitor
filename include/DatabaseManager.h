#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "SensorData.h"
#include <sqlite3.h>
#include <string>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool initialize();
    void insertSensorData(const SensorData& data);

private:
    sqlite3* db_;
    std::string dbPath_;
};

#endif
