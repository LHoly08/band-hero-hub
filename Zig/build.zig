const std = @import("std");

pub fn build(b: *std.Build) void {
    // 1. Precise ESP32-C3 Target Query (RV32IMC - No Atomic Extension)
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .riscv32,
        .os_tag = .freestanding,
        .cpu_model = .{ .explicit = &std.Target.riscv.cpu.generic_rv32 },
        .cpu_features_add = std.Target.riscv.featureSet(&.{
            .m, // Multiply/Divide
            .c, // Compressed instructions (Crucial for flash size optimization)
        }),
    });

    const optimize = std.builtin.OptimizeMode.ReleaseFast;

    // 2. Setup the Module Core
    const main_module = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    // 3. Resolve Environment Paths safely
    var env_map = b.allocator.create(std.process.EnvMap) catch @panic("OOM");
    env_map.* = std.process.getEnvMap(b.allocator) catch @panic("Failed to read env");
    const idf_path = env_map.get("IDF_PATH") orelse "C:/Espressif/frameworks/esp-idf";

    // 4. ESP-IDF Component Header Layout Array
    const includes = &[_][]const u8{
        "components/esp_wifi/include",
        "components/freertos/FreeRTOS-Kernel/include",
        "components/freertos/config/riscv/include", // Required for FreeRTOS RISC-V portmacro.h
        "components/freertos/esp_additions/include",
        "components/log/include",
        "components/nvs_flash/include",
        "components/esp_common/include",
        "components/soc/esp32c3/include",
        "components/hal/include",
    };

    // Loop through and register all paths directly to your core module context
    for (includes) |inc_path| {
        const full_path = std.fs.path.join(b.allocator, &.{ idf_path, inc_path }) catch @panic("OOM");
        main_module.addIncludePath(.{ .cwd_relative = full_path });
    }

    // Append the local generated build configurations folder so @cImport finds FreeRTOSConfig.h
    main_module.addIncludePath(b.path("build/config"));

    // 5. Create the Static Library Artifact linking our custom module
    const lib = b.addLibrary(.{
        .name = "zig_app",
        .linkage = .static,
        .root_module = main_module,
    });

    lib.linkLibC();

    b.installArtifact(lib);
}
