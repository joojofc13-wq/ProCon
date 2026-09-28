#pragma once
#include <stdint.h>
#include <stddef.h>

struct ProReport {
    uint32_t buttons;
    uint16_t lx, ly, rx, ry;
};

// Nintendo report 0x30; sticks use packed 12-bit values.
static inline bool decode_report(const uint8_t *p, size_t n, ProReport *out) {
    if (!p || !out || n < 49 || p[0] != 0x30) return false;
    out->buttons = (uint32_t)p[3] | ((uint32_t)p[4] << 8) | ((uint32_t)p[5] << 16);
    out->lx = p[6] | ((p[7] & 15) << 8);
    out->ly = (p[7] >> 4) | (p[8] << 4);
    out->rx = p[9] | ((p[10] & 15) << 8);
    out->ry = (p[10] >> 4) | (p[11] << 4);
    return true;
}
