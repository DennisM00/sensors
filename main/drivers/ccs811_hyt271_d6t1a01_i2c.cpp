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
    // Deinstalliere den I2C-Treiber, falls er bereits installiert ist
    i2c_driver_delete(I2C_MASTER_NUM);

    // I2C master initialization
    i2c_config_t conf = {                                                                       // conf
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

static esp_err_t ccs811_read(uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (CSS811_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);             // true entspricht ACK
    i2c_master_write_byte(cmd, 0x02, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (CSS811_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    cmd = NULL;
    return ESP_OK;
}

static esp_err_t ccs811_convert(float & eCO2, float & TVOC)
{
    uint8_t data[4] = {0};

    esp_err_t err = ccs811_read(data, sizeof(data));
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

    uint16_t raw_eCO2 = ((uint16_t)data[0] << 8) | ((uint16_t) data[1]);
    uint16_t raw_TVOC = ((uint16_t)data[2] << 8) | ((uint16_t) data[3]);

    #ifdef test
    printf("\neCO2bytes: %d, TVOCbytes: %d\n", raw_eCO2, raw_TVOC);
    #endif

    //Warum Rechnung? werte von oben sind OK?                                                       // werte überprüfen-Rechnung nötig?
    eCO2 = (raw_eCO2 - 0.5) / 512;
    TVOC = (raw_TVOC - 0.5) / 512;

    #ifdef test
    printf("eCO2: %f ppm, TVOC: %f \n", eCO2, TVOC);
    #endif
    
    return ESP_OK;
}

static void ccs811_timer_cb_internal(void *arg)                                                                     // ??????????????????????
{
    auto *ctx = (ccs811_sensor_ctx_t *) arg;
    if (!(ctx && ctx->config)) {
        return;
    }

    float eCO2, TVOC;
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
    if (config->eCO2.cb == NULL || config->TVOC.cb == NULL) {                       // we need at least one callback so that we can start notifying application layer
        return ESP_ERR_INVALID_ARG;
    }
    if (ccs811_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = init_i2c();
    if (err != ESP_OK) {
        return err;
    }

    ccs811_ctx.config = config;                                                          // keep the pointer to config

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

static esp_err_t hyt271_read(uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                               // Mearurement Request
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HYT271_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);             // true entspricht ACK
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    vTaskDelay(pdMS_TO_TICKS(100));                                                             // 100 ms zwischen Measurement Request und Data Fetch

    cmd = i2c_cmd_link_create();                                                                // Data Fetch         
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HYT271_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    cmd = NULL;
    return ESP_OK;
}

static esp_err_t hyt271_convert(float & temp, float & hum)
{
    uint8_t data[4] = {0};

    esp_err_t err = hyt271_read(data, sizeof(data));
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

static void hyt271_timer_cb_internal(void *arg)                                     // ??????????????????????
{
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
    if (config->temperature.cb == NULL || config->humidity.cb == NULL) {            // we need at least one callback so that we can start notifying application layer
        return ESP_ERR_INVALID_ARG;
    }
    if (hyt271_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = init_i2c();
    if (err != ESP_OK) {
        return err;
    }

    hyt271_ctx.config = config;                                                          // keep the pointer to config

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

static esp_err_t d6t1a01_read(uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (D6TA101_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);             // true entspricht ACK
    i2c_master_write_byte(cmd, 0x4c, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (D6TA101_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    cmd = NULL;
    return ESP_OK;
}

static esp_err_t d6t1a01_convert(float & ptat, float & p0)          // generell bool ausgeben?
{
    uint8_t data[4] = {0};

    esp_err_t err = d6t1a01_read(data, sizeof(data));
    if (err != ESP_OK) {
        return err;
    }  

    ptat = (256 * data[1] + data[0]);                             // Rechnung nach Bsp aus DB
    p0 = (256 * data[3] + data[2]);

    return ESP_OK;
}
//////////////////toooooooooooooooo dooooooooooooooooooo