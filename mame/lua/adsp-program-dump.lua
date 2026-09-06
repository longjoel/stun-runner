-- Dump the active ADSP program RAM at one bounded frame landmark.

local machine = manager.machine
local adsp = assert(machine.devices[':mainpcb:adsp'])
local space = adsp.spaces['program']
local limit = tonumber(os.getenv('STUNRUN_ADSP_DUMP_FRAMES') or '136')
local target = tonumber(os.getenv('STUNRUN_ADSP_DUMP_TARGET') or '136')
local frame = 0

local function dump()
    for address = 0, 0x1fff do
        print(string.format('M3_ADSP_WORD address=%04X value=%08X', address,
            space:read_u32(address) & 0xffffffff))
    end
    print(string.format('M3_ADSP_DUMP frame=%d pc=%04X', frame, adsp.state['PC'].value))
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame == target then dump() end
    if frame >= limit then
        print('M3_ADSP_DUMP_DONE frames=' .. frame)
        machine:exit()
    end
end)
