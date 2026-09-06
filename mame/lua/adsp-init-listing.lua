-- Emit an ADSP listing after the original title-path upload completes.

local machine = manager.machine
local limit = tonumber(os.getenv('STUNRUN_ADSP_LISTING_FRAME') or '412')
local output = assert(os.getenv('STUNRUN_ADSP_LISTING_OUT'), 'STUNRUN_ADSP_LISTING_OUT is required')
local frame = 0

emu.register_frame_done(function()
    frame = frame + 1
    if frame == limit then
        machine.debugger:command('dasm ' .. output .. ',0,0x1000,0,:mainpcb:adsp')
        print('M3_ADSP_LISTING frame=' .. frame .. ' output=' .. output)
        machine:exit()
    end
end)
