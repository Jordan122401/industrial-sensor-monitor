#include "SensorParser.h"

#include <cassert>

int main() {
    SensorParser parser;
    SensorData data;

    const bool parsed = parser.parse(
        "TEMP=22.9,HUM=62.5,DIST=7.6,ACCEL_X=-1440,ACCEL_Y=648,ACCEL_Z=17756,ORIENT=FLAT",
        data
    );

    assert(parsed);
    assert(data.temperature == 22.9);
    assert(data.humidity == 62.5);
    assert(data.distance == 7.6);
    assert(data.accelX == -1440);
    assert(data.accelY == 648);
    assert(data.accelZ == 17756);
    assert(data.orientation == "FLAT");

    SensorData invalidData;
    const bool invalidParsed = parser.parse(
        "TEMP=22.9,HUM=62.5,DIST=7.6,ACCEL_X=-1440,ACCEL_Y=648,ORIENT=FLAT",
        invalidData
    );

    assert(!invalidParsed);
    return 0;
}
