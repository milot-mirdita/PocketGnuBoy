#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "fb.h"
#include "refresh.h"

struct lcd lcd;

struct scan scan;

#define BG (scan.bg)
#define WND (scan.wnd)
#define BUF (scan.buf)
#define PRI (scan.pri)

#define PAL1 (scan.pal1)
#define PAL2 (scan.pal2)
#define PAL4 (scan.pal4)

#define VS (scan.vs) /* vissprites */
#define NS (scan.ns)

#define L (scan.l) /* line */
#define X (scan.x) /* screen position */
#define Y (scan.y)
#define S (scan.s) /* tilemap position */
#define T (scan.t)
#define U (scan.u) /* position within tile */
#define V (scan.v)
#define WX (scan.wx)
#define WY (scan.wy)
#define WT (scan.wt)
#define WV (scan.wv)

byte patpix[4096][8][8];
byte patdirty[1024];
byte anydirty;

static int contrast = 0;
static int sprsort = 1;

//#define DEF_PAL { 0xFFFFFF, 0x68a0b0, 0x60707C, 0x2C3C3C } // white & brown
#define DEF_PAL_HIGH { 0xffffff, 0xAAAAAA, 0x555555, 0x000000 }
#define DEF_PAL_LOW { 0xffffff, 0xc0c0c0, 0x808080, 0x404040 }

static int dmg_pal_high[4][4] = { DEF_PAL_HIGH, DEF_PAL_HIGH, DEF_PAL_HIGH, DEF_PAL_HIGH };
static int dmg_pal_low[4][4] = { DEF_PAL_LOW, DEF_PAL_LOW, DEF_PAL_LOW, DEF_PAL_LOW };

static int usefilter, filterdmg;
static int filter[3][4] = {
        { 195,  25,   0,  35 },
        {  25, 170,  25,  35 },
        {  25,  60, 125,  40 }
};

static byte *vdest;

#ifdef ALLOW_UNALIGNED_IO /* long long is ok since this is i386-only anyway? */
#define MEMCPY8(d, s) ((*(long long *)(d)) = (*(long long *)(s)))
#else
#define MEMCPY8(d, s) memcpy((d), (s), 8)
#endif

static void updatepatpix()
{
        int i, j, k, l;
        int a, c;
        byte *vram = lcd.vbank[0];

        if (!anydirty) return;

        for (i = 0; i < 384; i++)
        {
                if (patdirty[i]) {
                        for (j = 0; j < 8; j++)
                        {
                                a = ((i<<4) | (j<<1));
                                for (k = 0; k < 8; k++)
                                {
                                        l = 1 << k;
                                        c = vram[a] & l ? 1 : 0;
                                        c |= vram[a+1] & l ? 2 : 0;
                                        patpix[i+1024][j][k] = c;
                                }
                                for (k = 0; k < 8; k++)
                                        patpix[i][j][k] =
                                                patpix[i+1024][j][7-k];
                        }
                        for (j = 0; j < 8; j++)
                        {
                                for (k = 0; k < 8; k++)
                                {
                                        patpix[i+2048][j][k] =
                                                patpix[i][7-j][k];
                                        patpix[i+3072][j][k] =
                                                patpix[i+1024][7-j][k];
                                }
                        }
                        patdirty[i] = 0;
                }
        }

        for (i = 512; i < 896; i++) {
                if (patdirty[i]) {
                        for (j = 0; j < 8; j++)
                        {
                                a = ((i<<4) | (j<<1));
                                for (k = 0; k < 8; k++)
                                {
                                        l = 1 << k;
                                        c = vram[a] & l ? 1 : 0;
                                        c |= vram[a+1] & l ? 2 : 0;
                                        patpix[i+1024][j][k] = c;
                                }
                                for (k = 0; k < 8; k++)
                                        patpix[i][j][k] =
                                                patpix[i+1024][j][7-k];
                        }
                        for (j = 0; j < 8; j++)
                        {
                                for (k = 0; k < 8; k++)
                                {
                                        patpix[i+2048][j][k] =
                                                patpix[i][7-j][k];
                                        patpix[i+3072][j][k] =
                                                patpix[i+1024][7-j][k];
                                }
                        }
                        patdirty[i] = 0;
                }
        }

        anydirty = 0;
}

