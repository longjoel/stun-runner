-- Bounded 68010 tap for the observed GSP FIFO sink.

local output = assert(os.getenv('STUNRUN_GSP_BOUNDARY_OUT'), 'STUNRUN_GSP_BOUNDARY_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_GSP_BOUNDARY_FRAMES') or '600')
local input_mode = os.getenv('STUNRUN_GSP_BOUNDARY_INPUT') or 'none'
local frame = 0
local capture_start_frame = tonumber(os.getenv('STUNRUN_GSP_BOUNDARY_CAPTURE_START') or '132')
local first_limit = 32
local first = {}
local command_first = {}
local sw_off_prefix = {
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:1', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:2', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:3', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:4', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:5', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:6', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:7', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:8', action = 'set', value = 1}
}
local events = input_mode == 'coin_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'sw_off' and {
    sw_off_prefix[1], sw_off_prefix[2], sw_off_prefix[3], sw_off_prefix[4],
    sw_off_prefix[5], sw_off_prefix[6], sw_off_prefix[7], sw_off_prefix[8],
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'sw_off_prestart' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'sw_off_none' and sw_off_prefix or {}
local next_event = 1

local function apply_event(event)
    local port = manager.machine.ioport.ports[event.port]
    assert(port ~= nil, 'unknown input port: ' .. event.port)
    local field = port.fields[event.field]
    assert(field ~= nil, 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.action == 'press' then field:set_value(1) else field:set_value(0) end
    print('M1_GSP_INPUT frame=' .. frame .. ' field=' .. event.field .. ' action=' .. event.action)
end

if input_mode == 'sw_off_prestart' then
    emu.register_prestart(function()
        for _, event in ipairs(sw_off_prefix) do
            apply_event(event)
        end
    end)
end

local function install_tap()
    local cpu = manager.machine.devices[':mainpcb:maincpu']
    local space = cpu.spaces['program']
    local pc = cpu.state['CURPC']
    -- The installed MAME Lua bridge is not stable under a broad, high-rate
    -- device-window tap. The FIFO subrange is the proven stable boundary and
    -- is sufficient to identify runtime submission PCs and ordering.
    local function record_command(offset, data, mask)
        if frame < capture_start_frame or #command_first >= first_limit then return end
        command_first[#command_first + 1] = {
            frame = frame, pc = pc.value, address = offset, data = data, mask = mask,
            post_init = pc.value > 0x4f4
        }
    end
    space:install_write_tap(0xc00002, 0xc00003, 'stunrun_gsp_command', record_command)
    space:install_write_tap(0xc0000c, 0xc0000f, 'stunrun_gsp_fifo',
        function(offset, data, mask)
            if frame < capture_start_frame or #first >= first_limit then return end
            local pc_value = pc.value
            first[#first + 1] = {
                frame = frame, pc = pc_value, address = offset, data = data, mask = mask,
                post_init = pc_value > 0x4f4
            }
        end)
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame == 10 then install_tap() end
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if frame >= limit then
        for _, row in ipairs(first) do
            print(string.format('M1_GSP_WRITE_FIRST frame=%d pc=%08X addr=%08X data=%08X mask=%08X post_init=%s',
                row.frame, row.pc, row.address, row.data, row.mask, tostring(row.post_init)))
        end
        for _, row in ipairs(command_first) do
            print(string.format('M1_GSP_COMMAND_FIRST frame=%d pc=%08X addr=%08X data=%08X mask=%08X post_init=%s',
                row.frame, row.pc, row.address, row.data, row.mask, tostring(row.post_init)))
        end
        print('M1_GSP_BOUNDARY_DONE frames=' .. frame .. ' output=' .. output)
        manager.machine:exit()
    end
end)
