-- Capture bounded, low-rate GSP state samples at selected frame boundaries.

local machine = manager.machine
local gsp = assert(machine.devices[':mainpcb:gsp'])
local space = gsp.spaces['program']
local output = assert(os.getenv('STUNRUN_GSP_STATE_OUT'), 'STUNRUN_GSP_STATE_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_GSP_STATE_FRAMES') or '1800')
local target_text = os.getenv('STUNRUN_GSP_STATE_TARGETS') or '600,1800'
local input_mode = os.getenv('STUNRUN_GSP_STATE_INPUT') or 'none'
local detail = os.getenv('STUNRUN_GSP_STATE_DETAIL') == '1'
local screen_output = os.getenv('STUNRUN_GSP_STATE_SCREEN') or ''
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
local events = input_mode == 'late_drive' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
    {frame = 1500, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'}
} or input_mode == 'late' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'drive' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'set', value = 0},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'set', value = 1},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'set', value = 0},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'set', value = 1},
    {frame = 600, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'set', value = 0},
    {frame = 600, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 1200, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'set', value = 1},
    {frame = 1200, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128}
} or input_mode == 'sw_off_prestart' and {
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

if input_mode == 'sw_off_prestart' or input_mode == 'drive' or input_mode == 'late' or input_mode == 'late_drive' then
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

local function nonzero_words(base, count)
    local words = {}
    for index = 0, count - 1 do
        local value = space:read_u16(base + index * 2)
        if value ~= 0 then
            words[#words + 1] = string.format('%02X=%04X', index, value)
        end
    end
    return table.concat(words, ',')
end

local function snapshot()
    local function ioreg(index)
        return space:read_u16(0xc0000000 + index * 16)
    end
    local state = {
        frame = frame,
        time_seconds = machine.time:as_double(),
        pc = gsp.state['PC'].value,
        st = gsp.state['ST'].value,
        display = {
            dpyadr = ioreg(30),
            dpytap = ioreg(27),
            dpystart = ioreg(9),
            heblnk = ioreg(1),
            hsblnk = ioreg(2),
            dpyctl = ioreg(8),
        },
        control_lo = sample_range(0xf4000000, 0x100, 2),
        control_hi = sample_range(0xf4800000, 0x100, 2),
        vram_low = sample_range(0x02000000, 0x80000, 0x400),
        vram_high = sample_range(0xff800000, 0x800000, 0x4000)
    }
    print(string.format(
        'M1_GSP_STATE frame=%d pc=%08X st=%08X lo_sum=%u hi_sum=%u low_sum=%u high_sum=%u',
        state.frame, state.pc, state.st, state.control_lo.sum32, state.control_hi.sum32,
        state.vram_low.sum32, state.vram_high.sum32))
    print(string.format(
        'M1_GSP_DISPLAY frame=%d dpyadr=%s dpytap=%s dpystart=%s heblnk=%s hsblnk=%s dpyctl=%s',
        frame, tostring(state.display.dpyadr), tostring(state.display.dpytap),
        tostring(state.display.dpystart), tostring(state.display.heblnk),
        tostring(state.display.hsblnk), tostring(state.display.dpyctl)))
    if screen_output ~= '' then
        local screen = assert(machine.screens[':mainpcb:screen'], 'screen not found')
        assert(screen:snapshot(screen_output) == nil, 'screen snapshot failed')
        print('M1_GSP_SCREEN frame=' .. frame .. ' path=' .. screen_output)
    end
    if detail then
        print(string.format('M1_GSP_CONTROL frame=%d lo=%s hi=%s', frame,
            nonzero_words(0xf4000000, 128), nonzero_words(0xf4800000, 128)))
    end
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
