#include <stdint.h>
#include <stddef.h>
#include <orbis/libkernel.h>
#include <orbis/Usbd.h>
#include <orbis/Sysmodule.h>
#include "generic_hid.h"
#include "keyboard_mouse.h"
#include "usb_command.h"
extern "C" __attribute__((visibility("hidden"))) void *memset(void *p,int v,size_t n) {
    volatile uint8_t *d=(volatile uint8_t*)p; for(size_t i=0;i<n;++i) d[i]=(uint8_t)v; return p;
}
extern "C" __attribute__((visibility("hidden"))) void *memcpy(void *p,const void *q,size_t n) {
    volatile uint8_t *d=(volatile uint8_t*)p; const uint8_t *s=(const uint8_t*)q;
    for(size_t i=0;i<n;++i) d[i]=s[i]; return p;
}
struct Text {
    char data[512]; size_t used;
    void add(const char *s) { if(s) while(*s && used+1<sizeof(data)) data[used++]=*s++; data[used]=0; }
    void number(uint32_t n) { char tmp[11]; unsigned i=0; do { tmp[i++]=(char)('0'+n%10); n/=10; } while(n); while(i && used+1<sizeof(data)) data[used++]=tmp[--i]; data[used]=0; }
    void hex(uint32_t n) { add("0x"); for(int i=7;i>=0;--i) { char c[2]={"0123456789ABCDEF"[(n>>(i*4))&15],0}; add(c); } }
};
static void notify(const char *) {}
static int status_sent=0;
static void notify_status(bool connected) {
    if(__atomic_exchange_n(&status_sent,1,__ATOMIC_ACQ_REL))return;
    const char *text=connected?"Connected":"Error";
    OrbisNotificationRequest r; memset(&r,0,sizeof(r)); r.targetId=-1;
    size_t i=0; while(text[i] && i+1<sizeof(r.message)) { r.message[i]=text[i]; ++i; }
    sceKernelSendNotificationRequest(0,&r,sizeof(r),0);
}
static void error(const char *,int) {notify_status(false); }
extern "C" {
const char *g_pluginName="ProCon USB plugin";
const char *g_pluginDesc="Experimental USB to Pad bridge";
const char *g_pluginAuth="nickyrmldy";
uint32_t g_pluginVersion=0x00000300;
}
#define USB_FUNCTIONS(X) \
 X(sceUsbdInit) X(sceUsbdExit) X(sceUsbdGetDeviceList) X(sceUsbdFreeDeviceList) \
 X(sceUsbdGetDeviceDescriptor) X(sceUsbdGetActiveConfigDescriptor) X(sceUsbdFreeConfigDescriptor) \
 X(sceUsbdOpen) X(sceUsbdClose) X(sceUsbdClaimInterface) X(sceUsbdReleaseInterface) \
 X(sceUsbdSetInterfaceAltSetting) X(sceUsbdInterruptTransfer) X(sceUsbdControlTransfer)
