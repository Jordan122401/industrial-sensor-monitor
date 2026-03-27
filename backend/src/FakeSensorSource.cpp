#include "FakeSensorSource.h"
#include <sstream>
#include <iomanip>
#include <chrono>

const char* FakeSensorSource::orientationLabels_[] = {
    "FLAT", "FLAT", "FLAT", "FLAT",
    "LEFT", "RIGHT", "FORWARD", "BACKWARD", "UNKNOWN"
};

FakeSensorSource::FakeSensorSource()
    : rng_(std::chrono::steady_clock::now().time_since_epoch().count()),
      tempDist_(18.0, 35.0),
      humDist_(30.0, 80.0),
      distDist_(2.0, 100.0),
      accelNoiseDist_(-2000, 2000),
      orientDist_(0, 8) {}

std::string FakeSensorSource::generateReading() {
    double temp = tempDist_(rng_);
    double hum = humDist_(rng_);
    double dist = distDist_(rng_);

    int orientIdx = orientDist_(rng_);
    std::string orient = orientationLabels_[orientIdx];

    int ax, ay, az;
    if (orient == "FLAT") {
        ax = accelNoiseDist_(rng_);
        ay = accelNoiseDist_(rng_);
        az = 16384 + accelNoiseDist_(rng_);
    } else if (orient == "LEFT") {
        ax = -14000 + accelNoiseDist_(rng_);
        ay = accelNoiseDist_(rng_);
        az = accelNoiseDist_(rng_);
    } else if (orient == "RIGHT") {
        ax = 14000 + accelNoiseDist_(rng_);
        ay = accelNoiseDist_(rng_);
        az = accelNoiseDist_(rng_);
    } else if (orient == "FORWARD") {
        ax = accelNoiseDist_(rng_);
        ay = 14000 + accelNoiseDist_(rng_);
        az = accelNoiseDist_(rng_);
    } else if (orient == "BACKWARD") {
        ax = accelNoiseDist_(rng_);
        ay = -14000 + accelNoiseDist_(rng_);
        az = accelNoiseDist_(rng_);
    } else {
        ax = accelNoiseDist_(rng_) * 3;
        ay = accelNoiseDist_(rng_) * 3;
        az = accelNoiseDist_(rng_) * 3;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "TEMP=" << temp
        << ",HUM=" << hum
        << ",DIST=" << dist
        << ",ACCEL_X=" << ax
        << ",ACCEL_Y=" << ay
        << ",ACCEL_Z=" << az
        << ",ORIENT=" << orient;

    return oss.str();
}
