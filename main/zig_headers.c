/* Keeps component header paths in compile_commands.json and wraps C macros. */
#include "esp32_hw_i2c.h"
#include "esp_err.h"
#include "esp_wifi.h"

esp_err_t zig_wifi_init_default(void) {
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  return esp_wifi_init(&config);
}

void zig_esp_error_check(esp_err_t error) { ESP_ERROR_CHECK(error); }

u8g2_esp32_i2c_config_t zig_u8g2_esp32_i2c_config_default(void) {
  const u8g2_esp32_i2c_config_t config = U8G2_ESP32_I2C_CONFIG_DEFAULT();
  return config;
}

void zig_u8g2_set_i2c_address(u8g2_t *oled, uint8_t address) {
  u8g2_SetI2CAddress(oled, address);
}
