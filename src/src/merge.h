#pragma once
#include "mapping.h"
template<class Pad> static inline void merge_mapped(Pad &pad,const Mapped &m) {
    pad.buttons|=m.buttons;
    if(m.lx!=128) pad.leftStick.x=m.lx;
    if(m.ly!=128) pad.leftStick.y=m.ly;
    if(m.rx!=128) pad.rightStick.x=m.rx;
    if(m.ry!=128) pad.rightStick.y=m.ry;
    if(m.l2) pad.analogButtons.l2=m.l2;
    if(m.r2) pad.analogButtons.r2=m.r2;
}
