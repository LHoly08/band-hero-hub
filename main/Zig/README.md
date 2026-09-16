Build with Zig 0.15.2 from this directory:

```sh
zig build
```

The ESP-IDF project at the repository root must already be configured for ESP32-C6. The build
reads `../../build/config/sdkconfig.h` and `../../build/compile_commands.json` to reuse
its component include paths and locate the ESP toolchain's C library headers.
Reconfigure ESP-IDF if you move the project or change toolchains.

Optional overrides:

```sh
zig build -Doptimize=Debug
zig build -Didf-build-dir=/path/to/esp-idf/build
zig build -Dlibc-include-dir=/path/to/toolchain/libc/include
```

The standalone output is `zig-out/lib/libzig_app.a`. With ESP-IDF activated,
build the complete Zig firmware from the repository root:

```sh
idf.py -DIMPL=zig build
```

CMake runs Zig automatically, then links its library and the C wrappers in
`main/zig_headers.c`. The CMake-managed archive is stored under the selected
ESP-IDF build directory, in `esp-idf/main/zig-out/lib/libzig_app.a`.
Zig checks its cache on every firmware build, including changes to imported C headers.

Select the C++ implementation with `idf.py -DIMPL=cpp build` (the default for a
new build directory). The selected implementation is remembered in the CMake cache.

For a standalone `zig build`, first configure the parent project using
`idf.py -DIMPL=zig reconfigure`. If you use a custom ESP-IDF build directory,
pass the same directory to Zig with `-Didf-build-dir=/path/to/build`.
