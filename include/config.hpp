#ifndef BH_CONFIG
#define BH_CONFIG

#include "esp_attr.h"
#include "esp_now.h"
#include "oled.hpp"
#include "queue.hpp"
#include "utils.hpp"
#include <cstdint>
#include <iterator>

namespace bh {

class Config {
public:
  explicit Config(MAC &macs, AtomicState &state) noexcept;
  ~Config() noexcept;

  void loop() noexcept;

private:
  static Config *instance;

  static void ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                   const std::uint8_t *data, int data_len) noexcept;

  static void IRAM_ATTR ButtonPressed(void *args) noexcept {
    if (args != nullptr) {
      static_cast<AtomicState *>(args)->store(
          State::WorkingUSB, std::memory_order_relaxed);
    }
  }

  static void IRAM_ATTR ResetConfig(void *args) noexcept {
    if (args != nullptr) {
      static_cast<std::atomic<std::uint32_t> *>(args)->store(
          1, std::memory_order_relaxed);
    }
  }

  Queue<std::array<std::uint8_t, 6>, 3> m_queue{};
  MAC &m_peers;
  AtomicState &state;
  // Oled m_oled{};
  std::atomic<std::uint32_t> m_reset{0};
};

} // namespace bh

#endif