static void tilebuf()
{
        int i, cnt;
        int base;
        byte *tilemap, *attrmap;
        int *tilebuf;
        int *wrap;
        static int wraptable[64] =
        {
                0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,-32
        };

        if (R_LCDC & 0x01)  // bg on
        {
                base = ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5) + S;
                tilemap = lcd.vbank[0] + base;
                attrmap = lcd.vbank[1] + base;
                tilebuf = BG;
                wrap = wraptable + S;
                cnt = ((WX + 7) >> 3) + 1;

                if (hw.cgb)
                {
                        if (R_LCDC & 0x10)
                                for (i = cnt; i > 0; i--)
                                {
                                        *(tilebuf++) = *tilemap
                                                | (((int)*attrmap & 0x08) << 6)
                                                | (((int)*attrmap & 0x60) << 5);
                                        *(tilebuf++) = (((int)*attrmap & 0x07) << 2);
                                        attrmap += *wrap + 1;
                                        tilemap += *(wrap++) + 1;
                                }
                        else
                                for (i = cnt; i > 0; i--)
                                {
                                        *(tilebuf++) = (256 + ((n8)*tilemap))
                                                | (((int)*attrmap & 0x08) << 6)
                                                | (((int)*attrmap & 0x60) << 5);
                                        *(tilebuf++) = (((int)*attrmap & 0x07) << 2);
                                        attrmap += *wrap + 1;
                                        tilemap += *(wrap++) + 1;
                                }
                }
                else
                {
                        if (R_LCDC & 0x10)
                                for (i = cnt; i > 0; i--)
                                {
                                        *(tilebuf++) = *(tilemap++);
                                        tilemap += *(wrap++);
                                }
                        else
                                for (i = cnt; i > 0; i--)
                                {
                                        *(tilebuf++) = (256 + ((n8)*(tilemap++)));
                                        tilemap += *(wrap++);
                                }
                }
        }

        if (WX >= 160) return;

        base = ((R_LCDC&0x40)?0x1C00:0x1800) + ((WT >> 3) << 5);
        tilemap = lcd.vbank[0] + base;
        attrmap = lcd.vbank[1] + base;
        tilebuf = WND;
        cnt = ((160 - WX) >> 3) + 1;

        if (hw.cgb)
        {
                if (R_LCDC & 0x10)
                        for (i = cnt; i > 0; i--)
                        {
                                *(tilebuf++) = *(tilemap++)
                                        | (((int)*attrmap & 0x08) << 6)
                                        | (((int)*attrmap & 0x60) << 5);
                                *(tilebuf++) = (((int)*(attrmap++)&7) << 2);
                        }
                else
                        for (i = cnt; i > 0; i--)
                        {
                                *(tilebuf++) = (256 + ((n8)*(tilemap++)))
                                        | (((int)*attrmap & 0x08) << 6)
                                        | (((int)*attrmap & 0x60) << 5);
                                *(tilebuf++) = (((int)*(attrmap++)&7) << 2);
                        }
        }
        else
        {
                if (R_LCDC & 0x10)
                        for (i = cnt; i > 0; i--)
                                *(tilebuf++) = *(tilemap++);
                else
                        for (i = cnt; i > 0; i--)
                                *(tilebuf++) = (256 + ((n8)*(tilemap++)));
        }
}

static void bg_scan()
{
        int cnt;
        byte *src, *dest;
        int *tile;

        if (WX <= 0) return;

        cnt = WX;
        tile = BG;
        dest = BUF;

        if (!(R_LCDC & 0x01)) { // bg off
                memset(dest, 64, WX);
                return;
        }

        src = patpix[*(tile++)][V] + U;
        memcpy(dest, src, 8-U);
        dest += 8-U;
        cnt -= 8-U;
        if (cnt <= 0) return;

        while (cnt >= 8)
        {
                src = patpix[*(tile++)][V];
                MEMCPY8(dest, src);
                dest += 8;
                cnt -= 8;
        }
        src = patpix[*(tile++)][V];
        memcpy(dest, src, cnt);
}

