#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ranges>
#include <vector>

#include "esp_mac.h"
#include "hub.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

extern "C" void app_main() {

  bh::State state{bh::State::Configuring};

  bh::MAC instrumentMacs{
      {0x24, 0x0A, 0xC4, 0x00, 0x11, 0x22}, // Guitar
      {0x24, 0x0A, 0xC4, 0x00, 0x33, 0x44}, // Bass
      {0x24, 0x0A, 0xC4, 0x00, 0x55, 0x66}, // Drums
      {0x24, 0x0A, 0xC4, 0x00, 0x77, 0x88}  // Auxiliary
  };

  vTaskDelay(pdMS_TO_TICKS(1000));
  {
    std::array<std::uint8_t, 6> mac{};
    (void)esp_read_mac(mac.data(), ESP_MAC_EFUSE_FACTORY);

    constexpr std::uint8_t n = 17;

    constexpr auto h = [&]() -> std::array<std::uint8_t, n> {
      std::vector<std::uint8_t> v(n, ':');
      std::array<std::uint8_t, n> a{};
      std::memcpy(a.data(), v.data(), a.size());

      return a;
    };

    std::array<std::uint8_t, n> hex;

    {
      std::array<std::uint8_t, n> t = h();
      std::ranges::transform(t.begin(), t.end(), hex.begin(),
                             [](std::uint8_t i) -> std::uint8_t { return i; });
    }
    hex.back() = '\n';

    auto hexDigit = [](std::uint8_t value) -> std::uint8_t {
      return "0123456789ABCDEF"[value & 0xF];
    };

    for (std::uint8_t i{}; i < mac.size(); ++i) {
      hex[i * 3] = hexDigit(mac[i]);
      hex[i * 3 + 1] = hexDigit((mac[i] >> 4));
    }

    std::fwrite("MAC: ", 1, 5, stdout);
    std::fwrite(hex.data(), 1, n, stdout);
    std::fflush(stdout);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));

infinite_loop: {
  switch (state) {

  case bh::State::Configuring: {
  } break;

  case bh::State::WorkingUSB: {
    bh::Hub hub(instrumentMacs, state);
    hub.loop();
  } break;
  }
}
  goto infinite_loop;
}
