#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#ifdef MOCK_PICO_PI
#include "../test/bitchat_repeater_mocks.h"
#else
#include "bluetooth.h"
#include "ble/gatt_client.h"
#endif

const uint8_t bitchat_service_uuid[] = {
    0xF4, 0x7B, 0x5E, 0x2D, 0x4A, 0x9E, 0x4C, 0x5A, 0x9B, 0x3F, 0x8E, 0x1D, 0x2C, 0x3A, 0x4B, 0x5C,
};
const uint8_t bitchat_service_uuid_reversed[] = {
    0x5C, 0x4B, 0x3A, 0x2C, 0x1D, 0x8E, 0x3F, 0x9B, 0x5A, 0x4C, 0x9E, 0x4A, 0x2D, 0x5E, 0x7B, 0xF4
};
const auto bitchat_service_name = "Repeater";
const uint8_t bitchat_characteristic_uuid[] = {
    0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x4A, 0x5B, 0x8C, 0x9D, 0x0E, 0x1F, 0x2A, 0x3B, 0x4C, 0x5D
};

enum service_uuid_check_status {
    ServiceUUIDNotFound = 0,
    ServiceUUIDFound = 0b1,
    ServiceUUIDAndNameFound = 0b11
};

class BleConnection {
public:
    void setConnectionHandle(uint16_t hci_con_handle);

    void setNotificationEnabled(bool cond);

    [[nodiscard]] bool getNotificationEnabled() const;

    void setBitchatCharacteristicValueHandle(int handle);

    void setConnected(bool cond);

    void setBleAddress(const bd_addr_t &addr, bd_addr_type_t addr_type);

    [[nodiscard]] hci_con_handle_t getConnectionHandle() const;

    [[nodiscard]] bool hasData() const;

    [[nodiscard]] uint16_t getBitchatCharacteristicValueHandle() const;

    void setHasData(bool);

    void setServices(service_uuid_check_status services);

    void setRssi(int8_t scan_rssi);

    void setTimestamp(uint64_t value);

    [[nodiscard]] const bd_addr_t &getAddress() const;

    [[nodiscard]] bd_addr_type_t getAddressType() const;

    [[nodiscard]] bool isConnected() const;

    void setMtu(uint16_t mtu);

    bool canAndNeedToDiscoverBitchatCharacteristicsQuery(gatt_client_service_t &service) const;

    void storeHandlesIfServiceMatches(const gatt_client_service_t &service);

    void storeHandlesIfCharacteristicMatches(const gatt_client_characteristic_t &characteristic);

    [[nodiscard]] bool isRepeater() const;

    [[nodiscard]] bool isRandom() const;

    [[nodiscard]] uint64_t getTimestamp() const;

    [[nodiscard]] uint64_t getTimestampMs() const;

    [[nodiscard]] uint16_t getMtu() const;

    void setRole(uint8_t role);

    [[nodiscard]] uint8_t getRole() const;

    gatt_client_notification_t *getNotificationListener();

private:
    hci_con_handle_t connection_handle = 0;
    bd_addr_t bt_address{};
    bd_addr_type_t bt_address_type = BD_ADDR_TYPE_UNKNOWN;
    uint16_t bt_mtu = 0;
    uint16_t bitchat_service_start_group_handle = 0;
    uint16_t bitchat_service_end_group_handle = 0;
    uint16_t bitchat_characteristic_value_handle = 0;
    int notification_enabled = 0;
    bool has_data_to_send = false;
    bool connected = false;
    service_uuid_check_status services_found = ServiceUUIDFound;
    int8_t rssi = 0;
    uint8_t role = HCI_ROLE_INVALID;
    uint64_t last_seen_time = 0;
    gatt_client_notification_t notification_listener{};
};
