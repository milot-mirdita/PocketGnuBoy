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
#include "lcd.h"
#include "stdlib.h"
#include "rtc.h"

static const int framelen = 16743;
static int emu_run = 0;
static int init = 0;

void emu_reset();
void emu_pause();
void emu_step();

void emu_init()
{
        vid_init();
        pcm_init();
        emu_reset();
}

void emu_close()
{
        vid_close();
        pcm_close();
        loader_unload();

        emu_run = 0;
}

void emu_reset()
{
        sram_save();
        rtc_save();

        hw_reset();
        lcd_reset();
        cpu_reset();
        mbc_reset();
        sound_reset();

        init = 0;
}

void emu_pause()
{
        emu_run = 0;
        vid_suspend();
        pcm_suspend();
        joy_close();
}

void emu_resume()
{
        emu_run = 1;
        vid_resume();
        pcm_resume();
        joy_init();
}

int emu_get_run()
{
        return emu_run;
}

INLINE void emu_step()
{
        cpu_emulate(cpu.lcdc);
}

void emu_do_frame()
{
        vid_start_frame();
        vid_begin();
        cpu_emulate();
        vid_end();
        joy_process();
        rtc_tick();
        sound_mix();
        pcm_submit();

        vid_end_frame();
}
