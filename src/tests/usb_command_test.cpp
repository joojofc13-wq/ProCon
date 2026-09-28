#include <assert.h>
#include <stdio.h>
#include "../src/usb_command.h"
struct Fake { uint64_t time; int reads,writes,mode; };
static int write_packet(void *ctx,uint8_t *p,int n) {
    Fake &f=*(Fake *)ctx; ++f.writes;
    assert(n==64 && p[0]==0x80 && p[1]==2);
    for (int i=2;i<n;++i) assert(p[i]==0);
    return f.mode==3 ? (int32_t)0x80240004u : 0;
}
static int read_packet(void *ctx,uint8_t *p,int,int *n) {
    Fake &f=*(Fake *)ctx; ++f.reads; f.time+=100000;
    *n=0;
    if (f.mode==2) return (int32_t)0x80240004u;
    if (f.mode==1 || f.reads==1) return (int32_t)0x80240007u;
    if (f.reads==2) return -7;
    *n=2; p[0]=0x81; p[1]=f.reads==3 ? 1 : 2;
    return 0;
}
static uint64_t now(void *ctx) { return ((Fake *)ctx)->time; }
int main() {
    Fake f={}; UsbIo io={&f,write_packet,read_packet,now};
    auto r=run_usb_command(io,2,true);
    assert(r.code==0 && f.reads==4 && f.writes==1);
    f={}; f.mode=1; r=run_usb_command(io,2,true);
    assert(usb_timeout(r.code) && f.reads==60 && f.writes==3 && f.time==6000000);
    f={}; f.mode=2; r=run_usb_command(io,2,true);
    assert((uint32_t)r.code==0x80240004u && f.reads==1 && f.writes==1);
    f={}; f.mode=3; r=run_usb_command(io,2,true);
    assert(r.code!=0 && f.reads==0 && f.writes==1);
    f={}; r=run_usb_command(io,2,false);
    assert(r.code==0 && f.reads==0 && f.writes==1);
    puts("USB transport: Orbis/libusb timeout recovery, ACK filtering, bounded retries and 64-byte packets passed");
}
