#ifndef __FASTMEM_H__
#define __FASTMEM_H__


#include "defs.h"
#include "mem.h"


INLINE byte readb(int a)
{
        byte *p = mbc.rmap[a>>12];
        if (p) return p[a];
        else return mem_read(a);
}

INLINE void writeb(int a, byte b)
{
        byte *p = mbc.wmap[a>>12];
        if (p) p[a] = b;
        else mem_write(a, b);
}

INLINE int readw(int a)
{
#ifdef IS_LITTLE_ENDIAN
        return readb(a) | (readb(a + 1) << 8);
#else
        return readb(a + 1) | (readb(a) << 8);
#endif
}

INLINE void writew(int a, int w)
{
#ifdef IS_LITTLE_ENDIAN
  writeb(a, (byte)w);
  writeb(a + 1, (byte)(w >> 8));
#else
  /* ! not checked ! */
  writeb(a + 1, w);
  writeb(a, w >> 8);
#endif
}

INLINE byte readhi(int a)
{
        return readb(a | 0xff00);
}

INLINE void writehi(int a, byte b)
{
        writeb(a | 0xff00, b);
}

#endif
