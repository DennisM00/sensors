/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <bsp/esp-bsp.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_ota.h>
#include <nvs_flash.h>

#include <app_openthread_config.h>
#include <app_reset.h>
#include <common_macros.h>

#include <drivers/ccs811_hyt271_d6t1a01_i2c.h>

static const char * TAG = "app_main";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;



#include <esp_matter_core.h>
#include <esp_matter_endpoint.h>
#include <esp_matter_cluster.h>
#include <esp_matter_identify.h>
#include <esp_matter_attribute.h>

#define I2C_MASTER_NUM I2C_NUM_0                        // I2C port number for master dev
#define CSS811_SENSOR_ADDR 0x5a                         // I2C address of CSS811 sensor

#define ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_ID 0x0310
#define ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_VERSION 1
#define ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_ID 0x0311
#define ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_VERSION 1


namespace esp_matter {
    namespace endpoint {
        namespace eco2_sensor {

            uint32_t get_device_type_id() {
                return ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_ID;
            }

            uint8_t get_device_type_version() {
                return ESP_MATTER_ECO2_SENSOR_DEVICE_TYPE_VERSION;
            }

            endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data) {
                endpoint_t *endpoint = esp_matter::endpoint::create(node, flags, priv_data);
                if (!endpoint) {
                    return nullptr;
                }
                
                esp_err_t err = add(endpoint, config);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to add eco2 sensor clusters");
                    esp_matter::endpoint::destroy(node,endpoint);
                    return nullptr;
                }
                
                return endpoint;
            }
            
