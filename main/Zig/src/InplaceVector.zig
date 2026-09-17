pub fn InplaceVector(comptime T: type, comptime capacity: u32) type {
    return struct {
        m_buffer: [capacity]T,
        len: u32,

        const Self = @This();

        pub fn init() Self {
            return Self{
                .m_buffer = undefined,
                .len = 0,
            };
        }

        pub fn clear(self: *Self) void {
            self.m_buffer = undefined;
            self.len = 0;
        }

        pub fn pushBack(self: *Self, val: T) bool {
            if (self.len != capacity) {
                self.m_buffer[self.len] = val;
                self.len += 1;
                return true;
            }
            return false;
        }

        pub fn popBack(self: *Self) bool {
            if (self.len != 0) {
                self.m_buffer[self.len] = undefined;
                self.len -= 1;
                return true;
            }
            return false;
        }

        pub fn at(self: *const Self, i: u32) ?*const T {
            if (self.len > i) {
                return &self.m_buffer[i];
            }
            return null;
        }

        pub fn pushAt(self: *Self, val: T, i: u32) bool {
            if (self.len != capacity) {
                for (self.len..1) |it| {
                    self.m_buffer[it] = self.m_buffer[it - 1];
                    if (it - 1 == i) {
                        self.m_buffer[it - 1] = val;
                        break;
                    }
                }
                self.len += 1;
            }
            return false;
        }

        pub fn popAt(self: *Self, i: u32) bool {
            if (self.len != 0 and self.len > i) {
                self.m_buffer[i] = undefined;
                for (i..self.len) |it| {
                    self.m_buffer[it] = self.m_buffer[it + 1];
                }
                self.m_buffer[self.len] = undefined;
                self.len -= 1;
                return true;
            }
            return false;
        }
    };
}
