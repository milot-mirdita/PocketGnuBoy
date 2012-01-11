#include "defs.h"
#include "regs.h"
#include "mem.h"
#include "hw.h"
#include "rtc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int mbc_table[256] =
{
        0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        3, 3, 3, 3, 0, 0, 0, 0, 0, 5, 5, 5, MBC_RUMBLE, MBC_RUMBLE, MBC_RUMBLE, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, MBC_HUC3, MBC_HUC1
};

static int rtc_table[256] =
{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0
};

static int batt_table[256] =
{
        0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
        0
};

static int romsize_table[256] =
{
        2, 4, 8, 16, 32, 64, 128, 256, 512,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 128, 128, 128
        /* 0, 0, 72, 80, 96  -- actual values but bad to use these! */
};

static int ramsize_table[256] =
{
        1, 1, 1, 4, 16,
        8 /* FIXME - what value should this be?! */
};


static char *romfile = 0;
static char *sramfile = 0;
static char *rtcfile = 0;
static char *saveprefix = 0;

static char *savename = 0;
static char savedir[512] = {0};

static int saveslot = 0;

static int forcebatt, nobatt;
static int forcedmg, gbamode;

static int memfill = -1, memrand = -1;

static void initmem(void *mem, int size)
{
        char *p = mem;
        if (memrand >= 0)
        {
                srand(memrand ? memrand : time(0));
                while(size--) *(p++) = rand();
        }
        else if (memfill >= 0)
                memset(p, memfill, size);
        else
        {
                memset(p, 0, size);
        }
}

static byte *loadfile(FILE *f, int *len)
{
        long size;
        byte *d = 0;

        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);

        d = malloc(size);
        if (!d) return 0;
        *len = fread(d, 1, size, f);
        return d;
}

static byte *inf_buf;
static int inf_pos, inf_len, inf_error;

static void inflate_callback(byte b)
{
        if (inf_error)
                return;

        if (!inf_len) {
                inf_buf = malloc(16384);
                inf_len = 16384;
        }

        if (inf_pos == 16384) {
                byte *inf_head;
                int len;

                inf_head = inf_buf;
                len = romsize_table[inf_head[0x0148]];
                if (!len) {
                        free(inf_buf);
                        inf_error = 1;
                        return;
                }
                len *= 16384;

                inf_buf = malloc(len);
                memcpy(inf_buf, inf_head, 16384);

                free(inf_head);

                inf_len = len;
        }
        if (inf_len < inf_pos) {
                free(inf_buf);
                inf_error = 1;
                return;
        }

        inf_buf[inf_pos++] = b;
}

static byte *decompress(byte *data, int *len)
{
        unsigned long pos = 0;
        if (data[0] == 0x1f && data[1] == 0x8b) {
                inf_buf = 0;
                inf_pos = inf_len = 0;
                inf_error = 0;
                if (unzip(data, &pos, inflate_callback) < 0)
                        return data;
                if (inf_error)
                        return data;
                *len = inf_pos;
                free(data);
                return inf_buf;
        }
        if (data[0] == 0x50 && data[1] == 0x4b) {
               // inf_buf = inflate_zip(data, len);
                free(data);
                return inf_buf;
        }
        else return data;
}


int rom_load()
{
        FILE *f;
        byte c, *data, *header;
        int len = 0, rlen;

        f = fopen(romfile, "rb");
        if (!f)
                return -1;

        data = loadfile(f, &len);
        header = data = decompress(data, &len);

        if (len == 0) {
                fclose(f);
                return -1;
        }

        mem_init();
        memcpy(rom.name, header+0x0134, 16);
        if (rom.name[14] & 0x80) rom.name[14] = 0;
        if (rom.name[15] & 0x80) rom.name[15] = 0;
        rom.name[16] = 0;

        c = header[0x0147];
        mbc.type = mbc_table[c];
        mbc.batt = (batt_table[c] && !nobatt);// || forcebatt;
        memset(&rtc, 0, sizeof(rtc));
        rtc.batt = rtc_table[c];
        mbc.romsize = romsize_table[header[0x0148]];
        mbc.ramsize = ramsize_table[header[0x0149]];

        if (!mbc.romsize)
                return -1;
        if (!mbc.ramsize)
                return -1;

        rlen = 16384 * mbc.romsize;
        if (rlen > len)
                return -1;
        rom.bank = data;

        ram.sbank = malloc(8192 * mbc.ramsize);

        initmem(ram.sbank, 8192 * mbc.ramsize);
        initmem(ram.ibank, 4096 * 8);

        mbc.rombank = 1;
        mbc.rambank = 0;

        c = header[0x0143];
        hw.cgb = ((c == 0x80) || (c == 0xc0)) && !forcedmg;
        hw.gba = (hw.cgb && gbamode);

        fclose(f);
        return 0;
}