static void wnd_scan()
{
        int cnt;
        byte *src, *dest;
        int *tile;

        if (WX >= 160) return;

        cnt = 160 - WX;
        tile = WND;
        dest = BUF + WX;

        while (cnt >= 8)
        {
                src = patpix[*(tile++)][WV];
                MEMCPY8(dest, src);
                dest += 8;
                cnt -= 8;
        }
        src = patpix[*(tile++)][WV];
        memcpy(dest, src, cnt);
        WT++;
}

static void blendcpy(byte *dest, byte *src, byte b, int cnt)
{
        int i;
        for (i = 0; i < cnt; i++)
                dest[i] = src[i] | b;
}

static int priused(void *attr)
{
        un32 *a = attr;
        return (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);
}

static void bg_scan_pri()
{
        int cnt, i;
        byte *src, *dest;

        if (WX <= 0) return;

        if (!(R_LCDC & 0x01)) // bg off
                return;

        i = S;
        cnt = WX;
        dest = PRI;
        src = lcd.vbank[1] + ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5);

        /*if (!priused(src))
        {
                memset(dest, 0, cnt);
                return;
        }*/

        memset(dest, src[i++&31]&128, 8-U);
        dest += 8-U;
        cnt -= 8-U;
        if (cnt <= 0) return;
        while (cnt >= 8)
        {
                memset(dest, src[i++&31]&128, 8);
                dest += 8;
                cnt -= 8;
        }
        memset(dest, src[i&31]&128, cnt);
}

static void wnd_scan_pri()
{
        int cnt, i;
        byte *src, *dest;

        if (WX >= 160) return;

        i = 0;
        cnt = 160 - WX;
        dest = PRI + WX;
        src = lcd.vbank[1] + ((R_LCDC&0x40)?0x1C00:0x1800) + ((WT>>3)<<5);

        /*if (!priused(src))
        {
                memset(dest, 0, cnt);
                return;
        }*/

        while (cnt >= 8)
        {
                memset(dest, src[i++]&128, 8);
                dest += 8;
                cnt -= 8;
        }
        memset(dest, src[i]&128, cnt);
        WT++;
}

static void bg_scan_color()
{
        int cnt;
        byte *src, *dest;
        int *tile;

        if (WX <= 0) return;
        cnt = WX;
        tile = BG;
        dest = BUF;

        if (!(R_LCDC & 0x01)) { // bg off
                memset(dest, 64, WX);
                return;
        }

        src = patpix[*(tile++)][V] + U;
        blendcpy(dest, src, *(tile++), 8-U);
        dest += 8-U;
        cnt -= 8-U;
        if (cnt <= 0) return;

        while (cnt >= 8)
        {
                src = patpix[*(tile++)][V];
                blendcpy(dest, src, *(tile++), 8);
                dest += 8;
                cnt -= 8;
        }
        src = patpix[*(tile++)][V];
        blendcpy(dest, src, *(tile++), cnt);
}

static void wnd_scan_color()
{
        int cnt;
        byte *src, *dest;
        int *tile;

        if (WX >= 160) return;

        cnt = 160 - WX;
        tile = WND;
        dest = BUF + WX;

        while (cnt >= 8)
        {
                src = patpix[*(tile++)][WV];
                blendcpy(dest, src, *(tile++), 8);
                dest += 8;
                cnt -= 8;
        }
        src = patpix[*(tile++)][WV];
        blendcpy(dest, src, *(tile++), cnt);

        if (!NS) WT++;
}

static void recolor(byte *buf, byte fill, int cnt)
{
        int i;
        for (i = 0; i < cnt; i++)
                buf[i] |= fill;
}

