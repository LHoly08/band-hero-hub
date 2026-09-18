#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ranges>
#include <vector>

#include "config.hpp"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "hub.hpp"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

extern "C" void app_main() {

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  bh::AtomicState state{bh::State::WorkingUSB};

  bh::MAC instrumentMacs{};

  vTaskDelay(pdMS_TO_TICKS(1000));
  {
    std::array<std::uint8_t, 6> mac{};
    ESP_ERROR_CHECK(esp_read_mac(mac.data(), ESP_MAC_WIFI_STA));

    std::array<std::uint8_t, 18> hex;

    hex.fill(':');
    hex.back() = '\n';

    auto hexDigit = [](std::uint8_t value) -> std::uint8_t {
      return "0123456789ABCDEF"[value & 0xF];
    };

    for (std::uint8_t i{}; i < mac.size(); ++i) {
      hex[i * 3] = hexDigit(mac[i] >> 4);
      hex[i * 3 + 1] = hexDigit(mac[i]);
    }

    std::fwrite("MAC: ", 1, 5, stdout);
    std::fwrite(hex.data(), 1, hex.size(), stdout);
    std::fflush(stdout);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));

infinite_loop: {
  switch (state.load(std::memory_order_relaxed)) {

  case bh::State::Configuring: {

    bh::Config hub(instrumentMacs, state);
    hub.loop();
  } break;

  case bh::State::WorkingUSB: {

    bh::Hub hub(instrumentMacs, state);
    hub.loop();
  } break;
  }
}
  goto infinite_loop;
}
