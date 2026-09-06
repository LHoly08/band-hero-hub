# band-hero-hub

Build with CMake 3.31 or newer, C++26, and Espressif's GCC 16.1.0
`riscv32-esp-elf` toolchain. The SDK used for this setup is ESP-IDF
`85c826ecb1` (6.2 development), which selects `esp-16.1.0_20260609`.
The host machine's GCC does not compile the ESP32-C6 firmware.

Activate the updated SDK in each terminal. Clear an inherited Python environment
override if that terminal previously used ESP-IDF 6.1:

```sh
unset IDF_PYTHON_ENV_PATH
. /home/lsdias/esp/esp-idf/export.sh
riscv32-esp-elf-g++ --version
```

From the repository root, build the standalone `std::inplace_vector` firmware
check with:

```sh
idf.py -C tests/toolchain -B tests/toolchain/build -D IDF_TARGET=esp32c6 build
```

It uses the project's `bh::MAC` alias and checks the language/library feature
macros during compilation. When flashed, it checks insertion and capacity
handling and prints `inplace_vector: PASS (4/4 MAC addresses)` on success.

Build the hub firmware in the usual `build/` directory:

```sh
idf.py build
```

The current configuration enables Bluedroid with legacy BLE advertising and
uses the large single-app partition to fit Wi-Fi and BLE within the configured
2 MB flash.

Flash and open the serial log with `idf.py flash monitor`. The hub starts in
Bluetooth mode and advertises as **BandHero Hub**. Select that name in your
computer's Bluetooth pairing settings. Pairing uses Just Works (no PIN or
display on the hub); bonding keys are stored in NVS. After disconnecting, the
hub advertises again. The log reports advertising, pairing results, and whether
the host has enabled input notifications.

Bluetooth exposes a vendor-defined HID input report (usage page `0xff00`,
usage `1`, report ID `1`). Each notification contains the queue value as four
little-endian bytes: `0x12345678` becomes `78 56 34 12`. A host HID API may prefix
those bytes with report ID `01`. This preserves the existing packet format,
including the receive callback's instrument-index bits; it does not define a
gamepad button/axis mapping. A receiving application can read the raw HID
reports, or a BLE client can subscribe to the HID Report characteristic
(`0x2a4d`) in the HID service (`0x1812`) after pairing.

Set `instrumentMacs` in `main/main.cpp` to the instruments' actual Wi-Fi MAC
addresses. Only four-byte ESP-NOW packets from those addresses enter the
queue, and sender and receiver must use the same Wi-Fi channel. Reports wait
until the Bluetooth connection is encrypted and the host subscribes; failed
sends retain the pending value for retry. The existing queue holds 40 values
(plus one pending send); when full, new arrivals are dropped. Buffered packets
can therefore be old when a host reconnects. BLE notifications do not provide
application-level delivery acknowledgements.

Hardware verification: pair with the hub, subscribe/read raw HID reports, send
a known four-byte ESP-NOW packet from a configured peer, and check the bytes
above (including the existing instrument-index encoding). Disconnect and
reconnect to verify advertising and report delivery resume.
