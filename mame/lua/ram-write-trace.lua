-- Trace bounded writes to a selected memory-space window.

local machine = manager.machine
local output = assert(os.getenv('STUNRUN_RAM_TRACE_OUT'), 'STUNRUN_RAM_TRACE_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_RAM_TRACE_FRAMES') or '705')
local start_frame = tonumber(os.getenv('STUNRUN_RAM_TRACE_START') or '680')
local end_frame = tonumber(os.getenv('STUNRUN_RAM_TRACE_END') or tostring(limit))
local tap_frame = tonumber(os.getenv('STUNRUN_RAM_TRACE_TAP_FRAME') or '10')
local delay_frame_text = os.getenv('STUNRUN_RAM_TRACE_DELAY_FRAME') or ''
local delay_frame = delay_frame_text ~= '' and tonumber(delay_frame_text) or nil
local delay_seconds = tonumber(os.getenv('STUNRUN_RAM_TRACE_DELAY_SECONDS') or '0')
local input_mode = os.getenv('STUNRUN_RAM_TRACE_INPUT') or 'none'
local device_tag = assert(os.getenv('STUNRUN_RAM_TRACE_DEVICE'), 'STUNRUN_RAM_TRACE_DEVICE is required')
local space_name = os.getenv('STUNRUN_RAM_TRACE_SPACE') or 'program'
local start_address = assert(tonumber(os.getenv('STUNRUN_RAM_TRACE_BASE')), 'base is required')
local end_address = assert(tonumber(os.getenv('STUNRUN_RAM_TRACE_END_ADDRESS')), 'end address is required')
-- MAME's 68010 write-tap API requires an inclusive range ending on the
-- final byte of a bus word. Keep the requested range exact in the callback,
-- but widen an even end by one byte for tap installation.
local tap_end_address = end_address + (end_address % 2 == 0 and 1 or 0)
local max_events = tonumber(os.getenv('STUNRUN_RAM_TRACE_MAX_EVENTS') or '50000')
local pc_filter_text = os.getenv('STUNRUN_RAM_TRACE_PC') or ''
local pc_filter = pc_filter_text ~= '' and tonumber(pc_filter_text) or nil
local capture_registers = os.getenv('STUNRUN_RAM_TRACE_REGISTERS') == '1'
local read_base_text = os.getenv('STUNRUN_RAM_READ_BASE') or ''
local read_end_text = os.getenv('STUNRUN_RAM_READ_END') or ''
local read_start_address = read_base_text ~= '' and tonumber(read_base_text) or nil
local read_end_address = read_end_text ~= '' and tonumber(read_end_text) or nil
local read_tap_end_address = read_end_address ~= nil and
    read_end_address + (read_end_address % 2 == 0 and 1 or 0) or nil
local read_max_events = tonumber(os.getenv('STUNRUN_RAM_READ_MAX_EVENTS') or '50000')
local read_pc_text = os.getenv('STUNRUN_RAM_READ_PC') or ''
local read_pc_filter = read_pc_text ~= '' and tonumber(read_pc_text) or nil
local frame = 0
local events = {}
local read_events = {}
local next_event = 1
local tap
local read_tap

