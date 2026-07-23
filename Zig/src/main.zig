const std = @import("std");
const c = @cImport({
    @cInclude("freertos/FreeRTOS.h");
    @cInclude("freertos/queue.h");
    @cInclude("esp_now.h");
    @cInclude("esp_log.h");
    @cInclude("nvs_flash.h");
});

fn Queue(comptime T: type, comptime size: c_ulonglong) type {
    return struct {
        handle: c.QueueHandle_t,
        const Self = @This();

        pub fn init() ?Self {
            const q_handle = c.xQueueCreate(size, @sizeOf(T));

            if (q_handle == null) return null;

            return Self{ .handle = q_handle };
        }

        pub fn deinit(self: *Self) void {
            if (self.handle != null) {
                c.vQueueDelete(self.handle);
            }
        }

        pub fn empty(self: *Self) bool {
            return (c.uxQueueSpacesAvailable(self.handle) == size);
        }

        pub fn send(self: *Self, item: *const T) bool {
            return (c.xQueueSendToBack(self.handle, @ptrCast(item), 0) == c.pdTRUE);
        }

        pub fn sendISR(self: *Self, item: *const T) bool {
            var success: c.BaseType_t = c.pdFALSE;

            var xHigherPriorityTaskWoken: c.BaseType_t = c.pdFALSE;

            success = c.xQueueSendToBackFromISR(self.handle, @ptrCast(item), &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken == c.pdTRUE) {
                c.portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            return (success == c.pdTRUE);
        }

        pub fn receive(self: *Self, out_item: *T) bool {
            return (c.xQueueReceive(self.handle, @ptrCast(out_item), 0) == c.pdTRUE);
        }

        pub fn receiveISR(self: *Self, out_item: *T) bool {
            var success: c.BaseType_t = c.pdFALSE;

            var xHigherPriorityTaskWoken: c.BaseType_t = c.pdFALSE;
            success = c.xQueueReceiveFromISR(self.handle, @ptrCast(out_item), &xHigherPriorityTaskWoken);

            if (xHigherPriorityTaskWoken == c.pdTRUE) {
                c.portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            return (success == c.pdTRUE);
        }
    };
}

const State = enum {
    Configuring,
    WorkingUSB,
    WorkingBT,
};

const BluetoothHub = struct {
    m_queue: Queue(u32, 40),
    m_state: *State,

    const Self = @This();

    pub fn init(initState: *State) BluetoothHub {
        const q = Queue(u32, 40).init() orelse unreachable;

        return BluetoothHub{
            .m_queue = q,
            .m_state = initState,
        };
    }

    pub fn loop(self: *Self) void {
        while (self.m_state.* == State.WorkingBT) {
            while (!self.m_queue.empty()) {}
        }
    }

    pub fn deinit(self: *Self) void {
        self.m_queue.deinit();
    }

    pub fn idk() void {}
};

const USBHub = struct {
    m_queue: Queue(u32, 40),
    m_state: *State,

    const Self = @This();

    pub fn init(initState: *State) USBHub {
        const q = Queue(u32, 40).init() orelse unreachable;

        return USBHub{
            .m_queue = q,
            .m_state = initState,
        };
    }

    pub fn loop(self: *Self) void {
        self.m_state.* = .Configuring;
    }

    pub fn deinit(self: *Self) void {
        self.m_queue.deinit();
    }
};

export fn app_main() void {
    var state: State = .Configuring;

    while (true) {
        switch (state) {
            State.Configuring => {},
            State.WorkingUSB => {
                var hub: USBHub = USBHub.init(&state);
                defer hub.deinit();

                hub.loop();
            },
            State.WorkingBT => {
                var hub: BluetoothHub = BluetoothHub.init(&state);
                defer hub.deinit();

                hub.loop();
            },
        }
    }
}
