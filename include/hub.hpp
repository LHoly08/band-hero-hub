#ifndef BH_HUB
#define BH_HUB

#include "freertos/FreeRTOS.h"
#include "esp_now.h"
#include "esp_bt.h"
#include "queue.hpp"
#include <array>
#include <cstdint>
#include <chrono>
#include <thread>

namespace bh
{

  enum class State
  {
    Configuring = 0,
    WorkingBT,
    WorkingUSB,
  };

  enum class Connection
  {
    USB = 1,
    BLUETOOTH,
  };

  template <Connection T>
  class Hub
  {
  public:
    explicit Hub(std::array<std::array<std::uint8_t, 6>, 4> peers, State &state) noexcept;
    ~Hub() noexcept;
    void loop() noexcept;

  private:
    Queue<std::uint32_t, 40> m_queue;
    std::array<std::array<std::uint8_t, 6>, 4> m_peers;
    const std::uint8_t m_numberPeers;

    State &state;
  };

} // namespace bh

#endif