-- Bounded reads from a selected MAME memory-space window.

local machine = manager.machine
local output = assert(os.getenv('STUNRUN_RAM_READ_OUT'), 'STUNRUN_RAM_READ_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_RAM_READ_FRAMES') or '1800')
local start_frame = tonumber(os.getenv('STUNRUN_RAM_READ_START') or '600')
local end_frame = tonumber(os.getenv('STUNRUN_RAM_READ_END') or tostring(limit))
local input_mode = os.getenv('STUNRUN_RAM_READ_INPUT') or 'none'
local device_tag = assert(os.getenv('STUNRUN_RAM_READ_DEVICE'), 'STUNRUN_RAM_READ_DEVICE is required')
local space_name = os.getenv('STUNRUN_RAM_READ_SPACE') or 'program'
local start_address = assert(tonumber(os.getenv('STUNRUN_RAM_READ_BASE')), 'base is required')
local end_address = assert(tonumber(os.getenv('STUNRUN_RAM_READ_END_ADDRESS')), 'end address is required')
local tap_end_address = end_address + (end_address % 2 == 0 and 1 or 0)
local max_events = tonumber(os.getenv('STUNRUN_RAM_READ_MAX_EVENTS') or '100000')
local pc_filter_text = os.getenv('STUNRUN_RAM_READ_PC') or ''
local pc_filter = pc_filter_text ~= '' and tonumber(pc_filter_text) or nil
local frame = 0
local events = {}
local input_events = input_mode == 'late_drive' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', value = 1},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', value = 0},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', value = 0},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 1},
    {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 128},
    {frame = 1500, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0}
} or input_mode == 'weapon_probe' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', value = 1},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', value = 0},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', value = 0},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 1},
    {frame = 960, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0},
    {frame = 1100, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 1},
    {frame = 1160, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 0},
    {frame = 1250, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', value = 220},
    {frame = 1350, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', value = 128},
    {frame = 1400, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 1},
    {frame = 1460, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0},
    {frame = 1550, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 1},
    {frame = 1610, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 0}
} or input_mode == 'course_sweep' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', value = 1},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', value = 0},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', value = 0},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 1},
    {frame = 960, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0},
    {frame = 1050, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 1},
    {frame = 1110, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 0},
    {frame = 1200, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 0},
    {frame = 1260, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 128},
    {frame = 1350, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 255},
    {frame = 1410, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 128},
    {frame = 1500, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', value = 0},
    {frame = 1560, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', value = 128},
    {frame = 1650, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', value = 255},
    {frame = 1710, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', value = 128},
    {frame = 1800, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 1860, port = ':mainpcb:a80000', field = '1 Player Start', value = 0},
    {frame = 1950, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 1},
    {frame = 1950, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 1},
    {frame = 2010, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0},
    {frame = 2010, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 0}
} or input_mode == 'course_coin2' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 2', value = 1},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 2', value = 0},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', value = 0}
} or input_mode == 'service_probe' and {
    {frame = 100, port = ':mainpcb:IN0', field = 'Service Mode', value = 1},
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', value = 1},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', value = 0},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', value = 0},
    {frame = 900, port = ':mainpcb:IN0', field = 'Service Mode', value = 0}
} or input_mode == 'sw1_all_on' and {
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:1', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:2', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:3', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:4', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:5', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:6', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:7', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:8', value = 1},
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', value = 1},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', value = 0},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', value = 0}
} or input_mode == 'fork_hold_left' and {
    {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 0},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 0}
} or input_mode == 'fork_center' and {
    {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 128},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2', value = 0},
    {frame = 1800, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 128}
} or {}
local next_event = 1
local tap

local function apply_event(event)
    local port = assert(machine.ioport.ports[event.port], 'unknown input port: ' .. event.port)
    local field = assert(port.fields[event.field], 'unknown input field: ' .. event.port .. '/' .. event.field)
    field:set_value(event.value)
end

local function install_tap()
    local device = assert(machine.devices[device_tag], 'unknown device: ' .. device_tag)
    local space = assert(device.spaces[space_name], 'unknown space: ' .. space_name)
    local pc = device.state['CURPC'] or device.state['PC']
    tap = space:install_read_tap(start_address, tap_end_address, 'stunrun_ram_read_trace',
        function(offset, data, mask)
            if frame >= start_frame and frame <= end_frame and offset >= start_address and
                offset <= end_address and (pc_filter == nil or pc.value == pc_filter) and
                #events < max_events then
                events[#events + 1] = {frame = frame, pc = pc.value, address = offset,
                                       data = data, mask = mask}
            end
        end)
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame == 10 then install_tap() end
    while next_event <= #input_events and input_events[next_event].frame == frame do
        apply_event(input_events[next_event])
        next_event = next_event + 1
    end
    if frame >= limit then
        for _, event in ipairs(events) do
            print(string.format('M1_RAM_READ frame=%d pc=%08X addr=%08X data=%08X mask=%08X',
                event.frame, event.pc, event.address, event.data, event.mask))
        end
        print(string.format('M1_RAM_READ_DONE frames=%d events=%d truncated=%s output=%s',
            frame, #events, tostring(#events >= max_events), output))
        machine:exit()
    end
end)
