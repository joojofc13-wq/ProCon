#pragma once
#include <orbis/libkernel.h>
#include "jump_plan.h"
struct Hook {uint8_t *entry;JumpPlan plan;uint8_t reg;bool live;};
static inline int prepare_hook(Hook &h,void *entry,void *replacement,uint8_t reg) {
    h.entry=(uint8_t*)entry;h.reg=reg;
    return plan_jump(h.entry,(uintptr_t)entry,(uintptr_t)replacement,reg,h.plan);
}
static inline int switch_hook(Hook &h,bool enable) {
    if(!h.entry || !h.plan.worker) return -2105;
    if(h.entry[0]!=0x31 || h.entry[1]!=h.reg || h.entry[2]!=0xe9) return -2102;
    uintptr_t base=(uintptr_t)h.entry&~(uintptr_t)0x3fff;
    int r=sceKernelMprotect((void*)base,0x4000,7);if(r) return r;
    bool changed=replace_delta(h.entry+3,enable?h.plan.old_delta:h.plan.new_delta,
                               enable?h.plan.new_delta:h.plan.old_delta);
    if(changed) h.live=enable;
    int restored=sceKernelMprotect((void*)base,0x4000,5);
    if(!changed) return -2106;
    return restored;
}
