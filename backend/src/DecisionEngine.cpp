#include "DecisionEngine.h"

DecisionEngine::DecisionEngine()
    : distanceThreshold_(15.0) {}

void DecisionEngine::setDistanceThreshold(double threshold) {
    distanceThreshold_ = threshold;
}

DecisionResult DecisionEngine::evaluate(const SensorData& data) {
    DecisionResult result;
    result.distanceAtDecision = data.distance;
    result.orientationAtDecision = data.orientation;

    result.obstacleDetected = (data.distance > 0.0 && data.distance < distanceThreshold_);
    result.platformUnstable = (data.orientation != "FLAT");

    if (result.obstacleDetected && result.platformUnstable) {
        result.status = SystemStatus::AVOIDANCE_TRIGGER;
    } else if (result.obstacleDetected) {
        result.status = SystemStatus::OBSTACLE_NEAR;
    } else if (result.platformUnstable) {
        result.status = SystemStatus::UNSTABLE_PLATFORM;
    } else {
        result.status = SystemStatus::SAFE;
    }

    result.statusLabel = statusToString(result.status);
    return result;
}

std::string DecisionEngine::statusToString(SystemStatus status) {
    switch (status) {
        case SystemStatus::SAFE:               return "SAFE";
        case SystemStatus::OBSTACLE_NEAR:      return "OBSTACLE_NEAR";
        case SystemStatus::UNSTABLE_PLATFORM:  return "UNSTABLE_PLATFORM";
        case SystemStatus::AVOIDANCE_TRIGGER:  return "AVOIDANCE_TRIGGER";
        default:                               return "UNKNOWN";
    }
}
