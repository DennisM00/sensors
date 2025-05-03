idf_component_register(
    SRCS 
        "app_main.cpp"
        "drivers/ccs811_hyt271_d6t1a01_i2c.cpp"
    INCLUDE_DIRS 
        "."
        "drivers"
    PRIV_INCLUDE_DIRS
        "${ESP_MATTER_PATH}/examples/common/utils"
        "${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/src/include"
        "${ESP_MATTER_PATH}/components/esp_matter"
    REQUIRES 
        app_reset 
        esp_matter
)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
