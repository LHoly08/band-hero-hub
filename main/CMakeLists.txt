# Original C++ application configuration (preserved for switching back).
# idf_component_register(
#     SRCS
#         "main.cpp"
#         "hub.cpp"
#         "ble_reports.cpp"
#     INCLUDE_DIRS
#         "../include"
#     REQUIRES
#         nvs_flash
#         esp_wifi
#         esp_netif
#         freertos
#         esp_hid
#         driver
# )
#
# target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_26)

# Zig supplies app_main; this C file preserves ESP-IDF's component include paths.
idf_component_register(
    SRCS "zig_headers.c"
    REQUIRES
        nvs_flash esp_wifi esp_netif freertos
        esp_hid driver
)

add_prebuilt_library(
    zig_app
    "${CMAKE_CURRENT_LIST_DIR}/../Zig/zig-out/lib/libzig_app.a"
)
target_link_libraries(${COMPONENT_LIB} PUBLIC zig_app)
