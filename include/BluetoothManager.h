#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <string>

typedef struct _GDBusConnection GDBusConnection;

class BluetoothManager {
public:
    BluetoothManager();
    ~BluetoothManager();

    bool initialize();
    std::string getLatestRawData();

private:
    GDBusConnection* connection_;
    std::string characteristicPath_;
};

#endif
