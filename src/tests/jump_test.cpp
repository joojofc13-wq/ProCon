#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <thread>
#include "../src/jump_plan.h"
static const uint8_t read_bytes[16]={0x31,0xc9,0xe9,0xe9,0xed,0xff,0xff,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};
static const uint8_t state_bytes[16]={0x31,0xd2,0xe9,0x49,0xec,0xff,0xff,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};
static void execute(const uint8_t *photo,uint8_t reg) {
    uint8_t *memory=(uint8_t*)VirtualAlloc(0,16384,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);assert(memory);
    uint8_t *entry=memory+8192,*bridge=memory+12288;
    memcpy(entry,photo,16);JumpPlan plan={};assert(!plan_jump(entry,(uintptr_t)entry,(uintptr_t)bridge,reg,plan));
    uint8_t original_worker[]={0x89,(uint8_t)(reg==0xc9?0xc8:0xd0),0xc3};
    uint8_t replacement[]={0x8d,(uint8_t)(reg==0xc9?0x41:0x42),9,0xc3};
    memcpy((void*)plan.worker,original_worker,sizeof(original_worker));memcpy(bridge,replacement,sizeof(replacement));
    using Fn=int(__attribute__((sysv_abi)) *)(int,int,int,int);
    Fn fn=(Fn)entry;assert(fn(11,22,33,44)==0);
    assert(replace_delta(entry+3,plan.old_delta,plan.new_delta));
    FlushInstructionCache(GetCurrentProcess(),memory,16384);
    assert(fn(11,22,33,44)==9);
    assert(!replace_delta(entry+3,plan.old_delta,plan.new_delta)); // stale expected value refused
    assert(replace_delta(entry+3,plan.new_delta,plan.old_delta));
    assert(!memcmp(entry,photo,16));assert(fn(11,22,33,44)==0);
    std::atomic<bool> stop=false;std::atomic<unsigned> calls=0;
    std::thread running([&]{while(!stop){int r=fn(11,22,33,44);assert(r==0 || r==9);++calls;}});
    while(calls.load()==0) std::this_thread::yield();
    for(unsigned i=0;i<10000;++i){assert(replace_delta(entry+3,plan.old_delta,plan.new_delta));assert(replace_delta(entry+3,plan.new_delta,plan.old_delta));}
    stop=true;running.join();assert(!memcmp(entry,photo,16));
    printf("PASS: photographed %s wrapper, zero argument preserved, existing JMP swap and rollback, concurrent execution (%u calls)\n",reg==0xc9?"READ":"STATE",calls.load());
    VirtualFree(memory,0,MEM_RELEASE);
}
int main() {
    JumpPlan plan={};assert(!plan_jump(read_bytes,0x10000,0x20000,0xc9,plan) && plan.worker==0xedf0);
    assert(!plan_jump(state_bytes,0x10000,0x20000,0xd2,plan) && plan.worker==0xec50);
    assert(plan_jump(read_bytes,0x10001,0x20000,0xc9,plan)==-2101);
    assert(plan_jump(read_bytes,0x10000,0x800000000ULL,0xc9,plan)==-2104);
    assert(plan_jump(read_bytes,0x10000,0x20000,0xd2,plan)==-2102);
    uint8_t changed[16];memcpy(changed,read_bytes,16);changed[7]=0xcc;
    assert(plan_jump(changed,0x10000,0x20000,0xc9,plan)==-2102);
    execute(read_bytes,0xc9);execute(state_bytes,0xd2);
}
