#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_random.h>
#include <driver/i2c.h>

#include <lib/support/CodeUtils.h>

#include <drivers/ccs811_hyt271_d6t1a01_i2c.h>

static const char * TAG = "i2c";

#define test

#define I2C_MASTER_SCL_IO 19//I2C_SCL_PIN      // = GPIO_NUM_19 Kconfig
#define I2C_MASTER_SDA_IO 18//I2C_SDA_PIN      // = GPIO_NUM_18 Kconfig
#define I2C_MASTER_NUM I2C_NUM_0                        // I2C port number for master dev
#define I2C_MASTER_FREQ_HZ 100000                       // I2C master clock frequency

#define CSS811_SENSOR_ADDR 0x5a                         // I2C address of CSS811 sensor
#define HYT271_SENSOR_ADDR 0x28                         // I2C address of HYT271 sensor
#define D6TA101_SENSOR_ADDR 0x0a                        // I2C address of D6TA101 sensor


typedef struct {
    ccs811_sensor_config_t *config;
    esp_timer_handle_t timer;
    bool is_initialized = false;
} ccs811_sensor_ctx_t;
typedef struct {
    hyt271_sensor_config_t *config;
    esp_timer_handle_t timer;
    bool is_initialized = false;
} hyt271_sensor_ctx_t;
typedef struct {
    d6t1a01_sensor_config_t *config;
    esp_timer_handle_t timer;
    bool is_initialized = false;
} d6t1a01_sensor_ctx_t;

static ccs811_sensor_ctx_t ccs811_ctx;
static hyt271_sensor_ctx_t hyt271_ctx;
static d6t1a01_sensor_ctx_t d6t1a01_ctx;

static esp_err_t init_i2c()
{
    i2c_driver_delete(I2C_MASTER_NUM);                                  // Deinstalliere den I2C-Treiber, falls er bereits installiert ist

    i2c_config_t conf = {                                               // I2C master initialisierung                     
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = I2C_MASTER_FREQ_HZ,
        },
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C driver, err:%d", err);
        return err;
    }

    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver, err:%d", err);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

esp_err_t write(uint8_t ADDR, uint8_t reg, uint8_t value){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADDR << 1) | I2C_MASTER_WRITE, true);
    if (reg != 0xAA) {
        i2c_master_write_byte(cmd, reg, true);
    }
    if (value != 0xAA) {
        i2c_master_write_byte(cmd, value, true);
    }
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    cmd = NULL;
    return ESP_OK;
}

