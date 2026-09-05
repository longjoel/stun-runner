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
    for key, _ in pairs(value) do keys[#keys + 1] = key end
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

local function capture()
    local devices = machine.devices
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
        }
    }
    print('MAME_CHECKPOINT_JSON ' .. json(checkpoint))
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame >= checkpoint_frame and not captured then
        captured = true
        capture()
    end
end)
