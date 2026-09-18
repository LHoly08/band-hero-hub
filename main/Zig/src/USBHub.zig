const readInt = @import("std").mem.readInt;
const eql = @import("std").mem.eql;

const Queue = @import("Queue.zig").Queue;
const InplaceVector = @import("InplaceVector.zig").InplaceVector;

const State = @import("Util.zig").State;
const PIN = @import("Util.zig").PIN;

const zig_esp_error_check = @import("Util.zig").zig_esp_error_check;

const c = @cImport({
    @cInclude("freertos/FreeRTOS.h");
    @cInclude("esp_wifi.h");
    @cInclude("driver/gpio.h");
    @cInclude("esp_intr_alloc.h");
    @cInclude("stdio.h");
    @cInclude("esp_now.h");
    @cInclude("freertos/task.h");
});

pub const USBHub = struct {
    const Self = @This();

    m_queue: Queue(u32, 40),
    m_state: *State,
    m_macs: *const InplaceVector([6]u8, 4),

    var instance: ?*Self = null;

    pub fn init(initState: *State, macs: *const InplaceVector([6]u8, 4)) USBHub {
        const q = Queue(u32, 40).init() orelse unreachable;

        return USBHub{
            .m_queue = q,
            .m_state = initState,
            .m_macs = macs,
        };
    }

    pub fn start(self: *Self) void {
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

        zig_esp_error_check(c.gpio_isr_handler_add(PIN, Self.ButtonPressed, self.m_state));
        Self.instance = self;
        zig_esp_error_check(c.esp_now_register_recv_cb(Self.ReceivedCallback));
    }

    pub fn loop(self: *Self) void {
        while (@atomicLoad(State, self.m_state, .monotonic) == .WorkingUSB) {
            var val: u32 = undefined;

            if (self.m_queue.receive(&val)) {
                _ = c.fwrite(&val, 1, @sizeOf(@TypeOf(val)), c.stdout);
                _ = c.fflush(c.stdout);
            }
        }
    }

    pub fn deinit(self: *Self) void {
        _ = c.esp_now_unregister_recv_cb();
        _ = c.esp_now_deinit();

        _ = c.esp_wifi_stop();
        _ = c.esp_wifi_deinit();

        _ = c.gpio_isr_handler_remove(PIN);
        _ = c.gpio_uninstall_isr_service();

        Self.instance = null;

        self.m_queue.deinit();
    }

    fn ButtonPressed(args: ?*anyopaque) linksection(".iram1.usbhub_button") callconv(.c) void {
        if (args) |arg| {
            const state: *State = @ptrCast(@alignCast(arg));
            @atomicStore(State, state, .Configuring, .monotonic);
        }
    }

    fn ReceivedCallback(esp_now_info: [*c]const c.esp_now_recv_info_t, data: [*c]const u8, data_len: c_int) callconv(.c) void {
        if (esp_now_info == null or
            esp_now_info.*.src_addr == null or data == null or
            data_len != @sizeOf(u32))
        {
            return;
        }
        if (Self.instance) |self| {
            const mac: [6]u8 = esp_now_info.*.src_addr[0..6].*;

            for (0..self.m_macs.len) |i| {
                const macAddr = self.m_macs.at(i);
                if (macAddr != null and eql(u8, macAddr.?, &mac)) {
                    const value: u32 = readInt(u32, @as(*const [4]u8, @ptrCast(data)), .little) | i;
                    _ = self.m_queue.send(&value);
                }
            }
        }
    }
};
