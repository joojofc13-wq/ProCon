#include <assert.h>
#include <stdio.h>
#include <thread>
#include <atomic>
#include "../src/snapshot.h"
#include "../src/merge.h"
struct Pad {uint32_t buttons;struct{uint8_t x,y;}leftStick,rightStick;struct{uint8_t l2,r2;}analogButtons;uint64_t sentinel;};
int main() {
    const uint32_t expected[24]={0x8000,0x1000,0x4000,0x2000,0,0,0x800,0x200,0x100000,8,4,2,0,0x100000,0,0,0x40,0x10,0x20,0x80,0,0,0x400,0x100};
    ProReport r={0,2048,2048,2048,2048};
    for(unsigned i=0;i<24;++i) {r.buttons=1u<<i;assert(map_report(r).buttons==expected[i]);}
    assert(axis(0)==0 && axis(4095)==255 && axis(0,true)==255 && axis(4095,true)==0);
    for(int i=1;i<4096;++i) assert(axis(i)>=axis(i-1));
    r.buttons=0;Mapped neutral=map_report(r);
    Pad p={8,{10,20},{30,40},{50,60},0x123456789abcdefULL};
    merge_mapped(p,neutral);assert(p.buttons==8 && p.leftStick.x==10 && p.rightStick.y==40 && p.analogButtons.l2==50);
    r.buttons=(1<<2)|(1<<23);r.lx=4095;r.ly=4095;
    Mapped m=map_report(r);merge_mapped(p,m);
    assert(p.buttons==(8|0x4000|0x100) && p.leftStick.x==255 && p.leftStick.y==0 && p.analogButtons.l2==255 && p.sentinel==0x123456789abcdefULL);
    Snapshot s={};Mapped got={};assert(!read_snapshot(s,got,1000));publish(s,m,1000);
    assert(read_snapshot(s,got,251000) && got.buttons==m.buttons && got.lx==m.lx);
    assert(!read_snapshot(s,got,251001) && !read_snapshot(s,got,999));
    publish(s,neutral,0);assert(!read_snapshot(s,got,1001));
    std::atomic<bool> done=false;
    std::thread producer([&]{for(unsigned i=1;i<=100000;++i){uint8_t x=(uint8_t)i;Mapped v={(uint32_t)x,x,(uint8_t)(x^0x55),(uint8_t)(x^0xaa),(uint8_t)(255-x),0,0};publish(s,v,1000);}done=true;});
    unsigned observed=0;
    do {if(read_snapshot(s,got,1001)){assert(got.buttons==got.lx && got.ly==(uint8_t)(got.lx^0x55) && got.rx==(uint8_t)(got.lx^0xaa) && got.ry==(uint8_t)(255-got.lx));++observed;}}while(!done);
    producer.join();assert(read_snapshot(s,got,1001));
    printf("PASS: all buttons, axis bounds, DS4 merge, stale input, concurrent snapshots (%u observed)\n",observed);
}
