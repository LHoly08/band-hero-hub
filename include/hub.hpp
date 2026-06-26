#ifndef BH_HUB
#define BH_HUB

#include "esp_bt.h"
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "queue.hpp"
#include "utils.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>

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

  static void IRAM_ATTR ReceiveCallback(const esp_now_recv_info_t *recv_info,
                                        const std::uint8_t *data, int data_len);
  void loop() noexcept;

private:
  void IRAM_ATTR receive(const std::uint8_t *mac, std::uint32_t data);

  Queue<std::uint32_t, 40> m_queue;
  const MAC &m_peers;

  State &state;
};

template <Connection T>
void Hub<T>::receive(const std::uint8_t *mac, std::uint32_t data) {

  std::array<std::uint8_t, 6> macAddress{};
  std::memcpy(macAddress.data(), mac, macAddress.size());
  data <<= 2;

  for (std::uint8_t i{}; i < m_peers.numberOfActives; ++i) {
    if (macAddress == m_peers.macAddresses[i]) {
      data |= i;
      (void)m_queue.push<Type::ISR>(data);
      break;
    }
  }
}

} // namespace bh

#endif