-- Start one bounded, no-loop debugger trace and stop after a frame budget.
-- Input events are optional and are applied through the same replay contract.

local debugger = manager.machine.debugger
local output = assert(os.getenv('STUNRUN_TRACE_OUT'), 'STUNRUN_TRACE_OUT is required')
local cpu = assert(os.getenv('STUNRUN_TRACE_CPU'), 'STUNRUN_TRACE_CPU is required')
local limit = tonumber(os.getenv('STUNRUN_TRACE_FRAMES') or '60')
local input_mode = os.getenv('STUNRUN_TRACE_INPUT') or 'none'
local frame = 0
local events = input_mode == 'coin_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', press = true},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start'}
} or {}
local next_event = 1

local function apply_event(event)
    local field = assert(manager.machine.ioport.ports[event.port]).fields[event.field]
    assert(field ~= nil, 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.press then field:set_value(1) else field:set_value(0) end
    print('MAME_TRACE_INPUT frame=' .. frame .. ' field=' .. event.field)
end

debugger:command('trace ' .. output .. ',' .. cpu .. ',noloop')

emu.register_frame_done(function()
    frame = frame + 1
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
