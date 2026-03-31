#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <string>

struct SensorData {
    double temperature = 0.0;
    double humidity = 0.0;
    double distance = 0.0;
    int accelX = 0;
    int accelY = 0;
    int accelZ = 0;
    std::string orientation = "UNKNOWN";
};

#endif
