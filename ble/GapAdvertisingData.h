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

#ifndef __GAP_ADVERTISING_DATA_H__
#define __GAP_ADVERTISING_DATA_H__

#include <stdint.h>
#include <string.h>

#include "blecommon.h"
#include "BLEProtocol.h"

#define GAP_ADVERTISING_DATA_MAX_PAYLOAD        (31)

/**************************************************************************/
/*!
    \brief
    This class provides several helper functions to generate properly
    formatted GAP Advertising and Scan Response data payloads.

    \note
    See Bluetooth Specification 4.0 (Vol. 3), Part C, Sections 11 and 18
    for further information on Advertising and Scan Response data.

    \par Advertising and Scan Response Payloads
    Advertising data and Scan Response data are organized around a set of
    data types called 'AD types' in Bluetooth 4.0 (see the Bluetooth Core
    Specification v4.0, Vol. 3, Part C, Sections 11 and 18).

    \par
    Each AD type has its own standardized assigned number, as defined
    by the Bluetooth SIG:
    https://www.bluetooth.org/en-us/specification/assigned-numbers/generic-access-profile

    \par
    For convenience, all appropriate AD types are encapsulated
    in GapAdvertisingData::DataType.

    \par
    Before the AD Types and their payload (if any) can be inserted into
    the Advertising or Scan Response frames, they need to be formatted as
    follows:

    \li \c Record length (1 byte).
    \li \c AD Type (1 byte).
    \li \c AD payload (optional; only present if record length > 1).

    \par
    This class takes care of properly formatting the payload, performs
    some basic checks on the payload length, and tries to avoid common
    errors like adding an exclusive AD field twice in the Advertising
    or Scan Response payload.

    \par EXAMPLE

    \code

    // ToDo

    \endcode
*/
/**************************************************************************/
class GapAdvertisingData
{
public:
    /**********************************************************************/
    /*!
        \brief
        A list of Advertising Data types commonly used by peripherals.
        These AD types are used to describe the capabilities of the
        peripheral, and are inserted inside the advertising or scan
        response payloads.

        \par Source
        \li \c Bluetooth Core Specification 4.0 (Vol. 3), Part C, Section 11, 18
        \li \c https://www.bluetooth.org/en-us/specification/assigned-numbers/generic-access-profile
    */
    /**********************************************************************/
    enum DataType_t {
        FLAGS                              = 0x01, /**< \ref *Flags */
        INCOMPLETE_LIST_16BIT_SERVICE_IDS  = 0x02, /**< Incomplete list of 16-bit Service IDs */
        COMPLETE_LIST_16BIT_SERVICE_IDS    = 0x03, /**< Complete list of 16-bit Service IDs */
        INCOMPLETE_LIST_32BIT_SERVICE_IDS  = 0x04, /**< Incomplete list of 32-bit Service IDs (not relevant for Bluetooth 4.0) */
        COMPLETE_LIST_32BIT_SERVICE_IDS    = 0x05, /**< Complete list of 32-bit Service IDs (not relevant for Bluetooth 4.0) */
        INCOMPLETE_LIST_128BIT_SERVICE_IDS = 0x06, /**< Incomplete list of 128-bit Service IDs */
        COMPLETE_LIST_128BIT_SERVICE_IDS   = 0x07, /**< Complete list of 128-bit Service IDs */
        SHORTENED_LOCAL_NAME               = 0x08, /**< Shortened Local Name */
        COMPLETE_LOCAL_NAME                = 0x09, /**< Complete Local Name */
        TX_POWER_LEVEL                     = 0x0A, /**< TX Power Level (in dBm) */
        DEVICE_ID                          = 0x10, /**< Device ID */
        SLAVE_CONNECTION_INTERVAL_RANGE    = 0x12, /**< Slave Connection Interval Range */
        LIST_128BIT_SOLICITATION_IDS       = 0x15, /**< List of 128 bit service UUIDs the device is looking for */
        SERVICE_DATA                       = 0x16, /**< Service Data */
        APPEARANCE                         = 0x19, /**< \ref Appearance */
        ADVERTISING_INTERVAL               = 0x1A, /**< Advertising Interval */
        MANUFACTURER_SPECIFIC_DATA         = 0xFF  /**< Manufacturer Specific Data */
    };
    typedef enum DataType_t DataType; /* Deprecated type alias. This may be dropped in a future release. */

