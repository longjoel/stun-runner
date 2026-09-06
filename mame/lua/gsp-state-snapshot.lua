-- Capture bounded, low-rate GSP state samples at selected frame boundaries.

local machine = manager.machine
local gsp = assert(machine.devices[':mainpcb:gsp'])
local space = gsp.spaces['program']
local output = assert(os.getenv('STUNRUN_GSP_STATE_OUT'), 'STUNRUN_GSP_STATE_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_GSP_STATE_FRAMES') or '1800')
local target_text = os.getenv('STUNRUN_GSP_STATE_TARGETS') or '600,1800'
local input_mode = os.getenv('STUNRUN_GSP_STATE_INPUT') or 'none'
local frame = 0
local targets = {}
for value in string.gmatch(target_text, '[^,]+') do targets[tonumber(value)] = true end

local sw_off_prefix = {
    {port = ':mainpcb:SW1', field = 'SW1:1', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:2', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:3', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:4', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:5', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:6', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:7', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:8', action = 'set', value = 1}
}
local events = input_mode == 'sw_off_prestart' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or {}
local next_event = 1

local function apply_event(event)
    local port = assert(machine.ioport.ports[event.port], 'unknown input port: ' .. event.port)
    local field = assert(port.fields[event.field], 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.action == 'press' then
        field:set_value(1)
    elseif event.action == 'release' then
        field:set_value(0)
    else
        field:set_value(event.value)
    end
end

if input_mode == 'sw_off_prestart' then
    emu.register_prestart(function()
        for _, event in ipairs(sw_off_prefix) do apply_event(event) end
    end)
end

local function sample_range(base, span, stride)
    local count = 0
    local sum32 = 0
    local first = {}
    for offset = 0, span - 2, stride do
        local value = space:read_u16(base + offset)
        count = count + 1
        sum32 = (sum32 + value) % 4294967296
        if #first < 16 then first[#first + 1] = value end
    end
    return {base = base, span = span, stride = stride, samples = count, sum32 = sum32, first = first}
end

local function snapshot()
    local state = {
        frame = frame,
        time_seconds = machine.time:as_double(),
        pc = gsp.state['PC'].value,
        st = gsp.state['ST'].value,
        control_lo = sample_range(0xf4000000, 0x100, 2),
        control_hi = sample_range(0xf4800000, 0x100, 2),
        vram_low = sample_range(0x02000000, 0x80000, 0x400),
        vram_high = sample_range(0xff800000, 0x800000, 0x4000)
    }
    print(string.format(
        'M1_GSP_STATE frame=%d pc=%08X st=%08X lo_sum=%u hi_sum=%u low_sum=%u high_sum=%u',
        state.frame, state.pc, state.st, state.control_lo.sum32, state.control_hi.sum32,
        state.vram_low.sum32, state.vram_high.sum32))
    return state
end

local snapshots = {}
emu.register_frame_done(function()
    frame = frame + 1
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if targets[frame] then snapshots[#snapshots + 1] = snapshot() end
    if frame >= limit then
        print('M1_GSP_STATE_DONE frames=' .. frame .. ' output=' .. output)
        machine:exit()
    end
end)
