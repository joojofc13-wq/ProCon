#pragma once
#include "controllers.h"
// One report ID per layout.
struct HidField {uint16_t bit,page,usage;uint8_t size;int32_t lo,hi;};
struct HidLayout {HidField fields[48];unsigned count,bits;uint8_t id;bool ids;};
struct HidGlobal {uint32_t page,size,count,id;int32_t lo,hi;};
static inline int32_t hid_signed(uint32_t v,unsigned bytes) {
    if(bytes && bytes<4 && (v&(1u<<(bytes*8-1))))v|=~((1u<<(bytes*8))-1);
    return (int32_t)v;
}
static inline bool parse_hid(const uint8_t *p,size_t n,HidLayout &out,bool mouse=false) {
    if(!p || !n || n>1024)return false;
    HidLayout result={};HidGlobal g={},stack[4]={};unsigned sp=0;
    uint16_t offsets[256]={};uint32_t usages[48]={},umin=0,umax=0;unsigned nu=0;
    bool range=false,hasids=false,inpad=false,parents[16]={};unsigned depth=0;
    for(size_t pos=0;pos<n;) {
        uint8_t prefix=p[pos++];if(prefix==0xfe)return false;
        unsigned bytes=prefix&3;if(bytes==3)bytes=4;if(pos+bytes>n)return false;
        uint32_t v=0;for(unsigned i=0;i<bytes;++i)v|=(uint32_t)p[pos++]<<(i*8);
        unsigned type=(prefix>>2)&3,tag=prefix>>4;
        if(type==1) {
            switch(tag) {
                case 0:g.page=v;break;case 1:g.lo=hid_signed(v,bytes);break;
                case 2:g.hi=g.lo<0?hid_signed(v,bytes):(int32_t)v;break;
                case 7:g.size=v;break;
                case 8:if(!v || v>255)return false;g.id=v;hasids=true;break;
                case 9:g.count=v;break;
                case 10:if(sp==4)return false;stack[sp++]=g;break;
                case 11:if(!sp)return false;g=stack[--sp];break;
                default:break;
            }
        } else if(type==2) {
            uint32_t usage=bytes==4?v:(g.page<<16)|(v&0xffff);
            if(tag==0){if(nu==48)return false;usages[nu++]=usage;}
            else if(tag==1){umin=usage;range=true;}
            else if(tag==2)umax=usage;
            else if(tag==10)return false; // HID delimiters are unsupported.
        } else if(type==0) {
            if(tag==10) {
                if(depth==16)return false;parents[depth++]=inpad;
                uint32_t u=nu?usages[0]:umin;
                if(v==1)inpad=mouse?(u==0x10002):(u==0x10004 || u==0x10005);
            } else if(tag==12) {if(!depth)return false;inpad=parents[--depth];}
            else if(tag==8) {
                if(!g.size || g.size>32 || g.count>512 || g.size*g.count>512 || offsets[g.id]+g.size*g.count>512)return false;
                unsigned start=offsets[g.id];offsets[g.id]+=g.size*g.count;
                if(inpad && !(v&1)) {
                    if(!(v&2) || (!mouse && (v&4)) || g.lo>=g.hi)return false;
                    for(unsigned k=0;k<g.count;++k) {
                        uint32_t u=k<nu?usages[k]:range && umax>=umin && k<=umax-umin?umin+k:0;
                        unsigned page=u>>16,usage=u&0xffff;
                        bool supported=(page==9 && usage>=1 && usage<=12) || (page==1 && usage>=0x30 && usage<=0x35) || (page==1 && usage==0x39) || (page==2 && (usage==0xc4 || usage==0xc5));
                        if(mouse)supported=(page==9 && usage>=1 && usage<=3) || (page==1 && (usage==0x30 || usage==0x31));
                        if(!supported)continue;
                        if(mouse && page==1 && (!(v&4) || g.lo>=0))return false;
                        if(result.count && result.id!=g.id)return false;
                        if(result.count==48 || g.size>16)return false;
                        result.id=(uint8_t)g.id;
                        result.fields[result.count++]={(uint16_t)(start+k*g.size),(uint16_t)page,(uint16_t)usage,(uint8_t)g.size,g.lo,g.hi};
                    }
                }
            }
            nu=0;range=false;umin=umax=0;
        }
    }
    if(depth || sp || !result.count || (hasids && !result.id))return false;
    result.ids=hasids;result.bits=offsets[result.id];
    if(result.bits+(hasids?8:0)>512)return false;
    bool x=false,y=false;for(unsigned i=0;i<result.count;++i){x|=result.fields[i].page==1 && result.fields[i].usage==0x30;y|=result.fields[i].page==1 && result.fields[i].usage==0x31;}
    if(!x || !y)return false;
    out=result;return true;
}
static inline bool decode_hid(const HidLayout &h,const uint8_t *p,size_t n,Mapped &out) {
    if(!p || !h.count)return false;
    if(h.ids){if(!n || *p!=h.id)return false;++p;--n;}
    if(n*8<h.bits)return false;
    Mapped m=neutral_pad();bool rx=false,ry=false;
    for(unsigned i=0;i<h.count;++i){rx|=h.fields[i].page==1 && h.fields[i].usage==0x33;ry|=h.fields[i].page==1 && h.fields[i].usage==0x34;}
    const uint32_t buttons[]={0x4000,0x2000,0x8000,0x1000,0x400,0x800,0x100,0x200,0x100000,8,2,4};
    for(unsigned i=0;i<h.count;++i) {
        const HidField &f=h.fields[i];uint32_t raw=0;
        for(unsigned b=0;b<f.size;++b)raw|=((p[(f.bit+b)/8]>>((f.bit+b)%8))&1u)<<b;
        int32_t v=(int32_t)raw;if(f.lo<0 && (raw&(1u<<(f.size-1))))v=(int32_t)(raw|(~0u<<f.size));
        if(f.page==9){if(v)m.buttons|=buttons[f.usage-1];continue;}
        if(f.page==1 && f.usage==0x39){if(v>=f.lo && v<=f.hi)hat(m,v-f.lo);continue;}
        if(v<f.lo)v=f.lo;if(v>f.hi)v=f.hi;
        uint8_t a=(uint8_t)(((int64_t)v-f.lo)*255/((int64_t)f.hi-f.lo));
        if(f.page==2){if(f.usage==0xc5)m.l2=a;else m.r2=a;continue;}
        switch(f.usage) {
            case 0x30:m.lx=center_axis(a);break;case 0x31:m.ly=center_axis(a);break;
            case 0x33:m.rx=center_axis(a);break;case 0x34:m.ry=center_axis(a);break;
            case 0x32:if(rx && ry)m.l2=a;else m.rx=center_axis(a);break;
            case 0x35:if(rx && ry)m.r2=a;else m.ry=center_axis(a);break;
        }
    }
    if((m.buttons&0x100) && !m.l2)m.l2=255;
    if((m.buttons&0x200) && !m.r2)m.r2=255;
    triggers(m);out=m;return true;
}