    /**********************************************************************/
    /*!
        \brief
        A list of values for the FLAGS AD Type.

        \note
        You can use more than one value in the FLAGS AD Type (ex.
        LE_GENERAL_DISCOVERABLE and BREDR_NOT_SUPPORTED).

        \par Source
        \li \c Bluetooth Core Specification 4.0 (Vol. 3), Part C, Section 18.1
    */
    /**********************************************************************/
    enum Flags_t {
        LE_LIMITED_DISCOVERABLE = 0x01, /**< *Peripheral device is discoverable for a limited period of time. */
        LE_GENERAL_DISCOVERABLE = 0x02, /**< Peripheral device is discoverable at any moment. */
        BREDR_NOT_SUPPORTED     = 0x04, /**< Peripheral device is LE only. */
        SIMULTANEOUS_LE_BREDR_C = 0x08, /**< Not relevant - central mode only. */
        SIMULTANEOUS_LE_BREDR_H = 0x10  /**< Not relevant - central mode only. */
    };
    typedef enum Flags_t Flags; /* Deprecated type alias. This may be dropped in a future release. */

    /**
     * Appearance-type for BLEProtocol addresses.
     *
     * @note: deprecated. Use @ref BLEProtocol::AppearanceType_t instead.
     */
    typedef BLEProtocol::AppearanceType_t Appearance_t;

    /**
     * Appearance-type for BLEProtocol addresses.
     *
     * @note: deprecated. Use @ref BLEProtocol::AppearanceType_t instead.
     */
    typedef BLEProtocol::AppearanceType_t Appearance;

    GapAdvertisingData(void) : _payload(), _payloadLen(0), _appearance(BLEProtocol::AppearanceType::GENERIC_TAG) {
        /* empty */
    }

    /**
     * Adds advertising data based on the specified AD type (see DataType).
     *
     * @param  advDataType The Advertising 'DataType' to add.
     * @param  payload     Pointer to the payload contents.
     * @param  len         Size of the payload in bytes.
     *
     * @return BLE_ERROR_BUFFER_OVERFLOW if the specified data would cause the
     * advertising buffer to overflow, else BLE_ERROR_NONE.
     */
    ble_error_t addData(DataType advDataType, const uint8_t *payload, uint8_t len)
    {
        ble_error_t result = BLE_ERROR_BUFFER_OVERFLOW;

        // find field
        uint8_t* field = findField(advDataType);

        // Field type already exist, either add to field or replace
        if (field) {
            switch(advDataType) {
                //  These fields will be overwritten with the new value
                case FLAGS:
                case SHORTENED_LOCAL_NAME:
                case COMPLETE_LOCAL_NAME:
                case TX_POWER_LEVEL:
                case DEVICE_ID:
                case SLAVE_CONNECTION_INTERVAL_RANGE:
                case SERVICE_DATA:
                case APPEARANCE:
                case ADVERTISING_INTERVAL:
                case MANUFACTURER_SPECIFIC_DATA: {
                    // current field length, with the type subtracted
                    uint8_t dataLength = field[0] - 1;

                    // new data has same length, do in-order replacement
                    if (len == dataLength) {
                        for (uint8_t idx = 0; idx < dataLength; idx++) {
                            field[2 + idx] = payload[idx];
                        }
                    } else {
                        // check if data fits
                        if ((_payloadLen - dataLength + len) <= GAP_ADVERTISING_DATA_MAX_PAYLOAD) {

                            // remove old field
                            while ((field + dataLength + 2) < &_payload[_payloadLen]) {
                                *field = field[dataLength + 2];
                                field++;
                            }

                            // reduce length
                            _payloadLen -= dataLength + 2;

                            // add new field
                            result = appendField(advDataType, payload, len);
                        }
                    }

                    break;
                }
                // These fields will have the new data appended if there is sufficient space
                case INCOMPLETE_LIST_16BIT_SERVICE_IDS:
                case COMPLETE_LIST_16BIT_SERVICE_IDS:
                case INCOMPLETE_LIST_32BIT_SERVICE_IDS:
                case COMPLETE_LIST_32BIT_SERVICE_IDS:
                case INCOMPLETE_LIST_128BIT_SERVICE_IDS:
                case COMPLETE_LIST_128BIT_SERVICE_IDS:
                case LIST_128BIT_SOLICITATION_IDS: {
                    // check if data fits
                    if ((_payloadLen + len) <= GAP_ADVERTISING_DATA_MAX_PAYLOAD) {
                        // make room for new field by moving the remainder of the
                        // advertisement payload "to the right" starting after the
                        // TYPE field.
                        uint8_t* end = &_payload[_payloadLen];

                        while (&field[1] < end) {
                            end[len] = *end;
                            end--;
                        }

                        // insert new data
                        for (uint8_t idx = 0; idx < len; idx++) {
                            field[2 + idx] = payload[idx];
                        }

                        // increment lengths
                        field[0] += len;
                        _payloadLen += len;

                        result = BLE_ERROR_NONE;
                    }

                    break;
                }
                // Field exists but updating it is not supported. Abort operation.
                default:
                    result = BLE_ERROR_NOT_IMPLEMENTED;
                    break;
            }
        } else {
            // field doesn't exists, insert new
            result = appendField(advDataType, payload, len);
        }

        return result;
    }

