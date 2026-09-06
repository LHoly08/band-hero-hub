const std = @import("std");
const c = @cImport({
    @cInclude("freertos/FreeRTOS.h");
    @cInclude("freertos/queue.h");
    @cInclude("freertos/task.h");
    @cInclude("esp_now.h");
    @cInclude("esp_log.h");
    @cInclude("nvs_flash.h");
    @cInclude("stdio.h");
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
        while (self.m_state.* == .WorkingUSB) {
            while (!self.m_queue.empty()) {
                var val: u32 = undefined;
                if (self.m_queue.receive(&val)) {
                    _ = c.fwrite(&val, 1, @sizeOf(@TypeOf(val)), c.stdout);
                    _ = c.fflush(c.stdout);
                }
            }
            c.vTaskDelay(c.pdMS_TO_TICKS(1));
        }
    }

    pub fn deinit(self: *Self) void {
        self.m_queue.deinit();
    }
};

export fn app_main() void {
    var state: State = .Configuring;

    while (true) {
        switch (state) {
            State.Configuring => {
                _ = c.fwrite("Hello Zig\n", 1, 10, c.stdout);
                c.vTaskDelay(1000);
            },
            State.WorkingUSB => {
                var hub: USBHub = USBHub.init(&state);
                defer hub.deinit();

                hub.loop();
            },
        }
    }
}
