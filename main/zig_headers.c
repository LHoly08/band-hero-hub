/* Keeps component header paths in compile_commands.json and wraps C macros. */
#include "esp_err.h"
#include "esp_wifi.h"

esp_err_t zig_wifi_init_default(void) {
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  return esp_wifi_init(&config);
}

void zig_esp_error_check(esp_err_t error) { ESP_ERROR_CHECK(error); }
