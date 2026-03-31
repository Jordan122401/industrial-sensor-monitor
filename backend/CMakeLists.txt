cmake_minimum_required(VERSION 3.16)

project(AutonomousSensorPlatform VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)

include_directories(include)

add_executable(sensor_platform
    src/main.cpp
    src/BluetoothManager.cpp
    src/SensorParser.cpp
    src/FakeSensorSource.cpp
    src/DecisionEngine.cpp
    src/DatabaseManager.cpp
    src/SocketServer.cpp
    src/Logger.cpp
)

target_link_libraries(sensor_platform
    PRIVATE
    PkgConfig::GIO
    Threads::Threads
    sqlite3
)

if(BUILD_TESTING)
    add_executable(test_sensor_parser
        tests/test_sensor_parser.cpp
        src/SensorParser.cpp
    )
    target_link_libraries(test_sensor_parser PRIVATE Threads::Threads)
    add_test(NAME test_sensor_parser COMMAND test_sensor_parser)

    add_executable(test_decision_engine
        tests/test_decision_engine.cpp
        src/DecisionEngine.cpp
    )
    target_link_libraries(test_decision_engine PRIVATE Threads::Threads)
    add_test(NAME test_decision_engine COMMAND test_decision_engine)
endif()
