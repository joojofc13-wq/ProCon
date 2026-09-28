#pragma once
#include <stdint.h>
#include <stddef.h>

static inline bool usb_timeout(int r) { return r==-7 || (uint32_t)r==0x80240007u; }
struct UsbIo {
    void *context;
    int (*write)(void *,uint8_t *,int);
    int (*read)(void *,uint8_t *,int,int *);
    uint64_t (*now)(void *);
};
struct UsbCommandResult { int code; unsigned attempts; unsigned reads; };
static UsbCommandResult run_usb_command(const UsbIo &io,uint8_t cmd,bool ack) {
    UsbCommandResult result={0,0,0};
    uint8_t output[64]={0x80,cmd};
    for (unsigned attempt=0;attempt<(ack ? 3u : 1u);++attempt) {
        ++result.attempts;
        result.code=io.write(io.context,output,sizeof(output));
        if (result.code!=0 || !ack) return result;
        uint64_t end=io.now(io.context)+2000000ULL;
        for (unsigned poll=0;poll<40 && io.now(io.context)<end;++poll) {
            uint8_t input[64]={};
            int n=0;
            int r=io.read(io.context,input,sizeof(input),&n);
            ++result.reads;
            if (r!=0 && !usb_timeout(r)) { result.code=r; return result; }
            if (n>=2 && n<=(int)sizeof(input) && input[0]==0x81 && input[1]==cmd) {
                result.code=0; return result;
            }
        }
    }
    result.code=(int32_t)0x80240007u;
    return result;
}
