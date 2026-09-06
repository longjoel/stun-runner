-- Bounded 68010 watchpoint experiment for the MAME-confirmed ADSP windows.

local output = assert(os.getenv('STUNRUN_ADSP_WINDOW_OUT'), 'STUNRUN_ADSP_WINDOW_OUT is required')
local limit = tonumber(os.getenv('STUNRUN_ADSP_WINDOW_FRAMES') or '600')
local base = assert(os.getenv('STUNRUN_ADSP_WINDOW_BASE'), 'STUNRUN_ADSP_WINDOW_BASE is required')
local length = assert(os.getenv('STUNRUN_ADSP_WINDOW_LENGTH'), 'STUNRUN_ADSP_WINDOW_LENGTH is required')
local label = assert(os.getenv('STUNRUN_ADSP_WINDOW_LABEL'), 'STUNRUN_ADSP_WINDOW_LABEL is required')
local input_mode = os.getenv('STUNRUN_ADSP_WINDOW_INPUT') or 'none'
local access = os.getenv('STUNRUN_ADSP_WINDOW_ACCESS') or 'write'
local frame = 0
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
local events = input_mode == 'late_drive' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'},
    {frame = 900, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 220},
    {frame = 900, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'press'},
    {frame = 1500, port = ':mainpcb:8BADC.0', field = 'AD Stick X', action = 'set', value = 128},
    {frame = 1500, port = ':mainpcb:a80000', field = 'P1 Button 1', action = 'release'}
} or input_mode == 'late' and {
    {frame = 650, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 680, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 750, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 780, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'coin_start' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or input_mode == 'sw_off_prestart' and {
    {frame = 120, port = ':mainpcb:IN0', field = 'Coin 1', action = 'press'},
    {frame = 122, port = ':mainpcb:IN0', field = 'Coin 1', action = 'release'},
    {frame = 300, port = ':mainpcb:a80000', field = '1 Player Start', action = 'press'},
    {frame = 302, port = ':mainpcb:a80000', field = '1 Player Start', action = 'release'}
} or {}
local next_event = 1

local function apply_event(event)
    local port = manager.machine.ioport.ports[event.port]
    assert(port ~= nil, 'unknown input port: ' .. event.port)
    local field = port.fields[event.field]
    assert(field ~= nil, 'unknown input field: ' .. event.port .. '/' .. event.field)
    if event.action == 'press' then field:set_value(1) elseif event.action == 'release' then field:set_value(0) else field:set_value(event.value) end
    print('M1_ADSP_INPUT frame=' .. frame .. ' port=' .. event.port .. ' field=' .. event.field .. ' action=' .. event.action)
end

if input_mode == 'sw_off_prestart' then
    emu.register_prestart(function()
        for _, event in ipairs(sw_off_prefix) do
            apply_event(event)
        end
    end)
end

local tap
local source_tap
local source_bytes_tap
local gsp_fifo_tap
local block_sequence = 0
local active_block_count
local active_fifo_writes = 0
local function install_tap()
    local cpu = manager.machine.devices[':mainpcb:maincpu']
    local space = cpu.spaces['program']
    local start_address = tonumber(base)
    local end_address = start_address + tonumber(length) - 1
    local pc = cpu.state['CURPC']
    if access == 'read' then
        tap = space:install_read_tap(start_address, end_address, 'stunrun_adsp_window_read',
        function(offset, data, mask)
            if pc.value == 0x02f0c2 then
                block_sequence = block_sequence + 1
                active_block_count = data
                active_fifo_writes = 0
            elseif pc.value == 0x02f0e2 and active_block_count ~= nil then
                print(string.format('M1_ADSP_BLOCK frame=%d sequence=%d count=%d terminator=%04X fifo_writes=%d',
                    frame, block_sequence, active_block_count, data, active_fifo_writes))
                active_block_count = nil
            end
            print(string.format('M1_ADSP_READ kind=%s pc=%08X addr=%08X data=%08X mask=%08X',
                label, pc.value, offset, data, mask))
            end)
    else
        tap = space:install_write_tap(start_address, end_address, 'stunrun_adsp_window_write',
            function(offset, data, mask)
                print(string.format('M1_ADSP_WRITE kind=%s pc=%08X addr=%08X data=%08X',
                    label, pc.value, offset, data))
            end)
    end
    -- The observed title-path caller passes the source pointer from $17000
    -- into 0x02d2e0. Keep this narrow diagnostic alongside the window tap so
    -- the upload destination can be paired with its source pointer.
    source_tap = space:install_read_tap(0x17000, 0x17007, 'stunrun_adsp_source',
        function(offset, data, mask)
            if pc.value == 0x02c204 or pc.value == 0x02c1b6 then
                print(string.format('M1_ADSP_SOURCE pc=%08X addr=%08X data=%08X mask=%08X',
                    pc.value, offset, data, mask))
                end
        end)
    source_bytes_tap = space:install_read_tap(0x17000, 0x1ffff, 'stunrun_adsp_source_bytes',
        function(offset, data, mask)
            if pc.value >= 0x02d2e0 and pc.value <= 0x02d364 then
                print(string.format('M1_ADSP_SOURCE_BYTE pc=%08X addr=%08X data=%08X mask=%08X',
                    pc.value, offset, data, mask))
                end
        end)
    gsp_fifo_tap = space:install_write_tap(0xc0000c, 0xc0000f, 'stunrun_gsp_fifo',
        function(offset, data, mask)
            if active_block_count ~= nil and (pc.value == 0x02248e or pc.value == 0x02249a) then
                active_fifo_writes = active_fifo_writes + 1
            end
        end)
end

emu.register_frame_done(function()
    frame = frame + 1
    if frame == 10 then
        install_tap()
    end
    while next_event <= #events and events[next_event].frame == frame do
        apply_event(events[next_event])
        next_event = next_event + 1
    end
    if frame >= limit then
        print('M1_ADSP_WINDOW_DONE frames=' .. frame .. ' output=' .. output)
        manager.machine:exit()
    end
end)