static void spr_enum()
{
        int i, j;
        struct obj *o;
        struct vissprite ts[10];
        int v, pat;
        int l, x;

        NS = 0;
        if (!(R_LCDC & 0x02)) return;

        o = lcd.oam.obj;

        for (i = 0; i < 40; i++, o++)
        {
                if (L >= o->y || L + 16 < o->y)
                        continue;
                if (o->x <= -8 || o->x >= 168)
                        continue;
                if (L + 8 >= o->y && !(R_LCDC & 0x04))
                        continue;
                VS[NS].x = (int)o->x - 8;
                v = L - (int)o->y + 16;
                if (hw.cgb)
                {
                        pat = o->pat | (((int)o->flags & 0x60) << 5)
                                | (((int)o->flags & 0x08) << 6);
                        VS[NS].pal = 32 + ((o->flags & 0x07) << 2);
                }
                else
                {
                        pat = o->pat | (((int)o->flags & 0x60) << 5);
                        VS[NS].pal = 32 + ((o->flags & 0x10) >> 2);
                }
                VS[NS].pri = (o->flags & 0x80) >> 7;
                if ((R_LCDC & 0x04))
                {
                        pat &= ~1;
                        if (v >= 8)
                        {
                                v -= 8;
                                pat++;
                        }
                        if (o->flags & 0x40) pat ^= 1;
                }
                VS[NS].buf = patpix[pat][v];
                if (++NS == 10) break;
        }
        if (!sprsort || hw.cgb) return;
        /* not quite optimal but it finally works! */
        for (i = 0; i < NS; i++)
        {
                l = 0;
                x = VS[0].x;
                for (j = 1; j < NS; j++)
                {
                        if (VS[j].x < x)
                        {
                                l = j;
                                x = VS[j].x;
                        }
                }
                ts[i] = VS[l];
                VS[l].x = 160;
        }
        memcpy(VS, ts, sizeof VS);
}

static void spr_scan()
{
        int i, x;
        byte pal, b, ns = NS;
        byte *src, *dest, *bg, *pri;
        struct vissprite *vs;
        static byte bgdup[256];

        if (!ns) return;

        if (!(R_LCDC & 0x02)) // object off
                return;

        memcpy(bgdup, BUF, 256);
        vs = &VS[ns-1];

        for (; ns; ns--, vs--)
        {
                x = vs->x;
                if (x >= 160) continue;
                if (x <= -8) continue;
                if (x < 0)
                {
                        src = vs->buf - x;
                        dest = BUF;
                        i = 8 + x;
                }
                else
                {
                        src = vs->buf;
                        dest = BUF + x;
                        if (x > 152) i = 160 - x;
                        else i = 8;
                }
                pal = vs->pal;
                if (vs->pri)
                {
                        bg = bgdup + (dest - BUF);
                        while (i--)
                        {
                                b = src[i];
                                if (b && !(bg[i]&3)) dest[i] = pal|b;
                        }
                }
                else if (hw.cgb)
                {
                        bg = bgdup + (dest - BUF);
                        pri = PRI + (dest - BUF);
                        while (i--)
                        {
                                b = src[i];
                                if (b && (!pri[i] || !(bg[i]&3)))
                                        dest[i] = pal|b;
                        }
                }
                else while (i--) if (src[i]) dest[i] = pal|src[i];
                /* else while (i--) if (src[i]) dest[i] = 31 + ns; */
        }
}

void lcd_begin()
{
        fb.frames++;
        vdest = fb.ptr + ((fb.w * fb.pelsize) >> 1)
                - (80 * fb.pelsize) + ((fb.h >> 1) - 72) * fb.pitch;
        WY = R_WY;
        WT = 0;
}

