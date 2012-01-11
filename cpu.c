/*
 *  PocketGnuboy - GameBoy emulator for PocketPC
 *  Copyright (C) 2003  Y.Nagamidori
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  based on gnuboy 1.0.3
 *  Copyright (C) 2000-2001 Laguna and Gilgamesh
 */

#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "cpu.h"
#include "mem.h"
#include "fastmem.h"
#include "cpuregs.h"
#include "cpucore.h"

struct cpu cpu;

byte Z80toGB[256], GBtoZ80[256];

void cpu_reset()
{
        int i;

        cpu.speed = 0;
        cpu.halt = 0;
        cpu.div = 0;
        cpu.tim = 0;
        cpu.lcdc = 40;

        cpu.tac_speed = 0;
        cpu.tac_start = 0;

        IME = 0;

        xPC = 0x0100;
        xSP = 0xFFFE;
        xAF = 0x01B0;
        xBC = 0x0013;
        xDE = 0x00D8;
        xHL = 0x014D;

        if (hw.cgb) A = 0x11;
        if (hw.gba) B = 0x01;

        for (i = 0; i < 256; i++){
                Z80toGB[i] = ((i & 0x40) ? 0x80 : 0) |
                                        ((i & 0x10) ? 0x20 : 0) |
                                        ((i & 0x02) ? 0x40 : 0) |
                                        ((i & 0x01) ? 0x10 : 0);
                GBtoZ80[i] = ((i & 0x80) ? 0x40 : 0) |
                                        ((i & 0x40) ? 0x02 : 0) |
                                        ((i & 0x20) ? 0x10 : 0) |
                                        ((i & 0x10) ? 0x01 : 0);
        }
}

void div_advance()
{
        if (cpu.div >= 128)
        {
                R_DIV += (byte)(cpu.div >> 7);
                cpu.div &= 0xff;
        }
}

#define timer_advance(cnt) \
{ \
        if (cpu.tac_start) \
        { \
                cpu.tim += (cnt << cpu.tac_speed); \
                if (cpu.tim >= 512) \
                { \
                        int tima;        \
                        tima = R_TIMA + (cpu.tim >> 9); \
                        cpu.tim &= 0x1ff; \
                        if (tima >= 256) \
                        { \
                                hw_interrupt(IF_TIMER); \
                                while (tima >= 256) \
                                        tima = tima - 256 + R_TMA; \
                        } \
                        R_TIMA = tima; \
                } \
        } \
}

#define lcdc_advance(cnt)\
{\
        cpu.lcdc -= cnt;\
        if (cpu.lcdc <= 0) lcdc_trans();\
}\

#define sound_advance(cnt)\
{\
        cpu.snd += cnt;\
}\

INLINE int cpu_idle()
{
        int cnt, max;

        if (R_IF & R_IE)
        {
                cpu.halt = 0;
                return 0;
        }

        max = cpu.lcdc << cpu.speed;

        /* If timer interrupt cannot happen, this is very simple! */
        if (!((R_IE & IF_TIMER) && cpu.tac_start))
                cnt = max;
        else
        {
                /* Figure out when the next timer interrupt will happen */
                cnt = (511 - cpu.tim + (1<<cpu.tac_speed)) >> cpu.tac_speed;
                cnt += (255 - R_TIMA) << (9 - cpu.tac_speed);

                if (max < cnt)
                        cnt = max;
        }

        cpu.div += (cnt);
        timer_advance(cnt);
        cnt >>= cpu.speed;
        lcdc_advance(cnt);
        sound_advance(cnt);
        return cnt;
}

INLINE void cpu_interrupt()
{
        PRE_INT;
        switch (IF & IE)
        {
        case 0x01: case 0x03: case 0x05: case 0x07:
        case 0x09: case 0x0B: case 0x0D: case 0x0F:
        case 0x11: case 0x13: case 0x15: case 0x17:
        case 0x19: case 0x1B: case 0x1D: case 0x1F:
                THROW_INT(0); break;
        case 0x02: case 0x06: case 0x0A: case 0x0E:
        case 0x12: case 0x16: case 0x1A: case 0x1E:
                THROW_INT(1); break;
        case 0x04: case 0x0C: case 0x14: case 0x1C:
                THROW_INT(2); break;
        case 0x08: case 0x18:
                THROW_INT(3); break;
        case 0x10:
                THROW_INT(4); break;
        }
}

void cpu_emulate()
{
        byte op, cbop;
        int clen;
        union reg acc = {0};
        byte b;

        cpu.emulate = 1;
        while (cpu.emulate)
        {
                if (cpu.halt && IME)
                {
                        if (cpu_idle())
                                continue;
                }

                if (IME && (IF & IE))
                        cpu_interrupt();

                op = FETCH;
                clen = cycles_table[op];

                switch(op)
                {
#include "cpucodes.h"
                default:
                        //__assume(0);
                        return;
                }

                cpu.div += clen;
                timer_advance(clen);
                clen >>= cpu.speed;
                lcdc_advance(clen);
                sound_advance(clen);
        }
}





