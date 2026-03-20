#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include "SensorData.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

class SocketServer {
public:
    SocketServer();
    ~SocketServer();

    bool start(int port = 9000);
    void updateLatestData(const SensorData& data);

private:
    void runServer();
    std::string formatLatestData();

    int serverFd_;
    int port_;
    std::thread serverThread_;
    std::mutex dataMutex_;
    SensorData latestData_;
    bool hasData_;
    std::atomic<bool> running_;
};

#endif
