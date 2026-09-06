-- Capture a small, deterministic machine checkpoint after a bounded frame count.

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

local function state_value(device, symbol)
    if device == nil or device.state == nil then return nil end
    local entry = device.state[symbol]
    return entry and entry.value or nil
end

local machine = manager.machine
local frame = 0
local captured = false
local checkpoint_frame = tonumber(os.getenv('STUNRUN_CHECKPOINT_FRAME') or '600')
local input_mode = os.getenv('STUNRUN_CHECKPOINT_INPUT') or 'none'
local sw_off_prefix = {
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:1', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:2', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:3', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:4', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:5', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:6', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:7', action = 'set', value = 1},
    {frame = 1, port = ':mainpcb:SW1', field = 'SW1:8', action = 'set', value = 1}
}
local initial_events = (input_mode == 'sw_off_prestart' or input_mode == 'drive') and sw_off_prefix or {}
local events = input_mode == 'drive' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'set', value = 0},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'set', value = 1},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'set', value = 0},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'set', value = 1},
    {frame = 600, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'set', value = 0},
    {frame = 600, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 1200, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'set', value = 1},
    {frame = 1200, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128}
} or input_mode == 'sw_off' and {
    sw_off_prefix[1], sw_off_prefix[2], sw_off_prefix[3], sw_off_prefix[4],
    sw_off_prefix[5], sw_off_prefix[6], sw_off_prefix[7], sw_off_prefix[8],
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or (input_mode == 'sw_off_prestart') and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or {}
local next_event = 1

local function apply_event(event)
    local port = assert(machine.ioport.ports[event.port], 'unknown input port: ' .. event.port)
    local field = assert(port.fields[event.field], 'unknown input field: ' .. event.port .. '/' .. event.field)
    field:set_value(event.action == 'press' and 1 or event.action == 'release' and 0 or event.value)
end

emu.register_prestart(function()
    for _, event in ipairs(initial_events) do
        apply_event(event)
    end
end)

local function capture()
    local devices = machine.devices
    local adsp_program = devices[':mainpcb:adsp'].spaces['program']
    local adsp_nonzero_words = 0
    local adsp_sum32 = 0
    local adsp_first_words = {}
    for address = 0, 0x1fff do
        local value = adsp_program:read_u32(address)
        if value ~= 0 then
            adsp_nonzero_words = adsp_nonzero_words + 1
            if #adsp_first_words < 16 then
                adsp_first_words[#adsp_first_words + 1] = value
            end
        end
        adsp_sum32 = (adsp_sum32 + value) % 4294967296
    end
    local checkpoint = {
        schema = 'stunrun-checkpoint/v1',
        system = machine.system.name,
        description = machine.system.description,
        mame = emu.app_build(),
        frame = frame,
        time_seconds = machine.time:as_double(),
        processors = {
            maincpu = {
                tag = ':mainpcb:maincpu',
                pc = state_value(devices[':mainpcb:maincpu'], 'PC'),
                sr = state_value(devices[':mainpcb:maincpu'], 'SR'),
                sp = state_value(devices[':mainpcb:maincpu'], 'SP')
            },
            gsp = {
                tag = ':mainpcb:gsp',
                pc = state_value(devices[':mainpcb:gsp'], 'PC'),
                st = state_value(devices[':mainpcb:gsp'], 'ST')
            },
            adsp = {
                tag = ':mainpcb:adsp',
                pc = state_value(devices[':mainpcb:adsp'], 'PC'),
                astat = state_value(devices[':mainpcb:adsp'], 'ASTAT')
            },
            soundcpu = {
                tag = ':mainpcb:jsa:cpu',
                pc = state_value(devices[':mainpcb:jsa:cpu'], 'PC'),
                p = state_value(devices[':mainpcb:jsa:cpu'], 'P')
            }
        },
        regions = {
            adsp_program = {
                space = 'program',
                range = '0x0000-0x1fff',
                word_width = 32,
                nonzero_words = adsp_nonzero_words,
                sum32 = adsp_sum32,
                first_nonzero_words = adsp_first_words
            }
        },
        selectors = {
            adsp_program_loaded = adsp_nonzero_words > 0
        }
    }
    print('MAME_CHECKPOINT_JSON ' .. json(checkpoint))
end

emu.register_frame_done(function()
    frame = frame + 1
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if frame >= checkpoint_frame and not captured then
        captured = true
        capture()
    end
end)