int sram_load()
{
        FILE *f;

        if (!mbc.batt || !sramfile || !*sramfile) return -1;

        /* Consider sram loaded at this point, even if file doesn't exist */
        ram.loaded = 1;

        f = fopen(sramfile, "rb");
        if (!f) return -1;
        fread(ram.sbank, 8192, mbc.ramsize, f);
        fclose(f);

        return 0;
}


int sram_save()
{
        FILE *f;

        /* If we crash before we ever loaded sram, DO NOT SAVE! */
        if (!mbc.batt || !sramfile || !ram.loaded || !mbc.ramsize)
                return -1;

        f = fopen(sramfile, "wb");
        if (!f) return -1;
        fwrite(ram.sbank, 8192, mbc.ramsize, f);
        fclose(f);

        return 0;
}


void state_save(int n)
{
        FILE *f;
        char *name;

        if (n < 0) n = saveslot;
        if (n < 0) n = 0;
        name = malloc(strlen(saveprefix) + 5);
        sprintf(name, "%s.%03d", saveprefix, n);

        if ((f = fopen(name, "wb")))
        {
                savestate(f);
                fclose(f);
        }
        free(name);
}


void state_load(int n)
{
        FILE *f;
        char *name;

        if (n < 0) n = saveslot;
        if (n < 0) n = 0;
        name = malloc(strlen(saveprefix) + 5);
        sprintf(name, "%s.%03d", saveprefix, n);

        if ((f = fopen(name, "rb")))
        {
                loadstate(f);
                fclose(f);
                vram_dirty();
                pal_dirty();
                sound_dirty();
                mem_updatemap();
        }
        free(name);
}

void rtc_save()
{
        FILE *f;
        if (!rtc.batt) return;
        if (!(f = fopen(rtcfile, "wb"))) return;
        rtc_save_internal(f);
        fclose(f);
}

void rtc_load()
{
        FILE *f;
        if (!rtc.batt) return;
        if (!(f = fopen(rtcfile, "r"))) return;
        rtc_load_internal(f);
        fclose(f);
}


void loader_unload()
{
        sram_save();
        rtc_save();
        if (romfile) free(romfile);
        if (sramfile) free(sramfile);
        if (rtcfile) free(rtcfile);
        if (saveprefix) free(saveprefix);
        if (rom.bank) free(rom.bank);
        if (ram.sbank) free(ram.sbank);
        romfile = sramfile = saveprefix = rtcfile = 0;
        rom.bank = 0;
        ram.sbank = 0;
        mbc.type = mbc.romsize = mbc.ramsize = mbc.batt = 0;
}

static char *base(char *s)
{
        char *p;
        p = strrchr(s, '\\');
        if (p) return p+1;
        return s;
}

static void create_filenames()
{
        char dir[512];
        char name[512];
        char *p, *r;

        if (!romfile)
                return;

        if (!strlen(savedir))
        {
                r = strrchr(romfile, '\\');
                if (r)
                {
                        memset(dir, 0, r - romfile + 1);
                        strncpy(dir, romfile, r - romfile);
                }
                else
                        sprintf(dir, "\\");
        }
        else
                strcpy(dir, savedir);

        strcpy(name, base(romfile));
        p = strchr(name, '.');
        if (p) *p = 0;

        if (saveprefix) free(saveprefix);
        saveprefix = malloc(strlen(dir) + strlen(name) + 2);
        sprintf(saveprefix, "%s\\%s", dir, name);

        if (sramfile) free(sramfile);
        sramfile = malloc(strlen(saveprefix) + 5);
        strcpy(sramfile, saveprefix);
        strcat(sramfile, ".sav");

        if (rtcfile) free(rtcfile);
        rtcfile = malloc(strlen(saveprefix) + 5);
        strcpy(rtcfile, saveprefix);
        strcat(rtcfile, ".rtc");
}

int loader_init(char *s)
{
        romfile = malloc(strlen(s) + 1);
        strcpy(romfile, s);

        if (rom_load() != 0) {
                loader_unload();
                return 0;
        }

        //vid_settitle(rom.name);

        create_filenames();

        sram_load();
        rtc_load();

        return 1;
}

char* get_romfile()
{
        return romfile;
}

int get_saveslot()
{
        return saveslot;
}

void set_saveslot(int slot)
{
        saveslot = slot;
}

void set_savedir(char* dir)
{
        strcpy(savedir, dir);
        create_filenames();
}

void get_savedir(char* dir)
{
        strcpy(dir, savedir);
}

