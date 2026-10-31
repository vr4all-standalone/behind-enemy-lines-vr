-- Original BEL fire-path observation. Only cabinet inputs are written.
-- RAM/ROM/CPU state are observed without replacement or patching.
dofile(assert(os.getenv('BEL_STATE_SCRIPT')))
local machine=manager.machine
local memory=machine.devices[':maincpu'].spaces.program
local cpu=machine.devices[':maincpu']
local root=assert(os.getenv('BEL_RESEARCH_DIR'))
local rows=assert(io.open(root..'/fire-state.csv','w'))
rows:write('time,phase,player,in1,life,bytes\n')
local writes=assert(io.open(root..'/fire-writes.csv','w'))
writes:write('time,player,offset,ip,old,value,mask,in1\n')
local shots=assert(io.open(root..'/fire-shots.csv','w'))
shots:write('time,player,count,projectile,owner,in1\n')
local function set(tag,mask,value)
    for _,f in pairs(assert(machine.ioport.ports[tag]).fields) do
        if f.mask==mask then f:set_value(value);return end
    end
    error('Missing original cabinet field')
end
local function half(a)
    local x=memory:read_u16(a);return x>=0x8000 and x-0x10000 or x
end
local function hex(data)return (data:gsub('.',function(c)return string.format('%02x',c:byte())end))end
local requests={};local live_since
bel_fire_taps={}
for p=0,1 do
    local player=p;local base=0x5b4940+p*0x84
    bel_fire_taps[p+1]=memory:install_write_tap(base+0x24,base+0x83,'bel_fire_fields_'..p,
        function(address,data,mask)
            if not live_since then return end
            local old=memory:read_u32(address);local value=(old&(~mask))|(data&mask)
            if old==value then return end
            if address==base+0x24 and cpu.state['ip'].value==0x60c84 then
                local projectile=cpu.state['g0'].value
                local owner=projectile>=0x500000 and projectile<0x600000 and memory:read_u8(projectile+0x30) or -1
                shots:write(string.format('%.9f,%d,%d,%d,%d,%d\n',machine.time:as_double(),player,value,projectile,owner,memory:read_u8(0x1c00004)));shots:flush()
            end
            writes:write(string.format('%.9f,%d,%d,%d,%d,%d,%d,%d\n',machine.time:as_double(),player,address-base,cpu.state['ip'].value,old,value,mask,memory:read_u8(0x1c00004)))
        end)
end
bel_fire_observer=emu.add_machine_frame_notifier(function()
    local t=machine.time:as_double();local mode=memory:read_u8(0x5fa12c)
    local sub=memory:read_u8(0x5b4b5c)
    local ready=memory:read_u8(0x5a8831)==1 and (mode==4 and sub==0 or mode==10 and sub==1 or mode==21)
    for p=0,1 do
        if ready and half(0x5b4996+p*0x84)<=0 and (not requests[p] or t-requests[p]>2) then requests[p]=t end
        local elapsed=requests[p] and t-requests[p] or -1
        set(':IN0',1<<p,elapsed>=0 and elapsed<.2 and memory:read_u8(0x5a87f0)<2 and 1 or 0)
        set(':IN0',0x10<<p,elapsed>=.5 and elapsed<.8 and 1 or 0)
        set(p==0 and ':P1_X' or ':P2_X',0xff,p==0 and 0xb4 or 0x4b)
        set(p==0 and ':P1_Y' or ':P2_Y',0xff,0x60)
    end
    if mode~=21 or half(0x5b4996)<=0 or half(0x5b4a1a)<=0 then return end
    if not live_since then live_since=t end
    local phase=math.floor((t-live_since)/3)%10
    local continuous=os.getenv('BEL_FIRE_CONTINUOUS')=='1' and t-live_since>=3
    set(':IN1',1,(continuous or phase==1 or phase==2 or phase==5 or phase==6) and 1 or 0)
    set(':IN1',2,(continuous or phase==3 or phase==4 or phase==5 or phase==6) and 1 or 0)
    if continuous then phase=-1 end
    for p=0,1 do rows:write(string.format('%.9f,%d,%d,%d,%d,%s\n',t,phase,p,memory:read_u8(0x1c00004),half(0x5b4996+p*0x84),hex(memory:read_range(0x5b4940+p*0x84,0x5b49c3+p*0x84,8)))) end
    rows:flush();writes:flush()
end)
