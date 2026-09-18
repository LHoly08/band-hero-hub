const eql = @import("std").mem.eql;

const Queue = @import("Queue.zig").Queue;
const InplaceVector = @import("InplaceVector.zig").InplaceVector;

const State = @import("Util.zig").State;
const PIN = @import("Util.zig").PIN;

const Oled = @import("Oled.zig").Oled;

const c = @cImport({
    @cInclude("freertos/FreeRTOS.h");
    @cInclude("esp_wifi.h");
    @cInclude("driver/gpio.h");
    @cInclude("esp_intr_alloc.h");
    @cInclude("stdio.h");
    @cInclude("esp_now.h");
    @cInclude("freertos/task.h");
});

const zig_esp_error_check = @import("Util.zig").zig_esp_error_check;

pub const ConfigHub = struct {
    const Self = @This();

    m_queue: Queue([6]u8, 3),
    m_oled: Oled,
    m_macs: *InplaceVector([6]u8, 4),
    m_state: *State,
    m_reset: u32,

    var instance: ?*Self = null;

    pub fn init(initState: *State, macs: *InplaceVector([6]u8, 4)) ConfigHub {
        const q = Queue([6]u8, 3).init() orelse unreachable;

        return ConfigHub{
            .m_queue = q,
            .m_oled = Oled.Init(),
            .m_macs = macs,
            .m_state = initState,
            .m_reset = 0,
        };
    }

    pub fn start(self: *Self) void {
        //self.m_oled.Start();

        zig_esp_error_check(c.gpio_install_isr_service(c.ESP_INTR_FLAG_IRAM));
        {
            zig_esp_error_check(@import("Util.zig").zig_wifi_init_default());
            zig_esp_error_check(c.esp_wifi_set_mode(c.WIFI_MODE_STA));
            zig_esp_error_check(c.esp_wifi_start());

            zig_esp_error_check(c.esp_now_init());
        }
        {
            var cfg: c.gpio_config_t = .{};

            cfg.pin_bit_mask = 1 << PIN;
            cfg.mode = c.GPIO_MODE_INPUT;
            cfg.pull_up_en = c.GPIO_PULLUP_DISABLE;
            cfg.pull_down_en = c.GPIO_PULLDOWN_ENABLE;
            cfg.intr_type = c.GPIO_INTR_POSEDGE;

            zig_esp_error_check(c.gpio_config(&cfg));
        }
        const ResetPin = @import("Util.zig").RESET_PIN;
        {
            var cfg: c.gpio_config_t = .{};

            cfg.pin_bit_mask = 1 << ResetPin;
            cfg.mode = c.GPIO_MODE_INPUT;
            cfg.pull_up_en = c.GPIO_PULLUP_DISABLE;
            cfg.pull_down_en = c.GPIO_PULLDOWN_ENABLE;
            cfg.intr_type = c.GPIO_INTR_POSEDGE;

            zig_esp_error_check(c.gpio_config(&cfg));
        }

        zig_esp_error_check(c.gpio_isr_handler_add(ResetPin, Self.ResetPressed, &self.m_reset));
        zig_esp_error_check(c.gpio_isr_handler_add(PIN, Self.ButtonPressed, self.m_state));
        Self.instance = self;
        zig_esp_error_check(c.esp_now_register_recv_cb(Self.ReceivedCallback));
    }

    pub fn loop(self: *Self) void {
        while (@atomicLoad(State, self.m_state, .monotonic) == .Configuring) {
            if (@atomicRmw(u32, &self.m_reset, .Xchg, 0, .monotonic) != 0) {
                self.m_macs.clear();
            }
            var mac: [6]u8 = undefined;

            while (@atomicLoad(State, self.m_state, .monotonic) == .Configuring and
                @atomicLoad(u32, &self.m_reset, .monotonic) == 0 and self.m_queue.receive(&mac))
            {
                var exists: bool = false;

                for (0..self.m_macs.len) |i| {
                    const macAddr = self.m_macs.at(i);

                    if (macAddr != null and eql(u8, macAddr.?, &mac)) {
                        exists = true;
                        break;
                    }
                }

                if (!exists) {
                    _ = self.m_macs.pushBack(mac);
                }
            }

            //self.m_oled.clearBuffer();

            //self.m_oled.drawBase();
            //for (0..self.m_macs.len) |i| {
            //    const macAddr = self.m_macs.at(i);

            //    if (macAddr) |macAddress| {
            //        self.m_oled.drawLine(macAddress.*, @intCast(i));
            //    }
            //}

            //self.m_oled.sendBuffer();
        }
    }

    pub fn deinit(self: *Self) void {
        _ = c.esp_now_unregister_recv_cb();
        _ = c.esp_now_deinit();

        _ = c.esp_wifi_stop();
        _ = c.esp_wifi_deinit();

        _ = c.gpio_isr_handler_remove(PIN);
        _ = c.gpio_isr_handler_remove(@import("Util.zig").RESET_PIN);
        _ = c.gpio_uninstall_isr_service();

        Self.instance = null;

        self.m_queue.deinit();
    }

    fn ResetPressed(args: ?*anyopaque) linksection(".iram1.config_reset") callconv(.c) void {
        if (args) |arg| {
            const reset: *u32 = @ptrCast(@alignCast(arg));
            @atomicStore(u32, reset, 1, .monotonic);
        }
    }

    fn ButtonPressed(args: ?*anyopaque) linksection(".iram1.confighub_button") callconv(.c) void {
        if (args) |arg| {
            const state: *State = @ptrCast(@alignCast(arg));
            @atomicStore(State, state, .WorkingUSB, .monotonic);
        }
    }

    fn ReceivedCallback(esp_now_info: [*c]const c.esp_now_recv_info_t, data: [*c]const u8, data_len: c_int) callconv(.c) void {
        _ = data;
        _ = data_len;

        if (esp_now_info == null or
            esp_now_info.*.src_addr == null)
        {
            return;
        }
        if (Self.instance) |self| {
            const mac: [6]u8 = esp_now_info.*.src_addr[0..6].*;
            _ = self.m_queue.send(&mac);
        }
    }
};
