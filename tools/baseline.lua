-- BEL native cabinet diagnostic. Do not alter RAM, boot flow or native state.
local root=assert(os.getenv('BEL_RESEARCH_DIR'))
local mode=os.getenv('BEL_DIAGNOSTIC_MODE') or 'attract'
local page=os.getenv('BEL_OPERATOR_PAGE') or 'menu'
local machine=manager.machine
local ports=machine.ioport.ports
local log=assert(io.open(root..'/events.txt','w'))
for tag,port in pairs(ports) do
    for name,field in pairs(port.fields) do
        log:write(string.format('PORT %s | %s | %x\n',tag,name,field.mask))
    end
end
log:flush()
local function set(tag,mask,value)
    for _,field in pairs(assert(ports[tag]).fields) do
        if field.mask==mask then field:set_value(value);return end
    end
    error('Missing native field '..tag..' '..mask)
end
local function press(tag,mask,enabled) set(tag,mask,enabled and 1 or 0) end
local previous=-1
bel_diagnostic_subscription=emu.add_machine_frame_notifier(function()
    local t=machine.time:as_double();local second=math.floor(t)
    if second~=previous then
        previous=second
        log:write(string.format('TIME %.6f IP %08x IN0 %02x IN1 %02x X1 %02x Y1 %02x X2 %02x Y2 %02x\n',t,machine.devices[':maincpu'].state['ip'].value,ports[':IN0']:read(),ports[':IN1']:read(),ports[':P1_X']:read(),ports[':P1_Y']:read(),ports[':P2_X']:read(),ports[':P2_Y']:read()))
        log:flush()
        if second==1 or second%5==0 or mode=='service' and second>=28 then
            for _,screen in pairs(machine.screens) do screen:snapshot(root..string.format('/frame-%03d.png',second));break end
        end
        if second%5==0 and second>0 then
            local out=assert(io.open(root..string.format('/workram-%03d.bin',second),'wb'))
            out:write(machine.devices[':maincpu'].spaces.program:read_range(0x500000,0x5fffff,8));out:close()
        end
    end
    if mode=='gameplay' then
        local p2first=os.getenv('BEL_P2_FIRST')=='1'
        press(':IN0',p2first and 2 or 1,t>=16 and t<16.2)
        press(':IN0',p2first and 0x20 or 0x10,t>=16.5 and t<16.8)
        press(':IN0',p2first and 1 or 2,t>=30 and t<30.2)
        press(':IN0',p2first and 0x10 or 0x20,t>=30.5 and t<30.8)
        press(':IN1',1,t>=24 and t<40)
        press(':IN1',2,t>=32 and t<44)
        press(':IN1',0x10,t>=26 and t<26.3)
        press(':IN1',0x20,t>=34 and t<34.3)
    elseif mode=='service' then
        local yup=os.getenv('BEL_CALIBRATE_Y_UP')=='1'
        -- BEL swaps the inherited switches: Test 0x08, Service 0x04.
        local advances=page=='input' and 4 or page=='aim' and 3 or 0
        -- Observed native order from EXIT: Bookkeeping, Game, Gun, Input.
        press(':IN0',8,t>=18 and t<18.3 or t>=28 and t<28.3 or yup and (t>=44 and t<44.2 or t>=56 and t<56.2))
        press(':IN0',4,(t>=22 and t<22+advances or yup and t>=47 and t<54) and t-math.floor(t)<.2)
        if page=='input' then
            local u=t>=32 and t<34 and 0 or t>=34 and t<36 and 1 or .5
            set(':P1_X',0xff,math.floor(0x69+u*0x96+.5))
            set(':P2_X',0xff,math.floor(u*0x96+.5))
            set(':P1_Y',0xff,math.floor(0x11+u*0x9d+.5))
            set(':P2_Y',0xff,math.floor(0x11+u*0x9d+.5))
            press(':IN1',1,t>=32 and t<34)
            press(':IN1',2,t>=34 and t<36)
            press(':IN1',0x10,t>=36 and t<38)
            press(':IN1',0x20,t>=38 and t<40)
        elseif page=='aim' and os.getenv('BEL_CALIBRATE_STOPS')=='1' then
            -- Explicit private diagnostic only. Match the observed native
            -- lower-left / upper-right prompts using the audited ADC stops.
            set(':P1_X',0xff,t<35 and 0x69 or 0xff)
            set(':P1_Y',0xff,yup and (t<35 and 0x11 or 0xae) or (t<35 and 0xae or 0x11))
            set(':P2_X',0xff,t<41 and 0x00 or 0x96)
            set(':P2_Y',0xff,yup and (t<41 and 0x11 or 0xae) or (t<41 and 0xae or 0x11))
            press(':IN1',1,t>=32 and t<32.2 or t>=35 and t<35.2)
            press(':IN1',2,t>=38 and t<38.2 or t>=41 and t<41.2)
            if yup and t>=58 then
                set(':P1_X',0xff,0xb4);set(':P2_X',0xff,0x4b)
                set(':P1_Y',0xff,0x60);set(':P2_Y',0xff,0x60)
                press(':IN0',1,t>=60 and t<60.2)
                press(':IN0',0x10,t>=60.5 and t<60.8)
            end
        end
    end
end)