    /**
     * Update a particular ADV field in the advertising payload (based on
     * matching type and length). Note: the length of the new data must be the
     * same as the old one.
     *
     * @param[in] advDataType  The Advertising 'DataType' to add.
     * @param[in] payload      Pointer to the payload contents.
     * @param[in] len          Size of the payload in bytes.
     *
     * @return BLE_ERROR_UNSPECIFIED if the specified field is not found, else
     * BLE_ERROR_NONE.
     */
    ble_error_t updateData(DataType_t advDataType, const uint8_t *payload, uint8_t len)
    {
        if ((payload == NULL) || (len == 0)) {
            return BLE_ERROR_INVALID_PARAM;
        }

        /* A local struct to describe an ADV field. This definition comes from the Bluetooth Core Spec. (v4.2) Part C, Section 11. */
        struct ADVField_t {
            uint8_t  len;      /* Describes the length (in bytes) of the following type and bytes. */
            uint8_t  type;     /* Should have the same representation of DataType_t (above). */
            uint8_t  bytes[0]; /* A placeholder for variable length data. */
        };

        /* Iterate over the adv fields looking for the first match. */
        uint8_t byteIndex = 0;
        while (byteIndex < _payloadLen) {
            ADVField_t *currentADV = (ADVField_t *)&_payload[byteIndex];
            if ((currentADV->len  == (len + 1)) && /* Incoming len only describes the payload, whereas ADV->len describes [type + payload]. */
                (currentADV->type == advDataType)) {
                memcpy(currentADV->bytes, payload, len);
                return BLE_ERROR_NONE;
            }

            byteIndex += (currentADV->len + 1); /* Advance by len+1; '+1' is needed to span the len field itself. */
        }

        return BLE_ERROR_UNSPECIFIED;
    }

    /**
     * Helper function to add APPEARANCE data to the advertising payload.
     *
     * @param  appearance
     *           The APPEARANCE value to add.
     *
     * @return BLE_ERROR_BUFFER_OVERFLOW if the specified data would cause the
     * advertising buffer to overflow, else BLE_ERROR_NONE.
     */
    ble_error_t addAppearance(BLEProtocol::AppearanceType_t appearance = BLEProtocol::AppearanceType::GENERIC_TAG) {
        _appearance = appearance;
        return addData(GapAdvertisingData::APPEARANCE, (uint8_t *)&appearance, 2);
    }

