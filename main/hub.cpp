#include <chrono>
#include <thread>

#include "hub.hpp"

#include "esp_bt_defs.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_log.h"
#include "esp_now.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace bh {

static const char *TAG = "Hub";

template <>
Hub<Connection::USB>::Hub(std::array<std::array<std::uint8_t, 6>, 4> peers,
                          State &state) noexcept
    : m_peers(peers), state(state) {

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();

  esp_now_init();
  esp_now_register_recv_cb();

  instance = this;
}

template <>
Hub<Connection::BLUETOOTH>::Hub(
    std::array<std::array<std::uint8_t, 6>, 4> peers, State &state) noexcept
    : m_peers(peers), state(state) {

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    (void)nvs_flash_erase();
    ret = nvs_flash_init();
  }
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NVS flash init failed");
    return;
  }

  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
    ESP_LOGE(TAG, "Initialize controller failed");
    return;
  }
  if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
    ESP_LOGE(TAG, "Enable controller failed");
    return;
  }

  esp_ble_gap_set_device_name("BandHero Hub");

  esp_bluedroid_config_t cfg = {};
  cfg.ssp_en = true;

  if (esp_bluedroid_init_with_cfg(&cfg) != ESP_OK) {
    ESP_LOGE(TAG, "Initialize Bluedroid failed");
    return;
  }

  if (esp_bluedroid_enable() != ESP_OK) {
    ESP_LOGE(TAG, "Enable Bluedroid failed");
    return;
  }

  const std::uint8_t *bt_mac = esp_bt_dev_get_address();
  if (bt_mac) {
    std::array<std::uint8_t, 6> address;
    std::memcpy(address.data(), bt_mac, address.size());

    ESP_LOGI(TAG, "Local BT MAC: %02x:%02x:%02x:%02x:%02x:%02x", address[0],
             address[1], address[2], address[3], address[4], address[5]);
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();

  esp_now_init();
  esp_now_register_recv_cb();

  instance = this;
}

template <> Hub<Connection::USB>::~Hub() noexcept {

  (void)esp_now_unregister_recv_cb();
  (void)esp_now_deinit();

  (void)esp_wifi_stop();
  (void)esp_wifi_deinit();

  instance = nullptr;
}

template <> Hub<Connection::BLUETOOTH>::~Hub() noexcept {

  (void)esp_bluedroid_disable();
  (void)esp_bluedroid_deinit();

  (void)esp_bt_controller_disable();
  (void)esp_bt_controller_deinit();

  (void)esp_now_unregister_recv_cb();
  (void)esp_now_deinit();

  (void)esp_wifi_stop();
  (void)esp_wifi_deinit();

  instance = nullptr;
}

template <> void Hub<Connection::BLUETOOTH>::loop() noexcept {

  while (state == State::WorkingBT) {

    while (!m_queue.empty()) {
      if (std::uint32_t val{}; m_queue.pop(val)) {
      }
    }

    vTaskDelay(pdMS_TO_TICKS(0.01f));
  }
}

template <> void Hub<Connection::USB>::loop() noexcept {

  while (state == State::WorkingUSB) {

    while (!m_queue.empty()) {
      if (std::uint32_t val{}; m_queue.pop(val)) {

        std::fwrite(reinterpret_cast<std::uint8_t *>(val), 1, sizeof(val),
                    stdout);
        std::fflush(stdout);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(0.01f));
  }
}

} // namespace bh