            esp_err_t add(endpoint_t *endpoint, config_t *config) {
                esp_err_t err = esp_matter::endpoint::add_device_type(endpoint, get_device_type_id(), get_device_type_version());
                if (err != ESP_OK) {
                    return err;
                }
            
                // Add identify cluster
                cluster::identify::config_t identify_config;
                identify_config.identify_type = chip::to_underlying(chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator);
                cluster::identify::create(endpoint, &identify_config, CLUSTER_FLAG_SERVER);
            
                // Create CO2 measurement cluster with configuration
                cluster::carbon_dioxide_concentration_measurement::config_t co2_config = {};
                co2_config.measurement_medium = 0x00; // Air measurement medium
                
                cluster_t *cluster = cluster::carbon_dioxide_concentration_measurement::create(endpoint, &co2_config, CLUSTER_FLAG_SERVER);
                if (!cluster) {
                    ESP_LOGE(TAG, "Failed to create CO2 measurement cluster");
                    return ESP_FAIL;
                }
            
                // Add mandatory attributes
                attribute_t *attr = attribute::create(cluster, 
                                                  CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id,
                                                  ATTRIBUTE_FLAG_NULLABLE, 
                                                  esp_matter_nullable_uint16(400));
                if (!attr) {
                    ESP_LOGE(TAG, "Failed to create MeasuredValue attribute");
                    return ESP_FAIL;
                }
            
                // Add min/max value attributes
                attr = attribute::create(cluster,
                                     CarbonDioxideConcentrationMeasurement::Attributes::MinMeasuredValue::Id,
                                     ATTRIBUTE_FLAG_NULLABLE,
                                     esp_matter_nullable_uint16(400));
                if (!attr) {
                    ESP_LOGE(TAG, "Failed to create MinMeasuredValue attribute");
                    return ESP_FAIL;
                }
            
                attr = attribute::create(cluster,
                                     CarbonDioxideConcentrationMeasurement::Attributes::MaxMeasuredValue::Id,
                                     ATTRIBUTE_FLAG_NULLABLE,
                                     esp_matter_nullable_uint16(8192));
                if (!attr) {
                    ESP_LOGE(TAG, "Failed to create MaxMeasuredValue attribute");
                    return ESP_FAIL;
                }
            
                return ESP_OK;
            }
        } // namespace eco2_sensor
        namespace tvoc_sensor {

            uint32_t get_device_type_id() {
                return ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_ID;
            }

            uint8_t get_device_type_version() {
                return ESP_MATTER_TVOC_SENSOR_DEVICE_TYPE_VERSION;
            }

            endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data) {
                endpoint_t *endpoint = esp_matter::endpoint::create(node, flags, priv_data);
                if (!endpoint) {
                    return nullptr;
                }
                
                esp_err_t err = add(endpoint, config);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to add TVOC sensor clusters");
                    esp_matter::endpoint::destroy(node,endpoint);
                    return nullptr;
                }
                
                return endpoint;
            }
            
            esp_err_t add(endpoint_t *endpoint, config_t *config) {
                esp_err_t err = esp_matter::endpoint::add_device_type(endpoint, get_device_type_id(), get_device_type_version());
                if (err != ESP_OK) {
                    return err;
                }
            
                // Add identify cluster
                cluster::identify::config_t identify_config;
                identify_config.identify_type = chip::to_underlying(chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator);
                cluster::identify::create(endpoint, &identify_config, CLUSTER_FLAG_SERVER);
            
                // Create TVOC measurement cluster with configuration
                cluster::total_volatile_organic_compounds_concentration_measurement::config_t tvoc_config = {};
                tvoc_config.measurement_medium = 0x00; // Air measurement medium
                
                cluster_t *cluster = cluster::total_volatile_organic_compounds_concentration_measurement::create(endpoint, &tvoc_config, CLUSTER_FLAG_SERVER);
                if (!cluster) {
                    ESP_LOGE(TAG, "Failed to create TVOC measurement cluster");
                    return ESP_FAIL;
                }
            
                // Add mandatory attributes
                attribute_t *attr = attribute::create(cluster, 
                                                  TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Id,
                                                  ATTRIBUTE_FLAG_NULLABLE, 
                                                  esp_matter_nullable_uint16(400));
                if (!attr) {
                    ESP_LOGE(TAG, "Failed to create MeasuredValue attribute");
                    return ESP_FAIL;
                }
            
                // Add min/max value attributes
                attr = attribute::create(cluster,
                                     TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MinMeasuredValue::Id,
                                     ATTRIBUTE_FLAG_NULLABLE,
                                     esp_matter_nullable_uint16(400));
                if (!attr) {
                    ESP_LOGE(TAG, "Failed to create MinMeasuredValue attribute");
                    return ESP_FAIL;
                }
            
                attr = attribute::create(cluster,
                                     TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MaxMeasuredValue::Id,
                                     ATTRIBUTE_FLAG_NULLABLE,
                                     esp_matter_nullable_uint16(8192));
                if (!attr) {
                    ESP_LOGE(TAG, "Failed to create MaxMeasuredValue attribute");
                    return ESP_FAIL;
                }
            
                return ESP_OK;
            }
        } // namespace tvoc_sensor
    } // namespace endpoint
} // namespace esp_matter





// Application cluster specification, 7.18.2.11. Temperature
// represents a temperature on the Celsius scale with a resolution of 0.01°C.
// temp = (temperature in °C) x 100
static void temp_sensor_notification(uint16_t endpoint_id, float temp, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, temp]() {
        attribute_t * attribute = attribute::get(endpoint_id,
                                                 TemperatureMeasurement::Id,
                                                 TemperatureMeasurement::Attributes::MeasuredValue::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.i16 = static_cast<int16_t>(temp * 100);

        attribute::update(endpoint_id, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id, &val);
    });
}

// Application cluster specification, 2.6.4.1. MeasuredValue Attribute
// represents the humidity in percent.
// humidity = (humidity in %) x 100
static void humidity_sensor_notification(uint16_t endpoint_id, float humidity, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, humidity]() {
        attribute_t * attribute = attribute::get(endpoint_id,
                                                 RelativeHumidityMeasurement::Id,
                                                 RelativeHumidityMeasurement::Attributes::MeasuredValue::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.u16 = static_cast<uint16_t>(humidity * 100);

        attribute::update(endpoint_id, RelativeHumidityMeasurement::Id, RelativeHumidityMeasurement::Attributes::MeasuredValue::Id, &val);
    });
}


