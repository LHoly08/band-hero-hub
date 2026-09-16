const c = @cImport({
    @cInclude("freertos/FreeRTOS.h");
    @cInclude("freertos/queue.h");
});

pub fn Queue(comptime T: type, comptime size: c_ulonglong) type {
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
            return (c.xQueueSendToBack(self.handle, @as(*const anyopaque, @ptrCast(item)), 0) == c.pdTRUE);
        }

        pub fn sendISR(self: *Self, item: *const T) bool {
            var success: c.BaseType_t = c.pdFALSE;

            var xHigherPriorityTaskWoken: c.BaseType_t = c.pdFALSE;

            success = c.xQueueSendToBackFromISR(self.handle, @as(*const anyopaque, @ptrCast(item)), &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken == c.pdTRUE) {
                c.vPortYieldFromISR();
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
                c.vPortYieldFromISR();
            }
            return (success == c.pdTRUE);
        }
    };
}
