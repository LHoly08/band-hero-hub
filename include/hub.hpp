#ifndef BH_HUB
#define BH_HUB

#include "esp_attr.h"
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "queue.hpp"
#include "utils.hpp"
#include <array>
#include <cstdint>
#include <cstring>

namespace bh {

class Hub {
public:
  explicit Hub(const MAC &peers, AtomicState &state) noexcept;
  ~Hub() noexcept;
  void loop() noexcept;

private:
  static Hub *instance;

  static void ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                   const std::uint8_t *data, int data_len) noexcept;

  static void IRAM_ATTR ButtonPressed(void *args) noexcept {
    if (args != nullptr) {
      static_cast<AtomicState *>(args)->store(
          State::Configuring, std::memory_order_relaxed);
    }
  }

  Queue<std::uint32_t, 40> m_queue;
  const MAC &m_peers;

  AtomicState &state;
};

} // namespace bh

#endif
