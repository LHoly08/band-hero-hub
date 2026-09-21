#ifndef BH_UTILS
#define BH_UTILS

#include <array>
#include <atomic>
#include <cstdint>
#include <inplace_vector>

#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace bh {

enum class State : std::uint32_t {
  Configuring = 0,
  WorkingUSB,
};

// Native-width atomics keep GPIO ISR accesses lock-free on ESP32-C6.
using AtomicState = std::atomic<State>;
static_assert(AtomicState::is_always_lock_free);
static_assert(std::atomic<std::uint32_t>::is_always_lock_free);

using MAC = std::inplace_vector<std::array<std::uint8_t, 6>, 4>;

constexpr gpio_num_t PIN = GPIO_NUM_20;
constexpr gpio_num_t RESET_PIN = GPIO_NUM_19;

constexpr std::uint32_t BUTTON_DEBOUNCE_MS = 200;
constexpr TickType_t BUTTON_DEBOUNCE_TICKS =
    (BUTTON_DEBOUNCE_MS * configTICK_RATE_HZ + 999) / 1000;
static_assert(sizeof(TickType_t) == sizeof(std::uint32_t));

struct ButtonDebounce {
  TickType_t last_press_tick{0};
  bool has_press{false};

  bool IRAM_ATTR accept() noexcept {
    const TickType_t now = xTaskGetTickCountFromISR();
    // Unsigned subtraction also handles tick-counter wraparound.
    if (has_press && TickType_t(now - last_press_tick) < BUTTON_DEBOUNCE_TICKS) {
      return false;
    }
    last_press_tick = now;
    has_press = true;
    return true;
  }
};

// Accessed only by the serialized GPIO ISRs; persist across mode changes.
inline DRAM_ATTR ButtonDebounce mode_button_debounce{};
inline DRAM_ATTR ButtonDebounce reset_button_debounce{};

} // namespace bh

#endif
