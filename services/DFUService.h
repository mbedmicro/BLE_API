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

#ifndef __BLE_DFU_SERVICE_H__
#define __BLE_DFU_SERVICE_H__

#include "BLEDevice.h"
#include "UUID.h"

extern "C" void bootloader_start(void);

extern const uint8_t  DFUServiceBaseUUID[];
extern const uint16_t DFUServiceShortUUID;
extern const uint16_t DFUServiceControlCharacteristicShortUUID;

extern const uint8_t  DFUServiceUUID[];
extern const uint8_t  DFUServiceControlCharacteristicUUID[];

class DFUService {
public:
    /**
     * Signature for the handover callback. The application may provide such a
     * callback when setting up the DFU service, in which case it will be
     * invoked before handing control over to the Bootloader.
     */
    typedef void (*ResetPrepare_t)(void);

public:
    DFUService(BLEDevice &_ble, ResetPrepare_t _handoverCallback = NULL) :
        ble(_ble),
        controlBytes(),
        controlPoint(DFUServiceControlCharacteristicUUID, controlBytes, SIZEOF_CONTROL_BYTES, SIZEOF_CONTROL_BYTES,
                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        static bool serviceAdded = false; /* We should only ever need to add the DFU service once. */
        if (serviceAdded) {
            return;
        }

        GattCharacteristic *dfuChars[] = {&controlPoint};
        GattService         dfuService(DFUServiceUUID, dfuChars, sizeof(dfuChars) / sizeof(GattCharacteristic *));

        ble.addService(dfuService);
        handoverCallback = _handoverCallback;
        serviceAdded = true;

        ble.onDataWritten(this, &DFUService::onDataWritten);
    }

    uint16_t getControlHandle(void) {
        return controlPoint.getValueAttribute().getHandle();
    }

    /**
     * This callback allows the DFU service to receive the initial trigger to
     * handover control to the bootloader; but first the application is given a
     * chance to clean up.
     */
    virtual void onDataWritten(const GattCharacteristicWriteCBParams *params) {
        if (params->charHandle == controlPoint.getValueAttribute().getHandle()) {
            if (handoverCallback) {
                handoverCallback();
            }

            bootloader_start();
        }
    }

private:
    static const unsigned SIZEOF_CONTROL_BYTES = 2;

    static ResetPrepare_t  handoverCallback;  /**< application specific handover callback. */

private:
    BLEDevice          &ble;
    uint8_t             controlBytes[SIZEOF_CONTROL_BYTES];
    GattCharacteristic  controlPoint;
};

#endif /* #ifndef __BLE_DFU_SERVICE_H__*/