    /**
     * Helper function to add FLAGS data to the advertising payload.
     * @param  flags
     *           LE_LIMITED_DISCOVERABLE
     *             The peripheral is discoverable for a limited period of time.
     *           LE_GENERAL_DISCOVERABLE
     *             The peripheral is permanently discoverable.
     *           BREDR_NOT_SUPPORTED
     *             This peripheral is a Bluetooth Low Energy only device (no EDR support).
     *
     * @return BLE_ERROR_BUFFER_OVERFLOW if the specified data would cause the
     * advertising buffer to overflow, else BLE_ERROR_NONE.
     */
    ble_error_t addFlags(uint8_t flags = LE_GENERAL_DISCOVERABLE) {
        return addData(GapAdvertisingData::FLAGS, &flags, 1);
    }

    /**
     * Helper function to add TX_POWER_LEVEL data to the advertising payload.
     *
     * @return BLE_ERROR_BUFFER_OVERFLOW if the specified data would cause the
     * advertising buffer to overflow, else BLE_ERROR_NONE.
     */
    ble_error_t addTxPower(int8_t txPower) {
        /* To Do: Basic error checking to make sure txPower is in range. */
        return addData(GapAdvertisingData::TX_POWER_LEVEL, (uint8_t *)&txPower, 1);
    }

    /**
     * Clears the payload and resets the payload length counter.
     */
    void        clear(void) {
        memset(&_payload, 0, GAP_ADVERTISING_DATA_MAX_PAYLOAD);
        _payloadLen = 0;
    }

    /**
     * Returns a pointer to the current payload.
     */
    const uint8_t *getPayload(void) const {
        return _payload;
    }

    /**
     * Returns the current payload length (0..31 bytes).
     */
    uint8_t     getPayloadLen(void) const {
        return _payloadLen;
    }

    /**
     * Returns the 16-bit appearance value for this device.
     */
    uint16_t    getAppearance(void) const {
        return (uint16_t)_appearance;
    }

    /**
     * Search advertisement data for field.
     * Returns pointer to the first element in the field if found, NULL otherwise.
     * Where the first element is the length of the field.
     */
    const uint8_t* findField(DataType_t type) const {
        return findField(type);
    }

private:
    /**
     * Append advertising data based on the specified AD type (see DataType)
     */
    ble_error_t appendField(DataType advDataType, const uint8_t *payload, uint8_t len)
    {
        /* Make sure we don't exceed the 31 byte payload limit */
        if (_payloadLen + len + 2 > GAP_ADVERTISING_DATA_MAX_PAYLOAD) {
            return BLE_ERROR_BUFFER_OVERFLOW;
        }

        /* Field length. */
        memset(&_payload[_payloadLen], len + 1, 1);
        _payloadLen++;

        /* Field ID. */
        memset(&_payload[_payloadLen], (uint8_t)advDataType, 1);
        _payloadLen++;

        /* Payload. */
        memcpy(&_payload[_payloadLen], payload, len);
        _payloadLen += len;

        return BLE_ERROR_NONE;
    }

    /**
     * Search advertisement data for field.
     * Returns pointer to the first element in the field if found, NULL otherwise.
     * Where the first element is the length of the field.
     */
    uint8_t* findField(DataType_t type) {
        // scan through advertisement data
        for (uint8_t idx = 0; idx < _payloadLen; ) {
            uint8_t fieldType = _payload[idx + 1];

            if (fieldType == type) {
                return &_payload[idx];
            }

            // advance to next field
            idx += _payload[idx] + 1;
        }

        // field not found
        return NULL;
    }

    uint8_t  _payload[GAP_ADVERTISING_DATA_MAX_PAYLOAD];
    uint8_t  _payloadLen;
    uint16_t _appearance;
};

#endif // ifndef __GAP_ADVERTISING_DATA_H__
