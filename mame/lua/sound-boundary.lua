-- Bounded JSA transport watchpoints for the M1 sound-boundary experiment.

local debugger = manager.machine.debugger
local limit = tonumber(os.getenv('STUNRUN_SOUND_BOUNDARY_FRAMES') or '600')
local input_mode = os.getenv('STUNRUN_SOUND_BOUNDARY_INPUT') or 'none'
local frame = 0

local events = input_mode == 'coin_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', value = 1},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', value = 0},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', value = 1},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', value = 0}
} or {}
local next_event = 1

local taps = {}

local function cpu_pc(tag)
    local state = manager.machine.devices[tag].state
    return state['CURPC'].value
end

local function read_tap(address, tag, label)
    local cpu = manager.machine.devices[tag]
    taps[#taps + 1] = cpu.spaces['program']:install_read_tap(
        address, address + 1, 'stunrun_' .. label,
        function(offset, data, mask)
            print(string.format('M1_SOUND_READ label=%s frame=%d pc=%08X addr=%08X data=%08X mask=%08X',
                label, frame, cpu_pc(tag), offset, data, mask))
        end)
end

local function write_tap(address, tag, label)
    local cpu = manager.machine.devices[tag]
    taps[#taps + 1] = cpu.spaces['program']:install_write_tap(
        address, address + 1, 'stunrun_' .. label,
        function(offset, data, mask)
            print(string.format('M1_SOUND_WRITE label=%s frame=%d pc=%08X addr=%08X data=%08X mask=%08X',
                label, frame, cpu_pc(tag), offset, data, mask))
        end)
end

local function install_main_taps()
    -- The main window is dynamically installed by init_multisync(0).  Install
    -- these after machine start so the taps attach to the final handlers.
    read_tap(0x600000, ':mainpcb:maincpu', 'main_response')
    write_tap(0x600000, ':mainpcb:maincpu', 'main_command')
end

-- JSA-II's command and response registers are in the 6502 address space.
read_tap(0x2802, ':mainpcb:jsa:cpu', 'sound_command')
write_tap(0x2a02, ':mainpcb:jsa:cpu', 'sound_response')

local function apply_event(event)
    local port = assert(manager.machine.ioport.ports[event.port])
    local field = assert(port.fields[event.field])
    field:set_value(event.value)
    print('M1_SOUND_INPUT frame=' .. frame .. ' field=' .. event.field .. ' value=' .. event.value)
end

emu.register_frame_done(function()
    if frame == 10 then
        install_main_taps()
    end
    frame = frame + 1
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if frame >= limit then
        print('M1_SOUND_BOUNDARY_DONE frames=' .. frame)
        manager.machine:exit()
    end
end)
