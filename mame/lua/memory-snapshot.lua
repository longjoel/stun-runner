-- Capture a bounded raw memory-space snapshot at configured frame targets.

local function quote(value)
    value = tostring(value):gsub('\\', '\\\\'):gsub('"', '\\"')
    return '"' .. value .. '"'
end

local function json(value)
    if value == nil then return 'null' end
    if type(value) == 'string' then return quote(value) end
    if type(value) == 'number' or type(value) == 'boolean' then return tostring(value) end
    local parts = {}
    local keys = {}
    local is_array = true
    local max_index = 0
    for key, _ in pairs(value) do keys[#keys + 1] = key end
    for _, key in ipairs(keys) do
        if type(key) ~= 'number' or key < 1 or key % 1 ~= 0 then
            is_array = false
            break
        end
        if key > max_index then max_index = key end
    end
    if is_array then
        for index = 1, max_index do
            if value[index] == nil then is_array = false break end
        end
    end
    if is_array then
        for index = 1, max_index do parts[#parts + 1] = json(value[index]) end
        return '[' .. table.concat(parts, ',') .. ']'
    end
    table.sort(keys)
    for _, key in ipairs(keys) do
        parts[#parts + 1] = quote(key) .. ':' .. json(value[key])
    end
    return '{' .. table.concat(parts, ',') .. '}'
end

local machine = manager.machine
local device_tag = assert(os.getenv('STUNRUN_MEMORY_DEVICE'), 'STUNRUN_MEMORY_DEVICE is required')
local space_name = os.getenv('STUNRUN_MEMORY_SPACE') or 'program'
local base = tonumber(os.getenv('STUNRUN_MEMORY_BASE') or '0')
local count = tonumber(os.getenv('STUNRUN_MEMORY_COUNT') or '1')
local width = tonumber(os.getenv('STUNRUN_MEMORY_WIDTH') or '8')
local limit = tonumber(os.getenv('STUNRUN_MEMORY_FRAMES') or '600')
local target_text = os.getenv('STUNRUN_MEMORY_TARGETS') or tostring(limit)
local output = assert(os.getenv('STUNRUN_MEMORY_OUT'), 'STUNRUN_MEMORY_OUT is required')
local input_mode = os.getenv('STUNRUN_MEMORY_INPUT') or 'none'
local save_state_path = os.getenv('STUNRUN_MEMORY_SAVE_STATE') or ''
local load_state_path = os.getenv('STUNRUN_MEMORY_LOAD_STATE') or ''
local frame = 0
local targets = {}
local state_saved = false
local load_requested = load_state_path == ''

assert(width == 8 or width == 16 or width == 32, 'width must be 8, 16, or 32')
assert(count > 0 and count % 1 == 0, 'count must be a positive integer')
assert(base >= 0 and base % 1 == 0, 'base must be a nonnegative integer')
for value in string.gmatch(target_text, '[^,]+') do targets[tonumber(value)] = true end

local device = assert(machine.devices[device_tag], 'unknown device: ' .. device_tag)
local space = assert(device.spaces[space_name], 'unknown space: ' .. space_name)
local reader = width == 8 and space.read_u8 or width == 16 and space.read_u16 or space.read_u32

local events = input_mode == 'fork_lateral_sweep' and {
    {frame = 2, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 2, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 600, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255},
    {frame = 1200, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 1800, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255},
    {frame = 2400, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128}
} or input_mode == 'damage_probe_sweep' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255},
    {frame = 2100, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 2700, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255},
    {frame = 3300, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
    {frame = 3900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 4500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255}
} or input_mode == 'collision_probe_hold_right' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 255}
} or input_mode == 'collision_probe_hold_left' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0}
} or input_mode == 'collision_probe_center' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'collision_probe_left' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 0},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
    {frame = 1500, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'}
} or input_mode == 'weapon_probe' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 960, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 1100, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'press'},
    {frame = 1160, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'},
    {frame = 1250, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 220},
    {frame = 1350, port = ':mainpcb:8BADC.2', field = 'AD Stick Y', action = 'set', value = 128},
    {frame = 1400, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 1460, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'},
    {frame = 1550, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'press'},
    {frame = 1610, port = ':mainpcb:a80000', field = 'P1 Button 2', action = 'release'}
} or input_mode == 'late_drive' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'}
} or input_mode == 'late' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or {}
local next_event = 1

local function apply_event(event)
    local port = assert(machine.ioport.ports[event.port], 'unknown input port: ' .. event.port)
    local field = assert(port.fields[event.field], 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.action == 'press' then field:set_value(1)
    elseif event.action == 'release' then field:set_value(0)
    else field:set_value(event.value) end
end

local function capture()
    local values = {}
    local nonzero = 0
    for offset = 0, count - 1 do
        local value = reader(space, base + offset)
        values[#values + 1] = value
        if value ~= 0 then nonzero = nonzero + 1 end
    end
    local snapshot = {
        schema = 'stunrun-memory-snapshot/v1',
        system = machine.system.name,
        mame = emu.app_build(),
        frame = frame,
        time_seconds = machine.time:as_double(),
        device = device_tag,
        space = space_name,
        base = base,
        count = count,
        width = width,
        input = input_mode,
        nonzero = nonzero,
        values = values
    }
    local path = output .. '/snapshot-' .. tostring(frame) .. '.json'
    local handle = assert(io.open(path, 'w'))
    handle:write(json(snapshot) .. '\n')
    handle:close()
    print(string.format('MAME_MEMORY_SNAPSHOT frame=%d path=%s nonzero=%d', frame, path, nonzero))
end

emu.register_frame_done(function()
    frame = frame + 1
    if not load_requested then
        machine:load(load_state_path)
        load_requested = true
        return
    end
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if targets[frame] then capture() end
    if frame >= limit and save_state_path ~= '' and not state_saved then
        machine:save(save_state_path)
        state_saved = true
        print('MAME_MEMORY_STATE_SAVE path=' .. save_state_path .. ' frame=' .. frame)
    end
    if frame >= limit + (save_state_path ~= '' and 2 or 0) then
        print('MAME_MEMORY_SNAPSHOT_DONE frames=' .. frame)
        machine:exit()
    end
end)
