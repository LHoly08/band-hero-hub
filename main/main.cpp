#include <chrono>
#include <thread>

#include "esp_log.h"

void setup();
void loop();

extern "C" void app_main() {}

void setup() { ESP_LOGV("TEST", "Hello from ESP"); }

void loop() {
  using namespace std::chrono_literals;

  ESP_LOGV("TEST", "HELLO!");
  std::this_thread.sleep_for(100ms);
}
