#include <stdlib.h>

#include "defs.h"
#include "hw.h"
#include "cpu.h"
#include "regs.h"


#define C (cpu.lcdc)

void stat_change(byte b)
{
        R_STAT = 0x80 | ((R_STAT & 0x07) | (b & 0x78));
        if (!hw.cgb && (R_LCDC & 0x80) && (R_STAT & 0x03) == 1 && R_LY != 0)
                hw_interrupt(IF_STAT);

        if ((R_LY == 0) && (R_LYC == 0) && (R_STAT & 0x40))
                hw_interrupt(IF_STAT);
}

void lcdc_change(byte b)
{
        if ((b & 0x80) && !(R_LCDC & 0x80)) {
                R_STAT = (R_STAT & 0xFC) | 2;
                R_LY = 0;
                lcd_begin();
        } else if (!(b & 0x80) && (R_LCDC & 0x80)) {
                R_STAT = R_STAT & 0xFC;
        R_LY = 0;
        lcd_begin();
        }
        R_LCDC = b;
}

void ly_change(byte b)
{
}

INLINE void lyc_compare()
{
        if ((R_STAT & 0x40) && (R_LY == R_LYC))
                hw_interrupt(IF_STAT);
}

INLINE void stat_change0()
{
        R_STAT = (R_STAT & 0xFC);
        if (R_STAT & 0x08)
                hw_interrupt(IF_STAT);
}

INLINE void stat_change1()
{
        R_STAT = (R_STAT & 0xFC) | 1;
        if (R_STAT & 0x10)
                hw_interrupt(IF_STAT);
}

INLINE void stat_change2()
{
        R_STAT = (R_STAT & 0xFC) | 2;
        if (R_STAT & 0x20)
                hw_interrupt(IF_STAT);
}

INLINE void stat_change3()
{
        R_STAT = (R_STAT & 0xFC) | 3;
}

void lcdc_trans()
{
        if (!R_LCDC & 0x80)
        {
                while (C <= 0)
                {
                        R_LY++;
                        C += 228;

                        if (R_LY > 77) //?
                                R_STAT = (R_STAT & 0xFC) | 0x02;
                        else
                                R_STAT = R_STAT & 0xFC;

                        if (R_LY >= 154)
                        {
                                R_LY = 0;
                                lcd_begin();
                                cpu.emulate = 0;
                        }
                }
                return;
        }
        while (C <= 0)
        {
                switch ((byte)(R_STAT & 3))
                {
                        case 0:
                                R_LY++;

                                lyc_compare();
                                if (R_LY == 0x90) {
                                        stat_change1();
                                        hw.vblank = 1;
                                        C += 8;
                                }
                                else {
                                        stat_change2();
                                        C += 40;
                                }
                                break;
                        case 1:
                                if (hw.vblank) {
                                        hw_interrupt(IF_VBLANK);
                                        hw.vblank = 0;
                                        C += 228;
                                        break;
                                }

                                if (R_LY == 0) {
                                        stat_change2();
                                        C += 40;

                                        lcd_begin();
                                        break;
                                }

                                if (R_LY < 152)
                                        C += 228;
                                else if (R_LY == 152)
                                        C += 28;
                                else
                                {
                                        R_LY = -1;
                                        C += 200;
                                        cpu.emulate = 0;
                                }

                                R_LY++;
                                lyc_compare();
                                stat_change1();
                                break;
                        case 2:
                                stat_change3();
                                lcd_refreshline();
                                C += 86;
                                break;
                        case 3:
                                stat_change0();
                                if (hw.hdma & 0x80)
                                        hw_hdma();
                                C += 102;
                                break;
                }
        }
}
