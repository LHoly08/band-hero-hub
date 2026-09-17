#ifndef BH_OLED
#define BH_OLED

#include "u8g2.h"
#include <array>
#include <cstdint>

namespace bh {

class Oled {
public:
  Oled() noexcept;
  ~Oled() noexcept = default;

  inline void clearBuffer() noexcept { u8g2_ClearBuffer(&m_oled); }
  inline void sendBuffer() noexcept { u8g2_SendBuffer(&m_oled); }

  void drawLine(const std::array<std::uint8_t, 6> &mac,
                std::uint8_t line) noexcept;
  void drawBase() noexcept;

private:
  u8g2_t m_oled{};
  std::array<char, 23> m_mac{};
};

} // namespace bh

#endif
