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

#include <esp_matter.h>
#include <esp_matter_endpoint.h>
#include <esp_matter_cluster.h>
#include <esp_matter_core.h>
#include <esp_matter_identify.h>

#define ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_ID 0x0310
#define ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_VERSION 1
#define ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_ID 0x0311
#define ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_VERSION 1

/*
namespace esp_matter {
    namespace endpoint {
        namespace eCO2_sensor {
            typedef struct config : app_base_config {
                cluster::carbon_dioxide_concentration_measurement::config_t carbon_dioxide_concentration_measurement;
            } config_t;
            
            uint32_t get_device_type_id();
            uint8_t get_device_type_version();
            endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data);
            esp_err_t add(endpoint_t *endpoint, config_t *config);
            } // eCO2_sensor 


        namespace TVOC_sensor {
            typedef struct config : app_base_config {
                cluster::total_volatile_organic_compounds_concentration_measurement::config_t total_volatile_organic_compounds_concentration_measurement;
            } config_t;

            uint32_t get_device_type_id();
            uint8_t get_device_type_version();
            endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data);
            esp_err_t add(endpoint_t *endpoint, config_t *config);
        } // TVOC_sensor
    } // endpoint
} // esp_matter





namespace eCO2_sensor {
uint32_t get_device_type_id()
{
    return ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_ID;
}

uint8_t get_device_type_version()
{
    return ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_VERSION;
}

endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data)
{
    return common::create<config_t>(node, config, flags, priv_data, add);
}

esp_err_t add(endpoint_t *endpoint, config_t *config)
{
    esp_err_t err = add_device_type(endpoint, get_device_type_id(), get_device_type_version());
    VerifyOrReturnError(err == ESP_OK, err);

    identify::config_t identify_config;
    identify_config.identify_type = chip::to_underlying(Identify::IdentifyTypeEnum::kVisibleIndicator);
    identify::create(endpoint, &identify_config, CLUSTER_FLAG_SERVER);
    carbon_dioxide_concentration_measurement::create(endpoint, &(config->carbon_dioxide_concentration_measurement), CLUSTER_FLAG_SERVER);     /////////////

    return ESP_OK;
}
} // eCO2_sensor




namespace TVOC_sensor {
uint32_t get_device_type_id()
{
    return ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_ID;
}

uint8_t get_device_type_version()
{
    return ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_VERSION;
}

endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data)
{
    return common::create<config_t>(node, config, flags, priv_data, add);
}

esp_err_t add(endpoint_t *endpoint, config_t *config)
{
    esp_err_t err = add_device_type(endpoint, get_device_type_id(), get_device_type_version());
    VerifyOrReturnError(err == ESP_OK, err);

    identify::config_t identify_config;
    identify_config.identify_type = chip::to_underlying(Identify::IdentifyTypeEnum::kVisibleIndicator);
    identify::create(endpoint, &identify_config, CLUSTER_FLAG_SERVER);
    total_volatile_organic_compounds_concentration_measurement::create(endpoint, &(config->total_volatile_organic_compounds_concentration_measurement), CLUSTER_FLAG_SERVER);     ////////////

    return ESP_OK;
}
} // TVOC_sensor 
*/