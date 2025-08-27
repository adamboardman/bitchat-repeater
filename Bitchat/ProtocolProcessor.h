#pragma once

#include <vector>

#include "Message.h"
#include "../BLE/BleConnection.h"
#include "../BLE/BleConnectionTracker.h"

class ProtocolProcessor {
public:
    explicit ProtocolProcessor(BleConnectionTracker &ble_connection_tracker)
        : ble_connection_tracker(ble_connection_tracker) {
    }

    static const char *stringForType(uint8_t type);

    void updateOrStorePeerName(uint64_t sender, const std::string &peer_name, uint8_t ttl, BleConnection &connection) const;

    void updateOrStorePeerNoisePublicKey(uint64_t sender, const std::vector<uint8_t> &public_key) const;

    bool processMessage(Message &message, const uint8_t *payload, uint16_t payload_length) const;

    void processWrite(BleConnection &connection, uint16_t offset, const uint8_t *buffer, uint16_t buffer_size) const;

private:
    BleConnectionTracker &ble_connection_tracker;
};
