#include <cstdio>

#include "hub.hpp"

#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace bh {

static constexpr TickType_t LOOP_DELAY_TICKS{
    pdMS_TO_TICKS(1) > 0 ? pdMS_TO_TICKS(1) : 1};

static const char *TAG = "Hub";

Hub::Hub(const MAC &peers, State &state) noexcept
    : m_peers(peers), state(state) {

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();

  esp_now_init();
  instance = this;
  ESP_ERROR_CHECK(esp_now_register_recv_cb(ReceivedCallback));
}

Hub::~Hub() noexcept {

  (void)esp_now_unregister_recv_cb();
  (void)esp_now_deinit();

  (void)esp_wifi_stop();
  (void)esp_wifi_deinit();

  instance = nullptr;
}

void Hub::loop() noexcept {

  while (state == State::WorkingUSB) {

    while (!m_queue.empty()) {
      if (std::uint32_t val{}; m_queue.pop(val)) {

        std::fwrite(&val, 1, sizeof(val), stdout);
        std::fflush(stdout);
      }
    }
    vTaskDelay(LOOP_DELAY_TICKS);
  }
}

Hub *Hub::instance = nullptr;

void Hub::ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                           const std::uint8_t *data, int data_len) noexcept {
  if (instance == nullptr || esp_now_info == nullptr ||
      esp_now_info->src_addr == nullptr || data == nullptr ||
      data_len != sizeof(std::uint32_t)) {
    return;
  }

  std::uint32_t val{};
  std::memcpy(&val, data, data_len);

  std::array<std::uint8_t, 6> macAddress{};
  std::memcpy(macAddress.data(), esp_now_info->src_addr, macAddress.size());

  const auto &m_peers = instance->m_peers;
  for (std::uint8_t i{}; i < m_peers.size(); ++i) {
    if (macAddress == m_peers[i]) {
      val |= i;
      (void)instance->m_queue.push(val);
      break;
    }
  }
}

} // namespace bh
