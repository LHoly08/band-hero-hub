#ifndef BH_HUB
#define BH_HUB

#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "queue.hpp"
#include "utils.hpp"
#include <array>
#include <cstdint>
#include <cstring>

namespace bh {

enum class Connection {
  USB = 1,
};

class Hub {
public:
  explicit Hub(const MAC &peers, State &state) noexcept;
  ~Hub() noexcept;
  void loop() noexcept;

private:
  static Hub *instance;

  static void ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                               const std::uint8_t *data, int data_len) noexcept;

  Queue<std::uint32_t, 40> m_queue;
  const MAC &m_peers;

  State &state;
};

} // namespace bh

#endif
