#include <assert.h>
#include <stdio.h>
#include "../src/keyboard_mouse.h"
#include "../src/snapshot.h"
int main() {
    uint8_t report[8]={0,0,26,7,44,8,20,40};BootKeyboard k={};
    assert(decode_keyboard(report,8,k));Mapped m=map_keyboard(k);
    assert(m.lx==255 && m.ly==0 && (m.buttons&0xd008)==0xd008);
    report[0]=3;assert(decode_keyboard(report,8,k));m=map_keyboard(k);assert((m.buttons&0x2002)==0x2002);
    report[2]=4;report[3]=7;report[4]=26;report[5]=22;
    assert(decode_keyboard(report,8,k));m=map_keyboard(k);assert(m.lx==128 && m.ly==128);
    for(unsigned i=0;i<8;++i)report[i]=0;
    assert(decode_keyboard(report,8,k));m=map_keyboard(k);assert(!m.buttons && m.lx==128);
    report[2]=1;assert(!decode_keyboard(report,8,k) && !map_keyboard(k).buttons);
    for(unsigned n=0;n<8;++n)assert(!decode_keyboard(report,n,k));
    uint8_t mouse[]={3,5,0xfd};BootMouse b={};assert(decode_mouse(mouse,3,b) && b.x==5 && b.y==-3);
    MouseWindow w={};sample_mouse(w,1000);add_mouse(w,b);m=sample_mouse(w,17000);
    assert(m.rx==180 && m.ry==84 && m.l2==255 && m.r2==255 && (m.buttons&0x300)==0x300);
    m=sample_mouse(w,33000);assert(m.rx==128 && m.ry==128 && m.l2==255);
    mouse[0]=0;mouse[1]=mouse[2]=0;assert(decode_mouse(mouse,3,b));add_mouse(w,b);
    m=sample_mouse(w,34000);assert(!m.buttons && !m.l2 && !m.r2);
    assert(mouse_axis(0)==128 && mouse_axis(1)>160 && mouse_axis(-1)<96);
    for(int v=-511;v<=512;++v)assert(mouse_axis(v)>=mouse_axis(v-1));
    BootMouse fast={0,127,-127};for(unsigned i=0;i<100000;++i)add_mouse(w,fast);
    assert(w.x==512 && w.y==-512);m=sample_mouse(w,50000);assert(m.rx==255 && m.ry==0);
    m=sample_mouse(w,66000);assert(m.rx==128 && m.ry==128);
    assert(!decode_mouse(mouse,2,b));
    Snapshot keyboard={},pointer={};publish(keyboard,map_keyboard(k),1000);publish(pointer,m,1000);
    assert(read_snapshot(pointer,m,1001));publish(pointer,neutral_pad(),0);assert(!read_snapshot(pointer,m,1001));
    const uint8_t desc[]={0x05,1,0x09,2,0xa1,1,0x85,7,0x09,1,0xa1,0,
        0x05,9,0x19,1,0x29,3,0x15,0,0x25,1,0x75,1,0x95,3,0x81,2,
        0x75,5,0x95,1,0x81,1,0x05,1,0x09,0x30,0x09,0x31,
        0x15,0x81,0x25,0x7f,0x75,8,0x95,2,0x81,6,0xc0,0xc0};
    HidLayout layout={};assert(parse_hid(desc,sizeof(desc),layout,true));
    uint8_t horizontal[]={7,3,12,0};BootMouse parsed={};
    assert(decode_mouse_report(layout,horizontal,4,parsed));
    assert(parsed.buttons==3 && parsed.x==12 && parsed.y==0);
    MouseWindow held={};sample_mouse(held,1000);add_mouse(held,parsed);
    m=sample_mouse(held,17000);assert(m.rx>128 && m.ry==128);
    for(uint64_t t=33000;t<2000000;t+=16000) {
        m=sample_mouse(held,t);publish(pointer,m,t);assert(read_snapshot(pointer,m,t));
        assert(m.l2==255 && m.r2==255 && m.rx==128 && m.ry==128);
    }
    uint8_t vertical[]={7,3,0,0xf4};assert(decode_mouse_report(layout,vertical,4,parsed));
    assert(parsed.x==0 && parsed.y==-12);add_mouse(held,parsed);
    m=sample_mouse(held,2017000);assert(m.rx==128 && m.ry<128);
    uint8_t other[]={8,0,0,0};assert(!decode_mouse_report(layout,other,4,parsed));
    assert(!decode_mouse_report(layout,vertical,3,parsed));
    uint8_t release[]={7,0,0,0};assert(decode_mouse_report(layout,release,4,parsed));
    add_mouse(held,parsed);m=sample_mouse(held,2033000);assert(!m.l2 && !m.r2 && !m.buttons);
    for(size_t n=0;n<sizeof(desc);++n){HidLayout partial={};assert(!parse_hid(desc,n,partial,true));}
    assert(decode_mouse_compatible(layout,true,horizontal,4,parsed) && parsed.x==12 && parsed.y==0 && parsed.buttons==3);
    assert(!decode_mouse_compatible(layout,true,other,4,parsed));
    uint8_t padded[]={7,3,12,0,0,0,0,0};
    assert(decode_mouse_compatible(layout,true,padded,sizeof(padded),parsed) && parsed.x==12 && parsed.y==0 && parsed.buttons==3);
    assert(!decode_mouse_compatible(layout,true,padded,3,parsed));
    uint8_t boot[]={2,0,0};assert(decode_mouse_compatible(layout,false,boot,3,parsed) && parsed.buttons==2);
    boot[0]=0;assert(decode_mouse_compatible(layout,false,boot,3,parsed) && !parsed.buttons);
    puts("PASS: keyboard mapping, opposite directions, rollover, releases, mouse windows/buttons/clamps, stale/disconnected input");
}
