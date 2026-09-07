-- Start one bounded, no-loop debugger trace and stop after a frame budget.
-- Input events are optional and are applied through the same replay contract.

local debugger = manager.machine.debugger
local output = assert(os.getenv('STUNRUN_TRACE_OUT'), 'STUNRUN_TRACE_OUT is required')
local cpu = assert(os.getenv('STUNRUN_TRACE_CPU'), 'STUNRUN_TRACE_CPU is required')
local limit = tonumber(os.getenv('STUNRUN_TRACE_FRAMES') or '60')
local input_mode = os.getenv('STUNRUN_TRACE_INPUT') or 'none'
local trace_start_frame = tonumber(os.getenv('STUNRUN_TRACE_START_FRAME') or '1')
local playback = os.getenv('STUNRUN_TRACE_PLAYBACK')
local frame = 0
local sw_off_prefix = {
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:1', action = 'press'},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:2', action = 'press'},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:3', action = 'press'},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:4', action = 'press'},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:5', action = 'press'},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:6', action = 'press'},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:7', action = 'press'},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:8', action = 'press'}
}
local drive_prefix = {
    {port = ':mainpcb:SW1', field = 'SW1:1', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:2', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:3', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:4', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:5', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:6', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:7', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:8', action = 'set', value = 1}
}
local late_prefix = {
    {port = ':mainpcb:SW1', field = 'SW1:1', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:2', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:3', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:4', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:5', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:6', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:7', action = 'set', value = 1},
    {port = ':mainpcb:SW1', field = 'SW1:8', action = 'set', value = 1}
}
print('MAME_TRACE_CONFIG start=' .. trace_start_frame .. ' cpu=' .. cpu .. ' output=' .. output)
local events = (playback ~= nil and {}) or (input_mode == 'late_drive' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', press = true},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', press = true},
    {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
    {frame = 1500, port = ':mainpcb:a80000', field = 'P1 Button 1'}
} or input_mode == 'late' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', press = true},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start'}
} or input_mode == 'drive' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'set', value = 0},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'set', value = 1},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'set', value = 0},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'set', value = 1},
    {frame = 600, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'set', value = 0},
    {frame = 600, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 1200, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'set', value = 1},
    {frame = 1200, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128}
} or input_mode == 'coin_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', press = true},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start'}
} or input_mode == 'coin2_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 2', press = true},
    {frame = 150, port = ':mainpcb:IN0', field = 'Coin 2'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 330, port = ':mainpcb:a80000', field = '1 Player Start'}
} or input_mode == 'coin2_early' and {
    {frame = 1, port = ':mainpcb:IN0', field = 'Coin 2', press = true},
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 2'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 330, port = ':mainpcb:a80000', field = '1 Player Start'}
} or input_mode == 'service' and {
    {frame = 1, port = ':mainpcb:IN0', field = 'Service Mode', press = true},
    {frame = 120, port = ':mainpcb:IN0', field = 'Service Mode'}
} or input_mode == 'sw_off' and {
    sw_off_prefix[1], sw_off_prefix[2], sw_off_prefix[3], sw_off_prefix[4],
    sw_off_prefix[5], sw_off_prefix[6], sw_off_prefix[7], sw_off_prefix[8],
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', press = true},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start'}
} or input_mode == 'sw_off_none' and sw_off_prefix or {})
if input_mode == 'fork_hold_left' then
    events = {
        {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
        {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1'},
        {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2'}
    }
elseif input_mode == 'fork_center' then
    events = {
        {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
        {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1'},
        {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 2'},
        {frame = 1800, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128}
    }
end
local next_event = 1

local function apply_event(event)
    local field = assert(manager.machine.ioport.ports[event.port]).fields[event.field]
    assert(field ~= nil, 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.press then field:set_value(1) elseif event.action == 'set' then field:set_value(event.value) else field:set_value(0) end
    print('MAME_TRACE_INPUT frame=' .. frame .. ' field=' .. event.field)
end

if input_mode == 'drive' or input_mode == 'late' or input_mode == 'late_drive' then
    emu.register_prestart(function()
        local prefix = input_mode == 'drive' and drive_prefix or late_prefix
        for _, event in ipairs(prefix) do apply_event(event) end
    end)
end

if trace_start_frame <= 1 then
    debugger:command('trace ' .. output .. ',' .. cpu .. ',noloop')
else
    debugger:command('trace off,' .. cpu)
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame == trace_start_frame and trace_start_frame > 1 then
        debugger:command('trace ' .. output .. ',' .. cpu .. ',noloop')
    end
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if frame >= limit then
        debugger:command('traceflush')
        debugger:command('trace off,' .. cpu)
        print('MAME_TRACE_DONE cpu=' .. cpu .. ' frames=' .. frame .. ' output=' .. output)
        manager.machine:exit()
    end
end)
