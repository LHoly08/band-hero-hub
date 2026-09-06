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

Hub *Hub::instance = nullptr;

void Hub::ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                           const std::uint8_t *data, int data_len) noexcept {
  if (instance == nullptr || esp_now_info == nullptr ||
      esp_now_info->src_addr == nullptr || data == nullptr ||
      data_len != sizeof(std::uint32_t)) {
    return;
  }

  std::uint32_t val{};
  std::memcpy(&val, data, data_len);

  std::array<std::uint8_t, 6> macAddress{};
  std::memcpy(macAddress.data(), esp_now_info->src_addr, macAddress.size());

  const auto &m_peers = instance->m_peers;
  for (std::uint8_t i{}; i < m_peers.size(); ++i) {
    if (macAddress == m_peers[i]) {
      val |= i;
      (void)instance->m_queue.push(val);
      break;
    }
  }
}

} // namespace bh

#endif
