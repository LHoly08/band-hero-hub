#ifndef BH_UTILS
#define BH_UTILS

#include <array>
#include <atomic>
#include <cstdint>
#include <inplace_vector>

#include "driver/gpio.h"

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

} // namespace bh

#endif
