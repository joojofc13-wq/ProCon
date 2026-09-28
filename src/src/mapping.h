#pragma once
#include "report.h"
struct Mapped { uint32_t buttons; uint8_t lx,ly,rx,ry,l2,r2; };
static inline uint8_t axis(uint16_t raw,bool invert=false) {
    int d=(int)raw-2048;
    if(invert) d=-d;
    int magnitude=d<0 ? -d : d;
    if(magnitude<=180) return 128;
    if(magnitude>=1500) return d<0 ? 0 : 255;
    int scaled=(magnitude-180)*(d<0 ? 128 : 127)/1320;
    return (uint8_t)(128+(d<0 ? -scaled : scaled));
}
static inline Mapped map_report(const ProReport &r) {
    static const uint32_t bits[24]={
        0x8000,0x1000,0x4000,0x2000,0,0,0x800,0x200,
        0x100000,8,4,2,0,0x100000,0,0,
        0x40,0x10,0x20,0x80,0,0,0x400,0x100};
    Mapped m={};
    for(unsigned i=0;i<24;++i) if(r.buttons&(1u<<i)) m.buttons|=bits[i];
    m.lx=axis(r.lx); m.ly=axis(r.ly,true);
    m.rx=axis(r.rx); m.ry=axis(r.ry,true);
    m.l2=(m.buttons&0x100) ? 255 : 0;
    m.r2=(m.buttons&0x200) ? 255 : 0;
    return m;
}
static inline bool fresh_sample(uint64_t now,uint64_t at) {
    return at && now>=at && now-at<=250000;
}
