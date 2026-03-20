#include "BluetoothManager.h"
#include "SensorParser.h"
#include "DatabaseManager.h"
#include "SocketServer.h"
#include "Logger.h"
#include "SensorData.h"

#include <thread>
#include <chrono>

int main() {
    Logger::info("Industrial Sensor Monitor backend starting...");

    BluetoothManager bluetoothManager;
    SensorParser parser;
    DatabaseManager databaseManager;
    SocketServer socketServer;

    if (!bluetoothManager.initialize()) {
        Logger::error("BluetoothManager failed to initialize");
        return 1;
    }

    if (!databaseManager.initialize()) {
        Logger::error("DatabaseManager failed to initialize");
        return 1;
    }

    if (!socketServer.start(9000)) {
        Logger::error("SocketServer failed to start");
        return 1;
    }

    while (true) {
        std::string rawData = bluetoothManager.getLatestRawData();

        if (rawData.empty()) {
            Logger::error("No BLE data received");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        Logger::info("Raw BLE data: " + rawData);

        SensorData parsedData;
        if (!parser.parse(rawData, parsedData)) {
            Logger::error("Failed to parse sensor data");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        databaseManager.insertSensorData(parsedData);
        socketServer.updateLatestData(parsedData);

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
