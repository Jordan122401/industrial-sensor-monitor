#ifndef SENSOR_PARSER_H
#define SENSOR_PARSER_H

#include <string>
#include "SensorData.h"

class SensorParser {
public:
    bool parse(const std::string& rawData, SensorData& data);
};

#endif
