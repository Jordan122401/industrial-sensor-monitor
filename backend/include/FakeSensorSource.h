#ifndef FAKE_SENSOR_SOURCE_H
#define FAKE_SENSOR_SOURCE_H

#include <string>
#include <random>

class FakeSensorSource {
public:
    FakeSensorSource();

    std::string generateReading();

private:
    std::mt19937 rng_;
    std::uniform_real_distribution<double> tempDist_;
    std::uniform_real_distribution<double> humDist_;
    std::uniform_real_distribution<double> distDist_;
    std::uniform_int_distribution<int> accelNoiseDist_;
    std::uniform_int_distribution<int> orientDist_;

    static const char* orientationLabels_[];
};

#endif
