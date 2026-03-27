#include "BluetoothManager.h"
#include "Logger.h"

#include <gio/gio.h>
#include <string>

BluetoothManager::BluetoothManager()
    : connection_(nullptr),
      characteristicPath_("/org/bluez/hci0/dev_88_13_BF_04_1F_7A/service0028/char0029") {}

BluetoothManager::~BluetoothManager() {
    if (connection_ != nullptr) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
}

bool BluetoothManager::initialize() {
    GError* error = nullptr;

    connection_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);

    if (error != nullptr) {
        Logger::error("Failed to connect to system D-Bus: " + std::string(error->message));
        g_error_free(error);
        return false;
    }

    Logger::info("Connected to system D-Bus");
    Logger::info("Using BLE characteristic path: " + characteristicPath_);

    return true;
}

std::string BluetoothManager::getLatestRawData() {
    if (connection_ == nullptr) {
        Logger::error("BluetoothManager not initialized");
        return "";
    }

    GError* error = nullptr;

    GVariantBuilder optionsBuilder;
    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE("a{sv}"));

    GVariant* result = g_dbus_connection_call_sync(
        connection_,
        "org.bluez",
        characteristicPath_.c_str(),
        "org.bluez.GattCharacteristic1",
        "ReadValue",
        g_variant_new("(a{sv})", &optionsBuilder),
        G_VARIANT_TYPE("(ay)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error
    );

    if (error != nullptr) {
        Logger::error("ReadValue failed: " + std::string(error->message));
        g_error_free(error);
        return "";
    }

    GVariant* byteArrayVariant = nullptr;
    g_variant_get(result, "(@ay)", &byteArrayVariant);

    gsize length = 0;
    const guint8* bytes = static_cast<const guint8*>(
        g_variant_get_fixed_array(byteArrayVariant, &length, sizeof(guint8))
    );

    std::string value;
    if (bytes != nullptr && length > 0) {
        value.assign(reinterpret_cast<const char*>(bytes), length);
    }

    g_variant_unref(byteArrayVariant);
    g_variant_unref(result);

    return value;
}
