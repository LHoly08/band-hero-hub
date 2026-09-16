# band-hero-hub

Build with CMake 3.31 or newer, C++26, and Espressif's GCC 16.1.0
`riscv32-esp-elf` toolchain. The SDK used for this setup is ESP-IDF
`85c826ecb1` (6.2 development), which selects `esp-16.1.0_20260609`.
The Zig implementation uses Zig 0.15.2.

Activate the updated SDK in each terminal. Clear an inherited Python environment
override if that terminal previously used ESP-IDF 6.1:

```sh
unset IDF_PYTHON_ENV_PATH
. /home/lsdias/esp/esp-idf/export.sh
riscv32-esp-elf-g++ --version
```

From the repository root, build the hub firmware in the usual `build/` directory:

```sh
idf.py -DIMPL=cpp build
```

To build the Zig implementation instead, run `idf.py -DIMPL=zig build`. CMake
builds the Zig library automatically before linking the firmware. The selection
is remembered for subsequent `idf.py build` commands. See [the Zig build notes](main/Zig/README.md).

Flash the firmware and open the serial log:

```sh
idf.py flash monitor
```
