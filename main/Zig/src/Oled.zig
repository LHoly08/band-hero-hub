const bytesToHex = @import("std").fmt.bytesToHex;

const zig_u8g2_esp32_i2c_config_default = @import("Util.zig").zig_u8g2_esp32_i2c_config_default;

const c = @import("Util.zig").c;

const zig_esp_error_check = @import("Util.zig").zig_esp_error_check;

pub const Oled = struct {
    const Self = @This();

    m_oled: c.u8g2_t,
    m_mac: [23]u8,
    m_i2c: c.u8g2_esp32_i2c_ctx_t = .{},

    pub fn Init() Oled {
        var mac: [6]u8 = undefined;
        zig_esp_error_check(c.esp_read_mac(&mac, c.ESP_MAC_WIFI_STA));

        const hex: [12]u8 = bytesToHex(mac, .upper);
        var buffer: [23]u8 = [_]u8{':'} ** 23;
        buffer[22] = 0;

        buffer[0] = 'M';
        buffer[1] = 'A';
        buffer[2] = 'C';
        buffer[3] = ':';
        buffer[4] = ' ';

        for (0..6) |i| {
            const indexB: usize = (i * 3) + 5;
            const indexH: usize = i * 2;
            buffer[indexB] = hex[indexH];
            buffer[indexB + 1] = hex[indexH + 1];
        }

        return Oled{
            .m_oled = .{},
            .m_mac = buffer,
        };
    }

    pub fn Start(self: *Self) void {
        const i2c = &self.m_i2c;
        const oledAddress: comptime_int = 0x3D;

        i2c.cfg = zig_u8g2_esp32_i2c_config_default();
        i2c.cfg.sda_pin = c.GPIO_NUM_22;
        i2c.cfg.scl_pin = c.GPIO_NUM_23;
        i2c.cfg.clk_hz = 100000;
        i2c.cfg.dev_addr_7bit = oledAddress;
        i2c.cfg.reset_pin = c.U8G2_ESP32_PIN_UNUSED;

        zig_esp_error_check(c.u8g2_esp32_i2c_set_default_context(i2c));

        c.u8g2_Setup_sh1106_128x64_noname_f(&self.m_oled, c.U8G2_R0, c.u8x8_byte_esp32_hw_i2c, c.u8x8_gpio_and_delay_esp32_i2c);

        @import("Util.zig").zig_u8g2_set_i2c_address(&self.m_oled, oledAddress << 1);
        c.u8g2_InitDisplay(&self.m_oled);
        c.u8g2_SetPowerSave(&self.m_oled, 0);
    }

    pub fn clearBuffer(self: *Self) void {
        c.u8g2_ClearBuffer(&self.m_oled);
    }
    pub fn sendBuffer(self: *Self) void {
        c.u8g2_SendBuffer(&self.m_oled);
    }

    pub fn drawLine(self: *Self, mac: [6]u8, line: u8) void {
        c.u8g2_SetFont(&self.m_oled, c.u8g2_font_6x10_tf);

        const hex: [12]u8 = bytesToHex(mac, .upper);
        var text: [22]u8 = [_]u8{':'} ** 22;
        text[21] = 0;

        text[0] = line + '1';
        text[1] = ' ';
        text[2] = '-';
        text[3] = ' ';

        for (0..6) |i| {
            const indexB: usize = (i * 3) + 4;
            const indexH: usize = i * 2;
            text[indexB] = hex[indexH];
            text[indexB + 1] = hex[indexH + 1];
        }

        _ = c.u8g2_DrawStr(&self.m_oled, 0, 8 + (line * 9), &text);
    }

    pub fn drawBase(self: *Self) void {
        c.u8g2_SetFont(&self.m_oled, c.u8g2_font_5x8_tf);

        _ = c.u8g2_DrawStr(&self.m_oled, 0, 56, &self.m_mac);
        _ = c.u8g2_DrawStr(&self.m_oled, 0, 0, "Peers:");
    }
};
