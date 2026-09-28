#pragma once
#include <orbis/Pad.h>
#include "hook.h"
#include "snapshot.h"
#include "merge.h"
static_assert(sizeof(OrbisPadData)==120,"Pad size");
static_assert(offsetof(OrbisPadData,connected)==76,"Pad connected offset");
static_assert(offsetof(OrbisPadData,timestamp)==80,"Pad timestamp offset");
static Snapshot current_input={},keyboard_input={},mouse_input={};
static Hook read_hook={},state_hook={};
static int active=0,disabled=0,target_handle=-1;
static unsigned delivered=0,kbm_delivered=0;
static void clear_input() {Mapped m={};publish(current_input,m,0);}
static void combine(int handle,OrbisPadData &pad) {
    if(!__atomic_load_n(&active,__ATOMIC_ACQUIRE) || !pad.connected ||
       (pad.buttons&0x80000000u) || __atomic_load_n(&disabled,__ATOMIC_RELAXED)) return;
    int expected=-1;
    __atomic_compare_exchange_n(&target_handle,&expected,handle,false,__ATOMIC_RELAXED,__ATOMIC_RELAXED);
    if(handle!=__atomic_load_n(&target_handle,__ATOMIC_RELAXED)) return;
    if((pad.buttons&0xc08)==0xc08) {__atomic_store_n(&disabled,1,__ATOMIC_RELEASE);return;}
    Mapped sample={};
    uint64_t now=sceKernelGetProcessTime();
    if(read_snapshot(current_input,sample,now)) {
        merge_mapped(pad,sample);__atomic_add_fetch(&delivered,1,__ATOMIC_RELAXED);
    }
    if(read_snapshot(keyboard_input,sample,now)){merge_mapped(pad,sample);__atomic_add_fetch(&kbm_delivered,1,__ATOMIC_RELAXED);}
    if(read_snapshot(mouse_input,sample,now))merge_mapped(pad,sample);
}
static int read_bridge(int handle,OrbisPadData *data,int count) {
    auto original=(int(*)(int,OrbisPadData*,int,int))read_hook.plan.worker;
    int r=original(handle,data,count,0);
    if(data && r>0 && r<=count) for(int i=0;i<r;++i) combine(handle,data[i]);
    return r;
}
static int state_bridge(int handle,OrbisPadData *data) {
    auto original=(int(*)(int,OrbisPadData*,int))state_hook.plan.worker;
    int r=original(handle,data,0);if(!r && data) combine(handle,*data);return r;
}
static void explain_entry(const char *label,const void *entry,int result) {
    Text t={};t.add("ProCon: ");t.add(label);t.add(" ");t.hex((uint32_t)result);t.add(" bytes: ");
    const uint8_t *p=(const uint8_t*)entry;
    size_t available=0x4000-((uintptr_t)entry&0x3fff);
    if(p) for(unsigned i=0;i<16 && i<available;++i) {char x[4]={"0123456789ABCDEF"[p[i]>>4],"0123456789ABCDEF"[p[i]&15],' ',0};t.add(x);}
    notify(t.data);
}
static bool install_integration() {
    notify("ProCon P1: Opening Pad.");
    int id=load_module("libScePad.sprx");if(id<0) return false;
    void *read=0,*state=0;
    int r=sceKernelDlsym(id,"scePadRead",&read);
    if(r<0 || !read) {error("Resolve scePadRead",r);return false;}
    r=sceKernelDlsym(id,"scePadReadState",&state);
    if(r<0 || !state) {error("Resolve scePadReadState",r);return false;}
    if(read==state) {error("Control functions overlapped",-2006);return false;}
    r=prepare_hook(read_hook,read,(void*)read_bridge,0xc9);
    if(r) {explain_entry("Read input refused",read,r);return false;}
    r=prepare_hook(state_hook,state,(void*)state_bridge,0xd2);
    if(r) {explain_entry("State input refused",state,r);return false;}
    notify("ProCon P2: Patterns verified. Enabling Read.");
    r=switch_hook(read_hook,true);
    if(!r) {notify("ProCon P3: Read active. Enabling State.");r=switch_hook(state_hook,true);}
    if(r) {
        if(read_hook.live) {int undo=switch_hook(read_hook,false);if(undo) error("Restore Read",undo);}
        if(state_hook.live) {int undo=switch_hook(state_hook,false);if(undo) error("Restore State",undo);}
        error("Enable integration",r);return false;
    }
    __atomic_store_n(&active,1,__ATOMIC_RELEASE);return true;
}
