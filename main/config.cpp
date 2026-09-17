#include <cstring>

#include "config.hpp"

#include "esp_err.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal/gpio_types.h"

namespace bh {

Config::Config(MAC &peers, State &state) noexcept
    : m_peers(peers), state(state) {
  ESP_ERROR_CHECK(gpio_install_isr_service(0));

  {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
  }
  {
    gpio_config_t cfg;

    cfg.pin_bit_mask = 1ULL << PIN;
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_ENABLE;
    cfg.intr_type = GPIO_INTR_POSEDGE;

    ESP_ERROR_CHECK(gpio_config(&cfg));
  }
  instance = this;

  ESP_ERROR_CHECK(gpio_isr_handler_add(PIN, ButtonPressed, nullptr));
  ESP_ERROR_CHECK(esp_now_register_recv_cb(ReceivedCallback));
}

Config::~Config() noexcept {

  (void)esp_now_unregister_recv_cb();
  (void)esp_now_deinit();

  (void)esp_wifi_stop();
  (void)esp_wifi_deinit();

  gpio_isr_handler_remove(PIN);
  gpio_uninstall_isr_service();

  instance = nullptr;
}

void Config::loop() noexcept {

  while (state == State::Configuring) {
    std::array<std::uint8_t, 6> macAddr{};

    while (m_queue.pop(macAddr)) {

      bool exists{false};
      for (const auto &mac : m_peers) {
        if (mac == macAddr) {
          exists = true;
          break;
        }
      }
      if (!exists && m_peers.size() != m_peers.capacity()) {
        m_peers.push_back(macAddr);
      }
    }
    m_oled.clearBuffer();

    m_oled.drawBase();
    for (std::uint8_t i{}; i < m_peers.size(); ++i) {
      m_oled.drawLine(m_peers[i], i);
    }

    m_oled.sendBuffer();
  }
}

Config *Config::instance = nullptr;

void Config::ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                              const std::uint8_t *data, int data_len) noexcept {

  if (instance == nullptr || esp_now_info == nullptr ||
      esp_now_info->src_addr == nullptr || data == nullptr ||
      data_len != sizeof(std::uint32_t)) {
    return;
  }

  std::array<std::uint8_t, 6> macAddress{};
  std::memcpy(macAddress.data(), esp_now_info->src_addr, macAddress.size());
  (void)instance->m_queue.push<Type::ISR>(macAddress);
}

} // namespace bh
