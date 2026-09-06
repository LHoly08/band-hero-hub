Build with Zig 0.15.2 from this directory:

```sh
zig build
```

The parent ESP-IDF project must already be configured for ESP32-C6. The build
reads `../build/config/sdkconfig.h` and `../build/compile_commands.json` to reuse
its component include paths and locate the ESP toolchain's C library headers.
Reconfigure ESP-IDF if you move the project or change toolchains.

Optional overrides:

```sh
zig build -Doptimize=Debug
zig build -Didf-build-dir=/path/to/esp-idf/build
zig build -Dlibc-include-dir=/path/to/toolchain/libc/include
```

The output is `zig-out/lib/libzig_app.a`. The parent CMake project links this
archive instead of compiling the C++ application. Its original configuration is
preserved as comments in `main/CMakeLists.txt`; `main/zig_headers.c` keeps the
component's include paths available in the compilation database.

From the parent project directory, with ESP-IDF activated:

```sh
idf.py reconfigure
(cd Zig && zig build)
idf.py build
```

Rebuild the Zig archive before building firmware whenever Zig sources change.
The Zig application currently remains in its empty `Configuring` state; this
build integration does not implement the C++ application's runtime behavior.
