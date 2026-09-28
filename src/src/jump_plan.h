#pragma once
#include <stdint.h>
#include <stddef.h>
struct JumpPlan {uint32_t old_delta,new_delta;uintptr_t worker;};
static inline uint32_t read_u32(const uint8_t *p) {
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
// Expected entry: XOR, JMP rel32, 9 NOPs.
static inline int plan_jump(const uint8_t *bytes,uintptr_t entry,uintptr_t replacement,
                            uint8_t reg,JumpPlan &plan) {
    if(!bytes || (entry&7) || (entry&0x3fff)>0x3ff0) return -2101;
    if(reg!=0xc9 && reg!=0xd2) return -2102;
    if(bytes[0]!=0x31 || bytes[1]!=reg || bytes[2]!=0xe9) return -2102;
    for(unsigned i=7;i<16;++i) if(bytes[i]!=0x90) return -2102;
    int32_t original=(int32_t)read_u32(bytes+3);
    if(original>=-16 || original<-(1<<20)) return -2103;
    int64_t relative=(int64_t)replacement-(int64_t)(entry+7);
    if(relative<-2147483648LL || relative>2147483647LL) return -2104;
    plan.old_delta=(uint32_t)original;plan.new_delta=(uint32_t)(int32_t)relative;
    plan.worker=(uintptr_t)((int64_t)(entry+7)+original);
    return 0;
}
// LOCK CMPXCHG updates the unaligned displacement within one cache line.
static inline bool replace_delta(uint8_t *address,uint32_t expected,uint32_t desired) {
    unsigned char success;
    __asm__ volatile("lock cmpxchgl %3, %1; sete %0"
        : "=q"(success), "+m"(*(volatile uint8_t (*)[4])address), "+a"(expected)
        : "r"(desired) : "memory","cc");
    return success!=0;
}
