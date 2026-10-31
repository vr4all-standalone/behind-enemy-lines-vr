-- Private original-game calibration/aim experiment. Cabinet inputs only;
-- RAM/ROM/CPU state are observed without modifying the native program.
-- Use baseline attract mode and a private seed of native saved calibration.
-- Baseline service mode also writes cabinet inputs, so must not be combined.
dofile(assert(os.getenv('BEL_STATE_SCRIPT')))
local machine=manager.machine
local cpu=machine.devices[':maincpu']
local memory=cpu.spaces.program
assert(memory:read_u32(0x56690)==0x80803000,'Not the verified BEL program')
local root=assert(os.getenv('BEL_RESEARCH_DIR'))
local begin=tonumber(os.getenv('BEL_AIM_START_SECONDS') or '58')
local csv=assert(io.open(root..'/aim-samples.csv','w'))
csv:write('time,player,mode,x,y,xmin,ymin,dx,dy,xhalf,yhalf,focus,scale,xlimit,yfloor,ybase,yaw,pitch,point_x,point_y,point_z,origin_x,origin_y,origin_z,ip,in1,prior_yaw\n')
csv:flush()
local sites=assert(io.open(root..'/aim-tap-sites.csv','w'))
sites:write('time,player,ip,fp\n');sites:flush()
local seen={}
local angles=assert(io.open(root..'/angle-samples.csv','w'))
angles:write('time,player,mode,life,x,y,xhalf,yhalf,focus,scale,xlimit,yfloor,ybase,yaw,pitch,start1,start2\n');angles:flush()
local function f32(address)
    return string.unpack('<f',string.pack('<I4',memory:read_u32(address)))
end
local function s16(address)
    local value=memory:read_u16(address)
    return value>=0x8000 and value-0x10000 or value
end
local function set(tag,mask,value)
    for _,field in pairs(assert(machine.ioport.ports[tag]).fields) do
        if field.mask==mask then field:set_value(value);return end
    end
    error('Missing native cabinet field '..tag)
