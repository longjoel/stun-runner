-- Generate bounded static listings for every active programmable processor.

local output = os.getenv('STUNRUN_LISTING_DIR') or '/tmp/stunrun-listings'
local listings = {
    {id='maincpu-68010', tag=':mainpcb:maincpu', address='0', length='0x100000'},
    {id='gsp-tms34010', tag=':mainpcb:gsp', address='0', length='0x100000'},
    {id='adsp2100', tag=':mainpcb:adsp', address='0', length='0x4000'},
    {id='sound-6502', tag=':mainpcb:jsa:cpu', address='0', length='0x10000'}
}

for _, listing in ipairs(listings) do
    local filename = output .. '/' .. listing.id .. '.lst'
    manager.machine.debugger:command('dasm ' .. filename .. ',' .. listing.address .. ',' .. listing.length .. ',0,' .. listing.tag)
    print('MAME_LISTING id=' .. listing.id .. ' tag=' .. listing.tag .. ' file=' .. filename .. ' length=' .. listing.length)
end

manager.machine:exit()
