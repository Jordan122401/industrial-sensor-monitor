#include "BluetoothManager.h"
#include "FakeSensorSource.h"
#include "SensorParser.h"
#include "DecisionEngine.h"
#include "DatabaseManager.h"
#include "SocketServer.h"
#include "Logger.h"
#include "SensorData.h"

#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <string>

namespace {
std::string resolveDatabasePath(int argc, char* argv[]) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--db-path") == 0) {
            return argv[i + 1];
        }
    }

    const char* envPath = std::getenv("SENSOR_DB_PATH");
    if (envPath != nullptr && std::strlen(envPath) > 0) {
        return envPath;
    }

    return "sensor_data.db";
}

int resolveMaxIterations(int argc, char* argv[]) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], "--iterations") == 0) {
            return std::max(0, std::atoi(argv[i + 1]));
        }
    }

    const char* envIterations = std::getenv("SENSOR_PLATFORM_MAX_ITERATIONS");
    if (envIterations != nullptr && std::strlen(envIterations) > 0) {
        return std::max(0, std::atoi(envIterations));
    }

    return 0;
}
}

int main(int argc, char* argv[]) {
    Logger::info("Autonomous Sensor Integration Platform starting...");

    bool useFake = false;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--fake") == 0) {
            useFake = true;
        }
    }

    const std::string dbPath = resolveDatabasePath(argc, argv);
    const int maxIterations = resolveMaxIterations(argc, argv);

    BluetoothManager bluetoothManager;
    FakeSensorSource fakeSource;
    SensorParser parser;
    DecisionEngine decisionEngine;
    DatabaseManager databaseManager(dbPath);
    SocketServer socketServer;

    decisionEngine.setDistanceThreshold(15.0);

    if (!useFake) {
        if (!bluetoothManager.initialize()) {
            Logger::error("BluetoothManager failed to initialize");
            return 1;
        }
    } else {
        Logger::info("Running in FAKE SENSOR mode (no BLE required)");
    }

    if (!databaseManager.initialize()) {
        Logger::error("DatabaseManager failed to initialize");
        return 1;
    }

    Logger::info("Using SQLite database path: " + databaseManager.getDatabasePath());

    if (!socketServer.start(9000)) {
        Logger::error("SocketServer failed to start");
        return 1;
    }

    int iterationCount = 0;
    while (true) {
        std::string rawData;

        if (useFake) {
            rawData = fakeSource.generateReading();
        } else {
            rawData = bluetoothManager.getLatestRawData();
        }

        if (rawData.empty()) {
            Logger::error("No sensor data received");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        Logger::info("Raw data: " + rawData);

        SensorData parsedData;
        if (!parser.parse(rawData, parsedData)) {
            Logger::error("Failed to parse sensor data");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        DecisionResult decision = decisionEngine.evaluate(parsedData);

        Logger::info("System status: " + decision.statusLabel
            + " | Obstacle: " + (decision.obstacleDetected ? "YES" : "NO")
            + " | Unstable: " + (decision.platformUnstable ? "YES" : "NO"));

        databaseManager.insertSensorData(parsedData);
        databaseManager.insertDecisionEvent(
            decision.statusLabel,
            decision.obstacleDetected,
            decision.platformUnstable,
            decision.distanceAtDecision,
            decision.orientationAtDecision
        );

        socketServer.updateLatestData(parsedData);
        socketServer.updateSystemStatus(decision.statusLabel);

        ++iterationCount;
        if (maxIterations > 0 && iterationCount >= maxIterations) {
            Logger::info("Reached requested iteration limit, shutting down");
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
