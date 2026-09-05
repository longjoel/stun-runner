-- Bounded 68010 execution probe for static landmark candidates.

local debugger = manager.machine.debugger
local output = assert(os.getenv('STUNRUN_LANDMARK_OUT'), 'STUNRUN_LANDMARK_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_LANDMARK_FRAMES') or '600')
local frame = 0
local input_mode = os.getenv('STUNRUN_LANDMARK_INPUT') or 'none'
local events = input_mode == 'coin_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', press = true},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', press = true},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start'}
} or {}
local next_event = 1

for pc in string.gmatch(assert(os.getenv('STUNRUN_LANDMARK_PCS')), '[^,]+') do
    debugger:command('bpset ' .. pc .. ':mainpcb:maincpu,1,{ printf "M1_LANDMARK_HIT pc=%08X\\n",pc ; g }')
end

local function apply_event(event)
    local field = assert(manager.machine.ioport.ports[event.port]).fields[event.field]
    assert(field ~= nil, 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.press then field:set_value(1) else field:clear_value() end
    print('M1_LANDMARK_INPUT frame=' .. frame .. ' field=' .. event.field)
end

emu.register_frame_done(function()
    frame = frame + 1
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if frame >= limit then
        print('M1_LANDMARK_DONE frames=' .. frame .. ' output=' .. output)
        manager.machine:exit()
    end
end)