local input_events = input_mode == 'fork_button2_sweep' and {
    {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'press'},
    {frame = 600, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'},
    {frame = 600, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255},
    {frame = 1200, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 1800, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255}
} or input_mode == 'fork_hold_left' and {
    {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'}
} or input_mode == 'fork_hold_right' and {
    {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'}
} or input_mode == 'fork_center' and {
    {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'},
    {frame = 1800, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128}
} or input_mode == 'collision_probe_center' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'button1_probe' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 960, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 1250, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 220},
    {frame = 1350, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 128},
    {frame = 1400, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 1460, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'}
} or input_mode == 'button2_probe' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 1250, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 220},
    {frame = 1350, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 128},
    {frame = 1550, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'press'},
    {frame = 1610, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'}
} or input_mode == 'weapon_probe' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 960, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 1100, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'press'},
    {frame = 1160, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'},
    {frame = 1250, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 220},
    {frame = 1350, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 128},
    {frame = 1400, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 1460, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 1550, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'press'},
    {frame = 1610, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'}
} or input_mode == 'late_drive' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'}
} or {}

local function apply_event(event)
    local port = assert(machine.ioport.ports[event.port], 'unknown input port: ' .. event.port)
    local field = assert(port.fields[event.field], 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.action == 'press' then field:set_value(1)
    elseif event.action == 'release' then field:set_value(0)
    else field:set_value(event.value) end
end

local function install_tap()
    local device = assert(machine.devices[device_tag], 'unknown device: ' .. device_tag)
    local space = assert(device.spaces[space_name], 'unknown space: ' .. space_name)
    local pc = device.state['CURPC'] or device.state['PC']
    tap = space:install_write_tap(start_address, tap_end_address, 'stunrun_ram_write_trace',
        function(offset, data, mask)
            if frame < start_frame or frame > end_frame or offset < start_address or
                offset > end_address or #events >= max_events or
                (pc_filter ~= nil and pc.value ~= pc_filter) then return end
            events[#events + 1] = {frame = frame, pc = pc.value, address = offset,
                                   data = data, mask = mask,
                                   a0 = capture_registers and device.state['A0'].value or nil,
                                   a1 = capture_registers and device.state['A1'].value or nil,
                                   a5 = capture_registers and device.state['A5'].value or nil,
                                   a10 = capture_registers and device.state['A10'].value or nil,
                                   a11 = capture_registers and device.state['A11'].value or nil}
        end)
    if read_start_address ~= nil and read_end_address ~= nil then
        read_tap = space:install_read_tap(read_start_address, read_tap_end_address,
            'stunrun_ram_read_trace_same_run', function(offset, data, mask)
                if frame < start_frame or frame > end_frame or
                    offset < read_start_address or offset > read_end_address or
                    #read_events >= read_max_events or
                    (read_pc_filter ~= nil and pc.value ~= read_pc_filter) then
                    return
                end
                read_events[#read_events + 1] = {
                    frame = frame, pc = pc.value, address = offset,
                    data = data, mask = mask,
                    a0 = capture_registers and device.state['A0'].value or nil,
                    a1 = capture_registers and device.state['A1'].value or nil,
                    a5 = capture_registers and device.state['A5'].value or nil,
                    a10 = capture_registers and device.state['A10'].value or nil,
                    a11 = capture_registers and device.state['A11'].value or nil}
            end)
    end
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame == tap_frame then install_tap() end
    while next_event <= #input_events and input_events[next_event].frame == frame do
        apply_event(input_events[next_event])
        next_event = next_event + 1
    end
    if delay_frame ~= nil and frame == delay_frame and delay_seconds > 0 then
        -- This deliberately pauses wall-clock execution without changing the
        -- emulated frame counter. It is used to test frame-vs-wall-clock
        -- lifecycle hypotheses; the duration is wrapper-bounded.
        os.execute('sleep ' .. tostring(delay_seconds))
    end
    if frame >= limit then
        for _, event in ipairs(events) do
            if capture_registers then
                print(string.format('M1_RAM_WRITE frame=%d pc=%08X addr=%08X data=%08X mask=%08X a0=%08X a1=%08X a5=%08X a10=%08X a11=%08X',
                    event.frame, event.pc, event.address, event.data, event.mask,
                    event.a0, event.a1, event.a5, event.a10, event.a11))
            else
                print(string.format('M1_RAM_WRITE frame=%d pc=%08X addr=%08X data=%08X mask=%08X',
                    event.frame, event.pc, event.address, event.data, event.mask))
            end
        end
        print(string.format('M1_RAM_WRITE_DONE frames=%d events=%d truncated=%s output=%s',
            frame, #events, tostring(#events >= max_events), output))
        if read_start_address ~= nil and read_end_address ~= nil then
            for _, event in ipairs(read_events) do
                if capture_registers then
                    print(string.format('M1_RAM_READ frame=%d pc=%08X addr=%08X data=%08X mask=%08X a0=%08X a1=%08X a5=%08X a10=%08X a11=%08X',
                        event.frame, event.pc, event.address, event.data,
                        event.mask, event.a0, event.a1, event.a5, event.a10,
                        event.a11))
                else
                    print(string.format('M1_RAM_READ frame=%d pc=%08X addr=%08X data=%08X mask=%08X',
                        event.frame, event.pc, event.address, event.data,
                        event.mask))
                end
            end
            print(string.format('M1_RAM_READ_DONE frames=%d events=%d truncated=%s output=%s',
                frame, #read_events, tostring(#read_events >= read_max_events),
                output))
        end
        machine:exit()
    end
end)
