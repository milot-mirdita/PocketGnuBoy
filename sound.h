#ifndef __SOUND_H__
#define __SOUND_H__


struct sndchan
{
        int on;
        unsigned int pos;
        int cnt, encnt, swcnt;
        int len, enlen, swlen;
        int swfreq;
        int freq;
        int envol, endir;
        int mixl, mixr;

        int hold;
        int type; // S1, S2 only
        int swdir, swshift; // S1 only
};


struct snd
{
        int rate;
        struct sndchan ch[4];
        byte wave[16];
        int master_voll, master_volr;
};


extern struct snd snd;






#endif
