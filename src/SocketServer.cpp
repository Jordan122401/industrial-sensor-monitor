#include "SocketServer.h"
#include "Logger.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iomanip>

SocketServer::SocketServer()
    : serverFd_(-1), port_(9000), hasData_(false), running_(false) {}

SocketServer::~SocketServer() {
    running_ = false;

    if (serverFd_ != -1) {
        close(serverFd_);
        serverFd_ = -1;
    }

    if (serverThread_.joinable()) {
        serverThread_.join();
    }
}

bool SocketServer::start(int port) {
    port_ = port;
    running_ = true;

    serverThread_ = std::thread(&SocketServer::runServer, this);
    Logger::info("Socket server started on port " + std::to_string(port_));

    return true;
}

void SocketServer::updateLatestData(const SensorData& data) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    latestData_ = data;
    hasData_ = true;
}

std::string SocketServer::formatLatestData() {
    std::lock_guard<std::mutex> lock(dataMutex_);

    if (!hasData_) {
        return "NO_DATA\n";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "TEMP=" << latestData_.temperature
        << ",HUM=" << latestData_.humidity
        << ",DIST=" << latestData_.distance;

    oss << std::fixed << std::setprecision(2);
    oss << ",MOTION=" << latestData_.motion << "\n";

    return oss.str();
}

void SocketServer::runServer() {
    serverFd_ = socket(AF_INET, SOCK_STREAM, 0);

    if (serverFd_ < 0) {
        Logger::error("Failed to create socket");
        return;
    }

    int opt = 1;
    setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_);

    if (bind(serverFd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        Logger::error("Failed to bind socket to port " + std::to_string(port_));
        close(serverFd_);
        serverFd_ = -1;
        return;
    }

    if (listen(serverFd_, 5) < 0) {
        Logger::error("Failed to listen on socket");
        close(serverFd_);
        serverFd_ = -1;
        return;
    }

    Logger::info("Socket server is listening for clients...");

    while (running_) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int clientFd = accept(serverFd_, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientFd < 0) {
            if (running_) {
                Logger::error("Failed to accept client connection");
            }
            continue;
        }

        char buffer[1024] = {0};
        ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);

        if (bytesRead > 0) {
            std::string request(buffer);

            if (request.find("GET_DATA") != std::string::npos) {
                std::string response = formatLatestData();
                send(clientFd, response.c_str(), response.size(), 0);
            } else {
                std::string response = "INVALID_REQUEST\n";
                send(clientFd, response.c_str(), response.size(), 0);
            }
        }

        close(clientFd);
    }
}
