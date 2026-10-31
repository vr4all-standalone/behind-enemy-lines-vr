-- Original BEL controls observation. Only native cabinet inputs are written.
-- No ROM, RAM, CPU registers, lives or weapon quantities are modified.
dofile(assert(os.getenv('BEL_STATE_SCRIPT')))
local machine=manager.machine
local cpu=machine.devices[':maincpu'];local memory=cpu.spaces.program
local root=assert(os.getenv('BEL_RESEARCH_DIR'))
local out=assert(io.open(root..'/controls.csv','w'))
out:write('time,mode,submode,player,life,credits,in0,in1,decoded,block\n')
local function set(tag,mask,value)
    for _,field in pairs(assert(machine.ioport.ports[tag]).fields) do
        if field.mask==mask then field:set_value(value and 1 or 0);return end
    end
    error('Missing original cabinet field '..tag)
end
local function half(address)local v=memory:read_u16(address);return v>=0x8000 and v-0x10000 or v end
local function hex(s)return (s:gsub('.',function(c)return string.format('%02x',c:byte())end))end
local requested={};local joined_once={};local live_since;local continue_since;local prior=''
bel_controls_observer=emu.add_machine_frame_notifier(function()
    local time=machine.time:as_double();local mode=memory:read_u8(0x5fa12c)
    local sub=memory:read_u8(0x5b4b5c)
    local ready=(mode==4 and sub==0 or mode==10 and sub==1 or mode==21) and memory:read_u8(0x5a8831)==1
    for p=0,1 do
        local life=half(0x5b4996+p*0x84)
        if life>0 then joined_once[p]=true end
        if time>=16 and ready and not joined_once[p] and (p==0 or half(0x5b4996)>0) and life==0 and (not requested[p] or time-requested[p]>2) then requested[p]=time end
        local elapsed=requested[p] and time-requested[p] or -1
        set(':IN0',1<<p,elapsed>=0 and elapsed<.2)
        set(':IN0',0x10<<p,elapsed>=.5 and elapsed<.8)
        for _,axis in ipairs({{p==0 and ':P1_X' or ':P2_X',p==0 and 0xb4 or 0x4b},{p==0 and ':P1_Y' or ':P2_Y',0x60}}) do
            for _,field in pairs(machine.ioport.ports[axis[1]].fields) do if field.mask==0xff then field:set_value(axis[2]) end end
        end
    end
    if mode==21 and half(0x5b4996)>0 and half(0x5b4a1a)>0 and not live_since then live_since=time end
    local t=live_since and time-live_since or -100
    set(':IN1',1,t>=3 and t<3.12)
    set(':IN1',2,t>=5 and t<5.12)
    set(':IN1',0x10,t>=8 and t<8.25 or t>=12 and t<12.25)
    set(':IN1',0x20,t>=10 and t<10.25 or t>=14 and t<14.25)
    if (mode==23 or mode==27 or mode==29) and os.getenv('BEL_OBSERVE_CONTINUE')=='1' then
        if not continue_since then continue_since=time end
        local phase=(time-continue_since)%2
        for p=0,1 do
            local dead=half(0x5b4996+p*0x84)<0
            set(':IN0',1<<p,dead and phase<.2)
            set(':IN0',0x10<<p,dead and phase>=.5 and phase<.8)
        end
    end
    if mode==21 then continue_since=nil end
    local status=string.format('%d/%d',mode,sub)
    if status~=prior then
        prior=status
        if mode==23 or mode==27 or mode==29 then
            for _,screen in pairs(machine.screens) do screen:snapshot(root..string.format('/control-mode-%d-%.3f.png',mode,time));break end
        end
    end
    for p=0,1 do
        out:write(string.format('%.9f,%d,%d,%d,%d,%d,%d,%d,%s,%s\n',time,mode,sub,p,half(0x5b4996+p*0x84),memory:read_u8(0x5a87f0),machine.ioport.ports[':IN0']:read(),machine.ioport.ports[':IN1']:read(),hex(memory:read_range(0x5a87a0,0x5a87bf,8)),hex(memory:read_range(0x5b4940+p*0x84,0x5b49c3+p*0x84,8))))
    end
    out:flush()
end)
