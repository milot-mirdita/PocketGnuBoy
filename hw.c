#include "defs.h"
#include "cpu.h"
#include "hw.h"
#include "regs.h"
#include "lcd.h"
#include "mem.h"
#include "fastmem.h"


struct hw hw;

extern int dma[3];

/*
 * hw_interrupt changes the virtual interrupt lines included in the
 * specified mask to the values the corresponding bits in i take, and
 * in doing so, raises the appropriate bit of R_IF for any interrupt
 * lines that transition from low to high.
 */

void hw_interrupt(byte i)
{
        if (!(R_LCDC & 0x80) && (i & (IF_VBLANK | IF_STAT)))
                return;

        R_IF |= i;
        if (R_IF & R_IE) cpu.halt = 0;
}


/*
 * hw_dma performs plain old memory-to-oam dma, the original dmg
 * dma. Although on the hardware it takes a good deal of time, the cpu
 * continues running during this mode of dma, so no special tricks to
 * stall the cpu are necessary.
 */

INLINE hw_dma_copy(int da, int sa, int cnt)
{
        byte *src = mbc.rmap[sa>>12];
        if (src) {
#if 1
                switch (da >> 12) {
                case 8:
                case 9:
                        vram_copy(da & 0x1fff, src + sa, cnt);
                        break;
                default:
                        while (cnt--)
                                writeb(da++, src[sa++]);
                }
#else
                while (cnt--)
                        writeb(da++, src[sa++]);
#endif
        }
        else {
                while (cnt--)
                        writeb(da++, readb(sa++));
        }
}

void hw_dma(byte b)
{
        int i;
        int a;
        byte* ps;

        a = ((int)b) << 8;
        ps = mbc.rmap[a>>12];
        if (ps)
                memcpy(&lcd.oam.mem[0], &ps[a], 160);
        else {
                for (i = 0; i < 160; i++)
                        lcd.oam.mem[i] = readb(a + i);
        }
}

void hw_hdma_cmd(byte c)
{
        int cnt;
        int sa, da;

        /* Begin or cancel HDMA */
        if ((hw.hdma|c) & 0x80)
        {
                hw.hdma = c;
                R_HDMA5 = c & 0x7f;
                return;
        }

        /* Perform GDMA */
        sa = ((int)R_HDMA1 << 8) | (R_HDMA2&0xf0);
        da = 0x8000 | ((int)(R_HDMA3&0x1f) << 8) | (R_HDMA4&0xf0);
        cnt = ((int)c)+1;
        /* FIXME - this should use cpu time! */
        /*cpu_timers(102 * cnt);*/
        cnt <<= 4;

        hw_dma_copy(da, sa, cnt);
        sa += cnt;
        da += cnt;

        R_HDMA1 = sa >> 8;
        R_HDMA2 = sa & 0xF0;
        R_HDMA3 = 0x1F & (da >> 8);
        R_HDMA4 = da & 0xF0;
        R_HDMA5 = 0xFF;
}


void hw_hdma()
{
        int cnt;
        int sa, da;

        sa = ((int)R_HDMA1 << 8) | (R_HDMA2&0xf0);
        da = 0x8000 | ((int)(R_HDMA3&0x1f) << 8) | (R_HDMA4&0xf0);
        cnt = 16;

        hw_dma_copy(da, sa, cnt);
        sa += cnt;
        da += cnt;

        R_HDMA1 = sa >> 8;
        R_HDMA2 = sa & 0xF0;
        R_HDMA3 = 0x1F & (da >> 8);
        R_HDMA4 = da & 0xF0;
        R_HDMA5--;
        hw.hdma--;
}


/*
 * pad_refresh updates the P1 register from the pad states, generating
 * the appropriate interrupts (by quickly raising and lowering the
 * interrupt line) if a transition has been made.
 */

void pad_refresh()
{
        byte oldp1;
        oldp1 = R_P1;
        R_P1 &= 0x30;
        R_P1 |= 0xc0;
        if (!(R_P1 & 0x10))
                R_P1 |= (hw.pad & 0x0F);
        if (!(R_P1 & 0x20))
                R_P1 |= (hw.pad >> 4);
        R_P1 ^= 0x0F;
        if (oldp1 & ~R_P1 & 0x0F)
                hw_interrupt(IF_PAD);
}


/*
 * These simple functions just update the state of a button on the
 * pad.
 */

void pad_press(byte k)
{
        if (hw.pad & k)
                return;
        hw.pad |= k;
        pad_refresh();
}

void pad_release(byte k)
{
        if (!(hw.pad & k))
                return;
        hw.pad &= ~k;
        pad_refresh();
}

void pad_set(byte k, int st)
{
        st ? pad_press(k) : pad_release(k);
}

void hw_reset()
{
        hw.pad = 0;
        hw.vblank = 0;

        memset(ram.hi, 0, sizeof ram.hi);

        R_P1 = 0xFF;
        R_LCDC = 0x91;
        R_BGP = 0xFC;
        R_OBP0 = 0xFF;
        R_OBP1 = 0xFF;
        R_SVBK = 0x01;
        R_HDMA5 = 0xFF;
        R_VBK = 0xFE;
}

