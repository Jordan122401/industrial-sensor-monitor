#include "DecisionEngine.h"

#include <cassert>

int main() {
    DecisionEngine engine;
    engine.setDistanceThreshold(15.0);

    SensorData safeData;
    safeData.distance = 30.0;
    safeData.orientation = "FLAT";
    assert(engine.evaluate(safeData).statusLabel == "SAFE");

    SensorData obstacleData;
    obstacleData.distance = 10.0;
    obstacleData.orientation = "FLAT";
    assert(engine.evaluate(obstacleData).statusLabel == "OBSTACLE_NEAR");

    SensorData unstableData;
    unstableData.distance = 30.0;
    unstableData.orientation = "LEFT";
    assert(engine.evaluate(unstableData).statusLabel == "UNSTABLE_PLATFORM");

    SensorData avoidanceData;
    avoidanceData.distance = 5.0;
    avoidanceData.orientation = "FORWARD";
    const DecisionResult avoidance = engine.evaluate(avoidanceData);
    assert(avoidance.statusLabel == "AVOIDANCE_TRIGGER");
    assert(avoidance.obstacleDetected);
    assert(avoidance.platformUnstable);

    return 0;
}