esp_err_t read(uint8_t *data, size_t size, uint8_t ADDR){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    cmd = NULL;
    return ESP_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static esp_err_t ccs811_convert(int & eCO2, int & TVOC)
{
    uint8_t data[4] = {0};
    
    esp_err_t err = write(CSS811_SENSOR_ADDR, 0x02, 0xAA);
    if (err != ESP_OK) {
        return err;
    }

    err = read(data, sizeof(data), CSS811_SENSOR_ADDR);
    if (err != ESP_OK) {
        return err;
    }

    #ifdef test
    printf("\nCCS811:\n");
    for (int i = 0; i < sizeof(data); i++) {
        printf("\nByte %d: ", i);
        printf("0x%02x ", data[i]);                                                          // in hexa
    }
    #endif

     eCO2 = ((uint16_t)data[0] << 8) | ((uint16_t) data[1]);
     TVOC = ((uint16_t)data[2] << 8) | ((uint16_t) data[3]);

    #ifdef test
    printf("\neCO2: %d ppm, TVOC: %d ppb\n", eCO2, TVOC);
    #endif

    return ESP_OK;
}

static void ccs811_timer_cb_internal(void *arg)                                                                     // ??????????????????????
{
    auto *ctx = (ccs811_sensor_ctx_t *) arg;
    if (!(ctx && ctx->config)) {
        return;
    }

    int eCO2, TVOC;
    esp_err_t err = ccs811_convert(eCO2, TVOC);
    if (err != ESP_OK) {
        return;
    }
    if (ctx->config->eCO2.cb) {
        ctx->config->eCO2.cb(ctx->config->eCO2.endpoint_id, eCO2, ctx->config->user_data);
    }
    if (ctx->config->TVOC.cb) {
        ctx->config->TVOC.cb(ctx->config->TVOC.endpoint_id, TVOC, ctx->config->user_data);
    }
}

esp_err_t ccs811_sensor_init(ccs811_sensor_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->eCO2.cb == NULL || config->TVOC.cb == NULL) {           // min. ein callback benötigt
        return ESP_ERR_INVALID_ARG;
    }
    if (ccs811_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = init_i2c();
    if (err != ESP_OK) {
        return err;
    }

    // Initialisierungssequenz für CCS811
    err = write(CSS811_SENSOR_ADDR, 0xF4, 0xAA);  // App start
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start CCS811 app");
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Warte 100ms nach App start

    err = write(CSS811_SENSOR_ADDR, 0x01, 0x10);  // Set Mode 1 (1 sec measurement)
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set CCS811 measurement mode");
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Warte auf Moduswechsel

    ccs811_ctx.config = config;

    esp_timer_create_args_t args = {
        .callback = ccs811_timer_cb_internal,
        .arg = &ccs811_ctx,
    };

    err = esp_timer_create(&args, &ccs811_ctx.timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create failed, err:%d", err);
        return err;
    }

    err = esp_timer_start_periodic(ccs811_ctx.timer, config->interval_ms * 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_start_periodic failed: %d", err);
        return err;
    }

    ccs811_ctx.is_initialized = true;
    ESP_LOGI(TAG, "ccs811 initialized successfully");

    return ESP_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static esp_err_t hyt271_convert(float & temp, float & hum)
{
    uint8_t data[4] = {0};
    esp_err_t err = write(HYT271_SENSOR_ADDR, 0xFF, 0xFF);                                  // Measurement Request // 0xFF stehen für nicht vorhanden
    if (err != ESP_OK) {
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(100));                                                         // 100 ms zwischen Measurement Request und Data Fetch 
    err = read(data, sizeof(data), HYT271_SENSOR_ADDR);                                     // Data Fetch
    if (err != ESP_OK) {
        return err;
    }

    #ifdef test
    uint8_t bit[8];
    printf("\nHYT271:\n");
    for (int i = 0; i < sizeof(data); i++) {
        printf("\nByte %d: ", i);                             
        for (int j = 7; j >= 0; j--) {
            bit[j] = (data[i] >> j) & 1;                                        // binär: das data[i] wird um j stellen nach rechts verschoben und mit 00000001 verundet
            printf("%d", bit[j]);
        }
    }
    #endif

    data[0] = data[0] & 0b00111111;                                                             // Bytereihenfolge: 0-1-2-3
    data[3] = data[3] >> 2;

    #ifdef test
    printf("\n0:%d, 3:%d\n", data[0], data[3]);
    #endif

    uint16_t raw_hum = ((uint16_t)data[0] << 8) | ((uint16_t) data[1]);
    uint16_t raw_temp = ((uint16_t)data[2] << 6) | ((uint16_t) data[3]);                        // um 6, da vorher schon um 2 verschoben

    #ifdef test
    printf("Humiditybytes: %d, Temperaturebytes: %d\n", raw_hum, raw_temp);
    #endif

    hum = (float)raw_hum * 100 / 16383;
    temp = (float)raw_temp * 165 / 16383 - 40;

    #ifdef test
    printf("\nHumidity: %3.2f %%RH, Temperature: %3.2f °C\n", hum, temp);
    #endif

    return ESP_OK;
}

static void hyt271_timer_cb_internal(void *arg){
    auto *ctx = (hyt271_sensor_ctx_t *) arg;
    if (!(ctx && ctx->config)) {
        return;
    }

    float temp, hum;
    esp_err_t err = hyt271_convert(temp, hum);
    if (err != ESP_OK) {
        return;
    }
    if (ctx->config->temperature.cb) {
        ctx->config->temperature.cb(ctx->config->temperature.endpoint_id, temp, ctx->config->user_data);
    }
    if (ctx->config->humidity.cb) {
        ctx->config->humidity.cb(ctx->config->humidity.endpoint_id, hum, ctx->config->user_data);
    }
}

esp_err_t hyt271_sensor_init(hyt271_sensor_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->temperature.cb == NULL || config->humidity.cb == NULL) {        // min. ein callback benötigt
        return ESP_ERR_INVALID_ARG;
    }
    if (hyt271_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = init_i2c();
    if (err != ESP_OK) {
        return err;
    }

    hyt271_ctx.config = config;

    esp_timer_create_args_t args = {
        .callback = hyt271_timer_cb_internal,
        .arg = &hyt271_ctx,
    };

    err = esp_timer_create(&args, &hyt271_ctx.timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create failed, err:%d", err);
        return err;
    }

    err = esp_timer_start_periodic(hyt271_ctx.timer, config->interval_ms * 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_start_periodic failed: %d", err);
        return err;
    }

    hyt271_ctx.is_initialized = true;
    ESP_LOGI(TAG, "hyt271 initialized successfully");

    return ESP_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static esp_err_t d6t1a01_convert(float & ptat, float & p0)
{
    uint8_t data[4] = {0};

    esp_err_t err = write(D6TA101_SENSOR_ADDR, 0x4c, 0xFF); 
    if (err != ESP_OK) {
        return err;
    }

    err = read(data, sizeof(data), D6TA101_SENSOR_ADDR);
    if (err != ESP_OK) {
        return err;
    }

    ptat = (256.0f * data[1] + data[0]) / 10.0f;                             // Rechnung nach Bsp aus DB durch 10 dividiert
    p0 = (256.0f * data[3] + data[2]) / 10.0f;
    #ifdef test
    printf("\nD6T1a01:\nPTAT: %2.2f °C, P0: %2.2f °C\n", ptat, p0);
    #endif

    return ESP_OK;
}


static void d6t1a01_timer_cb_internal(void *arg)
{
    auto *ctx = (d6t1a01_sensor_ctx_t *) arg;
    if (!(ctx && ctx->config)) {
        return;
    }

    float ptat, p0;
    static bool occupancy = false;
    esp_err_t err = d6t1a01_convert(ptat, p0);
    if (err != ESP_OK) {
        return;
    }

    bool new_occupancy = (p0 > (ptat + 2.0f));                  // Bewegungserkennung basierend auf Temperaturdifferenz  // Schwellwert: 2 Grad über PTAT
    #ifdef test
    printf("Bool: %d\n", new_occupancy);
    #endif

    if (occupancy != new_occupancy) {                           // Benachrichtigung nur wenn sich der Status ändert
        occupancy = new_occupancy;
        if (ctx->config->cb) {
            ctx->config->cb(ctx->config->endpoint_id, new_occupancy, ctx->config->user_data);
        }
    }
}

esp_err_t d6t1a01_sensor_init(d6t1a01_sensor_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->cb == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (d6t1a01_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = init_i2c();
    if (err != ESP_OK) {
        return err;
    }

    d6t1a01_ctx.config = config;

    esp_timer_create_args_t args = {
        .callback = d6t1a01_timer_cb_internal,
        .arg = &d6t1a01_ctx,
    };

    err = esp_timer_create(&args, &d6t1a01_ctx.timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create failed, err:%d", err);
        return err;
    }

    err = esp_timer_start_periodic(d6t1a01_ctx.timer, config->interval_ms * 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_start_periodic failed: %d", err);
        return err;
    }

    d6t1a01_ctx.is_initialized = true;
    ESP_LOGI(TAG, "d6t1a01 initialized successfully");

    return ESP_OK;
}