#define DECLARE(name) static decltype(&name) p_##name;
USB_FUNCTIONS(DECLARE)
#undef DECLARE
static int load_module(const char *name) {
    const char *sandbox=sceKernelGetFsSandboxRandomWord();
    if(!sandbox || !sandbox[0]) { error("Sandbox unavailable",-1); return -1; }
    Text path={}; path.add("/"); path.add(sandbox); path.add("/common/lib/"); path.add(name);
    int result=0;
    int handle=(int32_t)sceKernelLoadStartModule(path.data,0,0,0,0,&result);
    if(handle<0) error(name,handle);
    return handle;
}
static bool load_usb() {
    int id=load_module("libSceSysmodule.sprx"); if(id<0) return false;
    decltype(&sceSysmoduleLoadModule) load=0;
    int r=sceKernelDlsym(id,"sceSysmoduleLoadModule",(void**)&load);
    if(r<0 || !load) { error("Resolve Sysmodule",r); return false; }
    r=load(ORBIS_SYSMODULE_USBD);
    if(r<0) { error("Load USB",r); return false; }
    id=load_module("libSceUsbd.sprx"); if(id<0) return false;
#define RESOLVE(name) r=sceKernelDlsym(id,#name,(void**)&p_##name); if(r<0 || !p_##name) { error(#name,r); return false; }
    USB_FUNCTIONS(RESOLVE)
#undef RESOLVE
    return true;
}
#include "integration.h"
struct libusb_endpoint_descriptor {
    uint8_t bLength,bDescriptorType,bEndpointAddress,bmAttributes;
    uint16_t wMaxPacketSize; uint8_t bInterval,bRefresh,bSynchAddress;
    const unsigned char *extra; int extra_length;
};
static_assert(sizeof(libusb_endpoint_descriptor)==32,"USB descriptor ABI");
struct Device { libusb_device_handle *handle; int iface,alt; uint8_t in,out,seq,in_size;uint16_t vid,pid;Driver driver;HidLayout hid; };
struct BootStats {unsigned candidates,opened,valid,invalid,timeouts,actions,control;int error,stage;};
static BootStats boot_stats[2]={};
static bool find_device(Device &d,Driver wanted=NONE) {
    libusb_device **list=0;int count=p_sceUsbdGetDeviceList(&list);
    if(count<0){error("List USB",count);return false;}
    for(int i=0;i<count && !d.handle;++i) {
        libusb_device_descriptor desc={};
        if(p_sceUsbdGetDeviceDescriptor(list[i],&desc)<0)continue;
        libusb_config_descriptor *cfg=0;
        if(p_sceUsbdGetActiveConfigDescriptor(list[i],&cfg)<0 || !cfg)continue;
        for(int j=0;j<cfg->bNumInterfaces && !d.handle;++j) {
            const libusb_interface &it=cfg->interface[j];
            for(int k=0;k<it.num_altsetting && !d.handle;++k) {
                const libusb_interface_descriptor &a=it.altsetting[k];
                Driver driver=identify_driver(desc.idVendor,desc.idProduct,a.bInterfaceClass,a.bInterfaceSubClass,a.bInterfaceProtocol,a.bInterfaceNumber);
                if(wanted!=NONE) {
                    driver=NONE;
                    if(a.bInterfaceClass==3 && a.bInterfaceSubClass==1) {
                        if(a.bInterfaceProtocol==1)driver=BOOT_KEYBOARD;
                        if(a.bInterfaceProtocol==2)driver=BOOT_MOUSE;
                    }
                    if(driver!=wanted)continue;
                }
                if(driver==NONE || a.bAlternateSetting)continue;
                BootStats *stats=wanted==NONE?0:&boot_stats[wanted==BOOT_MOUSE?1:0];
                if(stats){++stats->candidates;stats->stage=1;}
                uint8_t in=0,out=0,in_size=0;
                for(int e=0;e<a.bNumEndpoints;++e) {
                    const libusb_endpoint_descriptor &ep=a.endpoint[e];
                    if((ep.bmAttributes&3)!=3 || !ep.wMaxPacketSize || ep.wMaxPacketSize>64)continue;
                    if(ep.bEndpointAddress&0x80){in=ep.bEndpointAddress;in_size=(uint8_t)ep.wMaxPacketSize;}else out=ep.bEndpointAddress;
                }
                if(!in || ((driver==SWITCH_PRO || driver==XBOXONE) && !out))continue;
                libusb_device_handle *handle=0;
                int opened=p_sceUsbdOpen(list[i],&handle);
                if(opened<0 || !handle){if(stats)stats->error=opened<0?opened:-1;continue;}
                if(stats)++stats->opened;
                HidLayout layout={};
                if(driver==GENERIC_HID) {
                    uint8_t report[1024]={};
                    int length=p_sceUsbdControlTransfer(handle,0x81,6,0x2200,a.bInterfaceNumber,report,sizeof(report),500);
                    if(length<=0 || length>=(int)sizeof(report) || !parse_hid(report,(size_t)length,layout)){p_sceUsbdClose(handle);continue;}
                }
                d.handle=handle;d.iface=a.bInterfaceNumber;d.alt=0;d.in=in;d.out=out;d.in_size=in_size;
                d.vid=desc.idVendor;d.pid=desc.idProduct;d.driver=driver;d.hid=layout;
                Text t={};t.add("ProCon 0.3: ");t.add(driver_name(driver));t.add(" VID=");t.hex(d.vid);t.add(" PID=");t.hex(d.pid);notify(t.data);
            }
        }
        p_sceUsbdFreeConfigDescriptor(cfg);
    }
    if(list)p_sceUsbdFreeDeviceList(list);
    return d.handle!=0;
}
static int write_usb(void *opaque,uint8_t *p,int n) {
    Device &d=*(Device*)opaque; if(!d.out)return -1002;int actual=0;
    int r=p_sceUsbdInterruptTransfer(d.handle,d.out,p,n,&actual,500);
    return r ? r : actual==n ? 0 : -1001;
}
static int read_usb(void *opaque,uint8_t *p,int n,int *actual) {
    Device &d=*(Device*)opaque; return p_sceUsbdInterruptTransfer(d.handle,d.in,p,n,actual,100);
}
static uint64_t now_usb(void*) { return sceKernelGetProcessTime(); }
static bool command(Device &d,uint8_t cmd,bool ack) {
    UsbIo io={&d,write_usb,read_usb,now_usb}; UsbCommandResult r=run_usb_command(io,cmd,ack);
    if(r.code) { Text t={};t.add("ProCon: Command ");t.hex(cmd);t.add(" error ");t.hex((uint32_t)r.code);notify(t.data); }
    return r.code==0;
}
static bool start_xbox(Device &d) {
    uint8_t power[]={5,0x20,++d.seq,1,0};int r=write_usb(&d,power,sizeof(power));if(r)return false;
    if(d.pid==0x02ea || d.pid==0x0b00){uint8_t mode[]={5,0x20,++d.seq,0x0f,6};r=write_usb(&d,mode,sizeof(mode));if(r)return false;}
    uint8_t led[]={0x0a,0x20,++d.seq,3,0,1,0x14};r=write_usb(&d,led,sizeof(led));if(r)return false;
    uint8_t auth[]={6,0x20,++d.seq,2,1,0};return write_usb(&d,auth,sizeof(auth))==0;
}
static bool start(Device &d) {
    if(d.driver==XBOXONE)return start_xbox(d);
    if(d.driver==GENERIC_HID || d.driver==DUALSENSE) {
        // SET_IDLE is optional on some controllers.
        p_sceUsbdControlTransfer(d.handle,0x21,0x0a,(4<<8)|d.hid.id,d.iface,0,0,500);
        return true;
    }
    if(d.driver==XBOX360)return true;
    if(!command(d,2,true)) return false;
    command(d,3,true);
    if(!command(d,2,true) || !command(d,4,false)) return false;
    uint8_t p[64]={1,0,0,1,0x40,0x40,0,1,0x40,0x40,3,0x30};
    int r=write_usb(&d,p,sizeof(p));if(r) error("Enable packets",r);return r==0;
}
#include "keyboard_mouse_usb.h"
static void *test_worker(void*) {
    sceKernelUsleep(5000000);
    if(!install_integration()) {notify_status(false);return 0;}
    notify("ProCon: Integration active. Keep the DualShock connected.");
    notify("ProCon: Starting USB.");
    if(!load_usb()) return 0;
    int r=p_sceUsbdInit(); if(r<0) { error("Initialize USB",r);return 0; }
    bool kbm_started=scePthreadCreate(&kbm_thread,0,keyboard_mouse_worker,0,"procon-kbm")==0;
    if(!kbm_started)notify("ProCon: Could not start keyboard/mouse reader.");
    bool waiting=false;
    while(!__atomic_load_n(&disabled,__ATOMIC_ACQUIRE)) {
    Device d={};
    if(!find_device(d)) {
        if(!waiting) {notify("ProCon: Controller reader waiting. Keyboard/mouse reader is separate.");waiting=true;}
        sceKernelUsleep(2000000);continue;
    }
    int claim=p_sceUsbdClaimInterface(d.handle,d.iface);
    bool ready=claim==0;
    if(!ready) error("Access interface",claim);
    if(ready && d.alt) { r=p_sceUsbdSetInterfaceAltSetting(d.handle,d.iface,d.alt);ready=r==0;if(!ready) error("Select interface",r); }
    if(ready) ready=start(d);
    unsigned reports=0; int read_error=0;Mapped last_input=neutral_pad();
    if(ready) {
        uint64_t last=sceKernelGetProcessTime(),began=last;bool status_shown=false;
        while(!__atomic_load_n(&disabled,__ATOMIC_ACQUIRE)) {
            uint8_t packet[64]={};int n=0;r=read_usb(&d,packet,sizeof(packet),&n);
            uint64_t now=sceKernelGetProcessTime();
            if(usb_timeout(r)) {if(d.driver==GENERIC_HID && reports){publish(current_input,last_input,now);continue;}if(now-last>3000000) break;continue;}
            if(r) {read_error=r;break;}
            if(n<0 || n>64){read_error=-1003;break;}
            if(d.driver==XBOXONE && n>=4) {
                if(packet[0]==2){if(!start_xbox(d)){read_error=-1004;break;}continue;}
                if(packet[0]==7 && packet[1]==0x30 && n>=6) {
                    uint8_t ack[]={1,0x20,packet[2],9,0,7,0x20,2,0,0,0,0,0};
                    int ar=write_usb(&d,ack,sizeof(ack));if(ar){read_error=ar;break;}continue;
                }
            }
            Mapped mapped={};
            bool decoded=d.driver==GENERIC_HID?decode_hid(d.hid,packet,(size_t)n,mapped):decode_controller(d.driver,packet,(size_t)n,mapped);
            if(!decoded){if(now-last>3000000)break;continue;}
            last=now;last_input=mapped;publish(current_input,mapped,now);
            if(++reports==1) {notify_status(true);waiting=false;}
            if(!status_shown && now-began>10000000) {
                Text t={};t.add("ProCon: Reads delivered to the game=");t.number(__atomic_load_n(&delivered,__ATOMIC_RELAXED));notify(t.data);status_shown=true;
            }
        }
    }
    clear_input();
    if(claim==0) { if(d.driver==SWITCH_PRO)command(d,5,false);p_sceUsbdReleaseInterface(d.handle,d.iface); }
    p_sceUsbdClose(d.handle);
    if(read_error) error("USB read interrupted",read_error);
    if(!ready) {notify("ProCon: USB not initialized. DualShock kept connected; reopen the game to try again.");break;}
    if(!__atomic_load_n(&disabled,__ATOMIC_ACQUIRE)) {notify("ProCon: No USB data. Waiting for reconnection.");sceKernelUsleep(2000000);}
    }
    __atomic_store_n(&kbm_stop,1,__ATOMIC_RELEASE);
    if(kbm_started)while(!__atomic_load_n(&kbm_done,__ATOMIC_ACQUIRE))sceKernelUsleep(10000);
    clear_input();p_sceUsbdExit();
    if(__atomic_load_n(&disabled,__ATOMIC_ACQUIRE)) notify("ProCon disabled by shortcut. Reopen the game to reactivate it.");
    return 0;
}
static OrbisPthread worker;
static bool resident=false;
extern "C" int module_start(size_t,const void*) {return 0;}
extern "C" int module_stop(size_t,const void*) {return resident ? -1 : 0;}
extern "C" int plugin_load(int,const char**) {
    if(resident) return 0;
    notify("ProCon 0.3 P0: Loaded. Experimental controller and keyboard/mouse support.");
    resident=true;
    int r=scePthreadCreate(&worker,0,test_worker,0,"procon-usb-test");
    if(r) {error("Create USB reader",r);return 0;}
    resident=true;return 0;
}
// Worker and hooks must remain resident until process exit.
extern "C" int plugin_unload(int,const char**) {return resident ? -1 : 0;}
