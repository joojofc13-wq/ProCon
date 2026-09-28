#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/generic_hid.h"
static const uint8_t descriptor[]={
    5,1,9,5,0xa1,1, // gamepad application
    0x85,3, // report ID
    0x15,0,0x26,0xff,0,0x75,8,0x95,4,
    9,0x30,9,0x31,9,0x32,9,0x35,0x81,2, // X Y Z Rz
    0x15,0,0x25,7,0x75,4,0x95,1,9,0x39,0x81,0x42,
    0x75,4,0x95,1,0x81,3, // constant padding
    5,9,0x19,1,0x29,12,0x15,0,0x25,1,0x75,1,0x95,12,0x81,2,
    0x75,4,0x95,1,0x81,3,0xc0
};
int main() {
    assert(identify_driver(0x054c,0x09cc,3,0,0,0)==NONE);
    assert(identify_driver(0x054c,0x0ce6,3,0,0,3)==DUALSENSE);
    assert(identify_driver(0x045e,0x028e,0xff,0x5d,1,0)==XBOX360);
    assert(identify_driver(0x045e,0x0719,0xff,0x5d,0x81,0)==NONE);
    assert(identify_driver(0x045e,0x0b12,0xff,0x47,0xd0,0)==XBOXONE);
    assert(identify_driver(0x045e,0x0b12,0xff,0x47,0xd0,1)==NONE);
    assert(identify_driver(123,456,3,1,1,0)==NONE);
    assert(identify_driver(123,456,8,6,0x50,0)==NONE);
    uint8_t p[64]={};Mapped m={};
    p[0]=1;p[1]=p[2]=p[3]=p[4]=128;p[8]=8;
    assert(decode_controller(DUALSENSE,p,64,m) && !m.buttons && m.lx==128);
    p[8]=0x28;p[9]=0x21;p[5]=200;
    assert(decode_controller(DUALSENSE,p,64,m) && (m.buttons&0x4508)==0x4508 && m.l2==200);
    for(size_t n=0;n<64;++n)assert(!decode_controller(DUALSENSE,p,n,m));
    memset(p,0,sizeof(p));p[1]=20;p[2]=0x51;p[3]=0x90;p[4]=255;p[6]=0;p[7]=0x80;p[8]=0xff;p[9]=0x7f;
    assert(decode_controller(XBOX360,p,20,m) && (m.buttons&0x511a)==0x511a && m.lx==0 && m.ly==0 && m.l2==255);
    for(size_t n=0;n<20;++n)assert(!decode_controller(XBOX360,p,n,m));
    memset(p,0,sizeof(p));p[0]=0x20;p[3]=14;p[4]=0x18;p[5]=0x42;p[6]=0xff;p[7]=3;
    assert(decode_controller(XBOXONE,p,18,m) && (m.buttons&0x104142)==0x104142 && m.l2==255 && m.lx==128);
    for(size_t n=0;n<18;++n)assert(!decode_controller(XBOXONE,p,n,m));
    p[3]=60;assert(!decode_controller(XBOXONE,p,18,m));p[3]=14;p[1]=0x80;assert(!decode_controller(XBOXONE,p,18,m));
    HidLayout h={};assert(parse_hid(descriptor,sizeof(descriptor),h));
    const uint8_t input[]={3,128,128,255,0,8,1,0};
    assert(decode_hid(h,input,sizeof(input),m) && m.buttons==0x4000 && m.lx==128 && m.rx==255 && m.ry==0);
    for(size_t n=0;n<sizeof(input);++n)assert(!decode_hid(h,input,n,m));
    uint8_t other[8];memcpy(other,input,8);other[0]=4;assert(!decode_hid(h,other,8,m));
    other[0]=3;other[5]=1;assert(decode_hid(h,other,8,m) && (m.buttons&0x30)==0x30);
    other[5]=15;assert(decode_hid(h,other,8,m) && !(m.buttons&0xf0));
    for(size_t n=0;n<sizeof(descriptor);++n)assert(!parse_hid(descriptor,n,h));
    const uint8_t signed_axes[]={5,1,9,4,0xa1,1,0x15,0x81,0x25,0x7f,0x75,8,0x95,2,9,0x30,9,0x31,0x81,2,0xc0};
    assert(parse_hid(signed_axes,sizeof(signed_axes),h));
    const uint8_t extremes[]={0x81,0x7f};assert(decode_hid(h,extremes,2,m) && m.lx==0 && m.ly==255);
    uint8_t bad[sizeof(descriptor)];memcpy(bad,descriptor,sizeof(bad));bad[3]=2;assert(!parse_hid(bad,sizeof(bad),h)); // mouse
    memcpy(bad,descriptor,sizeof(bad));bad[sizeof(bad)-1]=0xb4;assert(!parse_hid(bad,sizeof(bad),h)); // unmatched global pop
    uint32_t seed=1234567;unsigned accepted=0;
    for(unsigned k=0;k<30000;++k) {
        memcpy(bad,descriptor,sizeof(bad));seed=seed*1664525+1013904223;unsigned at=seed%sizeof(bad);
        seed=seed*1664525+1013904223;bad[at]=(uint8_t)(seed>>24);
        struct Guard {uint64_t before;HidLayout layout;uint64_t after;} guard={0x1122334455667788ULL,{},0x8877665544332211ULL};
        if(parse_hid(bad,sizeof(bad),guard.layout)) {
            ++accepted;assert(guard.layout.count<=48 && guard.layout.bits<=512);
            uint8_t sample[64]={};sample[0]=guard.layout.id;decode_hid(guard.layout,sample,sizeof(sample),m);
        }
        assert(guard.before==0x1122334455667788ULL && guard.after==0x8877665544332211ULL);
    }
    printf("PASS: USB identification, DS4 exclusion, DualSense/Xbox packets, HID IDs/signed axes/hats/truncation; 30000 descriptor mutations (%u valid)\n",accepted);
}
