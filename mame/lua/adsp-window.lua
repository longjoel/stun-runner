-- Bounded 68010 watchpoint experiment for the MAME-confirmed ADSP windows.

local debugger = manager.machine.debugger
local output = assert(os.getenv('STUNRUN_ADSP_WINDOW_OUT'), 'STUNRUN_ADSP_WINDOW_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_ADSP_WINDOW_FRAMES') or '600')
local base = assert(os.getenv('STUNRUN_ADSP_WINDOW_BASE'), 'STUNRUN_ADSP_WINDOW_BASE is required')
local length = assert(os.getenv('STUNRUN_ADSP_WINDOW_LENGTH'), 'STUNRUN_ADSP_WINDOW_LENGTH is required')
local label = assert(os.getenv('STUNRUN_ADSP_WINDOW_LABEL'), 'STUNRUN_ADSP_WINDOW_LABEL is required')
local input_mode = os.getenv('STUNRUN_ADSP_WINDOW_INPUT') or 'none'
local access = os.getenv('STUNRUN_ADSP_WINDOW_ACCESS') or 'write'
local frame = 0
local events = input_mode == 'coin_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or {}
local next_event = 1

local function apply_event(event)
    local port = manager.machine.ioport.ports[event.port]
    assert(port ~= nil, 'unknown input port: ' .. event.port)
    local field = port.fields[event.field]
    assert(field ~= nil, 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.action == 'press' then field:set_value(1) else field:clear_value() end
    print('M1_ADSP_INPUT frame=' .. frame .. ' port=' .. event.port .. ' field=' .. event.field .. ' action=' .. event.action)
end

-- The action prints the machine debugger's write address/data and resumes.
-- The explicit CPU tag avoids dependence on the debugger's selected device.
local watch_command
if access == 'read' then
    watch_command = 'wpset ' .. base .. ':mainpcb:maincpu,' .. length .. ',r,1,{ printf "M1_ADSP_READ kind=' .. label .. ' pc=%08X addr=%08X\\n",pc,wpaddr ; g }'
else
    watch_command = 'wpset ' .. base .. ':mainpcb:maincpu,' .. length .. ',w,1,{ printf "M1_ADSP_WRITE kind=' .. label .. ' pc=%08X addr=%08X data=%08X\\n",pc,wpaddr,wpdata ; g }'
end
debugger:command(watch_command)
debugger:command('wplist')
local consolelog = debugger.consolelog
for index = math.max(1, #consolelog - 3), #consolelog do
    print('M1_DEBUGGER ' .. tostring(consolelog[index]))
end

emu.register_frame_done(function()
    frame = frame + 1
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if frame >= limit then
        print('M1_ADSP_WINDOW_DONE frames=' .. frame .. ' output=' .. output)
        manager.machine:exit()
    end
end)
