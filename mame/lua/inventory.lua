-- Generic MAME runtime inventory.
-- Run with -autoboot_script and disable incompatible third-party plugins.

local function json_escape(value)
    value = tostring(value)
    value = value:gsub('\\', '\\\\'):gsub('"', '\\"')
    value = value:gsub('\n', '\\n'):gsub('\r', '\\r'):gsub('\t', '\\t')
    return '"' .. value .. '"'
end

local function is_array(value)
    local count = 0
    for key, _ in pairs(value) do
        if type(key) ~= 'number' then return false end
        count = count + 1
    end
    for index = 1, count do
        if value[index] == nil then return false end
    end
    return true
end

local function json_encode(value)
    local value_type = type(value)
    if value == nil then return 'null' end
    if value_type == 'string' then return json_escape(value) end
    if value_type == 'number' or value_type == 'boolean' then return tostring(value) end
    if value_type ~= 'table' then return json_escape(value) end

    local parts = {}
    if is_array(value) then
        for index = 1, #value do
            parts[#parts + 1] = json_encode(value[index])
        end
        return '[' .. table.concat(parts, ',') .. ']'
    end

    local keys = {}
    for key, _ in pairs(value) do keys[#keys + 1] = key end
    table.sort(keys)
    for _, key in ipairs(keys) do
        parts[#parts + 1] = json_escape(key) .. ':' .. json_encode(value[key])
    end
    return '{' .. table.concat(parts, ',') .. '}'
end

local function sorted_keys(value)
    local keys = {}
    for key, _ in pairs(value) do keys[#keys + 1] = key end
    table.sort(keys)
    return keys
end

local function inventory_state(device)
    local result = {}
    if device.state == nil then return result end
    for _, key in ipairs(sorted_keys(device.state)) do
        local entry = device.state[key]
        if entry.visible then
            result[#result + 1] = {
                symbol = entry.symbol,
                datasize = entry.datasize,
                datamask = entry.datamask,
                writeable = entry.writeable,
                value = entry.value
            }
        end
    end
    return result
end

local function inventory_spaces(device)
    local result = {}
    for _, key in ipairs(sorted_keys(device.spaces)) do
        local space = device.spaces[key]
        result[#result + 1] = {
            name = space.name,
            index = space.index,
            shift = space.shift,
            address_mask = space.address_mask,
            data_width = space.data_width,
            endianness = space.endianness
        }
    end
    return result
end

local function inventory_devices(machine)
    local result = {}
    for _, key in ipairs(sorted_keys(machine.devices)) do
        local device = machine.devices[key]
        result[#result + 1] = {
            tag = device.tag,
            shortname = device.shortname,
            name = device.name,
            configured = device.configured,
            started = device.started,
            spaces = inventory_spaces(device),
            state = inventory_state(device)
        }
    end
    return result
end

local function inventory_inputs(machine)
    local result = {}
    for _, port_tag in ipairs(sorted_keys(machine.ioport.ports)) do
        local port = machine.ioport.ports[port_tag]
        local fields = {}
        for _, field_name in ipairs(sorted_keys(port.fields)) do
            local field = port.fields[field_name]
            fields[#fields + 1] = {
                name = field.name,
                mask = field.mask,
                defvalue = field.defvalue,
                minvalue = field.minvalue,
                maxvalue = field.maxvalue,
                player = field.player
            }
        end
        result[#result + 1] = { tag = port.tag, fields = fields }
    end
    return result
end

local function inventory_screens(machine)
    local result = {}
    for _, key in ipairs(sorted_keys(machine.screens)) do
        local screen = machine.screens[key]
        result[#result + 1] = {
            tag = screen.tag,
            shortname = screen.shortname,
            name = screen.name
        }
    end
    return result
end

local machine = manager.machine
local inventory = {
    schema = 'mame-runtime-inventory/v1',
    mame = { version = emu.app_version(), build = emu.app_build() },
    system = { name = machine.system.name, description = machine.system.description },
    devices = inventory_devices(machine),
    screens = inventory_screens(machine),
    inputs = inventory_inputs(machine)
}

print('MAME_INVENTORY_JSON ' .. json_encode(inventory))
machine:exit()
