#pragma once

#include <esp_err.h>

using ccs811_sensor_cb_t = void (*)(uint16_t endpoint_id, float value, void *user_data);
using hyt271_sensor_cb_t = void (*)(uint16_t endpoint_id, float value, void *user_data);
using d6t1a01_sensor_cb_t = void (*)(uint16_t endpoint_id, bool detected, void *user_data);


typedef struct {
    struct {
        ccs811_sensor_cb_t cb = NULL;               // callback für eCO2
        uint16_t endpoint_id;                       // endpoint_id für eCO2 sensor
    } eCO2;

    struct {
        ccs811_sensor_cb_t cb = NULL;               // callback für TVOC
        uint16_t endpoint_id;                       // endpoint_id für TVOC sensor
    } TVOC;

    void *user_data = NULL;                         // user data

    uint32_t interval_ms = 5000;                    // polling interval in milliseconds, defaults to 5000 ms
} ccs811_sensor_config_t;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    struct {
        hyt271_sensor_cb_t cb = NULL;               // callback für humidity
        uint16_t endpoint_id;                       // endpoint_id für humidity sensor
    } humidity;

    struct {
        hyt271_sensor_cb_t cb = NULL;               // callback für temperature
        uint16_t endpoint_id;                       // endpoint_id für temperature sensor
    } temperature;

    void *user_data = NULL;                         // user data

    uint32_t interval_ms = 5000;                    // polling interval in milliseconds, defaults to 5000 ms
} hyt271_sensor_config_t;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    d6t1a01_sensor_cb_t cb = NULL;                  // callback für temperature
    uint16_t endpoint_id;                           // endpoint_id für temperature sensor

    void *user_data = NULL;                         // user data

    uint32_t interval_ms = 5000;                    // polling interval in milliseconds, defaults to 5000 ms
} d6t1a01_sensor_config_t;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize sensor driver. This function should be called only once
 *        When initializing, at least one callback should be provided, else it
 *        returns ESP_ERR_INVALID_ARG.
 *
 * @param config sensor configurations. This should last for the lifetime of the driver
 *               as driver layer do not make a copy of this object.
 *
 * @return esp_err_t - ESP_OK on success,
 *                     ESP_ERR_INVALID_ARG if config is NULL
 *                     ESP_ERR_INVALID_STATE if driver is already initialized
 *                     appropriate error code otherwise
 */
esp_err_t ccs811_sensor_init(ccs811_sensor_config_t *config);
esp_err_t hyt271_sensor_init(hyt271_sensor_config_t *config);
esp_err_t d6t1a01_sensor_init(d6t1a01_sensor_config_t *config);



//eCO2Measurement + TVOCMeasurement
//#pragma once

#include <esp_matter.h>
#include <esp_matter_core.h>

namespace esp_matter
{
    namespace cluster
    {
        // eCO2 Measurement cluster
        // Messung von eCO2 in ppm (parts per million)
        struct eCO2Measurement {
            static constexpr chip::ClusterId Id = 0x040C;  // Choose an appropriate ID
                struct Attributes {
                struct MeasuredValue {
                    static constexpr chip::AttributeId Id = 0x0000;
                    static constexpr uint16_t kMinValue = 0;      // 0 ppm
                    static constexpr uint16_t kMaxValue = 65534;  // 65534 ppm
                    static constexpr uint16_t kInvalidValue = 0xFFFF;
                };
            };
        };

        //TVOC Measurement cluster
        // Messung in TVOC in ppb (parts per billion)
        struct TVOCMeasurement {
            static constexpr chip::ClusterId Id = 0x042E;    //0x040D;  // Choose an appropriate ID
            struct Attributes {
                struct MeasuredValue {
                    static constexpr chip::AttributeId Id = 0x0000;
                    static constexpr uint16_t kMinValue = 0;      // 0 ppb
                    static constexpr uint16_t kMaxValue = 65534;  // 65534 ppb
                    static constexpr uint16_t kInvalidValue = 0xFFFF;
                };
            };
        };
    }
}