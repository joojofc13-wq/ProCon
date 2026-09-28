#pragma once
#include "controllers.h"
#include "generic_hid.h"
struct BootKeyboard {uint8_t modifiers,keys[6];};
struct BootMouse {uint8_t buttons;int16_t x,y;};
static inline bool decode_keyboard(const uint8_t *p,size_t n,BootKeyboard &out) {
    if(!p || n<8)return false;
    BootKeyboard k={};k.modifiers=p[0];
    for(unsigned i=0;i<6;++i){if(p[i+2]>=1 && p[i+2]<=3){out={};return false;}k.keys[i]=p[i+2];}
    out=k;return true;
}
static inline bool decode_mouse(const uint8_t *p,size_t n,BootMouse &out) {
    if(!p || n<3)return false;
    out={uint8_t(p[0]&7),(int8_t)p[1],(int8_t)p[2]};return true;
}
static inline bool decode_mouse_report(const HidLayout &h,const uint8_t *p,size_t n,BootMouse &out) {
    if(!p || !h.count)return false;
    if(h.ids){if(!n || *p!=h.id)return false;++p;--n;}
    if(n*8<h.bits)return false;
    BootMouse m={};
    for(unsigned i=0;i<h.count;++i) {
        const HidField &f=h.fields[i];uint32_t raw=0;
        for(unsigned b=0;b<f.size;++b)raw|=((p[(f.bit+b)/8]>>((f.bit+b)%8))&1u)<<b;
        int32_t value=(int32_t)raw;
        if(f.lo<0 && (raw&(1u<<(f.size-1))))value=(int32_t)(raw|(~0u<<f.size));
        if(f.page==9 && value)m.buttons|=1u<<(f.usage-1);
        if(f.page==1 && f.usage==0x30)m.x=(int16_t)value;
        if(f.page==1 && f.usage==0x31)m.y=(int16_t)value;
    }
    out=m;return true;
}
static inline bool decode_mouse_compatible(const HidLayout &layout,bool described,const uint8_t *p,size_t n,BootMouse &out) {
    if(described)return decode_mouse_report(layout,p,n,out);
    return decode_mouse(p,n,out);
}
static inline bool key_down(const BootKeyboard &k,uint8_t code) {
    for(unsigned i=0;i<6;++i)if(k.keys[i]==code)return true;
    return false;
}
static inline Mapped map_keyboard(const BootKeyboard &k) {
    Mapped m=neutral_pad();
    int x=(int)key_down(k,7)-(int)key_down(k,4);
    int y=(int)key_down(k,22)-(int)key_down(k,26);
    m.lx=x<0?0:x>0?255:128;m.ly=y<0?0:y>0?255:128;
    const uint8_t keys[]={44,8,20,40,41,82,81,80,79,21,9};
    const uint32_t buttons[]={0x4000,0x8000,0x1000,8,0x100000,0x10,0x40,0x80,0x20,0x400,0x800};
    for(unsigned i=0;i<sizeof(keys);++i)if(key_down(k,keys[i]))m.buttons|=buttons[i];
    if(k.modifiers&0x11)m.buttons|=0x2000;
    if(k.modifiers&0x22)m.buttons|=2;
    return m;
}
struct MouseWindow {int x,y;uint8_t buttons;uint64_t since;Mapped last;};
static inline int bounded_delta(int x) {return x<-512?-512:x>512?512:x;}
static inline void add_mouse(MouseWindow &w,const BootMouse &m) {
    w.x=bounded_delta(w.x+m.x);w.y=bounded_delta(w.y+m.y);w.buttons=m.buttons;
}
static inline uint8_t mouse_axis(int delta) {
    if(!delta)return 128;
    int magnitude=delta<0?-delta:delta;
    int offset=32+magnitude*4;
    int v=128+(delta<0?-offset:offset);
    return (uint8_t)(v<0?0:v>255?255:v);
}
static inline Mapped sample_mouse(MouseWindow &w,uint64_t now) {
    if(!w.since || now<w.since){w.since=now;w.last=neutral_pad();}
    if(now-w.since>=16000) {
        w.last.rx=mouse_axis(w.x);w.last.ry=mouse_axis(w.y);w.x=w.y=0;w.since=now;
    }
    w.last.buttons=0;w.last.l2=(w.buttons&2)?255:0;w.last.r2=(w.buttons&1)?255:0;
    if(w.buttons&4)w.last.buttons|=4;
    triggers(w.last);return w.last;
}
