const bytesToHex = @import("std").fmt.bytesToHex;

const InplaceVector = @import("InplaceVector.zig").InplaceVector;

const State = @import("Util.zig").State;

const USBHub = @import("USBHub.zig").USBHub;
const ConfigHub = @import("ConfigHub.zig").ConfigHub;

const zig_esp_error_check = @import("Util.zig").zig_esp_error_check;

const c = @cImport({
    @cInclude("freertos/FreeRTOS.h");
    @cInclude("freertos/task.h");
    @cInclude("nvs_flash.h");
    @cInclude("esp_mac.h");
    @cInclude("esp_netif.h");
    @cInclude("esp_event.h");
    @cInclude("stdio.h");
});

export fn app_main() void {
    zig_esp_error_check(c.nvs_flash_init());
    zig_esp_error_check(c.esp_netif_init());
    zig_esp_error_check(c.esp_event_loop_create_default());

    c.vTaskDelay(c.pdMS_TO_TICKS(1000));
    {
        // Display MAC Address on Startup to Serial

        var mac: [6]u8 = undefined;
        _ = c.esp_read_mac(&mac, c.ESP_MAC_EFUSE_FACTORY);

        const hex: [12]u8 = bytesToHex(mac, .upper);
        var buffer: [18]u8 = [_]u8{':'} ** 18;
        buffer[17] = '\n';

        for (0..6) |i| {
            const indexB: usize = i * 3;
            const indexH: usize = i * 2;
            buffer[indexB] = hex[indexH];
            buffer[indexB + 1] = hex[indexH + 1];
        }

        _ = c.fwrite("MAC: ", 1, 5, c.stdout);
        _ = c.fwrite(&buffer, 1, buffer.len, c.stdout);
        _ = c.fflush(c.stdout);
    }
    c.vTaskDelay(c.pdMS_TO_TICKS(1000));

    var state: State = .Configuring;
    var macs = InplaceVector([6]u8, 4).init();

    while (true) {
        switch (@atomicLoad(State, &state, .monotonic)) {
            State.Configuring => {
                var hub: ConfigHub = ConfigHub.init(&state, &macs);
                defer hub.deinit();

                hub.start();
                hub.loop();
            },
            State.WorkingUSB => {
                var hub: USBHub = USBHub.init(&state, &macs);
                defer hub.deinit();

                hub.start();
                hub.loop();
            },
        }
    }
}
