/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ble/DiscoveredCharacteristic.h"
#include "ble/GattClient.h"
#include "ble/OneShotWriteCallback.h"
#include "ble/OneShotReadCallback.h"
#include "ble/CCCDSetter.h"

ble_error_t
DiscoveredCharacteristic::read(uint16_t offset) const
{
    if (!props.read()) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }

    if (!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    return gattc->read(connHandle, valueHandle, offset);
}

ble_error_t DiscoveredCharacteristic::read(uint16_t offset, const GattClient::ReadCallback_t& onRead) const {
    ble_error_t error = read(offset);
    if (error) {
        return error;
    }

    OneShotReadCallback::launch(gattc, connHandle, valueHandle, onRead);

    return error;
}

ble_error_t
DiscoveredCharacteristic::write(uint16_t length, const uint8_t *value) const
{
    if (!props.write()) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }

    if (!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    return gattc->write(GattClient::GATT_OP_WRITE_REQ, connHandle, valueHandle, length, value);
}

ble_error_t
DiscoveredCharacteristic::writeWoResponse(uint16_t length, const uint8_t *value) const
{
    if (!props.writeWoResp()) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }

    if (!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    return gattc->write(GattClient::GATT_OP_WRITE_CMD, connHandle, valueHandle, length, value);
}

ble_error_t DiscoveredCharacteristic::write(uint16_t length, const uint8_t *value, const GattClient::WriteCallback_t& onRead) const {
    ble_error_t error = write(length, value);
    if (error) {
        return error;
    }

    OneShotWriteCallback::launch(gattc, connHandle, valueHandle, onRead);

    return error;
}

ble_error_t DiscoveredCharacteristic::discoverDescriptors(
    const CharacteristicDescriptorDiscovery::DiscoveryCallback_t& onCharacteristicDiscovered,
    const CharacteristicDescriptorDiscovery::TerminationCallback_t& onTermination) const {

    if(!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    ble_error_t err = gattc->discoverCharacteristicDescriptors(
        *this, onCharacteristicDiscovered, onTermination
    );

    return err;
}

ble_error_t DiscoveredCharacteristic::setCCCD(
    bool indicationValue,
    bool notificationValue,
    const GattClient::WriteCallback_t &writeCallback) const {
    if (!props.notify() && notificationValue) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }
    if (!props.indicate() && indicationValue) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }

    /* Configure the internal structure to prepare for descriptor update */
    CCCDSetter::launch(gattc, *this, writeCallback, notificationValue | (indicationValue << 1));

    return BLE_ERROR_NONE;
}