void lcd_refreshline()
{
        if (!fb.enabled) return;

        if (!(R_LCDC & 0x80))
        {
                //memset(BUF, 64, 160);
                //refresh_2(vdest, BUF, PAL2, 160);
                //fb.dirty = 0;

                vdest += fb.pitch;
                return;
        }

        updatepatpix();

        L = R_LY;
        X = R_SCX;
        Y = (R_SCY + L) & 0xff;
        S = X >> 3;
        T = Y >> 3;
        U = X & 7;
        V = Y & 7;

        WX = R_WX - 7;
        if (WY>L || WY<0 || WY>143 || WX<-7 || WX>159 || !(R_LCDC&0x20))
                WX = 160;
        WV = (L - WY) & 7;

        spr_enum();
        tilebuf();
        if (hw.cgb)
        {
                bg_scan_color();
                wnd_scan_color();
                if (NS)
                {
                        bg_scan_pri();
                        wnd_scan_pri();
                }
        }
        else
        {
                bg_scan();
                wnd_scan();
                recolor(BUF+WX, 0x04, 160-WX);
        }
        spr_scan();

        fb.dirty = 0;
        refresh_2((un16*)vdest, BUF, PAL2, 160);
        vdest += fb.pitch;
}

static void updatepalette(int i)
{
        int c, r, g, b;

        c = (lcd.pal[i<<1] | ((int)lcd.pal[(i<<1)|1] << 8)) & 0x7FFF;
        r = (c & 0x001F) << 3;
        g = (c & 0x03E0) >> 2;
        b = (c & 0x7C00) >> 7;
        r |= (r >> 5);
        g |= (g >> 5);
        b |= (b >> 5);
        r = (r >> 3) << 11; // for RGB565
        g = (g >> 2) << 5;
        b = (b >> 3);
        c = r|g|b;
        PAL2[i] = c;
}

void pal_write(int i, byte b)
{
        if (lcd.pal[i] != b) {
                lcd.pal[i] = b;
                updatepalette(i>>1);
        }
}

void pal_write_dmg(int i, int mapnum, byte d)
{
        int j;
        int *cmap = contrast ? dmg_pal_high[mapnum] : dmg_pal_low[mapnum];
        int c, r, g, b;

        if (hw.cgb) return;

        /* if (mapnum >= 2) d = 0xe4; */
        for (j = 0; j < 8; j += 2)
        {
                c = cmap[(d >> j) & 3];
                r = (c & 0xf8) >> 3;
                g = (c & 0xf800) >> 6;
                b = (c & 0xf80000) >> 9;
                c = r|g|b;
                /* FIXME - handle directly without faking cgb */
                pal_write(i+j, c & 0xff);
                pal_write(i+j+1, c >> 8);
        }
}

void vram_write(int a, byte b)
{
        lcd.vbank[R_VBK&1][a] = b;
        if (a >= 0x1800) return;
        patdirty[((R_VBK&1)<<9)+(a>>4)] = 1;
        anydirty = 1;
}

void vram_copy(int a, byte* src, int cnt)
{
        int i;
        int bank = R_VBK & 1;
        int bank2 = bank << 9;

        memcpy(&lcd.vbank[R_VBK&1][a], src, cnt);

        if (a >= 0x1800)
                return;

        cnt >>= 4;
        for (i = 0; i < cnt; i++) {
                patdirty[bank2 + (a >> 4)] = 1;
                a += (1 << 4);
        }
        anydirty = 1;
}

void vram_dirty()
{
        anydirty = 1;
        memset(patdirty, 1, sizeof patdirty);
}

void pal_dirty()
{
        int i;
        if (!hw.cgb)
        {
                pal_write_dmg(0, 0, R_BGP);
                pal_write_dmg(8, 1, R_BGP);
                pal_write_dmg(64, 2, R_OBP0);
                pal_write_dmg(72, 3, R_OBP1);
        }
        for (i = 0; i < 64; i++)
                updatepalette(i);
        PAL2[64] = 0xFFFF; // for bg off
}

void lcd_reset()
{
        memset(&lcd, 0, sizeof lcd);
        lcd_begin();
        vram_dirty();
        pal_dirty();
}

void lcd_set_contrast(int high)
{
        contrast = high;
        pal_dirty();
}

int lcd_get_contrast()
{
        return contrast;
}

