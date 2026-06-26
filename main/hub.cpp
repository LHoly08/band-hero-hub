#include "hub.hpp"
#include "driver/uart.h"
#include "esp_bt_defs.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_log.h"
#include "esp_now.h"
#include "nvs_flash.h"
#include "esp_gap_ble_api.h"
#include <array>
#include <atomic>
#include <cstring>

namespace bh {

static Hub<Connection::USB> *s_usbHubInstance = nullptr;
static Hub<Connection::BLUETOOTH> *s_bluetoothHubInstance = nullptr;

template <Connection T> void register_instance(Hub<T> *instance) {
  if constexpr (T == Connection::USB) {
    s_usbHubInstance = instance;
  } else {
    s_bluetoothHubInstance = instance;
  }
}

static constexpr uart_port_t UART_NUM = UART_NUM_0;

static const char *TAG = "Hub";

template <>
Hub<Connection::USB>::Hub(const MAC &peers, State &state) noexcept
    : m_peers(peers), state(state) {

  s_usbHubInstance = this;

  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
      .source_clk = UART_SCLK_DEFAULT,
  };

  uart_param_config(UART_NUM, &uart_config);

  uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM, BUFSIZ * 2, 0, 0, NULL, 0);

  (void)esp_now_init();
}

template <>
Hub<Connection::BLUETOOTH>::Hub(const MAC &peers, State &state) noexcept
    : m_peers(peers), state(state) {
  s_bluetoothHubInstance = this;

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
}

template <> Hub<Connection::USB>::~Hub() noexcept {
  (void)esp_now_unregister_recv_cb();
  (void)esp_now_deinit();
  s_usbHubInstance = nullptr;
}

template <> Hub<Connection::BLUETOOTH>::~Hub() noexcept {

  (void)esp_bluedroid_disable();
  (void)esp_bluedroid_deinit();

  (void)esp_bt_controller_disable();
  (void)esp_bt_controller_deinit();

  (void)esp_now_unregister_recv_cb();
  (void)esp_now_deinit();

  s_bluetoothHubInstance = nullptr;
}

template <> void Hub<Connection::BLUETOOTH>::loop() noexcept {

  while (state == State::WorkingBT) {

    std::uint32_t dataValue{};
    while (m_queue.pop(dataValue)) {
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
}

template <> void Hub<Connection::USB>::loop() noexcept {

  while (state == State::WorkingUSB) {

    std::uint32_t dataValue{};
    while (m_queue.pop(dataValue)) {

      uart_write_bytes(UART_NUM, reinterpret_cast<const char *>(&dataValue),
                       sizeof(dataValue));
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
}

template <>
void Hub<Connection::USB>::ReceiveCallback(const esp_now_recv_info_t *recv_info,
                                           const std::uint8_t *data,
                                           int data_len) {
  if (s_usbHubInstance) {
    std::uint32_t note{};
    std::memcpy(&note, data, data_len);
    s_usbHubInstance->receive(recv_info->src_addr, note);
  }
}

template <>
void Hub<Connection::BLUETOOTH>::ReceiveCallback(
    const esp_now_recv_info_t *recv_info, const std::uint8_t *data,
    int data_len) {

  if (s_bluetoothHubInstance) {
    std::uint32_t note{};
    std::memcpy(&note, data, data_len);
    s_bluetoothHubInstance->receive(recv_info->src_addr, note);
  }
}

} // namespace bh