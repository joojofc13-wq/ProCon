#pragma once
#include "mapping.h"
enum Driver { NONE, SWITCH_PRO, DUALSENSE, XBOX360, XBOXONE, GENERIC_HID, BOOT_KEYBOARD, BOOT_MOUSE };
static inline Mapped neutral_pad() {return {0,128,128,128,128,0,0};}
static inline unsigned u16le(const uint8_t *p) {return p[0]|(unsigned)p[1]<<8;}
static inline void hat(Mapped &m,int v) {
    const uint32_t masks[]={0x10,0x30,0x20,0x60,0x40,0xc0,0x80,0x90};
    if(v>=0 && v<8) m.buttons|=masks[v];
}
static inline uint8_t center_axis(uint8_t v) {return v>=120 && v<=136 ? 128 : v;}
static inline uint8_t xbox_axis(const uint8_t *p,bool invert=false) {
    int v=(int16_t)u16le(p);if(invert)v=-1-v;
    return center_axis((uint8_t)((v+32768)>>8));
}
static inline void triggers(Mapped &m) {
    if(m.l2>30)m.buttons|=0x100;
    if(m.r2>30)m.buttons|=0x200;
}
static inline bool decode_controller(Driver driver,const uint8_t *p,size_t n,Mapped &out) {
    if(!p)return false;
    Mapped m=neutral_pad();
    if(driver==SWITCH_PRO) {ProReport r={};if(!decode_report(p,n,&r))return false;out=map_report(r);return true;}
    if(driver==DUALSENSE) {
        if(n!=64 || p[0]!=1)return false; // USB full report, not Bluetooth/simple.
        m.lx=center_axis(p[1]);m.ly=center_axis(p[2]);m.rx=center_axis(p[3]);m.ry=center_axis(p[4]);
        m.l2=p[5];m.r2=p[6];hat(m,p[8]&15);
        const uint32_t face[]={0x8000,0x4000,0x2000,0x1000};
        for(unsigned b=0;b<4;++b)if(p[8]&(0x10<<b))m.buttons|=face[b];
        const uint32_t buttons[]={0x400,0x800,0x100,0x200,0x100000,8,2,4};
        for(unsigned b=0;b<8;++b)if(p[9]&(1<<b))m.buttons|=buttons[b];
        if(p[10]&2)m.buttons|=0x100000; // Touchpad click; PS stays on DS4.
    } else if(driver==XBOX360) {
        if(n<20 || p[0]!=0 || p[1]!=20)return false;
        const uint32_t a[]={0x10,0x40,0x80,0x20,8,0x100000,2,4};
        const uint32_t b[]={0x400,0x800,0,0,0x4000,0x2000,0x8000,0x1000};
        for(unsigned k=0;k<8;++k){if(p[2]&(1<<k))m.buttons|=a[k];if(p[3]&(1<<k))m.buttons|=b[k];}
        m.l2=p[4];m.r2=p[5];m.lx=xbox_axis(p+6);m.ly=xbox_axis(p+8,true);m.rx=xbox_axis(p+10);m.ry=xbox_axis(p+12,true);
    } else if(driver==XBOXONE) {
        if(n<18 || p[0]!=0x20 || (p[1]&0xc0) || p[3]<14 || (size_t)p[3]+4>n)return false;
        const uint32_t a[]={0,0,8,0x100000,0x4000,0x2000,0x8000,0x1000};
        const uint32_t b[]={0x10,0x40,0x80,0x20,0x400,0x800,2,4};
        for(unsigned k=0;k<8;++k){if(p[4]&(1<<k))m.buttons|=a[k];if(p[5]&(1<<k))m.buttons|=b[k];}
        unsigned l=u16le(p+6),r=u16le(p+8);m.l2=(l>1023?1023:l)*255/1023;m.r2=(r>1023?1023:r)*255/1023;
        m.lx=xbox_axis(p+10);m.ly=xbox_axis(p+12,true);m.rx=xbox_axis(p+14);m.ry=xbox_axis(p+16,true);
    } else return false;
    triggers(m);out=m;return true;
}
static inline Driver identify_driver(uint16_t vid,uint16_t pid,uint8_t cls,uint8_t sub,uint8_t proto,uint8_t iface) {
    if(vid==0x057e && pid==0x2009)return cls==3?SWITCH_PRO:NONE;
    if(vid==0x054c) {
        if(pid==0x0ce6 || pid==0x0df2)return cls==3?DUALSENSE:NONE;
        return NONE; // Reserved for the native Sony driver.
    }
    if(vid==0x045e && cls==0xff && iface==0) {
        if(sub==0x5d && proto==1 && pid==0x028e)return XBOX360;
        if(sub==0x47 && proto==0xd0 && (pid==0x02d1 || pid==0x02dd || pid==0x02e3 || pid==0x02ea || pid==0x0b00 || pid==0x0b12))return XBOXONE;
    }
    return cls==3 && sub==0 && proto==0?GENERIC_HID:NONE;
}
static inline const char *driver_name(Driver d) {
    switch(d){case BOOT_KEYBOARD:return "USB keyboard (boot)";case BOOT_MOUSE:return "USB mouse (boot)";case SWITCH_PRO:return "Nintendo Pro Controller";case DUALSENSE:return "DualSense";case XBOX360:return "Xbox 360 wired";case XBOXONE:return "Xbox One/Series USB";case GENERIC_HID:return "Generic HID (experimental)";default:return "Unsupported";}
}
