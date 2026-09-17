pub const c = @cImport({
    @cInclude("esp_mac.h");
    @cInclude("driver/gpio.h");
    @cInclude("esp_err.h");
    @cInclude("esp32_hw_i2c.h");
});

pub const State = enum(u8) {
    Configuring,
    WorkingUSB,
};

pub const PIN: c.gpio_num_t = c.GPIO_NUM_0;

pub extern fn zig_wifi_init_default() c.esp_err_t;

pub extern fn zig_esp_error_check(c.esp_err_t) void;

pub extern fn zig_u8g2_esp32_i2c_config_default() c.u8g2_esp32_i2c_config_t;

pub extern fn zig_u8g2_set_i2c_address(*c.u8g2_t, u8) void;
