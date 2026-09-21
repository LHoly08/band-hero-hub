pub const c = @cImport({
    @cInclude("esp_mac.h");
    @cInclude("driver/gpio.h");
    @cInclude("esp_err.h");
    @cInclude("freertos/FreeRTOS.h");
    @cInclude("freertos/task.h");
    @cInclude("esp32_hw_i2c.h");
});

// Native-width flags avoid out-of-line byte atomic helpers on ESP32-C6.
pub const State = enum(u32) {
    Configuring,
    WorkingUSB,
};

pub const PIN: c.gpio_num_t = c.GPIO_NUM_20;
pub const RESET_PIN: c.gpio_num_t = c.GPIO_NUM_19;

pub const BUTTON_DEBOUNCE_MS: u32 = 200;
const BUTTON_DEBOUNCE_TICKS: u32 = (BUTTON_DEBOUNCE_MS * c.configTICK_RATE_HZ + 999) / 1000;

pub const ButtonDebounce = struct {
    last_press_tick: u32 = 0,
    has_press: bool = false,

    pub inline fn accept(self: *ButtonDebounce) bool {
        const now: u32 = c.xTaskGetTickCountFromISR();
        // Wrapping subtraction also handles tick-counter wraparound.
        if (self.has_press and now -% self.last_press_tick < BUTTON_DEBOUNCE_TICKS) return false;
        self.last_press_tick = now;
        self.has_press = true;
        return true;
    }
};

// Accessed only by the serialized GPIO ISRs; persist across mode changes.
pub var mode_button_debounce: ButtonDebounce = .{};
pub var reset_button_debounce: ButtonDebounce = .{};

pub extern fn zig_wifi_init_default() c.esp_err_t;

pub extern fn zig_esp_error_check(c.esp_err_t) void;

pub extern fn zig_u8g2_esp32_i2c_config_default() c.u8g2_esp32_i2c_config_t;

pub extern fn zig_u8g2_set_i2c_address(*c.u8g2_t, u8) void;
