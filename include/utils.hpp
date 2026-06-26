#ifndef BH_UTILS
#define BH_UTILS

#include <array>
#include <cstdint>

namespace bh {

struct MAC {
  std::array<std::array<std::uint8_t, 6>, 4> macAddresses;
  std::uint8_t numberOfActives;
};

} // namespace bh

#endif