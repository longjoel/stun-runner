-- Human-play instrumented capture for S.T.U.N. Runner course hunting.
--
-- No scripted inputs: a human plays in real time (video + sound on,
-- throttled). Every frame watches the course word 0xFF9578 and the score
-- longword 0xFF9532, printing plus logging each change. Every EVERY frames
-- it dumps the full work-RAM window and (unless disabled) a MAME save
-- state, so a course transition can be rewound and re-examined.
--
-- Env: STUNRUN_PLAY_OUT (required output dir),
--      STUNRUN_PLAY_EVERY (snapshot cadence in frames, default 600),
--      STUNRUN_PLAY_FRAMES (auto-exit frame, 0 = run until window closes),
--      STUNRUN_PLAY_STATES (1 = save states, 0 = snapshots only).

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
local output = assert(os.getenv('STUNRUN_PLAY_OUT'), 'STUNRUN_PLAY_OUT is required')
local every = tonumber(os.getenv('STUNRUN_PLAY_EVERY') or '600')
local limit = tonumber(os.getenv('STUNRUN_PLAY_FRAMES') or '21600')
local want_states = (os.getenv('STUNRUN_PLAY_STATES') or '1') ~= '0'
assert(every > 0, 'STUNRUN_PLAY_EVERY must be positive')

local device_tag = ':mainpcb:maincpu'
local space_name = 'program'
local base = 0xFF8000
local count = 0x80000
local width = 8
local course_addr = 0xFF9578
local score_addr = 0xFF9532

local device = assert(machine.devices[device_tag], 'unknown device: ' .. device_tag)
local space = assert(device.spaces[space_name], 'unknown space: ' .. space_name)

local frame = 0
local snapshots = 0
local last_course = nil
local last_score = nil
local log_handle = assert(io.open(output .. '/course-log.txt', 'w'))
local write_handle = assert(io.open(output .. '/course-writes.txt', 'w'))
local pc_state = device.state['CURPC'] or device.state['PC']
local course_write_tap
log_handle:write('frame course score_lo\n')
write_handle:write('frame pc address data mask\n')

local function read_u16(addr)
    return space:read_u8(addr) * 256 + space:read_u8(addr + 1)
end

local function read_u32(addr)
    return ((space:read_u8(addr) * 256 + space:read_u8(addr + 1)) * 256
        + space:read_u8(addr + 2)) * 256 + space:read_u8(addr + 3)
end

-- During reset/service transitions the work-RAM window can temporarily read
-- as bus-fill 0xffff values.  Those are not valid gameplay observations and
-- must not become apparent course/score changes in the human-run log.  The
-- largest statically observed state value is 0x16; retain a little headroom
-- for an as-yet-unseen progression state without accepting the fill pattern.
local function valid_gameplay_sample(course, score)
    return course ~= 0xffff and course <= 0x20 and score ~= 0xffffffff
end

local function capture()
    local values = {}
    local nonzero = 0
    for offset = 0, count - 1 do
        local value = space:read_u8(base + offset)
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
        input = 'human-play',
        nonzero = nonzero,
        values = values
    }
    local path = output .. '/snapshot-' .. tostring(frame) .. '.json'
    local handle = assert(io.open(path, 'w'))
    handle:write(json(snapshot) .. '\n')
    handle:close()
    snapshots = snapshots + 1
    print(string.format('MAME_PLAY_SNAPSHOT frame=%d path=%s nonzero=%d', frame, path, nonzero))
    if want_states then
        local state_path = output .. '/state-' .. tostring(frame) .. '.sta'
        machine:save(state_path)
        print('MAME_PLAY_STATE_SAVE path=' .. state_path .. ' frame=' .. frame)
    end
end

local function install_course_write_tap()
    -- The inclusive end address is widened for the 68010 bus tap, then
    -- filtered back to the exact two-byte word in the callback.
    course_write_tap = space:install_write_tap(
        course_addr, course_addr + 1, 'stunrun_play_course_write',
        function(offset, data, mask)
            if offset >= course_addr and offset <= course_addr + 1 then
                write_handle:write(string.format('%d %08X %08X %08X %08X\n',
                    frame, pc_state.value, offset, data, mask))
                write_handle:flush()
            end
        end)
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame == 10 then install_course_write_tap() end
    local course = read_u16(course_addr)
    local score = read_u32(score_addr)
    if not valid_gameplay_sample(course, score) then
        -- Keep waiting for a real RAM sample.  In particular, do not update
        -- the previous values with a transient invalid read.
    elseif last_course == nil then
        last_course = course
        last_score = score
        print(string.format('MAME_PLAY_START frame=%d course=%d score=%d', frame, course, score))
    elseif course ~= last_course or score ~= last_score then
        print(string.format('MAME_PLAY_CHANGE frame=%d course=%d (was %d) score=%d (was %d)',
            frame, course, last_course, score, last_score))
        log_handle:write(string.format('%d %d %d\n', frame, course, score % 65536))
        log_handle:flush()
        last_course = course
        last_score = score
    end
    if frame % every == 0 then capture() end
    if limit > 0 and frame >= limit then
        log_handle:close()
        write_handle:close()
        print(string.format('MAME_PLAY_DONE frames=%d snapshots=%d', frame, snapshots))
        machine:exit()
    end
end)
