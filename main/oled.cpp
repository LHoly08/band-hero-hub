#include <array>

#include "oled.hpp"

#include "esp32_hw_i2c.h"
#include "esp_err.h"
#include "esp_mac.h"

#include "driver/gpio.h"

namespace bh {

Oled::Oled() noexcept {

  std::array<std::uint8_t, 6> mac{};
  ESP_ERROR_CHECK(esp_read_mac(mac.data(), ESP_MAC_WIFI_STA));

  std::array<char, 23> hex;

  hex.fill(':');
  hex.back() = '\n';

  auto hexDigit = [](std::uint8_t value) -> char {
    return "0123456789ABCDEF"[value & 0xF];
  };

  for (std::uint8_t i{}; i < mac.size(); ++i) {
    const std::uint8_t index = (i * 3) + 5;
    hex[index] = hexDigit(mac[i] >> 4);
    hex[index + 1] = hexDigit(mac[i]);
  }

  hex[0] = 'M';
  hex[1] = 'A';
  hex[2] = 'C';
  hex[3] = ':';
  hex[4] = ' ';

  m_mac = hex;

  u8g2_esp32_i2c_ctx_t i2c{};
  constexpr std::uint8_t oledAddress = 0x3D;

  i2c.cfg = U8G2_ESP32_I2C_CONFIG_DEFAULT();
  i2c.cfg.sda_pin = GPIO_NUM_22;
  i2c.cfg.scl_pin = GPIO_NUM_23;
  i2c.cfg.clk_hz = 100000;
  i2c.cfg.dev_addr_7bit = oledAddress;
  i2c.cfg.reset_pin = U8G2_ESP32_PIN_UNUSED;

  ESP_ERROR_CHECK(u8g2_esp32_i2c_set_default_context(&i2c));

  u8g2_Setup_sh1106_128x64_noname_f(&m_oled, U8G2_R0, u8x8_byte_esp32_hw_i2c,
                                    u8x8_gpio_and_delay_esp32_i2c);

  u8g2_SetI2CAddress(&m_oled, oledAddress << 1);
  u8g2_InitDisplay(&m_oled);
  u8g2_SetPowerSave(&m_oled, 0);
}

void Oled::drawLine(const std::array<std::uint8_t, 6> &mac,
                    std::uint8_t line) noexcept {

  u8g2_SetFont(&m_oled, u8g2_font_6x10_tf);
  std::array<char, 22> text{};
  text.fill(':');
  text.back() = '\0';

  text[0] = line + '1';
  text[1] = ' ';
  text[2] = '-';
  text[3] = ' ';

  auto hexDigit = [](std::uint8_t value) -> char {
    return "0123456789ABCDEF"[value & 0xF];
  };

  for (std::uint8_t i{}; i < 6; ++i) {
    const std::uint8_t index = (4 + (i * 3));
    text[index] = hexDigit(mac[i] >> 4);
    text[index + 1] = hexDigit(mac[i]);
  }

  u8g2_DrawStr(&m_oled, 0, 8 + (line * 9), text.data());
}

void Oled::drawBase() noexcept {

  u8g2_SetFont(&m_oled, u8g2_font_5x8_tf);

  u8g2_DrawStr(&m_oled, 0, 56, m_mac.data());
  u8g2_DrawStr(&m_oled, 0, 0, "Peers:");
}

} // namespace bh
