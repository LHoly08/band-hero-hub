idf_component_register(
    SRCS "src/main.cpp"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_now 
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_20)
