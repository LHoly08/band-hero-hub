const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .riscv32,
        .os_tag = .freestanding,
        .cpu_model = .{ .explicit = &std.Target.riscv.cpu.generic_rv32 },
        .cpu_features_add = std.Target.riscv.featureSet(&.{
            .m,
            .a,
            .c,
        }),
    });

    const lib = b.addLibrary(.{
        .name = "zig_app",
        .linkage = .static,
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = std.builtin.OptimizeMode.ReleaseFast,
        }),
        .use_llvm = true,
        .use_lld = true,
    });

    lib.addIncludePath(.{ .cwd_relative = "C:/Users/santo/esp/esp-idf/components/esp_wifi/include" });
    lib.addIncludePath(.{ .cwd_relative = "C:/Users/santo/esp/esp-idf/components/freertos/FreeRTOS-Kernel/include" });

    b.installArtifact(lib);
}
