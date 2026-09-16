#ifndef BH_UTILS
#define BH_UTILS

#include <array>
#include <cstdint>
#include <inplace_vector>

#include "driver/gpio.h"

namespace bh {

enum class State {
  Configuring = 0,
  WorkingUSB,
};

using MAC = std::inplace_vector<std::array<std::uint8_t, 6>, 4>;

constexpr gpio_num_t PIN = GPIO_NUM_0;

} // namespace bh

#endif
