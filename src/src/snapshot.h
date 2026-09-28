#pragma once
#include "mapping.h"
// Single writer; multiple readers.
struct Snapshot { uint32_t sequence; uint64_t value,time; };
static inline void publish(Snapshot &s,const Mapped &m,uint64_t time) {
    uint64_t value=m.buttons | (uint64_t)m.lx<<24 | (uint64_t)m.ly<<32 |
                   (uint64_t)m.rx<<40 | (uint64_t)m.ry<<48;
    __atomic_add_fetch(&s.sequence,1,__ATOMIC_SEQ_CST);
    __atomic_store_n(&s.value,value,__ATOMIC_SEQ_CST);
    __atomic_store_n(&s.time,time,__ATOMIC_SEQ_CST);
    __atomic_add_fetch(&s.sequence,1,__ATOMIC_SEQ_CST);
}
static inline bool read_snapshot(const Snapshot &s,Mapped &m,uint64_t now) {
    for(unsigned retry=0;retry<3;++retry) {
        uint32_t before=__atomic_load_n(&s.sequence,__ATOMIC_SEQ_CST);
        if(before&1) continue;
        uint64_t v=__atomic_load_n(&s.value,__ATOMIC_SEQ_CST);
        uint64_t at=__atomic_load_n(&s.time,__ATOMIC_SEQ_CST);
        if(before!=__atomic_load_n(&s.sequence,__ATOMIC_SEQ_CST)) continue;
        if(!fresh_sample(now,at)) return false;
        m.buttons=(uint32_t)(v&0xffffff);m.lx=v>>24;m.ly=v>>32;m.rx=v>>40;m.ry=v>>48;
        m.l2=(m.buttons&0x100)?255:0;m.r2=(m.buttons&0x200)?255:0;
        return true;
    }
    return false;
}