end
local function press(mask,enabled) set(':IN0',mask,enabled and 1 or 0) end
local request={};local live_since
bel_aim_sweep=emu.add_machine_frame_notifier(function()
    if os.getenv('BEL_AIM_OBSERVE_ONLY')=='1' then return end
    local time=machine.time:as_double()
    if time<begin then return end
    local mode=memory:read_u8(0x5fa12c)
    local phase=memory:read_u8(0x5b4b5c)
    local ready=memory:read_u8(0x5a8831)==1 and (mode==4 and phase==0 or mode==10 and phase==1 or mode==21)
    local player=s16(0x5b4996)>0 and 1 or 0
    local address=player==0 and 0x5b4996 or 0x5b4a1a
    if ready and s16(address)<=0 and (not request[player] or time-request[player]>=2) then request[player]=time end
    for p=0,1 do
        local elapsed=request[p] and time-request[p] or -1
        press(1<<p,elapsed>=0 and elapsed<.2 and memory:read_u8(0x5a87f0)<memory:read_u8(0x5a85ff))
        press(0x10<<p,elapsed>=.5 and elapsed<.8)
    end
    if mode==21 and s16(0x5b4996)>0 and s16(0x5b4a1a)>0 then
        if not live_since then live_since=time end
        -- Five-by-five stops plus intermediate coordinates, held for .3s.
        -- The player grids traverse opposite orders, exposing ownership mixups.
        local cell=math.floor((time-live_since)/.3)%25
        for p=0,1 do
            local index=p==0 and cell or 24-cell
            local u=(index%5)/4;local v=math.floor(index/5)/4
            set(p==0 and ':P1_X' or ':P2_X',0xff,math.floor((p==0 and 0x69 or 0)+u*0x96+.5))
            set(p==0 and ':P1_Y' or ':P2_Y',0xff,math.floor(0x11+v*0x9d+.5))
            if os.getenv('BEL_AIM_FIRE')=='1' then
                set(':IN1',1<<p,time>=live_since+3 and 1 or 0)
            end
        end
    end
    if mode==21 then
        for p=0,1 do
            local base=0x5b4940+p*0x84
            local xptr=memory:read_u32(0x5b4c44+p*4)
            local yptr=memory:read_u32(0x5a8130+p*4)
            local row={time,p,mode,s16(base+0x56),memory:read_u8(xptr),memory:read_u8(yptr),
                f32(0x5a021c+p*4),f32(0x5a0224+p*4),f32(0x5b489c),f32(0x5b4aa8),
                s16(0x5b4a80),s16(0x5b4b1a),s16(0x5b4b1c),s16(base+0x44),s16(base+0x46),
                memory:read_u8(0x5a87ac),memory:read_u8(0x5a87ad)}
            for i,value in ipairs(row) do if i>1 then angles:write(',') end;angles:write(string.format('%.12g',value)) end
            angles:write('\n');angles:flush()
        end
    end
end)
bel_aim_taps={}
bel_aim_angle_taps={}
local prior_yaw={}
local resets=assert(io.open(root..'/native-angle-resets.csv','w'))
resets:write('time,player,ip,old_yaw,data,mask\n');resets:flush()
for p=0,1 do
    local player=p
    local base=0x5b4940+player*0x84
    bel_aim_angle_taps[p+1]=memory:install_write_tap(base+0x44,base+0x47,'bel_prior_yaw_'..p,
        function(address,data,mask)
            local ip=cpu.state['ip'].value
            if ip==0x63784 then prior_yaw[player]=s16(base+0x44) end
            if (mask&0xffff)~=0 and ip~=0x637e0 and machine.time:as_double()>=begin then
                resets:write(string.format('%.9f,%d,%d,%d,%d,%d\n',machine.time:as_double(),player,ip,s16(base+0x44),data,mask));resets:flush()
            end
        end)
    -- The original 0x63c7c store follows TGP transformation of the aim point.
    -- Read its still-live stack result, then the native cabinet/angle inputs.
    bel_aim_taps[p+1]=memory:install_write_tap(base+0x18,base+0x1b,'bel_aim_'..p,
        function(address,data,mask)
            local ip=cpu.state['ip'].value
            local frame=cpu.state['fp'].value
            if machine.time:as_double()>=begin and not seen[ip] then
                seen[ip]=true;sites:write(string.format('%.9f,%d,%d,%d\n',machine.time:as_double(),player,ip,frame));sites:flush()
            end
            -- The tap observes IP after the four-byte store at 0x63c80.
            if ip~=0x63c84 or machine.time:as_double()<begin then return end
            -- Model2C has a distinct 256 KiB local CPU RAM/stack bank.
            if frame<0x200000 or frame>0x23ff00 then return end
            local xptr=memory:read_u32(0x5b4c44+player*4)
            local yptr=memory:read_u32(0x5a8130+player*4)
            if xptr<0x500000 or xptr>0x5fffff or yptr<0x500000 or yptr>0x5fffff then return end
            local row={machine.time:as_double(),player,memory:read_u8(0x5fa12c),
                memory:read_u8(xptr),memory:read_u8(yptr),
                memory:read_u8(0x5a8607+player),memory:read_u8(0x5a8609+player),
                memory:read_u8(0x5a860b+player),memory:read_u8(0x5a860d+player),
                f32(0x5a021c+player*4),f32(0x5a0224+player*4),
                f32(0x5b489c),f32(0x5b4aa8),s16(0x5b4a80),s16(0x5b4b1a),s16(0x5b4b1c),
                s16(base+0x44),s16(base+0x46),
                f32(frame+0x50),f32(frame+0x54),f32(frame+0x58),
                f32(base+0x2c),f32(base+0x30),f32(base+0x34),ip,memory:read_u8(0x1c00004),prior_yaw[player] or 0/0}
            for i,value in ipairs(row) do if i>1 then csv:write(',') end;csv:write(string.format('%.12g',value)) end
            csv:write('\n');csv:flush()
        end)
end
