cmake_minimum_required(VERSION 3.16)

project(IndustrialSensorMonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)

include_directories(include)

add_executable(sensor_monitor
    src/main.cpp
    src/BluetoothManager.cpp
    src/SensorParser.cpp
    src/DatabaseManager.cpp
    src/SocketServer.cpp
    src/Logger.cpp
)

target_link_libraries(sensor_monitor
    PRIVATE
    PkgConfig::GIO
    Threads::Threads
    sqlite3
)