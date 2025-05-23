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

// Initialisierungsfunktionen für die Sensoren
/**
 * @brief Initialisiert den CCS811-Sensor (eCO2 und TVOC).
 *
 * @param config Konfiguration des Sensors. Diese muss für die Lebensdauer des Treibers bestehen bleiben.
 * @return esp_err_t - ESP_OK bei Erfolg, ESP_ERR_INVALID_ARG bei ungültigen Argumenten,
 *                     ESP_ERR_INVALID_STATE, wenn der Treiber bereits initialisiert ist.
 */
 esp_err_t ccs811_sensor_init(ccs811_sensor_config_t *config);

 /**
  * @brief Initialisiert den HYT271-Sensor (Temperatur und Luftfeuchtigkeit).
  *
  * @param config Konfiguration des Sensors. Diese muss für die Lebensdauer des Treibers bestehen bleiben.
  * @return esp_err_t - ESP_OK bei Erfolg, ESP_ERR_INVALID_ARG bei ungültigen Argumenten,
  *                     ESP_ERR_INVALID_STATE, wenn der Treiber bereits initialisiert ist.
  */
 esp_err_t hyt271_sensor_init(hyt271_sensor_config_t *config);
 
 /**
  * @brief Initialisiert den D6T1A01-Sensor (Bewegungserkennung).
  *
  * @param config Konfiguration des Sensors. Diese muss für die Lebensdauer des Treibers bestehen bleiben.
  * @return esp_err_t - ESP_OK bei Erfolg, ESP_ERR_INVALID_ARG bei ungültigen Argumenten,
  *                     ESP_ERR_INVALID_STATE, wenn der Treiber bereits initialisiert ist.
  */
 esp_err_t d6t1a01_sensor_init(d6t1a01_sensor_config_t *config);
 
 esp_err_t write(uint8_t ADDR, uint8_t reg, uint8_t value);

 // Matter-spezifische Includes
 #include <esp_matter.h>
 #include <esp_matter_endpoint.h>
 #include <esp_matter_cluster.h>
 #include <esp_matter_core.h>
 #include <esp_matter_identify.h>
 /*
 // Namespace für Matter-Endpunkte
 namespace esp_matter {
     namespace endpoint {
         namespace eco2_sensor {
             typedef struct config : app_base_config {
                 cluster::carbon_dioxide_concentration_measurement::config_t carbon_dioxide_concentration_measurement;
             } config_t;
 
             uint32_t get_device_type_id();
             uint8_t get_device_type_version();
             endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data);
             esp_err_t add(endpoint_t *endpoint, config_t *config);
         } // namespace eco2_sensor
 
         namespace tvoc_sensor {
             typedef struct config : app_base_config {
                 cluster::total_volatile_organic_compounds_concentration_measurement::config_t total_volatile_organic_compounds_concentration_measurement;
             } config_t;
 
             uint32_t get_device_type_id();
             uint8_t get_device_type_version();
             endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data);
             esp_err_t add(endpoint_t *endpoint, config_t *config);
         } // namespace tvoc_sensor
     } // namespace endpoint
 } // namespace esp_matter
*/




/*
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


namespace esp_matter {
    namespace endpoint {
        namespace eco2_sensor {
            esp_err_t add(endpoint_t *endpoint, config_t *config)
            {
                esp_err_t err = add_device_type(endpoint, get_device_type_id(), get_device_type_version());
                VerifyOrReturnError(err == ESP_OK, err);

                identify::config_t identify_config;
                identify_config.identify_type = chip::to_underlying(Identify::IdentifyTypeEnum::kVisibleIndicator);
                identify::create(endpoint, &identify_config, CLUSTER_FLAG_SERVER);

                carbon_dioxide_concentration_measurement::create(endpoint, &(config->carbon_dioxide_concentration_measurement), CLUSTER_FLAG_SERVER);

                return ESP_OK;
            }
        } // namespace eco2_sensor
    } // namespace endpoint
} // namespace esp_matter
*/


/*
namespace esp_matter {
    namespace endpoint {
        namespace eco2_sensor {
            typedef struct config : app_base_config {
                cluster::carbon_dioxide_concentration_measurement::config_t carbon_dioxide_concentration_measurement;
            } config_t;
            
            uint32_t get_device_type_id();
            uint8_t get_device_type_version();
            endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data);
            esp_err_t add(endpoint_t *endpoint, config_t *config);
            } // eCO2_sensor 


        namespace tvoc_sensor {
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
*/



/*
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