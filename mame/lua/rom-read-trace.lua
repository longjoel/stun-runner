-- Bounded 68010 program-ROM read trace, filtered by current PC.

local output = assert(os.getenv('STUNRUN_ROM_TRACE_OUT'), 'STUNRUN_ROM_TRACE_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_ROM_TRACE_FRAMES') or '1800')
local start_frame = tonumber(os.getenv('STUNRUN_ROM_TRACE_START') or '600')
local end_frame = tonumber(os.getenv('STUNRUN_ROM_TRACE_END') or tostring(limit))
local rom_end = tonumber(os.getenv('STUNRUN_ROM_TRACE_ROM_END') or '1048575')
local max_events = tonumber(os.getenv('STUNRUN_ROM_TRACE_MAX_EVENTS') or '200000')
local range_text = assert(os.getenv('STUNRUN_ROM_TRACE_PC_RANGES'), 'PC ranges are required')
local ranges = {}
for item in string.gmatch(range_text, '[^,]+') do
    local first, last = string.match(item, '^(%d+)%-(%d+)$')
    assert(first and last, 'invalid PC range: ' .. item)
    ranges[#ranges + 1] = {first = tonumber(first), last = tonumber(last)}
end
local address_ranges = {}
local address_range_text = os.getenv('STUNRUN_ROM_TRACE_ADDRESS_RANGES') or ''
for item in string.gmatch(address_range_text, '[^,]+') do
    local first, last = string.match(item, '^(%d+)%-(%d+)$')
    assert(first and last, 'invalid address range: ' .. item)
    address_ranges[#address_ranges + 1] = {first = tonumber(first), last = tonumber(last)}
end
local frame = 0
local events = {}
local input_mode = os.getenv('STUNRUN_ROM_TRACE_INPUT') or 'none'
local input_events = input_mode == 'late_drive' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', value = 1},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', value = 0},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', value = 0},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 1},
    {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', value = 128},
    {frame = 1500, port = ':mainpcb:a80000', field = 'P1 Button 1', value = 0}
} or {}
local next_event = 1
local cpu = manager.machine.devices[':mainpcb:maincpu']
local pc = cpu.state['CURPC'] or cpu.state['PC']
local events_tap

local function in_pc_range(value)
    for _, range in ipairs(ranges) do
        if value >= range.first and value <= range.last then return true end
    end
    return false
end

local function in_address_range(value)
    if #address_ranges == 0 then return true end
    for _, range in ipairs(address_ranges) do
        if value >= range.first and value <= range.last then return true end
    end
    return false
end

local function apply_event(event)
    local port = assert(manager.machine.ioport.ports[event.port])
    local field = assert(port.fields[event.field])
    field:set_value(event.value)
end

local function install_tap()
    events_tap = cpu.spaces['program']:install_read_tap(0, rom_end, 'stunrun_rom_read_trace',
        function(offset, data, mask)
            local current_pc = pc.value
            if frame >= start_frame and frame <= end_frame and offset <= rom_end and
                in_address_range(offset) and
                #events < max_events and in_pc_range(current_pc) then
                events[#events + 1] = {frame = frame, pc = current_pc, address = offset,
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
            print(string.format('M1_ROM_READ frame=%d pc=%08X addr=%08X data=%08X mask=%08X',
                event.frame, event.pc, event.address, event.data, event.mask))
        end
        print(string.format('M1_ROM_READ_DONE frames=%d events=%d truncated=%s output=%s',
            frame, #events, tostring(#events >= max_events), output))
        manager.machine:exit()
    end
end)
