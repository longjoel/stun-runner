-- Capture bounded snapshots of the MAME ADSP program RAM.

local machine = manager.machine
local adsp = machine.devices[':mainpcb:adsp']
local space = adsp.spaces['program']
local limit = tonumber(os.getenv('STUNRUN_ADSP_SNAPSHOT_FRAMES') or '600')
local frame = 0
local targets = {}
for value in string.gmatch(os.getenv('STUNRUN_ADSP_SNAPSHOT_TARGETS') or '1,10,122,136,600', '[^,]+') do
    targets[tonumber(value)] = true
end

local function word(value)
    return string.format('%08X', value & 0xffffffff)
end

local function snapshot()
    local nonzero = 0
    local first = {}
    for address = 0, 0x1fff do
        local value = space:read_u32(address)
        if value ~= 0 then nonzero = nonzero + 1 end
        if address < 16 then first[#first + 1] = word(value) end
    end
    print(string.format(
        'M1_ADSP_PROGRAM_SNAPSHOT frame=%d pc=%08X mstat=%08X nonzero=%d first=%s',
        frame, adsp.state['PC'].value, adsp.state['MSTAT'].value,
        nonzero, table.concat(first, ',')))
end

emu.register_frame_done(function()
    frame = frame + 1
    if targets[frame] then snapshot() end
    if frame >= limit then
        print('M1_ADSP_PROGRAM_SNAPSHOT_DONE frames=' .. frame)
        machine:exit()
    end
end)