static void eCO2_sensor_notification(uint16_t endpoint_id, float eCO2, void *user_data)
{
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, eCO2]() {
        attribute_t * attribute = attribute::get(endpoint_id,
                                               CarbonDioxideConcentrationMeasurement::Id,
                                               CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id);
            
        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.type = ESP_MATTER_VAL_TYPE_UINT16;
        val.val.u16 = static_cast<uint16_t>(eCO2);  // CO2 wird in ppm gemessen, keine Multiplikation nötig

        attribute::update(endpoint_id, 
                         CarbonDioxideConcentrationMeasurement::Id,
                         CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id,
                         &val);
    });
}

static void TVOC_sensor_notification(uint16_t endpoint_id, float TVOC, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, TVOC]() {
        attribute_t * attribute = attribute::get(endpoint_id,
                                                 TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                                                 TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasurementMedium::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.type = ESP_MATTER_VAL_TYPE_UINT16;
        val.val.u16 = static_cast<uint16_t>(TVOC);

        attribute::update(endpoint_id, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasurementMedium::Id, &val);
    });
}

static void occupancy_sensor_notification(uint16_t endpoint_id, bool occupancy, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, occupancy]() {
        attribute_t * attribute = attribute::get(endpoint_id,
                                                 OccupancySensing::Id,
                                                 OccupancySensing::Attributes::Occupancy::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.b = occupancy;

        attribute::update(endpoint_id, OccupancySensing::Id, OccupancySensing::Attributes::Occupancy::Id, &val);
    });
}



static esp_err_t factory_reset_button_register()
{
    button_handle_t push_button;
    esp_err_t err = bsp_iot_button_create(&push_button, NULL, BSP_BUTTON_NUM);
    VerifyOrReturnError(err == ESP_OK, err);
    return app_reset_button_register(push_button);
}

static void open_commissioning_window_if_necessary()
{
    VerifyOrReturn(chip::Server::GetInstance().GetFabricTable().FabricCount() == 0);

    chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
    VerifyOrReturn(commissionMgr.IsCommissioningWindowOpen() == false);

    // After removing last fabric, this example does not remove the Wi-Fi credentials
    // and still has IP connectivity so, only advertising on DNS-SD.
    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(chip::System::Clock::Seconds16(300),
                                    chip::CommissioningWindowAdvertisement::kDnssdOnly);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
    }
}

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        ESP_LOGI(TAG, "Fabric removed successfully");
        open_commissioning_window_if_necessary();
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE deinitialized and memory reclaimed");
        break;

    default:
        break;
    }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

// This callback is called for every attribute update. The callback implementation shall
// handle the desired attributes and return an appropriate error code. If the attribute
// is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    // Since this is just a sensor and we don't expect any writes on our temperature sensor,
    // so, return success.
    return ESP_OK;
}

