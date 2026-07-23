#include "hub.hpp"
#include "utils.hpp"

extern "C" void app_main() {

  bh::State state{bh::State::Configuring};

  bh::MAC instrumentMacs{.macAddresses = {{
                             {0x24, 0x0A, 0xC4, 0x00, 0x11, 0x22}, // Guitar
                             {0x24, 0x0A, 0xC4, 0x00, 0x33, 0x44}, // Bass
                             {0x24, 0x0A, 0xC4, 0x00, 0x55, 0x66}, // Drums
                             {0x24, 0x0A, 0xC4, 0x00, 0x77, 0x88}  // Auxiliary
                         }},
                         .numberOfActives = 4};

infinite_loop: {

  switch (state) {

  case bh::State::Configuring: {
  } break;

  case bh::State::WorkingBT: {
    bh::Hub<bh::Connection::BLUETOOTH> hub({}, state);
    hub.loop();
  } break;

  case bh::State::WorkingUSB: {
    bh::Hub<bh::Connection::USB> hub({}, state);
    hub.loop();
  } break;
  }
}
  goto infinite_loop;
}
