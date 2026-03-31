#include "SensorParser.h"
#include <sstream>

bool SensorParser::parse(const std::string& rawData, SensorData& data) {
    std::stringstream ss(rawData);
    std::string token;

    bool gotTemp = false;
    bool gotHum = false;
    bool gotDist = false;
    bool gotAccelX = false;
    bool gotAccelY = false;
    bool gotAccelZ = false;
    bool gotOrient = false;

    while (std::getline(ss, token, ',')) {
        size_t eqPos = token.find('=');
        if (eqPos == std::string::npos) {
            return false;
        }

        std::string key = token.substr(0, eqPos);
        std::string value = token.substr(eqPos + 1);

        try {
            if (key == "TEMP") {
                data.temperature = std::stod(value);
                gotTemp = true;
            } else if (key == "HUM") {
                data.humidity = std::stod(value);
                gotHum = true;
            } else if (key == "DIST") {
                data.distance = std::stod(value);
                gotDist = true;
            } else if (key == "ACCEL_X") {
                data.accelX = std::stoi(value);
                gotAccelX = true;
            } else if (key == "ACCEL_Y") {
                data.accelY = std::stoi(value);
                gotAccelY = true;
            } else if (key == "ACCEL_Z") {
                data.accelZ = std::stoi(value);
                gotAccelZ = true;
            } else if (key == "ORIENT") {
                data.orientation = value;
                gotOrient = true;
            } else {
                return false;
            }
        } catch (...) {
            return false;
        }
    }

    return gotTemp && gotHum && gotDist && gotAccelX && gotAccelY && gotAccelZ && gotOrient;
}