extern "C" void app_main()
{


    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    /* Initialize push button on the dev-kit to reset the device */
    esp_err_t err = factory_reset_button_register();
    ABORT_APP_ON_FAILURE(ESP_OK == err, ESP_LOGE(TAG, "Failed to initialize reset button, err:%d", err));

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));



    
    // add temperature sensor device
    temperature_sensor::config_t temp_sensor_config;
    endpoint_t * temp_sensor_ep = temperature_sensor::create(node, &temp_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    ABORT_APP_ON_FAILURE(temp_sensor_ep != nullptr, ESP_LOGE(TAG, "Failed to create temperature_sensor endpoint"));

    // add the humidity sensor device
    humidity_sensor::config_t humidity_sensor_config;
    endpoint_t * humidity_sensor_ep = humidity_sensor::create(node, &humidity_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    ABORT_APP_ON_FAILURE(humidity_sensor_ep != nullptr, ESP_LOGE(TAG, "Failed to create humidity_sensor endpoint"));


    // add the eCO2 sensor device
    eco2_sensor::config_t eCO2_sensor_config;
    endpoint_t * eCO2_sensor_ep = eco2_sensor::create(node, &eCO2_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    ABORT_APP_ON_FAILURE(eCO2_sensor_ep != nullptr, ESP_LOGE(TAG, "Failed to create eCO2_sensor endpoint"));

    // add the TVOC sensor device
    tvoc_sensor::config_t TVOC_sensor_config;
    endpoint_t * TVOC_sensor_ep = tvoc_sensor::create(node, &TVOC_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    ABORT_APP_ON_FAILURE(TVOC_sensor_ep != nullptr, ESP_LOGE(TAG, "Failed to create TVOC_sensor endpoint"));

    // add the occupancy sensor device
    occupancy_sensor::config_t occupancy_sensor_config;
    occupancy_sensor_config.occupancy_sensing.occupancy_sensor_type =
        chip::to_underlying(OccupancySensing::OccupancySensorTypeEnum::kPir);           // kPir = infraroter themischer sensor
    occupancy_sensor_config.occupancy_sensing.occupancy_sensor_type_bitmap =
        chip::to_underlying(OccupancySensing::OccupancySensorTypeBitmap::kPir);

    endpoint_t * occupancy_sensor_ep = occupancy_sensor::create(node, &occupancy_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    ABORT_APP_ON_FAILURE(occupancy_sensor_ep != nullptr, ESP_LOGE(TAG, "Failed to create occupancy_sensor endpoint"));

    

    // initialize CO2 and TVOC sensor driver (ccs811)
/*
    write(CSS811_SENSOR_ADDR, 0xF4, 0xAA);            // 
    write(CSS811_SENSOR_ADDR, 0x01, 0x10);            // 0001 0000 = Mode 1 = constant power mode

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CSS811_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xF4, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    cmd = NULL;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CSS811_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x01, true);
    i2c_master_write_byte(cmd, 0x10, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    cmd = NULL;
*/
    static ccs811_sensor_config_t ccs811_config = {
        .eCO2 = {
            .cb = eCO2_sensor_notification,
            .endpoint_id = endpoint::get_id(eCO2_sensor_ep),
        },
        .TVOC = {
            .cb = TVOC_sensor_notification,
            .endpoint_id = endpoint::get_id(TVOC_sensor_ep),
        },
    };
    err = ccs811_sensor_init(&ccs811_config);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize CO2 and TVOC sensor driver"));    


    // initialize temperature and humidity sensor driver (hyt271)
    static hyt271_sensor_config_t hyt271_config = {
        .humidity = {
            .cb = humidity_sensor_notification,
            .endpoint_id = endpoint::get_id(humidity_sensor_ep),
        },
        .temperature = {
            .cb = temp_sensor_notification,
            .endpoint_id = endpoint::get_id(temp_sensor_ep),
        },
    };
    err = hyt271_sensor_init(&hyt271_config);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize temperature and humidity sensor driver"));

    // initialize detection sensor driver (d6t1a01)
    static d6t1a01_sensor_config_t d6t1a01_config = {
        .cb = occupancy_sensor_notification,
        .endpoint_id = endpoint::get_id(occupancy_sensor_ep),
    };
    err = d6t1a01_sensor_init(&d6t1a01_config);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize occupancy sensor driver"));





/*  
    // initialize temperature and humidity sensor driver (shtc3)
    static shtc3_sensor_config_t shtc3_config = {
        .temperature = {
            .cb = temp_sensor_notification,
            .endpoint_id = endpoint::get_id(temp_sensor_ep),
        },
        .humidity = {
            .cb = humidity_sensor_notification,
            .endpoint_id = endpoint::get_id(humidity_sensor_ep),
        },
    };
    err = shtc3_sensor_init(&shtc3_config);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize temperature sensor driver"));


    // initialize occupancy sensor driver (pir)
    static pir_sensor_config_t pir_config = {
        .cb = occupancy_sensor_notification,
        .endpoint_id = endpoint::get_id(occupancy_sensor_ep),
    };
    err = pir_sensor_init(&pir_config);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialize occupancy sensor driver"));
*/


#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));
}