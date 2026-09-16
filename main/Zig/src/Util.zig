const c = @cImport({
    @cInclude("driver/gpio.h");
    @cInclude("esp_err.h");
});

pub const State = enum(u8) {
    Configuring,
    WorkingUSB,
};

pub const PIN: c.gpio_num_t = c.GPIO_NUM_0;

pub extern fn zig_wifi_init_default() c.esp_err_t;

pub extern fn zig_esp_error_check(c.esp_err_t) void;
