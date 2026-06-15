idf_component_register(
    SRCS 
        "main.cpp"
        "hub.cpp"
    INCLUDE_DIRS
        "../include"
    REQUIRES
        nvs_flash
        esp_wifi
        esp_netif
        freertos
        bt
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_23)
