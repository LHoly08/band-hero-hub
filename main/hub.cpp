#include "hub.hpp"
#include <cstring>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

namespace bh
{

    static const char* TAG = "Hub";

    template <>
    Hub<Connection::USB>::Hub(std::array<std::array<std::uint8_t, 6>, 4> peers, State &state) noexcept : m_peers(peers), state(state) {}

    template <>
    Hub<Connection::BLUETOOTH>::Hub(std::array<std::array<std::uint8_t, 6>, 4> peers, State &state) noexcept : m_peers(peers), state(state)
    {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            (void)nvs_flash_erase();
            ret = nvs_flash_init();
        }
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "NVS flash init failed");
            return;
        }

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if (esp_bt_controller_init(&bt_cfg) != ESP_OK)
        {
            ESP_LOGE(TAG, "Initialize controller failed");
            return;
        }
        if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK)
        {
            ESP_LOGE(TAG, "Enable controller failed");
            return;
        }

        esp_bluedroid_config_t cfg = {};
        cfg.ssp_en = true;

        if (esp_bluedroid_init_with_cfg(&cfg) != ESP_OK)
        {
            ESP_LOGE(TAG, "Initialize Bluedroid failed");
            return;
        }

        if (esp_bluedroid_enable() != ESP_OK)
        {
            ESP_LOGE(TAG, "Enable Bluedroid failed");
            return;
        }

        const std::uint8_t *bt_mac = esp_bt_dev_get_address();
        if (bt_mac)
        {
            std::array<std::uint8_t, 6> address;
            std::memcpy(address.data(), bt_mac, address.size());

            ESP_LOGI(TAG, "Local BT MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                     address[0], address[1], address[2], address[3], address[4], address[5]);
        }
    }

    template <>
    Hub<Connection::USB>::~Hub() noexcept {}

    template <>
    Hub<Connection::BLUETOOTH>::~Hub() noexcept
    {
        (void)esp_bluedroid_disable();
        (void)esp_bluedroid_deinit();

        (void)esp_bt_controller_disable();
        (void)esp_bt_controller_deinit();
    }

    template <>
  void Hub<Connection::BLUETOOTH>::loop() noexcept
  {

    while (state == State::WorkingBT)
    {

      while (!m_queue.empty())
      {

      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  }

    template <>
  void Hub<Connection::USB>::loop() noexcept
  {

    while (state == State::WorkingUSB)
    {

      while (!m_queue.empty())
      {

      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  }
}