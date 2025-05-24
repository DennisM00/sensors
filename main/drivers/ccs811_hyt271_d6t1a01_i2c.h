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
 