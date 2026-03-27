#ifndef DECISION_ENGINE_H
#define DECISION_ENGINE_H

#include "SensorData.h"
#include <string>

enum class SystemStatus {
    SAFE,
    OBSTACLE_NEAR,
    UNSTABLE_PLATFORM,
    AVOIDANCE_TRIGGER
};

struct DecisionResult {
    SystemStatus status;
    std::string statusLabel;
    bool obstacleDetected;
    bool platformUnstable;
    double distanceAtDecision;
    std::string orientationAtDecision;
};

class DecisionEngine {
public:
    DecisionEngine();

    void setDistanceThreshold(double threshold);
    DecisionResult evaluate(const SensorData& data);

    static std::string statusToString(SystemStatus status);

private:
    double distanceThreshold_;
};

#endif
