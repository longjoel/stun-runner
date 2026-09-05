-- Start one bounded, no-loop debugger trace and stop after a frame budget.

local debugger = manager.machine.debugger
local output = assert(os.getenv('STUNRUN_TRACE_OUT'), 'STUNRUN_TRACE_OUT is required')
local cpu = assert(os.getenv('STUNRUN_TRACE_CPU'), 'STUNRUN_TRACE_CPU is required')
local limit = tonumber(os.getenv('STUNRUN_TRACE_FRAMES') or '60')
local frame = 0

debugger:command('trace ' .. output .. ',' .. cpu .. ',noloop')

emu.register_frame_done(function()
    frame = frame + 1
    if frame >= limit then
        debugger:command('traceflush')
        debugger:command('trace off,' .. cpu)
        print('MAME_TRACE_DONE cpu=' .. cpu .. ' frames=' .. frame .. ' output=' .. output)
        manager.machine:exit()
    end
end)
