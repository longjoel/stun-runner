-- Capture direct reads from the input-adjacent 68010 RAM state at landmarks.

local machine = manager.machine
local cpu = assert(machine.devices[':mainpcb:maincpu'])
local space = cpu.spaces['program']
local output = assert(os.getenv('STUNRUN_MAIN_STATE_OUT'), 'STUNRUN_MAIN_STATE_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_MAIN_STATE_FRAMES') or '1800')
local target_text = os.getenv('STUNRUN_MAIN_STATE_TARGETS') or '600,750,900,1200,1800'
local input_mode = os.getenv('STUNRUN_MAIN_STATE_INPUT') or 'late'
local frame = 0
local targets = {}
for value in string.gmatch(target_text, '[^,]+') do targets[tonumber(value)] = true end

local function events_for(mode)
    if mode == 'none' then
        return {}
    end
    if mode == 'late_drive' then
        return {
            {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
            {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
            {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
            {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
            {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
            {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
            {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
            {frame = 1500, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'}
        }
    end
    return {
        {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
        {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
        {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
        {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
    }
end

local events = events_for(input_mode)
local next_event = 1

local function apply_event(event)
    local port = assert(machine.ioport.ports[event.port], 'unknown input port: ' .. event.port)
    local field = assert(port.fields[event.field], 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.action == 'press' then field:set_value(1)
    elseif event.action == 'release' then field:set_value(0)
    else field:set_value(event.value) end
end

local function read_bytes(base, count)
    local values = {}
    for offset = 0, count - 1 do values[#values + 1] = space:read_u8(base + offset) end
    return values
end

local function snapshot()
    local values = {
        frame = frame,
        time_seconds = machine.time:as_double(),
        pc = cpu.state['PC'].value,
        record = read_bytes(0xff9000, 4),
        record_aux = read_bytes(0xff9004, 4),
        queue_counter = space:read_u8(0xffdb4a),
        update_flag = space:read_u16(0xffdaee)
    }
    print(string.format('M1_MAIN_STATE frame=%d pc=%08X r=%02X%02X%02X%02X aux=%02X%02X%02X%02X counter=%02X flag=%04X',
        values.frame, values.pc, values.record[1], values.record[2], values.record[3], values.record[4],
        values.record_aux[1], values.record_aux[2], values.record_aux[3], values.record_aux[4],
        values.queue_counter, values.update_flag))
    return values
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
        print('M1_MAIN_STATE_DONE frames=' .. frame .. ' output=' .. output)
        machine:exit()
    end
end)
