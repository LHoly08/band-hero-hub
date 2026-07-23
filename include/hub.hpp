#ifndef BH_HUB
#define BH_HUB

#include "esp_bt.h"
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "queue.hpp"
#include "utils.hpp"
#include <array>
#include <cstdint>

namespace bh {

enum class State {
  Configuring = 0,
  WorkingBT,
  WorkingUSB,
};

enum class Connection {
  USB = 1,
  BLUETOOTH,
};

template <Connection T> class Hub {
public:
  explicit Hub(const MAC &peers, State &state) noexcept;
  ~Hub() noexcept;
  void loop() noexcept;

private:
  static Hub<T> *instance{nullptr};

  static void IRAM_ATTR
  ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                   const std::uint8_t *data, int data_len) noexcept;

  Queue<std::uint32_t, 40> m_queue;
  const MAC &m_peers;

  State &state;
};

template <Connection T>
void Hub<T>::ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                              const std::uint8_t *data, int data_len) noexcept {
  std::uint32_t val{};
  std::memcpy(&val, data, data_len);

  std::array<std::uint8_t, 6> macAddress{};
  std::memcpy(macAddress.data(), esp_now_info->macAddress, macAddress.size());

  for (std::uint8_t i{}; i < m_peers.numberOfActives; ++i) {
    if (macAddress == m_peers.macAddresses[i]) {
      val |= i;
      (void)m_queue.push<Type::ISR>(val);
      break;
    }
  }
}

} // namespace bh

#endif
