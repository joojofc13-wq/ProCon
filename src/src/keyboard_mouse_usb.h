#pragma once
static OrbisPthread kbm_thread;
static int kbm_stop=0,kbm_done=0;
struct BootDevice {Device device;uint8_t protocol;bool claimed,switched,described;HidLayout mouse_layout;};
static void close_boot(BootDevice &b) {
    if(b.device.handle) {
        if(b.switched)p_sceUsbdControlTransfer(b.device.handle,0x21,0x0b,b.protocol,b.device.iface,0,0,500);
        if(b.claimed)p_sceUsbdReleaseInterface(b.device.handle,b.device.iface);
        p_sceUsbdClose(b.device.handle);
    }
    b={};
}
static bool open_boot(BootDevice &b,Driver kind) {
    BootStats &stats=boot_stats[kind==BOOT_MOUSE?1:0];
    if(!find_device(b.device,kind))return false;
    int r=p_sceUsbdClaimInterface(b.device.handle,b.device.iface);
    if(r){stats.stage=2;stats.error=r;error("Claim keyboard/mouse interface",r);close_boot(b);return false;}
    b.claimed=true;
    r=p_sceUsbdControlTransfer(b.device.handle,0xa1,3,0,b.device.iface,&b.protocol,1,500);
    if(r!=1 || b.protocol>1){stats.stage=3;stats.error=r;error("Read HID protocol",r);close_boot(b);return false;}
    if(kind==BOOT_MOUSE) {
        uint8_t descriptor[1024]={};
        int length=p_sceUsbdControlTransfer(b.device.handle,0x81,6,0x2200,b.device.iface,descriptor,sizeof(descriptor),500);
        b.described=length>0 && length<=1024 && parse_hid(descriptor,(size_t)length,b.mouse_layout,true);
    }
    r=p_sceUsbdControlTransfer(b.device.handle,0x21,0x0b,(kind==BOOT_MOUSE && b.described)?1:0,b.device.iface,0,0,500);
    if(r){stats.stage=4;stats.error=r;error("Select HID boot protocol",r);close_boot(b);return false;}
    b.switched=true;
    p_sceUsbdControlTransfer(b.device.handle,0x21,0x0a,4<<8,b.device.iface,0,0,100);
    stats.stage=5;stats.error=0;return true;
}
static int reader_done[2]={};
static OrbisPthread reader_threads[2];
static bool stop_input() {
    return __atomic_load_n(&kbm_stop,__ATOMIC_ACQUIRE) || __atomic_load_n(&disabled,__ATOMIC_ACQUIRE);
}
static void *boot_reader(void *argument) {
    unsigned index=(unsigned)(uintptr_t)argument;
    Driver kind=index?BOOT_MOUSE:BOOT_KEYBOARD;
    BootDevice device={};BootKeyboard keys={};MouseWindow motion={};
    BootStats &stats=boot_stats[index];Snapshot &output=index?mouse_input:keyboard_input;
    uint64_t retry=0,last_valid=0,last_control=0,last_loop=0,max_gap=0;
    uint64_t next_status=sceKernelGetProcessTime()+(index?25000000:15000000);
    unsigned summaries=0;
    while(!stop_input()) {
        uint64_t now=sceKernelGetProcessTime();
        if(!device.device.handle && now>=retry) {
            keys={};motion={};publish(output,neutral_pad(),0);
            open_boot(device,kind);retry=sceKernelGetProcessTime()+2000000;
            last_valid=last_control=now;
        }
        if(device.device.handle) {
            uint8_t packet[64]={};int n=0;
            int result=p_sceUsbdInterruptTransfer(device.device.handle,device.device.in,packet,device.device.in_size,&n,20);
            now=sceKernelGetProcessTime();
            if(usb_timeout(result) && n==0) {
                ++stats.timeouts;
                if(!index && now-last_valid>=500000 && now-last_control>=500000) {
                    last_control=now;
                    int got=p_sceUsbdControlTransfer(device.device.handle,0xa1,1,0x0100,device.device.iface,packet,8,20);
                    if(got==8){n=got;result=0;++stats.control;}
                }
            }
            if(!result || (usb_timeout(result) && n>0)) {
                bool valid=false;
                if(n>=0 && n<=64) {
                    if(index) {
                        BootMouse input={};valid=decode_mouse_compatible(device.mouse_layout,device.described,packet,(size_t)n,input);
                        if(valid){add_mouse(motion,input);if(input.buttons || input.x || input.y)++stats.actions;}
                    } else {
                        valid=decode_keyboard(packet,(size_t)n,keys);
                        if(valid){Mapped input=map_keyboard(keys);if(input.buttons || input.lx!=128 || input.ly!=128)++stats.actions;}
                    }
                }
                if(valid){++stats.valid;last_valid=now;notify_status(true);}
                else {++stats.invalid;if(!index){keys={};motion={};}}
            } else if(!usb_timeout(result)) {
                stats.stage=6;stats.error=result;
                error(index?"Mouse read":"Keyboard read",result);
                keys={};motion={};publish(output,neutral_pad(),0);close_boot(device);
            }
        }
        now=sceKernelGetProcessTime();
        if(device.device.handle) {
            publish(output,index?sample_mouse(motion,now):map_keyboard(keys),now);
            if(last_loop && now-last_loop>max_gap)max_gap=now-last_loop;
            last_loop=now;
        } else {last_loop=0;publish(output,neutral_pad(),0);}
        if(summaries<4 && now>=next_status) {
            Text t={};t.add("ProCon 0.3 ");t.add(index?"MOUSE":"KEYBOARD");
            t.add("\npackets=");t.number(stats.valid);t.add(" active=");t.number(stats.actions);
            t.add("\ngap_ms=");t.number((uint32_t)(max_gap/1000));t.add(" bad=");t.number(stats.invalid);
            t.add("\nerror=");t.hex((uint32_t)stats.error);notify(t.data);
            ++summaries;next_status=now+20000000;
        }
        sceKernelUsleep(device.device.handle?1000:10000);
    }
    publish(output,neutral_pad(),0);close_boot(device);
    __atomic_store_n(&reader_done[index],1,__ATOMIC_RELEASE);return 0;
}
static void *keyboard_mouse_worker(void*) {
    notify("ProCon 0.3: Independent keyboard and mouse readers.");
    for(unsigned i=0;i<2;++i) {
        int result=scePthreadCreate(&reader_threads[i],0,boot_reader,(void*)(uintptr_t)i,i?"procon-mouse":"procon-keyboard");
        if(result){error("Start input reader",result);__atomic_store_n(&reader_done[i],1,__ATOMIC_RELEASE);}
    }
    while(!__atomic_load_n(&reader_done[0],__ATOMIC_ACQUIRE) || !__atomic_load_n(&reader_done[1],__ATOMIC_ACQUIRE))sceKernelUsleep(10000);
    __atomic_store_n(&kbm_done,1,__ATOMIC_RELEASE);return 0;
}
