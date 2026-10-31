-- Read-only BEL state/credit/join observations. All writes are native cabinet
-- inputs from baseline.lua; taps observe existing stores without replacing data.
dofile(assert(os.getenv('BEL_BASELINE_SCRIPT')))
local machine=manager.machine
local cpu=machine.devices[':maincpu']
local space=cpu.spaces.program
local root=assert(os.getenv('BEL_RESEARCH_DIR'))
local out=assert(io.open(root..'/state-events.txt','w'))
local function signed16(address)
    local value=space:read_u16(address)
    return value>=0x8000 and value-0x10000 or value
end
bel_state_taps={}
for index,range in ipairs({{0x5fa12c,0x5fa12f},{0x5a8830,0x5a8833},
                           {0x5b4994,0x5b4997},{0x5b4a18,0x5b4a1b},
                           {0x5a87f0,0x5a87f3}}) do
    bel_state_taps[index]=space:install_write_tap(range[1],range[2],'bel_state_'..index,
        function(address,data,mask)
            out:write(string.format('STORE %.6f IP %08x ADDR %08x DATA %08x MASK %08x\n',
                machine.time:as_double(),cpu.state['ip'].value,address,data,mask))
        end)
end
local previous=''
bel_state_observer=emu.add_machine_frame_notifier(function()
    local config=space:read_u32(0x5a85e8)
    local summary=string.format('EXEC %02x MODE %02x PHASE %02x P1 %d P2 %d CREDITS %d FREE %d CONFIG %08x COST %d/%d MENU %d/%d/%d',
        space:read_u8(0x5a8831),space:read_u8(0x5fa12c),space:read_u8(0x5b4b5c),
        signed16(0x5b4996),signed16(0x5b4a1a),space:read_u8(0x5a87f0),space:read_u8(0x5a8810),config,
        config>=0x500000 and config<0x600000 and space:read_u8(config+0xf) or -1,
        config>=0x500000 and config<0x600000 and space:read_u8(config+0x10) or -1,
        space:read_u8(0x5a883c),space:read_u8(0x5a883d),space:read_u8(0x5b4b64))
    if summary~=previous then
        previous=summary
        out:write(string.format('STATE %.6f IP %08x %s\n',machine.time:as_double(),cpu.state['ip'].value,summary))
        out:flush()
    end
end)
