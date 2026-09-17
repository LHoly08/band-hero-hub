const std = @import("std");

pub fn build(b: *std.Build) void {
    const idf_build = b.option([]const u8, "idf-build-dir", "Configured ESP-IDF build directory") orelse "../../build";
    const config_dir = b.pathJoin(&.{ idf_build, "config" });
    const config = std.fs.cwd().readFileAlloc(b.allocator, b.pathJoin(&.{ config_dir, "sdkconfig.h" }), 1024 * 1024) catch
        @panic("Missing ESP-IDF sdkconfig.h; configure the parent project first, or set -Didf-build-dir");
    if (std.mem.indexOf(u8, config, "#define CONFIG_IDF_TARGET_ESP32C6 1") == null)
        @panic("This Zig library currently targets ESP32-C6; configure ESP-IDF for esp32c6");

    const target = b.resolveTargetQuery(.{
        .cpu_arch = .riscv32,
        .os_tag = .freestanding,
        .cpu_model = .{ .explicit = &std.Target.riscv.cpu.generic_rv32 },
        .cpu_features_add = std.Target.riscv.featureSet(&.{ .m, .a, .c, .zicsr, .zifencei }),
    });
    const main_module = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = b.option(std.builtin.OptimizeMode, "optimize", "Optimization mode") orelse .ReleaseSmall,
    });

    // Reuse ESP-IDF's component include paths, including generated configuration.
    const database = std.fs.cwd().readFileAlloc(b.allocator, b.pathJoin(&.{ idf_build, "compile_commands.json" }), 32 * 1024 * 1024) catch
        @panic("Missing compile_commands.json; configure the parent ESP-IDF project first");
    const Entry = struct { directory: []const u8, file: []const u8, command: []const u8 };
    const entries = std.json.parseFromSlice([]Entry, b.allocator, database, .{ .ignore_unknown_fields = true }) catch
        @panic("Invalid ESP-IDF compile_commands.json");
    for (entries.value) |entry| {
        if (!std.mem.endsWith(u8, entry.file, "/main/zig_headers.c")) continue;
        var args = std.process.ArgIteratorGeneral(.{}).init(b.allocator, entry.command) catch @panic("OOM");
        const compiler = args.next() orelse @panic("Missing ESP-IDF compiler");
        const compiler_dir = std.fs.path.dirname(compiler) orelse @panic("Expected an absolute ESP-IDF compiler path");
        // ESP-IDF 6.x uses picolibc; older configurations use newlib.
        const libc_include = b.option([]const u8, "libc-include-dir", "ESP toolchain C library headers") orelse
            if (std.mem.indexOf(u8, config, "#define CONFIG_LIBC_PICOLIBC 1") != null)
                b.pathJoin(&.{ compiler_dir, "..", "picolibc", "include" })
            else
                b.pathJoin(&.{ compiler_dir, "..", "riscv32-esp-elf", "include" });
        while (args.next()) |arg| {
            if (std.mem.startsWith(u8, arg, "-I")) {
                const path = if (arg.len == 2) args.next() orelse @panic("Missing include path") else arg[2..];
                main_module.addIncludePath(.{ .cwd_relative = if (std.fs.path.isAbsolute(path)) b.dupe(path) else b.pathJoin(&.{ entry.directory, path }) });
            }
        }
        main_module.addSystemIncludePath(.{ .cwd_relative = libc_include });
        main_module.addCMacro("ESP_PLATFORM", "1");
        main_module.addCMacro("__PICOLIBC_ERRNO_FUNCTION", "__errno");
        main_module.addCMacro("__STDC_WANT_LIB_EXT1__", "0");
        const lib = b.addLibrary(.{ .name = "zig_app", .linkage = .static, .root_module = main_module });
        // ESP-IDF supplies libc and FreeRTOS when it links the final firmware.
        b.installArtifact(lib);
        return;
    }
    @panic("No main/zig_headers.c entry found; reconfigure the parent ESP-IDF project");
}
