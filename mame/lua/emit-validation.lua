-- Validate the minimal emitted instruction fixtures through MAME's decoders.

local output = assert(os.getenv('STUNRUN_EMIT_VALIDATION_DIR'), 'output directory required')
local machine = manager.machine
local debugger = machine.debugger

machine.devices[':mainpcb:maincpu'].spaces.program:write_u16(0xff8000, 0x4e71)
machine.devices[':mainpcb:gsp'].spaces.program:write_u16(0xff800000, 0x0300)
machine.devices[':mainpcb:adsp'].spaces.program:write_u32(0, 0)
machine.devices[':mainpcb:jsa:cpu'].spaces.program:write_u8(0x1000, 0xea)

debugger:command('dasm ' .. output .. '/maincpu-68010.lst,0xff8000,2,0,:mainpcb:maincpu')
debugger:command('dasm ' .. output .. '/gsp-tms34010.lst,0xff800000,0x10,0,:mainpcb:gsp')
debugger:command('dasm ' .. output .. '/adsp2100.lst,0,1,0,:mainpcb:adsp')
debugger:command('dasm ' .. output .. '/sound-6502.lst,0x1000,1,0,:mainpcb:jsa:cpu')
manager.machine:exit()
