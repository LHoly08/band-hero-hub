#ifndef BH_CONFIG
#define BH_CONFIG

#include "esp_attr.h"
#include "esp_now.h"
#include "oled.hpp"
#include "queue.hpp"
#include "utils.hpp"
#include <cstdint>

namespace bh {

class Config {
public:
  explicit Config(MAC &macs, State &state) noexcept;
  ~Config() noexcept;

  void loop() noexcept;

private:
  static Config *instance;

  static void IRAM_ATTR
  ReceivedCallback(const esp_now_recv_info_t *esp_now_info,
                   const std::uint8_t *data, int data_len) noexcept;

  inline static void IRAM_ATTR ButtonPressed(void *args) noexcept {
    instance->state = State::WorkingUSB;
  }

  inline static void IRAM_ATTR ResetConfig(void *args) noexcept {
    (static_cast<MAC *>(args))->clear();
  }

  Queue<std::array<std::uint8_t, 6>, 3> m_queue{};
  MAC &m_peers;
  State &state;
  Oled m_oled{};
};

} // namespace bh

#endif
